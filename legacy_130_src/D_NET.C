// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_net.c,v 1.5 2000/04/16 18:38:07 bpereira Exp $
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
// $Log: d_net.c,v $
// Revision 1.5  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.4  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      DOOM Network game communication and protocol,
//      all OS independend parts.
//
//      Implemente a Sliding window protocol without receiver window
//      (out of order reception)
//      This protocol use mix of "goback n" and "selective repeat" implementation
//      The NOTHING packet is send when connection is idle for acknowledge packets
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "i_net.h"
#include "i_system.h"
#include "m_argv.h"
#include "d_net.h"
#include "w_wad.h"
#include "d_netfil.h"
#include "d_clisrv.h"

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// server:
//   maketic is the tic that hasn't had control made for it yet
//   nettics : is the tic for eatch node
//   firsttictosend : is the lowest value of nettics
// client:
//   neededtic : is the tic needed by the client for run the game
//   firsttictosend : is used to optimize a condition
// normaly maketic>=gametic>0,


doomcom_t*  doomcom;
doomdata_t* netbuffer;        // points inside doomcom

#define CONNECTIONTIMEOUT  (3*TICRATE) // Tails 03-30-2001

// UNUSED
int         ticdup;

FILE*       debugfile;        // put some net info in a file
                              // during the game

#define     MAXREBOUND 8
static doomdata_t  reboundstore[MAXREBOUND];
static short       reboundsize[MAXREBOUND];
static int         rebound_head,rebound_tail;
int         net_bandwidth;
short       hardware_MAXPACKETLENGHT;

void    (*I_NetGet) (void);
void    (*I_NetSend) (void);
boolean (*I_NetCanSend) (void);
void    (*I_NetShutdown) (void);
void    (*I_NetFreeNodenum) (int nodenum);
int     (*I_NetGetServerNode) (char *serveraddress);

void ExecuteExitCmd(); // Tails 03-30-2001

// network stats
ULONG       statstarttic;
int         getbytes=0;
INT64       sendbytes=0;
int         retransmit=0   ,duppacket=0;
int         sendackpacket=0,getackpacket=0;
int         ticruned=0     ,ticmiss=0;

// globals
int    getbps,sendbps;
float  lostpercent,duppercent,gamelostpercent;
int    packetheaderlength;

boolean Net_GetNetStat(void)
{
    ULONG t=I_GetTime();
static int oldsendbyte=0;
    if( statstarttic+STATLENGTH<=t )
    {
        getbps=(getbytes*TICRATE)/(t-statstarttic);
        sendbps=((sendbytes-oldsendbyte)*TICRATE)/(t-statstarttic);
        if(sendackpacket)
            lostpercent=100.0*(float)retransmit/(float)sendackpacket;
        else
            lostpercent=0;
        if(getackpacket)
            duppercent=100.0*(float)duppacket/(float)getackpacket;
        else
            duppercent=0;
        if( ticruned )
            gamelostpercent=100.0*(float)ticmiss/(float)ticruned;
        else
            gamelostpercent=0;

        ticmiss=ticruned=0;
        oldsendbyte=sendbytes;
        getbytes=0;
        sendackpacket=getackpacket=duppacket=retransmit=0;
        statstarttic=t;

        return 1;
    }
    return 0;
}

// -----------------------------------------------------------------
//  Some stuct and function for acknowledgment of packets
// -----------------------------------------------------------------
#define MAXACKPACKETS    64 // minimum number of nodes
#define MAXACKTOSEND     64
#define URGENTFREESLOTENUM   6
#define ACKTOSENDTIMEOUT  2

typedef struct {
  byte   acknum;
  byte   nextacknum;
  byte   destinationnode;
  byte   resent;
  ULONG  senttime;
  USHORT length;
  char   pak[MAXPACKETLENGHT];
} ackpak_t;

