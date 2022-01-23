// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_tcp.c,v 1.12 2000/08/10 14:55:56 ydario Exp $
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
// $Log: i_tcp.c,v $
// Revision 1.12  2000/08/10 14:55:56  ydario
// OS/2 port
//
// Revision 1.11  2000/08/10 14:08:48  hurdler
// no message
//
// Revision 1.10  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.9  2000/04/21 13:03:27  hurdler
// apply Robert's patch for SOCK_Get error. Boris, can you verify this?
//
// Revision 1.8  2000/04/21 00:01:45  hurdler
// apply Robert's patch for SOCK_Get error. Boris, can you verify this?
//
// Revision 1.7  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.5  2000/03/08 14:44:52  hurdler
// fix "select" problem under linux
//
// Revision 1.4  2000/03/07 03:32:24  hurdler
// fix linux compilation
//
// Revision 1.3  2000/03/06 15:46:43  hurdler
// compiler warning removed
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
// NOTE:    This is not realy Os dependant because all Os have the same Socket api
//          Just use '#ifdef' for Os dependant stuffs
//
//-----------------------------------------------------------------------------


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef __OS2__
#include <sys/types.h>
#include <sys/time.h>
#endif // __OS2__

#ifdef __WIN32__
#include <winsock.h>
#include <wsipx.h>
#else
#if !defined(SCOUW2) && !defined(SCOUW7) && !defined(__OS2__)
#include <arpa/inet.h>
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <time.h>

/* BP: is anyone use this ?
#if !defined(LINUX) && !defined(SCOOS5) && !defined(_AIX)
#include <sys/filio.h>
#endif
*/

#ifdef __DJGPP__
#include <lsck/lsck.h>
//#define strerror  lsck_strerror

// ipx not iet supported in libsocket (cut and pasted from wsipx.h (winsock)
typedef struct sockaddr_ipx {
    short sa_family;
    char  sa_netnum[4];
    char  sa_nodenum[6];
    unsigned short sa_socket;
} SOCKADDR_IPX, *PSOCKADDR_IPX;
#define NSPROTO_IPX      1000

#endif // djgpp

#ifdef __OS2__

// ipx not iet supported in libsocket (cut and pasted from wsipx.h (winsock)
#define AF_IPX          23              /* Novell Internet Protocol */
typedef struct sockaddr_ipx {
    short sa_family;
    char  sa_netnum[4];
    char  sa_nodenum[6];
    unsigned short sa_socket;
} SOCKADDR_IPX, *PSOCKADDR_IPX;
#define NSPROTO_IPX      1000

#endif // os2

#ifdef LINUX
    #include <sys/time.h>
    # ifdef __GLIBC__
        #include <netipx/ipx.h>
    #else
        #include <linux/ipx.h>
    #endif // glibc
    typedef struct sockaddr_ipx SOCKADDR_IPX, *PSOCKADDR_IPX;
    #define NSPROTO_IPX      PF_IPX
#endif // linux
#endif // win32

#include "doomdef.h"
#include "i_system.h"
#include "i_net.h"
#include "d_net.h"
#include "m_argv.h"
#include "command.h"

#include "doomstat.h"

#ifdef __WIN32__
    // some undifined under win32
    #define IPPORT_USERRESERVED 5000
    #define errno             h_errno // some very strange things happen when not use h_error ?!?
    #define EWOULDBLOCK   WSAEWOULDBLOCK
    #define EMSGSIZE      WSAEMSGSIZE
    #define ECONNREFUSED  WSAECONNREFUSED
#else // linux or djgpp
    #define  SOCKET ULONG
    #define  INVALID_SOCKET -1
#endif

// win32 or djgpp
#if !defined( LINUX) && !defined( __OS2__)
    // winsock stuff (in winsock a socket is not a file)
    #define ioctl ioctlsocket
    #define close closesocket
#endif

typedef union {
        struct sockaddr_in  ip;
        struct sockaddr_ipx ipx;
}  mysockaddr_t;

static mysockaddr_t clientaddress[MAXNETNODES+1];
        
