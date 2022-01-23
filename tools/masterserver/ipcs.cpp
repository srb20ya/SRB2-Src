// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: ipcs.cpp,v 1.8 2001/04/16 22:57:22 ydario Exp $
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
// $Log: ipcs.cpp,v $
// Revision 1.8  2001/04/16 22:57:22  ydario
// fixed member init in client socket class
//
// Revision 1.7  2001/03/31 10:18:26  ydario
// OS/2 compilation fixes
//
// Revision 1.6  2001/02/16 01:14:13  hurdler
// Ouch, fix a stupid bug
//
// Revision 1.5  2001/02/16 00:41:42  hurdler
// Better solution
//
// Revision 1.4  2001/02/16 00:33:28  hurdler
// Better solution
//
// Revision 1.3  2000/09/01 23:30:32  hurdler
// minor changes (stats, log & timeout)
//
// Revision 1.2  2000/08/21 11:08:46  hurdler
// Improve the stats
//
// Revision 1.1.1.1  2000/08/18 10:32:11  hurdler
// Initial import of the Master Server code into CVS
//
//
// DESCRIPTION:
//
//
//-----------------------------------------------------------------------------

#include <unistd.h>      //

#if defined (_WIN32) || defined (_WIN64)
#include <winsock.h>
#define close closesocket
#else
#include <sys/types.h>   // socket(),...
#include <sys/socket.h>  // socket(),...
#endif
#include <stdlib.h>      // atoi(),...
#include <signal.h>      // signal(),...
#if !defined (_WIN32) && !defined (_WIN64)
#include <netdb.h>       // gethostbyname(),...
#endif
#if defined (_WIN32) || defined (_WIN64)
#include <time.h>
#else
#include <sys/time.h>    // timeval,... (TIMEOUT)
#endif
#include <string.h>      // memset(),...
#include "ipcs.h"
#include "common.h"


typedef void Sigfunc(int);

#if !defined (_WIN32) && !defined (_WIN64)
static void sigHandler(int signr);
static Sigfunc *mySignal(int signo, Sigfunc *func);
#endif


//=============================================================================

#if defined (_WIN32) || defined (_WIN64) || defined (__OS2__)
// it seems windows doesn't define that... maybe some other OS? OS/2
static int inet_aton(char *hostname, struct in_addr *addr)
{
    return ( (addr->s_addr=inet_addr(hostname)) != INADDR_NONE );
}   
#endif

/*
** CSocket()
*/
CSocket::CSocket()
{
    dbgPrintf(DEFCOL, "Initializing socket... ");
#if defined (_WIN32) || defined (_WIN64)
//#warning SIGPIPE needed
    WSADATA winsockdata;
    if( WSAStartup(MAKEWORD(1,1),&winsockdata) )
        conPrintf(RED,"No Tcp/Ip driver detected");
#else
    signal(SIGPIPE, sigHandler);
#endif
    FD_ZERO(&rset);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    dbgPrintf(DEFCOL, "DONE.\n");
}


/*
** ~CSocket()
*/
CSocket::~CSocket()
{
    dbgPrintf(DEFCOL, "Freeing socket... ");
#if defined(_WIN32) || defined(_WIN64)
	WSACleanup();
#endif
    dbgPrintf(DEFCOL, "DONE.\n");
}


/*
** getIP()
*/
int CSocket::getIP(char *ip_addr)
{
    struct hostent *host_ent;

    dbgPrintf(DEFCOL, "get IP for %s... ", ip_addr);
    if (!inet_aton(ip_addr, &addr.sin_addr)) {
        host_ent = gethostbyname(ip_addr);
        if (host_ent==NULL)
            return GETHOSTBYNAME_ERROR;
        memcpy(&addr.sin_addr, host_ent->h_addr_list[0], sizeof(struct in_addr));
    }
    dbgPrintf(DEFCOL, "got %s. ", inet_ntoa(addr.sin_addr));
    dbgPrintf(DEFCOL, "DONE.\n");
    return 0;
}


