// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_clisrv.c,v 1.11 2000/08/03 17:57:41 bpereira Exp $
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
// $Log: d_clisrv.c,v $
// Revision 1.11  2000/08/03 17:57:41  bpereira
// no message
//
// Revision 1.10  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.9  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.8  2000/04/16 18:38:06  bpereira
// no message
//
// Revision 1.7  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.5  2000/03/08 17:02:42  hurdler
// fix the joiningame problem under Linux
//
// Revision 1.4  2000/03/06 16:51:08  hurdler
// hack for OpenGL / Open Entry problem
//
// Revision 1.3  2000/02/27 16:30:28  hurdler
// dead player bug fix + add allowmlook <yes|no>
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      DOOM Network game communication and protocol,
//      all OS independend parts.
//
//-----------------------------------------------------------------------------


#include <time.h>
#ifdef __DJGPP__
#include <unistd.h>
#endif

#include "doomdef.h"
#include "command.h"
#include "i_net.h"
#include "i_system.h"
#include "i_video.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "d_main.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "keys.h"
#include "doomstat.h"
#include "m_argv.h"
#include "m_menu.h"
#include "console.h"
#include "d_netfil.h"
#include "byteptr.h"

#include "p_saveg.h"
#include "p_setup.h"
#include "z_zone.h"
#include "p_tick.h"
#include "p_local.h"
#include "m_misc.h"
#include "am_map.h"
#include "m_random.h"
#include "d_clisrv.h"

//
// NETWORKING
//
// gametic is the tic about to (or currently being) run
// maketic is the tic that hasn't had control made for it yet
// server:
//   nettics is the tic for eatch node
//   firstticstosend is the lowest value of nettics
// client:
//   neededtic is the tic needed by the client for run the game
//   firstticstosend is used to optimize a condition
// normaly maketic>=gametic>0,

#define PREDICTIONQUEUE         BACKUPTICS
#define PREDICTIONMASK          (PREDICTIONQUEUE-1)
// more precise version number to compare in network
#define SUBVERSION              13

boolean       server;           // true or false but !server=client
char          serverplayer;

// server specific vars
static byte          playernode[MAXPLAYERS];
static ULONG         cl_maketic[MAXNETNODES];
static char          nodetoplayer[MAXNETNODES];
static char          nodetoplayer2[MAXNETNODES]; // say the numplayer for this node if any (splitscreen)
static byte          playerpernode[MAXNETNODES]; // used specialy for scplitscreen
static boolean       nodeingame[MAXNETNODES];  // set false as nodes leave game
static ULONG         nettics[MAXNETNODES];     // what tic the client have received
static ULONG         supposedtics[MAXNETNODES];// nettics prevision for smaller packet
static byte          nodewaiting[MAXNETNODES];
static ULONG         firstticstosend;          // min of the nettics
static short         consistancy[BACKUPTICS];
static ULONG         tictoclear=0;             // optimize d_cleartic
static ULONG         maketic;
boolean       dedicated;                // dedicate server


// client specific
#ifdef CLIENTPREDICTION
ticcmd_t      localcmds[PREDICTIONQUEUE];
ULONG         localgametic;
#else
static ticcmd_t      localcmds;
static ticcmd_t      localcmds2;
#endif
static boolean       cl_packetmissed;
boolean       drone;
// here it is for the secondary local player (splitscreen)
static byte          mynode;        // my address pointofview server


static byte          localtextcmd[MAXTEXTCMD];
static byte          localtextcmd2[MAXTEXTCMD]; // splitscreen
static ULONG         neededtic;
char          servernode;       // the number of the server node

// engine
ticcmd_t      netcmds[BACKUPTICS][MAXPLAYERS];
static byte   textcmds[BACKUPTICS][MAXPLAYERS][MAXTEXTCMD];
boolean       notimecheck; // check file time through network ?

consvar_t cv_playdemospeed  = {"playdemospeed","0",0,CV_Unsigned};

void P_PlayerFlagBurst(player_t* player); // Protos Tails 08-02-2001


// some software don't support largest packet
// (original sersetup, not exactely, but the probabylity of sending a packet
// of 512 octet is like 0.1)
USHORT software_MAXPACKETLENGHT;

int ExpandTics (int low)
{
    int delta;

    delta = low - (maketic&0xff);

    if (delta >= -64 && delta <= 64)
        return (maketic&~0xff) + low;
    if (delta > 64)
        return (maketic&~0xff) - 256 + low;
    if (delta < -64)
        return (maketic&~0xff) + 256 + low;
#ifdef PARANOIA
    I_Error ("ExpandTics: strange value %i at maketic %i",low,maketic);
#endif
    return 0;
}

// -----------------------------------------------------------------
//  Some extra data function for handle textcmd buffer
// -----------------------------------------------------------------

static void (*listnetxcmd[MAXNETXCMD])(char **p,int playernum);

void RegisterNetXCmd(netxcmd_t id,void (*cmd_f) (char **p,int playernum))
{
#ifdef PARANOIA
   if(id>=MAXNETXCMD)
      I_Error("command id %d too big",id);
   if(listnetxcmd[id]!=0)
      I_Error("Command id %d already used",id);
#endif
   listnetxcmd[id]=cmd_f;
}

void SendNetXCmd(byte id,void *param,int nparam)
{
   if(demoplayback)
       return;

   if(localtextcmd[0]+1+nparam>MAXTEXTCMD)
   {
#ifdef PARANOIA
       I_Error("No more place in the buffer for netcmd %d\n",id);
#else
       CONS_Printf("\2Net Command fail\n");
#endif
       return;
   }
   localtextcmd[0]++;
   localtextcmd[localtextcmd[0]]=id;
   if(param && nparam)
   {
       memcpy(&localtextcmd[localtextcmd[0]+1],param,nparam);
       localtextcmd[0]+=nparam;
   }
}

// splitscreen player
void SendNetXCmd2(byte id,void *param,int nparam)
{
   if(demoplayback)
       return;

   if(localtextcmd2[0]+1+nparam>MAXTEXTCMD)
   {
#ifdef PARANOIA
       I_Error("No more place in the buffer for netcmd %d\n",id);
#else
       CONS_Printf("\2Net Command fail\n");
#endif
       return;
   }
   localtextcmd2[0]++;
   localtextcmd2[localtextcmd2[0]]=id;
   if(param && nparam)
   {
       memcpy(&localtextcmd2[localtextcmd2[0]+1],param,nparam);
       localtextcmd2[0]+=nparam;
   }
}


void ExtraDataTicker(void)
{
    int  i,tic;
    byte *curpos,*bufferend;

    tic=gametic % BACKUPTICS;

    for(i=0;i<MAXPLAYERS;i++)
        if((playeringame[i]) || (i==0))
        {
            curpos=(byte *)&(textcmds[tic][i]);
            bufferend=&curpos[curpos[0]+1];
            curpos++;
            while(curpos<bufferend)
            {
                if(*curpos < MAXNETXCMD && listnetxcmd[*curpos])
                {
                    byte id=*curpos;
                    curpos++;
                    DEBFILE(va("executing x_cmd %d ply %d ",id,i));
                    (listnetxcmd[id])((char **)&curpos,i);
                    DEBFILE("done\n");
                }
                else
                    I_Error("Got unknow net command [%d]=%d (max %d)\n"
                           ,curpos-(byte *)&(textcmds[tic][i])
                           ,*curpos,textcmds[tic][i][0]);
            }
        }
}

void D_Cleartic(int tic)
{
    int i;

    for(i=0;i<MAXPLAYERS;i++)
    {
        textcmds[tic%BACKUPTICS][i][0]=0;
        netcmds[tic%BACKUPTICS][i].angleturn = 0; //&= ~TICCMD_RECEIVED;
    }
    DEBFILE(va("clear tic %5d (%2d)\n",tic,tic%BACKUPTICS));
}

// -----------------------------------------------------------------
//  end of extra data function
// -----------------------------------------------------------------

// -----------------------------------------------------------------
//  extra data function for lmps
// -----------------------------------------------------------------

