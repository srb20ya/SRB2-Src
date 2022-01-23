// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_saveg.c,v 1.9 2000/07/01 09:23:49 bpereira Exp $
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
// $Log: p_saveg.c,v $
// Revision 1.9  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.8  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.7  2000/04/15 22:12:57  stroggonmeth
// Minor bug fixes
//
// Revision 1.6  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.5  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
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
//      Archiving: SaveGame I/O.
//
//-----------------------------------------------------------------------------

// REWROTE THIS WHOLE SUCKA! Tails 06-10-2001

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_setup.h"
#include "byteptr.h"
#include "r_things.h" // Tails 03-26-2001

byte*           save_p;

// Pads save_p to a 4-byte boundary
//  so that the load/save works on SGI&Gecko.
#ifdef SGI
// BP: this stuff isn't be removed but i think it will no more work
//     anyway what processor can't read/write unaligned data ?
#define PADSAVEP()      save_p += (4 - ((int) save_p & 3)) & 3
#else
#define PADSAVEP()
#endif

// BP: damned this #if don't work ! why ?
#if NUMWEAPONS > 8
#error please update the player_saveflags enum
#endif

typedef enum {
 // weapons   = 0x01ff,
    BACKPACK  = 0x0200,
    ORIGNWEAP = 0x0400,
    AUTOAIM   = 0x0800,
    ATTACKDWN = 0x1000,
    USEDWN    = 0x2000,
    JMPDWN    = 0x4000,
    DIDSECRET = 0x8000,
} player_saveflags;

//
// P_ArchivePlayers
//
void P_ArchivePlayers (void)
{
    int         i;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
            continue;

//        PADSAVEP();
        WRITELONG(save_p, players[i].killcount);
        WRITELONG(save_p, players[i].itemcount);
        WRITELONG(save_p, players[i].secretcount);
        WRITELONG(save_p, players[i].bonuscount);
        WRITEBYTE(save_p, players[i].skincolor);
		WRITEBYTE(save_p, players[i].skin); // Tails 03-25-2001
		// Tails
		WRITELONG(save_p, players[i].score);
		WRITEBYTE(save_p, players[i].charspeed);
		WRITEBYTE(save_p, players[i].charability);
		WRITEBYTE(save_p, players[i].lives);
		WRITEBYTE(save_p, players[i].continues);
		WRITEBYTE(save_p, players[i].emerald1);
		WRITEBYTE(save_p, players[i].emerald2);
		WRITEBYTE(save_p, players[i].emerald3);
		WRITEBYTE(save_p, players[i].emerald4);
		WRITEBYTE(save_p, players[i].emerald5);
		WRITEBYTE(save_p, players[i].emerald6);
		WRITEBYTE(save_p, players[i].emerald7);
		WRITEBYTE(save_p, players[i].superready);
		WRITEBYTE(save_p, players[i].snowbuster);
		WRITEBYTE(save_p, players[i].xtralife);
		WRITEBYTE(save_p, players[i].xtralife2);
		// Tails
    }
}



//
// P_UnArchivePlayers
//
void P_UnArchivePlayers (void)
{
    int     i;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
//        memset (&players[i],0 , sizeof(player_t));
        if (!playeringame[i])
            continue;

//        PADSAVEP();
        players[i].killcount = READLONG(save_p);
        players[i].itemcount = READLONG(save_p);
        players[i].secretcount = READLONG(save_p);
        players[i].bonuscount = READLONG(save_p);
        players[i].skincolor = READBYTE(save_p);
        players[i].skin = READBYTE(save_p); // Tails 03-25-2001
		// Tails
		players[i].score = READLONG(save_p);
		players[i].charspeed = READBYTE(save_p);
		players[i].charability = READBYTE(save_p);
		players[i].lives = READBYTE(save_p);
		players[i].continues = READBYTE(save_p);
		players[i].emerald1 = READBYTE(save_p);
		players[i].emerald2 = READBYTE(save_p);
		players[i].emerald3 = READBYTE(save_p);
		players[i].emerald4 = READBYTE(save_p);
		players[i].emerald5 = READBYTE(save_p);
		players[i].emerald6 = READBYTE(save_p);
		players[i].emerald7 = READBYTE(save_p);
		players[i].superready = READBYTE(save_p);
		players[i].snowbuster = READBYTE(save_p);
		players[i].xtralife = READBYTE(save_p);
		players[i].xtralife2 = READBYTE(save_p);
		// Tails
    }
}