/*
** CClientSocket()
*/
CClientSocket::CClientSocket()
{
    dbgPrintf(DEFCOL, "Initializing client socket... ");
    socket_fd = (SOCKET)-1;
    dbgPrintf(DEFCOL, "DONE.\n");
}


/*
** ~CClientSocket()
*/
CClientSocket::~CClientSocket()
{
    dbgPrintf(DEFCOL, "Freeing client socket... ");
    if(socket_fd != (SOCKET)-1) close(socket_fd);
    dbgPrintf(DEFCOL, "DONE.\n");
}


/*
** connect()
*/
int CClientSocket::connect(char *ip_addr, char *str_port)
{
    dbgPrintf(DEFCOL, "Connect to %s %s...\n", ip_addr, str_port);

    // put that in the constructor?
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == (SOCKET)-1)
        return SOCKET_ERROR;

    if (getIP(ip_addr)==GETHOSTBYNAME_ERROR)
        return GETHOSTBYNAME_ERROR;
    addr.sin_port = htons(atoi(str_port));

    if (::connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        return CONNECT_ERROR;

    FD_SET(socket_fd, &rset);

    dbgPrintf(DEFCOL, "DONE.\n");

    return 0;
}


/*
 * write():
 */
int CClientSocket::write(msg_t *msg)
{
    int len;

    if (msg->length < 0)
        msg->length = strlen(msg->buffer);
    len = msg->length+HEADER_SIZE;

    msg->id = htonl(msg->id);
    msg->type = htonl(msg->type);
    msg->length = htonl(msg->length);
        
    dbgPrintf(DEFCOL, "Write a message %d (%s)... ", len, msg->buffer);
    
    if (send(socket_fd, (char*)msg, len, 0) != len)
        return WRITE_ERROR;

    dbgPrintf(DEFCOL, "DONE.\n");

    return 0;
}


/*
 * read():
 */
int CClientSocket::read(msg_t *msg)
{
    struct timeval  timeout;
    fd_set          tset;

    dbgPrintf(DEFCOL, "Waiting a message... ");

    timeout.tv_sec = 60, timeout.tv_usec = 0;
    memcpy(&tset, &rset, sizeof(tset));
    if ((select(255, &tset, NULL, NULL, &timeout)) <= 0)
        return TIMEOUT_ERROR;

    dbgPrintf(DEFCOL, "got a message.\n");

    if (!FD_ISSET(socket_fd, &tset))
        return READ_ERROR;

    dbgPrintf(DEFCOL, "Reading a message.\n");

    if (recv(socket_fd, (char*)msg, HEADER_SIZE, 0) != HEADER_SIZE)
        return READ_ERROR;
        
    msg->id = ntohl(msg->id);
    msg->type = ntohl(msg->type);
    msg->length = ntohl(msg->length);

    if(!msg->length) // fix a bug in Windows 2000
        return 0;

    if (recv(socket_fd, msg->buffer, msg->length, 0) != msg->length)
        return READ_ERROR;

    dbgPrintf(DEFCOL, "DONE.\n");

    return 0;
}


/*
** CServerSocket()
*/
CServerSocket::CServerSocket()
{
	int id;
    dbgPrintf(DEFCOL, "Initializing server socket... ");
    num_clients = 0;
    accept_fd = udp_fd = (SOCKET)-1;
    for(id=0;(id < MAX_CLIENT);id++)
        client_fd[id] = (SOCKET)-1;
    udp_addr.sin_family = AF_INET;
    dbgPrintf(DEFCOL, "DONE.\n");
}


/*
** ~CServerSocket()
*/
CServerSocket::~CServerSocket()
{
    int id;
    dbgPrintf(DEFCOL, "Freeing server socket... ");
    if(udp_fd != (SOCKET)-1) close(udp_fd);
    if(accept_fd != (SOCKET)-1) close(accept_fd);
    for(id=0;(id < num_clients) || (id < MAX_CLIENT);id++)
        if(client_fd[id] != (SOCKET)-1) close(client_fd[id]);
    dbgPrintf(DEFCOL, "DONE.\n");
}