// desciption of extradate byte of LEGACY 1.12 not the same of the 1.20
// 1.20 don't have the extradata bits fields but a byte for eatch command
// see XD_xxx in d_netcmd.h
//
// if extradatabit is set, after the ziped tic you find this :
//
//   type   |  description
// ---------+--------------
//   byte   | size of the extradata
//   byte   | the extradata (xd) bits : see XD_...
//            with this byte you know what parameter folow
// if(xd & XDNAMEANDCOLOR)
//   byte   | color
//   char[MAXPLAYERNAME] | name of the player
// endif
// if(xd & XD_WEAPON_PREF)
//   byte   | original weapon switch : boolean, true if use the old
//          | weapon switch methode
//   char[NUMWEAPONS] | the weapon switch priority
//   byte   | autoaim : true if use the old autoaim system
// endif
boolean AddLmpExtradata(byte **demo_point,int playernum)
{
    int  tic;

    tic=gametic % BACKUPTICS;
    if(textcmds[tic][playernum][0]==0)
        return false;

    memcpy(*demo_point,textcmds[tic][playernum],textcmds[tic][playernum][0]+1);
    *demo_point += textcmds[tic][playernum][0]+1;
    return true;
}

void ReadLmpExtraData(byte **demo_pointer,int playernum)
{
    unsigned char nextra,ex;

    if(!demo_pointer)
    {
        textcmds[gametic%BACKUPTICS][playernum][0]=0;
        return;
    }
    nextra=**demo_pointer;
    if(demoversion==112) // support old demos v1.12
    {
        int    size=0;
        char   *p=*demo_pointer+1; // skip nextra

        textcmds[gametic%BACKUPTICS][playernum][0]=0;

        ex=*p++;
        if(ex & 1)
        {
            size=textcmds[gametic%BACKUPTICS][playernum][0];
            textcmds[gametic%BACKUPTICS][playernum][size+1]=XD_NAMEANDCOLOR;
            memcpy(&textcmds[gametic%BACKUPTICS][playernum][size+2],
                   p,
                   MAXPLAYERNAME+1);
            p+=MAXPLAYERNAME+1;
            textcmds[gametic%BACKUPTICS][playernum][0]+=MAXPLAYERNAME+1+1;
        }
        if(ex & 2)
        {
            size=textcmds[gametic%BACKUPTICS][playernum][0];
            textcmds[gametic%BACKUPTICS][playernum][size+1]=XD_WEAPONPREF;
            memcpy(&textcmds[gametic%BACKUPTICS][playernum][size+2],
                   p,
                   NUMWEAPONS+2);
            p+=NUMWEAPONS+2;
            textcmds[gametic%BACKUPTICS][playernum][0]+=NUMWEAPONS+2+1;
        }
        nextra--;
    }
    else
        memcpy(textcmds[gametic%BACKUPTICS][playernum],*demo_pointer,nextra+1);
    // increment demo pointer
    *demo_pointer +=nextra+1;
}

// -----------------------------------------------------------------
//  end extra data function for lmps
// -----------------------------------------------------------------

static short Consistancy(void);

typedef enum {
   cl_searching,
   cl_downloadfiles,
   cl_askjoin,
   cl_waitjoinresponce,
   cl_downloadsavegame,
   cl_connected
} cl_mode_t;

void GetPackets(void);
void SV_ResetServer( void );

cl_mode_t cl_mode=cl_searching;

//
// SendClJoin
//
// send a special packet for declare how many player in local
// used only in arbitratrenetstart()
static boolean CL_SendJoin()
{
    CONS_Printf("Send join request...\n");
    netbuffer->packettype=CLIENTJOIN;

    if (drone)
        netbuffer->u.clientcfg.localplayers=0;
    else
    if (cv_splitscreen.value)
        netbuffer->u.clientcfg.localplayers=2;
    else
        netbuffer->u.clientcfg.localplayers=1;
    netbuffer->u.clientcfg.version = VERSION;
    netbuffer->u.clientcfg.subversion = SUBVERSION;

    return HSendPacket(servernode,true,0,sizeof(clientconfig_pak));
}


static boolean SV_SendServerConfig(int node)
{
    int   i,playermask=0;

    netbuffer->packettype=SERVERCFG;
    for(i=0;i<MAXPLAYERS;i++)
         if(playeringame[i])
              playermask|=1<<i;

    netbuffer->u.servercfg.version         = VERSION;
    netbuffer->u.servercfg.subversion      = SUBVERSION;

    netbuffer->u.servercfg.serverplayer    = serverplayer;
    netbuffer->u.servercfg.totalplayernum  = doomcom->numplayers;
    netbuffer->u.servercfg.playerdetected  = playermask;
    netbuffer->u.servercfg.gametic         = gametic;
    netbuffer->u.servercfg.clientnode      = node;
    netbuffer->u.servercfg.gamestate       = gamestate;

    return HSendPacket(node,true,0,sizeof(serverconfig_pak));
}

static void SV_SendServerInfo(int node, ULONG time)
{
    byte  *p;

    netbuffer->packettype=SERVERINFO;
    netbuffer->u.serverinfo.version = VERSION;
    netbuffer->u.serverinfo.subversion = SUBVERSION;
    // return back the time value so client can compute their ping
    netbuffer->u.serverinfo.time = time;
    netbuffer->u.serverinfo.numberofplayer = doomcom->numplayers;
    netbuffer->u.serverinfo.maxplayer = cv_maxplayers.value;
    netbuffer->u.serverinfo.load = 0;        // unused for the moment
    netbuffer->u.serverinfo.deathmatch = cv_deathmatch.value;
    netbuffer->u.serverinfo.gametype = cv_gametype.value; // Tails 03-13-2001
    netbuffer->u.serverinfo.autoctf = cv_autoctf.value; // Tails 07-22-2001
    if(gamemapname[0])
        strcpy(netbuffer->u.serverinfo.mapname,gamemapname);
    else
        strcpy(netbuffer->u.serverinfo.mapname,G_BuildMapName(gameepisode,gamemap));

    p=PutFileNeeded();

    HSendPacket(node,false,0, p-((byte *)&netbuffer->u));
}

//#define JOININGAME // Don't use this! It crashes! Tails 11-30-2000
#ifdef JOININGAME
#define SAVEGAMESIZE    (128*1024)

static void SV_SendSaveGame(int node)
{
    int length;
    byte*   savebuffer;

    // first save it in a malloced buffer
    save_p = savebuffer = (byte *)malloc(SAVEGAMESIZE);
    if(!save_p)
    {
        CONS_Printf ("No More free memory for savegame\n");
        return;
    }

    *save_p++ = gameskill;
    *save_p++ = gameepisode;
    *save_p++ = gamemap;
    *save_p++ = leveltime>>16;
    *save_p++ = leveltime>>8;
    *save_p++ = leveltime;
    *save_p++ = P_GetRandIndex();

    P_ArchivePlayers ();
    P_ArchiveWorld ();
    P_ArchiveThinkers ();
    P_ArchiveSpecials ();

    *save_p++ = 0x1d;           // consistancy marker

    length = save_p - savebuffer;
    if (length > SAVEGAMESIZE)
        I_Error ("Savegame buffer overrun");

    // then send it !
    SendRam(node,savebuffer, length,SF_RAM, 0);
}

static const char *tmpsave="$$$.sav";

static void CL_LoadReceivedSavegame(void)
{
    int a,b,c;
    byte*   savebuffer;
    int length = FIL_ReadFile(tmpsave,&savebuffer);


    CONS_Printf("loading savegame length %d\n",length);
    if (!length)
    {
        I_Error ("Can't read savegame sent");
        return;
    }

    G_Downgrade (VERSION);
    save_p = savebuffer;

    gameskill = *save_p++;
    gameepisode = *save_p++;
    gamemap = *save_p++;

    usergame      = true;      // will be set false if a demo
    paused        = false;
    demoplayback  = false;
    automapactive = false;

    // load a base level
    //G_InitNew (gameskill, G_BuildMapName(gameepisode, gamemap),true);
    // get from G_InitNew
    playerdeadview = false;
    DEBFILE(va("loading level ep:%d, map:%d, skill:%d, \"%s\"\n",gameepisode, gamemap, gameskill, gamemapname[0] ? gamemapname:NULL));

    if (!P_SetupLevel (gameepisode, gamemap, gameskill, NULL) )
    {
        CONS_Printf("Can't load the level !!!\n");
        return;
    }

    // get the times
    a = *save_p++;
    b = *save_p++;
    c = *save_p++;
    leveltime = (a<<16) + (b<<8) + c;
    P_SetRandIndex(*save_p++);

    DEBFILE(va("loading savegame start %d\n",save_p-savebuffer));
    // dearchive all the modifications
    P_UnArchivePlayers ();
    DEBFILE(va("loading savegame players %d\n",save_p-savebuffer));
    P_UnArchiveWorld ();
    DEBFILE(va("loading savegame world %d\n",save_p-savebuffer));
    P_UnArchiveThinkers ();
    DEBFILE(va("loading savegame thinkers %d\n",save_p-savebuffer));
    P_UnArchiveSpecials ();

    if(*save_p != 0x1d)
    {
        CONS_Printf ("Bad savegame\n");
#ifdef PARANOIA
        I_Error("BAD SAVEGAME\n");
#endif
    }

    DEBFILE(va("loading savegame special (%d) done\n",save_p-savebuffer));
    // done
    Z_Free (savebuffer);
    unlink(tmpsave);
    consistancy[gametic%BACKUPTICS]=Consistancy();
    CON_ToggleOff ();
}


