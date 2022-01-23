// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: g_game.c,v 1.43 2001/12/26 17:24:46 hurdler Exp $
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
// $Log: g_game.c,v $
// Revision 1.43  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.42  2001/12/15 18:41:35  hurdler
// small commit, mainly splitscreen fix
//
// Revision 1.41  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.40  2001/08/20 18:34:18  bpereira
// glide ligthing and map30 bug
//
// Revision 1.39  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.38  2001/08/02 19:15:59  bpereira
// fix player reset in secret level of doom2
//
// Revision 1.37  2001/07/16 22:35:40  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.36  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.35  2001/05/03 21:22:25  hurdler
// remove some warnings
//
// Revision 1.34  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.33  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.32  2001/03/03 06:17:33  bpereira
// no message
//
// Revision 1.31  2001/02/24 13:35:19  bpereira
// no message
//
// Revision 1.30  2001/02/10 12:27:13  bpereira
// no message
//
// Revision 1.29  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.28  2000/11/26 20:36:14  hurdler
// Adding autorun2
//
// Revision 1.27  2000/11/11 13:59:45  bpereira
// no message
//
// Revision 1.26  2000/11/06 20:52:15  bpereira
// no message
//
// Revision 1.25  2000/11/04 16:23:42  bpereira
// no message
//
// Revision 1.24  2000/11/02 19:49:35  bpereira
// no message
//
// Revision 1.23  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.22  2000/10/21 08:43:28  bpereira
// no message
//
// Revision 1.21  2000/10/09 14:03:31  crashrl
// *** empty log message ***
//
// Revision 1.20  2000/10/08 13:30:00  bpereira
// no message
//
// Revision 1.19  2000/10/07 20:36:13  crashrl
// Added deathmatch team-start-sectors via sector/line-tag and linedef-type 1000-1031
//
// Revision 1.18  2000/10/01 10:18:17  bpereira
// no message
//
// Revision 1.17  2000/09/28 20:57:14  bpereira
// no message
//
// Revision 1.16  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.15  2000/08/10 14:08:48  hurdler
// no message
//
// Revision 1.14  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.13  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.12  2000/04/19 10:56:51  hurdler
// commited for exe release and tag only
//
// Revision 1.11  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.10  2000/04/11 19:07:23  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.9  2000/04/07 23:11:17  metzgermeister
// added mouse move
//
// Revision 1.8  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.7  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.5  2000/03/23 22:54:00  metzgermeister
// added support for HOME/.legacy under Linux
//
// Revision 1.4  2000/02/27 16:30:28  hurdler
// dead player bug fix + add allowmlook <yes|no>
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      game loop functions, events handling
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "command.h"
#include "console.h"
#include "dstrings.h"

#include "d_main.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "f_finale.h"
#include "p_setup.h"
#include "p_saveg.h"

#include "i_system.h"

#include "wi_stuff.h"
#include "am_map.h"
#include "m_random.h"
#include "p_local.h"
#include "p_tick.h"

// SKY handling - still the wrong place.
#include "r_data.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_sky.h"

#include "s_sound.h"

#include "g_game.h"
#include "g_state.h"
#include "g_input.h"

//added:16-01-98:quick hack test of rocket trails
#include "p_fab.h"
#include "m_cheat.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_argv.h"

#include "hu_stuff.h"

#include "st_stuff.h"

#include "keys.h"
#include "w_wad.h"
#include "z_zone.h"

#include "i_video.h"
#include "p_inter.h"
#include "p_info.h"
#include "byteptr.h"

#include "i_joy.h"
#include "dehacked.h"
#include "r_things.h"

// added 8-3-98 increse savegame size from 0x2c000 (180kb) to 512*1024
#define SAVEGAMESIZE    (512*1024)
#define SAVESTRINGSIZE  24


boolean G_CheckDemoStatus (void);
void    G_ReadDemoTiccmd (ticcmd_t* cmd,int playernum);
void    G_WriteDemoTiccmd (ticcmd_t* cmd,int playernum);
void    G_InitNew (skill_t skill, char* mapname, boolean resetplayer);

void    G_DoCompleted (void);
void    G_DoVictory (void);
void    G_DoWorldDone (void);

void V_SetPaletteLump(); // Tails 02-20-2002
void SetSavedSkin(int playernum, int skinnum, int skincolor); // Tails

byte            gameepisode;
short           gamemap;
char            gamemapname[MAX_WADPATH];      // an external wad filename

boolean         modifiedgame;                  // Set if homebrew PWAD stuff has been added.
boolean         paused;

boolean         timingdemo;             // if true, exit with report on completion
boolean         nodrawers;              // for comparative timing purposes
boolean         noblit;                 // for comparative timing purposes
tic_t           demostarttime;              // for comparative timing purposes

boolean         netgame;                // only true if packets are broadcast
boolean         multiplayer;
boolean         playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];

int             consoleplayer;          // player taking events and displaying
int             displayplayer;          // view being displayed
int             secondarydisplayplayer; // for splitscreen
int             statusbarplayer;        // player who's statusbar is displayed
                                        // (for spying with F12)

tic_t           gametic;
tic_t           levelstarttic;          // gametic at level start
int             totalkills, totalitems, totalsecret, totalrings;    // for intermission Tails 08-11-2001
int             lastmap; // Tails

int spstage_start;
int sstage_start;
int sstage_end;

tic_t countdowntimer = 0;
boolean mapcleared[NUMMAPS];
boolean countdowntimeup = false;

cutscene_t cutscenes[128];

int nextmapoverride; // Not a byte! Graue 12-31-2003
boolean skipstats;

mapthing_t* rflagpoint; // Original flag spawn location Tails 08-02-2001
mapthing_t* bflagpoint; // Original flag spawn location Tails 08-02-2001

boolean twodlevel;

// Map Header Information Tails 04-08-2003
mapheader_t mapheaderinfo[NUMMAPS];

tic_t playerchangedelay;
boolean delayoverride;

// start emeralds Tails 04-08-2000
unsigned short emeralds;
int token;
int tokenlist; // List of tokens collected Tails 12-18-2003
byte tokenbits; // Used for setting token bits Tails 12-18-2003
int	sstimer; // Time allotted in the special stage Tails 08-11-2001
// end emeralds Tails 04-08-2000

int par; // Tails 07-02-2003

char lvltable[LEVELARRAYSIZE+3][64];

tic_t totalplaytime;
int gottenemblems;
int foundeggs;

// Mystic's funky SA ripoff emblem idea Tails 12-08-2002
emblem_t emblemlocations[NUMEMBLEMS-2] = {
	{  1566,  4352,  608, 0,      1, 1}, //GFZ1 Sonic
	{  2079, -1284,  768, 1,      2, 1}, //GFZ1 Tails
	{  1270,  5597,  960, 2,      4, 1}, //GFZ1 Knuckles
	{ -5560, -2865, 2816, 0,      8, 2}, //GFZ2 Sonic
	{    29,  -589, 3840, 1,     16, 2}, //GFZ2 Tails
	{ -3872,  4203, 3072, 2,     32, 2}, //GFZ2 Knuckles
	{ -985, -12193, 2172, 0,     64, 4}, //THZ1 Sonic
	{  4272, -5720, 3700, 1,    128, 4}, //THZ1 Tails
	{ 14318, -9281, 4040, 2,    256, 4}, //THZ1 Knuckles
	{-11100,  3233, 1312, 0,    512, 5}, //THZ2 Sonic
	{  6978,  3341, 2300, 1,   1024, 5}, //THZ2 Tails
	{   972, -4094, 2432, 2,   2048, 5}, //THZ2 Knuckles
	{  2848, -4094, 3800, 0,   4096, 7}, //CEZ1 Sonic
	{ -5936,   322, 2900, 1,   8192, 7}, //CEZ1 Tails
	{   -17,  1169,  912, 2,  16384, 7}, //CEZ1 Knuckles
	{ -3039,  -321, 1080, 0,  32768, 8}, //CEZ2 Sonic
	{  5002,  4948,  460, 1,  65536, 8}, //CEZ2 Tails
	{  2411, -2224,  212, 2, 131072, 8} //CEZ2 Knuckles
};

// Easter Eggs - Literally! Tails 02-06-2003
emblem_t egglocations[NUMEGGS] = {
	{  -472,   2041,  103, 0,    1, 1},
	{  3201,   6794,  103, 0,    2, 1},
	{  6300,   4544, 1536, 0,    4, 2},
	{  2803,  -6232, 1300, 0,    8, 2},
	{ -1072, -10618, 2172, 0,   16, 4},
	{  2158,  -1378, 2468, 0,   32, 4},
	{ -7746,   4688, 1280, 0,   64, 5},
	{  2840,  -1992,  928, 0,  128, 5},
	{  3322,   5459,  624, 0,  256, 7},
	{ -5441,   3907, 1088, 0,  512, 7},
	{  5030,  -2610,  408, 0, 1024, 8},
	{ -3617,  -1886,  601, 0, 2048, 8}
};

// Time attack data for levels Tails 12-08-2002
timeattack_t timedata[NUMMAPS];

int bluescore; // Team Scores Tails 07-31-2001
int redscore; // Team Scores Tails 07-31-2001

// Powerup durations Tails 07-26-2003
int invulntics = 20*TICRATE;
int sneakertics = 20*TICRATE;
int flashingtics = 3*TICRATE;
int tailsflytics = 8*TICRATE;
int underwatertics = 30*TICRATE;
int spacetimetics = 11*TICRATE;
int extralifetics = 4*TICRATE;
int paralooptics = 20*TICRATE;
int helpertics = 20*TICRATE;

byte introtoplay;

mobj_t* hunt1;
mobj_t* hunt2;
mobj_t* hunt3;

// For racing
int countdown;
int countdown2;

// Tails 08-20-2002
fixed_t gravity;

// Grading Tails 08-13-2002
int grade;
boolean veryhardcleared; // Tails 05-19-2003

char            demoname[32];
boolean         demorecording;
boolean         demoplayback;
byte*           demobuffer;
byte*           demo_p;
byte*           demoend;
boolean         singledemo;             // quit after playing a demo from cmdline

boolean         precache = true;        // if true, load all graphics at start

wbstartstruct_t wminfo;                 // parms for world map / intermission

byte*           savebuffer;

void ShowMessage_OnChange(void);
void AllowTurbo_OnChange(void);
void Analog_OnChange(void); // Analog Test Tails 06-11-2001
void Analog2_OnChange(void); // Analog Test Tails 12-16-2002

CV_PossibleValue_t showmessages_cons_t[]={{0,"Off"},{1,"On"},{2,"Not All"},{0,NULL}};
CV_PossibleValue_t crosshair_cons_t[]   ={{0,"Off"},{1,"Cross"},{2,"Angle"},{3,"Point"},{0,NULL}};