/*
** listen()
*/
int CServerSocket::listen(char *str_port)
{
    dbgPrintf(DEFCOL, "Listen on %s... ", str_port);

    // Init TCP socket
    if ((accept_fd = socket(AF_INET, SOCK_STREAM, 0)) == (SOCKET)-1)
        return SOCKET_ERROR;

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(str_port));

    if (bind(accept_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        return BIND_ERROR;

    if (::listen(accept_fd, 5) < 0)
        return LISTEN_ERROR;

    FD_SET(accept_fd, &rset);
    
    // Init UDP socket
    if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) == (SOCKET)-1)
        return SOCKET_ERROR;

    udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    udp_addr.sin_port = htons(atoi(str_port)+1);

    if (bind(udp_fd, (struct sockaddr *) &udp_addr, sizeof(udp_addr)) < 0)
        return BIND_ERROR;

    FD_SET(udp_fd, &rset);
        
    dbgPrintf(DEFCOL, "DONE.\n");

    return 0;
}


/*
 * deleteClient():
 */
int CServerSocket::deleteClient(int id)
{
    dbgPrintf(DEFCOL, "Deleting (client %d) of %d... ", id+1, num_clients);

    FD_CLR(client_fd[id], &rset);
    close(client_fd[id]);
    num_clients--;
    client_fd[id] = client_fd[num_clients];
    client_fd[num_clients] = (SOCKET)-1;

    dbgPrintf(DEFCOL, "DONE.\n");

    return 0;
}


/*
 * getUdpIP()
 */
const char* CServerSocket::getUdpIP()
{
    sockaddr_in from_addr;
    char        buffer[PACKET_SIZE];
    socklen_t   lg = sizeof(sockaddr_in);
  
    recvfrom(udp_fd,  buffer,  PACKET_SIZE, 0, (struct sockaddr *)&from_addr, &lg);
    return inet_ntoa(from_addr.sin_addr);
}


/*
 * read(): wait message on a socket, if it's an accept, call the accept method,
 *         otherwise read the message and update msg->id to the right value
 *         (client cannot put valid entry in msg->id)
 */
int CServerSocket::read(msg_t *msg)
{
    struct timeval  timeout;
    fd_set          tset;
    int             id;

    timeout.tv_sec = 20, timeout.tv_usec = 0;
    memcpy(&tset, &rset, sizeof(tset));
    if ((select(255, &tset, NULL, NULL, &timeout)) <= 0)
    {
        msg->type = TIMEOUT_MSG;
        return TIMEOUT_ERROR;
    }

    dbgPrintf(DEFCOL, "Got a message. (%d)\n", num_clients);

    if (FD_ISSET(udp_fd, &tset))
    {
        msg->type = UDP_RECV_MSG;
        return 0;
    }

    if (FD_ISSET(accept_fd, &tset))
    {
        msg->type = ACCEPT_MSG;
        return this->accept();
    }

    id = 0;
    while ( !FD_ISSET(client_fd[id], &tset) && (id < MAX_CLIENT) )
        id++;

    // paranoia
    if ( (id >= num_clients) || (id >= MAX_CLIENT) )
    {
        dbgPrintf(DEFCOL, "This should never appens\n");
        return READ_ERROR;
    }

    dbgPrintf(DEFCOL, "Reading a message from (socket %d) for (client %d)... ", client_fd[id], id+1);

    if (recv(client_fd[id], (char*)msg, HEADER_SIZE, 0) != HEADER_SIZE)
    {
        deleteClient(id); // the connection is not good with the client, kick him
        return READ_ERROR;
    }

    //msg->id = ntohl(msg->id); // see comment above
    msg->type = ntohl(msg->type);
    msg->length = ntohl(msg->length);

    if ((msg->length > 0) && (msg->length < PACKET_SIZE))
    {
        timeout.tv_sec = 0, timeout.tv_usec = 0; // the info should be already in the socket, so don't wait
        memcpy(&tset, &rset, sizeof(tset));
        if ((select(255, &tset, NULL, NULL, &timeout)) <= 0)
        {
			//deleteClient(id);
            return READ_ERROR;
		}

        if (!FD_ISSET(client_fd[id], &tset))
        {
            deleteClient(id);
            return READ_ERROR;
        }
        if (recv(client_fd[id], msg->buffer, msg->length, 0) != msg->length)
        {
            deleteClient(id);
            return READ_ERROR;
        }
    }

    msg->id = id;

    dbgPrintf(DEFCOL, "DONE.\n");

    return 0;
}


