// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
/// \file
/// \brief Commands used for communicate with the master server

#ifdef __GNUC__
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#endif

#ifndef NONET
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN64)
#define RPC_NO_WINDOWS_H
#include <winsock.h>     // socket(),...
#else
#ifdef __OS2__
#include <sys/types.h>
#endif // __OS2__
#include <sys/socket.h> // socket(),...
#include <sys/time.h> // timeval,... (TIMEOUT)
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_addr(),...
#include <netdb.h> // gethostbyname(),...
#include <sys/ioctl.h>
#include <errno.h>
#endif // _WIN32/_WIN64/_WIN32_WCE

#ifdef __OS2__
#include <errno.h>
#endif // __OS2__
#endif // !NONET

#include "doomstat.h"
#include "doomdef.h"
#include "command.h"
#include "i_net.h"
#include "console.h"
#include "mserv.h"
#include "d_net.h"
#include "i_tcp.h"
#include "i_system.h"

#ifdef _WIN32_WCE
#include "sdl/SRB2CE/cehelp.h"
#endif

// ================================ DEFINITIONS ===============================

#define PACKET_SIZE 1024

#define  MS_NO_ERROR               0
#define  MS_SOCKET_ERROR        -201
#define  MS_CONNECT_ERROR       -203
#define  MS_WRITE_ERROR         -210
#define  MS_READ_ERROR          -211
#define  MS_CLOSE_ERROR         -212
#define  MS_GETHOSTBYNAME_ERROR -220
#define  MS_GETHOSTNAME_ERROR   -221
#define  MS_TIMEOUT_ERROR       -231

// see master server code for the values
#define ADD_SERVER_MSG           101
#define REMOVE_SERVER_MSG        103
#define GET_SERVER_MSG           200
#define GET_SHORT_SERVER_MSG     205

#define HEADER_SIZE ((long)sizeof(long)*3)

#define HEADER_MSG_POS    0
#define IP_MSG_POS       16
#define PORT_MSG_POS     32
#define HOSTNAME_MSG_POS 40

/** A message to be exchanged with the master server.
  */
typedef struct
{
	long id;                  ///< Unused?
	long type;                ///< Type of message.
	long length;              ///< Length of the message.
	char buffer[PACKET_SIZE]; ///< Actual contents of the message.
} msg_t;

typedef struct Copy_CVarMS_t
{
	char ip[64];
	char port[8];
	char name[64];
} Copy_CVarMS_s;
static Copy_CVarMS_s registered_server;

// win32 or djgpp
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(__DJGPP__) || defined(_WIN64)
#define ioctl ioctlsocket
#define close closesocket
#ifdef WATTCP
#define strerror strerror_s
#endif
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN64)
#undef errno
#define errno h_errno // some very strange things happen when not using h_error
#endif
#endif

#if !defined(__APPLE_CC__) && (defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN64) || defined(__OS2__) || defined(SOLARIS)) && !defined(NONET)
// it seems windows doesn't define that... maybe some other OS? OS/2
static int inet_aton(const char* hostname, struct in_addr* addr)
{
	return (addr->s_addr = inet_addr(hostname)) != INADDR_NONE;
}
#endif

static void Command_Listserv_f(void);