static SOCKET   mysocket = -1;
static int     DOOMPORT = (IPPORT_USERRESERVED +0x1d );  // 5029
static boolean nodeconnected[MAXNETNODES+1];
static boolean  ipx;

char *SOCK_AddrToStr(mysockaddr_t *sk)
{
    static char s[50];

    if( sk->ip.sin_family==AF_INET)
    {
        sprintf(s,"%d.%d.%d.%d:%d",((byte *)(&(sk->ip.sin_addr.s_addr)))[0],
                                   ((byte *)(&(sk->ip.sin_addr.s_addr)))[1],
                                   ((byte *)(&(sk->ip.sin_addr.s_addr)))[2],
                                   ((byte *)(&(sk->ip.sin_addr.s_addr)))[3],
                                   ntohs(sk->ip.sin_port));
    }
    else
#ifdef LINUX
    if( sk->ipx.sipx_family==AF_IPX )
    {
        sprintf(s,"%08x.%02x%02x%02x%02x%02x%02x:%d",
                  sk->ipx.sipx_network,
                  (byte)sk->ipx.sipx_node[0],
                  (byte)sk->ipx.sipx_node[1],
                  (byte)sk->ipx.sipx_node[2],
                  (byte)sk->ipx.sipx_node[3],
                  (byte)sk->ipx.sipx_node[4],
                  (byte)sk->ipx.sipx_node[5],
                  sk->ipx.sipx_port);
    }
#else
    if( sk->ipx.sa_family==AF_IPX )
    {
        sprintf(s,"%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x:%d",
                  (byte)sk->ipx.sa_netnum[0],
                  (byte)sk->ipx.sa_netnum[1],
                  (byte)sk->ipx.sa_netnum[2],
                  (byte)sk->ipx.sa_netnum[3],
                  (byte)sk->ipx.sa_nodenum[0],
                  (byte)sk->ipx.sa_nodenum[1],
                  (byte)sk->ipx.sa_nodenum[2],
                  (byte)sk->ipx.sa_nodenum[3],
                  (byte)sk->ipx.sa_nodenum[4],
                  (byte)sk->ipx.sa_nodenum[5],
                  sk->ipx.sa_socket);
    }
#endif // linux
    else
        sprintf(s,"Unknow type");
    return s;
}

boolean IPX_cmpaddr(mysockaddr_t *a,mysockaddr_t *b)
{
#ifdef LINUX
    return ((memcmp(&(a->ipx.sipx_network) ,&(b->ipx.sipx_network) ,4)==0) &&
            (memcmp(&(a->ipx.sipx_node),&(b->ipx.sipx_node),6)==0));
#else
    return ((memcmp(&(a->ipx.sa_netnum) ,&(b->ipx.sa_netnum) ,4)==0) &&
            (memcmp(&(a->ipx.sa_nodenum),&(b->ipx.sa_nodenum),6)==0));
#endif // linux
}

boolean UDP_cmpaddr(mysockaddr_t *a,mysockaddr_t *b)
{
    return (a->ip.sin_addr.s_addr == b->ip.sin_addr.s_addr);
}

boolean (*SOCK_cmpaddr) (mysockaddr_t *a,mysockaddr_t *b);


static int getfreenode( void )
{
    int j;

    for(j=0;j<MAXNETNODES;j++)
        if( !nodeconnected[j] )
        {
            nodeconnected[j]=true;
            return j;
        }
    return -1;
}