/*
 * write():
 */
int CServerSocket::write(msg_t *msg)
{
    struct timeval  timeout;
    fd_set          tset;
    int             len;

    dbgPrintf(DEFCOL, "Write a message... ");

    FD_ZERO(&tset);
    FD_SET(client_fd[msg->id], &tset);  // be sure the msg->id is still valid (we cannot do a delete between a read and a write)
                                        // should we use a readpending set in read method, so we know that we are still waiting
                                        // for writing? answer: for now, it's the responability of server code
    timeout.tv_sec = 0, timeout.tv_usec = 0;
    if ((select(255, NULL, &tset, NULL, &timeout)) <= 0)
    {
        deleteClient(msg->id); // this shouldn't happen if the net connection is good
        return TIMEOUT_ERROR;
    }

    if (msg->length < 0)
        msg->length = strlen(msg->buffer);
    len = msg->length+HEADER_SIZE;

    //msg->id = htonl(msg->id);  // same comment as for read
    msg->type = htonl(msg->type);
    msg->length = htonl(msg->length);

    if (send(client_fd[msg->id], (char*)msg, len, 0) != len)
    {
        deleteClient(msg->id);
        return WRITE_ERROR;
    }

    dbgPrintf(DEFCOL, "DONE.\n");

    return 0;
}


/*
 * accept():
 */
int CServerSocket::accept()
{
    socklen_t   lg = sizeof(addr);

    dbgPrintf(DEFCOL, "Accepting on (socket %d)... ", accept_fd);

    if ( (client_fd[num_clients] = ::accept(accept_fd, (struct sockaddr *) &addr, &lg)) < 0)
    {
        client_fd[num_clients] = (SOCKET)-1; // we haven't accepted the client, so set it at default value
        return ACCEPT_ERROR;
    }
    
    if (num_clients < MAX_CLIENT-1)
    {
        extern FILE *logfile;
        
        logPrintf(logfile, "Accepting on socket %d for client %d (%s)\n", 
                    client_fd[num_clients], num_clients+1, inet_ntoa(addr.sin_addr));
        memcpy(&client_addr[num_clients], &addr, sizeof(addr));
        FD_SET(client_fd[num_clients], &rset);
        num_clients++;
    }
    else // we have reached the maximum number of clients
    {
        // close the connection (meaning "try again" for the client)
        close(client_fd[num_clients]);
        client_fd[num_clients] = (SOCKET)-1;
        return ACCEPT_ERROR;
    }

    dbgPrintf(DEFCOL, "DONE.\n");

    return 0;
}


/*
 * getClientIP():
 */
const char* CServerSocket::getClientIP(int id)
{
    dbgPrintf(DEFCOL, "getClientIP: %d\n", id);
    return inet_ntoa(client_addr[id].sin_addr);
}


/*
 * mySignal():
 */
#if !defined (_WIN32) && !defined (_WIN64)
static Sigfunc *mySignal(int signo, Sigfunc *func)
{
    struct sigaction    act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
#if defined (SA_RESTART)
    act.sa_flags = SA_RESTART;
#else
    act.sa_flags = 0;
#endif
    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;
    return oact.sa_handler;
}

/*
 * sigHandler():
 */
static void sigHandler(int signr)
{
    mySignal(signr, sigHandler);
    switch (signr)
    {
        case SIGPIPE:
            dbgPrintf(DEFCOL, "Catch SIG_PIPE\n");
            /// TODO be sure the socket is closed if the client didn't quit properly
            break;
    }
}
#endif
