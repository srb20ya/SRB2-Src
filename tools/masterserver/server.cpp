// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: server.cpp,v 1.10 2001/02/16 00:33:16 hurdler Exp $
//
// Copyright (C) 2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: server.cpp,v $
// Revision 1.10  2001/02/16 00:33:16  hurdler
// Better solution
//
// Revision 1.9  2001/02/15 17:00:34  crashrl
// added htonl and ntohl for interendieness compatibility
//
// Revision 1.8  2000/10/22 00:24:12  hurdler
// Add version support
//
// Revision 1.7  2000/10/16 21:10:45  hurdler
// Improving stats
//
// Revision 1.6  2000/09/01 23:30:32  hurdler
// minor changes (stats, log & timeout)
//
// Revision 1.5  2000/08/21 11:08:46  hurdler
// Improve the stats
//
// Revision 1.4  2000/08/20 14:12:47  hurdler
// Adding statistics
//
// Revision 1.3  2000/08/19 14:33:05  hurdler
// changes on log file management
//
// Revision 1.2  2000/08/19 11:31:03  hurdler
// Adding password and log file access tru server
//
// Revision 1.1.1.1  2000/08/18 10:32:12  hurdler
// Initial import of the Master Server code into CVS
//
//
// DESCRIPTION:
//
//
//-----------------------------------------------------------------------------

#include <unistd.h>      //
#include <typeinfo>
#include <string.h>
#include <stdarg.h>
#include "ipcs.h"
#include "common.h"
#include "srvlist.h"
#include "stats.h"

//=============================================================================

static CServerList     servers_list;
static CServerSocket   server_socket;
static CServerStats    server_stats;

FILE *logfile;

//=============================================================================


/*
** checkPassword()
*/
static int checkPassword(char *pw)
{
    char *cpw;

    pw[15] = '\0'; // for security reason
    cpw = pCrypt(pw, "04");
    memset(pw, 0, 16); // erase that ASAP!
    if (strcmp(cpw, "04JjB22hBsuiE"))
    {
        logPrintf(logfile, "Bad password\n");
        return 0;
    }
    return 1;
}


/*
** sendServersInformations()
*/
static void sendServersInformations(int id)
{
    msg_t       msg;
    CServerItem *p = (CServerItem *) servers_list.getFirst();

    logPrintf(logfile, "Sending servers informations\n");
    msg.id   = id;
    msg.type = SEND_SERVER_MSG;
    while (p)
    {
        const char *str = p->getString();

        msg.length = strlen(str)+1; // send also the '\0'
        strcpy(msg.buffer, str);
        dbgPrintf(CYAN, "Writing: (%d)\n%s\n", msg.length, msg.buffer);
        if (server_socket.write(&msg) < 0)
        {
            dbgPrintf(LIGHTRED, "Write error... client %d deleted\n", id);
            return;
        }
        p = (CServerItem *) servers_list.getNext();
    }
    msg.length = 0;
    dbgPrintf(CYAN, "Writing: (%d) %s\n", msg.length, "");
    if (server_socket.write(&msg) < 0)
    {
        dbgPrintf(LIGHTRED, "Write error... client %d deleted\n", id);
    }
    server_stats.num_retrieval++;
}


/*
** sendShortServersInformations()
*/
static void sendShortServersInformations(int id)
{
    msg_t       msg;
    CServerItem *p = (CServerItem *) servers_list.getFirst();

    logPrintf(logfile, "Sending short servers informations\n");
    msg.id   = id;
    msg.type = SEND_SHORT_SERVER_MSG;
    while (p)
    {
        msg_server_t *info = (msg_server_t *) msg.buffer;

        info->header[0] = '\0'; // nothing interresting in it (for now)
        strcpy(info->ip,      p->getIP());
        strcpy(info->port,    p->getPort());
        strcpy(info->name,    p->getName());
        strcpy(info->version, p->getVersion());

        msg.length = sizeof(msg_server_t);
        if (server_socket.write(&msg) < 0)
        {
            dbgPrintf(LIGHTRED, "Write error... client %d deleted\n", id);
            return;
        }
        p = (CServerItem *) servers_list.getNext();
    }
    msg.length = 0;
    dbgPrintf(CYAN, "Writing: (%d) %s\n", msg.length, "");
    if (server_socket.write(&msg) < 0)
    {
        dbgPrintf(LIGHTRED, "Write error... client %d deleted\n", id);
    }
    server_stats.num_retrieval++;
}


