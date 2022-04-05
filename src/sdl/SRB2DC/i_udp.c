// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 2005 by Sonic Team Jr.
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
/// \brief KOS UDP network interface

#include "../../doomdef.h"

#include "../../i_system.h"
#include "../../d_event.h"
#include "../../d_net.h"
#include "../../m_argv.h"

#include "../../doomstat.h"

#include "../../i_net.h"

#include "../../z_zone.h"

#include "../../i_tcp.h"

#include <kos/net.h>
//#include <net/net_ipv4.h>
#define NET_NONE  0x00000000
#define NET_LOCAL 0x0100007F
#define NET_ANY   0xFFFFFFFF
uint16 net_ntohs(uint16 n);
uint32 net_ntohl(uint32 n);

#define MAXBANS 20

typedef struct 
{
	uint32 host;
	uint16 port;
} IPaddress;

static IPaddress clientaddress[MAXNETNODES+1];
static boolean nodeconnected[MAXNETNODES+1];

static int mysocket = 0;
static boolean init_KOSUDP_driver = false;

static size_t numbans = 0;
static IPaddress banned[MAXBANS];
static boolean KOSUDP_bannednode[MAXNETNODES+1]; /// \note do we really need the +1?

static inline int net_udp_sendto(int sock, const uint8 *data, int size, uint16 rem_port, uint32 rem_addr)
{
	uint8 dst_ip[4] = {((uint8*)(&(rem_addr)))[0],
			((uint8*)(&(rem_addr)))[1],
			((uint8*)(&(rem_addr)))[2],
			((uint8*)(&(rem_addr)))[3]};
	return net_udp_send_raw(net_default_dev, clientaddress[0].port, rem_port, dst_ip, data, size);
	sock = 0;
}

static inline int net_udp_recvfrom(int sock, uint8 *buf, int size, uint16 *rem_port, uint32 *rem_addr)
{
	return net_udp_recv(sock, buf, size);
	rem_port = NULL;
	rem_addr = NULL;
}

static inline uint32 inet_addr(const char *host)
{
	uint32 val;
	int base, n;
	char c;
	uint32 parts[4];
	uint32* pp = parts;
 
	c = *host;
	for (;;)
	{
		/*
		 * Collect number up to ``.''.
		 * Values are specified as for C:
		 * 0x=hex, 0=octal, isdigit=decimal.
		 */
		if (!isdigit(c))
			return (0);
		val = 0; base = 10;
		if (c == '0')
		{
			c = *++host;
			if (c == 'x' || c == 'X')
				base = 16, c = *++host;
			else
				base = 8;
		}
		for (;;)
		{
			if (isascii(c) && isdigit(c))
			{
				val = (val * base) + (c - '0');
				c = *++host;
			}
			else if (base == 16 && isascii(c) && isxdigit(c))
			{
				val = (val << 4) |
				 (c + 10 - (islower(c) ? 'a' : 'A'));
				c = *++host;
			}
			else
				break;
		}
		if (c == '.')
		{
			/*
			 * Internet format:
			 *  a.b.c.d
			 *  a.b.c   (with c treated as 16 bits)
			 *  a.b (with b treated as 24 bits)
			 */
			if (pp >= parts + 3)
				return (0);
			*pp++ = val;
			c = *++host;
		}
		else
			break;
	}
	/*
	 * Check for trailing characters.
	 */
	if (c != '\0' && (!isascii(c) || !isspace(c)))
		return (0);
	/*
	 * Concoct the address according to
	 * the number of parts specified.
	 */
	n = pp - parts + 1;
	switch (n)
	{
 
		case 0:
			return (0);     /* initial nondigit */
 
		case 1:             /* a -- 32 bits */
			break;
 
		case 2:             /* a.b -- 8.24 bits */
			if (val > 0xffffff)
				return (0);
			val |= parts[0] << 24;
			break;
 
		case 3:             /* a.b.c -- 8.8.16 bits */
			if (val > 0xffff)
				return (0);
			val |= (parts[0] << 24) | (parts[1] << 16);
			break;
 
		case 4:             /* a.b.c.d -- 8.8.8.8 bits */
			if (val > 0xff)
				return (0);
			val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
			break;
	}
	return net_ntohl(val);
}

