// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: doomstat.h,v 1.12 2001/07/16 22:35:40 bpereira Exp $
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
// $Log: doomstat.h,v $
// Revision 1.12  2001/07/16 22:35:40  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.11  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.10  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.9  2001/02/24 13:35:19  bpereira
// no message
//
// Revision 1.8  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.7  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.6  2000/10/21 08:43:28  bpereira
// no message
//
// Revision 1.5  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.4  2000/08/10 19:58:04  bpereira
// no message
//
// Revision 1.3  2000/08/10 14:53:10  ydario
// OS/2 port
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//   All the global variables that store the internal state.
//   Theoretically speaking, the internal state of the engine
//    should be found by looking at the variables collected
//    here, and every relevant module will have to include
//    this header file.
//   In practice, things are a bit messy.
//
//-----------------------------------------------------------------------------


#ifndef __D_STATE__
#define __D_STATE__


// We need globally shared data structures,
//  for defining the global state variables.
#include "doomdata.h"

// We need the player data structure as well.
#include "d_player.h"
#include "d_clisrv.h"

// =============================
// Selected skill type, map etc.
// =============================

// Selected by user.
extern  skill_t         gameskill;
extern  byte            gameepisode;
extern  short            gamemap;

// Set if homebrew PWAD stuff has been added.
extern  boolean modifiedgame;

// Nightmare mode flag, single player.
// extern  boolean         respawnmonsters;

// Netgame? only true in a netgame
extern  boolean         netgame;
// Only true if >1 player. netgame => multiplayer but not (multiplayer=>netgame)
extern  boolean         multiplayer;

extern consvar_t		cv_gametype; // Tails 03-13-2001
extern consvar_t		cv_timetic; // Tails 04-01-2001
extern boolean		cv_debug; // Tails 06-17-2001
extern consvar_t		cv_autoctf; // Tails 07-22-2001
extern consvar_t		cv_forceskin; // Tails 04-30-2002


// ========================================
// Internal parameters for sound rendering.
// ========================================

extern boolean         nomusic; //defined in d_main.c
extern boolean         nosound;
extern boolean         nofmod; // Tails 11-21-2002

// =========================
// Status flags for refresh.
// =========================
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern  boolean statusbaractive;

extern  boolean menuactive;     // Menu overlayed?
extern  boolean paused;         // Game Pause?

extern  boolean         nodrawers;
extern  boolean         noblit;

extern  int             viewwindowx;
extern  int             viewwindowy;
extern  int             viewheight;
extern  int             viewwidth;
extern  int             scaledviewwidth;



// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern  int     viewangleoffset;

// Player taking events, and displaying.
extern  int     consoleplayer;
extern  int     displayplayer;
extern  int     secondarydisplayplayer; // for splitscreen

//added:16-01-98: player from which the statusbar displays the infos.
extern  int     statusbarplayer;


// ============================================
// Statistics on a given map, for intermission.
// ============================================
//
extern int spstage_start;
extern int sstage_start;
extern int sstage_end;

extern tic_t countdowntimer;
extern boolean mapcleared[NUMMAPS];
extern boolean countdowntimeup;

typedef struct
{
  byte numpics;
  char picname[8][9];
  boolean pichires[8];
  char text[512]; // This should be PLENTY. I don't even think you can fit that many on the screen!
  unsigned short xcoord[8];
  unsigned short ycoord[8];
  unsigned short picduration[8];
  short musicslot;
  unsigned short textxpos;
  unsigned short textypos;
} scene_t;

typedef struct
{
	scene_t scene[128]; // 128 scenes per cutscene.
	int numscenes; // Number of scenes in this cutscene
} cutscene_t;

extern cutscene_t cutscenes[128];

extern int nextmapoverride; // Not a byte! Graue 12-31-2003
extern boolean skipstats;

extern  int     totalkills;
extern  int     totalitems;
extern  int     totalsecret;

extern  int     totalrings; //  Total # of rings in a level Tails 08-11-2001

// Fun extra stuff Tails 08-05-2002
extern int      lastmap; // Last level you were at (returning from special stages).

extern mapthing_t* rflagpoint; // Original flag spawn location Tails 08-02-2001
extern mapthing_t* bflagpoint; // Original flag spawn location Tails 08-02-2001
#define MF_REDFLAG  1
#define MF_BLUEFLAG 2

#define LEVELARRAYSIZE 1035+2
extern char lvltable[LEVELARRAYSIZE+3][64];

extern boolean twodlevel;

typedef struct
{
	char lvlttl[33];
	char interscreen[9];// Not supported in old format
	byte actnum;
	short typeoflevel;
	short nextlevel;
	short musicslot;
	byte forcecharacter;
	byte weather;
	short skynum;
	char scriptname[255];// Not supported in old format
	boolean scriptislump; // Not supported in old format
	byte cutscenenum;// Not supported in old format
	short countdown;// Not supported in old format
	boolean nozone;
} mapheader_t;
// Map Header Information Tails 04-08-2003