/*
** addServer()
*/
static void addServer(int id, char *buffer)
{
    msg_server_t *info;
    
    //TODO: Be sure there is no flood from a given IP: 
    //      We won't allow more than 2 servers by IP with that kind of add. 
    //      If a host need more than 2 servers, then it should be registrated 
    //      manually

    info = (msg_server_t *) buffer;

    // I want to be sure the informations are correct, of course!
    info->port[sizeof(info->port)-1] = '\0';
    info->name[sizeof(info->name)-1] = '\0';
    info->version[sizeof(info->version)-1] = '\0';
    // retrieve the true ip of the server
    strcpy(info->ip, server_socket.getClientIP(id));

    logPrintf(logfile, "Adding the temporary server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
    servers_list.insert(info->ip, info->port, info->name, info->version, ST_TEMPORARY);
    server_stats.num_servers++;
    server_stats.num_add++;
    server_stats.putLastServer(info);
}


/*
** addPermanentServer()
*/
static void addPermanentServer(char *buffer)
{
    msg_server_t *info;

    info = (msg_server_t *) buffer;

    // I want to be sure the informations are correct, of course!
    info->ip[sizeof(info->ip)-1]     = '\0';
    info->port[sizeof(info->port)-1] = '\0';
    info->name[sizeof(info->name)-1] = '\0';
    info->version[sizeof(info->version)-1] = '\0';

    if (!checkPassword(info->header))
        return;

    logPrintf(logfile, "Adding the pemanent server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
    servers_list.insert(info->ip, info->port, info->name, info->version, ST_PERMANENT);
    //servers_list.insert(new CServerItem(info->ip, info->port, info->name, info->version, ST_PERMANENT));
    server_stats.num_servers++;
    server_stats.num_add++;
    server_stats.putLastServer(info);
}


/*
** removeServer()
*/
static void removeServer(int id, char *buffer)
{
    msg_server_t *info;

    info = (msg_server_t *) buffer;

    // I want to be sure the informations are correct, of course!
    info->port[sizeof(info->port)-1] = '\0';
    info->name[sizeof(info->name)-1] = '\0';
    info->version[sizeof(info->version)-1] = '\0';

    // retrieve the true ip of the server
    // TODO: do the same for the port? is it possible without problem?
    strcpy(info->ip, server_socket.getClientIP(id));

    logPrintf(logfile, "Removing the temporary server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
    if (servers_list.remove(info->ip, info->port, info->name, info->version, ST_TEMPORARY))
        server_stats.num_servers--;
    else
        server_stats.num_badconnection++;
    server_stats.num_removal++;
}


/*
** removePermanentServer()
*/
static void removePermanentServer(char *buffer)
{
    msg_server_t *info;

    info = (msg_server_t *) buffer;

    // I want to be sure the informations are correct, of course!
    info->ip[sizeof(info->ip)-1]     = '\0';
    info->port[sizeof(info->port)-1] = '\0';
    info->name[sizeof(info->name)-1] = '\0';
    info->version[sizeof(info->version)-1] = '\0';

    if (!checkPassword(info->header))
        return;

    logPrintf(logfile, "Removing the pemanent server: %s %s %s %s\n", info->ip, info->port, info->name, info->version);
    if (servers_list.remove(info->ip, info->port, info->name, info->version, ST_PERMANENT))
        server_stats.num_servers--;
    server_stats.num_removal++;
}


/*
** sendFile()
*/
static void sendFile(int id, char *buffer, FILE *f)
{
    msg_t        msg;
    int          count;
    msg_server_t *info;

    info = (msg_server_t *) buffer;
    if (!checkPassword(info->header))
        return;
    
    logPrintf(logfile, "Sending file\n");
    msg.id   = id;
    msg.type = SEND_FILE_MSG;
    fseek(f, 0, SEEK_SET);
    while ( (count=fread(msg.buffer, 1, PACKET_SIZE, f)) > 0 )
    {
        msg.length = count;
        if (count != PACKET_SIZE) // send a null terminated string
        {
            msg.length++;
            msg.buffer[count] = '\0';
        }
        if (server_socket.write(&msg) < 0)
        {
            dbgPrintf(LIGHTRED, "Write error... client %d deleted\n", id);
            return;
        }
    }
    msg.length = 0;
    dbgPrintf(CYAN, "Writing: (%d) %s\n", msg.length, "");
    if (server_socket.write(&msg) < 0)
    {
        dbgPrintf(LIGHTRED, "Write error... client %d deleted\n", id);
    }
}


/*
** sendHttpLine()
*/
static void sendHttpLine(int id, char *lpFmt, ...)
{
    msg_t   msg;
    va_list arglist;
    
    va_start(arglist, lpFmt);
    vsprintf(msg.buffer, lpFmt, arglist);
    va_end  (arglist);
    strcat(msg.buffer, "\n");

    msg.id   = id;
    msg.type = SEND_HTTP_REQUEST_MSG;
    msg.length = strlen(msg.buffer);
    if (server_socket.write(&msg) < 0)
    {
        dbgPrintf(LIGHTRED, "Write error... client %d deleted\n", id);
        return;
    }
}


/*
** sendHttpServersList()
*/
static void sendHttpServersList(int id)
{
    CServerItem *p = (CServerItem *) servers_list.getFirst();

    sendHttpLine(id, "<FONT COLOR=\"#00FF00\"><CODE>");
    if (!p)
    {
        sendHttpLine(id, "<A NAME=\"%23s\">No server connected.</A>","00000000000000000000000");
    }
    else while (p)
    {
        sendHttpLine(id, "<A NAME=\"%23s\">IP: %15s    Port: %5s    Name: %s    Version: %s </A><BR>",
                        p->getGuid(),p->getIP(), p->getPort(), p->getName(), p->getVersion());
        p = (CServerItem *) servers_list.getNext();
    }
    sendHttpLine(id, "</CODE></FONT>");
}

static void sendtextServersList(int id)
{
    CServerItem *p = (CServerItem *) servers_list.getFirst();

    if (!p)
    {
        sendHttpLine(id, "No SRB2 Games are running, so why don't you start one?");
    }
    else while (p)
    {
        sendHttpLine(id, "IP: %15s | Port: %5s | Name: %s | Version: %s",
                        p->getIP(), p->getPort(), p->getName(), p->getVersion());
        p = (CServerItem *) servers_list.getNext();
    }
}

static void sendRSS10ServersList(int id)
{
    CServerItem *p = (CServerItem *) servers_list.getFirst();
    
    sendHttpLine(id, "<items> <rdf:Seq>");
    if (!p)
    {
        sendHttpLine(id, "<rdf:li rdf:resource=\"http://srb2.kicks-ass.net/srb2ms_status.php#00000000000000000000000\" />");
    }
    else while (p)
    {
        sendHttpLine(id, "<rdf:li rdf:resource=\"http://srb2.kicks-ass.net/srb2ms_status.php#%23s\" />",p->getGuid());
        p = (CServerItem *) servers_list.getNext();
    }
    
    sendHttpLine(id, "</rdf:Seq> </items> </channel>");
    
    p = (CServerItem *) servers_list.getFirst();

    if (!p)
    {
        sendHttpLine(id, "<item rdf:about=\"http://srb2.kicks-ass.net/srb2ms_status.php#00000000000000000000000\"><title>No servers</title><dc:description>I'm sorry, but no servers are running now</dc:description></item>");
    }
    else while (p)
    {
        sendHttpLine(id, "<item rdf:about=\"http://srb2.kicks-ass.net/srb2ms_status.php#%23s\"><title>%s</title><dc:description>| IP: %15s | Port: %5s | Version: %s</dc:description><dc:date>%24s</dc:date><guid isPermaLink=\"false\">%23s</guid></item>", p->getGuid(), p->getName(), p->getIP(), p->getPort(), p->getVersion(), p->getRegtime(), p->getGuid());
        p = (CServerItem *) servers_list.getNext();
    }
}

static void sendRSS92ServersList(int id)
{
    CServerItem *p = (CServerItem *) servers_list.getFirst();

    if (!p)
    {
        sendHttpLine(id, "<item><title>No servers</title><description>I'm sorry, but no servers are running now</description></item>");
    }
    else while (p)
    {
        sendHttpLine(id, "<item><title>%s</title><description>IP: %15s | Port: %5s | Version: %s</description></item>", p->getName(), p->getIP(), p->getPort(), p->getVersion());
        p = (CServerItem *) servers_list.getNext();
    }
}



/*
** sendHttpRequest()
*/
static void sendHttpRequest(int id)
{
    server_stats.num_http_con++;
    logPrintf(logfile, "Sending http request\n");
    // Status
    sendHttpLine(id, "<P>Server up and running since: %s (%d days %d hours)<BR>",
                    server_stats.getUptime(), server_stats.getDays(), server_stats.getHours());
    sendHttpLine(id, "Total number of connections: %d (%d HTTP requests)<BR>",
                    server_stats.num_connections, server_stats.num_http_con);
    sendHttpLine(id, "Number of bad connections: %d<BR>",
                    server_stats.num_badconnection);
    sendHttpLine(id, "Current number of servers: %d<BR></P>",
                    server_stats.num_servers);
    // Motd
    sendHttpLine(id, "<H3>Message of the day</H3><P>%s</P>",
                    server_stats.getMotd());
    // Usage
    sendHttpLine(id, "<H3>Usage</H3>");
    sendHttpLine(id, "<P>Number of server adds: %d<BR>",
                    server_stats.num_add);
    sendHttpLine(id, "Number of server removals: %d<BR>",
                    server_stats.num_removal);
    sendHttpLine(id, "Number of auto removals: %d<BR>",
                    server_stats.num_autoremoval);
    sendHttpLine(id, "Number of list retrievals: %d<BR></P>",
                    server_stats.num_retrieval);
    // Servers' list
    sendHttpLine(id, "<H3>Servers' list</H3>");
    sendHttpServersList(id);
    // Last server registered
    sendHttpLine(id, "<H3>Last server registered</H3>");
    sendHttpLine(id, "<P><PRE>%s</PRE>",
                    server_stats.getLastServers());
    // Version                
    sendHttpLine(id, "<H3>Version</H3>");
    sendHttpLine(id, "<P>Build date/time: %s</P>",
                    server_stats.getVersion());
                    
    server_socket.deleteClient(id); // close connection with the script
}

static void sendtextRequest(int id)
{
    server_stats.num_text_con++;
    logPrintf(logfile, "Sending text request\n");
    sendtextServersList(id);
    server_socket.deleteClient(id); // close connection with the script
}

static void sendRSS92Request(int id)
{
    server_stats.num_RSS92_con++;
    logPrintf(logfile, "Sending RSS .92 feed items request\n");
    sendRSS92ServersList(id);
    server_socket.deleteClient(id); // close connection with the script
}

static void sendRSS10Request(int id)
{
    server_stats.num_RSS10_con++;
    logPrintf(logfile, "Sending RSS 1.0 feed items request\n");
    sendRSS10ServersList(id);
    server_socket.deleteClient(id); // close connection with the script
}


/*
** eraseLogFile()
*/
static void eraseLogFile(char *buffer)
{
    msg_server_t    *info;

    info = (msg_server_t *) buffer;
    if (!checkPassword(info->header))
        return;
    
    logPrintf(logfile, "Erasing file\n"); // well, quite useless!
    fclose(logfile);
    logfile = fopen("server.log", "w+t");
}


/*
** pingServers(): for now, this is a fake ping, since we expect that the servers 
**                show us by themselves they are still alive.
*/
static void pingServers()
{
    static time_t   last_ping_time = 0;
    const  time_t   ping_timeout = 20;
           time_t   cur_time = time(NULL);
    
    if (cur_time-last_ping_time > ping_timeout/2) // it's high time to "ping"
    {
        CServerItem *p = (CServerItem *) servers_list.getFirst();
        while (p)
        {
            if ( (cur_time-p->ping_time > ping_timeout) && // do not allow a ping higher than ping_timeout
                 (p->type != ST_PERMANENT) )
            {
                CServerItem *q = p;
                p = (CServerItem *) servers_list.getNext();
                if (servers_list.remove(q))
                {
                    server_stats.num_servers--;
                    server_stats.num_autoremoval++;
                }
            }
            else
            {
                p = (CServerItem *) servers_list.getNext();
            }
        }
        last_ping_time = cur_time;
    }
}


/*
** updateServerPing()
*/
static void updateServerPing()
{
    CServerItem *p = (CServerItem *) servers_list.getFirst();
    const char        *from = server_socket.getUdpIP();

    while (p)
    {
        if (!strcmp(from, p->getIP()))
        {
            p->ping_time = time(NULL);
            return;
        }
        p = (CServerItem *) servers_list.getNext();
    }
    logPrintf(logfile, "Warning: unknown UDP connection from %s\n", from);
}


/*
** analyseMessage()
*/
static int analyseMessage(msg_t *msg)
{
    dbgPrintf(DEFCOL, "%li, %li, %li: ", msg->id, msg->type,msg->length);
    switch (msg->type)
    {
        case UDP_RECV_MSG:
            updateServerPing();
            dbgPrintf(LIGHTGREEN, "Got an UDP message\n");
            break;
        case ACCEPT_MSG:
            server_stats.num_connections++;
            dbgPrintf(LIGHTGREEN, "Got a new player\n");
            break;
        case ADD_SERVER_MSG:
            addServer(msg->id, msg->buffer);
            break;
        case REMOVE_SERVER_MSG:
            removeServer(msg->id, msg->buffer);
            break;
        case ADD_PSERVER_MSG:
            addPermanentServer(msg->buffer);
            break;
        case REMOVE_PSERVER_MSG:
            removePermanentServer(msg->buffer);
            break;
        case GET_LOGFILE_MSG:
            sendFile(msg->id, msg->buffer, logfile);
            break;
        case ERASE_LOGFILE_MSG:
            eraseLogFile(msg->buffer);
            break;
        case ADD_CLIENT_MSG:
            logPrintf(logfile, "New client <unsupported>\n");
            //TODO: add him in the player list
            break;
        case GET_SERVER_MSG:
            sendServersInformations(msg->id);
            break;
        case GET_SHORT_SERVER_MSG:
            sendShortServersInformations(msg->id);
            break;
        case HTTP_REQUEST_MSG:
            sendHttpRequest(msg->id);
            return 0;
        case TEXT_REQUEST_MSG:
            sendtextRequest(msg->id);
            return 0;
        case RSS92_REQUEST_MSG:
            sendRSS92Request(msg->id);
            return 0;
        case RSS10_REQUEST_MSG:
            sendRSS10Request(msg->id);
            return 0;
        default:
            logPrintf(logfile, "Warning: unknown message: %d (from %d)\n", msg->type, msg->id);
            return INVALID_MSG;
    }
    return 0;
}


/*
** main()
*/
int main(int argc, char *argv[])
{
    msg_t   msg;

    if (argc > 1)
    {
        if (server_socket.listen(argv[1]) < 0)
        {
            printf("Error while initializing the server\n");
        }
        else
        {
            logfile = openFile("server.log");
#if !defined (DEBUG) && !defined(_WIN32) && !defined (_WIN64)
            switch (fork())
            {
                case  0: break;  // son
                case -1: printf("Error while launching the server in background\n"); return -1;
                default: return 0; // father: keep son in background
            }
#endif
            srand((unsigned)time(NULL)); //Alam: GUIDs
            for ( ; ; )
            {
                memset(&msg, 0, sizeof(msg)); // remove previous message
                if (!server_socket.read(&msg))
                {
                    // valid message: header message seems ok
                    analyseMessage(&msg);
                    //servers_list.show(); // for debug purpose
                }
                pingServers();
            }
        }
    }
    printf("syntax: %s port\n", argv[0]);
    return 0;
}