static const char* KOSUDP_AddrToStr(IPaddress* sk)
{
	static char s[50];
	sprintf(s,"%d.%d.%d.%d:%d",
			((uint8*)(&(sk->host)))[0],
			((uint8*)(&(sk->host)))[1],
			((uint8*)(&(sk->host)))[2],
			((uint8*)(&(sk->host)))[3],
			net_ntohs(sk->port));
	return s;
}

static boolean KOSUDP_cmpaddr(IPaddress* a, IPaddress* b)
{
	return (a->host == b->host && a->port == b->port);
}

static signed char getfreenode(void)
{
	signed char j;

	for(j = 0; j < MAXNETNODES; j++)
		if(!nodeconnected[j])
		{
			nodeconnected[j] = true;
			return j;
		}
	return -1;
}

static void KOSUDP_Get(void)
{
	int size;
	size_t i;
	signed char j;
	IPaddress temp = {clientaddress[BROADCASTADDR].host,clientaddress[BROADCASTADDR].port};

	size = net_udp_recvfrom(mysocket,(char *)&doomcom->data, MAXPACKETLENGTH, &temp.port, &temp.host);
	if(size == 0)
	{
		doomcom->remotenode = -1; // no packet
		return;
	}

	// find remote node number
	for(i = 0; i < MAXNETNODES; i++)
		if(KOSUDP_cmpaddr(&temp, &(clientaddress[i])))
		{
			doomcom->remotenode = (short)i; // good packet from a game player
			doomcom->datalength = (short)size;
			return;
		}

	// not found

	// find a free slot
	j = getfreenode();
	if(j > 0)
	{
		memcpy(&clientaddress[j], &temp, sizeof(temp));
		DEBFILE(va("New node detected: node:%d address:%s\n", j,
				KOSUDP_AddrToStr(&clientaddress[j])));
		doomcom->remotenode = (short)j; // good packet from a game player
		doomcom->datalength = (short)size;
		// check if it's a banned dude so we can send a refusal later
		for(i = 0; i < numbans; i++)
			if(KOSUDP_cmpaddr(&temp, &banned[i]))
			{
				KOSUDP_bannednode[j] = true;
				DEBFILE("This dude has been banned\n");
				break;
			}
		if(i == numbans)
			KOSUDP_bannednode[j] = false;
		return;
	}

	DEBFILE("New node detected: No more free slots\n");
	doomcom->remotenode = -1; // no packet
}

#if 0
static boolean KOSUDP_CanSend(void)
{
	return true;
}
#endif

static void KOSUDP_Send(void)
{
	const IPaddress *nodeinfo;

	if(!doomcom->remotenode || !nodeconnected[doomcom->remotenode])
		return;

	nodeinfo = clientaddress + doomcom->remotenode;

	if(net_udp_sendto(mysocket, (char *)&doomcom->data, doomcom->datalength, nodeinfo->port, nodeinfo->host) == -1)
	{
		CONS_Printf("KOSUDP: error sending data\n");
	}
}

static void KOSUDP_FreeNodenum(int numnode)
{
	// can't disconnect from self :)
	if(!numnode)
		return;

	DEBFILE(va("Free node %d (%s)\n", numnode, KOSUDP_AddrToStr(&clientaddress[numnode])));

	nodeconnected[numnode] = false;

	memset(&clientaddress[numnode], 0, sizeof(IPaddress));
}

static int KOSUDP_Socket(void)
{
	int temp = 0;
	uint16 portnum = 0;
	const uint32 hostip = net_default_dev?net_ntohl(net_ipv4_address(net_default_dev->ip_addr)):NET_LOCAL;
	//Hurdler: I'd like to put a server and a client on the same computer
	//Logan: Me too
	//BP: in fact for client we can use any free port we want i have read
	//    in some doc that connect in udp can do it for us...
	//Alam: where?
	if(M_CheckParm("-clientport"))
	{
		if(!M_IsNextParm())
			I_Error("syntax: -clientport <portnum>");
		portnum = net_ntohs(atoi(M_GetNextParm()));
	}
	else
		portnum = net_ntohs(sock_port);

	temp = net_udp_sock_open(portnum, hostip, portnum, NET_NONE);
	if(temp)
	{
		int btemp = net_udp_sock_open(portnum, hostip, portnum, NET_ANY);
		clientaddress[0].port = portnum;
		clientaddress[0].host = NET_NONE;
		if(btemp)
		{
			clientaddress[BROADCASTADDR].port = net_ntohs(sock_port);
			clientaddress[BROADCASTADDR].host = NET_ANY;
		}
		else
		{
			CONS_Printf("KOSUDP: can't setup broadcast sock\n");
			net_udp_sock_close(temp);
			return 0;
		}
	}
	else
	{
		CONS_Printf("KOSUDP: can't setup main sock\n");
		return 0;
	}

	doomcom->extratics = 1; // internet is very high ping

	return temp;
}

