// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_net.c,v 1.2 2000/09/10 10:56:00 metzgermeister Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: i_net.c,v $
// Revision 1.2  2000/09/10 10:56:00  metzgermeister
// clean up & made it work again
//
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//-----------------------------------------------------------------------------
/// \file
/// \brief SDL network interface

#include "../doomdef.h"

#include "../i_system.h"
#include "../d_event.h"
#include "../d_net.h"
#include "../m_argv.h"

#include "../doomstat.h"

#include "../i_net.h"

#include "../z_zone.h"

#include "../i_tcp.h"

#ifdef HAVE_SDLNET

#include <SDL/SDL_net.h>

#define MAXBANS 20

static IPaddress clientaddress[MAXNETNODES+1];
static IPaddress banned[MAXBANS];

static UDPpacket mypacket;
static UDPsocket mysocket = NULL;
static SDLNet_SocketSet myset = NULL;

static size_t numbans = 0;
static boolean NET_bannednode[MAXNETNODES+1]; /// \note do we really need the +1?
static boolean init_SDLNet_driver = false;

static const char* NET_AddrToStr(IPaddress* sk)
{
	static char s[50];
	sprintf(s,"%d.%d.%d.%d:%d",
			((Uint8*)(&(sk->host)))[0],
			((Uint8*)(&(sk->host)))[1],
			((Uint8*)(&(sk->host)))[2],
			((Uint8*)(&(sk->host)))[3],
			sk->port);
	return s;
}

static boolean NET_cmpaddr(IPaddress* a, IPaddress* b)
{
	return (a->host == b->host && a->port == b->port);
}

static boolean NET_CanGet(void)
{
	return myset?(SDLNet_CheckSockets(myset,0)  == 1):false;
}

static void NET_Get(void)
{
	int mystatus;
	int newnode;
	mypacket.len = MAXPACKETLENGTH;
	if(!NET_CanGet())
	{
		doomcom->remotenode = -1; // no packet
		return;
	}
	mystatus = SDLNet_UDP_Recv(mysocket,&mypacket);
	if(mystatus != -1)
	{
		if(mypacket.channel != -1)
		{
			doomcom->remotenode = mypacket.channel+1; // good packet from a game player
			doomcom->datalength = mypacket.len;
			return;
		}
		newnode = SDLNet_UDP_Bind(mysocket,-1,&mypacket.address);
		if(newnode != -1)
		{
			size_t i;
			newnode++;
			memcpy(&clientaddress[newnode], &mypacket.address, sizeof(IPaddress));
			DEBFILE(va("New node detected: node:%d address:%s\n", newnode,
					NET_AddrToStr(&clientaddress[newnode])));
			doomcom->remotenode = newnode; // good packet from a game player
			doomcom->datalength = mypacket.len;
			for(i = 0; i < numbans; i++)
				if(NET_cmpaddr(&mypacket.address, &banned[i]))
				{
					DEBFILE("This dude has been banned\n");
					NET_bannednode[newnode] = true;
					break;
				}
			if(i == numbans)
				NET_bannednode[newnode] = false;
			return;
		}
		else
			CONS_Printf("SDL_Net: %s",SDLNet_GetError());
	}
	else if(mystatus == -1)
	{
		CONS_Printf("SDL_Net: %s",SDLNet_GetError());
	}
	
	DEBFILE("New node detected: No more free slots\n");
	doomcom->remotenode = -1; // no packet
}

#if 0
static boolean NET_CanSend(void)
{
	return true;
}
#endif

static void NET_Send(void)
{
	if(!doomcom->remotenode)
		return;
	mypacket.len = doomcom->datalength;
	if(SDLNet_UDP_Send(mysocket,doomcom->remotenode-1,&mypacket) == 0)
	{
		CONS_Printf("SDL_Net: %s",SDLNet_GetError());
	}
}