// table of packet that was not acknowleged can be resend (the sender window)
static ackpak_t ackpak[MAXACKPACKETS];
// little queu of ack to send to a node
static byte     acktosend[MAXNETNODES][MAXACKTOSEND];
static byte     firstacktosend[MAXNETNODES];
static byte     remotefirstack[MAXNETNODES];
static byte     nextacknum[MAXNETNODES];
// head and tail of eatch queu
static byte     acktosend_head[MAXNETNODES],acktosend_tail[MAXNETNODES];

static ULONG    lasttimeacktosend_sent[MAXNETNODES];
static ULONG    lasttimepacketreceived[MAXNETNODES]; // Tails 03-30-2001

// jacobson tcp timeout evaluation algorithm (Karn variation)
static fixed_t  ping[MAXNETNODES];
static fixed_t  varping[MAXNETNODES];
int timeout[MAXNETNODES]; // Tails 03-30-2001
#define  PINGDEFAULT     ((200*TICRATE*FRACUNIT)/1000)
#define  VARPINGDEFAULT  ( (50*TICRATE*FRACUNIT)/1000)
#define  TIMEOUT(p,v)    (p+4*v+FRACUNIT/2)>>FRACBITS; // Tails 03-30-2001


// return <0 if a<b (mod 256)
//         0 if a=n (mod 256)
//        >0 if a>b (mod 256)
// mnemonic: to use it compare to 0 : cmpack(a,b)<0 is "a<b" ...
static int cmpack(byte a,byte b)
{
    register int d=a-b;

    if(d>=127 || d<-128)
        return -d;
    return d;
}

// return a free acknum and copy netbuffer in the ackpak table
static boolean GetFreeAcknum(byte *freeack, boolean lowtimer)
{
   int node=doomcom->remotenode;
   int i,numfreeslote=0;
   
   if(cmpack((remotefirstack[node]+MAXACKTOSEND) % 256,nextacknum[node])<0)
   {
       DEBFILE(va("too fast %d %d\n",remotefirstack[node],nextacknum[node]));
       return false;
   }

   for(i=0;i<MAXACKPACKETS;i++)
       if(ackpak[i].acknum==0)
       {
           // for low priority packet, make sure let freeslotes so urgents packets can be sent
           numfreeslote++;
           if( netbuffer->packettype >= CANFAIL && numfreeslote<URGENTFREESLOTENUM)
               continue;

           ackpak[i].acknum=nextacknum[node];
           ackpak[i].nextacknum=nextacknum[node];
           nextacknum[node]++;
           if( nextacknum[node]==0 )
               nextacknum[node]++;
           ackpak[i].destinationnode=node;
           ackpak[i].length=doomcom->datalength;
           if(lowtimer)
           {
               // lowtime mean can't be sent now so try it soon as possible
               ackpak[i].senttime=0;
               ackpak[i].resent=true;
           }
           else
           {
               ackpak[i].senttime=I_GetTime();
               ackpak[i].resent=false;
           }
           memcpy(ackpak[i].pak,netbuffer,ackpak[i].length);
           
           *freeack=ackpak[i].acknum;
           
           sendackpacket++; // for stat
           
           return true;
       }
#ifdef PARANOIA
   CONS_Printf("No more free ackpacket\n");
#endif
   if( netbuffer->packettype < CANFAIL )
       I_Error("Connection lost\n");
   return false;
}

// Get a ack to send in the queu of this node
static byte GetAcktosend(int node)
{
    lasttimeacktosend_sent[node]=I_GetTime()+ACKTOSENDTIMEOUT;
    return firstacktosend[node];
}