void SOCK_Get(void)
{
    int           i,j,c;
    size_t        fromlen;
    mysockaddr_t  fromaddress;

    fromlen = sizeof(fromaddress);
    c = recvfrom (mysocket, (char *)&doomcom->data, MAXPACKETLENGHT, 0,
                  (struct sockaddr *)&fromaddress, &fromlen );
    if (c == -1 )
    {
        if ( (errno==EWOULDBLOCK) || 
             (errno==EMSGSIZE)    || 
             (errno==ECONNREFUSED) )
        {
             doomcom->remotenode = -1;      // no packet
             return;
        }
        I_Error ("SOCK_Get: %s",strerror(errno));
    }
    
//    DEBFILE(va("Get from %s\n",SOCK_AddrToStr(&fromaddress)));

    // find remote node number
    for (i=0 ; i<MAXNETNODES ; i++)
        if ( SOCK_cmpaddr(&fromaddress,&(clientaddress[i])) )
        {
            doomcom->remotenode = i;      // good packet from a game player
            doomcom->datalength = c;
            return;
        }

    // not found
    // find a free slot
    j=getfreenode();
    if(j>0)
    {
        memcpy(&clientaddress[j],&fromaddress,fromlen);
#ifdef DEBUGFILE
            if( debugfile )
                fprintf(debugfile,"New node detected : node:%d address:%s\n",j,SOCK_AddrToStr(&clientaddress[j]));
#endif
            doomcom->remotenode = j; // good packet from a game player
            doomcom->datalength = c;
            return;
        }

    // node table full
    if( debugfile )
        fprintf(debugfile,"New node detected : No more free slote\n");

    doomcom->remotenode = -1;               // no packet
    return;

    // byte swap
    // -= insert here endian conversions =-
}

struct timeval timeval_for_select={0,0};
fd_set set;

// check if we can send (do not go over the buffer)
boolean SOCK_CanSend(void)
{
   return select(1,NULL,&set,NULL,&timeval_for_select);
}

void SOCK_Send(void)
{
    int         c;
    // -= insert here endian conversions =-
                         
    if( !nodeconnected[doomcom->remotenode] )
        return;

    c = sendto (mysocket , (char *)&doomcom->data, doomcom->datalength
                ,0,(struct sockaddr *)&clientaddress[doomcom->remotenode]
                ,sizeof(struct sockaddr));

//    DEBFILE(va("send to %s\n",SOCK_AddrToStr(&clientaddress[doomcom->remotenode])));
    // ECONNREFUSED was send by linux port
    if (c == -1 && errno!=ECONNREFUSED && errno!=EWOULDBLOCK)
        I_Error ("SOCK_Send sending to node %d (%s): %s",doomcom->remotenode,SOCK_AddrToStr(&clientaddress[doomcom->remotenode]),strerror(errno));
}

void SOCK_FreeNodenum(int numnode)
{
    // can't disconnect to self :)
    if(!numnode)
        return;

    if( debugfile )
        fprintf(debugfile,"Free node %d (%s)\n",numnode,SOCK_AddrToStr(&clientaddress[numnode]));

    nodeconnected[numnode]=false;

    // put invalide address
    memset(&clientaddress[numnode],0,sizeof(clientaddress[numnode]));
}

//
// UDPsocket
//
SOCKET UDP_Socket (void)
{
    SOCKET s;
    struct sockaddr_in  address;
    int    trueval = true;
    int i,j;

    // allocate a socket
    s = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s<0 || s==INVALID_SOCKET)
        I_Error ("Udp_socket: Can't create socket: %s",strerror(errno));

    memset (&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    
    //Hurdler: I'd like to put a server and a client on the same computer
    if ( (i = M_CheckParm ("-clientport"))!=0 )
        address.sin_port = htons(atoi(myargv[i+1]));
    else
        address.sin_port = htons(DOOMPORT);

    if (bind (s, (struct sockaddr *)&address, sizeof(address)) == -1)
        I_Error ("UDP_Bind: %s", strerror(errno));

    // make it non blocking
    ioctl (s, FIONBIO, &trueval);

    // make it broadcastable
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *)&trueval, sizeof(trueval));

    j=4;
    getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, (size_t *)&j);
    CONS_Printf("Network system buffer : %dKb\n",i>>10);
    if(i<64<<10)
    {
        i=64<<10;
        if( setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, sizeof(i))!=0 )
            CONS_Printf("Can't set buffer lenght to 64k, file transfer will be bad\n");
        else
            CONS_Printf("Network system buffer set to : %d\n",i);
    }

    // ip + udp
    packetheaderlength=20 + 8; // for stats

    clientaddress[0].ip.sin_family      = AF_INET;
    clientaddress[0].ip.sin_port        = htons(DOOMPORT);
    clientaddress[0].ip.sin_addr.s_addr = INADDR_LOOPBACK; //GetLocalAddress(); // my own ip
                                      // inet_addr("127.0.0.1");
    // setup broadcast adress to BROADCASTADDR entry
    clientaddress[BROADCASTADDR].ip.sin_family      = AF_INET;
    clientaddress[BROADCASTADDR].ip.sin_port        = htons(DOOMPORT);
    clientaddress[BROADCASTADDR].ip.sin_addr.s_addr = INADDR_BROADCAST;

    doomcom->extratics=1; // internet is very high ping

    SOCK_cmpaddr=UDP_cmpaddr;
    return s;
}