#define SD_FLOORHT     0x01
#define SD_CEILHT      0x02
#define SD_FLOORPIC    0x04
#define SD_CEILPIC     0x08
#define SD_LIGHT       0x10
#define SD_SPECIAL     0x20
//SoM: 4/10/2000: Fix sector related savegame bugs
#define SD_FXOFFS     0x40
#define SD_FYOFFS     0x80
#define SD_CXOFFS     0x100
#define SD_CYOFFS     0x200
#define SD_STAIRLOCK  0x400
#define SD_PREVSEC    0x800
#define SD_NEXTSEC    0x1000

#define LD_FLAG     0x01
#define LD_SPECIAL  0x02
//#define LD_TAG      0x04
#define LD_S1TEXOFF 0x08
#define LD_S1TOPTEX 0x10
#define LD_S1BOTTEX 0x20
#define LD_S1MIDTEX 0x40
#define LD_DIFF2    0x08

#define LD_S2TEXOFF 0x01
#define LD_S2TOPTEX 0x02
#define LD_S2BOTTEX 0x04
#define LD_S2MIDTEX 0x08


//
// P_ArchiveWorld
//
void P_ArchiveWorld (void)
{
}



//
// P_UnArchiveWorld
//
void P_UnArchiveWorld (void)
{
}


//
// Thinkers
//

typedef enum {
    MD_SPAWNPOINT = 0x000001,
    MD_POS        = 0x000002,
    MD_TYPE       = 0x000004,
    MD_Z          = 0x000008,
    MD_MOM        = 0x000010,
    MD_RADIUS     = 0x000020,
    MD_HEIGHT     = 0x000040,
    MD_FLAGS      = 0x000080,
    MD_HEALTH     = 0x000100,
    MD_RTIME      = 0x000200,
    MD_STATE      = 0x000400,
    MD_TICS       = 0x000800,
    MD_SPRITE     = 0x001000,
    MD_FRAME      = 0x002000,
    MD_EFLAGS     = 0x004000,
    MD_PLAYER     = 0x008000,
    MD_MOVEDIR    = 0x010000,
    MD_MOVECOUNT  = 0x020000,
    MD_THRESHOLD  = 0x040000,
    MD_LASTLOOK   = 0x080000,
    MD_TARGET     = 0x100000,
    MD_TRACER     = 0x200000,
    MD_FRICTION   = 0x400000,
    MD_MOVEFACTOR = 0x800000,
	MD_FUSE       = 0x1000000, // Tails
} mobj_diff_t;

enum
{
    tc_mobj,
    tc_ceiling,
    tc_door,
    tc_floor,
    tc_plat,
    tc_flash,
    tc_strobe,
    tc_glow,
    tc_fireflicker,
    tc_elevator, //SoM: 3/15/2000: Add extra boom types.
    tc_scroll,
    tc_friction,
    tc_pusher,
    tc_end

} specials_e;

//
// P_ArchiveThinkers
//
//
// Things to handle:
//
// P_MobjsThinker (all mobj)
// T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
// T_VerticalDoor, (vldoor_t: sector_t * swizzle),
// T_MoveFloor, (floormove_t: sector_t * swizzle),
// T_LightFlash, (lightflash_t: sector_t * swizzle),
// T_StrobeFlash, (strobe_t: sector_t *),
// T_Glow, (glow_t: sector_t *),
// T_PlatRaise, (plat_t: sector_t *), - active list
// BP: added missing : T_FireFlicker
//
void P_ArchiveThinkers (void)
{
}

// Now save the pointers, tracer and target, but at load time we must
// relink to this, the savegame contain the old position in the pointer
// field copyed in the info field temporarely, but finaly we just search
// for to old postion and relink to
/*
static mobj_t *FindNewPosition(void *oldposition)
{
}
*/
//
// P_UnArchiveThinkers
//
void P_UnArchiveThinkers (void)
{
}


//
// P_ArchiveSpecials
//


// BP: added : itemrespawnqueue
//
void P_ArchiveSpecials (void)
{
}


//
// P_UnArchiveSpecials
//
void P_UnArchiveSpecials (void)
{
}
