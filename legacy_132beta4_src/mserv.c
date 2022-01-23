// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: mserv.c,v 1.31 2001/05/14 19:02:58 metzgermeister Exp $
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: mserv.c,v $
// Revision 1.31  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.30  2001/05/12 16:33:08  hurdler
// fix master server registration problem under some OS
//
// Revision 1.29  2001/03/03 19:44:50  ydario
// Added OS/2 headers
//
// Revision 1.28  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.27  2001/02/16 01:17:55  hurdler
// No need to convert msg->id
//
// Revision 1.26  2001/02/16 00:45:07  hurdler
// Better solution
//
// Revision 1.25  2001/01/11 01:15:57  hurdler
// Fix Little/Big Endian issue
//
// Revision 1.24  2001/01/05 18:17:43  hurdler
// fix master server bug
//
// Revision 1.23  2000/11/26 00:46:31  hurdler
// small bug fixes
//
// Revision 1.22  2000/10/22 00:38:22  hurdler
// Fix %s to %d in version string
//
// Revision 1.21  2000/10/22 00:20:53  hurdler
// Updated for the latest master server code
//
// Revision 1.20  2000/10/21 23:21:56  hurdler
// Minor updates
//
// Revision 1.19  2000/10/21 08:43:29  bpereira
// no message
//
// Revision 1.18  2000/10/17 10:09:27  hurdler
// Update master server code for easy connect from menu
//
// Revision 1.17  2000/10/16 20:02:29  bpereira
// no message
//
// Revision 1.16  2000/10/08 13:30:01  bpereira
// no message
//
// Revision 1.15  2000/10/07 18:36:50  hurdler
// fix a bug with Win2k
//
// Revision 1.14  2000/10/01 15:20:23  hurdler
// Add private server
//
// Revision 1.13  2000/09/14 10:39:59  hurdler
// Fix compiling problem under win32
//
// Revision 1.12  2000/09/10 10:45:14  metzgermeister
// *** empty log message ***
//
// Revision 1.11  2000/09/08 22:28:30  hurdler
// merge masterserver_ip/port in one cvar, add -private
//
// Revision 1.10  2000/09/02 15:38:24  hurdler
// Add master server to menus (temporaray)
//
// Revision 1.9  2000/09/01 18:23:42  hurdler
// fix some issues with latest network code changes
//
// Revision 1.8  2000/08/29 15:53:47  hurdler
// Remove master server connect timeout on LAN (not connected to Internet)
//
// Revision 1.7  2000/08/21 12:44:45  hurdler
// fix SOCKET not defined under some OS
//
// Revision 1.6  2000/08/21 11:06:44  hurdler
// Add ping and some fixes
//
// Revision 1.5  2000/08/16 23:39:41  hurdler
// fix a bug with windows sockets
//
// Revision 1.4  2000/08/16 17:21:50  hurdler
// update master server code (bis)
//
// Revision 1.3  2000/08/16 16:24:45  ydario
// OS/2 also needs inet_aton
//
// Revision 1.2  2000/08/16 15:44:18  hurdler
// update master server code
//
// Revision 1.1  2000/08/16 14:04:57  hurdler
// add master server code
//
//
//
// DESCRIPTION:
//      Commands used for communicate with the master server
//
//-----------------------------------------------------------------------------


#ifdef WIN32
#include <windows.h>     // socket(),...
#else
#include <unistd.h>
#ifdef __OS2__
#include <sys/types.h>
#endif
#include <sys/socket.h>  // socket(),...
#include <sys/time.h>    // timeval,... (TIMEOUT)
#include <netinet/in.h>  // sockaddr_in
#include <arpa/inet.h>   // inet_addr(),...
#include <netdb.h>       // gethostbyname(),...
#include <sys/ioctl.h>
#include <errno.h>
/*
#include <string.h>      // memset(),...
#include <sys/types.h>   // socket(),...
*/
#endif

#ifdef __OS2__
#include <errno.h>
#endif

#include "doomdef.h"
#include "command.h"
#include "console.h"
#include "mserv.h"
#include "i_tcp.h"
#include "i_net.h"
#include "i_system.h"
#include "d_clisrv.h"

// ================================ DEFINITIONS ===============================

#define PACKET_SIZE 1024

#define  MS_NO_ERROR                   0
#define  MS_SOCKET_ERROR            -201
#define  MS_CONNECT_ERROR           -203
#define  MS_WRITE_ERROR             -210
#define  MS_READ_ERROR              -211
#define  MS_CLOSE_ERROR             -212
#define  MS_GETHOSTBYNAME_ERROR     -220
#define  MS_GETHOSTNAME_ERROR       -221
#define  MS_TIMEOUT_ERROR           -231