#endif


// use addaptive send using net_bandwidth and stat.sendbytes
void CL_ConnectToServer()
{
    int     numnodes,nodewaited=doomcom->numnodes,i;
    boolean waitmore;
    ULONG   asksent;
    ULONG   oldtic;

    cl_mode=cl_searching;

    CONS_Printf("Press ESC to abort\n");
    if( servernode<0 || servernode>=MAXNETNODES )
        CONS_Printf("Searching the server...\n");
    else
        CONS_Printf("Contacting the server...\n");
    DEBFILE(va("waiting %d nodes\n",doomcom->numnodes));
    gamestate = wipegamestate = GS_WAITINGPLAYERS;

    numnodes=1;
    oldtic=I_GetTime()-1;
    asksent=-TICRATE/2;
    do {
        switch(cl_mode) {
            case cl_searching :
                // ask the info to the server (askinfo packet)
                if(asksent+TICRATE/2<I_GetTime())
                {
                   netbuffer->packettype = ASKINFO;
                    netbuffer->u.askinfo.version = VERSION;
                    netbuffer->u.askinfo.time = I_GetTime();
                   HSendPacket(servernode,false,0,0);
                   asksent=I_GetTime();
                }
                break;
            case cl_downloadfiles :
                waitmore=false;
                for(i=0;i<fileneedednum;i++)
                    if(fileneeded[i].status==FS_DOWNLOADING || fileneeded[i].status==FS_REQUESTED)
                    {
                        waitmore=true;
                        break;
                    }
                if(waitmore==false)
                    cl_mode=cl_askjoin; //don't break case continue to cljoin request now
                else
                    break; // exit the case
            case cl_askjoin :
                CL_LoadServerFiles();
#ifdef JOININGAME
                // prepare structures to save the file
                // WARNING: this can be useless in case of server not in GS_LEVEL
                // but since the network layer don't provide order packet ...
                CL_PrepareDownloadSaveGame(tmpsave);
#endif
                if( CL_SendJoin() )
                    cl_mode=cl_waitjoinresponce;
                break;
#ifdef JOININGAME
            case cl_downloadsavegame :
                 if( fileneeded[0].status==FS_FOUND )
                 {
                         CL_LoadReceivedSavegame();
                     gamestate = GS_LEVEL;
                     cl_mode=cl_connected;
                 }           //don't break case continue to cl_connected
                 else
                     break;
#endif
            case cl_waitjoinresponce :
            case cl_connected :
                break;
			default: // Tails 11-16-2001
				break; // Tails 11-16-2001
        }

        GetPackets();
        // connection closed by cancel or timeout
        if( !server && !netgame )
        {
            cl_mode = cl_searching;
            return;
        }
        AckTicker();

        // call it only one by tic
        if( oldtic!=I_GetTime() )
        {
            int key;

            I_StartTic();
            key = I_GetKey();
            if (key==KEY_ESCAPE)
                I_Error ("Network game synchronization aborted.");
            if( key=='s' && server) // don't work because fab don't have implemented a proper i_getkey under win32
                doomcom->numnodes=numnodes;

            FiletxTicker();
            oldtic=I_GetTime();

                CON_Drawer ();
                I_FinishUpdate ();              // page flip or blit buffer
        }

        if(server)
        {
            numnodes=0;
            for(i=0;i<MAXNETNODES;i++)
                if(nodeingame[i]) numnodes++;

        }
    }  while (!( (cl_mode == cl_connected) &&
                 ( (!server) || (server && (nodewaited<=numnodes)) )));

    DEBFILE(va("Synchronisation Finished\n"));

    consoleplayer&= ~DRONE;
    displayplayer = consoleplayer;
}

void Command_connect(void)
{
    if( COM_Argc()!=2 )
    {
        CONS_Printf ("connect <serveraddress> : connect to a server\n"
                     "connect ANY : connect to the first lan server found\n"
                     "connect SELF: connect to self server\n");
        return;
    }
    server = false;
    if( stricmp(COM_Argv(1),"any")==0 )
        servernode = BROADCASTADDR;
    else
    if( stricmp(COM_Argv(1),"self")==0 )
    {
        servernode = 0;
        server = true;
        // should be but...
        //SV_SpawnServer();
    }
    else
        if( I_NetGetServerNode )
            servernode = I_NetGetServerNode(COM_Argv(1));
        else
        {
            CONS_Printf("There is no server identification with this network driver\n");
            return;
        }

    CL_ConnectToServer();
}

void ExecuteExitCmd(char **cp,int playernum)
{
    int i;
    if(!demoplayback)
    {
        if( server )
        {
            int node = playernode[playernum];
            playerpernode[node]--;
            if( playerpernode[node]<=0 ) // Fix green sonics! Tails 08-02-2001
           {
                nodeingame[playernode[playernum]] = false;
                D_FreeNodeNum(playernode[playernum]);
            }
        }
    }
    for(i=0;i<MAXPLAYERS;i++)
    {
        players[i].addfrags += players[i].frags[playernum];
        players[i].frags[playernum] = 0;
        // we should use a reset player but there is not such function
        players[playernum].frags[i] = 0;
    }
	players[playernum].tagit = 0; // Tails 08-02-2001
	if(players[playernum].gotflag)
		P_PlayerFlagBurst(&players[playernum]); // Don't take the flag with you! Tails 08-02-2001
	players[playernum].ctfteam = 0; // Tails 08-04-2001
    players[playernum].addfrags = 0;
    if( players[playernum].mo )
    {
        players[playernum].mo->player = NULL;
        P_RemoveMobj (players[playernum].mo);
    }
    players[playernum].mo = NULL;
    playeringame[playernum] = false;
    while(playeringame[doomcom->numplayers-1]==0 && doomcom->numplayers>1) doomcom->numplayers--;
    CONS_Printf("\2%s left the game\n",player_names[playernum]);
}

void StartTitle (void)
{
    if (demorecording)
        G_CheckDemoStatus ();

    // reset client/server code
    DEBFILE(va("\n-=-=-=-=-=-=-= Client reset =-=-=-=-=-=-=-\n\n"));

    if( servernode>0 && servernode<MAXNETNODES)
    {
        D_FreeNodeNum(servernode);
        nodeingame[(byte)servernode]=false;
    }
    D_CloseConnection();         // netgame=false
    servernode=0;
    server=true;
    doomcom->numnodes=1;
    doomcom->numplayers=1;
    SV_StopServer();
    SV_ResetServer();

    // reset game engine
    D_StartTitle ();
}

void ExecuteQuitCmd(char **cp,int playernum)
{
    M_StartMessage("Server has Shutdown\n\nPress Esc",NULL,true);
    StartTitle();
}

void Command_GetPlayerNum(void)
{
    int i;

    for(i=0;i<MAXPLAYERS;i++)
        if(playeringame[i])
        {
            if(serverplayer==i)
                CONS_Printf("\2num:%2d  node:%2d  %s\n",i,playernode[i],player_names[i]);
            else
                CONS_Printf("num:%2d  node:%2d  %s\n",i,playernode[i],player_names[i]);
        }
}

int nametonum(char *name)
{
    int playernum,i;

    if( strcmp(name,"0")==0 )
        return 0;

    playernum=atoi(name);

    if(playernum)
    {
        if(playeringame[playernum])
            return playernum;
        else 
            return -1;
    }

    for(i=0;i<MAXPLAYERS;i++)
        if(playeringame[i] && stricmp(player_names[i],name)==0)
            return i;
    
    CONS_Printf("there is no player named\"%s\"\n",name);

    return -1;
}

#define KICK_MSG_GO_AWAY     1
#define KICK_MSG_CON_FAIL    2
#define KICK_MSG_PLAYER_QUIT 3
#define KICK_MSG_TIMEOUT     4 // Tails 03-30-2001

void Command_Kick(void)
{
    char  buf[3];

    if (COM_Argc() != 2)
    {
        CONS_Printf ("kick <playername> or <playernum> : kick a player\n");
        return;
    }

    if(server)
    {
        buf[0]=nametonum(COM_Argv(1));
        if(buf[0]==-1)
           return;
        buf[1]=KICK_MSG_GO_AWAY;
        SendNetXCmd(XD_KICK,&buf,2);
    }
    else
        CONS_Printf("You are not the server\n");

}