static void NET_FreeNodenum(int numnode)
{
	// can't disconnect from self :)
	if(!numnode)
		return;

	DEBFILE(va("Free node %d (%s)\n", numnode, NET_AddrToStr(&clientaddress[numnode])));

	SDLNet_UDP_Unbind(mysocket,numnode-1);

	memset(&clientaddress[numnode], 0, sizeof(IPaddress));
}

static UDPsocket NET_Socket(void)
{
	UDPsocket temp = NULL;
	Uint16 portnum = 0;
	//Hurdler: I'd like to put a server and a client on the same computer
	//Logan: Me too
	//BP: in fact for client we can use any free port we want i have read
	//    in some doc that connect in udp can do it for us...
	//Alam: where?
	if(M_CheckParm("-clientport"))
	{
		if(!M_IsNextParm())
			I_Error("syntax: -clientport <portnum>");
		portnum = atoi(M_GetNextParm());
	}
	else
		portnum = sock_port;
	temp = SDLNet_UDP_Open(portnum);
	if(temp)
	{
		IPaddress tempip = {INADDR_BROADCAST,0};
		if(SDLNet_UDP_Bind(temp,BROADCASTADDR-1,&tempip) == -1)
		{
			CONS_Printf("SDL_Net: %s",SDLNet_GetError());
			SDLNet_UDP_Close(temp);
			return NULL;
		}
		else
		{
			clientaddress[BROADCASTADDR].port = sock_port;
			clientaddress[BROADCASTADDR].host = INADDR_BROADCAST;
		}
	}
	else
	{
		CONS_Printf("SDL_Net: %s",SDLNet_GetError());
		return NULL;
	}

	doomcom->extratics = 1; // internet is very high ping

	return temp;
}

static void I_ShutdownSDLNetDriver(void)
{
	if(myset) SDLNet_FreeSocketSet(myset);
	myset = NULL;	
	SDLNet_Quit();
	init_SDLNet_driver = false;
}

static void I_InitSDLNetDriver(void)
{
	if(init_SDLNet_driver)
		I_ShutdownSDLNetDriver();
	if(SDLNet_Init() == -1)
	{
		CONS_Printf("SDL_Net: %s",SDLNet_GetError());
		return; // No good!
	}
	D_SetDoomcom();
	mypacket.data = doomcom->data;
	init_SDLNet_driver = true;
}

static void NET_CloseSocket(void)
{
	if(mysocket)
		SDLNet_UDP_Close(mysocket);
	mysocket = NULL;
}

static signed char NET_NetMakeNode(const char *hostname)
{
	int newnode;
	char* portchar;
	unsigned short portnum = sock_port;
	IPaddress hostnameIP;

	// retrieve portnum from address!
	{
		char* localhostname = strdup(hostname);
		strtok(localhostname, ":");
		portchar = strtok(NULL, ":");
		if(portchar)
			portnum = atoi(portchar);
		free(localhostname);
	}
	
	if(SDLNet_ResolveHost(&hostnameIP,hostname,portnum) == -1)
	{
		CONS_Printf("SDL_Net: %s",SDLNet_GetError());
		return -1;
	}
	newnode = SDLNet_UDP_Bind(mysocket,-1,&hostnameIP);
	if(newnode == -1)
	{
		CONS_Printf("SDL_Net: %s",SDLNet_GetError());
		return newnode;
	}
	newnode++;
	memcpy(&clientaddress[newnode],&hostnameIP,sizeof(IPaddress));
	return (signed char)newnode;
}