static void Removeack(int i)
{
    int node=ackpak[i].destinationnode;
    fixed_t trueping=(I_GetTime()-ackpak[i].senttime)<<FRACBITS;
    if( ackpak[i].resent==false )
    {
        // +4 for round
        ping[node] = (ping[node]*7 + trueping)/8;
        varping[node] = (varping[node]*7 + abs(ping[node]-trueping))/8;
		timeout[node] = TIMEOUT(ping[node],varping[node]); // Tails 03-30-2001
    }
    DEBFILE(va("Remove ack %d trueping %d ping %f var %f\n",ackpak[i].acknum,trueping>>FRACBITS,FIXED_TO_FLOAT(ping[node]),FIXED_TO_FLOAT(varping[node])));
    ackpak[i].acknum=0;
}

// we have got a packet proceed the ack request and ack return
static boolean inline Processackpak()
{
   int i;
   boolean goodpacket=true;
   int  node=doomcom->remotenode;

// received a ack return remove the ack in the list
   if(netbuffer->ackreturn && cmpack(remotefirstack[node],netbuffer->ackreturn)<0)
   {
       remotefirstack[node]=netbuffer->ackreturn;
       // search the ackbuffer and free it
       for(i=0;i<MAXACKPACKETS;i++)
           if( ackpak[i].acknum &&
               ackpak[i].destinationnode==node && 
               cmpack(ackpak[i].acknum,netbuffer->ackreturn)<=0 )
               Removeack(i);
   }

// received a packet with ack put it in to queue for send the ack back
   if( netbuffer->ack )
   {
       byte ack=netbuffer->ack;
       getackpacket++;
       if( cmpack(ack,firstacktosend[node])<=0 )
       {
           DEBFILE(va("Discard(1) ack %d (duplicated)\n",ack));
           duppacket++;
           goodpacket=false; // discard packet (duplicat)
       }
       else
       {
           // check if it is not allready in the queue
           for(i =acktosend_tail[node];
               i!=acktosend_head[node];
               i =(i+1)%MAXACKTOSEND    )
               if(acktosend[node][i]==ack)
               {
                   DEBFILE(va("Discard(2) ack %d (duplicated)\n",ack));
                   duppacket++;
                   goodpacket=false; // discard packet (duplicat)
                   break;
               }
           if( goodpacket )
           {
               // is a good packet so increment the acknoledge number, search a "hole" in the queue
               byte nextfirstack=firstacktosend[node]+1;
               if(nextfirstack==0) nextfirstack=1;

               if(ack==nextfirstack)
               { 
                   byte hm1; // head-1
                   boolean change=true;

                   firstacktosend[node]=nextfirstack++;
                   if(nextfirstack==0) nextfirstack=1;
                   hm1=(acktosend_head[node]-1+MAXACKTOSEND)%MAXACKTOSEND;
                   while(change)
                   {
                       change=false;
                       for( i=acktosend_tail[node];i!=acktosend_head[node];i=(i+1)%MAXACKTOSEND)
                           if( cmpack(acktosend[node][i],nextfirstack)<=0 )
                           {
                               if( acktosend[node][i]==nextfirstack )
                               {
                                   firstacktosend[node]=nextfirstack++;
                                   if(nextfirstack==0) nextfirstack=1;
                                   change=true;
                               }
                               if( i==acktosend_tail[node] )
                               {
                                   acktosend[node][acktosend_tail[node]] = 0;
                                   acktosend_tail[node] = (i+1)%MAXACKTOSEND;
                               }
                               else
                                   if( i==hm1 )
                                   {
                                       acktosend[node][hm1] = 0;
                                       acktosend_head[node] = hm1;
                                       hm1=(hm1-1+MAXACKTOSEND)%MAXACKTOSEND; 
                                   }
                                   
                           }
                   }
               }
               else
               { // out of order packet
                 // don't increment firsacktosend, put it in asktosend queue
                   // will be incremented when the nextfirstack come (code above)
                   byte newhead=(acktosend_head[node]+1)%MAXACKTOSEND;
                   DEBFILE(va("out of order packet (%d expected)\n",nextfirstack));
                   if(newhead != acktosend_tail[node])
                   {
                       acktosend[node][acktosend_head[node]]=ack;
                       acktosend_head[node] = newhead;
                   }
                   else // buffer full discard packet, sender will resend it
                       // remark that we can admit the packet but we will not detect the duplication after :(
                   {
                       DEBFILE("no more freeackret\n");
                       goodpacket=false;
                   }
               }
           }
       }
   }
   return goodpacket;
}