// see master server code for the values
#define ADD_SERVER_MSG               101
#define REMOVE_SERVER_MSG            103
#define GET_SERVER_MSG               200
#define GET_SHORT_SERVER_MSG         205

#define HEADER_SIZE ((long)sizeof(long)*3)

#define HEADER_MSG_POS      0
#define IP_MSG_POS         16
#define PORT_MSG_POS       32
#define HOSTNAME_MSG_POS   40

#ifndef SOCKET
#define SOCKET int
#endif

typedef struct {
    long    id;
    long    type;
    long    length;
    char    buffer[PACKET_SIZE];
} msg_t;

struct Copy_CVarMS_t
{
    char ip[64];
    char port[8];
    char name[64];
} registered_server;

// win32 or djgpp
#if defined( WIN32) || defined( __DJGPP__ ) 
#define ioctl ioctlsocket
#define close closesocket
#endif

#if defined( WIN32) || defined( __OS2__)
// it seems windows doesn't define that... maybe some other OS? OS/2
int inet_aton(char *hostname, struct in_addr *addr)
{
    return ( (addr->s_addr=inet_addr(hostname)) != INADDR_NONE );
}   
#endif

static void Command_Listserv_f(void);
//TODO: when we change the port or ip, unregister to the old master server, register to the new one

#define DEF_PORT "28910"
consvar_t cv_internetserver= {"internetserver",                 "No", 0, CV_YesNo };
consvar_t cv_masterserver  = {"masterserver",   "srb2.ssntails.org:28910", CV_SAVE, NULL };
consvar_t cv_servername    = {"servername",     "SRB2 server", CV_SAVE, NULL };

enum { MSCS_NONE, MSCS_WAITING, MSCS_REGISTERED, MSCS_FAILED } con_state = MSCS_NONE;

#define NEWCODE
#ifndef NEWCODE
static SOCKET               mysocket;        // UDP socket
static int                  current_port;
static struct sockaddr_in   udp_addr;
#else
static int msnode=-1;
#define current_port sock_port
#endif

static SOCKET               socket_fd = -1;  // TCP/IP socket
static struct sockaddr_in   addr;
static struct timeval       select_timeout;
static fd_set               wset;

static int  MS_Connect(char *ip_addr, char *str_port, int async);
static int  MS_Read(msg_t *msg);
static int  MS_Write(msg_t *msg);
static int  MS_GetIP(char *);

void AddMServCommands(void)
{
    CV_RegisterVar(&cv_internetserver);
    CV_RegisterVar(&cv_masterserver);
    CV_RegisterVar(&cv_servername);
    COM_AddCommand("listserv", Command_Listserv_f);
}

static void CloseConnection(void)
{
    if (socket_fd > 0)
        close(socket_fd);
    socket_fd = -1;
}

static int GetServersList(void)
{
    msg_t   msg;
    int     count = 0;

    msg.type = GET_SERVER_MSG;
    msg.length = 0;
    if (MS_Write(&msg) < 0)
        return MS_WRITE_ERROR;

    while (MS_Read(&msg) >= 0)
    {
        if (msg.length == 0)
        {
            if (!count)
                CONS_Printf("No server currently running.\n");
            return MS_NO_ERROR;
        }
        count++;
        CONS_Printf(msg.buffer);
    }


    return MS_READ_ERROR;
}

#define NUM_LIST_SERVER 10
msg_server_t *GetShortServersList(void)
{
    static msg_server_t server_list[NUM_LIST_SERVER+1]; // +1 for easy test
    msg_t               msg;
    int                 i;

    //arf, we must be connected to the master server before writing to it
    if (MS_Connect(GetMasterServerIP(), GetMasterServerPort(), 0))
    {
        CONS_Printf("cannot connect to the master server\n");
        return NULL;
    }

    msg.type = GET_SHORT_SERVER_MSG;
    msg.length = 0;
    if (MS_Write(&msg) < 0)
        return NULL;

    for (i=0; (i<NUM_LIST_SERVER) && (MS_Read(&msg)>=0); i++)
    {
        if (msg.length == 0)
        {
            server_list[i].header[0] = 0;
            CloseConnection();
            return server_list;
        }
        memcpy(&server_list[i], msg.buffer, sizeof(msg_server_t));
        server_list[i].header[0] = 1;
    }
    CloseConnection();
    if (i==NUM_LIST_SERVER)
    {
        server_list[i].header[0] = 0;
        return server_list;
    }
    else
        return NULL;
}