SOCKET IPX_Socket (void)
{
    SOCKET s;
    SOCKADDR_IPX  address;
    int    trueval = true;
    int    i,j;

    // allocate a socket
    s = socket (AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
    if (s<0 || s==INVALID_SOCKET)
        I_Error ("IPX_socket: Can't create socket: %s",strerror(errno));

    memset (&address, 0, sizeof(address));
#ifdef LINUX
    address.sipx_family = AF_IPX;
    address.sipx_port = htons(DOOMPORT);
#else
    address.sa_family = AF_IPX;
    address.sa_socket = htons(DOOMPORT);
#endif // linux
    if (bind (s, (struct sockaddr *)&address, sizeof(address)) == -1)
        I_Error ("IPX_Bind: %s", strerror(errno));

    // make it non blocking
    ioctl (s, FIONBIO, &trueval);

    // make it broadcastable
    setsockopt(s, SOL_SOCKET, SO_BROADCAST, (char *)&trueval, sizeof(trueval));

    // set receive buffer to 64Kb
    j=4;
    getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, (size_t *)&j);
    CONS_Printf("Network system receive buffer : %d\n",i);
    if(i<128<<10)
    {
        i=64<<10;
        if( setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&i, sizeof(i))!=0 )
            CONS_Printf("Can't set receive buffer lenght to 64k, file transfer will be bad\n");
        else
            CONS_Printf("Network system receive buffer set to : %d\n",i);
    }

    // ipx header
    packetheaderlength=30; // for stats

    // setup broadcast adress to BROADCASTADDR entry
#ifdef LINUX
    clientaddress[BROADCASTADDR].ipx.sipx_family = AF_IPX;
    clientaddress[BROADCASTADDR].ipx.sipx_port = htons(DOOMPORT);
    clientaddress[BROADCASTADDR].ipx.sipx_network = 0;
    for(i=0;i<6;i++)
       clientaddress[BROADCASTADDR].ipx.sipx_node[i] = (byte)0xFF;
#else
    clientaddress[BROADCASTADDR].ipx.sa_family = AF_IPX;
    clientaddress[BROADCASTADDR].ipx.sa_socket = htons(DOOMPORT);
    for(i=0;i<4;i++)
       clientaddress[BROADCASTADDR].ipx.sa_netnum[i] = 0;
    for(i=0;i<6;i++)
       clientaddress[BROADCASTADDR].ipx.sa_nodenum[i] = (byte)0xFF;
#endif // linux
    SOCK_cmpaddr=IPX_cmpaddr;
    return s;
}


void SOCK_Shutdown( void )
{
    if( mysocket>=0 )
    {
// quick fix bug in libsocket 0.7.4 beta 4 onder winsock 1.1 (win95)
#ifndef __DJGPP__
        close(mysocket);
#endif
        mysocket = -1;
    }
#ifdef __WIN32__
    WSACleanup();
#else
#ifdef __DJGPP__
    __lsck_uninit();
#endif
#endif
}

#ifdef __WIN32__
WSADATA winsockdata;
#endif