void Got_KickCmd(char **p,int playernum)
{
    int pnum=READBYTE(*p);
    int msg =READBYTE(*p);

    if( msg!=KICK_MSG_PLAYER_QUIT )
        CONS_Printf("\2%s ",player_names[pnum]);

    switch(msg)
    {
       case KICK_MSG_GO_AWAY:
               CONS_Printf("have been kicked (Go away)\n");
               break;
       case KICK_MSG_CON_FAIL:
               CONS_Printf("have been kicked (Consistancy failure)\n");
               break;
       case KICK_MSG_TIMEOUT: // Tails 03-30-2001
               CONS_Printf("left the game (Connection timeout)\n"); // Tails 03-30-2001
               break; // Tails 03-30-2001
       case KICK_MSG_PLAYER_QUIT:
               break;
    }
    if( pnum==consoleplayer )
    {
         StartTitle();
         M_StartMessage("You have been kicked by the server\n\nPress ESC\n",NULL,true);
    }
    else
         ExecuteExitCmd(NULL,pnum);

}

CV_PossibleValue_t maxplayers_cons_t[]={{1,"MIN"},{32,"MAX"},{0,NULL}};

consvar_t cv_allownewplayer = {"sv_allownewplayers","1",0,CV_OnOff}; // Turn this off Tails 11-29-2000
consvar_t cv_maxplayers     = {"sv_maxplayers","32",CV_NETVAR,maxplayers_cons_t,NULL,32};

void Got_AddPlayer(char **p,int playernum);

// called one time at init
void D_ClientServerInit (void)
{
    DEBFILE(va("- - -== Doom LEGACY v%i.%i.%i"VERSIONSTRING" debugfile ==- - -\n",VERSION/100,VERSION%100,SUBVERSION));

//	Remove this Tails 06-06-2001
//    notimecheck = M_CheckParm("-notime");
	notimecheck = true;

    if(M_CheckParm("-left"))
    {
        drone = true;
        viewangleoffset = ANG90;
    }
    if(M_CheckParm("-right"))
    {
        drone = true;
        viewangleoffset = -ANG90;
    }
    dedicated=M_CheckParm("-dedicated")!=0;

        RegisterNetXCmd(XD_EXIT,ExecuteExitCmd);
        RegisterNetXCmd(XD_QUIT,ExecuteQuitCmd);
        COM_AddCommand("getplayernum",Command_GetPlayerNum);
        COM_AddCommand("kick",Command_Kick);
    COM_AddCommand("connect",Command_connect);

        RegisterNetXCmd(XD_KICK,Got_KickCmd);
        RegisterNetXCmd(XD_ADDPLAYER,Got_AddPlayer);
        CV_RegisterVar (&cv_allownewplayer);
        CV_RegisterVar (&cv_maxplayers);

    gametic = 0;

    // do not send anything before the real begin
    SV_StopServer();

    SV_ResetServer();
}

void SV_ResetServer( void )
{
    int    i;

    // +1 because this command will be executed in com_executebuffer in
    // tryruntic so gametic will be incremented, anyway maketic > gametic 
    // is not a issue
    maketic=gametic+1;
    neededtic=maketic;
#ifdef CLIENTPREDICTION
    localgametic = gametic;
#endif
    tictoclear=maketic+BACKUPTICS;

    for (i=0 ; i<MAXNETNODES ; i++)
    {
        nodeingame[i] = false;
        nodetoplayer[i]=-1;
        nodetoplayer2[i]=-1;
        nettics[i]=gametic;
        supposedtics[i]=gametic;
        cl_maketic[i]=maketic;
        nodewaiting[i]=0;
        playerpernode[i]=0;
    }
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        playeringame[i]=false;
        playernode[i]=-1;
    }

    mynode=0;
    cl_packetmissed=false;
    viewangleoffset=0;

    if( dedicated )
    {
        dedicated=true;
        nodeingame[0]=true;
        serverplayer=-1;
    }
    else
        serverplayer=consoleplayer;

    if(server)
        servernode=0;

    doomcom->numplayers=0;

    DEBFILE(va("\n-=-=-=-=-=-=-= Server Reset =-=-=-=-=-=-=-\n\n"));
}

//
// D_QuitNetGame
// Called before quitting to leave a net game
// without hanging the other players
//
void D_QuitNetGame (void)
{
    ULONG timout=I_GetTime()+5*TICRATE;
    ULONG tictac=I_GetTime();

    if (!netgame || demoplayback)
        return;

    DEBFILE("===========================================================================\n"
            "                  Quitting Game, closing connection\n"
            "===========================================================================\n");

    // abort send/receive of files
    CloseNetFile();

    if(server)
    {
        int i;

        netbuffer->packettype=SERVERSHUTDOWN;
        for(i=0;i<MAXNETNODES;i++)
            if( nodeingame[i] )
                HSendPacket(i,true,0,0);
    }
    else
    if(servernode>0 && servernode<MAXNETNODES && nodeingame[(byte)servernode]!=0)
    {
        netbuffer->packettype=CLIENTQUIT;
        HSendPacket(servernode,true,0,0);
    }

    // wait the ackreturn with timout of 5 Sec
    HGetPacket();
    while(timout>I_GetTime() && !AllAckReceived())
    {
        while(tictac==I_GetTime()) ;
        tictac=I_GetTime();
        HGetPacket();
        AckTicker();
    }

    if (I_NetShutdown)
        I_NetShutdown();

    DEBFILE(va("===========================================================================\n"
               "                         Log finish\n"
               "===========================================================================\n"));
#ifdef DEBUGFILE
    if (debugfile)
        fclose (debugfile);
#endif
}

// add a node to the game (player will follow at map change or at savegame....)
void SV_AddNode(int node)
{
    nettics[node]       = gametic;
    supposedtics[node]  = gametic;
    cl_maketic[node]    = maketic;
    // little hack because the server connect to itself and put
    // nodeingame when connected not here
    if(node)
       nodeingame[node]=true;
}

// Xcmd XD_ADDPLAYER
void Got_AddPlayer(char **p,int playernum)
{
    int node=READBYTE(*p);
    int newplayernum=READBYTE(*p);
    boolean splitscreenplayer=newplayernum&0x80;
    static ULONG sendconfigtic=0xffffffff;

    newplayernum&=~0x80;

    playeringame[newplayernum]=true;
    players[newplayernum].playerstate = PST_REBORN;
    if( newplayernum+1>doomcom->numplayers )
        doomcom->numplayers=newplayernum+1;
    CONS_Printf("Player %d is in the game (node %d)\n",newplayernum,node);

    // the server is creating my player
    if(node==mynode)
    {
        playernode[newplayernum]=0;  // for information only
        if(!splitscreenplayer)
        {
            consoleplayer=newplayernum;
            displayplayer=newplayernum;
            secondarydisplayplayer=newplayernum;
            DEBFILE("spawning my\n");
        }
        else
        {
            secondarydisplayplayer=newplayernum;
            DEBFILE("spawning my brother\n");
        }
    }

    // the new player send there config
    // and the old player send there config to the new one
    // WARNING : this can cause a bottleneck in the txtcmd
    //           this can also produse consistancy failure if packet get lost
    //           because everibody know the actualconfig except the joiner
    //    TODO : fixthis

    //  don't send more than once the config par tic (more than one player join)
    if( sendconfigtic!=gametic )
    {
        sendconfigtic=gametic;
        D_SendPlayerConfig();
    }
}

boolean SV_AddWaitingPlayers(void)
{
    int  node,n;
    int  newplayer=false;
    byte buf[2],newplayernum;

    newplayernum=0;
    for(node=0;node<MAXNETNODES;node++)
    {
        // splitscreen can allow 2 player in one node
        for(;nodewaiting[node]>0;nodewaiting[node]--)
        {
            newplayer=true;

            // search for a free playernum
            // we can't use playeringame since it is not updated here
            //while(newplayernum<MAXPLAYERS && playeringame[newplayernum])
            //    newplayernum++;
            for ( ;newplayernum<MAXPLAYERS;newplayernum++)
            {
                for(n=0;n<MAXNETNODES;n++)
                    if( nodetoplayer[n]  == newplayernum ||
                        nodetoplayer2[n] == newplayernum )
                        break;
                if( n == MAXNETNODES )
                    break;
            }
            
#ifdef PARANOIA
            // should never happens since we check the playernum before accept 
            // the join
            if(newplayernum==MAXPLAYERS)
                I_Error("SV_AddWaitingPlayers : j=MAXPLAYERS\n");
#endif
            playernode[newplayernum]=node;

            buf[0]=node;
            buf[1]=newplayernum;
            if( playerpernode[node]<1 )
                nodetoplayer[node]  = newplayernum;
            else
            {
                nodetoplayer2[node] = newplayernum;
                buf[1]|=0x80;
            }
            playerpernode[node]++;

            SendNetXCmd(XD_ADDPLAYER,&buf,2);
            if( doomcom->numplayers==0 )
                doomcom->numplayers++;  //we must send the change to other players
            
            DEBFILE(va("Server added player %d node %d\n", newplayernum, node));
            // use the next free slote (we can't put playeringame[j]=true here)            
            newplayernum++; 
        }
    }

    if( newplayer )
        CV_SendModifiedNetVars();

    return newplayer;
}