static void Command_Listserv_f(void)
{
    if (con_state == MSCS_WAITING)
    {
        CONS_Printf("Not yet registered to the master server.\n");
        return;
    }

    CONS_Printf("Retrieving server list...\n");

    if (MS_Connect(GetMasterServerIP(), GetMasterServerPort(), 0))
    {
        CONS_Printf("cannot connect to the master server\n");
        return;
    }

    if (GetServersList())
        CONS_Printf("cannot get server list\n");

    CloseConnection();
}

static char *int2str(int n)
{
    int         i;
    static char res[16];

    res[15] = '\0';
    res[14] = (n%10)+'0';
    for (i=13; (n /= 10); i--)
        res[i] = (n%10)+'0';

    return &res[i+1];
}

int ConnectionFailed(void)
{
    con_state = MSCS_FAILED;
    CONS_Printf("Connection to master server failed\n");
    CloseConnection();
    return MS_CONNECT_ERROR;
}

static int AddToMasterServer(void)
{
    static int      retry = 0;
    int             i, j, res;
    msg_t           msg;
    msg_server_t    *info = (msg_server_t *) msg.buffer;
    fd_set          tset;

    memcpy(&tset, &wset, sizeof(tset));
    res = select(socket_fd+1, NULL, &tset, NULL, &select_timeout);
    if (res == 0)
    {
        if (retry++ > 30) // an about 30 second timeout
        {
            retry = 0;
	    CONS_Printf("Timeout on masterserver\n");
            return ConnectionFailed();
        }
        return MS_CONNECT_ERROR;
    }
    retry = 0;
    if (res < 0)
    {
	CONS_Printf("Error on select : %s\n", strerror(errno));
        return ConnectionFailed();
    }
    
    // so, the socket is writable, but what does that mean, that the connection is
    // ok, or bad... let see that!
    j = 4;
    getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (char *)&i, (size_t *)&j);
    if (i != 0) // it was bad
    {
        CONS_Printf("getsockopt: %s\n", strerror(errno));
	return ConnectionFailed();
    }
    
    strcpy(info->header, "");
    strcpy(info->ip,     "");
    strcpy(info->port,   int2str(current_port));
    strcpy(info->name,   cv_servername.string);
    sprintf(info->version, "%d.%d.%d", VERSION/100, VERSION%100, SUBVERSION);
    strcpy(registered_server.name, cv_servername.string);

    msg.type = ADD_SERVER_MSG;
    msg.length = sizeof(msg_server_t);
    if (MS_Write(&msg) < 0)
        return ConnectionFailed();

    CONS_Printf("The server has been registered on the master server...\n");
    con_state = MSCS_REGISTERED;
    CloseConnection();

    return MS_NO_ERROR;
}

static int RemoveFromMasterSever(void)
{
    msg_t           msg;
    msg_server_t    *info = (msg_server_t *) msg.buffer;

    strcpy(info->header, "");
    strcpy(info->ip,     "");
    strcpy(info->port,   int2str(current_port));
    strcpy(info->name,   registered_server.name);
    sprintf(info->version, "%d.%d.%d", VERSION/100, VERSION%100, SUBVERSION);

    msg.type = REMOVE_SERVER_MSG;
    msg.length = sizeof(msg_server_t);
    if (MS_Write(&msg) < 0)
        return MS_WRITE_ERROR;

    return MS_NO_ERROR;
}

char *GetMasterServerPort(void)
{
    char *t = cv_masterserver.string;

    while ((*t != ':') && (*t != '\0'))
        t++;

    if (*t)
        return ++t;
    else
        return DEF_PORT;
}

char *GetMasterServerIP(void)
{
    static char str_ip[64];
    char        *t = str_ip;

    strcpy(t, cv_masterserver.string);

    while ((*t != ':') && (*t != '\0'))
        t++;
    *t = '\0';
    
    return str_ip;
}



static void openUdpSocket()
{
#ifdef NEWCODE
    if( I_NetMakeNode )
    {
        char hostname[24];

        sprintf(hostname, "%s:%d", inet_ntoa(addr.sin_addr), atoi(GetMasterServerPort())+1);
        msnode = I_NetMakeNode(hostname);
    }
    else
        msnode = -1;
#else
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(atoi(GetMasterServerPort())+1);
    udp_addr.sin_addr.s_addr = addr.sin_addr.s_addr; // same IP as for TCP
#endif
}

