// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: ipcs.h,v 1.7 2001/03/31 10:18:45 ydario Exp $
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
// $Log: ipcs.h,v $
// Revision 1.7  2001/03/31 10:18:45  ydario
// OS/2 compilation fixes
//
// Revision 1.6  2000/10/16 21:10:45  hurdler
// Improving stats
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
// Revision 1.1.1.1  2000/08/18 10:32:11  hurdler
// Initial import of the Master Server code into CVS
//
//
// DESCRIPTION:
//
//
//-----------------------------------------------------------------------------

#ifndef _IPCS_H_
#define _IPCS_H_

#if defined (_WIN32) || defined (_WIN64) || defined ( __OS2__)
#include <io.h>
#include <sys/types.h>
typedef int socklen_t;
#if defined (__OS2__)
#include <netinet/in.h>
#endif
#endif
#if defined (_WIN32) || defined (_WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>   // inet_addr(),...
#endif

#ifndef SOCKET
#define SOCKET u_int
#endif

// ================================ DEFINITIONS ===============================

#define PACKET_SIZE 1024
#define MAX_CLIENT    64

#if !defined (_WIN32) && !defined (_WIN64)
#define  NO_ERROR                      0
#define  SOCKET_ERROR               -201
#endif
#define  BIND_ERROR                 -202
#define  CONNECT_ERROR              -203
#define  LISTEN_ERROR               -204
#define  ACCEPT_ERROR               -205
#define  WRITE_ERROR                -210
#define  READ_ERROR                 -211
#define  CLOSE_ERROR                -212
#define  GETHOSTBYNAME_ERROR        -220
#define  SELECT_ERROR               -230
#define  TIMEOUT_ERROR              -231
#define  MALLOC_ERROR               -301

#define INVALID_MSG                   -1
#define ACCEPT_MSG                   100
#define ADD_SERVER_MSG               101
#define ADD_CLIENT_MSG               102
#define REMOVE_SERVER_MSG            103
#define GET_SERVER_MSG               200
#define SEND_SERVER_MSG              201
#define GET_LOGFILE_MSG              202
#define SEND_FILE_MSG                203
#define ERASE_LOGFILE_MSG            204
#define GET_SHORT_SERVER_MSG         205
#define SEND_SHORT_SERVER_MSG        206
#define UDP_RECV_MSG                 300
#define TIMEOUT_MSG                  301
#define HTTP_REQUEST_MSG       875770417    // "4321"
#define SEND_HTTP_REQUEST_MSG  875770418    // "4322"
#define TEXT_REQUEST_MSG       825373494    // "1236"
#define SEND_TEXT_REQUEST_MSG  825373495    // "1237"
#define RSS92_REQUEST_MSG      825373496    // "1238"
#define SEND_RSS92_REQUEST_MSG 825373497    // "1239"
#define RSS10_REQUEST_MSG      825373744    // "1240"
#define SEND_RSS10_REQUEST_MSG 825373745    // "1241"
#define ADD_PSERVER_MSG        0xabacab81    // this number just need to be different than the others
#define REMOVE_PSERVER_MSG     0xabacab82

#define HEADER_SIZE ((long)sizeof(long)*3)

#define HEADER_MSG_POS      0
#define IP_MSG_POS         16
#define PORT_MSG_POS       32
#define HOSTNAME_MSG_POS   40

// I want that structure 8 bytes aligned (current size is 80)
typedef struct
{
    char    header[16];     // information such as password
    char    ip[16];
    char    port[8];
    char    name[32];       
    char    version[8];     // format is: x.yy.z (like 1.30.2 or 1.31)
} msg_server_t;

typedef struct {
    long    id;
    long    type;
    long    length;
    char    buffer[PACKET_SIZE];
} msg_t;


class CSocket
{
protected:
    sockaddr_in     addr;
    msg_t           msg;
    fd_set          rset;
    void            setPort(unsigned short);
    unsigned short  getPort();
public:
    int             getIP(char *);
                    CSocket();
                    ~CSocket();
};

class CServerSocket : public CSocket
{
private:
    sockaddr_in udp_addr;
    SOCKET      udp_fd;
    SOCKET      accept_fd;
    int         num_clients;
    SOCKET      client_fd[MAX_CLIENT];
    sockaddr_in client_addr[MAX_CLIENT];
    
public:
    int         deleteClient(int id);
    int         listen(char *str_port);
    int         accept();
    int         deleteClient(int *);
    int         read(msg_t *msg);
    const char  *getUdpIP();
    int         write(msg_t *msg);
    const char  *getClientIP(int id);
                CServerSocket();
                ~CServerSocket();
};

class CClientSocket : public CSocket
{
private:
    SOCKET  socket_fd;
public:
    int     connect(char *ip_addr, char *str_port);
    int     read(msg_t *msg);
    int     write(msg_t *msg);
            CClientSocket();
            ~CClientSocket();
};


// ================================== PROTOS ==================================


// ================================== EXTERNS =================================


#endif