void CL_AddSplitscreenPlayer( void )
{
    if( cl_mode == cl_connected )
        CL_SendJoin();
}

void CL_RemoveSplitscreenPlayer( void )
{
    char  buf[2];
    
    if( cl_mode != cl_connected )
        return;

    buf[0]=secondarydisplayplayer;
    buf[1]=KICK_MSG_PLAYER_QUIT;
    SendNetXCmd(XD_KICK,&buf,2);
}

boolean SV_SpawnServer( void )
{
    if( demoplayback )
        G_StopDemo(); // reset engine parameter

    // mmmm, very difficult to know if a server is allready runing
    if( gamestate == GS_DEMOSCREEN  || 
        gamestate == GS_NULL        || 
        gamestate == GS_WAITINGPLAYERS )
    {
        CONS_Printf("Starting Server....\n");
        SV_ResetServer();

        // server just connect to itself
        if( !dedicated )
            CL_ConnectToServer();
    }

    return SV_AddWaitingPlayers();
}

void SV_StopServer( void )
{
    int i;

    gamestate = wipegamestate = GS_NULL;

    localtextcmd[0]=0;
    localtextcmd2[0]=0;

    for(i=0;i<BACKUPTICS;i++)
        D_Cleartic(i);

    consoleplayer=0;
    cl_mode = cl_searching;
    maketic=gametic+1;
    neededtic=maketic;
    
}

// called at singleplayer start and stopdemo
void SV_StartSinglePlayerServer(void)
{
    server        = true;
    netgame       = false;
    multiplayer   = false;

    // no more tic the game with this settings !
    SV_StopServer();

    if( cv_splitscreen.value )
        multiplayer    = true;
}

static void D_SendRefuse(int node,char *fmt,...)
{
    va_list     argptr;

    va_start (argptr,fmt);
    vsprintf (netbuffer->u.serverrefuse.reason,fmt,argptr);
    va_end   (argptr);

    netbuffer->packettype=SERVERREFUSE;
    HSendPacket(node,true,0,strlen(netbuffer->u.serverrefuse.reason)+1);
}

// used at txtcmds received to check packetsize bound
static int TotalTextCmdPerTic(int tic)
{
    int i,total=1; // num of textcmds per tic

    tic %= BACKUPTICS;

    for(i=0;i<MAXPLAYERS;i++)
        if( textcmds[tic][i][0] )
            total += (textcmds[tic][i][0] + 2); // size and playernum

    return total;
}