// send special packet with only ack on it
extern void SendAcks(int node)
{
    netbuffer->packettype = NOTHING;
    memcpy(netbuffer->u.textcmd,acktosend[node],MAXACKTOSEND);
    HSendPacket(node,false,0,MAXACKTOSEND);
}

static void GotAcks(void)
{
    int i,j;

    for(j=0;j<MAXACKTOSEND;j++)
        if( netbuffer->u.textcmd[j] )
           for(i=0;i<MAXACKPACKETS;i++)
               if( ackpak[i].acknum && 
                   ackpak[i].destinationnode==doomcom->remotenode)
               {
                   if( ackpak[i].acknum==netbuffer->u.textcmd[j])
                       Removeack(i);
                   else
                   // nextacknum is first equal to acknum, then when receiving bigger ack
                   // there is big chance the packet is lost
                   // when resent, nextacknum=nextacknum[node] this will redo the same but with differant value 
                   if( cmpack(ackpak[i].nextacknum,netbuffer->u.textcmd[j])<=0 && ackpak[i].senttime>0)
                       ackpak[i].senttime--; // hurry up
               }
}

// Start Tails 03-30-2001
void ConnectionTimeout( int node )
{
    // send a very special packet to self (hack the reboundstore queu)
    // main code will handle it
    reboundstore[rebound_head].packettype = TIMEOUT;
    reboundstore[rebound_head].ack = 0;
    reboundstore[rebound_head].ackreturn = 0;
    reboundstore[rebound_head].u.textcmd[0] = node;
    reboundsize[rebound_head]=BASEPACKETSIZE+1;
    rebound_head=(rebound_head+1)%MAXREBOUND;

    // do not redo it quickly (if we do not close connection is for a good reason !)
    lasttimepacketreceived[node] = I_GetTime();
}
// End Tails 03-30-2001

// resend the data if needed
extern void AckTicker(void)
{
    int i;

    for(i=0;i<MAXACKPACKETS;i++)
    {
        int node=ackpak[i].destinationnode;
        if(ackpak[i].acknum)
            if(ackpak[i].senttime+((ping[node]+4*varping[node]+FRACUNIT/2)>>FRACBITS)<I_GetTime())
            {
                
                DEBFILE(va("Resend ack %d, %d+%f+4*%f=%d<%d\n",ackpak[i].acknum,
                           ackpak[i].senttime,
                           FIXED_TO_FLOAT(ping[node]),
                           FIXED_TO_FLOAT(varping[node]),
                           ackpak[i].senttime+((ping[node]+4*varping[node])>>FRACBITS),
                           I_GetTime()));
                memcpy(netbuffer,ackpak[i].pak,ackpak[i].length);
                ackpak[i].senttime=I_GetTime();
                ackpak[i].resent=true;
                ackpak[i].nextacknum=nextacknum[node];
                retransmit++; // for stat
                HSendPacket(node,false,ackpak[i].acknum,ackpak[i].length-BASEPACKETSIZE);
            }
    }
   // we haven't send a packet since long time acknoledge packet if needed
//#if 1
//    for(i=1;i<MAXNETNODES;i++)
//        if( lasttimeacktosend_sent[i]<I_GetTime() && firstacktosend[i])
//            SendAcks(i);
//#endif
// Start Tails 03-30-2001
    for(i=1;i<MAXNETNODES;i++)
    {
        // this is something like node open flag
        if( firstacktosend[i] )
        {
            // we haven't sent a packet since long time acknoledge packet if needed
            if( lasttimeacktosend_sent[i] + ACKTOSENDTIMEOUT < I_GetTime() )
                SendAcks(i);

//            if( (nodes[i].flags & CLOSE) == 0 && 
              if(lasttimepacketreceived[i] + CONNECTIONTIMEOUT < I_GetTime() )
					D_FreeNodeNum(i);
//				  ExecuteExitCmd(NULL,i); // Boot dat baby, yeah!

				  
//	           ConnectionTimeout( i );
//			  if(lasttimepacketreceived[i] + CONNECTIONTIMEOUT >= I_GetTime() )
//				  ExecuteExitCmd(NULL,i);
        }
    }
// End Tails 03-30-2001
}


