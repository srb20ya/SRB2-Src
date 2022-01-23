// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_clisrv.h,v 1.7 2000/04/30 10:30:10 bpereira Exp $
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
// $Log: d_clisrv.h,v $
// Revision 1.7  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.6  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.5  2000/04/19 10:56:51  hurdler
// commited for exe release and tag only
//
// Revision 1.4  2000/04/16 18:38:06  bpereira
// no message
//
// Revision 1.3  2000/04/06 20:32:26  hurdler
// no message
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      high level networking stuff
//
//-----------------------------------------------------------------------------


#ifndef __D_CLISRV__
#define __D_CLISRV__

#include "d_ticcmd.h"
#include "d_netcmd.h"

//
// Network play related stuff.
// There is a data struct that stores network
//  communication related stuff, and another
//  one that defines the actual packets to
//  be transmitted.
//

// Networking and tick handling related.
#define BACKUPTICS            32
#define DRONE               0x80    // bit set in consoleplayer

#define MAXTEXTCMD           256
//
// Packet structure
//
typedef enum   {
    NOTHING,      // to send a nop through network :)
    SERVERCFG,    // server config used in start game (stay 1 for backward compatibility issue)
                  // this is positive responce to CLIENTJOIN request
    CLIENTCMD,    // ticcmd of the client
    CLIENTMIS,    // same as above with but saying resend from
    CLIENT2CMD,   // 2 cmd in the packed for splitscreen
    CLIENT2MIS,   // same as above with but saying resend from
    NODEKEEPALIVE,    // same but without ticcmd and consistancy
    NODEKEEPALIVEMIS,
    SERVERTICS,   // all cmd for the tic
    SERVERREFUSE, // server refuse joiner (reson incide)
    SERVERSHUTDOWN, // self explain
    CLIENTQUIT,   // client close the connection
	TIMEOUT, // Client times out Tails 03-30-2001

    ASKINFO,      // anyone can ask info to the server
    SERVERINFO,   // send game & server info (gamespy)
    REQUESTFILE,  // client request a file transfer

    CANFAIL,      // this is kind of priority, biger then CANFAIL the HSendPacket(,true,,) can return false
                  // also this packet can't occupate all slotes
    FILEFRAGMENT=CANFAIL, // a part of a file
    TEXTCMD,      // extra text command from the client
    TEXTCMD2,     // extra text command from the client (splitscreen)
    CLIENTJOIN,   // client want to join used in start game
    NUMPACKETTYPE
} packettype_t;

//#pragma pack(1)

// client to server packet
typedef struct {
   byte        client_tic;
   byte        resendfrom;
   short       consistancy;
   ticcmd_t    cmd;
} clientcmd_pak;

// splitscreen packet
// WARNING : must have the same format of clientcmd_pak, for more easy use
typedef struct {
   byte        client_tic;
   byte        resendfrom;
   short       consistancy;
   ticcmd_t    cmd;
   ticcmd_t    cmd2;
} client2cmd_pak;

// Server to client packet
// this packet is too large !!!!!!!!!
typedef struct {
   byte        starttic;
   byte        numtics;
   byte        numplayers;
   ticcmd_t    cmds[45]; // normaly [BACKUPTIC][MAXPLAYERS] but too large
//   char        textcmds[BACKUPTICS][MAXTEXTCMD];
} servertics_pak;

typedef struct {
   byte        version;    // exe from differant version don't work
   ULONG       subversion; // contain build version and maybe crc

   // server lunch stuffs
   byte        serverplayer;
   byte        totalplayernum;
   ULONG       gametic;
   byte        clientnode;
   byte        gamestate;
   
   ULONG       playerdetected; // playeringame vector in bit field
} serverconfig_pak;

typedef struct {
   byte        version;    // exe from differant version don't work
   ULONG       subversion; // contain build version and maybe crc
   byte        localplayers;
   byte        mode;
} clientconfig_pak;

typedef struct {
   char        fileid;
   ULONG       position;
   USHORT      size;
   byte        data[100];  // size is variable using hardare_MAXPACKETLENGTH
} filetx_pak;

typedef struct {
    byte       version;
    ULONG      subversion;
    byte       numberofplayer;
    byte       maxplayer;
    byte       deathmatch;
	byte	   gametype; // Tails 03-13-2001
	byte	   autoctf; // Tails 07-22-2001
    ULONG      time;
    float      load;        // unused for the moment
    char       mapname[8];
    byte       fileneedednum;
    byte       fileneeded[300];   // will fileed with writexxx (byteptr.h)
} serverinfo_pak;

typedef struct {
   byte        version;
   ULONG       time;          // used for ping evaluation
} askinfo_pak;

typedef struct {
    char       reason[255];
} serverrefuse_pak;


//
// Network packet data.
//
typedef struct
{                
    unsigned   checksum;
    byte       ack;           // if not null the node ask a acknolegement
                              // the receiver must to resend the ack
    byte       ackreturn;     // the return of the ack number

    byte       packettype;
    union  {   clientcmd_pak     clientpak;
               client2cmd_pak    client2pak;
               servertics_pak    serverpak;
               serverconfig_pak  servercfg;
               byte              textcmd[MAXTEXTCMD+1];
               filetx_pak        filetxpak;
               clientconfig_pak  clientcfg;
               serverinfo_pak    serverinfo;
               serverrefuse_pak  serverrefuse;
               askinfo_pak       askinfo;
           } u;

} doomdata_t;

//#pragma pack()

// points inside doomcom
extern  doomdata_t*   netbuffer;        

extern consvar_t cv_playdemospeed;

#define BASEPACKETSIZE ((int)&( ((doomdata_t *)0)->u))
#define FILETXHEADER   ((int)((filetx_pak *)0)->data)

extern boolean   dedicated;
extern boolean   server;
extern USHORT    software_MAXPACKETLENGHT;
extern boolean   acceptnewnode;
extern char      servernode;
extern boolean   drone;

extern consvar_t cv_allownewplayer;
extern consvar_t cv_maxplayers;
extern boolean   notimecheck; // check file time through network ?  

// used in d_net, the only depandence
int    ExpandTics (int low);
void   D_ClientServerInit (void);

// initialise the other field
void   RegisterNetXCmd(netxcmd_t id,void (*cmd_f) (char **p,int playernum));
void   SendNetXCmd(byte id,void *param,int nparam);
void   SendNetXCmd2(byte id,void *param,int nparam); // splitsreen player

// Create any new ticcmds and broadcast to other players.
void   NetUpdate (void);
void   D_PredictPlayerPosition(void);

boolean SV_AddWaitingPlayers(void);
void    SV_StartSinglePlayerServer(void);
boolean SV_SpawnServer( void );
void    SV_SpawnPlayer(int playernum,mobj_t *mobj); 
void    SV_StopServer( void );
void    SV_ResetServer( void );

void CL_AddSplitscreenPlayer( void );
void CL_RemoveSplitscreenPlayer( void );


// Broadcasts special packets to other players
//  to notify of game exit
void   D_QuitNetGame (void);

//? how many ticks to run?
void   TryRunTics (int realtic);

// extra data for lmps
boolean AddLmpExtradata(byte **demo_p,int playernum);
void   ReadLmpExtraData(byte **demo_pointer,int playernum);

// translate a playername in a player number return -1 if not found and
// print a error message in the console
int    nametonum(char *name);

#endif