//
// GetPackets
//
// TODO : break this 300 line function to mutliple functions
void GetPackets (void)
{
    int         netconsole;
    ULONG       realend,realstart;
    int         p=maketic%BACKUPTICS;
    byte        *pak,*txtpak,numtxtpak;
    int         node;

    while ( HGetPacket() )
    {
        node=doomcom->remotenode;
        if(netbuffer->packettype == CLIENTJOIN)
        if(server)
        {
            if(    netbuffer->u.clientcfg.version!=VERSION
                || netbuffer->u.clientcfg.subversion!=SUBVERSION)
            {
                D_SendRefuse(node,"Different DOOM versions cannot play a net game! (server version %d.%d build %d)",VERSION/100,VERSION%100,SUBVERSION);
                D_FreeNodeNum(node);
            }
            else
            if(!cv_allownewplayer.value)
            {
                D_SendRefuse(node,"The server is not accepting people for the moment");
                D_FreeNodeNum(node);
            }
            else
            // TODO; compute it using nodewaiting and playeringame
            if(doomcom->numplayers+1>cv_maxplayers.value)
            {
                D_SendRefuse(node,"Maximum of player reached (max:%d)",cv_maxplayers.value);
                D_FreeNodeNum(node);
            }
            else
            {
                boolean newnode=false;
                // client autorised to join
                nodewaiting[node]=netbuffer->u.clientcfg.localplayers-playerpernode[node];
                if(!nodeingame[node])
                {
                    newnode = true;
                    SV_AddNode(node);
                    if(!SV_SendServerConfig(node))
                    {
                        // TODO : fix this !!!
                        CONS_Printf("Internal Error 5 : client lost\n");
                        continue; // the while
                    }
                }
                CONS_Printf("\2new node joined\n");
#ifdef JOININGAME
                if( nodewaiting[node] )
                {
                    if( gamestate == GS_LEVEL && newnode)
                    {
                        SV_SendSaveGame(node);
                        CONS_Printf("send savegame\n");
                    }
                    SV_AddWaitingPlayers();
                }
#endif
// Start Tails 03-30-2001
            }
            continue;
        } // if(netbuffer->packettype == PT_CLIENTJOIN)
        if( netbuffer->packettype == SERVERSHUTDOWN && node==servernode && 
            !server && cl_mode != cl_searching)
        {
            M_StartMessage("Server has Shutdown\n\nPress Esc",NULL,true);
			StartTitle();
            continue;
        }
        if( netbuffer->packettype == TIMEOUT && node==servernode && 
            !server && cl_mode != cl_searching)
        {
            M_StartMessage("Server Timeout\n\nPress Esc",NULL,true);
            StartTitle();
            continue;
        }

/*        if( netbuffer->packettype == SERVERINFO )
        {
            // compute ping in ms
            netbuffer->u.serverinfo.time = (I_GetTime()-netbuffer->u.serverinfo.time)*1000/TICRATE; 
//            netbuffer->u.serverinfo.servername[MAXSERVERNAME-1]=0;

//            SL_InsertServer( &netbuffer->u.serverinfo, node);
            continue;
        }*/
// End Tails 03-30-2001
/*            }
            continue;
        }
        else
        {
            DEBFILE("Received CLIENTJOIN in !server\n");
            continue;
        }
*/
        if(!nodeingame[node])
        {
            DEBFILE(va("Received packet from unknow host %d\n",node));

            // anyone trying to join !
            switch(netbuffer->packettype) {
                case ASKINFO:
                    if(server)
                    {
                        SV_SendServerInfo(node, netbuffer->u.askinfo.time);
                        D_FreeNodeNum(node);
                    }
                    break;
                case SERVERINFO : // responce of client ask request
                    if( cl_mode!=cl_searching )
                        break;
                    // this can be a responce to our broadcast request
                    if( servernode==-1 || servernode>=MAXNETNODES)
                    {
                        servernode=node;
                        CONS_Printf("Found, ");
                    }
                    Got_FileneededPak(notimecheck);
                    CONS_Printf("Checking files...\n");
                    if( CL_CheckFiles() )
                        cl_mode=cl_askjoin;
                    else
                    {
                        // no problem if can't send packet, we will retry later
                        if( SendRequestFile() )
                            cl_mode=cl_downloadfiles;
                    }
                    break;
                case SERVERREFUSE : // negative responce of client join request
                    if( cl_mode==cl_waitjoinresponce )
                        I_Error("Server refused connection\n\nReason :\n%s" // Tails 04-02-2001
                               ,netbuffer->u.serverrefuse.reason);
                    break;
                case SERVERCFG :    // positive responce of client join request
                {
                    int j;

                    if( cl_mode!=cl_waitjoinresponce )
                        break;

                    if(!server)
                        maketic = gametic = neededtic = netbuffer->u.servercfg.gametic;;

#ifdef CLIENTPREDICTION
                    localgametic=gametic;
#endif
                    nodeingame[(byte)servernode]=true;
                    serverplayer = netbuffer->u.servercfg.serverplayer;
                    doomcom->numplayers = netbuffer->u.servercfg.totalplayernum;
                    mynode = netbuffer->u.servercfg.clientnode;
                    if (serverplayer>=0)
                        playernode[(byte)serverplayer]=servernode;

                    CONS_Printf("Join accepted, wait next map change...\n");
                    DEBFILE(va("Server accept join gametic=%d mynode=%d\n",gametic,mynode));

                    for(j=0;j<MAXPLAYERS;j++)
                        playeringame[j]=(netbuffer->u.servercfg.playerdetected & (1<<j))!=0;

#ifdef JOININGAME
                    if( netbuffer->u.servercfg.gamestate == GS_LEVEL )
                        cl_mode = cl_downloadsavegame;
                    else
#endif
                    cl_mode = cl_connected;

                    break;
                }
                // handled in d_netfil.c
                case FILEFRAGMENT :
                    if( !server )
                        Got_Filetxpak();
                    break;
                case REQUESTFILE :
                    if( server )
                        Got_RequestFilePak(node);
                    break;
				case TIMEOUT: // Tails 03-30-2001
                case CLIENTQUIT:
                    if( server )
                        D_FreeNodeNum(node);
                    break;
                case SERVERSHUTDOWN:
                    if( !server && cl_mode != cl_searching && node==servernode)
                        I_Error("The server has shutdown during connection\n");
                    break;
                default:
                    DEBFILE(va("unknow packet received (%d) from unknow host !\n",netbuffer->packettype));
                    D_FreeNodeNum(node);
                    break; // ignore it


            } // switch
            continue; //while
        }
        netconsole = nodetoplayer[node];
#ifdef PARANOIA
        if(!(netconsole & DRONE) && netconsole>=MAXPLAYERS)
            I_Error("bad table nodetoplayer : node %d player %d"
            ,doomcom->remotenode,netconsole);
#endif


        switch(netbuffer->packettype) {
// -------------------------------------------- SERVER RECEIVE ----------
            case CLIENT2CMD :
            case CLIENTCMD  :
            case CLIENTMIS  :
            case CLIENT2MIS :
            case NODEKEEPALIVE :
            case NODEKEEPALIVEMIS :
                if(!server)
                    break;

                // to save bytes, only the low byte of tic numbers are sent
                // Figure out what the rest of the bytes are
                realstart  = ExpandTics (netbuffer->u.clientpak.client_tic);
                realend = ExpandTics (netbuffer->u.clientpak.resendfrom);

                // update the nettics
                if( nettics[node] < realend )
                    nettics[node] = realend;
                if( (netbuffer->packettype==CLIENTMIS)
                  ||(netbuffer->packettype==CLIENT2MIS)
                  ||(netbuffer->packettype==NODEKEEPALIVEMIS)
                  ||(realend>supposedtics[node]))
                    supposedtics[node] = realend;

                // don't do anything for drones just update there nettics
                if((netconsole & DRONE)!=0 || netconsole==-1 || (netbuffer->packettype==NODEKEEPALIVE) || (netbuffer->packettype==NODEKEEPALIVEMIS))
                     break;

                // check consistancy
                if(realstart<=gametic && realstart>gametic-BACKUPTICS+1 &&
                   consistancy[realstart%BACKUPTICS]!=netbuffer->u.clientpak.consistancy)
                {
#if 1
                    char buf[3];

                    buf[0]=netconsole;
                    buf[1]=KICK_MSG_CON_FAIL;
                    SendNetXCmd(XD_KICK,&buf,2);
                    DEBFILE(va("player %d kicked [%d] %d!=%d\n",netconsole,realstart,consistancy[realstart%BACKUPTICS],netbuffer->u.clientpak.consistancy));
#else
                    CONS_Printf("\2player %d kicked [%d] consistancy failure\n",netconsole,realstart);
#endif
                }

                memcpy(&netcmds[maketic%BACKUPTICS][netconsole]
                      ,&netbuffer->u.clientpak.cmd
                      ,sizeof(ticcmd_t));

                if( netbuffer->packettype==CLIENT2CMD && nodetoplayer2[node]>=0)
                {
                    memcpy(&netcmds[maketic%BACKUPTICS][(byte)nodetoplayer2[node]]
                          ,&netbuffer->u.client2pak.cmd2
                          ,sizeof(ticcmd_t));
                }

                break;
            case TEXTCMD2 : // splitscreen special
                netconsole=nodetoplayer2[node];
            case TEXTCMD :
                if( netconsole<0 || netconsole>=MAXPLAYERS )
                    UnAcknowledgPacket(node);
                else
                {
                    int j;
                    ULONG tic=maketic;

                    j=software_MAXPACKETLENGHT-(netbuffer->u.textcmd[0]+2+BASEPACKETSIZE+doomcom->numplayers*sizeof(ticcmd_t));

                    // search a tic that have enougth space in the ticcmd
                    while((TotalTextCmdPerTic(tic)>j || netbuffer->u.textcmd[0]+textcmds[tic % BACKUPTICS][netconsole][0]>MAXTEXTCMD) && tic<firstticstosend+BACKUPTICS)
                        tic++;

                    if(tic>=firstticstosend+BACKUPTICS)
                    {
                        DEBFILE(va("GetPacket: Too much textcmd (max %d, used %d, mak %d,tosend %d,node %d, player %d)\n",j,TotalTextCmdPerTic(maketic),maketic,firstticstosend,node,netconsole));
                        UnAcknowledgPacket(node);
                        break;
                    }
                    DEBFILE(va("textcmd put in tic %d at position %d (player %d) ftts %d mk %d\n",tic,textcmds[p][netconsole][0]+1,netconsole,firstticstosend,maketic));
                    p=tic % BACKUPTICS;
                    memcpy(&textcmds[p][netconsole][textcmds[p][netconsole][0]+1]
                          ,netbuffer->u.textcmd+1
                          ,netbuffer->u.textcmd[0]);
                    textcmds[p][netconsole][0]+=netbuffer->u.textcmd[0];
                }
                break;
			case TIMEOUT: // Tails 03-30-2001
            case CLIENTQUIT:
                // nodeingame will be put false in the execution of kick command
                // this permit to send some packets to the quiting client to have there ack back
                nodewaiting[node]= 0;
                if(netconsole!=-1 && playeringame[netconsole])
                {
                    char  buf[2];
                    buf[0]=netconsole;
                    if( netbuffer->packettype == TIMEOUT )
                        buf[1]=KICK_MSG_TIMEOUT;
                    else
                        buf[1]=KICK_MSG_PLAYER_QUIT;
                    SendNetXCmd(XD_KICK,&buf,2);
                    nodetoplayer[node]=-1;
                    if(nodetoplayer2[node]!=-1 && nodetoplayer2[node]>=0 && playeringame[(byte)nodetoplayer2[node]])
                    {
                        buf[0]=nodetoplayer2[node];
                        buf[1]=KICK_MSG_PLAYER_QUIT;
                        SendNetXCmd(XD_KICK,&buf,2);
                        nodetoplayer2[node]=-1;
                    }
                }
                else
                {
                    // try to Send ack back (two army problem)
                    netbuffer->packettype=NOTHING;
                    HSendPacket(node,false,0,0);
                    D_FreeNodeNum(node);
                    nodeingame[node]=false;
                }
                break;
// -------------------------------------------- CLIENT RECEIVE ----------
            case SERVERSHUTDOWN:
                if(node!=servernode) // it it my server ?
                    break;
                // Send ack back
                SendAcks(node);
                if( cl_mode != cl_connected )
                {
                    if( servernode>0 && servernode<MAXNETNODES )
                        nodeingame[(byte)servernode]=0;
                    I_Error("The server has shutdown during connection\n");
                }
                else
                ExecuteQuitCmd(NULL,0);
                break;
            case SERVERTICS :
                realstart  = ExpandTics (netbuffer->u.serverpak.starttic);
                realend    = realstart+netbuffer->u.serverpak.numtics;

                txtpak=(char *)&netbuffer->u.serverpak.cmds[netbuffer->u.serverpak.numplayers*netbuffer->u.serverpak.numtics];

                if( realend>gametic+BACKUPTICS )
                    realend=gametic+BACKUPTICS;
                cl_packetmissed=realstart>neededtic;

                if(realstart<=neededtic && realend>neededtic)
                {
                    ULONG i,j;
                    pak=(char *)&netbuffer->u.serverpak.cmds;

                    for(i=realstart;i<realend;i++)
                    {
                    // clear first
                        D_Cleartic(i);

                    // copy the tics
                        memcpy(netcmds[i%BACKUPTICS]
                              ,pak
                              ,netbuffer->u.serverpak.numplayers*sizeof(ticcmd_t));
                        pak+=netbuffer->u.serverpak.numplayers*sizeof(ticcmd_t);

                    // copy the textcmds
                        numtxtpak=*txtpak++;
                        for(j=0;j<numtxtpak;j++)
                        {
                            int k=*txtpak++; // playernum

                            memcpy(textcmds[i%BACKUPTICS][k]
                                  ,txtpak
                                  ,txtpak[0]+1);
                            txtpak+=txtpak[0]+1;
                        }
                    }

                    neededtic=realend;
                }
                else
                    DEBFILE(va("frame not in bound : %u\n", neededtic));
                break;
            case CLIENTJOIN :
                   DEBFILE("got player config during game\n");
                   break;
            case SERVERCFG :
                   break;
            case FILEFRAGMENT :
                if( !server )
                    Got_Filetxpak();
                break;
            default:
                DEBFILE(va("UNKNOW PACKET TYPE RECEIVED %f from host %d\n",netbuffer->packettype,node));
        } // end switch
    } // end while
}