consvar_t cv_crosshair        = {"crosshair"   ,"0",CV_SAVE,crosshair_cons_t};
consvar_t cv_crosshair2        = {"crosshair2"   ,"0",CV_SAVE,crosshair_cons_t};
//consvar_t cv_crosshairscale   = {"crosshairscale","0",CV_SAVE,CV_YesNo};
consvar_t cv_invertmouse      = {"invertmouse" ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_alwaysfreelook   = {"alwaysmlook" ,"0",CV_SAVE,CV_OnOff};
consvar_t cv_invertmouse2     = {"invertmouse2","0",CV_SAVE,CV_OnOff};
consvar_t cv_alwaysfreelook2  = {"alwaysmlook2","0",CV_SAVE,CV_OnOff};
consvar_t cv_showmessages     = {"showmessages","1",CV_SAVE | CV_CALL | CV_NOINIT,showmessages_cons_t,ShowMessage_OnChange};
//consvar_t cv_allowturbo       = {"allowturbo"  ,"0",CV_NETVAR | CV_CALL, CV_YesNo, AllowTurbo_OnChange};
consvar_t cv_mousemove        = {"mousemove"   ,"1",CV_SAVE,CV_OnOff};
consvar_t cv_mousemove2       = {"mousemove2"  ,"1",CV_SAVE,CV_OnOff};
consvar_t cv_analog			= {"analog"		,"0",CV_NETVAR | CV_CALL,CV_OnOff, Analog_OnChange}; // Analog Test Tails 06-10-2001
consvar_t cv_analog2		= {"analog2"	,"0",CV_NETVAR | CV_CALL,CV_OnOff, Analog2_OnChange}; // Analog Test Tails 06-10-2001
consvar_t cv_joystickfreelook = {"joystickfreelook" ,"0",CV_SAVE,CV_OnOff};


#if MAXPLAYERS>32
#error please update "player_name" table using the new value for MAXPLAYERS
#endif
#if MAXPLAYERNAME!=21
#error please update "player_name" table using the new value for MAXPLAYERNAME
#endif
// changed to 2d array 19990220 by Kin
char    player_names[MAXPLAYERS][MAXPLAYERNAME] =
{
    // THESE SHOULD BE AT LEAST MAXPLAYERNAME CHARS
    "Player 1\0a123456789a\0",
    "Player 2\0a123456789a\0",
    "Player 3\0a123456789a\0",
    "Player 4\0a123456789a\0",
    "Player 5\0a123456789a\0",        // added 14-1-98 for support 8 players
    "Player 6\0a123456789a\0",        // added 14-1-98 for support 8 players
    "Player 7\0a123456789a\0",        // added 14-1-98 for support 8 players
    "Player 8\0a123456789a\0",        // added 14-1-98 for support 8 players
    "Player 9\0a123456789a\0",
    "Player 10\0a123456789\0",
    "Player 11\0a123456789\0",
    "Player 12\0a123456789\0",
    "Player 13\0a123456789\0",
    "Player 14\0a123456789\0",
    "Player 15\0a123456789\0",
    "Player 16\0a123456789\0",
    "Player 17\0a123456789\0",
    "Player 18\0a123456789\0",
    "Player 19\0a123456789\0",
    "Player 20\0a123456789\0",
    "Player 21\0a123456789\0",
    "Player 22\0a123456789\0",
    "Player 23\0a123456789\0",
    "Player 24\0a123456789\0",
    "Player 25\0a123456789\0",
    "Player 26\0a123456789\0",
    "Player 27\0a123456789\0",
    "Player 28\0a123456789\0",
    "Player 29\0a123456789\0",
    "Player 30\0a123456789\0",
    "Player 31\0a123456789\0",
    "Player 32\0a123456789\0"
};

char *team_names[MAXPLAYERS] =
{
    "Team 1\0a890123456789b\0",
    "Team 2\0a890123456789b\0",
    "Team 3\0a890123456789b\0",
    "Team 4\0a890123456789b\0",
    "Team 5\0a890123456789b\0",
    "Team 6\0a890123456789b\0",
    "Team 7\0a890123456789b\0",
    "Team 8\0a890123456789b\0",
    "Team 9\0a890123456789b\0",
    "Team 10\0a90123456789b\0",
    "Team 11\0a90123456789b\0",
    "Team 12\0a90123456789b\0",      // the other name hare not used because no colors
    "Team 13\0a90123456789b\0",      // but who know ?
    "Team 14\0a90123456789b\0",
    "Team 15\0a90123456789b\0",
    "Team 16\0a90123456789b\0",
    "Team 17\0a90123456789b\0",
    "Team 18\0a90123456789b\0",
    "Team 19\0a90123456789b\0",
    "Team 20\0a90123456789b\0",
    "Team 21\0a90123456789b\0",
    "Team 22\0a90123456789b\0",
    "Team 23\0a90123456789b\0",
    "Team 24\0a90123456789b\0",
    "Team 25\0a90123456789b\0",
    "Team 26\0a90123456789b\0",
    "Team 27\0a90123456789b\0",
    "Team 28\0a90123456789b\0",
    "Team 29\0a90123456789b\0",
    "Team 30\0a90123456789b\0",
    "Team 31\0a90123456789b\0",
    "Team 32\0a90123456789b\0"
};

void*     statcopy;                      // for statistics driver

void ShowMessage_OnChange(void)
{
    if (!cv_showmessages.value)
        CONS_Printf("%s\n",MSGOFF);
    else
        CONS_Printf("%s\n",MSGON);
}


//  Build an original game map name from episode and map number,
//  based on the game mode (doom1, doom2...)
//
char* G_BuildMapName (int map)
{
   static char  mapname[9];    // internal map name (wad resource name)

   sprintf(mapname, "MAP%.2d", map);
   if(map >= 100) {
      mapname[3] = 'A' + (map - 100) / 36;
      if((map - 100) % 36 < 10) mapname[4] = '0' + ((map - 100) % 36);
      else mapname[4] = 'A' + ((map - 100) % 36) - 10;
      mapname[5] = '\0';
   }

   return mapname;
}


//
//  Clip the console player mouse aiming to the current view,
//  also returns a signed char for the player ticcmd if needed.
//  Used whenever the player view pitch is changed manually
//
//added:22-02-98:
//changed:3-3-98: do a angle limitation now
short G_ClipAimingPitch (int* aiming)
{
    int limitangle;

    //note: the current software mode implementation doesn't have true perspective
    if ( rendermode == render_soft )
		limitangle = ANG45 + (ANG45/2); // Tails 10-31-99 Some viewing fun, but not too far down...
    else
        limitangle = ANG90 - 1;

    if (*aiming > limitangle )
        *aiming = limitangle;
    else
    if (*aiming < -limitangle)
        *aiming = -limitangle;

    return (*aiming)>>16;
}


//
// G_BuildTiccmd
// Builds a ticcmd from all of the available inputs
// or reads it from the demo buffer.
// If recording a demo, write it out
//
// set secondaryplayer true to build player 2's ticcmd in splitscreen mode
//
int     localaiming,localaiming2;
angle_t localangle,localangle2;

//added:06-02-98: mouseaiming (looking up/down with the mouse or keyboard)
#define KB_LOOKSPEED    (1<<25)
#define MAXPLMOVE       (forwardmove[1])
#define SLOWTURNTICS    (6*NEWTICRATERATIO)

static fixed_t forwardmove[2] = {25/NEWTICRATERATIO, 50/NEWTICRATERATIO};
static fixed_t sidemove[2]    = {25/NEWTICRATERATIO, 50/NEWTICRATERATIO}; // faster! Tails 09-08-2001
static fixed_t angleturn[3]   = {640/NEWTICRATERATIO, 1280/NEWTICRATERATIO, 320/NEWTICRATERATIO};        // + slow turn // Tails

void G_BuildTiccmd (ticcmd_t* cmd, int realtics)
{
    boolean     strafe;
    int         speed;
    int         tspeed;
    int         forward;
    int         side;
    ticcmd_t*   base;
    //added:14-02-98: these ones used for multiple conditions
    boolean     turnleft,turnright,mouseaiming,analogjoystickmove,gamepadjoystickmove;

    static int  turnheld;               // for accelerative turning
    static boolean  keyboard_look;      // true if lookup/down using keyboard

    base = I_BaseTiccmd ();             // empty, or external driver
    memcpy (cmd,base,sizeof(*cmd));

    // a little clumsy, but then the g_input.c became a lot simpler!
    strafe = gamekeydown[gamecontrol[gc_strafe][0]] ||
             gamekeydown[gamecontrol[gc_strafe][1]];
    speed  = (gamekeydown[gamecontrol[gc_speed][0]] ||
             gamekeydown[gamecontrol[gc_speed][1]]) ^
             1;

    turnright = gamekeydown[gamecontrol[gc_turnright][0]]
              ||gamekeydown[gamecontrol[gc_turnright][1]];
    turnleft  = gamekeydown[gamecontrol[gc_turnleft][0]]
              ||gamekeydown[gamecontrol[gc_turnleft][1]];
    mouseaiming = (gamekeydown[gamecontrol[gc_mouseaiming][0]]
                ||gamekeydown[gamecontrol[gc_mouseaiming][1]])
                ^ cv_alwaysfreelook.value;
    analogjoystickmove  = cv_usejoystick.value && !Joystick.bGamepadStyle && !cv_splitscreen.value;
    gamepadjoystickmove = cv_usejoystick.value &&  Joystick.bGamepadStyle && !cv_splitscreen.value;

    if( gamepadjoystickmove )
    {
        turnright = turnright || (joyxmove > 0);
        turnleft  = turnleft  || (joyxmove < 0);
    }
    forward = side = 0;

    // use two stage accelerative turning
    // on the keyboard and joystick
    if (turnleft || turnright)
        turnheld += realtics;
    else
        turnheld = 0;

    if (turnheld < SLOWTURNTICS)
        tspeed = 2;             // slow turn
    else
        tspeed = speed;

    // let movement keys cancel each other out
	if(cv_analog.value) // Analog Test Tails 06-10-2001
	{
        if (turnright)
		{
			cmd->angleturn -= angleturn[tspeed];
		}
        if (turnleft)
		{
			cmd->angleturn += angleturn[tspeed];
		}
	}
    if (strafe || cv_analog.value || twodlevel || players[consoleplayer].climbing || players[consoleplayer].nightsmode) // Analog Test Tails 06-10-2001
    {
        if (turnright)
		{
            side += sidemove[speed];
		}
        if (turnleft)
		{
            side -= sidemove[speed];
		}

        if (analogjoystickmove)
        {
            //faB: JOYAXISRANGE is supposed to be 1023 ( divide by 1024 )
            side += ( (joyxmove * sidemove[1]) >> 10 );
        }
    }
    else
    {
        if ( turnright )
            cmd->angleturn -= angleturn[tspeed];
        else
        if ( turnleft  )
            cmd->angleturn += angleturn[tspeed];
        if ( joyxmove && analogjoystickmove) {
            //faB: JOYAXISRANGE should be 1023 ( divide by 1024 )
            cmd->angleturn -= ( (joyxmove * angleturn[1]) >> 10 );        // ANALOG!
            //CONS_Printf ("joyxmove %d  angleturn %d\n", joyxmove, cmd->angleturn);
        }

    }

    //added:07-02-98: forward with key or button
    if (gamekeydown[gamecontrol[gc_forward][0]] ||
        gamekeydown[gamecontrol[gc_forward][1]] ||
        ( joyymove < 0 && gamepadjoystickmove && !cv_joystickfreelook.value) )
    {
        forward += forwardmove[speed];
    }
    if (gamekeydown[gamecontrol[gc_backward][0]] ||
        gamekeydown[gamecontrol[gc_backward][1]] ||
        ( joyymove > 0 && gamepadjoystickmove && !cv_joystickfreelook.value) )
    {
        forward -= forwardmove[speed];
    }
        
    if ( joyymove && analogjoystickmove && !cv_joystickfreelook.value) 
        forward -= ( (joyymove * forwardmove[1]) >> 10 );               // ANALOG!

    //added:07-02-98: some people strafe left & right with mouse buttons
    if (gamekeydown[gamecontrol[gc_straferight][0]] ||
        gamekeydown[gamecontrol[gc_straferight][1]])
	{
        side += sidemove[speed];
	}
    if (gamekeydown[gamecontrol[gc_strafeleft][0]] ||
        gamekeydown[gamecontrol[gc_strafeleft][1]])
	{
        side -= sidemove[speed];
	}

    //added:07-02-98: fire with any button/key
    if (gamekeydown[gamecontrol[gc_fire][0]] ||
        gamekeydown[gamecontrol[gc_fire][1]])
        cmd->buttons |= BT_ATTACK;

    if (gamekeydown[gamecontrol[gc_lightdash][0]] ||
        gamekeydown[gamecontrol[gc_lightdash][1]])
        cmd->buttons |= BT_LIGHTDASH;

    //added:07-02-98: use with any button/key
    if (gamekeydown[gamecontrol[gc_use][0]] ||
        gamekeydown[gamecontrol[gc_use][1]])
        cmd->buttons |= BT_USE;

	// Taunts Tails 09-06-2002
	if (gamekeydown[gamecontrol[gc_taunt][0]] ||
		gamekeydown[gamecontrol[gc_taunt][1]])
		cmd->buttons |= BT_TAUNT;

	// Camera Controls! Finally! Tails 01-12-2003
	if (gamekeydown[gamecontrol[gc_camleft][0]] ||
		gamekeydown[gamecontrol[gc_camleft][1]])
		cmd->buttons |= BT_CAMLEFT;

	if (gamekeydown[gamecontrol[gc_camright][0]] ||
		gamekeydown[gamecontrol[gc_camright][1]])
		cmd->buttons |= BT_CAMRIGHT;

	if (gamekeydown[gamecontrol[gc_camreset][0]] ||
		gamekeydown[gamecontrol[gc_camreset][1]])
		P_ResetCamera(&players[displayplayer], &camera);

    //added:22-02-98: jump button
    if (gamekeydown[gamecontrol[gc_jump][0]] ||
        gamekeydown[gamecontrol[gc_jump][1]])
        cmd->buttons |= BT_JUMP;

    // mouse look stuff (mouse look is not the same as mouse aim)
    if (mouseaiming)
    {
        keyboard_look = false;

        // looking up/down
        if (cv_invertmouse.value)
            localaiming -= mlooky<<19;
        else
            localaiming += mlooky<<19;
    }
    if (cv_usejoystick.value && analogjoystickmove && cv_joystickfreelook.value)
        localaiming += joyymove<<16;

    // spring back if not using keyboard neither mouselookin'
    if (!keyboard_look && !cv_joystickfreelook.value && !mouseaiming)
        localaiming = 0;

    if (gamekeydown[gamecontrol[gc_lookup][0]] ||
        gamekeydown[gamecontrol[gc_lookup][1]])
    {
        localaiming += KB_LOOKSPEED;
        keyboard_look = true;
    }
    else
    if (gamekeydown[gamecontrol[gc_lookdown][0]] ||
        gamekeydown[gamecontrol[gc_lookdown][1]])
    {
        localaiming -= KB_LOOKSPEED;
        keyboard_look = true;
    }
    else
    if (gamekeydown[gamecontrol[gc_centerview][0]] ||
        gamekeydown[gamecontrol[gc_centerview][1]])
        localaiming = 0;

    //26/02/2000: added by Hurdler: accept no mlook for network games
    if (!cv_allowmlook.value)
        localaiming = 0;

    cmd->aiming = G_ClipAimingPitch (&localaiming);

    if (!mouseaiming && cv_mousemove.value)
        forward += mousey;

    if (strafe || cv_analog.value) // Analog for mouse Tails 06-20-2001
        side += mousex*2;
    else
        cmd->angleturn -= mousex*8;

    mousex = mousey = mlooky = 0;

    if (forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;
    if (side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if (side < -MAXPLMOVE)
        side = -MAXPLMOVE;

    cmd->forwardmove += forward;
    cmd->sidemove += side;
#ifdef ABSOLUTEANGLE
    localangle += (cmd->angleturn<<16);
    cmd->angleturn = localangle >> 16;
#endif
}


// like the g_buildticcmd 1 but using mouse2, gamcontrolbis, ...
void G_BuildTiccmd2 (ticcmd_t* cmd, int realtics)
{
    boolean     strafe;
    int         speed;
    int         tspeed;
    int         forward;
    int         side;
    ticcmd_t*   base;
    //added:14-02-98: these ones used for multiple conditions
    boolean     turnleft,turnright,mouseaiming,analogjoystickmove,gamepadjoystickmove;

    static int  turnheld;               // for accelerative turning
    static boolean  keyboard_look;      // true if lookup/down using keyboard

    base = I_BaseTiccmd ();             // empty, or external driver
    memcpy (cmd,base,sizeof(*cmd));

    // a little clumsy, but then the g_input.c became a lot simpler!
    strafe = gamekeydown[gamecontrolbis[gc_strafe][0]] ||
             gamekeydown[gamecontrolbis[gc_strafe][1]];
    speed  = (gamekeydown[gamecontrolbis[gc_speed][0]] ||
             gamekeydown[gamecontrolbis[gc_speed][1]])
             ^ 1;

    turnright = gamekeydown[gamecontrolbis[gc_turnright][0]]
              ||gamekeydown[gamecontrolbis[gc_turnright][1]];
    turnleft  = gamekeydown[gamecontrolbis[gc_turnleft][0]]
              ||gamekeydown[gamecontrolbis[gc_turnleft][1]];

    mouseaiming = (gamekeydown[gamecontrolbis[gc_mouseaiming][0]]
                ||gamekeydown[gamecontrolbis[gc_mouseaiming][1]])
                ^ cv_alwaysfreelook2.value;
    analogjoystickmove  = cv_usejoystick.value && !Joystick.bGamepadStyle;
    gamepadjoystickmove = cv_usejoystick.value &&  Joystick.bGamepadStyle;

    if(gamepadjoystickmove)
    {
        turnright = turnright || joyxmove > 0;
        turnleft  = turnleft  || joyxmove < 0;
    }

    forward = side = 0;

    // use two stage accelerative turning
    // on the keyboard and joystick
    if (turnleft || turnright)
        turnheld += realtics;
    else
        turnheld = 0;

    if (turnheld < SLOWTURNTICS)
        tspeed = 2;             // slow turn
    else
        tspeed = speed;

    // let movement keys cancel each other out
	if(cv_analog2.value) // Analog Test Tails 06-10-2001
	{
        if (turnright)
		{
			cmd->angleturn -= angleturn[tspeed];
		}
        if (turnleft)
		{
			cmd->angleturn += angleturn[tspeed];
		}
	}

    if (strafe || cv_analog2.value || twodlevel || players[secondarydisplayplayer].climbing || players[secondarydisplayplayer].nightsmode) // Analog Test Tails 06-10-2001
    {
        if (turnright)
		{
            side += sidemove[speed];
		}
        if (turnleft)
		{
            side -= sidemove[speed];
		}

        if (analogjoystickmove)
        {
            //faB: JOYAXISRANGE is supposed to be 1023 ( divide by 1024 )
            side += ( (joyxmove * sidemove[1]) >> 10 );
        }
    }
    else
    {
        if (turnright )
            cmd->angleturn -= angleturn[tspeed];
        if (turnleft  )
            cmd->angleturn += angleturn[tspeed];

        if ( joyxmove && analogjoystickmove )
        {
            //faB: JOYAXISRANGE should be 1023 ( divide by 1024 )
            cmd->angleturn -= ( (joyxmove * angleturn[1]) >> 10 );        // ANALOG!
            //CONS_Printf ("joyxmove %d  angleturn %d\n", joyxmove, cmd->angleturn);
        }
    }

    //added:07-02-98: forward with key or button
    if (gamekeydown[gamecontrolbis[gc_forward][0]] ||
        gamekeydown[gamecontrolbis[gc_forward][1]] ||
        (joyymove < 0 && gamepadjoystickmove && !cv_joystickfreelook.value))
    {
        forward += forwardmove[speed];
    }

    if (gamekeydown[gamecontrolbis[gc_backward][0]] ||
        gamekeydown[gamecontrolbis[gc_backward][1]] ||
        (joyymove > 0 && gamepadjoystickmove && !cv_joystickfreelook.value))
    {
        forward -= forwardmove[speed];
    }

    if ( joyymove && analogjoystickmove && !cv_joystickfreelook.value) 
        forward -= ( (joyymove * forwardmove[1]) >> 10 ); // ANALOG!
    

    //added:07-02-98: some people strafe left & right with mouse buttons
    if (gamekeydown[gamecontrolbis[gc_straferight][0]] ||
        gamekeydown[gamecontrolbis[gc_straferight][1]])
        side += sidemove[speed];
    if (gamekeydown[gamecontrolbis[gc_strafeleft][0]] ||
        gamekeydown[gamecontrolbis[gc_strafeleft][1]])
        side -= sidemove[speed];

    //added:07-02-98: fire with any button/key
    if (gamekeydown[gamecontrolbis[gc_fire][0]] ||
        gamekeydown[gamecontrolbis[gc_fire][1]])
        cmd->buttons |= BT_ATTACK;

    if (gamekeydown[gamecontrolbis[gc_lightdash][0]] ||
        gamekeydown[gamecontrolbis[gc_lightdash][1]])
        cmd->buttons |= BT_LIGHTDASH;

    //added:07-02-98: use with any button/key
    if (gamekeydown[gamecontrolbis[gc_use][0]] ||
        gamekeydown[gamecontrolbis[gc_use][1]])
        cmd->buttons |= BT_USE;

	// Taunts Tails 09-06-2002
	if (gamekeydown[gamecontrolbis[gc_taunt][0]] ||
		gamekeydown[gamecontrolbis[gc_taunt][1]])
		cmd->buttons |= BT_TAUNT;

	// Camera Controls! Finally! Tails 01-12-2003
	if (gamekeydown[gamecontrolbis[gc_camleft][0]] ||
		gamekeydown[gamecontrolbis[gc_camleft][1]])
		cmd->buttons |= BT_CAMLEFT;

	if (gamekeydown[gamecontrolbis[gc_camright][0]] ||
		gamekeydown[gamecontrolbis[gc_camright][1]])
		cmd->buttons |= BT_CAMRIGHT;

	if (gamekeydown[gamecontrolbis[gc_camreset][0]] ||
		gamekeydown[gamecontrolbis[gc_camreset][1]])
		P_ResetCamera(&players[secondarydisplayplayer], &camera2);

    //added:22-02-98: jump button
    if (gamekeydown[gamecontrolbis[gc_jump][0]] ||
        gamekeydown[gamecontrolbis[gc_jump][1]])
        cmd->buttons |= BT_JUMP;

    // mouse look stuff (mouse look is not the same as mouse aim)
    if (mouseaiming)
    {
        keyboard_look = false;

        // looking up/down
        if (cv_invertmouse2.value)
            localaiming2 -= mlook2y<<19;
        else
            localaiming2 += mlook2y<<19;
    }

    if( analogjoystickmove && cv_joystickfreelook.value )
        localaiming2 += joyymove<<16;
    // spring back if not using keyboard neither mouselookin'
    if (!keyboard_look && !cv_joystickfreelook.value && !mouseaiming)
        localaiming2 = 0;

    if (gamekeydown[gamecontrolbis[gc_lookup][0]] ||
        gamekeydown[gamecontrolbis[gc_lookup][1]])
    {
        localaiming2 += KB_LOOKSPEED;
        keyboard_look = true;
    }
    else
    if (gamekeydown[gamecontrolbis[gc_lookdown][0]] ||
        gamekeydown[gamecontrolbis[gc_lookdown][1]])
    {
        localaiming2 -= KB_LOOKSPEED;
        keyboard_look = true;
    }
    else
    if (gamekeydown[gamecontrolbis[gc_centerview][0]] ||
        gamekeydown[gamecontrolbis[gc_centerview][1]])
        localaiming2 = 0;

    //26/02/2000: added by Hurdler: accept no mlook for network games
    if (!cv_allowmlook.value)
        localaiming2 = 0;

    // look up max (viewheight/2) look down min -(viewheight/2)
    cmd->aiming = G_ClipAimingPitch (&localaiming2);;

    if (!mouseaiming && cv_mousemove2.value)
        forward += mouse2y;

    if (strafe || cv_analog2.value) // Analog for mouse Tails 06-20-2001
        side += mouse2x*2;
    else
        cmd->angleturn -= mouse2x*8;

    mouse2x = mouse2y = mlook2y = 0;

    if (forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;
    if (side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if (side < -MAXPLMOVE)
        side = -MAXPLMOVE;

    cmd->forwardmove += forward;
    cmd->sidemove += side;

#ifdef ABSOLUTEANGLE
    localangle2 += (cmd->angleturn<<16);
    cmd->angleturn = localangle2 >> 16;
#endif
}

void Analog_OnChange(void) // Analog Test Tails 06-11-2001
{
	if(leveltime > 1)
		CV_SetValue(&cv_cam_dist, 128);

    if(netgame) // Analog is OK in split screen now!
		cv_analog.value = 0;
	else if(cv_analog.value)
		CV_SetValue(&cv_cam_dist, 192);
}// Analog Test Tails 06-11-2001

void Analog2_OnChange(void) // Tails 12-16-2002
{
	if(leveltime > 1)
		CV_SetValue(&cv_cam2_dist, 128);

    if(netgame) // Analog is OK in split screen now!
		cv_analog2.value = 0;
	else if(cv_analog2.value)
		CV_SetValue(&cv_cam2_dist, 192);
}

extern consvar_t cv_chasecam; // Tails
extern consvar_t cv_chasecam2; // Tails 12-16-2002

void Z_JampThatMem(void); // Graue 12-25-2003

//
// G_DoLoadLevel
//
void G_DoLoadLevel (boolean resetplayer)
{
    int             i;

    levelstarttic = gametic;        // for time calculation

    if (wipegamestate == GS_LEVEL)
        wipegamestate = -1;             // force a wipe

    gamestate = GS_LEVEL;
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if( resetplayer || (playeringame[i] && players[i].playerstate == PST_DEAD))
            players[i].playerstate = PST_REBORN;
    }

    if (!P_SetupLevel (gameepisode, gamemap, gameskill, gamemapname[0] ? gamemapname:NULL) )
    {
        // fail so reset game stuff
        Command_ExitGame_f();
        return;
    }

    //BOT_InitLevelBots ();

    displayplayer = consoleplayer;          // view the guy you are playing
    if(!cv_splitscreen.value)
        secondarydisplayplayer = consoleplayer;

    gameaction = ga_nothing;
#ifdef PARANOIA
	Z_JampThatMem(); // We're paranoid, okay? Graue 12-25-2003
    Z_CheckHeap (-2);
#endif

    if (cv_chasecam.value)
	{
        P_ResetCamera (&players[displayplayer], &camera);
	}
	if(cv_chasecam2.value && cv_splitscreen.value) // Tails 12-16-2002
	{
		P_ResetCamera (&players[secondarydisplayplayer], &camera2);
	}

    // clear cmd building stuff
    memset (gamekeydown, 0, sizeof(gamekeydown));
    joyxmove = joyymove = 0;
    mousex = mousey = 0;

    // clear hud messages remains (usually from game startup)
    CON_ClearHUD ();

	// Tails 03-02-2002
	if(!(cv_debug || devparm || modifiedgame) && !(multiplayer || netgame))
		SetSavedSkin(0, players[0].skin, players[0].prefcolor);
}

//
// G_Responder
//  Get info needed to make ticcmd_ts for the players.
//
boolean G_Responder (event_t* ev)
{
    // allow spy mode changes even during the demo
    if (gamestate == GS_LEVEL && ev->type == ev_keydown
        && ev->data1 == KEY_F12) // Tails 03-13-2001
    {
		if(!cv_debug && (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG
			|| cv_gametype.value == GT_CTF || cv_gametype.value == GT_CHAOS))
			displayplayer = consoleplayer;
        else
		{
			// spy mode
			do
			{
				displayplayer++;
				if (displayplayer == MAXPLAYERS)
					displayplayer = 0;
			} while (!playeringame[displayplayer] && displayplayer != consoleplayer);

			//added:16-01-98:change statusbar also if playingback demo
			if( singledemo )
				ST_changeDemoView ();

			//added:11-04-98: tell who's the view
			CONS_Printf("Viewpoint : %s\n", player_names[displayplayer]);

			return true;
		}
	}

    // any other key pops up menu if in demos
    if (gameaction == ga_nothing && !singledemo &&
        (demoplayback || gamestate == GS_DEMOSCREEN || gamestate == GS_TITLESCREEN) )
    {
        if (ev->type == ev_keydown && ev->data1 != 301)
        {
            M_StartControlPanel ();
            return true;
        }
        return false;
    }

    if (gamestate == GS_LEVEL)
    {
#if 0
        if (devparm && ev->type == ev_keydown && ev->data1 == ';')
        {
            // added Boris : test different player colors
            players[consoleplayer].skincolor = (players[consoleplayer].skincolor+1) %MAXSKINCOLORS;
            players[consoleplayer].mo->flags |= (players[consoleplayer].skincolor)<<MF_TRANSSHIFT;
            G_DeathMatchSpawnPlayer (0);
            return true;
        }
#endif
        if(!multiplayer)
           if( cht_Responder (ev))
               return true;
        if (HU_Responder (ev))
            return true;        // chat ate the event
//        if (ST_Responder (ev)) // Tails 03-15-2002
//            return true;        // status window ate it
        if (AM_Responder (ev))
            return true;        // automap ate it
        //added:07-02-98: map the event (key/mouse/joy) to a gamecontrol
    }

    if (gamestate == GS_FINALE)
    {
        if (F_Responder (ev))
            return true;        // finale ate the event
    }

	// Intro Tails 02-17-2002
    else if(gamestate == GS_INTRO || gamestate == GS_INTRO2)
	{
		if(F_IntroResponder(ev))
		{
			D_StartTitle();
			return true;
		}
	}
    else if(gamestate == GS_CUTSCENE)
	{
		if(F_IntroResponder(ev))
		{
			F_EndCutScene();
			return true;
		}
	}

	else if(gamestate == GS_CREDITS)
	{
		if(F_CreditResponder(ev))
		{
			F_StartGameEvaluation();
			return true;
		}
	}

	// Demo End Tails 09-01-2002
	else if(gamestate == GS_DEMOEND || gamestate == GS_EVALUATION || gamestate == GS_CREDITS)
	{
		return true;
	}

	else if(gamestate == GS_INTERMISSION)
	{
        if (HU_Responder (ev))
            return true;        // chat ate the event
	}


    // update keys current state
    G_MapEventsToControls (ev);

    switch (ev->type)
    {
      case ev_keydown:
        if (ev->data1 == KEY_PAUSE)
        {
            COM_BufAddText("pause\n");
            return true;
        }
        return true;

      case ev_keyup:
        return false;   // always let key up events filter down

      case ev_mouse:
        return true;    // eat events

      case ev_joystick:
        return true;    // eat events

      default:
        break;
    }

    return false;
}


//
// G_Ticker
// Make ticcmd_ts for the players.
//
void G_Ticker (void)
{
    ULONG       i;
    int         buf;
    ticcmd_t*   cmd;

    // do player reborns if needed
    if( gamestate == GS_LEVEL )
    {
        for (i=0 ; i<MAXPLAYERS ; i++)
		{
            if (playeringame[i])
            {
                if( players[i].playerstate == PST_REBORN )
				{
                    G_DoReborn (i);
				}
            }
		}
    }

    // do things to change the game state
    while (gameaction != ga_nothing)
        switch (gameaction)
        {
            case ga_completed :  G_DoCompleted (); break;
            case ga_worlddone :  G_DoWorldDone (); break;
            case ga_nothing   :  break;
            default : I_Error("gameaction = %d\n", gameaction);
        }

    buf = gametic%BACKUPTICS;

    // read/write demo and check turbo cheat
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        // BP: i==0 for playback of demos 1.29 now new players is added with xcmd
        if ((playeringame[i] || i==0)/* && !dedicated*/)
        {
            cmd = &players[i].cmd;

            if (demoplayback)
                G_ReadDemoTiccmd (cmd,i);
            else
                memcpy (cmd, &netcmds[buf][i], sizeof(ticcmd_t));

            if (demorecording)
                G_WriteDemoTiccmd (cmd,i);

            // check for turbo cheats
/*            if (cmd->forwardmove > TURBOTHRESHOLD
                && !(gametic % (32*NEWTICRATERATIO)) && ((gametic / (32*NEWTICRATERATIO))&3) == i )
            {
                static char turbomessage[80];
                sprintf (turbomessage, "%s is turbo!",player_names[i]);
                players[consoleplayer].message = turbomessage;
            }*/
        }
    }

    // do main actions
    switch (gamestate)
    {
      case GS_LEVEL:
        //IO_Color(0,255,0,0);
        P_Ticker ();             // tic the game
        //IO_Color(0,0,255,0);
        ST_Ticker ();
        AM_Ticker ();
        HU_Ticker ();
        break;

      case GS_INTERMISSION:
		WI_Ticker ();
		HU_Ticker ();
        break;

      case GS_FINALE:
        F_Ticker ();
        break;

	  case GS_INTRO: // Intro Tails 02-15-2002
	  case GS_INTRO2: // Intro Tails 02-15-2002
		F_IntroTicker ();
		break;

	  case GS_CUTSCENE:
		  F_CutsceneTicker();
		  break;

	  case GS_DEMOEND: // Tails 09-01-2002
		  F_DemoEndTicker ();
		  break;

	  case GS_EVALUATION:
		  F_GameEvaluationTicker();
		  break;

	  case GS_CREDITS:
		  F_CreditTicker(); // Tails 05-03-2003
		  break;

	  case GS_TITLESCREEN: // Tails 11-30-2002
		  F_TitleScreenTicker();
		  break;

      case GS_DEMOSCREEN:
        D_PageTicker ();
        break;

      case GS_WAITINGPLAYERS:
      case GS_DEDICATEDSERVER:
      case GS_NULL:
      // do nothing
        break;
    }
}


//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_InitPlayer
// Called at the start.
// Called by the game initialization functions.
//
/* BP:UNUSED !
void G_InitPlayer (int player)
{
    player_t*   p;

    // set up the saved info
    p = &players[player];

    // clear everything else to defaults
    G_PlayerReborn (player);
}
*/


//
// G_PlayerFinishLevel
//  Can when a player completes a level.
//
void G_PlayerFinishLevel (int player)
{
    player_t*  p;

    p = &players[player];

    memset (p->powers, 0, sizeof (p->powers));

    p->mo->flags2 &= ~MF2_SHADOW;         // cancel invisibility
    p->extralight = 0;                  // cancel gun flashes
    p->fixedcolormap = 0;               // cancel ir gogles
    p->bonuscount = 0;
	p->starpostangle = 0;
	p->starposttime = 0;
	p->starpostx = 0;
	p->starposty = 0;
	p->starpostz = 0;
	p->starpostnum = 0;
	p->starpostbit = 0;

	if(rendermode == render_soft)
		V_SetPaletteLump("PLAYPAL"); // Reset the palette Tails 02-20-2002
}

void P_FindEmerald(player_t* player); // Tails 09-11-2002
//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//
void G_PlayerReborn (int player)
{
    player_t*   p;
	int         score; // Score Tails 03-09-2000
    int         killcount;
    int         itemcount;
    int         secretcount;
	int         lives; // Lives Tails 03-11-2000
	int			continues; // Tails
	int			xtralife;
	int			xtralife2;
	int			charability; // Tails
	int			normalspeed; // Tails
	int         thrustfactor; // Tails
	int         accelstart; // Tails
	int         acceleration; // Tails
	int         charspin; // Tails
	int         starttrans; // Tails
	int         endtrans; // Tails
	int         prefcolor; // Tails 12-15-2003
	int			tagit; // Tails 05-08-2001
	int			ctfteam; // Tails 08-03-2001
	int			tagcount; // Tails 08-17-2001
	int         starposttime; // Tails 07-04-2002
	int         starpostx; // Tails 07-04-2002
	int         starposty; // Tails 07-04-2002
	int         starpostz; // Tails 07-04-2002
	int			starpostnum; // Tails 07-05-2002
	int         starpostangle; // Tails 07-04-2002
	unsigned short starpostbit; // Tails 07-05-2002
	int         jumpfactor;
	int         laps; // Graue 11-18-2003
	int         exiting; // Graue 12-24-2003
	byte        mare;

    //from Boris
    int         skincolor;
    int         skin;                           //Fab: keep same skin
	boolean     autoaim;
#ifdef CLIENTPREDICTION2
    mobj_t      *spirit;
#endif

	// Graue 12-22-2003
	int oldnormalspeed, oldthrustfactor, oldaccelstart, oldacceleration;
	int circnormalspeed, circthrustfactor, circaccelstart, circacceleration;

	score = players[player].score; // Score Tails 03-09-2000
    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;
    lives       = players[player].lives; // Tails 03-11-2000
    continues   = players[player].continues; // Tails
	xtralife = players[player].xtralife;
	xtralife2 = players[player].xtralife2;
	tagit = players[player].tagit; // Tails 05-08-2001
	ctfteam = players[player].ctfteam; // Tails 08-03-2001
	tagcount = players[player].tagcount; // Tails 08-17-2001
	exiting = players[player].exiting; // Graue 12-24-2003

    //from Boris
    skincolor = players[player].skincolor;
    skin = players[player].skin;
	autoaim = players[player].autoaim_toggle;
	charability = players[player].charability; // Tails
	normalspeed = players[player].normalspeed; // Tails
	thrustfactor = players[player].thrustfactor; // Tails
	accelstart = players[player].accelstart; // Tails
	acceleration = players[player].acceleration; // Tails
	charspin = players[player].charspin; // Tails
	starttrans = players[player].starttranscolor; // Tails
	endtrans = players[player].endtranscolor; // Tails
	prefcolor = players[player].prefcolor; // Tails 12-15-2003

	// Graue 12-22-2003
	oldnormalspeed = players[player].oldnormalspeed;
	oldthrustfactor = players[player].oldthrustfactor;
	oldaccelstart = players[player].oldaccelstart;
	oldacceleration = players[player].oldacceleration;
	circnormalspeed = players[player].circnormalspeed;
	circthrustfactor = players[player].circthrustfactor;
	circaccelstart = players[player].circaccelstart;
	circacceleration = players[player].circacceleration;

	starposttime = players[player].starposttime; // Tails 07-04-2002
	starpostx = players[player].starpostx; // Tails 07-04-2002
	starposty = players[player].starposty; // Tails 07-04-2002
	starpostz = players[player].starpostz; // Tails 07-04-2002
	starpostnum = players[player].starpostnum; // Tails 07-05-2002
	starpostangle = players[player].starpostangle; // Tails 07-04-2002
	starpostbit = players[player].starpostbit; // Tails 07-05-2002
	jumpfactor = players[player].jumpfactor;
	laps = players[player].laps; // Graue 11-18-2003

	mare = players[player].mare;

#ifdef CLIENTPREDICTION2
    spirit = players[player].spirit;
#endif
    p = &players[player];
    memset (p, 0, sizeof(*p));

	players[player].score     = score; // Score Tails 03-09-2000
    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;
	players[player].lives = lives; // Tails 03-11-2000
	players[player].continues = continues; // Tails
	players[player].xtralife = xtralife;
	players[player].xtralife2 = xtralife2;
	players[player].tagit = tagit; // Tails 05-08-2001
	players[player].ctfteam = ctfteam; // Tails 08-03-2001
	players[player].tagcount = tagcount; // Tails 08-17-2001

    // save player config truth reborn
    players[player].skincolor = skincolor;
    players[player].skin = skin;
	players[player].autoaim_toggle = autoaim;
	players[player].charability = charability; // Tails
	players[player].normalspeed = normalspeed; // Tails
	players[player].thrustfactor = thrustfactor; // Tails
	players[player].accelstart = accelstart; // Tails
	players[player].acceleration = acceleration; // Tails
	players[player].charspin = charspin; // Tails
	players[player].starttranscolor = starttrans; // Tails
	players[player].endtranscolor = endtrans; // Tails
	players[player].prefcolor = prefcolor; // Tails 12-15-2003

	// Graue 12-22-2003
	players[player].oldnormalspeed = oldnormalspeed;
	players[player].oldthrustfactor = oldthrustfactor;
	players[player].oldaccelstart = oldaccelstart;
	players[player].oldacceleration = oldacceleration;
	players[player].circnormalspeed = circnormalspeed;
	players[player].circthrustfactor = circthrustfactor;
	players[player].circaccelstart = circaccelstart;
	players[player].circacceleration = circacceleration;

	players[player].starposttime = starposttime;
	players[player].starpostx = starpostx;
	players[player].starposty = starposty;
	players[player].starpostz = starpostz;
	players[player].starpostnum = starpostnum;
	players[player].starpostangle = starpostangle;
	players[player].starpostbit = starpostbit;
	players[player].jumpfactor = jumpfactor;
	players[player].laps = laps; // Graue 11-18-2003
	players[player].exiting = exiting; // Graue 12-24-2003

	players[player].mare = mare;

#ifdef CLIENTPREDICTION2
    players[player].spirit = spirit;
#endif

    p->usedown = p->attackdown = true;  // don't do anything immediately
    p->playerstate = PST_LIVE;
    p->health = 1; // 0 rings

	if(cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG
		|| cv_gametype.value == GT_CTF || cv_gametype.value == GT_CHAOS
		|| cv_gametype.value == GT_CIRCUIT)
		p->powers[pw_flashing] = flashingtics; // Babysitting deterrent Tails 09-02-2002

	// Scan for emeralds to hunt for Tails 09-11-2002
	P_FindEmerald(p);

	if(netgame || multiplayer)
	{
		if(p == &players[consoleplayer])
			S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, true);
	}
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//
boolean G_CheckSpot ( int           playernum,
                      mapthing_t*   mthing )
{
    fixed_t             x;
    fixed_t             y;
    subsector_t*        ss;
    int                 i;

    // added 25-4-98 : maybe there is no player start
    if(!mthing || mthing->type<0)
        return false;

    if (!players[playernum].mo)
    {
        // first spawn of level, before corpses
        for (i=0 ; i<playernum ; i++)
            // added 15-1-98 check if player is in game (mistake from id)
            if (playeringame[i] && players[i].mo
                && players[i].mo->x == mthing->x << FRACBITS
                && players[i].mo->y == mthing->y << FRACBITS)
                return false;
        return true;
    }

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    ss = R_PointInSubsector (x,y);

    if (!P_CheckPosition (players[playernum].mo, x, y) )
        return false;

    return true;
}


//
// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//
boolean G_DeathMatchSpawnPlayer (int playernum)
{
    int             i,j,n;

	if(numdmstarts)
	{
		n=64;

	    for (j=0 ; j<n ; j++)
		{
			i = P_Random() % numdmstarts;
			if (G_CheckSpot (playernum, deathmatchstarts[i]) )
			{
				//deathmatchstarts[i]->type = playernum+1;
	            P_SpawnPlayer (deathmatchstarts[i], playernum);
		        return true;
			}
	    }
	}

	// Use a coop start dependent on playernum Graue 12-23-2003
	CONS_Printf("No deathmatch start in this map - shifting to player starts to avoid crash...\n");

	if(numcoopstarts == 0)
		I_Error("There aren't enough starts in this map!\n");

	i = playernum % numcoopstarts;
	//playerstarts[i]->type = playernum+1;
	P_SpawnPlayer (playerstarts[i], playernum);
	playerstarts[i]->type = i+1;
	return true;
    
	// no good spot, so the player will probably get stuck
    //P_SpawnPlayer (playerstarts[playernum]);
    //    return true;
}

// G_CoopSpawnPlayer
//
void G_CoopSpawnPlayer (int playernum, boolean starpost)
{
    int i,j,n; // Tails

	// CTF Start Spawns Tails
	if(cv_gametype.value == GT_CTF)
	{
		switch(players[playernum].ctfteam)
		{
		case 1: // Red Team
			if( !numredctfstarts)
			{
				CONS_Printf("No Red Team start in this map, resorting to Deathmatch starts!\n");
				goto startdeath;
			}

			n = 32;

			for (j=0 ; j<n ; j++)
			{
				i = P_Random() % numredctfstarts;
				if (G_CheckSpot (playernum, redctfstarts[i]) )
				{
					//redctfstarts[i]->type = playernum+1;
					P_SpawnPlayer (redctfstarts[i], playernum);
					return;
				}
			}
			break;
		case 2: // Blue Team
			if(!numbluectfstarts)
			{
				CONS_Printf("No Blue Team start in this map, resorting to Deathmatch starts!\n");
				goto startdeath;
			}

			n = 32;

			for (j=0 ; j<n ; j++)
			{
				i = P_Random() % numbluectfstarts;
				if (G_CheckSpot (playernum, bluectfstarts[i]) )
				{
					//bluectfstarts[i]->type = playernum+1;
					P_SpawnPlayer (bluectfstarts[i], playernum);
					return;
				}
			}
			break;
		default:
startdeath:
			G_DeathMatchSpawnPlayer(playernum);
			return;
		}
	}

	// Tails 07-03-2002
	if(starpost)
	{
		P_SpawnStarpostPlayer(players[playernum].mo, playernum);
		return;
	}

    // no deathmatch use the spot
    if (G_CheckSpot (playernum, playerstarts[playernum]) )
    {
		// playerstarts[playernum]->type = playernum+1; // I'm paranoid, okay? Graue 12-23-2003
        P_SpawnPlayer (playerstarts[playernum], playernum);
        return;
    }

	// use the player start anyway if it exists Graue 12-23-2003
	if(playerstarts[playernum] && playerstarts[playernum]->type >= 0)
	{
		//playerstarts[playernum]->type = playernum+1;
		P_SpawnPlayer(playerstarts[playernum], playernum);
		return;
	}

	// resort to the player one start, and if there is none, we're screwed
	if(!playerstarts[0] || playerstarts[0]->type < 0)
		I_Error("Not enough starts in this map!\n");

	//playerstarts[0]->type = playernum+1;
	P_SpawnPlayer(playerstarts[0], playernum);
	//playerstarts[0]->type = 1;

	/* darn this broken crud Graue 12-23-2003

    // try to spawn at one of the other players spots
    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (G_CheckSpot (playernum, playerstarts[i]) )
        {
            playerstarts[i]->type = playernum+1;     // fake as other player
            P_SpawnPlayer (playerstarts[i]);
            playerstarts[i]->type = i+1;               // restore
            return;
        }
        // he's going to be inside something.  Too bad.
    }

	i = -1;
	while(i == -1)
	{
		i++;
		if(!(!playerstarts[i] || playerstarts[i]->type<0))
			break;
		if(i>=MAXPLAYERS)
		{
			i = 42;
			break;
		}
	}

	if(i == 42 || playerstarts[i] == NULL)
		I_Error("Not enough starts in this map!\n");

	playerstarts[i]->type = playernum+1;
	P_SpawnPlayer (playerstarts[i]); */
}

//
// G_DoReborn
//
void G_DoReborn (int playernum)
{
    player_t*  player = &players[playernum];
	boolean starpost = false;

    // boris comment : this test is like 'single player game'
    //                 all this kind of hidden variable must be removed
    if (countdowntimeup || (!multiplayer && cv_gametype.value == GT_COOP)) // Tails 03-13-2001
    {
        // reload the level from scratch
		if(countdowntimeup)
		{
			player->starpostangle = 0;
			player->starposttime = 0;
			player->starpostx = 0;
			player->starposty = 0;
			player->starpostz = 0;
			player->starpostnum = 0;
			player->starpostbit = 0;
		}
        G_DoLoadLevel (true);
    }
    else
    {
        // respawn at the start

		if(player->starposttime)
			starpost = true;

        // first dissasociate the corpse
        if(player->mo)
        {
            player->mo->player = NULL;
            player->mo->flags2 &= ~MF2_DONTDRAW;
			// Don't leave your carcass stuck
		    // 10-billion feet in the ground! Tails 10-07-2001
			P_SetMobjState(player->mo, S_DISS);
        }
        // spawn at random spot if in death match
        if (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CHAOS) // Tails 03-13-2001
        {
            if(G_DeathMatchSpawnPlayer (playernum))
				return;
        }

		// Star post support Tails 07-03-2002
		G_CoopSpawnPlayer(playernum, starpost);
    }
}

void G_AddPlayer( int playernum )
{
    player_t *p=&players[playernum];

    p->playerstate = PST_REBORN;
}

// DOOM Par Times
static const int pars[4][10] =
{
    {0},
    {0,30,75,120,90,165,180,180,30,165},
    {0,90,90,90,120,90,360,240,30,170},
    {0,90,45,90,150,90,90,165,30,135}
};

// DOOM II Par Times
static const int cpars[32] =
{
    30,90,120,120,90,150,120,120,270,90,        //  1-10
    210,150,150,150,210,150,420,150,210,150,    // 11-20
    240,150,180,150,150,300,330,420,300,180,    // 21-30
    120,30                                      // 31-32
};


//
// G_DoCompleted
//
boolean         secretexit;

void G_ExitLevel (void)
{
    if( gamestate==GS_LEVEL )
    {
        secretexit = false;
        gameaction = ga_completed;
    }
}

// Here's for the german edition.
void G_SecretExitLevel (void)
{
    // IF NO WOLF3D LEVELS, NO SECRET EXIT!
    if (W_CheckNumForName("map31")<0)
        secretexit = false;
    else
        secretexit = true;
    gameaction = ga_completed;
}

extern consvar_t cv_advancemap;

void G_DoCompleted (void)
{
    int             i;
	boolean	 gottoken; // I got it! Tails 08-17-2001
	gottoken = false; // Tails 08-17-2001

	tokenlist = 0; // Reset the list

    gameaction = ga_nothing;

    for (i=0 ; i<MAXPLAYERS ; i++)
        if (playeringame[i])
            G_PlayerFinishLevel (i);        // take away cards and stuff

    if (automapactive)
        AM_Stop ();

	S_StopSounds(); // Tails 12-06-2002

    wminfo.last = gamemap -1;

    // go to next level
    // wminfo.next is 0 biased, unlike gamemap
	if(nextmapoverride != 0)
		wminfo.next = nextmapoverride-1;
	else
		wminfo.next = mapheaderinfo[gamemap-1].nextlevel-1; //gamemap; Tails 04-08-2003
    
	if(cv_gametype.value == GT_COOP)
	{
		if(token)
		{
			if(!(emeralds & EMERALD1))
				wminfo.next = sstage_start-1; // Special Stage 1
			else if(!(emeralds & EMERALD2))
				wminfo.next = sstage_start; // Special Stage 2
			else if(!(emeralds & EMERALD3))
				wminfo.next = sstage_start+1; // Special Stage 3
			else if(!(emeralds & EMERALD4))
				wminfo.next = sstage_start+2; // Special Stage 4
			else if(!(emeralds & EMERALD5))
				wminfo.next = sstage_start+3; // Special Stage 5
			else if(!(emeralds & EMERALD6))
				wminfo.next = sstage_start+4; // Special Stage 6
			else if(!(emeralds & EMERALD7))
				wminfo.next = sstage_start+5; // Special Stage 7
			else
			{
				gottoken = false;
				goto skipit;
			}


			token--;
			gottoken = true;
		}
	}
skipit:
	if((emeralds & EMERALD1 && emeralds & EMERALD2 && emeralds & EMERALD3 && emeralds & EMERALD4 && 
		emeralds & EMERALD5 && emeralds & EMERALD6 && emeralds & EMERALD7)
		&& false/*(gamemap == 4 || (xmasmode && gamemap == 5))*/)
		wminfo.next = 21; // Castle Eggman Tails 12-23-2001
/*	else if (gamemap == 24)
		wminfo.next = 0;*/
/*	else if(xmasmode) // Tails 12-13-2001
	{
		if(gamemap == 5)
			wminfo.next = 0;
	}*/
/*	else if(gamemap == 4 && !gottoken) // Tails 08-16-2001
		wminfo.next = 0; // Tails 08-16-2001
*/
	if((gamemap >= sstage_start && gamemap <= sstage_end) && !gottoken)
	{
		wminfo.next = lastmap; // Exiting from a special stage? Go back to the game. Tails 08-11-2001
	}

	if(!(gamemap >= sstage_start && gamemap <= sstage_end)) // Remember last map
	{
		if(nextmapoverride != 0)
			lastmap = nextmapoverride-1;
		else
			lastmap = mapheaderinfo[gamemap-1].nextlevel-1; // Remember last map you were at for when you come out of the special stage! Tails 08-11-2001
	}

    wminfo.maxkills = totalkills;
    wminfo.maxitems = totalitems;
    wminfo.maxsecret = totalsecret;
    wminfo.maxfrags = 0;

    wminfo.partime = TICRATE*cpars[gamemap-1];

    wminfo.pnum = consoleplayer;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        wminfo.plyr[i].in = playeringame[i];
	    wminfo.plyr[i].sscore = players[i].fscore; // show da score man! Tails 03-09-2000
        wminfo.plyr[i].skills = players[i].killcount;
        wminfo.plyr[i].sitems = players[i].itemcount;
        wminfo.plyr[i].ssecret = players[i].secretcount;
    }

    gamestate = GS_INTERMISSION;
    automapactive = false;

    if (statcopy)
        memcpy (statcopy, &wminfo, sizeof(wminfo));

	if(!cv_advancemap.value && (netgame || multiplayer)
		&& (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG
		|| cv_gametype.value == GT_CTF || cv_gametype.value == GT_CHAOS))
		wminfo.next = wminfo.last; // Stay on the same map.

	mapcleared[gamemap-1] = true;

	if(skipstats)
		G_AfterIntermission();
	else
		WI_Start (&wminfo);
}


//
// G_NextLevel (WorldDone)
//
// init next level or go to the final scene
// called by end of intermision screen (wi_stuff)
void G_AfterIntermission(void)
{
	if(mapheaderinfo[gamemap-1].cutscenenum != 0)
	{
		// Start custom cutscene here
		F_StartCustomCutscene(mapheaderinfo[gamemap-1].cutscenenum-1);
	}
	else
		G_NextLevel();
}

void G_NextLevel (void)
{
    gameaction = ga_worlddone;
}

void G_DoWorldDone (void)
{
    // not in demo because demo have the mapcommand on it
    if(server && !demoplayback) 
    {
        if(cv_gametype.value == GT_COOP)
            // don't reset player between maps
            COM_BufAddText (va("map \"%s\" -noresetplayers -fromcode\n",G_BuildMapName(wminfo.next+1)));
        else
            // resetplayer in deathmatch for more equality
            COM_BufAddText (va("map \"%s\" -fromcode\n",G_BuildMapName(wminfo.next+1)));
    }
    
    gameaction = ga_nothing;
}

//
// G_LoadGameSettings
//
// Sets a tad of default info we need. // Tails 05-19-2003
void G_LoadGameSettings(void)
{
	// defaults
	spstage_start = 1;
	sstage_start = 50;
	sstage_end = 56;
}

#define GAMEDATAFILENAME "gamedata.dat"
#define GAMEDATASIZE (1*8192)
// G_LoadGameData
// Loads the main data file, which stores information such as emblems found, etc.
void G_LoadGameData()
{
    int         length;
	int              i;

    length = FIL_ReadFile (GAMEDATAFILENAME, &savebuffer);
    if (!length)
    {
        // Aw, no game data. Their loss!
        return;
    }

    save_p = savebuffer;

    gottenemblems   = (READLONG(save_p)-15)/3;
	foundeggs      = (READLONG(save_p)-30)/5;
	totalplaytime   = READULONG(save_p);
	grade           = (READLONG(save_p)-75)/4;
	veryhardcleared = (READBYTE(save_p)-3);

	// Initialize the table
	memset(timedata, 0, sizeof(timeattack_t) * NUMMAPS);

	for(i=0; i<99; i++) // Temporary hack because 100-1035 aren't needed yet Graue 12-03-2003
		timedata[i].time = READULONG(save_p);

	// Aha! Someone's been screwing with the save file!
	if(veryhardcleared > 1)
		I_Error("Corrupt game data file.\nDelete gamedata.dat\nand try again.");

    // done
    Z_Free (savebuffer);
}

// G_SaveGameData
// Saves the main data file, which stores information such as emblems found, etc.
void G_SaveGameData()
{
    int         length;
	int              i;

    save_p = savebuffer = (byte *)malloc(GAMEDATASIZE);
    if(!save_p)
    {
        CONS_Printf ("No More free memory for savegame\n");
        return;
    }

    WRITELONG(save_p, (gottenemblems*3)+15);
	WRITELONG(save_p, (foundeggs*5)+30);
	WRITEULONG(save_p, totalplaytime);
	WRITELONG(save_p, (grade*4)+75);
	WRITEBYTE(save_p, veryhardcleared+3);

	for(i=0; i<NUMMAPS; i++)
		WRITEULONG(save_p, timedata[i].time);

    length = save_p - savebuffer;

    FIL_WriteFile (GAMEDATAFILENAME, savebuffer, length);
    free(savebuffer);
}

//
// G_InitFromSavegame
// Can be called by the startup code or the menu task.
//
void G_LoadGame (int slot)
{
    COM_BufAddText(va("load %d\n",slot));
}

#define VERSIONSIZE             16

void G_DoLoadGame (int slot) // Working on lots of garbage here Tails 06-10-2001
{
    int         length;
    char        vcheck[VERSIONSIZE];
    char        savename[255];

    sprintf(savename, savegamename, slot);

    length = FIL_ReadFile (savename, &savebuffer);
    if (!length)
    {
        CONS_Printf ("Couldn't read file %s", savename);
        return;
    }

    // skip the description field
    save_p = savebuffer + SAVESTRINGSIZE;
    
    memset (vcheck,0,sizeof(vcheck));
    sprintf (vcheck,"version %i",VERSION);
    if (strcmp (save_p, vcheck))
    {
        M_StartMessage ("Save game from different version\n\nPress ESC\n",NULL,MM_NOTHING);
        return;                         // bad version
    }
    save_p += VERSIONSIZE;

    if(demoplayback)  // reset game engine
        G_StopDemo();

    paused        = false;
    automapactive = false;

    // dearchive all the modifications
    if( !P_LoadGame() )
    {
        M_StartMessage ("savegame file corrupted\n\nPress ESC\n", NULL, MM_NOTHING);
        Command_ExitGame_f();
        Z_Free (savebuffer);
        return;
    }

    gameaction = ga_nothing;
    gamestate = GS_LEVEL;
    displayplayer = consoleplayer;

    // done
    Z_Free (savebuffer);

    multiplayer = playeringame[1];
    if(playeringame[1] && !netgame)
        CV_SetValue(&cv_splitscreen,1);

    if (setsizeneeded)
        R_ExecuteSetViewSize ();

    // draw the pattern into the back screen
    R_FillBackScreen ();
    CON_ToggleOff ();
}

//
// G_SaveGame
// Called by the menu task.
// Description is a 24 byte text string
//
void G_SaveGame ( int   slot, char* description )
{
    if (server)
        COM_BufAddText(va("save %d \"%s\"\n",slot,description));
}

void G_DoSaveGame (int   savegameslot, char* savedescription)
{
    char        name2[VERSIONSIZE];
    char        description[SAVESTRINGSIZE];
    int         length;
    char        name[256];

    gameaction = ga_nothing;

    sprintf(name, savegamename, savegameslot);

    gameaction = ga_nothing;

    save_p = savebuffer = (byte *)malloc(SAVEGAMESIZE);
    if(!save_p)
    {
        CONS_Printf ("No More free memory for savegame\n");
        return;
    }

    strcpy(description,savedescription);
    description[SAVESTRINGSIZE]=0;
    WRITEMEM(save_p, description, SAVESTRINGSIZE);
    memset (name2,0,sizeof(name2));
    sprintf (name2,"version %i",VERSION);
    WRITEMEM(save_p, name2, VERSIONSIZE);

    P_SaveGame();

    length = save_p - savebuffer;
    if (length > SAVEGAMESIZE)
        I_Error ("Savegame buffer overrun");
    FIL_WriteFile (name, savebuffer, length);
    free(savebuffer);

    gameaction = ga_nothing;

    players[consoleplayer].message = GGSAVED;

    // draw the pattern into the back screen
    R_FillBackScreen ();
}

//
// G_InitNew
//  Can be called by the startup code or the menu task,
//  consoleplayer, displayplayer, playeringame[] should be set.
//
// Boris comment : single player start game
void G_DeferedInitNew (skill_t skill, char* mapname, int pickedchar, boolean StartSplitScreenGame)
{
	int i;
    paused        = false;

	for(i=0; i<NUMMAPS; i++)
		mapcleared[i] = false;

    if( demoplayback )
        COM_BufAddText ("stopdemo\n");

    // this leave the actual game if needed
    SV_StartSinglePlayerServer();

    COM_BufAddText (va("splitscreen %d;gametype 0;fastmonsters 0;" // Tails 03-13-2001
                       "respawnmonsters 0;timelimit 0;fraglimit 0\n",
                       StartSplitScreenGame));

	// Tails 03-02-2002
	SetSavedSkin(0, pickedchar, atoi(skins[pickedchar].prefcolor));

    COM_BufAddText (va("map \"%s\" -skill %d -monsters 1 -fromcode\n",mapname,skill+1));
}

//
// This is the map command interpretation something like Command_Map_f
//
// called at : map cmd execution, doloadgame, doplaydemo
void G_InitNew (skill_t skill, char* mapname, boolean resetplayer)
{
	int i; // Tails

	for(i=0; i<NUMMAPS; i++)
		mapcleared[i] = false;

    if (paused)
    {
        paused = false;
        S_ResumeSound ();
    }

	if(skill < sk_easy)
		skill = sk_easy;
    else if (skill > sk_insane)
        skill = sk_insane;

	if(veryhardcleared == false && skill == sk_insane) // Nice try, haxor.
		skill = sk_nightmare;

    M_ClearRandom ();

    if( server && skill == sk_nightmare )
    {
        //init extra naughty crap here. Tails
    }

    if( resetplayer )
	{
		// Clear a bunch of variables Tails 08-05-2002
		lastmap = 0; // Last level you were at (returning from special stages).
		emeralds = 0;
		tokenlist = 0; // Tails 12-18-2003
		token = 0; // Number of tokens collected in a level Tails 08-11-2001
		sstimer = 0; // Time allotted in the special stage Tails 08-11-2001
		bluescore = 0; // Team Scores Tails 07-31-2001
		redscore = 0; // Team Scores Tails 07-31-2001
		countdown = 0;
		countdown2 = 0;

        for (i=0 ; i<MAXPLAYERS ; i++)
		{
            players[i].playerstate = PST_REBORN;

			players[i].starpostangle = 0;
			players[i].starpostnum = 0;
			players[i].starposttime = 0;
			players[i].starpostx = 0;
			players[i].starposty = 0;
			players[i].starpostz = 0;
			players[i].starpostbit = 0;

			// start set lives/continues via game skill Tails 03-11-2000
			switch(skill)
			{
				case sk_insane:
					players[i].lives = 1;
					players[i].continues = 0;
					break;
				case sk_nightmare:
				case sk_hard:
				case sk_medium:
					players[i].lives = 3;
					players[i].continues = 1;
					break;
				case sk_easy:
					players[i].lives = 5;
					players[i].continues = 2;
					break;
				case sk_baby:
					players[i].lives = 9;
					players[i].continues = 5;
					break;
				default: // Oops!?
					CONS_Printf("ERROR: GAME SKILL UNDETERMINED!");
					break;
			}
			// end set lives/continues via game skill Tails 03-11-2000

			if(cv_gametype.value != GT_MATCH) // Don't do this in match for some reason - bad logic fix by Graue 12-13-2003
				players[i].score = 0; // Set score to 0 Tails 03-10-2000

			players[i].xtralife = players[i].xtralife2 = 0;
		}

		// Tails 03-02-2002
		if(!(cv_debug || devparm) && !(multiplayer || netgame))
			SetSavedSkin(0, players[0].skin, players[0].prefcolor);
	}

    // for internal maps only
    if (FIL_CheckExtension(mapname))
    {
        // external map file
        strncpy (gamemapname, mapname, MAX_WADPATH);
        gameepisode = 1;
        gamemap = 1;
    }
    else
    {
        // internal game map
        // well this  check is useless because it is done before (d_netcmd.c::command_map_f)
        // but in case of for demos....
        if (W_CheckNumForName(mapname)==-1)
        {
            CONS_Printf("\2Internal game map '%s' not found\n"
                        "(use .wad extension for external maps)\n",mapname);
            Command_ExitGame_f();
            return;
        }

        gamemapname[0] = 0;             // means not an external wad file

        gamemap = mapnumber(mapname[3], mapname[4]);  // get xx out of MAPxx
        gameepisode = 1;
    }

    gameskill      = skill;
    playerdeadview = false;
    automapactive  = false;

    G_DoLoadLevel (resetplayer);
}

//
// DEMO RECORDING
//

#define ZT_FWD          0x01
#define ZT_SIDE         0x02
#define ZT_ANGLE        0x04
#define ZT_BUTTONS      0x08
#define ZT_AIMING       0x10
#define ZT_CHAT         0x20    // no more used
#define ZT_EXTRADATA    0x40
#define DEMOMARKER      0x80    // demoend

ticcmd_t oldcmd[MAXPLAYERS];

void G_ReadDemoTiccmd (ticcmd_t* cmd,int playernum)
{
    char ziptic=*demo_p++;

    if (*demo_p == DEMOMARKER)
    {
        // end of demo data stream
        G_CheckDemoStatus ();
        return;
    }

    if(ziptic & ZT_FWD)
        oldcmd[playernum].forwardmove = READCHAR(demo_p);
    if(ziptic & ZT_SIDE)
        oldcmd[playernum].sidemove = READCHAR(demo_p);
    if(ziptic & ZT_ANGLE)
    {
        oldcmd[playernum].angleturn = READSHORT(demo_p);
    }
    if(ziptic & ZT_BUTTONS)
        oldcmd[playernum].buttons = READBYTE(demo_p);
    if(ziptic & ZT_AIMING)
    {
        oldcmd[playernum].aiming = READSHORT(demo_p);
    }
    if(ziptic & ZT_CHAT)
        demo_p++;
    if(ziptic & ZT_EXTRADATA)
        ReadLmpExtraData(&demo_p,playernum);
    else
        ReadLmpExtraData(0,playernum);

    memcpy(cmd,&(oldcmd[playernum]),sizeof(ticcmd_t));
}

void G_WriteDemoTiccmd (ticcmd_t* cmd,int playernum)
{
    char ziptic=0;
    byte *ziptic_p;

    ziptic_p=demo_p++;  // the ziptic
                        // write at the end of this function

    if(cmd->forwardmove != oldcmd[playernum].forwardmove)
    {
        *demo_p++ = cmd->forwardmove;
        oldcmd[playernum].forwardmove = cmd->forwardmove;
        ziptic|=ZT_FWD;
    }

    if(cmd->sidemove != oldcmd[playernum].sidemove)
    {
        *demo_p++ = cmd->sidemove;
        oldcmd[playernum].sidemove=cmd->sidemove;
        ziptic|=ZT_SIDE;
    }

    if(cmd->angleturn != oldcmd[playernum].angleturn)
    {
        *(short *)demo_p = cmd->angleturn;
        demo_p +=2;
        oldcmd[playernum].angleturn=cmd->angleturn;
        ziptic|=ZT_ANGLE;
    }

    if(cmd->buttons != oldcmd[playernum].buttons)
    {
        *demo_p++ = cmd->buttons;
        oldcmd[playernum].buttons=cmd->buttons;
        ziptic|=ZT_BUTTONS;
    }

    if(cmd->aiming != oldcmd[playernum].aiming)
    {
        *(short *)demo_p = cmd->aiming;
        demo_p+=2;
        oldcmd[playernum].aiming=cmd->aiming;
        ziptic|=ZT_AIMING;
    }

    if(AddLmpExtradata(&demo_p,playernum))
        ziptic|=ZT_EXTRADATA;

    *ziptic_p=ziptic;
    //added:16-02-98: attention here for the ticcmd size!
    // latest demos with mouse aiming byte in ticcmd
    if (ziptic_p > demoend - (5*MAXPLAYERS))
    {
        G_CheckDemoStatus ();   // no more space
        return;
    }

//  don't work in network the consistency is not copyed in the cmd
//    demo_p = ziptic_p;
//    G_ReadDemoTiccmd (cmd,playernum);         // make SURE it is exactly the same
}



//
// G_RecordDemo
//
void G_RecordDemo (char* name)
{
    int             i;
    int             maxsize;

    strcpy (demoname, name);
    strcat (demoname, ".lmp");
    maxsize = 10240*1024;//0x20000;
    i = M_CheckParm ("-maxdemo");
    if (i && i<myargc-1)
        maxsize = atoi(myargv[i+1])*1024;
    demobuffer = Z_Malloc (maxsize,PU_STATIC,NULL);
    demoend = demobuffer + maxsize;

    demorecording = true;
}


void G_BeginRecording (void)
{
    int             i;

    demo_p = demobuffer;

    *demo_p++ = VERSION;
    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
//    *demo_p++ = cv_deathmatch.value;     // just to be compatible with old demo (no more used)
	*demo_p++ = cv_gametype.value; // Game Types Tails 03-13-2001
	*demo_p++ = cv_analog.value; // Analog Tails 06-22-2001
	*demo_p++ = cv_analog2.value; // Tails 12-16-2002
//    *demo_p++ = nomonsters;
    *demo_p++ = consoleplayer;
    *demo_p++ = cv_timelimit.value;      // just to be compatible with old demo (no more used)
    *demo_p++ = multiplayer;             // 1.31

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if(playeringame[i])
          *demo_p++ = 1;
        else
          *demo_p++ = 0;
    }

    memset(oldcmd,0,sizeof(oldcmd));
}


//
// G_PlayDemo
//

void G_DeferedPlayDemo (char* name)
{
    COM_BufAddText("playdemo \"");
    COM_BufAddText(name);
    COM_BufAddText("\"\n");
}


//
//  Start a demo from a .LMP file or from a wad resource (eg: DEMO1)
//
void G_DoPlayDemo (char *defdemoname)
{
    skill_t skill;
    int     i, episode, map;

//
// load demo file / resource
//

    //it's an internal demo
    if ((i=W_CheckNumForName(defdemoname)) == -1)
    {
        FIL_DefaultExtension(defdemoname,".lmp");
        if (!FIL_ReadFile (defdemoname, &demobuffer) )
        {
            CONS_Printf ("\2ERROR: couldn't open file '%s'.\n", defdemoname);
            gameaction = ga_nothing;
			return;
        }
        demo_p = demobuffer;
    }
    else
        demobuffer = demo_p = W_CacheLumpNum (i, PU_STATIC);

//
// read demo header
//

    gameaction = ga_nothing;

    skill       = *demo_p++;
    episode     = *demo_p++;
    map         = *demo_p++;

    demo_p++;

    demo_p++;

    demo_p++;

//    nomonsters  = *demo_p++;

    //added:08-02-98: added displayplayer because the status bar links
    // to the display player when playing back a demo.
    displayplayer = consoleplayer = *demo_p++;

     //added:11-01-98:
    //  support old v1.9 demos with ONLY 4 PLAYERS ! Man! what a shame!!!

        demo_p++;

        multiplayer = *demo_p++;

        for (i=0 ; i<32 ; i++)
            playeringame[i] = *demo_p++;

#if MAXPLAYERS>32
#error Please add support for old lmps
#endif

    memset(oldcmd,0,sizeof(oldcmd));

    // wait map command in the demo
    gamestate = wipegamestate = GS_WAITINGPLAYERS;

    demoplayback = true;
}

//
// G_TimeDemo
//             NOTE: name is a full filename for external demos
//
static int restorecv_vidwait;

void G_TimeDemo (char* name)
{
    nodrawers = M_CheckParm ("-nodraw");
    noblit = M_CheckParm ("-noblit");
    restorecv_vidwait = cv_vidwait.value;
    if( cv_vidwait.value )
        CV_Set( &cv_vidwait, "0");
    timingdemo = true;
    singletics = true;
    framecount = 0;
    demostarttime = I_GetTime ();
    G_DeferedPlayDemo (name);
}


void G_DoneLevelLoad(void)
{
    CONS_Printf("Load Level in %f sec\n",(float)(I_GetTime()-demostarttime)/TICRATE);
    framecount = 0;
    demostarttime = I_GetTime ();
}

/*
===================
=
= G_CheckDemoStatus
=
= Called after a death or level completion to allow demos to be cleaned up
= Returns true if a new demo loop action will take place
===================
*/

// reset engine variable set for the demos
// called from stopdemo command, map command, and g_checkdemoStatus.
void G_StopDemo(void)
{
    Z_Free (demobuffer);
    demoplayback  = false;
    timingdemo = false;
    singletics = false;

    gamestate=wipegamestate=GS_NULL;
    SV_StopServer();
//    SV_StartServer();
    SV_ResetServer();
}

boolean G_CheckDemoStatus (void)
{
    if (timingdemo)
    {
        int time;
        float f1,f2;
        time = I_GetTime () - demostarttime;
        if(!time) return true;
        G_StopDemo ();
        timingdemo = false;
        f1=time;
        f2=framecount*TICRATE;
        CONS_Printf ("timed %i gametics in %i realtics\n"
                     "%f seconds, %f avg fps\n"
                     ,leveltime,time,f1/TICRATE,f2/f1);
        if( restorecv_vidwait != cv_vidwait.value )
            CV_SetValue(&cv_vidwait, restorecv_vidwait);
        D_AdvanceDemo ();
        return true;
    }

    if (demoplayback)
    {
        if (singledemo)
            I_Quit ();
        G_StopDemo();
        D_AdvanceDemo ();
        return true;
    }

    if (demorecording)
    {
        *demo_p++ = DEMOMARKER;
        FIL_WriteFile (demoname, demobuffer, demo_p - demobuffer);
        Z_Free (demobuffer);
        demorecording = false;

        CONS_Printf("\2Demo %s recorded\n",demoname);
        return true;
        //I_Quit ();
    }

    return false;
}