static boolean NET_OpenSocket(void)
{
	memset(clientaddress, 0, sizeof(clientaddress));

	//CONS_Printf("SDL_Net Code starting up\n");

	I_NetSend = NET_Send;
	I_NetGet = NET_Get;
	I_NetCloseSocket = NET_CloseSocket;
	I_NetFreeNodenum = NET_FreeNodenum;
	I_NetMakeNode = NET_NetMakeNode;

	//I_NetCanSend = NET_CanSend;

	// build the socket but close it first
	NET_CloseSocket();
	mysocket = NET_Socket();

	if(mysocket)
	{
		// for select
		myset = SDLNet_AllocSocketSet(1);
		if(!myset)
		{
			CONS_Printf("SDL_Net: %s",SDLNet_GetError());
			return false;
		}
		if(SDLNet_UDP_AddSocket(myset,mysocket) == -1)
		{
			CONS_Printf("SDL_Net: %s",SDLNet_GetError());
			return false;
		}
		return true;
	}
	return false;
}

static boolean NET_Ban(int node)
{
	if(numbans == MAXBANS)
		return false;

	memcpy(&banned[numbans], &clientaddress[node], sizeof(IPaddress));
	numbans++;
	return true;
}

static void NET_ClearBans(void)
{
	numbans = 0;
}
#endif

//
// I_InitNetwork
// Only required for DOS, so this is more a dummy
//
boolean I_InitNetwork(void)
{
#ifdef HAVE_SDLNET
	char serverhostname[255];
	boolean ret = false;
	//if(!M_CheckParm ("-sdlnet"))
	//	return false;
	// initilize the driver
	I_InitSDLNetDriver();
	I_AddExitFunc(I_ShutdownSDLNetDriver);
	if(!init_SDLNet_driver)
		return false;

	if(M_CheckParm("-udpport"))
	{
		if(M_IsNextParm())
			sock_port = (unsigned short)atoi(M_GetNextParm());
		else
			sock_port = 0;
	}

	// parse network game options,
	if(M_CheckParm("-server") || dedicated)
	{
		server = true;

		// If a number of clients (i.e. nodes) is specified, the server will wait for the clients
		// to connect before starting.
		// If no number is specified here, the server starts with 1 client, and others can join
		// in-game.
		// Since Boris has implemented join in-game, there is no actual need for specifying a
		// particular number here.
		// FIXME: for dedicated server, numnodes needs to be set to 0 upon start
		if(M_IsNextParm())
			doomcom->numnodes = (short)atoi(M_GetNextParm());
		else if(dedicated)
			doomcom->numnodes = 0;
		else
			doomcom->numnodes = 1;

		if(doomcom->numnodes < 0)
			doomcom->numnodes = 0;
		if(doomcom->numnodes > MAXNETNODES)
			doomcom->numnodes = MAXNETNODES;

		// server
		servernode = 0;
		// FIXME:
		// ??? and now ?
		// server on a big modem ??? 4*isdn
		net_bandwidth = 16000;
		hardware_MAXPACKETLENGTH = INETPACKETLENGTH;

		ret = true;
	}
	else if(M_CheckParm("-connect"))
	{
		if(M_IsNextParm())
			strcpy(serverhostname, M_GetNextParm());
		else
			serverhostname[0] = 0; // assuming server in the LAN, use broadcast to detect it

		// server address only in ip
		if(serverhostname[0])
		{
			COM_BufAddText("connect \"");
			COM_BufAddText(serverhostname);
			COM_BufAddText("\"\n");

			// probably modem
			hardware_MAXPACKETLENGTH = INETPACKETLENGTH;
		}
		else
		{
			// so we're on a LAN
			COM_BufAddText("connect any\n");

			net_bandwidth = 800000;
			hardware_MAXPACKETLENGTH = MAXPACKETLENGTH;
		}
	}

	mypacket.maxlen = hardware_MAXPACKETLENGTH;
	I_NetOpenSocket = NET_OpenSocket;
	I_Ban = NET_Ban;
	I_ClearBans = NET_ClearBans;
	bannednode = NET_bannednode;

	return ret;
#else
	if( M_CheckParm ("-net") )
	{
		I_Error("-net not supported, use -server and -connect\n"
			"see docs for more\n");
	}
	return false;
#endif
}