//
// NetUpdate
// Builds ticcmds for console player,
// sends out a packet
//
// no more use random generator, because at very first tic isn't yet synchronized
static short Consistancy(void)
{
    short ret=0;
    int   i;

    DEBFILE(va("TIC %d ",gametic));
    for(i=0;i<MAXPLAYERS;i++)
        if( playeringame[i] && players[i].mo)
        {
            DEBFILE(va("p[%d].x = %f ",i,FIXED_TO_FLOAT(players[i].mo->x)));
            ret+=players[i].mo->x;
        }

    DEBFILE(va("pos = %d, rnd %d\n",ret,P_GetRandIndex()));
    ret+=P_GetRandIndex();

    return ret;
}

// send the client packet to the server
static void CL_SendClientCmd (void)
{
/* oops can do that until i have implemented a real dead reckoning
    static ticcmd_t lastcmdssent;
    static int      lastsenttime=-TICRATE;

    if( memcmp(&localcmds,&lastcmdssent,sizeof(ticcmd_t))!=0 || lastsenttime+TICRATE/3<I_GetTime())
    {
        lastsenttime=I_GetTime();
*/
    int packetsize=0;

    netbuffer->packettype=CLIENTCMD;

    if (cl_packetmissed)
        netbuffer->packettype++;
    netbuffer->u.clientpak.resendfrom = neededtic;
    netbuffer->u.clientpak.client_tic = gametic;

    if(gamestate==GS_WAITINGPLAYERS)
    {
        // send NODEKEEPALIVE packet
        netbuffer->packettype+=4;
        packetsize = sizeof(clientcmd_pak)-sizeof(ticcmd_t)-sizeof(short);
        HSendPacket (servernode,false,0,packetsize);
    }
    else
    if( gamestate!=GS_NULL )
    {
#ifdef CLIENTPREDICTION
        DEBFILE(va("clientsend tic %d\n",localgametic-1));
        memcpy(&netbuffer->u.clientpak.cmd, &localcmds[(localgametic-1)& PREDICTIONMASK], sizeof(ticcmd_t));
#else
        memcpy(&netbuffer->u.clientpak.cmd, &localcmds, sizeof(ticcmd_t));
#endif
        netbuffer->u.clientpak.consistancy = consistancy[gametic%BACKUPTICS];

        // send a special packet with 2 cmd for splitscreen
        if (cv_splitscreen.value)
        {
            netbuffer->packettype+=2;
            memcpy(&netbuffer->u.client2pak.cmd2, &localcmds2, sizeof(ticcmd_t));
            packetsize = sizeof(client2cmd_pak);
        }
        else
            packetsize = sizeof(clientcmd_pak);
        HSendPacket (servernode,false,0,packetsize);
    }

    if( cl_mode == cl_connected )
    {
        // send extra data if needed
        if (localtextcmd[0])
        {
            netbuffer->packettype=TEXTCMD;
            memcpy(netbuffer->u.textcmd,localtextcmd,localtextcmd[0]+1);
            // all extra data have been sended
            if( HSendPacket(servernode,true,0,localtextcmd[0]+1) ) // send can fail for some reasons...
                localtextcmd[0]=0;
        }
        
        // send extra data if needed for player 2 (splitscreen)
        if (localtextcmd2[0])
        {
            netbuffer->packettype=TEXTCMD2;
            memcpy(netbuffer->u.textcmd,localtextcmd2,localtextcmd2[0]+1);
            // all extra data have been sended
            if( HSendPacket(servernode,true,0,localtextcmd2[0]+1) ) // send can fail for some reasons...
                localtextcmd2[0]=0;
        }
    }

}


// send the server packet
// send tic from firstticstosend to maketic-1
static void SV_SendTics (void)
{
    ULONG realfirsttic,lasttictosend=maketic,i;
    int  j,packsize;
    char *bufpos=(char *)&netbuffer->u.serverpak.cmds;
    char *ntextcmd,*begintext;

    realfirsttic=maketic;
    for(i=0;i<MAXNETNODES;i++)
        if( nodeingame[i] && realfirsttic>supposedtics[i] )
            realfirsttic = supposedtics[i];
    if(realfirsttic==maketic)
    {
        realfirsttic = firstticstosend;
        DEBFILE(va("Nothing to send mak=%u\n",maketic));
    }
    if(realfirsttic<firstticstosend)
        realfirsttic=firstticstosend;

// compute the lenght of the packet and cut it if too large
    packsize=(int)&( ((doomdata_t *)0)->u.serverpak.cmds[0]);
    for(i=realfirsttic;i<lasttictosend;i++)
    {
        packsize+=sizeof(ticcmd_t)*doomcom->numplayers;
        packsize++; // the ntextcmd byte
        for(j=0;j<MAXPLAYERS;j++)
            if( ((j==0) || playeringame[j]) && textcmds[i%BACKUPTICS][j][0] )
                packsize += (textcmds[i%BACKUPTICS][j][0]+2); // the player num, the number of byte
        if(packsize>software_MAXPACKETLENGHT)
        {
            lasttictosend=i-1;
// CONS_Printf("Ajusting packet size to %d (orginal %d)\n",lasttictosend,maketic);
            // too bad : too much player have send extradata and there is too
            //           mutch data in one tic. Too avoid it put the data
            //           on the next tic. Hey it is the server !
            if(i==realfirsttic)
                I_Error("Tic to large can send %d data for %d players\n"
                        "use a largest value for '-packetsize' (max 1400)\n"
                        ,packsize,doomcom->numplayers);
            break;
        }
    }

// Send the tics

    netbuffer->packettype=SERVERTICS;
    netbuffer->u.serverpak.starttic=realfirsttic;
    netbuffer->u.serverpak.numtics=lasttictosend-realfirsttic;
    netbuffer->u.serverpak.numplayers=doomcom->numplayers;

    for(i=realfirsttic;i<lasttictosend;i++)
    {
         memcpy(bufpos
               ,netcmds[i%BACKUPTICS]
               ,doomcom->numplayers*sizeof(ticcmd_t));
         bufpos+=doomcom->numplayers*sizeof(ticcmd_t);
    }

    begintext=bufpos;

// add textcmds
    for(i=realfirsttic;i<lasttictosend;i++)
    {
        ntextcmd=bufpos++;
        *ntextcmd=0;
        for(j=0;j<MAXPLAYERS;j++)
        {
            int size=textcmds[i%BACKUPTICS][j][0];

            if(((j==0) || playeringame[j]) && size)
            {
                (*ntextcmd)++;
                *bufpos++ = j;
                memcpy(bufpos,textcmds[i%BACKUPTICS][j],size+1);
                bufpos += size+1;
            }
        }
    }
    packsize = bufpos - (char *)&(netbuffer->u);

    // node 0 is me !
    supposedtics[0] = lasttictosend;
    // send to  all client but not to me
    for(i=1;i<MAXNETNODES;i++)
       if( nodeingame[i] )
       {
           HSendPacket(i,false,0,packsize);
           supposedtics[i] = lasttictosend-doomcom->extratics;
       }
}

static void inline CopyTic(ticcmd_t *a,ticcmd_t *b)
{
    memcpy(a,b,sizeof(ticcmd_t));
    a->angleturn &=~TICCMD_RECEIVED;
}

//
// TryRunTics
//
static void Local_Maketic(int realtics)
{
    I_StartTic ();       // i_getevent
    D_ProcessEvents ();  // menu responder ???!!!
                         // Cons responder
                         // game responder call :
                         //    HU_responder,St_responder, Am_responder
                         //    F_responder (final)
                         //    and G_MapEventsToControls

    // the server can run more fast than the client
//    if(gametic>=localgametic)
//        localgametic=gametic+1;

    rendergametic=gametic;
#ifndef CLIENTPREDICTION

    // translate inputs (keyboard/mouse/joystick) into game controls
    G_BuildTiccmd (&localcmds,realtics);
    if (cv_splitscreen.value)
        G_BuildTiccmd2(&localcmds2,realtics);

#else
/*
    if(realtics>1)  // do the same of the server
    {
        int i;
        for(i=1;i<realtics;i++)
        {
           CopyTic(&localcmds[localgametic & PREDICTIONMASK]
                  ,&localcmds[(localgametic-1) & PREDICTIONMASK]);
           if(gametic+PREDICTIONQUEUE-2<=localgametic)
               break;
           localgametic++;
        }
    }
*/
    G_BuildTiccmd (&localcmds[localgametic & PREDICTIONMASK],realtics);
    localcmds[localgametic & PREDICTIONMASK].localtic = localgametic;

  // -2 because we send allway the -1
    if(gametic+PREDICTIONQUEUE-2>localgametic)
        localgametic++;
#endif
}