#define DEF_PORT "28910"
consvar_t cv_internetserver = {"internetserver", "No", 0, CV_YesNo, NULL, 0, NULL, NULL, 0, 0, NULL};
/// \todo when we change the port or ip, unregister to the old master server, register to new one
consvar_t cv_masterserver = {"masterserver", "srb2.servegame.org:"DEF_PORT, CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_servername = {"servername", "SRB2 server", CV_SAVE, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};

static enum { MSCS_NONE, MSCS_WAITING, MSCS_REGISTERED, MSCS_FAILED } con_state = MSCS_NONE;

static int msnode = -1;
#define current_port sock_port

#if (defined(_WIN32) || defined(_WIN32_WCE) || defined(_WIN32)) && !defined(NONET)
typedef SOCKET SOCKET_TYPE;
#define BADSOCKET INVALID_SOCKET
#define ERRSOCKET (SOCKET_ERROR)
#else
typedef unsigned int SOCKET_TYPE;
#define BADSOCKET (SOCKET_TYPE)(~0)
#define ERRSOCKET (-1)
#endif

#ifndef NONET
static SOCKET_TYPE socket_fd = BADSOCKET; // WINSOCK socket
static struct sockaddr_in addr;
static struct timeval select_timeout;
static fd_set wset;
#endif

/** Adds variables and commands relating to the master server.
  *
  * \sa cv_internetserver, cv_masterserver, cv_servername,
  *     Command_Listserv_f
  */
void AddMServCommands(void)
{
	CV_RegisterVar(&cv_internetserver);
	CV_RegisterVar(&cv_masterserver);
	CV_RegisterVar(&cv_servername);
	COM_AddCommand("listserv", Command_Listserv_f);
}

/** Closes the connection to the master server.
  *
  * \todo Fix for Windows?
  */
static void CloseConnection(void)
{
#ifndef NONET
	if(socket_fd != (SOCKET_TYPE)ERRSOCKET && socket_fd != BADSOCKET)
		close(socket_fd);
	socket_fd = BADSOCKET;
#endif
}

//
// MS_Write():
//
static int MS_Write(msg_t* msg)
{
#ifdef NONET
	msg = NULL;
	return MS_WRITE_ERROR;
#else
	int len;

	if(msg->length < 0)
		msg->length = (long)strlen(msg->buffer);
	len = msg->length + HEADER_SIZE;

	msg->type = htonl(msg->type);
	msg->length = htonl(msg->length);

	if(send(socket_fd, (char*)msg, len, 0) != len)
		return MS_WRITE_ERROR;
	return 0;
#endif
}

//
// MS_Read():
//
static int MS_Read(msg_t* msg)
{
#ifdef NONET
	msg = NULL;
	return MS_READ_ERROR;
#else
	if(recv(socket_fd, (char*)msg, HEADER_SIZE, 0) != HEADER_SIZE)
		return MS_READ_ERROR;

	msg->type = ntohl(msg->type);
	msg->length = ntohl(msg->length);

	if(!msg->length) // fix a bug in Windows 2000
		return 0;

	if(recv(socket_fd, (char*)msg->buffer, msg->length, 0) != msg->length)
		return MS_READ_ERROR;
	return 0;
#endif
}

/** Gets a list of game servers from the master server.
  */
static inline int GetServersList(void)
{
	msg_t msg;
	int count = 0;

	msg.type = GET_SERVER_MSG;
	msg.length = 0;
	if(MS_Write(&msg) < 0)
		return MS_WRITE_ERROR;

	while(MS_Read(&msg) >= 0)
	{
		if(!msg.length)
		{
			if(!count)
				CONS_Printf("No server currently running.\n");
			return MS_NO_ERROR;
		}
		count++;
		CONS_Printf(msg.buffer);
	}

	return MS_READ_ERROR;
}

//
// MS_GetIP()
//
#ifndef NONET
static inline int MS_GetIP(const char* hostname)
{
	struct hostent* host_ent;
	if(!inet_aton(hostname, &addr.sin_addr))
	{
		/// \todo only when we are connected to the Internet, or use a non blocking call
		host_ent = gethostbyname(hostname);
		if(!host_ent)
			return MS_GETHOSTBYNAME_ERROR;
		memcpy(&addr.sin_addr, host_ent->h_addr_list[0], sizeof(struct in_addr));
	}
	return 0;
}
#endif

//
// MS_Connect()
//
static int MS_Connect(const char* ip_addr, const char* str_port, int async)
{
#ifdef NONET
	str_port = ip_addr = NULL;
	async = MS_CONNECT_ERROR;
	return async;
#else
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	I_InitTcpDriver(); // this is done only if not already done
	if(!init_tcp_driver)
		return MS_SOCKET_ERROR;

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd == BADSOCKET || socket_fd == (SOCKET_TYPE)ERRSOCKET)
		return MS_SOCKET_ERROR;

	if(MS_GetIP(ip_addr) == MS_GETHOSTBYNAME_ERROR)
		return MS_GETHOSTBYNAME_ERROR;
	addr.sin_port = htons((unsigned short)atoi(str_port));

	if(async) // do asynchronous connection
	{
#ifdef WATTCP
		char res = 1;
#else
		unsigned long res = 1;
#endif

		ioctl(socket_fd, FIONBIO, &res);

		if(connect(socket_fd, (struct sockaddr*)&addr, sizeof(addr)) == ERRSOCKET)
		{
#if defined(_WIN32) || defined(_WIN64)  // humm, on win32/win64 it doesn't work with EINPROGRESS (stupid windows)
			if(WSAGetLastError() != WSAEWOULDBLOCK)
#else
			if(errno != EINPROGRESS)
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
	else if(connect(socket_fd, (struct sockaddr*) &addr, sizeof(addr)) == ERRSOCKET)
		return MS_CONNECT_ERROR;
	return 0;
#endif
}

#define NUM_LIST_SERVER 10
const msg_server_t* GetShortServersList(void)
{
	static msg_server_t server_list[NUM_LIST_SERVER+1]; // +1 for easy test
	msg_t msg;
	int i;

	// we must be connected to the master server before writing to it
	if(MS_Connect(GetMasterServerIP(), GetMasterServerPort(), 0))
	{
		CONS_Printf("cannot connect to the master server\n");
		return NULL;
	}

	msg.type = GET_SHORT_SERVER_MSG;
	msg.length = 0;
	if(MS_Write(&msg) < 0)
		return NULL;

	for(i = 0; i < NUM_LIST_SERVER && MS_Read(&msg) >= 0; i++)
	{
		if(!msg.length)
		{
			server_list[i].header[0] = 0;
			CloseConnection();
			return server_list;
		}
		memcpy(&server_list[i], msg.buffer, sizeof(msg_server_t));
		server_list[i].header[0] = 1;
	}
	CloseConnection();
	if(i == NUM_LIST_SERVER)
	{
		server_list[i].header[0] = 0;
		return server_list;
	}
	else
		return NULL;
}

/** Gets a list of game servers. Called from console.
  */
static void Command_Listserv_f(void)
{
	if(con_state == MSCS_WAITING)
	{
		CONS_Printf("Not yet registered to the master server.\n");
		return;
	}

	CONS_Printf("Retrieving server list...\n");

	if(MS_Connect(GetMasterServerIP(), GetMasterServerPort(), 0))
	{
		CONS_Printf("cannot connect to the master server\n");
		return;
	}

	if(GetServersList())
		CONS_Printf("cannot get server list\n");

	CloseConnection();
}

static const char* int2str(int n)
{
	int i;
	static char res[16];

	res[15] = '\0';
	res[14] = (char)((char)(n%10)+'0');
	for(i = 13; (n /= 10); i--)
		res[i] = (char)((char)(n%10)+'0');

	return &res[i+1];
}

#ifndef NONET
static int ConnectionFailed(void)
{
	con_state = MSCS_FAILED;
	CONS_Printf("Connection to master server failed\n");
	CloseConnection();
	return MS_CONNECT_ERROR;
}
#endif

/** Tries to register the local game server on the master server.
  */
static int AddToMasterServer(void)
{
#ifndef NONET
	static int retry = 0;
	int i, res;
#if defined(__APPLE_CC__) || defined(WATTCP) || defined(_WIN32) || defined(_WIN64)
	int j;
#else
	size_t j;
#endif
	msg_t msg;
	msg_server_t* info = (msg_server_t*)msg.buffer;
	fd_set tset;

	memcpy(&tset, &wset, sizeof(tset));
	res = select((int)(socket_fd + 1), NULL, &tset, NULL, &select_timeout);
	if(res != ERRSOCKET && !res)
	{
		if(retry++ > 30) // an about 30 second timeout
		{
			retry = 0;
			CONS_Printf("Timeout on masterserver\n");
			return ConnectionFailed();
		}
		return MS_CONNECT_ERROR;
	}
	retry = 0;
	if(res == ERRSOCKET)
	{
		CONS_Printf("Error on select: %s\n", strerror(errno));
		return ConnectionFailed();
	}

	// so, the socket is writable, but what does that mean, that the connection is
	// ok, or bad... let see that!
	j = sizeof(i);
	getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (char*)&i, &j);
	if(i) // it was bad
	{
		CONS_Printf("getsockopt: %s\n", strerror(errno));
		return ConnectionFailed();
	}

	strcpy(info->header, "");
	strcpy(info->ip, "");
	strcpy(info->port, int2str(current_port));
	strcpy(info->name, cv_servername.string);
	sprintf(info->version, "%d.%.2d.%d", VERSION/100, VERSION%100, SUBVERSION);
	strcpy(registered_server.name, cv_servername.string);

	msg.type = ADD_SERVER_MSG;
	msg.length = sizeof(msg_server_t);
	if(MS_Write(&msg) < 0)
		return ConnectionFailed();

	CONS_Printf("The server has been registered on the master server...\n");
	con_state = MSCS_REGISTERED;
	CloseConnection();
#endif
	return MS_NO_ERROR;
}

static inline int RemoveFromMasterSever(void)
{
	msg_t msg;
	msg_server_t* info = (msg_server_t*)msg.buffer;

	strcpy(info->header, "");
	strcpy(info->ip, "");
	strcpy(info->port, int2str(current_port));
	strcpy(info->name, registered_server.name);
	sprintf(info->version, "%d.%.2d.%d", VERSION/100, VERSION%100, SUBVERSION);

	msg.type = REMOVE_SERVER_MSG;
	msg.length = sizeof(msg_server_t);
	if(MS_Write(&msg) < 0)
		return MS_WRITE_ERROR;

	return MS_NO_ERROR;
}

const char* GetMasterServerPort(void)
{
	const char* t = cv_masterserver.string;

	while((*t != ':') && (*t != '\0'))
		t++;

	if(*t)
		return ++t;
	else
		return DEF_PORT;
}

/** Gets the IP address of the master server. Actually, it seems to just
  * return the hostname, instead; the lookup is done elsewhere.
  *
  * \return Hostname of the master server, without port number on the end.
  * \todo Rename function?
  */
const char* GetMasterServerIP(void)
{
	static char str_ip[64];
	char* t = str_ip;

	if(strstr(cv_masterserver.string, "srb2.ssntails.org:28910"))
	{
		// replace it with the current default one
		CV_Set(&cv_masterserver, cv_masterserver.defaultvalue);
	}

	strcpy(t, cv_masterserver.string);

	while((*t != ':') && (*t != '\0'))
		t++;
	*t = '\0';

	return str_ip;
}

static inline void openUdpSocket(void)
{
#ifndef NONET
	if(I_NetMakeNode)
	{
		char hostname[24];

		sprintf(hostname, "%s:%d", inet_ntoa(addr.sin_addr), atoi(GetMasterServerPort())+1);
		msnode = I_NetMakeNode(hostname);
	}
	else
#endif
		msnode = -1;
}

void RegisterServer(int s, int port)
{
	s = port = 0;
	CONS_Printf("Registering this server to the master server...\n");

	strcpy(registered_server.ip, GetMasterServerIP());
	strcpy(registered_server.port, GetMasterServerPort());

	if(MS_Connect(registered_server.ip, registered_server.port, 1))
	{
		CONS_Printf("cannot connect to the master server\n");
		return;
	}
	openUdpSocket();

	// keep the TCP connection open until AddToMasterServer() is completed;
}

void SendPingToMasterServer(void)
{
	static tic_t next_time = 0;
	tic_t cur_time;

	cur_time = I_GetTime();
	if(cur_time > next_time) // ping every 2 second if possible
	{
		next_time = cur_time+2*TICRATE;

		if(con_state == MSCS_WAITING)
			AddToMasterServer();

		if(con_state != MSCS_REGISTERED)
			return;

		// cur_time is just a dummy data to send
		*((tic_t*)netbuffer) = cur_time;
		doomcom->datalength = sizeof(cur_time);
		doomcom->remotenode = (short)msnode;
		I_NetSend();
	}
}

void UnregisterServer(void)
{
	if(con_state != MSCS_REGISTERED)
	{
		con_state = MSCS_NONE;
		CloseConnection();
		return;
	}

	con_state = MSCS_NONE;

	CONS_Printf("Unregistering this server to the master server...\n");

	if(MS_Connect(registered_server.ip, registered_server.port, 0))
	{
		CONS_Printf("cannot connect to the master server\n");
		return;
	}

	if(RemoveFromMasterSever() < 0)
		CONS_Printf("cannot remove this server from the master server\n");

	CloseConnection();
	I_NetFreeNodenum(msnode);
}