// remove last packet received ack before the resend the ackret
// (the higer layer don't have room, or something else ....)
extern void UnAcknowledgPacket(int node)
{
    int hm1=(acktosend_head[node]-1+MAXACKTOSEND)%MAXACKTOSEND;
    DEBFILE(va("UnAcknowledg node %d\n",node));
    if(!node)
        return;
    if( acktosend[node][hm1] == netbuffer->ack )
    {
        acktosend[node][hm1] = 0;
        acktosend_head[node] = hm1;
    }
    else
    if( firstacktosend[node] == netbuffer->ack )
    {
        firstacktosend[node]--;
        if( firstacktosend[node]==0 )
            firstacktosend[node]--;
    }
    else
    {
        while (firstacktosend[node]!=netbuffer->ack)
        {
            acktosend_tail[node] = (acktosend_tail[node]-1+MAXACKTOSEND)%MAXACKTOSEND;
            acktosend[node][acktosend_tail[node]]=firstacktosend[node];

            firstacktosend[node]--;
            if( firstacktosend[node]==0 ) firstacktosend[node]--;
        } 
        firstacktosend[node]++;
        if( firstacktosend[node]==0 ) firstacktosend[node]++;
    }
//    I_Error("can't Removing ackret\n");

}

extern boolean AllAckReceived(void)
{
   int i;

   for(i=0;i<MAXACKPACKETS;i++)
      if(ackpak[i].acknum)
          return false;

   return true;
}

static void InitAck()
{
   int i;

   for(i=0;i<MAXACKPACKETS;i++)
      ackpak[i].acknum=0;

   for(i=0;i<MAXNETNODES;i++)
   {
      acktosend_head[i]=0;
      acktosend_tail[i]=0;
      ping[i]=PINGDEFAULT;
      varping[i]=VARPINGDEFAULT;
      firstacktosend[i]=0;
      nextacknum[i]=1;
      remotefirstack[i]=0;
   }
}

extern void Net_Abort(char packettype)
{
    int i;
    for( i=0;i<MAXACKPACKETS;i++ )
         if( ackpak[i].acknum && 
             (((doomdata_t *)ackpak[i].pak)->packettype==packettype || packettype==-1 ))
             ackpak[i].acknum=0;
}

// -----------------------------------------------------------------
//  end of acknowledge function
// -----------------------------------------------------------------


// remove a node, clear all ack from this node and reset askret
extern void D_FreeNodeNum(int node)
{
    int i;

    // free the asktosend (if not this will cause wrong duplacation detection)
    acktosend_head[node]=0;
    acktosend_tail[node]=0;
              
    for(i=0;i<MAXACKTOSEND;i++)
        acktosend[node][i]=0;

    // no more packet for this node
    for(i=0;i<MAXACKPACKETS;i++)
        if( ackpak[i].acknum && ackpak[i].destinationnode==node)
            ackpak[i].acknum=0;

    // for the next connection on this node
    ping[node]=PINGDEFAULT;
    varping[node]=VARPINGDEFAULT;
    firstacktosend[node]=0;
    remotefirstack[node]=0;
    nextacknum[node]=1;
    AbortSendFiles(node);
    I_NetFreeNodenum(node);
}