void SV_SpawnPlayer(int playernum,mobj_t *mobj)
{
    // for futur copytic use the good x,y and angle!
    if( server )
    {
#ifdef CLIENTPREDICTION2
        netcmds[maketic%BACKUPTICS][playernum].x=mobj->x;
        netcmds[maketic%BACKUPTICS][playernum].y=mobj->y;
#endif
        netcmds[maketic%BACKUPTICS][playernum].angleturn=(mobj->angle>>16) | TICCMD_RECEIVED;
    }
}

// create missed tic
void SV_Maketic(void)
{
    int j;

    for(j=0;j<MAXNETNODES;j++)
       if(playerpernode[j])
       {
           int player=nodetoplayer[j];
           if((netcmds[maketic%BACKUPTICS][player].angleturn & TICCMD_RECEIVED) == 0)   // we don't received this tic
           {
               int i;

               DEBFILE(va("MISS tic%4u for node %d\n",maketic,j));

               if( devparm )
                   CONS_Printf("\2Client Misstic %d\n",maketic);
               // copy the old tic
               for(i=0;i<playerpernode[j];i++,player=nodetoplayer2[j])
                   CopyTic(&netcmds[maketic%BACKUPTICS][player]
                          ,&netcmds[(maketic-1)%BACKUPTICS][player]);
           }
       }

    // all tic are now proceed make the next
    maketic++;
}

extern  boolean advancedemo;
static  int     load;

void TryRunTics (int realtics)
{
    // the machine have laged but is not so bad
    if(realtics>5)
    {
        if(server)
            realtics=1;
        else
            realtics=5;
    }

    if(singletics)
        realtics = 1;

    NetUpdate();

    if(demoplayback)
    {
        neededtic = gametic + realtics + cv_playdemospeed.value;
        // start a game after a demo
        maketic+=realtics;
        firstticstosend=maketic;
        tictoclear=firstticstosend+BACKUPTICS;
    }

#ifdef DEBUGFILE
    if(realtics==0)
        if(load) load--;
#endif
    GetPackets ();

#ifdef DEBUGFILE
    if (debugfile && (realtics || neededtic>gametic))
    {
        //SoM: 3/30/2000: Need long int in the format string for args 4 & 5.
        //Shut up stupid warning!
        fprintf (debugfile,
                 "------------ Tryruntic : REAL:%i NEED:%li GAME:%li LOAD: %i\n",
                 realtics, neededtic, gametic, load);
        load=100000;
    }
#endif

    if (neededtic > gametic)
    {
        if (advancedemo)
            D_DoAdvanceDemo ();
        else
        // run the count * tics
        while (neededtic > gametic)
        {
            DEBFILE(va("============ Runing tic %u (local %d)\n",gametic, 0/*netcmds[gametic%BACKUPTICS][consoleplayer].localtic*/));

            COM_BufExecute();
            G_Ticker ();
            ExtraDataTicker();
            gametic++;
            // skip paused tic in a demo
            if(demoplayback)
            {   if(paused) neededtic++; }
            else
                consistancy[gametic%BACKUPTICS]=Consistancy();
        }
        //if(gametic>maketic) maketic = gametic;
    }
}


#ifdef CLIENTPREDICTION
ULONG oldgametic=0;
ULONG oldlocalgametic=0;

// make sure (gamestate==GS_LEVEL && !server) before calling this function
void D_PredictPlayerPosition(void)
{
    // extrapolate the render position
    if(oldgametic!=gametic || oldlocalgametic!=localgametic)
    {
        player_t *p=&players[consoleplayer];
        ULONG    cmdp,resynch=0;

#ifdef PARANOIA
        if(!p->mo) I_Error("Pas d'mobj");
#endif

        // try to find where the server have put my localgametic :P
        // and resynch with this
        if(oldgametic!=gametic)
        {
            for(cmdp=oldgametic;cmdp<localgametic;cmdp++)
            {
                DEBFILE(va("-> localcmds[%d]=%d\n",cmdp,localcmds[cmdp & PREDICTIONMASK].localtic));
                if(netcmds[(gametic-1)%BACKUPTICS][consoleplayer].localtic ==
                   localcmds[cmdp & PREDICTIONMASK].localtic )
                {
                    ULONG i;

                    resynch=1;
                    if(cmdp==gametic-1)
                    {
                        DEBFILE(va("-> good prediction %d\n",cmdp));
                        cmdp=oldlocalgametic;
                        break;          // nothing to do
                                        // leave the for
                    }

                    localgametic=gametic+localgametic-cmdp-1;
                    for(i=gametic;i<localgametic;i++)
                        memcpy(&localcmds[i & PREDICTIONMASK],
                               &localcmds[(cmdp+i+1-gametic) & PREDICTIONMASK],
                               sizeof(ticcmd_t));
                    DEBFILE(va("-> resynch on %d, local=%d\n",cmdp,localgametic));

                    // reset spirit position to player position
                    memcpy(p->spirit,p->mo,sizeof(mobj_t));
                    p->spirit->type=MT_SPIRIT;
                    p->spirit->info=&mobjinfo[MT_SPIRIT];
                    p->spirit->flags=p->spirit->info->flags;

                    cmdp=gametic;
                    break;  // leave the for
                }
                }
            if(!resynch)
            {
                if(devparm)
                    CONS_Printf("\2Miss prediction\n");
                DEBFILE(va("-> Miss Preditcion\n"));
            }
        }
        else
            cmdp=oldlocalgametic;

        DEBFILE(va("+-+-+-+-+-+- PreRuning tic %d to %d\n",cmdp,localgametic));

        p->mo->flags &=~(MF_SOLID|MF_SHOOTABLE); // put player temporarily in noclip

        // execute tic prematurely
        while(cmdp!=localgametic)
        {
            // sets momx,y,z
            P_MoveSpirit(p,&localcmds[cmdp & PREDICTIONMASK]);
            cmdp++;
            // move using momx,y,z
            P_MobjThinker(p->spirit);
        }
        // bac to reality :) and put the player invisible just for the render
        p->mo->flags |=(MF_SOLID|MF_SHOOTABLE);

        oldlocalgametic=localgametic;
        oldgametic=gametic;
    }
}
#endif

void NetUpdate(void)
{
    static ULONG gametime=0;
    ULONG        nowtime;
    int          i;
    int          realtics;
    int          counts;

    nowtime  = I_GetTime ();
    realtics = nowtime - gametime;

    if (realtics <= 0)   // nothing new to update
        return;
    if(realtics>5)
    {
        if(server)
            realtics=1;
        else
            realtics=5;
    }

    gametime = nowtime;

    if(!server)
        maketic = neededtic;

    Local_Maketic (realtics);    // make local tic, and call menu ?!
    if(server && !demoplayback)
        CL_SendClientCmd ();     // send it
    GetPackets ();               // get packet from client or from server

    // client send the command after a receive of the server
    // the server send before because in single player is beter

    if(!server)
        CL_SendClientCmd ();   // send tic cmd
    else
    {
        if(!demoplayback)
        {

            firstticstosend=gametic;
            for(i=0;i<MAXNETNODES;i++)
                if(nodeingame[i] && nettics[i]<firstticstosend)
                    firstticstosend=nettics[i];

            // Don't erace tics not acknowledged

            counts=realtics;

            if( maketic+counts>=firstticstosend+BACKUPTICS )
                counts=firstticstosend+BACKUPTICS-maketic-1;

            for(i=0;i<counts;i++)
                SV_Maketic();       // create missed tics

            for(i=tictoclear;(unsigned)i<firstticstosend+BACKUPTICS;i++) // clear only when acknoledged
                D_Cleartic(i);                 // clear the maketic the new tic

            tictoclear=firstticstosend+BACKUPTICS;
            SV_SendTics();

            neededtic=maketic; // the server is a client too
        }
    }
    AckTicker();
    M_Ticker ();
    CON_Ticker();
    FiletxTicker();
}
