// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: g_game.h,v 1.10 2001/12/15 18:41:35 hurdler Exp $
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
// $Log: g_game.h,v $
// Revision 1.10  2001/12/15 18:41:35  hurdler
// small commit, mainly splitscreen fix
//
// Revision 1.9  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.8  2001/01/25 22:15:42  bpereira
// added heretic support
//
// Revision 1.7  2000/11/26 20:36:14  hurdler
// Adding autorun2
//
// Revision 1.6  2000/11/02 19:49:35  bpereira
// no message
//
// Revision 1.5  2000/10/21 08:43:29  bpereira
// no message
//
// Revision 1.4  2000/10/01 10:18:17  bpereira
// no message
//
// Revision 1.3  2000/04/07 23:11:17  metzgermeister
// added mouse move
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//   
// 
//-----------------------------------------------------------------------------


#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"

//added:11-02-98: yeah now you can change it!
// changed to 2d array 19990220 by Kin
extern char          player_names[MAXPLAYERS][MAXPLAYERNAME];
extern char*                    team_names[];

extern  boolean xmasmode; // Xmas mode Tails 12-02-2001
extern  boolean xmasoverride;
extern  boolean mariomode; // Mario mode Tails 12-18-2001
extern  boolean eastermode; // Easter mode! Tails 02-06-2003
extern  char      gamemapname[128];

extern  player_t        players[MAXPLAYERS];
extern  boolean   playeringame[MAXPLAYERS];

// ======================================
// DEMO playback/recording related stuff.
// ======================================

// demoplaying back and demo recording
extern  boolean demoplayback;
extern  boolean demorecording;
extern  boolean   timingdemo;       

// Quit after playing a demo from cmdline.
extern  boolean         singledemo;

// gametic at level start
extern  tic_t     levelstarttic;  

// used in game menu
extern consvar_t  cv_crosshair;
extern consvar_t  cv_crosshair2;
//extern consvar_t  cv_crosshairscale;
extern consvar_t  cv_invertmouse;
extern consvar_t  cv_alwaysfreelook;
extern consvar_t  cv_mousemove;
extern consvar_t  cv_showmessages;
extern consvar_t cv_joystickfreelook;

void Command_Turbo_f (void);

// build an internal map name ExMx MAPxx from episode,map numbers
char* G_BuildMapName (int map);
void G_BuildTiccmd (ticcmd_t* cmd, int realtics);
void G_BuildTiccmd2(ticcmd_t* cmd, int realtics);

//added:22-02-98: clip the console player aiming to the view
short G_ClipAimingPitch (int* aiming);

extern angle_t localangle,localangle2;
extern int     localaiming,localaiming2; // should be a angle_t but signed


//
// GAME
//
void    G_DoReborn (int playernum);
boolean G_DeathMatchSpawnPlayer (int playernum);
void G_CoopSpawnPlayer (int playernum, boolean starpost);
void    G_PlayerReborn (int player);

void G_InitNew (skill_t skill, char* mapname, boolean resetplayer);

// Can be called by the startup code or M_Responder.
// A normal game starts at map 1,
// but a warp test can start elsewhere
void G_DeferedInitNew (skill_t skill, char* mapname, int pickedchar, boolean StartSplitScreenGame); // Tails 03-02-2002
void G_DoLoadLevel (boolean resetplayer);

void G_DeferedPlayDemo (char* demo);

// Can be called by the startup code or M_Responder,
// calls P_SetupLevel or W_EnterWorld.
void G_LoadGame (int slot);
void G_DoLoadGame (int slot);


// Called by M_Responder.
void G_DoSaveGame(int slot, char* description);
void G_SaveGame  (int slot, char* description);

// Only called by startup code.
void G_RecordDemo (char* name);

void G_BeginRecording (void);

void G_DoPlayDemo (char *defdemoname);
void G_TimeDemo (char* name);
void G_DoneLevelLoad(void);
void G_StopDemo(void);
boolean G_CheckDemoStatus (void);

void G_ExitLevel (void);
void G_SecretExitLevel (void);

void G_NextLevel (void);
void G_AfterIntermission(void);

void G_Ticker (void);
boolean G_Responder (event_t*   ev);

void G_AddPlayer( int playernum );

#endif