//
// Checksum
//
static unsigned NetbufferChecksum (void)
{
    unsigned    c;
    int         i,l;
    unsigned char   *buf;

    c = 0x1234567;

    l = doomcom->datalength - 4;
    buf = (unsigned char*)netbuffer+4;
    for (i=0 ; i<l ; i++,buf++)
        c += (*buf) * (i+1);

    return c;
}

#ifdef DEBUGFILE

static void fprintfstring(char *s,byte len)
{
    int i;
    int mode=0;

    for (i=0 ; i<len ; i++)
       if(s[i]<32)
           if(mode==0) {
               fprintf (debugfile,"[%d",(byte)s[i]);
               mode = 1;
           } else
               fprintf (debugfile,",%d",(byte)s[i]);
       else
       {
           if(mode==1) {
              fprintf (debugfile,"]");
              mode=0;
           }
           fprintf (debugfile,"%c",s[i]);
       }
    if(mode==1) fprintf (debugfile,"]");
    fprintf(debugfile,"\n");
}

static char *packettypename[NUMPACKETTYPE]={
    "NOTHING",
    "SERVERCFG",
    "CLIENTCMD",
    "CLIENTMIS",
    "CLIENT2CMD",
    "CLIENT2MIS",
    "NODEKEEPALIVE",
    "NODEKEEPALIVEMIS",
    "SERVERTICS",
    "SERVERREFUSE",
    "SERVERSHUTDOWN",
    "CLIENTQUIT",
	"TIMEOUT", // Client times out Tails 03-30-2001

    "ASKINFO",
    "SERVERINFO",
    "REQUESTFILE",

    "FILEFRAGMENT",
    "TEXTCMD",
    "TEXTCMD2",
    "CLIENTJOIN",

};

static void DebugPrintpacket(char *header)
{
    fprintf (debugfile,"%-12s (node %d,ack %d,ackret %d,size %d) type(%d) : %s\n"
                      ,header
                      ,doomcom->remotenode
                      ,netbuffer->ack
                      ,netbuffer->ackreturn
                      ,doomcom->datalength
                      ,netbuffer->packettype,packettypename[netbuffer->packettype]);

    switch(netbuffer->packettype)
    {
       case CLIENTJOIN:
           fprintf(debugfile
                  ,"    number %d mode %d\n"
                  ,netbuffer->u.clientcfg.localplayers
                  ,netbuffer->u.clientcfg.mode);
           break;
       case SERVERTICS:
           fprintf(debugfile
                  ,"    firsttic %d ply %d tics %d ntxtcmd %d\n    "
                  ,ExpandTics (netbuffer->u.serverpak.starttic)
                  ,netbuffer->u.serverpak.numplayers
                  ,netbuffer->u.serverpak.numtics
                  ,(int)(&((char *)netbuffer)[doomcom->datalength] - (char *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numplayers*netbuffer->u.serverpak.numtics]));
           fprintfstring((char *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numplayers*netbuffer->u.serverpak.numtics]
                      ,&((char *)netbuffer)[doomcom->datalength] - (char *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numplayers*netbuffer->u.serverpak.numtics]);
           break;
       case CLIENTCMD:
       case CLIENT2CMD:
       case CLIENTMIS:
       case CLIENT2MIS:
       case NODEKEEPALIVE:
       case NODEKEEPALIVEMIS:
           fprintf(debugfile
                  ,"    tic %4d resendfrom %d localtic %d\n"
                  ,ExpandTics (netbuffer->u.clientpak.client_tic)
                  ,ExpandTics (netbuffer->u.clientpak.resendfrom)
                  ,0 /*netbuffer->u.clientpak.cmd.localtic*/);
           break;
       case TEXTCMD:
       case TEXTCMD2:
           fprintf(debugfile
                  ,"    length %d\n    "
                  ,*(unsigned char*)netbuffer->u.textcmd);
           fprintfstring(netbuffer->u.textcmd+1,netbuffer->u.textcmd[0]);
           break;
       case SERVERCFG:
           fprintf(debugfile
                  ,"    playermask %x players %d clientnode %d serverplayer %d gametic %li gamestate %d\n"
                  ,(unsigned int)netbuffer->u.servercfg.playerdetected
                  ,netbuffer->u.servercfg.totalplayernum
                  ,netbuffer->u.servercfg.clientnode
                  ,netbuffer->u.servercfg.serverplayer
                  ,netbuffer->u.servercfg.gametic
                  ,netbuffer->u.servercfg.gamestate);
           break;
       case SERVERINFO :
           fprintf(debugfile
                  ,"    player %i/%i, map %s\n"
                  ,netbuffer->u.serverinfo.numberofplayer
                  ,netbuffer->u.serverinfo.maxplayer
                  ,netbuffer->u.serverinfo.mapname);
           break;
       case SERVERREFUSE :
           fprintf(debugfile
                  ,"    reason %s\n"
                  ,netbuffer->u.serverrefuse.reason);
           break;
       case FILEFRAGMENT :
           fprintf(debugfile
                  ,"    fileid %d datasize %d position %li\n"
                  ,netbuffer->u.filetxpak.fileid
                  ,netbuffer->u.filetxpak.size
                  ,netbuffer->u.filetxpak.position);
           break;
       case REQUESTFILE :
       default : // write as a raw packet
           fprintfstring(netbuffer->u.textcmd,(char *)netbuffer+doomcom->datalength-(char *)netbuffer->u.textcmd);
           break;

    }
}
#endif