int SOCK_NetGetServerNode (char *serverhostname)
{
    int servernode;
    // server address only in ip
    if(!ipx) // tcp/ip
    {
        struct  hostent*     hostentry;      // host information entry
#define serveraddress clientaddress[servernode]
        servernode = getfreenode();
        // find ip of the server
        serveraddress.ip.sin_family      = AF_INET;
        serveraddress.ip.sin_port        = htons(DOOMPORT);
        serveraddress.ip.sin_addr.s_addr = inet_addr(serverhostname);
        
        if(serveraddress.ip.sin_addr.s_addr==INADDR_NONE) // not a ip ask to the dns
        {
            CONS_Printf("Resolving %s\n",serverhostname);
            hostentry = gethostbyname (serverhostname);
            if (!hostentry)
            {
                CONS_Printf ("Server %s unknow\n", serverhostname);
                I_NetFreeNodenum(servernode);
                return -1;
            }
            serveraddress.ip.sin_addr.s_addr = *(int *)hostentry->h_addr_list[0];
        }
        CONS_Printf("Server is %s\n",inet_ntoa(*(struct in_addr *)&serveraddress.ip.sin_addr.s_addr));

        return servernode;
    }

    return BROADCASTADDR;
}

int I_InitTcpNetwork( void )
{
    int     i;
    char    serverhostname[255];

    // parse network game options,
    if ( M_CheckParm ("-server") )
    {
        server=true;

        if( M_IsNextParm() )
            doomcom->numnodes=atoi(M_GetNextParm());
        else
            doomcom->numnodes=1;

        if (doomcom->numnodes<1)
            doomcom->numnodes=1;
        if (doomcom->numnodes>MAXNETNODES)
            doomcom->numnodes=MAXNETNODES;
    }
    else
    {
        if( M_CheckParm ("-connect") )
        {
            if(M_IsNextParm())
                strcpy(serverhostname,M_GetNextParm());
            else
                serverhostname[0]=0; // assuming server in the LAN, use broadcast to detect it
            server=false;
        }
        else
            // no command line for tcp/ip :(
            return 0;
    }

    ipx=M_CheckParm("-ipx");
    // initilize the driver
#ifdef __WIN32__
    if( WSAStartup(MAKEWORD(1,1),&winsockdata) )
        I_Error("No Tcp/Ip driver detected");
#else
    #ifdef __DJGPP__
        if( !__lsck_init() )
            I_Error("No Tcp/Ip driver detected");
    #endif
#endif


    if ( (i = M_CheckParm ("-udpport"))!=0 )
        DOOMPORT = atoi(myargv[i+1]);

    for(i=0;i<MAXNETNODES;i++)
        nodeconnected[i]=false;

    nodeconnected[0] = true; // always connected to self
    nodeconnected[BROADCASTADDR] = true;
    I_NetSend     = SOCK_Send;
    I_NetGet      = SOCK_Get;
    I_NetShutdown = SOCK_Shutdown;
    I_NetFreeNodenum = SOCK_FreeNodenum;
    I_NetGetServerNode = SOCK_NetGetServerNode;

#ifdef __WIN32__
    // seem like not work with libsocket nor linux :(
    I_NetCanSend  = SOCK_CanSend;
#endif

    memset(clientaddress,0,sizeof(clientaddress));

    if( !server )
    {
        // server address only in ip
        if(serverhostname[0] && !ipx)
        {
            COM_BufAddText("connect \"");
            COM_BufAddText(serverhostname);
            COM_BufAddText("\"\n");

            // probably modem
            hardware_MAXPACKETLENGHT = 512;
        }
        else
        {
            COM_BufAddText("connect any\n");

            servernode = BROADCASTADDR;
            net_bandwidth = 800000;
            hardware_MAXPACKETLENGHT = MAXPACKETLENGHT;
        }
    }
    else
    {
        // server
        servernode = 0;
        // FIXME:
        // ??? and now ?
        // server on a big modem ??? 4*isdn
        net_bandwidth = 16000;
        hardware_MAXPACKETLENGHT = 512;
    }

    // build the socket
    if(ipx) {
        mysocket = IPX_Socket ();
        net_bandwidth = 800000;
        hardware_MAXPACKETLENGHT = MAXPACKETLENGHT;
    }
    else
        mysocket = UDP_Socket ();

    // for select
    FD_ZERO(&set);
    FD_SET(mysocket,&set);
    //set.fd_count=1;
    //set.fd_array[0]=;


    return 1;
}