void RegisterServer(int s, int port)
{
    CONS_Printf("Registering this server to the master server...\n");

    strcpy(registered_server.ip, GetMasterServerIP());
    strcpy(registered_server.port, GetMasterServerPort());
    //current_port = port;
    //mysocket = s;

    if (MS_Connect(registered_server.ip, registered_server.port, 1))
    {
        CONS_Printf("cannot connect to the master server\n");
        return;
    }
    openUdpSocket();

    // keep the TCP connection open until AddToMasterServer() is completed;
}

void SendPingToMasterServer(void)
{
    static tic_t   next_time = 0;
    tic_t          cur_time;

    cur_time = I_GetTime();
    if (cur_time > next_time) // ping every 2 second if possible
    {
        next_time = cur_time+2*TICRATE;

        if (con_state == MSCS_WAITING)
            AddToMasterServer();

        if (con_state != MSCS_REGISTERED)
            return;

        // cur_time is just a dummy data to send
#ifdef NEWCODE
        *((tic_t *)netbuffer) = cur_time;
        doomcom->datalength = sizeof(cur_time);
        doomcom->remotenode = msnode;
        I_NetSend();
#else
        sendto(mysocket, (char*)&cur_time, sizeof(cur_time), 0, (struct sockaddr *)&udp_addr, sizeof(struct sockaddr));
#endif
    }
}

void UnregisterServer()
{
    if (con_state != MSCS_REGISTERED)
    {
        con_state = MSCS_NONE;
        CloseConnection();
        return;
    }
    con_state = MSCS_NONE;

    CONS_Printf("Unregistering this server to the master server...\n");

    if (MS_Connect(registered_server.ip, registered_server.port, 0))
    {
        CONS_Printf("cannot connect to the master server\n");
        return;
    }

    if (RemoveFromMasterSever() < 0)
        CONS_Printf("cannot remove this server from the master server\n");

    CloseConnection();
#ifdef NEWCODE
    I_NetFreeNodenum( msnode );
#endif
}

/*
** MS_GetIP()
*/
static int MS_GetIP(char *hostname)
{
    struct hostent *host_ent;

    if (!inet_aton(hostname, &addr.sin_addr)) {
        //TODO: only when we are connected to Internet, or use a non bloking call
        host_ent = gethostbyname(hostname);
        if (host_ent==NULL)
            return MS_GETHOSTBYNAME_ERROR;
        memcpy(&addr.sin_addr, host_ent->h_addr_list[0], sizeof(struct in_addr));
    }
    return 0;
}


/*
** MS_Connect()
*/
static int MS_Connect(char *ip_addr, char *str_port, int async)
{
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    I_InitTcpDriver(); // this is done only if not already done

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return MS_SOCKET_ERROR;

    if (MS_GetIP(ip_addr)==MS_GETHOSTBYNAME_ERROR)
        return MS_GETHOSTBYNAME_ERROR;
    addr.sin_port = htons(atoi(str_port));

    if (async) // do asynchronous connection
    {
        int res = 1;

        ioctl(socket_fd, FIONBIO, &res);
        res = connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr));
        if (res < 0)
        {
#ifdef WIN32  // humm, on win32 it doesn't work with EINPROGRESS (stupid windows)
            if (WSAGetLastError() != WSAEWOULDBLOCK)
#else
            if (errno != EINPROGRESS)
#endif
            {
                con_state = MSCS_FAILED;
                CloseConnection();
                return MS_CONNECT_ERROR;
            }
        }
        con_state = MSCS_WAITING;
        FD_ZERO(&wset);
        FD_SET(socket_fd, &wset);  
        select_timeout.tv_sec = 0, select_timeout.tv_usec = 0;
    }
    else
    {
        if (connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
            return MS_CONNECT_ERROR;
    }

    return 0;
}


/*
 * MS_Write():
 */
static int MS_Write(msg_t *msg)
{
    int len;

    if (msg->length < 0)
        msg->length = strlen(msg->buffer);
    len = msg->length+HEADER_SIZE;

    //msg->id = htonl(msg->id);
    msg->type = htonl(msg->type);
    msg->length = htonl(msg->length);

    if (send(socket_fd, (char*)msg, len, 0) != len)
        return MS_WRITE_ERROR;

    return 0;
}


/*
 * MS_Read():
 */
static int MS_Read(msg_t *msg)
{
    if (recv(socket_fd, (char*)msg, HEADER_SIZE, 0) != HEADER_SIZE)
        return MS_READ_ERROR;

    //msg->id = ntohl(msg->id);
    msg->type = ntohl(msg->type);
    msg->length = ntohl(msg->length);

    if (!msg->length) //Hurdler: fix a bug in Windows 2000
        return 0;

    if (recv(socket_fd, (char*)msg->buffer, msg->length, 0) != msg->length)
        return MS_READ_ERROR;

    return 0;
}