//
// HSendPacket
//
extern boolean HSendPacket(int   node,boolean reliable ,byte acknum,int packetlength)
{

    doomcom->datalength = packetlength+BASEPACKETSIZE;
    if (!node)
    {
        if((rebound_head+1)%MAXREBOUND==rebound_tail)
        {
#ifdef PARANOIA
            CONS_Printf("No more rebound buf\n");
#endif
            return false;
        }
        memcpy(&reboundstore[rebound_head],netbuffer,doomcom->datalength);
        reboundsize[rebound_head]=doomcom->datalength;
        rebound_head=(rebound_head+1)%MAXREBOUND;
#ifdef DEBUGFILE
        if (debugfile)
        {
            doomcom->remotenode = node;
            DebugPrintpacket("SENDLOCAL");
        }
#endif
        return true;
    }

    if (demoplayback)
        return true;

    if (!netgame)
        I_Error ("Tried to transmit to another node");

    // do this before GetFreeAcknum because this function
    // backup the current paket
    doomcom->remotenode = node;
    if(doomcom->datalength<=0)
    {
        DEBFILE("HSendPacket : nothing to send\n");
#ifdef DEBUGFILE
        if (debugfile)
            DebugPrintpacket("TRISEND");
#endif
        return false;
    }

    if(acknum==0 && node<MAXNETNODES) // can be a broadcast
        netbuffer->ackreturn=GetAcktosend(node);
    if(reliable)
    {
        if( I_NetCanSend && !I_NetCanSend() )
        {
            if( netbuffer->packettype < CANFAIL )
                GetFreeAcknum(&netbuffer->ack,true) ;

            DEBFILE("HSendPacket : Out of bandwidth\n");
            return false;
        }
        else
            if( !GetFreeAcknum(&netbuffer->ack,false) )
            return false;
    }
    else
        netbuffer->ack=acknum;

    netbuffer->checksum=NetbufferChecksum ();
    sendbytes+=(packetheaderlength+doomcom->datalength); // for stat

    // simulate internet :)
//    if(rand()<0x7fff/3)
    {
#ifdef DEBUGFILE
        if (debugfile)
            DebugPrintpacket("SEND");
#endif
        I_NetSend();
    }
    return true;
}