extern mapheader_t mapheaderinfo[NUMMAPS];

#define TOL_COOP        1
#define TOL_RACE        2
#define TOL_MATCH       4
#define TOL_TAG         8
#define TOL_CTF        16
#define TOL_CHAOS      32
#define TOL_NIGHTS     64
#define TOL_ADVENTURE 128
#define TOL_MARIO     256
#define TOL_2D        512
#define TOL_XMAS     1024
#define TOL_GOLF     2048
#define TOL_MEGAMAN  4096
#define TOL_CIRCUIT  8192 // Graue 11-15-2003

// Emeralds stored as bits to throw savegame hackers off. Tails 09-17-2002
extern    unsigned short emeralds;
extern    tic_t    totalplaytime; // Tails 12-08-2002
#define EMERALD1 1
#define EMERALD2 2
#define EMERALD3 4
#define EMERALD4 8
#define EMERALD5 16
#define EMERALD6 32
#define EMERALD7 64
#define EMERALD8 256 // ??

extern tic_t playerchangedelay;
extern boolean delayoverride;

#define NUMEMBLEMS 20
extern  int gottenemblems;

#define NUMEGGS 12
extern  int foundeggs;

typedef struct
{
  signed short x;
  signed short y;
  signed short z;

  byte    player;

  int flagnum;
  byte level;
} emblem_t;

extern emblem_t emblemlocations[NUMEMBLEMS-2];

extern emblem_t egglocations[NUMEGGS]; // Easter eggs... literally!

// Tails 12-08-2002
typedef struct
{
  tic_t time;
} timeattack_t;

extern timeattack_t timedata[NUMMAPS];

extern	int token; // Number of tokens collected in a level Tails 08-11-2001
extern  int tokenlist; // List of tokens collected Tails 12-18-2003
extern  byte tokenbits; // Used for setting token bits Tails 12-18-2003
extern	int	sstimer; // Time allotted in the special stage Tails 08-11-2001
extern  int bluescore; // Team Scores Tails 07-31-2001
extern  int redscore; // Team Scores Tails 07-31-2001

// Powerup durations Tails 07-26-2003
extern int invulntics;
extern int sneakertics;
extern int flashingtics;
extern int tailsflytics;
extern int underwatertics;
extern int spacetimetics;
extern int extralifetics;
// NiGHTS Powerups Tails 12-15-2003
extern int paralooptics;
extern int helpertics;

extern byte introtoplay;

// 'Golf' mode Tails 07-02-2003
extern int par;

extern mobj_t* hunt1;
extern mobj_t* hunt2;
extern mobj_t* hunt3;

// For racing
extern int countdown;
extern int countdown2;

// Tails 08-20-2002
extern fixed_t gravity;

// Grading Tails 08-13-2002
// 0 = No grade
// 1 = F
// 2 = E
// 3 = D
// 4 = C
// 5 = B
// 6 = A
// 7 = A+
extern int grade;

extern boolean veryhardcleared; // Tails 05-19-2003

// ===========================
// Internal parameters, fixed.
// ===========================
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern  tic_t           gametic;
#ifdef CLIENTPREDICTION2
extern  tic_t           localgametic;
#else
#define localgametic  leveltime
#endif

// Player spawn spots.
extern  mapthing_t      *playerstarts[MAXPLAYERS];
extern  mapthing_t      *bluectfstarts[MAXPLAYERS]; // CTF Tails 08-04-2001
extern  mapthing_t      *redctfstarts[MAXPLAYERS]; // CTF Tails 08-04-2001

// Intermission stats.
// Parameters for world map / intermission.
extern  wbstartstruct_t         wminfo;


// =====================================
// Internal parameters, used for engine.
// =====================================
//

// File handling stuff.
extern  char            basedefault[1024];

#ifdef __MACOS__
#define DEBFILE(msg) I_OutputMsg(msg)
extern  FILE*           debugfile;
#else
#define DEBUGFILE
#ifdef DEBUGFILE
#define DEBFILE(msg) { if(debugfile) fputs(msg,debugfile); }
extern  FILE*           debugfile;
#else
#define DEBFILE(msg) {}
extern  FILE*           debugfile;
#endif
#endif //__MACOS__


// if true, load all graphics at level load
extern  boolean         precache;


// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern  gamestate_t     wipegamestate;

//?
// debug flag to cancel adaptiveness
extern  boolean         singletics;

// =============
// Netgame stuff
// =============


//extern  ticcmd_t        localcmds[BACKUPTICS];

extern  ticcmd_t        netcmds[BACKUPTICS][MAXPLAYERS];

// Gametype stuff Graue 12-13-2003
#define GT_COOP 0
#define GT_MATCH 1
#define GT_RACE 2
#define GT_TAG 3
#define GT_CTF 4
#define GT_CHAOS 5
#define GT_CIRCUIT 6

#endif //__D_STATE__