static void I_ShutdownKOSUDPDriver(void)
{
	//net_shutdown();
	init_KOSUDP_driver = false;
}

static void I_InitKOSUDPDriver(void)
{
	if(init_KOSUDP_driver)
		I_ShutdownKOSUDPDriver();
	else
		net_init();
	D_SetDoomcom();
	memset(&clientaddress,0,sizeof(clientaddress));
	init_KOSUDP_driver = true;
}

static void KOSUDP_CloseSocket(void)
{
	if(mysocket)
		net_udp_sock_close(mysocket);
	mysocket = 0;
}

static signed char KOSUDP_NetMakeNode(const char *hostname)
{
	signed char newnode;
	char* portchar, *t, *localhostname = strdup(hostname);
	uint16 portnum = net_ntohs(sock_port);

	// retrieve portnum from address!
	strtok(localhostname, ":");
	portchar = strtok(NULL, ":");
	if(portchar)
		portnum = net_ntohs((unsigned short)atoi(portchar));

	// remove the port in the hostname as we've it already
	t = localhostname = strdup(hostname);
	while((*t != ':') && (*t != '\0'))
		t++;
	*t = '\0';

	newnode = getfreenode();
	if(newnode == -1)
		return -1;
	// find ip of the server
	clientaddress[newnode].port = portnum;
	clientaddress[newnode].host = inet_addr(localhostname);
	
	if(clientaddress[newnode].host == NET_NONE)
	{
		free(localhostname);
		return -1;
	}
	free(localhostname);
	return newnode;
}


static boolean KOSUDP_OpenSocket(void)
{
	size_t i;

	memset(clientaddress, 0, sizeof(clientaddress));

	for(i = 0; i < MAXNETNODES; i++)
		nodeconnected[i] = false;

	//CONS_Printf("KOSUDP Code starting up\n");

	nodeconnected[0] = true; // always connected to self
	nodeconnected[BROADCASTADDR] = true;
	I_NetSend = KOSUDP_Send;
	I_NetGet = KOSUDP_Get;
	I_NetCloseSocket = KOSUDP_CloseSocket;
	I_NetFreeNodenum = KOSUDP_FreeNodenum;
	I_NetMakeNode = KOSUDP_NetMakeNode;

	//I_NetCanSend = KOSUDP_CanSend;

	// build the socket but close it first
	KOSUDP_CloseSocket();
	mysocket = KOSUDP_Socket();

	if(mysocket)
	{
#if 0
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
#endif
		return true;
	}
	return false;
}

static boolean KOSUDP_Ban(int node)
{
	if(numbans == MAXBANS)
		return false;

	memcpy(&banned[numbans], &clientaddress[node], sizeof(IPaddress));
	numbans++;
	return true;
}

static void KOSUDP_ClearBans(void)
{
	numbans = 0;
}

//
// I_InitNetwork
// Only required for DOS, so this is more a dummy
//
boolean I_InitNetwork(void)
{
	char serverhostname[255];
	boolean ret = false;
	//if(!M_CheckParm ("-kosnet"))
	//	return false;
	// initilize the driver
	I_InitKOSUDPDriver();
	I_AddExitFunc(I_ShutdownKOSUDPDriver);
	if(!init_KOSUDP_driver)
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

	I_NetOpenSocket = KOSUDP_OpenSocket;
	I_Ban = KOSUDP_Ban;
	I_ClearBans = KOSUDP_ClearBans;
	bannednode = KOSUDP_bannednode;

	return ret;
}