//
// HGetPacket
// Returns false if no packet is waiting
// Check Datalength and checksum
//
extern boolean HGetPacket (void)
{
    // get a packet from my
    if (rebound_tail!=rebound_head)
    {
        memcpy(netbuffer,&reboundstore[rebound_tail],reboundsize[rebound_tail]);
        rebound_tail=(rebound_tail+1)%MAXREBOUND;
        doomcom->remotenode = 0;
#ifdef DEBUGFILE
        if (debugfile)
           DebugPrintpacket("GETLOCAL");
#endif
        return true;
    }

    if (!netgame || demoplayback)
        return false;

    I_NetGet();

    if (doomcom->remotenode == -1)
        return false;

    getbytes+=(packetheaderlength+doomcom->datalength); // for stat

    if (doomcom->remotenode >= MAXNETNODES)
    {
        DEBFILE(va("receive packet from node %d !\n", doomcom->remotenode));
        return false;
    }

    lasttimepacketreceived[doomcom->remotenode] = I_GetTime(); // Tails 03-30-2001

    if (netbuffer->checksum != NetbufferChecksum ())
    {
        DEBFILE("Bad packet checksum\n");
        return false;
    }

#ifdef DEBUGFILE
    if (debugfile)
        DebugPrintpacket("GET");
#endif

    // proceed the ack and ackreturn field
    if(!Processackpak())
        return false;    // discated (duplicated)

    // a packet with just ackreturn
    if( netbuffer->packettype == NOTHING)
    {
        GotAcks();
        return false;
    }

    return true;
}

//
// D_CheckNetGame
// Works out player numbers among the net participants
//
extern void D_CheckNetGame (void)
{
    int p;

    InitAck();
    rebound_tail=0;
    rebound_head=0;

    statstarttic=I_GetTime();

    I_NetGet           = NULL;
    I_NetSend          = NULL;
    I_NetCanSend       = NULL;
    I_NetShutdown      = NULL;
    I_NetFreeNodenum   = NULL;
    I_NetGetServerNode = NULL;

    hardware_MAXPACKETLENGHT = MAXPACKETLENGHT;
    net_bandwidth = 3000;
    // I_InitNetwork sets doomcom and netgame
    // check and initialize the network driver
    I_InitNetwork ();

    if(M_CheckParm ("-bandwidth"))
    {
        if(M_IsNextParm())
        {
            net_bandwidth = atoi(M_GetNextParm());
            if( net_bandwidth<1000 ) 
                net_bandwidth=1000;
            if( net_bandwidth>100000 )  
                hardware_MAXPACKETLENGHT = MAXPACKETLENGHT;
            CONS_Printf("Network bandwidth set to %d\n",net_bandwidth);
        }
        else
            I_Error("usage : -bandwidth <byte_per_sec>");
    }

    software_MAXPACKETLENGHT=hardware_MAXPACKETLENGHT;
    p=M_CheckParm ("-packetsize");
    if(p)
    {
        p=atoi(myargv[p+1]);
        if(p<75)
           p=75;
        if(p>hardware_MAXPACKETLENGHT)
           p=hardware_MAXPACKETLENGHT;
        software_MAXPACKETLENGHT=p;
    }

    if( netgame )
        multiplayer = true;

    if (doomcom->id != DOOMCOM_ID)
        I_Error ("Doomcom buffer invalid!");
    if (doomcom->numnodes>MAXNETNODES)
        I_Error ("To much nodes (%d), max:%d",doomcom->numnodes,MAXNETNODES);

    netbuffer = (doomdata_t *)&doomcom->data;
    ticdup=1;

#ifdef DEBUGFILE
    if (M_CheckParm ("-debugfile"))
    {
        char    filename[20];
        sprintf (filename,"debug%i.txt",doomcom->consoleplayer);
        CONS_Printf ("debug output to: %s\n",filename);
        debugfile = fopen (filename,"w");
    }
#endif

    D_ClientServerInit();
}


extern void D_CloseConnection( void )
{
    netgame = false;
    if( I_NetShutdown )
        I_NetShutdown();
}
