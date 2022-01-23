// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_saveg.c,v 1.20 2001/06/16 08:07:55 bpereira Exp $
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
// Revision 1.20  2001/06/16 08:07:55  bpereira
// no message
//
// Revision 1.19  2001/06/10 21:16:01  bpereira
// no message
//
// Revision 1.18  2001/03/03 06:17:33  bpereira
// no message
//
// Revision 1.17  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.16  2001/02/10 12:27:14  bpereira
// no message
//
// Revision 1.15  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.14  2000/11/11 13:59:45  bpereira
// no message
//
// Revision 1.13  2000/11/04 16:23:43  bpereira
// no message
//
// Revision 1.12  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.11  2000/09/28 20:57:16  bpereira
// no message
//
// Revision 1.10  2000/08/31 14:30:56  bpereira
// no message
//
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

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_setup.h"
#include "byteptr.h"
#include "t_vari.h"
#include "t_script.h"
#include "t_func.h"
#include "m_random.h"

byte*           save_p;

void SetPlayerSkinByNum(int playernum, int skinnum); // Tails 03-16-2002
void P_FindEmerald(player_t* player); // Tails
void SetSavedSkin(int playernum, int skinnum, int skincolor);

// Pads save_p to a 4-byte boundary
//  so that the load/save works on SGI&Gecko.
#ifdef SGI
// BP: this stuff isn't be removed but i think it will no more work
//     anyway what processor can't read/write unaligned data ?
#define PADSAVEP()      save_p += (4 - ((int) save_p & 3)) & 3
#else
#define PADSAVEP()
#endif

// New "functions" Tails
#define WRITEDOUBLE(p,b)      *((double   *)p)++ = b
#define READDOUBLE(p)         *((double   *)p)++

typedef enum {
    ATTACKDWN = 0x0001,
    USEDWN    = 0x0002,
    JMPDWN    = 0x0004,
	AUTOAIM   = 0x0008,
	LASTAXIS  = 0x0010,
	RFLAGPOINT= 0x0020,
	BFLAGPOINT= 0x0040,
	CAPSULE   = 0x0080,
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
		WRITEBYTE(save_p, players[i].charability);
		WRITEBYTE(save_p, players[i].charspin);
		WRITELONG(save_p, players[i].lives);
		WRITELONG(save_p, players[i].continues);
		WRITEBYTE(save_p, players[i].superready);
		WRITEBYTE(save_p, players[i].snowbuster);
		WRITELONG(save_p, players[i].xtralife);
		WRITELONG(save_p, players[i].xtralife2);
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
		players[i].charability = READBYTE(save_p);
		players[i].charspin = READBYTE(save_p);
		players[i].lives = READLONG(save_p);
		players[i].continues = READLONG(save_p);
		players[i].superready = READBYTE(save_p);
		players[i].snowbuster = READBYTE(save_p);
		players[i].xtralife = READLONG(save_p);
		players[i].xtralife2 = READLONG(save_p);
    }
}

//
// P_NetArchivePlayers
//
void P_NetArchivePlayers (void)
{
    int         i,j,q;
    int         flags;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
            continue;

        PADSAVEP();

        flags = 0;

        WRITEANGLE(save_p, players[i].aiming);
		WRITEANGLE(save_p, players[i].slope);
        WRITELONG(save_p, players[i].health);

        for(j=0;j<NUMPOWERS;j++)
                WRITELONG(save_p, players[i].powers[j]);

        WRITEBYTE(save_p, players[i].playerstate);

        if(players[i].attackdown)            flags |= ATTACKDWN;
        if(players[i].usedown)               flags |= USEDWN;
        if(players[i].jumpdown)              flags |= JMPDWN;
		if(players[i].autoaim_toggle)        flags |= AUTOAIM;

		WRITELONG(save_p, players[i].killcount);
		WRITELONG(save_p, players[i].itemcount);
		WRITELONG(save_p, players[i].secretcount);
		WRITELONG(save_p, players[i].bonuscount);
		WRITELONG(save_p, players[i].specialsector);
		WRITELONG(save_p, players[i].skincolor); 
		WRITELONG(save_p, players[i].skin);
		WRITELONG(save_p, players[i].score); 
		WRITELONG(save_p, players[i].dashspeed); 
		WRITELONG(save_p, players[i].lives);
		WRITELONG(save_p, players[i].continues); 
		WRITELONG(save_p, players[i].timebonus); 
		WRITELONG(save_p, players[i].ringbonus); 
		WRITELONG(save_p, players[i].fscore); 
		WRITELONG(save_p, players[i].seconds); 
		WRITELONG(save_p, players[i].minutes); 
		WRITELONG(save_p, players[i].superready); 
		WRITELONG(save_p, players[i].xtralife); 
		WRITELONG(save_p, players[i].xtralife2); 
		WRITELONG(save_p, players[i].walking); 
		WRITELONG(save_p, players[i].running); 
		WRITELONG(save_p, players[i].spinning); 
		WRITELONG(save_p, players[i].speed); 
		WRITELONG(save_p, players[i].jumping); 
		WRITELONG(save_p, players[i].mfjumped); 
		WRITELONG(save_p, players[i].mfspinning); 
		WRITELONG(save_p, players[i].mfstartdash); 
		WRITELONG(save_p, players[i].fly1); 
		WRITELONG(save_p, players[i].scoreadd); 
		WRITELONG(save_p, players[i].gliding); 
		WRITELONG(save_p, players[i].glidetime); 
		WRITELONG(save_p, players[i].climbing); 
		WRITELONG(save_p, players[i].deadtimer); 
		WRITELONG(save_p, players[i].splish); 
		WRITELONG(save_p, players[i].exiting); 
		WRITELONG(save_p, players[i].blackow); 
		WRITEBYTE(save_p, players[i].homing);

		////////////////////////////
		// Conveyor Belt Movement //
		////////////////////////////
		WRITEFIXED(save_p, players[i].cmomx); // Conveyor momx Tails 04-13-2001
		WRITEFIXED(save_p, players[i].cmomy); // Conveyor momy Tails 04-13-2001
		WRITEFIXED(save_p, players[i].rmomx); // "Real" momx (momx - cmomx) Tails 04-13-2001
		WRITEFIXED(save_p, players[i].rmomy); // "Real" momy (momy - cmomy)Tails 04-13-2001

		/////////////////////
		// Race Mode Stuff //
		/////////////////////
		WRITELONG(save_p, players[i].numboxes); 
		WRITELONG(save_p, players[i].totalring); 
		WRITELONG(save_p, players[i].realtime);
		WRITELONG(save_p, players[i].racescore); 
		WRITELONG(save_p, players[i].laps); // Graue 11-27-2003

		////////////////////
		// Tag Mode Stuff //
		////////////////////
		WRITELONG(save_p, players[i].tagit); 
		WRITELONG(save_p, players[i].tagcount); 
		WRITELONG(save_p, players[i].tagzone); 
		WRITELONG(save_p, players[i].taglag); 

		////////////////////
		// CTF Mode Stuff //
		////////////////////
		WRITELONG(save_p, players[i].ctfteam); 
		WRITEUSHORT(save_p, players[i].gotflag); 

		WRITELONG(save_p, players[i].redxvi); 
		WRITELONG(save_p, players[i].emeraldhunt); 
		WRITELONG(save_p, players[i].snowbuster); 
		WRITELONG(save_p, players[i].bustercount);

		WRITELONG(save_p, players[i].weapondelay);
		WRITELONG(save_p, players[i].taunttimer);

		WRITELONG(save_p, players[i].starposttime);
		WRITELONG(save_p, players[i].starpostx);
		WRITELONG(save_p, players[i].starposty);
		WRITELONG(save_p, players[i].starpostz);
		WRITELONG(save_p, players[i].starpostnum);
		WRITEANGLE(save_p, players[i].starpostangle);
		WRITEUSHORT(save_p,(short)players[i].starpostbit);

		WRITEDOUBLE(save_p, players[i].angle_speed);
		WRITEDOUBLE(save_p, players[i].angle_pos);
		WRITEDOUBLE(save_p, players[i].old_angle_pos);
		WRITEBYTE(save_p, players[i].nightsmode);

		WRITEBYTE(save_p, players[i].axishit);
		WRITEBYTE(save_p, players[i].axistransferred);
		WRITEBYTE(save_p, players[i].transfertoclosest);
		WRITELONG(save_p, players[i].flyangle);
		WRITELONG(save_p, players[i].drilltimer);
		WRITELONG(save_p, players[i].linkcount);
		WRITELONG(save_p, players[i].linktimer);
		WRITELONG(save_p, players[i].anotherflyangle);
		WRITELONG(save_p, players[i].transferangle);
		WRITELONG(save_p, players[i].nightstime);
		WRITEBYTE(save_p, players[i].nightsfall);
		WRITELONG(save_p, players[i].drillmeter);
		WRITEBYTE(save_p, players[i].drilldelay);
		WRITEBYTE(save_p, players[i].drilling);
		WRITEBYTE(save_p, players[i].skiddown);
		WRITEBYTE(save_p, players[i].bonustime);
		WRITEBYTE(save_p, players[i].mare);

		if(players[i].lastaxis)
			flags |= LASTAXIS;

		if(players[i].capsule)
			flags |= CAPSULE;

		WRITELONG(save_p, players[i].lastsidehit);
		WRITELONG(save_p, players[i].lastlinehit);

		WRITEBYTE(save_p, players[i].carried);

		WRITEBYTE(save_p, players[i].lightdash);
		WRITEBYTE(save_p, players[i].lightdashallowed);

		WRITEBYTE(save_p, players[i].thokked);

		WRITELONG(save_p, players[i].onconveyor);

		if(i == 0)
		{
			for(q = 0; q < nummapthings; q++)
			{
				if(&mapthings[q] == rflagpoint)
				{
					flags |= RFLAGPOINT;
					break;
				}
			}

			for(q = 0; q < nummapthings; q++)
			{
				if(&mapthings[q] == bflagpoint)
				{
					flags |= BFLAGPOINT;
					break;
				}
			}
		}

        WRITEUSHORT(save_p, flags);

		if(flags & LASTAXIS)
			WRITEULONG(save_p, players[i].lastaxis->mobjnum);

		if(flags & CAPSULE)
			WRITEULONG(save_p, players[i].capsule->mobjnum);

		if(flags & RFLAGPOINT)
			for(q = 0; q < nummapthings; q++)
			{
				if(&mapthings[q] == rflagpoint)
				{
					WRITEULONG(save_p, q);
					break;
				}
			}

		if(flags & BFLAGPOINT)
			for(q = 0; q < nummapthings; q++)
			{
				if(&mapthings[q] == bflagpoint)
				{
					WRITEULONG(save_p, q);
					break;
				}
			}
    }
}



//
// P_NetUnArchivePlayers
//
void P_NetUnArchivePlayers (void)
{
    int     i,j;
    int     flags;

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        memset (&players[i],0 , sizeof(player_t));
        if (!playeringame[i])
            continue;

        PADSAVEP();

        players[i].aiming = READANGLE(save_p);
		players[i].slope = READANGLE(save_p);
        players[i].health = READLONG(save_p);

        for(j=0;j<NUMPOWERS;j++)
            players[i].powers[j] = READLONG(save_p);

        players[i].playerstate = READBYTE(save_p);

		players[i].killcount   = READLONG(save_p);
		players[i].itemcount   = READLONG(save_p);
		players[i].secretcount = READLONG(save_p);
		players[i].bonuscount  = READLONG(save_p);

		players[i].specialsector = READLONG(save_p);

        players[i].skincolor = READLONG(save_p);
        players[i].skin = READLONG(save_p);

		players[i].score = READLONG(save_p);

		players[i].dashspeed = READLONG(save_p); // dashing speed Tails 03-01-2000

		players[i].lives = READLONG(save_p); // do lives now, worry about continues later Tails 03-09-2000
		players[i].continues = READLONG(save_p); // continues that player has acquired Tails 03-11-2000

		players[i].timebonus = READLONG(save_p); // Time Bonus Tails 03-10-2000
		players[i].ringbonus = READLONG(save_p); // Ring Bonus Tails 03-10-2000
		players[i].fscore = READLONG(save_p); // Fake score for intermissions Tails 03-12-2000
		players[i].seconds = READLONG(save_p); // Tails 06-13-2000
		players[i].minutes = READLONG(save_p); // Tails 06-13-2000

		players[i].superready = READLONG(save_p); // Ready for Super? Tails 04-08-2000

		players[i].xtralife = READLONG(save_p); // Ring Extra Life counter

		players[i].xtralife2 = READLONG(save_p); // Score xtra life counter

		players[i].walking = READLONG(save_p); // Are the walking frames playing? Tails 08-18-2000
		players[i].running = READLONG(save_p); // Are the running frames playing? Tails 08-18-2000
		players[i].spinning = READLONG(save_p); // Are the spinning frames playing? Tails 08-18-2000
		players[i].speed = READLONG(save_p); // Player's speed (distance formula of MOMX and MOMY values) Tails 08-21-2000
		players[i].jumping = READLONG(save_p); // Jump counter Tails 10-14-2000

		// Moved eflags to player ints Tails 10-30-2000
		players[i].mfjumped = READLONG(save_p);
		players[i].mfspinning = READLONG(save_p);
		players[i].mfstartdash = READLONG(save_p);

		players[i].fly1 = READLONG(save_p); // Tails flying Tails 11-01-2000
		players[i].scoreadd = READLONG(save_p); // Used for multiple enemy attack bonus Tails 11-03-2000
		players[i].gliding = READLONG(save_p); // Are you gliding? Tails 11-15-2000
		players[i].glidetime = READLONG(save_p); // Glide counter for thrust Tails 11-17-2000
		players[i].climbing = READLONG(save_p); // Climbing on the wall Tails 11-18-2000
		players[i].deadtimer = READLONG(save_p); // End game if game over lasts too long Tails 11-21-2000
		players[i].splish = READLONG(save_p); // Don't make splish repeat tons Tails 12-08-2000
		players[i].exiting = READLONG(save_p); // Exitlevel timer Tails 12-15-2000
		players[i].blackow = READLONG(save_p); // Tails 01-11-2001

		players[i].homing = READBYTE(save_p); // Are you homing? Tails 06-20-2001

		////////////////////////////
		// Conveyor Belt Movement //
		////////////////////////////
		players[i].cmomx = READFIXED(save_p); // Conveyor momx Tails 04-13-2001
		players[i].cmomy = READFIXED(save_p); // Conveyor momy Tails 04-13-2001
		players[i].rmomx = READFIXED(save_p); // "Real" momx (momx - cmomx) Tails 04-13-2001
		players[i].rmomy = READFIXED(save_p); // "Real" momy (momy - cmomy)Tails 04-13-2001

		/////////////////////
		// Race Mode Stuff //
		/////////////////////
		players[i].numboxes = READLONG(save_p); // Number of item boxes obtained for Race Mode Tails 04-25-2001
		players[i].totalring = READLONG(save_p); // Total number of rings obtained for Race Mode Tails 04-25-2001
		players[i].realtime = READLONG(save_p); // integer replacement for leveltime Tails 04-25-2001
		players[i].racescore = READLONG(save_p); // Total of won categories Tails 05-01-2001
		players[i].laps = READLONG(save_p); // How many laps have you done? Graue 11-27-2003

		////////////////////
		// Tag Mode Stuff //
		////////////////////
		players[i].tagit = READLONG(save_p); // The player is it! For Tag Mode Tails 05-08-2001
		players[i].tagcount = READLONG(save_p); // Number of tags player has avoided Tails 05-09-2001
		players[i].tagzone = READLONG(save_p); // Tag Zone timer Tails 05-11-2001
		players[i].taglag = READLONG(save_p); // Don't go back in the tag zone too early Tails 05-11-2001

		////////////////////
		// CTF Mode Stuff //
		////////////////////
		players[i].ctfteam = READLONG(save_p); // 1 == Red, 2 == Blue Tails 07-22-2001
		players[i].gotflag = READUSHORT(save_p); // 1 == Red  2 == Blue Do you have the flag? Tails 07-22-2001

		players[i].redxvi = READLONG(save_p); // RedXVI
		players[i].emeraldhunt = READLONG(save_p); // # of emeralds found Tails 12-12-2001
		players[i].snowbuster = READLONG(save_p); // Snow Buster upgrade! Tails 12-12-2001
		players[i].bustercount = READLONG(save_p); // Charge for Snow Buster Tails 12-12-2001

		players[i].weapondelay = READLONG(save_p); // Tails 07-12-2002
		players[i].taunttimer = READLONG(save_p); // Tails 09-06-2002

		players[i].starposttime = READLONG(save_p);
		players[i].starpostx = READLONG(save_p);
		players[i].starposty = READLONG(save_p);
		players[i].starpostz = READLONG(save_p);
		players[i].starpostnum = READLONG(save_p);
		players[i].starpostangle = READANGLE(save_p);
		players[i].starpostbit = READUSHORT(save_p);

		players[i].angle_speed = READDOUBLE(save_p);
		players[i].angle_pos = READDOUBLE(save_p);
		players[i].old_angle_pos = READDOUBLE(save_p);
		players[i].nightsmode = READBYTE(save_p);

		players[i].axishit = READBYTE(save_p);
		players[i].axistransferred = READBYTE(save_p);
		players[i].transfertoclosest = READBYTE(save_p);
		players[i].flyangle = READLONG(save_p);
		players[i].drilltimer = READLONG(save_p);
		players[i].linkcount = READLONG(save_p);
		players[i].linktimer = READLONG(save_p);
		players[i].anotherflyangle = READLONG(save_p);
		players[i].transferangle = READLONG(save_p);
		players[i].nightstime = READLONG(save_p);
		players[i].nightsfall = READBYTE(save_p);
		players[i].drillmeter = READLONG(save_p);
		players[i].drilldelay = READBYTE(save_p);
		players[i].drilling = READBYTE(save_p);
		players[i].skiddown = READBYTE(save_p);
		players[i].bonustime = READBYTE(save_p);
		players[i].mare = READBYTE(save_p);

		players[i].lastsidehit = READLONG(save_p);
		players[i].lastlinehit = READLONG(save_p);

		players[i].carried = READBYTE(save_p);

		players[i].lightdash = READBYTE(save_p);
		players[i].lightdashallowed = READBYTE(save_p);

		players[i].thokked = READBYTE(save_p);

		players[i].onconveyor = READLONG(save_p);

        flags = READUSHORT(save_p);

        players[i].attackdown           = (flags & ATTACKDWN) !=0;
        players[i].usedown              = (flags & USEDWN)    !=0;
        players[i].jumpdown             = (flags & JMPDWN)    !=0;
		players[i].autoaim_toggle       = (flags & AUTOAIM)   !=0;

		if(flags & LASTAXIS)
			players[i].lastaxis = (mobj_t*) READULONG(save_p);

		if(flags & CAPSULE)
			players[i].capsule = (mobj_t*) READULONG(save_p);

		if(i == 0)
		{
			if(flags & RFLAGPOINT)
				rflagpoint = &mapthings[READULONG(save_p)];

			if(flags & BFLAGPOINT)
				bflagpoint = &mapthings[READULONG(save_p)];	
		}

        players[i].viewheight = cv_viewheight.value<<FRACBITS;

		SetPlayerSkinByNum(i, players[i].skin);
    }
}

#define SD_FLOORHT     0x01
#define SD_CEILHT      0x02
#define SD_FLOORPIC    0x04
#define SD_CEILPIC     0x08
#define SD_LIGHT       0x10
#define SD_SPECIAL     0x20
#define SD_DIFF2       0x40

//SoM: 4/10/2000: Fix sector related savegame bugs
// diff2 flags
#define SD_FXOFFS     0x01
#define SD_FYOFFS     0x02
#define SD_CXOFFS     0x04
#define SD_CYOFFS     0x08
#define SD_STAIRLOCK  0x10
#define SD_PREVSEC    0x20
#define SD_NEXTSEC    0x40

#define LD_FLAG     0x01
#define LD_SPECIAL  0x02
//#define LD_TAG      0x04
#define LD_S1TEXOFF 0x08
#define LD_S1TOPTEX 0x10
#define LD_S1BOTTEX 0x20
#define LD_S1MIDTEX 0x40
#define LD_DIFF2    0x80

// diff2 flags
#define LD_S2TEXOFF 0x01
#define LD_S2TOPTEX 0x02
#define LD_S2BOTTEX 0x04
#define LD_S2MIDTEX 0x08


//
// P_NetArchiveWorld
//
void P_NetArchiveWorld (void)
{
    int                 i;
    int           statsec=0,statline=0;
    line_t*       li;
    side_t*       si;
    byte*         put;

    // reload the map just to see difference
    mapsector_t   *ms;
    mapsidedef_t  *msd;
    maplinedef_t  *mld;
    sector_t      *ss;
    byte           diff;
    byte           diff2;

    ms = W_CacheLumpNum (lastloadedmaplumpnum+ML_SECTORS,PU_CACHE);
    ss = sectors;
    put = save_p;

    for (i=0 ; i<numsectors ; i++, ss++, ms++)
    {
        diff=0;diff2=0;
        if (ss->floorheight != SHORT(ms->floorheight)<<FRACBITS)
            diff |= SD_FLOORHT;
        if (ss->ceilingheight != SHORT(ms->ceilingheight)<<FRACBITS)
            diff |= SD_CEILHT;
        //
        //  flats
        //
        // P_AddLevelFlat should not add but just return the number
        if (ss->floorpic != P_AddLevelFlat (ms->floorpic,levelflats))
            diff |= SD_FLOORPIC;
        if (ss->ceilingpic != P_AddLevelFlat (ms->ceilingpic,levelflats))
            diff |= SD_CEILPIC;

        if (ss->lightlevel != SHORT(ms->lightlevel))     diff |= SD_LIGHT;
        if (ss->special != SHORT(ms->special))           diff |= SD_SPECIAL;

        if (ss->floor_xoffs != 0)                        diff2 |= SD_FXOFFS;
        if (ss->floor_yoffs != 0)                        diff2 |= SD_FYOFFS;
        if (ss->ceiling_xoffs != 0)                      diff2 |= SD_CXOFFS;
        if (ss->ceiling_yoffs != 0)                      diff2 |= SD_CYOFFS;
        if (ss->stairlock < 0)                           diff2 |= SD_STAIRLOCK;
        if (ss->nextsec != -1)                           diff2 |= SD_NEXTSEC;
        if (ss->prevsec != -1)                           diff2 |= SD_PREVSEC;
        if (diff2)                                       diff |= SD_DIFF2;

        if(diff)
        {
            statsec++;

            WRITESHORT(put,i);
            WRITEBYTE(put,diff);
            if( diff & SD_DIFF2   )     WRITEBYTE(put,diff2);
            if( diff & SD_FLOORHT )     WRITEFIXED(put,ss->floorheight);
            if( diff & SD_CEILHT  )     WRITEFIXED(put,ss->ceilingheight);
            if( diff & SD_FLOORPIC)
            {
                memcpy(put,levelflats[ss->floorpic].name,8);
                put+=8;
            }
            if( diff & SD_CEILPIC )
            {
                memcpy(put,levelflats[ss->ceilingpic].name,8);
                put+=8;
            }
            if( diff & SD_LIGHT   )     WRITESHORT(put,(short)ss->lightlevel);
            if( diff & SD_SPECIAL )     WRITESHORT(put,(short)ss->special);

            if( diff2 & SD_FXOFFS  )    WRITEFIXED(put,ss->floor_xoffs);
            if( diff2 & SD_FYOFFS  )    WRITEFIXED(put,ss->floor_yoffs);
            if( diff2 & SD_CXOFFS  )    WRITEFIXED(put,ss->ceiling_xoffs);
            if( diff2 & SD_CYOFFS  )    WRITEFIXED(put,ss->ceiling_yoffs);
            if( diff2 & SD_STAIRLOCK)   WRITELONG (put,ss->stairlock);
            if( diff2 & SD_NEXTSEC )    WRITELONG (put,ss->nextsec);
            if( diff2 & SD_PREVSEC )    WRITELONG (put,ss->prevsec);
        }
    }
    *((unsigned short *)put)++=0xffff;

    mld = W_CacheLumpNum (lastloadedmaplumpnum+ML_LINEDEFS,PU_CACHE);
    msd = W_CacheLumpNum (lastloadedmaplumpnum+ML_SIDEDEFS,PU_CACHE);
    li = lines;
    // do lines
    for (i=0 ; i<numlines ; i++,mld++,li++)
    {
        diff=0;diff2=0;
/*
        // we don't care of map in deathmatch !
        if(((cv_deathmatch.value==0) && (li->flags != SHORT(mld->flags))) ||
           ((cv_deathmatch.value!=0) && ((li->flags & ~ML_MAPPED) != SHORT(mld->flags))))
            diff |= LD_FLAG;*/
        if(li->special != SHORT(mld->special))
            diff |= LD_SPECIAL;

        if (li->sidenum[0] != -1)
        {
            si = &sides[li->sidenum[0]];
            if (si->textureoffset != SHORT(msd[li->sidenum[0]].textureoffset)<<FRACBITS)
                diff |= LD_S1TEXOFF;
            //SoM: 4/1/2000: Some textures are colormaps. Don't worry about invalid textures.
            if(R_CheckTextureNumForName(msd[li->sidenum[0]].toptexture) != -1)
              if (si->toptexture != R_TextureNumForName(msd[li->sidenum[0]].toptexture) )
                diff |= LD_S1TOPTEX;
            if(R_CheckTextureNumForName(msd[li->sidenum[0]].bottomtexture) != -1)
              if (si->bottomtexture != R_TextureNumForName(msd[li->sidenum[0]].bottomtexture) )
                diff |= LD_S1BOTTEX;
            if(R_CheckTextureNumForName(msd[li->sidenum[0]].midtexture) != -1)
              if (si->midtexture != R_TextureNumForName(msd[li->sidenum[0]].midtexture) )
                diff |= LD_S1MIDTEX;
        }
        if (li->sidenum[1] != -1)
        {
            si = &sides[li->sidenum[1]];
            if (si->textureoffset != SHORT(msd[li->sidenum[1]].textureoffset)<<FRACBITS)
                diff2 |= LD_S2TEXOFF;
            if(R_CheckTextureNumForName(msd[li->sidenum[1]].toptexture) != -1)
              if (si->toptexture != R_TextureNumForName(msd[li->sidenum[1]].toptexture) )
                diff2 |= LD_S2TOPTEX;
            if(R_CheckTextureNumForName(msd[li->sidenum[1]].bottomtexture) != -1)
              if (si->bottomtexture != R_TextureNumForName(msd[li->sidenum[1]].bottomtexture) )
                diff2 |= LD_S2BOTTEX;
            if(R_CheckTextureNumForName(msd[li->sidenum[1]].midtexture) != -1)
              if (si->midtexture != R_TextureNumForName(msd[li->sidenum[1]].midtexture) )
                diff2 |= LD_S2MIDTEX;
            if(diff2)
                diff |= LD_DIFF2;

        }

        if(diff)
        {
            statline++;
            WRITESHORT(put,(short)i);
            WRITEBYTE(put,diff);
            if( diff & LD_DIFF2    )     WRITEBYTE(put,diff2);
            if( diff & LD_FLAG     )     WRITESHORT(put,li->flags);
            if( diff & LD_SPECIAL  )     WRITESHORT(put,li->special);

            si = &sides[li->sidenum[0]];
            if( diff & LD_S1TEXOFF )     WRITEFIXED(put,si->textureoffset);
            if( diff & LD_S1TOPTEX )     WRITESHORT(put,si->toptexture);
            if( diff & LD_S1BOTTEX )     WRITESHORT(put,si->bottomtexture);
            if( diff & LD_S1MIDTEX )     WRITESHORT(put,si->midtexture);

            si = &sides[li->sidenum[1]];
            if( diff2 & LD_S2TEXOFF )    WRITEFIXED(put,si->textureoffset);
            if( diff2 & LD_S2TOPTEX )    WRITESHORT(put,si->toptexture);
            if( diff2 & LD_S2BOTTEX )    WRITESHORT(put,si->bottomtexture);
            if( diff2 & LD_S2MIDTEX )    WRITESHORT(put,si->midtexture);
        }
    }
    WRITEUSHORT(put,0xffff);

    //CONS_Printf("sector saved %d/%d, line saved %d/%d\n",statsec,numsectors,statline,numlines);
    save_p = put;
}



//
// P_NetUnArchiveWorld
//
void P_NetUnArchiveWorld (void)
{
    int                 i;
    line_t*     li;
    side_t*     si;
    byte*       get;
    byte        diff,diff2;

    get = save_p;

    while (1)
    {
        i=*((unsigned short *)get)++;

        if (i==0xffff)
            break;

        diff=READBYTE(get);
        if( diff & SD_DIFF2    )   diff2 = READBYTE(get);
                              else diff2 = 0;
        if( diff & SD_FLOORHT  )   sectors[i].floorheight   = READFIXED(get);
        if( diff & SD_CEILHT   )   sectors[i].ceilingheight = READFIXED(get);
        if( diff & SD_FLOORPIC )
        {
            sectors[i].floorpic = P_AddLevelFlat (get,levelflats);
            get+=8;
        }
        if( diff & SD_CEILPIC )
        {
            sectors[i].ceilingpic = P_AddLevelFlat (get,levelflats);
            get+=8;
        }
        if( diff & SD_LIGHT )    sectors[i].lightlevel = READSHORT(get);
        if( diff & SD_SPECIAL )  sectors[i].special    = READSHORT(get);

        if( diff2 & SD_FXOFFS )  sectors[i].floor_xoffs     = READFIXED(get);
        if( diff2 & SD_FYOFFS )  sectors[i].floor_yoffs     = READFIXED(get);
        if( diff2 & SD_CXOFFS )  sectors[i].ceiling_xoffs   = READFIXED(get);
        if( diff2 & SD_CYOFFS )  sectors[i].ceiling_yoffs   = READFIXED(get);
        if( diff2 & SD_STAIRLOCK)sectors[i].stairlock       = READLONG (get);
                            else sectors[i].stairlock       = 0;
        if( diff2 & SD_NEXTSEC)  sectors[i].nextsec         = READLONG (get);
                            else sectors[i].nextsec         = -1;
        if( diff2 & SD_PREVSEC)  sectors[i].prevsec         = READLONG (get);
                            else sectors[i].prevsec         = -1;
    }

    while(1)
    {
        i=READUSHORT(get);

        if (i==0xffff)
            break;
        diff = READBYTE(get);
        li = &lines[i];

        if( diff & LD_DIFF2    )    diff2 = READBYTE(get);
                               else diff2 = 0;
        if( diff & LD_FLAG     )    li->flags = READSHORT(get);
        if( diff & LD_SPECIAL  )    li->special = READSHORT(get);

        si = &sides[li->sidenum[0]];
        if( diff & LD_S1TEXOFF )    si->textureoffset = READFIXED(get);
        if( diff & LD_S1TOPTEX )    si->toptexture    = READSHORT(get);
        if( diff & LD_S1BOTTEX )    si->bottomtexture = READSHORT(get);
        if( diff & LD_S1MIDTEX )    si->midtexture    = READSHORT(get);

        si = &sides[li->sidenum[1]];
        if( diff2 & LD_S2TEXOFF )   si->textureoffset = READFIXED(get);
        if( diff2 & LD_S2TOPTEX )   si->toptexture    = READSHORT(get);
        if( diff2 & LD_S2BOTTEX )   si->bottomtexture = READSHORT(get);
        if( diff2 & LD_S2MIDTEX )   si->midtexture    = READSHORT(get);
    }

    save_p = get;
}


//
// Thinkers
//

typedef enum {
    MD_SPAWNPOINT = 0x000001,
    MD_POS        = 0x000002,
    MD_TYPE       = 0x000004,
	// Eliminated MD_Z to prevent 3dfloor hiccups Tails 03-17-2002
    MD_MOM        = 0x000008,
    MD_RADIUS     = 0x000010,
    MD_HEIGHT     = 0x000020,
    MD_FLAGS      = 0x000040,
    MD_HEALTH     = 0x000080,
    MD_RTIME      = 0x000100,
    MD_STATE      = 0x000200,
    MD_TICS       = 0x000400,
    MD_SPRITE     = 0x000800,
    MD_FRAME      = 0x001000,
    MD_EFLAGS     = 0x002000,
    MD_PLAYER     = 0x004000,
    MD_MOVEDIR    = 0x008000,
    MD_MOVECOUNT  = 0x010000,
    MD_THRESHOLD  = 0x020000,
    MD_LASTLOOK   = 0x040000,
    MD_TARGET     = 0x080000,
    MD_TRACER     = 0x100000,
    MD_FRICTION   = 0x200000,
    MD_MOVEFACTOR = 0x400000,
    MD_FLAGS2     = 0x800000,
	MD_FUSE       =0x1000000, // Tails
	MD_WATERTOP   =0x2000000, // Tails
	MD_WATERBOTTOM=0x4000000, // Tails
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
    tc_bouncecheese, // Tails 02-08-2002
	tc_startcrumble, // Tails 03-11-2002
	tc_airbob, // Tails 03-11-2002
	tc_marioblock, // Tails 03-11-2002
	tc_spikesector, // Tails 09-20-2002
	tc_floatsector, // Tails 09-20-2002
	tc_crushceiling,
    tc_scroll,
    tc_friction,
    tc_pusher,
	tc_laserflash, // Graue 12-30-2003
    tc_end

} specials_e;

//
// P_NetArchiveThinkers
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
void P_NetArchiveThinkers (void)
{
    thinker_t*          th;
    mobj_t*             mobj;
    ULONG               diff;
//    int                 i; //SoM: 3/16/2000: Removed. Not used any more.
	int i;
	i = 0;

	// Assign the mobjnumber for pointer tracking Tails 04-01-2003
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj = (mobj_t *)th;
			mobj->mobjnum = i++;
			CONS_Printf("mobjnum is %d\n", mobj->mobjnum);
		}
	}

    // save off the current thinkers
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
        {
            mobj = (mobj_t *)th;
/*
            // not a monster nor a picable item so don't save it
            if( (((mobj->flags & (MF_COUNTKILL | MF_PICKUP | MF_SHOOTABLE )) == 0)
                 && (mobj->flags & MF_MISSILE)
                 && (mobj->info->doomednum !=-1) )
                || (mobj->type == MT_BLOOD) )
                continue;
*/
            if(mobj->spawnpoint && (mobj->info->doomednum !=-1)) {
                // spawnpoint is not modified but we must save it since it is an identifier
                diff = MD_SPAWNPOINT;

                if((mobj->x != mobj->spawnpoint->x << FRACBITS) ||
                   (mobj->y != mobj->spawnpoint->y << FRACBITS) ||
                   (mobj->angle != (unsigned)(ANG45 * (mobj->spawnpoint->angle/45))) ) diff |= MD_POS;
                if(mobj->info->doomednum != mobj->spawnpoint->type)        diff |= MD_TYPE;
            }
            else
            {
                // not a map spawned thing so make it from scratch
                diff = MD_POS | MD_TYPE;
            }

            // not the default but the most probable
            if((mobj->momx != 0)||(mobj->momy != 0)||(mobj->momz != 0) )   diff |= MD_MOM;
            if( mobj->radius        != mobj->info->radius       )          diff |= MD_RADIUS;
            if( mobj->height        != mobj->info->height       )          diff |= MD_HEIGHT;
            if( mobj->flags         != mobj->info->flags        )          diff |= MD_FLAGS;
			diff |= MD_FLAGS2; // Force saving of flags2
            if( mobj->health        != mobj->info->spawnhealth  )          diff |= MD_HEALTH;
            if( mobj->reactiontime  != mobj->info->reactiontime )          diff |= MD_RTIME;
            if( mobj->state-states  != mobj->info->spawnstate   )          diff |= MD_STATE;
            if( mobj->tics          != mobj->state->tics        )          diff |= MD_TICS;
            if( mobj->sprite        != mobj->state->sprite      )          diff |= MD_SPRITE;
            if( mobj->frame         != mobj->state->frame       )          diff |= MD_FRAME;
            if( mobj->eflags        )                                      diff |= MD_EFLAGS;
            if( mobj->player        )                                      diff |= MD_PLAYER;

            if( mobj->movedir       )                                      diff |= MD_MOVEDIR;
            if( mobj->movecount     )                                      diff |= MD_MOVECOUNT;
            if( mobj->threshold     )                                      diff |= MD_THRESHOLD;
            if( mobj->lastlook      != -1 )                                diff |= MD_LASTLOOK;
            if( mobj->target        )                                      diff |= MD_TARGET;
            if( mobj->tracer        )                                      diff |= MD_TRACER;
            if( mobj->friction      !=ORIG_FRICTION             )          diff |= MD_FRICTION;
            if( mobj->movefactor    !=ORIG_FRICTION_FACTOR      )          diff |= MD_MOVEFACTOR;
            if( mobj->fuse          )                                      diff |= MD_FUSE; // Tails
            if( mobj->watertop      )                                      diff |= MD_WATERTOP; // Tails
            if( mobj->waterbottom   )                                      diff |= MD_WATERBOTTOM; // Tails

            PADSAVEP();
            WRITEBYTE(save_p, tc_mobj);
            WRITEULONG(save_p, diff);

            // save pointer, at load time we will search this pointer to reinitilize pointers
            WRITEULONG(save_p, (ULONG)mobj);

            WRITEFIXED(save_p, mobj->z); // Force this so 3dfloor problems don't arise. Tails 03-17-2002
			WRITEFIXED(save_p, mobj->floorz);

            if( diff & MD_SPAWNPOINT )
			{
				int z;

				for(z=0; z<nummapthings; z++)
				{
					if(&mapthings[z] == mobj->spawnpoint)
					{
						WRITESHORT(save_p, z);
					}
				}
			}

            if( diff & MD_TYPE       )   WRITEULONG(save_p, mobj->type);
            if( diff & MD_POS        ) { WRITEFIXED(save_p, mobj->x);
                                         WRITEFIXED(save_p, mobj->y);
                                         WRITEANGLE(save_p, mobj->angle);     }
            if( diff & MD_MOM        ) { WRITEFIXED(save_p, mobj->momx);
                                         WRITEFIXED(save_p, mobj->momy);
                                         WRITEFIXED(save_p, mobj->momz);      }
            if( diff & MD_RADIUS     )   WRITEFIXED(save_p, mobj->radius      );
            if( diff & MD_HEIGHT     )   WRITEFIXED(save_p, mobj->height      );
            if( diff & MD_FLAGS      )   WRITELONG (save_p, mobj->flags       );
            if( diff & MD_FLAGS2     )   WRITELONG (save_p, mobj->flags2      );
            if( diff & MD_HEALTH     )   WRITELONG (save_p, mobj->health      );
            if( diff & MD_RTIME      )   WRITELONG (save_p, mobj->reactiontime);
            if( diff & MD_STATE      )  WRITEUSHORT(save_p, mobj->state-states);
            if( diff & MD_TICS       )   WRITELONG (save_p, mobj->tics        );
            if( diff & MD_SPRITE     )  WRITEUSHORT(save_p, mobj->sprite      );
            if( diff & MD_FRAME      )   WRITEULONG(save_p, mobj->frame       );
            if( diff & MD_EFLAGS     )   WRITEULONG(save_p, mobj->eflags      );
            if( diff & MD_PLAYER     )   WRITEBYTE (save_p, mobj->player-players );
            if( diff & MD_MOVEDIR    )   WRITELONG (save_p, mobj->movedir     );
            if( diff & MD_MOVECOUNT  )   WRITELONG (save_p, mobj->movecount   );
            if( diff & MD_THRESHOLD  )   WRITELONG (save_p, mobj->threshold   );
            if( diff & MD_LASTLOOK   )   WRITELONG (save_p, mobj->lastlook    );
            if( diff & MD_TARGET     )   WRITEULONG(save_p, mobj->target->mobjnum      );
            if( diff & MD_TRACER     )   WRITEULONG(save_p, mobj->tracer->mobjnum      );
            if( diff & MD_FRICTION   )   WRITELONG (save_p, mobj->friction    );
            if( diff & MD_MOVEFACTOR )   WRITELONG (save_p, mobj->movefactor  );
			if( diff & MD_FUSE       )   WRITELONG (save_p, mobj->fuse        ); // Tails
			if( diff & MD_WATERTOP   )   WRITELONG (save_p, mobj->watertop    ); // Tails
			if( diff & MD_WATERBOTTOM)   WRITELONG (save_p, mobj->waterbottom ); // Tails

			// Special case for use of the block and sector links Tails 11-07-2002
			if(mobj->type == MT_HOOPCOLLIDE || mobj->type == MT_HOOP)
			{
				if(mobj->bnext)
					WRITEULONG(save_p, mobj->bnext->mobjnum);
				else
					WRITEULONG(save_p, 0xFFFFFFFF);

				if(mobj->bprev)
					WRITEULONG(save_p, mobj->bprev->mobjnum);
				else
					WRITEULONG(save_p, 0xFFFFFFFF);

				if(mobj->snext)
					WRITEULONG(save_p, mobj->snext->mobjnum);
				else
					WRITEULONG(save_p, 0xFFFFFFFF);

				if(mobj->sprev)
					WRITEULONG(save_p, mobj->sprev->mobjnum);
				else
					WRITEULONG(save_p, 0xFFFFFFFF);
			}

			WRITEULONG(save_p, mobj->mobjnum); // Tails 04-01-2003
        }
        else
        if (th->function.acv == (actionf_v)NULL)
        { 
            //SoM: 3/15/2000: Boom stuff...
            ceilinglist_t* cl;
            
            for (cl = activeceilings; cl; cl = cl->next)
                if (cl->ceiling == (ceiling_t *)th)
                {
                    ceiling_t*          ceiling;
                    WRITEBYTE(save_p, tc_ceiling);
                    PADSAVEP();
                    ceiling = (ceiling_t *)save_p;
                    memcpy (save_p, th, sizeof(*ceiling));
                    save_p += sizeof(*ceiling);
                    ceiling->sector = (sector_t *)(ceiling->sector - sectors);
                }
                
            continue;
        }
        else
        if (th->function.acp1 == (actionf_p1)T_MoveCeiling)
        {
            ceiling_t*          ceiling;
            WRITEBYTE(save_p, tc_ceiling);
            PADSAVEP();
            ceiling = (ceiling_t *)save_p;
            memcpy (ceiling, th, sizeof(*ceiling));
            save_p += sizeof(*ceiling);
            ceiling->sector = (sector_t *)(ceiling->sector - sectors);
            continue;
        }
		else
        if (th->function.acp1 == (actionf_p1)T_CrushCeiling)
        {
            ceiling_t*          ceiling;
            WRITEBYTE(save_p, tc_crushceiling);
            PADSAVEP();
            ceiling = (ceiling_t *)save_p;
            memcpy (ceiling, th, sizeof(*ceiling));
            save_p += sizeof(*ceiling);
            ceiling->sector = (sector_t *)(ceiling->sector - sectors);
            continue;
        }
        else
        if (th->function.acp1 == (actionf_p1)T_VerticalDoor)
        {
            vldoor_t*           door;
            WRITEBYTE(save_p, tc_door);
            PADSAVEP();
            door = (vldoor_t *)save_p;
            memcpy (door, th, sizeof(*door));
            save_p += sizeof(*door);
            door->sector = (sector_t *)(door->sector - sectors);
            door->line   = (line_t *)(door->line - lines);
            continue;
        }
        else
        if (th->function.acp1 == (actionf_p1)T_MoveFloor)
        {
            floormove_t*        floor;
            WRITEBYTE(save_p, tc_floor);
            PADSAVEP();
            floor = (floormove_t *)save_p;
            memcpy (floor, th, sizeof(*floor));
            save_p += sizeof(*floor);
            floor->sector = (sector_t *)(floor->sector - sectors);
            continue;
        }
        else
        if (th->function.acp1 == (actionf_p1)T_PlatRaise)
        {
            plat_t*             plat;
            WRITEBYTE(save_p, tc_plat);
            PADSAVEP();
            plat = (plat_t *)save_p;
            memcpy (plat, th, sizeof(*plat));
            save_p += sizeof(*plat);
            plat->sector = (sector_t *)(plat->sector - sectors);
            continue;
        }
        else
        if (th->function.acp1 == (actionf_p1)T_LightFlash)
        {
            lightflash_t*       flash;
            WRITEBYTE(save_p, tc_flash);
            PADSAVEP();
            flash = (lightflash_t *)save_p;
            memcpy (flash, th, sizeof(*flash));
            save_p += sizeof(*flash);
            flash->sector = (sector_t *)(flash->sector - sectors);
            continue;
        }
        else
        if (th->function.acp1 == (actionf_p1)T_StrobeFlash)
        {
            strobe_t*           strobe;
            WRITEBYTE(save_p, tc_strobe);
            PADSAVEP();
            strobe = (strobe_t *)save_p;
            memcpy (strobe, th, sizeof(*strobe));
            save_p += sizeof(*strobe);
            strobe->sector = (sector_t *)(strobe->sector - sectors);
            continue;
        }
        else
        if (th->function.acp1 == (actionf_p1)T_Glow)
        {
            glow_t*             glow;
            WRITEBYTE(save_p, tc_glow);
            PADSAVEP();
            glow = (glow_t *)save_p;
            memcpy (glow, th, sizeof(*glow));
            save_p += sizeof(*glow);
            glow->sector = (sector_t *)(glow->sector - sectors);
            continue;
        }
        else
        // BP added T_FireFlicker
        if (th->function.acp1 == (actionf_p1)T_FireFlicker)
        {
            fireflicker_t*      fireflicker;
            WRITEBYTE(save_p, tc_fireflicker);
            PADSAVEP();
            fireflicker = (fireflicker_t *)save_p;
            memcpy (fireflicker, th, sizeof(*fireflicker));
            save_p += sizeof(*fireflicker);
            fireflicker->sector = (sector_t *)(fireflicker->sector - sectors);
            continue;
        }
        else
        //SoM: 3/15/2000: Added extra Boom thinker types.
        if (th->function.acp1 == (actionf_p1) T_MoveElevator)
        {
          elevator_t  *elevator;
          WRITEBYTE(save_p, tc_elevator);
          PADSAVEP();
          elevator = (elevator_t *)save_p;
          memcpy (elevator, th, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = (sector_t *)(elevator->sector - sectors);
          continue;
        }
        else
        if (th->function.acp1 == (actionf_p1) T_Scroll)
        {
          WRITEBYTE(save_p, tc_scroll);
          memcpy (save_p, th, sizeof(scroll_t));
          save_p += sizeof(scroll_t);
          continue;
        }
        else
        if (th->function.acp1 == (actionf_p1) T_Friction)
        {
          WRITEBYTE(save_p, tc_friction);
          memcpy (save_p, th, sizeof(friction_t));
          save_p += sizeof(friction_t);
          continue;
        }
        else
        if (th->function.acp1 == (actionf_p1) T_Pusher)
        {
          WRITEBYTE(save_p, tc_pusher);
          memcpy (save_p, th, sizeof(pusher_t));
          save_p += sizeof(pusher_t);
          continue;
        }
		// Check special thinkers here. Tails 03-11-2002
		else
        if (th->function.acp1 == (actionf_p1) T_BounceCheese)
        {
          elevator_t  *elevator;
          WRITEBYTE(save_p, tc_bouncecheese);
          PADSAVEP();
          elevator = (elevator_t *)save_p;
          memcpy (elevator, th, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = (sector_t *)(elevator->sector - sectors);
		  elevator->actionsector = (sector_t *)(elevator->actionsector - sectors);
          continue;
        }
		else
        if (th->function.acp1 == (actionf_p1) T_StartCrumble)
        {
          elevator_t  *elevator;
          WRITEBYTE(save_p, tc_startcrumble);
          PADSAVEP();
          elevator = (elevator_t *)save_p;
          memcpy (elevator, th, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = (sector_t *)(elevator->sector - sectors);
		  elevator->actionsector = (sector_t *)(elevator->actionsector - sectors);
          continue;
        }
		else
        if (th->function.acp1 == (actionf_p1) T_AirBob)
        {
          elevator_t  *elevator;
          WRITEBYTE(save_p, tc_airbob);
          PADSAVEP();
          elevator = (elevator_t *)save_p;
          memcpy (elevator, th, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = (sector_t *)(elevator->sector - sectors);
		  elevator->player = (player_t *)(elevator->player - players);
          continue;
        }
		else
        if (th->function.acp1 == (actionf_p1) T_MarioBlock)
        {
          elevator_t  *elevator;
          WRITEBYTE(save_p, tc_marioblock);
          PADSAVEP();
          elevator = (elevator_t *)save_p;
          memcpy (elevator, th, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = (sector_t *)(elevator->sector - sectors);
          continue;
        }
		else
        if (th->function.acp1 == (actionf_p1) T_SpikeSector)
        {
          elevator_t  *elevator;
          WRITEBYTE(save_p, tc_spikesector);
          PADSAVEP();
          elevator = (elevator_t *)save_p;
          memcpy (elevator, th, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = (sector_t *)(elevator->sector - sectors);
          continue;
        }
		else
        if (th->function.acp1 == (actionf_p1) T_FloatSector)
        {
          elevator_t  *elevator;
          WRITEBYTE(save_p, tc_floatsector);
          PADSAVEP();
          elevator = (elevator_t *)save_p;
          memcpy (elevator, th, sizeof(*elevator));
          save_p += sizeof(*elevator);
          elevator->sector = (sector_t *)(elevator->sector - sectors);
		  elevator->actionsector = (sector_t *)(elevator->actionsector - sectors);
          continue;
        }
		// Graue 12-30-2003
		else
		if (th->function.acp1 == (actionf_p1) T_LaserFlash)
		{
          laserthink_t  *laser;
          WRITEBYTE(save_p, tc_laserflash);
          PADSAVEP();
          laser = (laserthink_t *)save_p;
          memcpy (laser, th, sizeof(*laser));
          save_p += sizeof(*laser);
		  laser->ffloor = (ffloor_t *)(laser->ffloor - laser->sector->ffloors);
          laser->sector = (sector_t *)(laser->sector - sectors);
          continue;
        }
#ifdef PARANOIA
        else if( (int)th->function.acp1 != -1 ) // wait garbage colection
		{
			I_Error("unknown thinker type 0x%X",th->function.acp1);
		}
#endif
    }

    WRITEBYTE(save_p, tc_end);
}

// Now save the pointers, tracer and target, but at load time we must
// relink to this, the savegame contain the old position in the pointer
// field copyed in the info field temporarely, but finaly we just search
// for to old postion and relink to
static mobj_t *FindNewPosition(int oldposition)
{
    thinker_t*          th;
    mobj_t*             mobj;

    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        mobj = (mobj_t *)th;
        if(mobj->mobjnum == oldposition)
            return mobj;
    }
    if(devparm) CONS_Printf("\2not found\n");
    DEBFILE("not found\n");
    return NULL;
}

//
// P_NetUnArchiveThinkers
//
void P_NetUnArchiveThinkers (void)
{
    thinker_t*          currentthinker;
    thinker_t*          next;
    mobj_t*             mobj;
    ULONG               diff;
    int                 i;
    byte                tclass;
    ceiling_t*          ceiling;
    vldoor_t*           door;
    floormove_t*        floor;
    plat_t*             plat;
    lightflash_t*       flash;
    strobe_t*           strobe;
    glow_t*             glow;
    fireflicker_t*      fireflicker;
    elevator_t*         elevator; //SoM: 3/15/2000
    scroll_t*           scroll;
    friction_t*         friction;
    pusher_t*           pusher;
	laserthink_t*       laser; // Graue 12-30-2003

    // remove all the current thinkers
    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        next = currentthinker->next;

        mobj = (mobj_t *)currentthinker;
        if (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
            // since this item isn't save don't remove it
/*            if( !((((mobj->flags & (MF_COUNTKILL | MF_PICKUP | MF_SHOOTABLE )) == 0)
                   && (mobj->flags & MF_MISSILE)
                   && (mobj->info->doomednum !=-1) )
                  || (mobj->type == MT_BLOOD) ) )
*/
            P_RemoveSavegameMobj ((mobj_t *)currentthinker);
        else
            Z_Free (currentthinker);

        currentthinker = next;
    }
    // BP: we don't want the removed mobj come back !!!
    iquetail = iquehead = 0 ;
    P_InitThinkers ();

    // read in saved thinkers
    while (1)
    {
        tclass = READBYTE(save_p);
        if( tclass == tc_end )
            break; // leave the while
        switch (tclass)
        {
          case tc_mobj :
              PADSAVEP();

              diff = READULONG(save_p);
              next = (void*)READULONG(save_p); // &mobj in the old system

              mobj = Z_Malloc (sizeof(mobj_t), PU_LEVEL, NULL);
              memset (mobj, 0, sizeof(mobj_t));

              mobj->z            = READFIXED(save_p);   // Force this so 3dfloor problems don't arise. Tails 03-17-2002
			  mobj->floorz       = READFIXED(save_p);

              if( diff & MD_SPAWNPOINT ) {  short spawnpointnum = READSHORT(save_p);
                                            mobj->spawnpoint = &mapthings[spawnpointnum];
                                            mapthings[spawnpointnum].mobj = mobj;
                                         }
              if( diff & MD_TYPE       )   mobj->type         = READULONG(save_p);
              else
              {
                  for (i=0 ; i< NUMMOBJTYPES ; i++)
                       if (mobj->spawnpoint->type == mobjinfo[i].doomednum)
                           break;
                  if ( i == NUMMOBJTYPES )
                      I_Error("Savegame corrupted\n");
                  mobj->type = i;
              }
              mobj->info = &mobjinfo[mobj->type];
              if( diff & MD_POS        ) { mobj->x            = READFIXED(save_p);
                                           mobj->y            = READFIXED(save_p);
                                           mobj->angle        = READANGLE(save_p); }
              else
              {
                  mobj->x      = mobj->spawnpoint->x << FRACBITS;
                  mobj->y      = mobj->spawnpoint->y << FRACBITS;
                  mobj->angle  = ANG45 * (mobj->spawnpoint->angle/45);
              }
              if( diff & MD_MOM      ){mobj->momx         = READFIXED(save_p);
                                       mobj->momy         = READFIXED(save_p);
                                       mobj->momz         = READFIXED(save_p); } // else null (memset)

              if( diff & MD_RADIUS   ) mobj->radius       = READFIXED(save_p);
                                  else mobj->radius       = mobj->info->radius;
              if( diff & MD_HEIGHT   ) mobj->height       = READFIXED(save_p);
                                  else mobj->height       = mobj->info->height;
              if( diff & MD_FLAGS    ) mobj->flags        = READLONG (save_p);
                                  else mobj->flags        = mobj->info->flags;
			  mobj->flags2       = READLONG (save_p);
              if( diff & MD_HEALTH   ) mobj->health       = READLONG (save_p);
                                  else mobj->health       = mobj->info->spawnhealth;
              if( diff & MD_RTIME    ) mobj->reactiontime = READLONG (save_p);
                                  else mobj->reactiontime = mobj->info->reactiontime;

              if( diff & MD_STATE    ) mobj->state        = &states[READUSHORT(save_p)];
                                  else mobj->state        = &states[mobj->info->spawnstate];
              if( diff & MD_TICS     ) mobj->tics         = READLONG (save_p);
                                  else mobj->tics         = mobj->state->tics;
              if( diff & MD_SPRITE   ) mobj->sprite       = READUSHORT(save_p);
                                  else mobj->sprite       = mobj->state->sprite;
              if( diff & MD_FRAME    ) mobj->frame        = READULONG(save_p);
                                  else mobj->frame        = mobj->state->frame;
              if( diff & MD_EFLAGS   ) mobj->eflags       = READULONG(save_p);
              if( diff & MD_PLAYER   ) {
                  i  = READBYTE (save_p);
                  mobj->player = &players[i];
                  mobj->player->mo = mobj;
                  // added for angle prediction
                  if( consoleplayer == i)                localangle=mobj->angle;
                  if( secondarydisplayplayer == i)       localangle2=mobj->angle;
              }
              if( diff & MD_MOVEDIR  ) mobj->movedir      = READLONG (save_p);
              if( diff & MD_MOVECOUNT) mobj->movecount    = READLONG (save_p);
              if( diff & MD_THRESHOLD) mobj->threshold    = READLONG (save_p);
              if( diff & MD_LASTLOOK ) mobj->lastlook     = READLONG (save_p);
                                  else mobj->lastlook     = -1;
              if( diff & MD_TARGET   ) mobj->target       = (mobj_t *)READULONG(save_p);
              if( diff & MD_TRACER   ) mobj->tracer       = (mobj_t *)READULONG(save_p);
              if( diff & MD_FRICTION ) mobj->friction     = READLONG (save_p);
                                  else mobj->friction     = ORIG_FRICTION;
              if( diff & MD_MOVEFACTOR)mobj->movefactor   = READLONG (save_p);
                                  else mobj->movefactor   = ORIG_FRICTION_FACTOR;
			  if( diff & MD_FUSE     ) mobj->fuse         = READLONG (save_p); // Tails
			  if( diff & MD_WATERTOP     ) mobj->watertop         = READLONG (save_p); // Tails
			  if( diff & MD_WATERBOTTOM     ) mobj->waterbottom         = READLONG (save_p); // Tails

              // now set deductable field
              // TODO : save this too
              mobj->skin = NULL;

              // set sprev, snext, bprev, bnext, subsector
              P_SetThingPosition (mobj);

			  // Special case for use of the block and sector links Tails 11-07-2002
			  if(mobj->type == MT_HOOPCOLLIDE || mobj->type == MT_HOOP)
			  {
				  unsigned int value;
				  
				  value = READULONG(save_p);

				  if(value != 0xFFFFFFFF)
					mobj->bnext = (mobj_t *)value;

				  value = READULONG(save_p);

				  if(value != 0xFFFFFFFF)
					mobj->bprev = (mobj_t *)value;

				  value = READULONG(save_p);

				  if(value != 0xFFFFFFFF)
					mobj->snext = (mobj_t *)value;

				  value = READULONG(save_p);

				  if(value != 0xFFFFFFFF)
					mobj->sprev = (mobj_t *)value;
			  }

			  mobj->mobjnum = READULONG(save_p); // Tails 04-01-2003

			  CONS_Printf("client mobjnum is %d\n", mobj->mobjnum);

//              mobj->floorz = mobj->subsector->sector->floorheight;
/*              if( (diff & MD_Z) == 0 )
                   mobj->z = mobj->floorz;*/ // This causes 3dfloor problems! Tails 03-17-2002
              if( mobj->player ) {
                  mobj->player->viewz = mobj->player->mo->z + mobj->player->viewheight;
                  //CONS_Printf("viewz = %f\n",FIXED_TO_FLOAT(mobj->player->viewz));
              }
              mobj->ceilingz = mobj->subsector->sector->ceilingheight;
              mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
              P_AddThinker (&mobj->thinker);

              mobj->info  = (mobjinfo_t *)next;  // temporarely, set when leave this function
              break;

          case tc_ceiling:
              PADSAVEP();
              ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVEL, NULL);
              memcpy (ceiling, save_p, sizeof(*ceiling));
              save_p += sizeof(*ceiling);
              ceiling->sector = &sectors[(int)ceiling->sector];
              ceiling->sector->ceilingdata = ceiling;

              if (ceiling->thinker.function.acp1)
                  ceiling->thinker.function.acp1 = (actionf_p1)T_MoveCeiling;

              P_AddThinker (&ceiling->thinker);
              P_AddActiveCeiling(ceiling);
              break;

		  case tc_crushceiling:
              PADSAVEP();
              ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVEL, NULL);
              memcpy (ceiling, save_p, sizeof(*ceiling));
              save_p += sizeof(*ceiling);
              ceiling->sector = &sectors[(int)ceiling->sector];
              ceiling->sector->ceilingdata = ceiling;

              if (ceiling->thinker.function.acp1)
                  ceiling->thinker.function.acp1 = (actionf_p1)T_CrushCeiling;

              P_AddThinker (&ceiling->thinker);
              P_AddActiveCeiling(ceiling);
              break;

          case tc_door:
              PADSAVEP();
              door = Z_Malloc (sizeof(*door), PU_LEVEL, NULL);
              memcpy (door, save_p, sizeof(*door));
              save_p += sizeof(*door);
              door->sector = &sectors[(int)door->sector];
              door->sector->ceilingdata = door;
              door->line   = &lines[(int)door->line];
              door->thinker.function.acp1 = (actionf_p1)T_VerticalDoor;
              P_AddThinker (&door->thinker);
              break;

          case tc_floor:
              PADSAVEP();
              floor = Z_Malloc (sizeof(*floor), PU_LEVEL, NULL);
              memcpy (floor, save_p, sizeof(*floor));
              save_p += sizeof(*floor);
              floor->sector = &sectors[(int)floor->sector];
              floor->sector->floordata = floor;
              floor->thinker.function.acp1 = (actionf_p1)T_MoveFloor;
              P_AddThinker (&floor->thinker);
              break;

          case tc_plat:
              PADSAVEP();
              plat = Z_Malloc (sizeof(*plat), PU_LEVEL, NULL);
              memcpy (plat, save_p, sizeof(*plat));
              save_p += sizeof(*plat);
              plat->sector = &sectors[(int)plat->sector];
              plat->sector->floordata = plat;

              if (plat->thinker.function.acp1)
                  plat->thinker.function.acp1 = (actionf_p1)T_PlatRaise;

              P_AddThinker (&plat->thinker);
              P_AddActivePlat(plat);
              break;

          case tc_flash:
              PADSAVEP();
              flash = Z_Malloc (sizeof(*flash), PU_LEVEL, NULL);
              memcpy (flash, save_p, sizeof(*flash));
              save_p += sizeof(*flash);
              flash->sector = &sectors[(int)flash->sector];
              flash->thinker.function.acp1 = (actionf_p1)T_LightFlash;
              P_AddThinker (&flash->thinker);
              break;

          case tc_strobe:
              PADSAVEP();
              strobe = Z_Malloc (sizeof(*strobe), PU_LEVEL, NULL);
              memcpy (strobe, save_p, sizeof(*strobe));
              save_p += sizeof(*strobe);
              strobe->sector = &sectors[(int)strobe->sector];
              strobe->thinker.function.acp1 = (actionf_p1)T_StrobeFlash;
              P_AddThinker (&strobe->thinker);
              break;

          case tc_glow:
              PADSAVEP();
              glow = Z_Malloc (sizeof(*glow), PU_LEVEL, NULL);
              memcpy (glow, save_p, sizeof(*glow));
              save_p += sizeof(*glow);
              glow->sector = &sectors[(int)glow->sector];
              glow->thinker.function.acp1 = (actionf_p1)T_Glow;
              P_AddThinker (&glow->thinker);
              break;

          case tc_fireflicker:
              PADSAVEP();
              fireflicker = Z_Malloc (sizeof(*fireflicker), PU_LEVEL, NULL);
              memcpy (fireflicker, save_p, sizeof(*fireflicker));
              save_p += sizeof(*fireflicker);
              fireflicker->sector = &sectors[(int)fireflicker->sector];
              fireflicker->thinker.function.acp1 = (actionf_p1)T_FireFlicker;
              P_AddThinker (&fireflicker->thinker);
            break;

          case tc_elevator:
              PADSAVEP();
              elevator = Z_Malloc (sizeof(elevator_t), PU_LEVEL, NULL);
              memcpy (elevator, save_p, sizeof(elevator_t));
              save_p += sizeof(elevator_t);
              elevator->sector = &sectors[(int)elevator->sector];
              elevator->sector->floordata = elevator; //jff 2/22/98
              elevator->sector->ceilingdata = elevator; //jff 2/22/98
              elevator->thinker.function.acp1 = (actionf_p1) T_MoveElevator;
              P_AddThinker (&elevator->thinker);
              break;

          case tc_bouncecheese:
              PADSAVEP();
              elevator = Z_Malloc (sizeof(elevator_t), PU_LEVEL, NULL);
              memcpy (elevator, save_p, sizeof(elevator_t));
              save_p += sizeof(elevator_t);
              elevator->sector = &sectors[(int)elevator->sector];
			  elevator->actionsector = &sectors[(int)elevator->actionsector];
              elevator->thinker.function.acp1 = (actionf_p1) T_BounceCheese;
              P_AddThinker (&elevator->thinker);
              break;

          case tc_startcrumble:
              PADSAVEP();
              elevator = Z_Malloc (sizeof(elevator_t), PU_LEVEL, NULL);
              memcpy (elevator, save_p, sizeof(elevator_t));
              save_p += sizeof(elevator_t);
              elevator->sector = &sectors[(int)elevator->sector];
			  elevator->actionsector = &sectors[(int)elevator->actionsector];
              elevator->sector->floordata = elevator; //jff 2/22/98
              elevator->thinker.function.acp1 = (actionf_p1) T_StartCrumble;
              P_AddThinker (&elevator->thinker);
              break;

          case tc_airbob:
              PADSAVEP();
              elevator = Z_Malloc (sizeof(elevator_t), PU_LEVEL, NULL);
              memcpy (elevator, save_p, sizeof(elevator_t));
              save_p += sizeof(elevator_t);
              elevator->sector = &sectors[(int)elevator->sector];
			  elevator->sector->ceilingdata = elevator; //jff 2/22/98
			  elevator->player = &players[(int)elevator->player];
              elevator->thinker.function.acp1 = (actionf_p1) T_AirBob;
              P_AddThinker (&elevator->thinker);
              break;

          case tc_marioblock:
              PADSAVEP();
              elevator = Z_Malloc (sizeof(elevator_t), PU_LEVEL, NULL);
              memcpy (elevator, save_p, sizeof(elevator_t));
              save_p += sizeof(elevator_t);
              elevator->sector = &sectors[(int)elevator->sector];
              elevator->sector->floordata = elevator; //jff 2/22/98
              elevator->sector->ceilingdata = elevator; //jff 2/22/98
              elevator->thinker.function.acp1 = (actionf_p1) T_MarioBlock;
              P_AddThinker (&elevator->thinker);
              break;

		  case tc_spikesector:
              PADSAVEP();
              elevator = Z_Malloc (sizeof(elevator_t), PU_LEVEL, NULL);
              memcpy (elevator, save_p, sizeof(elevator_t));
              save_p += sizeof(elevator_t);
              elevator->sector = &sectors[(int)elevator->sector];
              elevator->sector->floordata = elevator; //jff 2/22/98
              elevator->sector->ceilingdata = elevator; //jff 2/22/98
              elevator->thinker.function.acp1 = (actionf_p1) T_SpikeSector;
              P_AddThinker (&elevator->thinker);
              break;

		  case tc_floatsector:
              PADSAVEP();
              elevator = Z_Malloc (sizeof(elevator_t), PU_LEVEL, NULL);
              memcpy (elevator, save_p, sizeof(elevator_t));
              save_p += sizeof(elevator_t);
              elevator->sector = &sectors[(int)elevator->sector];
			  elevator->actionsector = &sectors[(int)elevator->actionsector];
              elevator->thinker.function.acp1 = (actionf_p1) T_FloatSector;
              P_AddThinker (&elevator->thinker);
              break;

		  // Graue 12-30-2003
		  case tc_laserflash:
              PADSAVEP();
              laser = Z_Malloc (sizeof(laserthink_t), PU_LEVEL, NULL);
              memcpy (laser, save_p, sizeof(laserthink_t));
              save_p += sizeof(laserthink_t);
			  CONS_Printf("recv: sector is %d, ffloor is %d\n", (int)laser->sector, (int)laser->ffloor);
              laser->sector = &sectors[(int)laser->sector];
			  laser->ffloor = &(laser->sector->ffloors)[(int)laser->ffloor];
              laser->thinker.function.acp1 = (actionf_p1) T_LaserFlash;
              P_AddThinker (&laser->thinker);
              break;

          case tc_scroll:
              scroll = Z_Malloc (sizeof(scroll_t), PU_LEVEL, NULL);
              memcpy (scroll, save_p, sizeof(scroll_t));
              save_p += sizeof(scroll_t);
              scroll->thinker.function.acp1 = (actionf_p1) T_Scroll;
              P_AddThinker(&scroll->thinker);
              break;
    
          case tc_friction:
              friction = Z_Malloc (sizeof(friction_t), PU_LEVEL, NULL);
              memcpy (friction, save_p, sizeof(friction_t));
              save_p += sizeof(friction_t);
              friction->thinker.function.acp1 = (actionf_p1) T_Friction;
              P_AddThinker(&friction->thinker);
              break;
    
          case tc_pusher:
              pusher = Z_Malloc (sizeof(pusher_t), PU_LEVEL, NULL);
              memcpy (pusher, save_p, sizeof(pusher_t));
              save_p += sizeof(pusher_t);
              pusher->thinker.function.acp1 = (actionf_p1) T_Pusher;
              pusher->source = P_GetPushThing(pusher->affectee);
              P_AddThinker(&pusher->thinker);
              break;

          default:
            I_Error ("P_UnarchiveSpecials:Unknown tclass %i "
                     "in savegame",tclass);
        }
    }
}


//
// P_FinishMobjs
// SoM: Delay this until AFTER we load fragglescript because FS needs this
// data!
void P_FinishMobjs()
{
    thinker_t*          currentthinker;
    mobj_t*             mobj;

    // put info field there real value
    for (currentthinker = thinkercap.next ; currentthinker != &thinkercap ; currentthinker=currentthinker->next)
    {
        if( currentthinker->function.acp1 == (actionf_p1)P_MobjThinker )
        {
            mobj = (mobj_t *)currentthinker;
            mobj->info = &mobjinfo[mobj->type];
        }
    }
}

void P_RelinkPointers() // Tails 04-01-2003
{
    thinker_t*          currentthinker;
    mobj_t*             mobj;

	// use info field (value = oldposition) to relink mobjs
    for (currentthinker = thinkercap.next ; currentthinker != &thinkercap ; currentthinker=currentthinker->next)
    {
        if( currentthinker->function.acp1 == (actionf_p1)P_MobjThinker )
		{
			mobj = (mobj_t *)currentthinker;
			if (mobj->tracer)
			{
				mobj->tracer = FindNewPosition((int)mobj->tracer);
				if( !mobj->tracer )
					CONS_Printf("tracer not found on %d\n",mobj->type);
			}
			if(mobj->target)
			{
				mobj->target = FindNewPosition((int)mobj->target);
				if( !mobj->target)
					CONS_Printf("target not found on %d\n",mobj->type);
			}
			if(mobj->type == MT_HOOPCOLLIDE || mobj->type == MT_HOOP)
			{
				if((int)mobj->bnext != 0xFFFFFFFF)
					mobj->bnext = FindNewPosition((int)mobj->bnext);
				else
					mobj->bnext = NULL;

				if((int)mobj->bprev != 0xFFFFFFFF)
					mobj->bprev = FindNewPosition((int)mobj->bprev);
				else
					mobj->bprev = NULL;

				if((int)mobj->snext != 0xFFFFFFFF)
					mobj->snext = FindNewPosition((int)mobj->snext);
				else
					mobj->snext = NULL;

				if((int)mobj->sprev != 0xFFFFFFFF)
					mobj->sprev = FindNewPosition((int)mobj->sprev);
				else
					mobj->sprev = NULL;

				if(!mobj->bnext)
					CONS_Printf("bnext not found on %d\n", mobj->type);

				if(!mobj->bprev)
					CONS_Printf("bprev not found on %d\n", mobj->type);

				if(!mobj->snext)
					CONS_Printf("snext not found on %d\n", mobj->type);

				if(!mobj->sprev)
					CONS_Printf("sprev not found on %d\n", mobj->type);
			}
			if(mobj->player && mobj->player->lastaxis)
			{
				mobj->player->lastaxis = FindNewPosition((int)mobj->player->lastaxis);
				if( !mobj->player->lastaxis)
					CONS_Printf("lastaxis not found on %d\n",mobj->type);
			}
			if(mobj->player && mobj->player->capsule)
			{
				mobj->player->capsule = FindNewPosition((int)mobj->player->capsule);
				if( !mobj->player->capsule)
					CONS_Printf("capsule not found on %d\n",mobj->type);
			}
        }
    }
}


//
// P_NetArchiveSpecials
//


// BP: added : itemrespawnqueue
//
void P_NetArchiveSpecials (void)
{
    int                 i;
	int z;

    // BP: added save itemrespawn queue for deathmatch
    i = iquetail;
    while (iquehead != i)
    {
		for(z = 0; z < nummapthings; z++)
		{
			if(&mapthings[z] == itemrespawnque[i])
			{
				WRITELONG(save_p,z);//itemrespawnque[i]-mapthings);
				break;
			}
		}
        WRITELONG(save_p,itemrespawntime[i]);
        i = (i+1)&(ITEMQUESIZE-1);
    }

    // end delimiter
    WRITELONG(save_p,0xffffffff);
}


//
// P_NetUnArchiveSpecials
//
void P_NetUnArchiveSpecials (void)
{
    int                 i;

    // BP: added save itemrespawn queue for deathmatch
    iquetail = iquehead = 0 ;
    while ((i = READLONG(save_p))!=0xffffffff)
    {
        itemrespawnque[iquehead]=&mapthings[i];
        itemrespawntime[iquehead++]=READLONG(save_p);
    }
}

// =======================================================================
//          Misc
// =======================================================================
void P_ArchiveMisc()
{
    ULONG pig=0;
    int i;

    WRITEBYTE ( save_p, gameskill);
    WRITEBYTE ( save_p, gameepisode);
    WRITESHORT ( save_p, gamemap);

	WRITEULONG (save_p, tokenlist); // Tails 12-18-2003
	WRITEULONG (save_p, token); // Tails 12-18-2003

	WRITEUSHORT(save_p, emeralds);
	WRITEBYTE(save_p, modifiedgame);

    for (i=0 ; i<MAXPLAYERS ; i++)
        pig |= (playeringame[i] != 0)<<i;

    WRITEULONG( save_p, pig );

	for(i=0; i<NUMMAPS; i++)
		WRITEBYTE(save_p, mapcleared[i]);
}

boolean P_UnArchiveMisc()
{
    ULONG pig;
    int i;

    gameskill   = READBYTE(save_p);
    gameepisode = READBYTE(save_p);
    gamemap     = READSHORT(save_p);

	tokenlist = READULONG(save_p); // Tails 12-18-2003
	token = READULONG(save_p); // Tails 12-18-2003

	emeralds    = READUSHORT(save_p);

	i = READBYTE(save_p);

	if(i == true && !modifiedgame)
		modifiedgame = i;

    pig         = READULONG(save_p);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        playeringame[i] = (pig & (1<<i))!=0;
        players[i].playerstate = PST_REBORN;
    }

	for(i=0; i<NUMMAPS; i++)
		mapcleared[i] = READBYTE(save_p);

    M_ClearRandom ();

    levelstarttic = gametic;        // for time calculation

    if (wipegamestate == GS_LEVEL)
        wipegamestate = -1;             // force a wipe

    gamestate = GS_LEVEL;

	for(i=0;i<MAXPLAYERS;i++)
		if(playeringame[i])
		{
			players[i].starposttime = 0; // Tails 07-04-2002
			players[i].starpostx = 0; // Tails 07-04-2002
			players[i].starposty = 0; // Tails 07-04-2002
			players[i].starpostz = 0; // Tails 07-04-2002
			players[i].starpostnum = 0; // Tails 07-05-2002
			players[i].starpostangle = 0; // Tails 07-04-2002
			players[i].starpostbit = 0; // Tails 07-05-2002
		}

    if( !P_SetupLevel (gameepisode, gamemap, gameskill, NULL) )
        return false;

	gameaction = ga_nothing;

    return true;
}

void P_NetArchiveMisc()
{
    ULONG pig=0;
    int i;

    WRITEBYTE ( save_p, gameskill);
    WRITEBYTE ( save_p, gameepisode);
    WRITESHORT ( save_p, gamemap);

	WRITEULONG(save_p, tokenlist); // Tails 12-18-2003

    for (i=0 ; i<MAXPLAYERS ; i++)
        pig |= (playeringame[i] != 0)<<i;

    WRITEULONG( save_p, pig );

	for(i=0; i<NUMMAPS; i++)
		WRITEBYTE(save_p, mapcleared[i]);

	WRITEULONG( save_p, leveltime );
	WRITEULONG( save_p, totalrings );
	WRITEULONG( save_p, lastmap );

	WRITEUSHORT(save_p, emeralds);

	WRITEULONG( save_p, token );
	WRITEULONG( save_p, sstimer );
	WRITEULONG( save_p, bluescore );
	WRITEULONG( save_p, redscore );

	WRITEULONG( save_p, countdown );
	WRITEULONG( save_p, countdown2 );

	WRITEFIXED( save_p, gravity );

	WRITEULONG( save_p, countdowntimer);
	WRITEBYTE(save_p, countdowntimeup);

    WRITEBYTE ( save_p, P_GetRandIndex());
}

boolean P_NetUnArchiveMisc()
{
    ULONG pig;
    int i;

    gameskill   = READBYTE(save_p);
    gameepisode = READBYTE(save_p);
    gamemap     = READSHORT(save_p);

	tokenlist = READULONG(save_p); // Tails 12-18-2003

    pig         = READULONG(save_p);

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        playeringame[i] = (pig & (1<<i))!=0;
        players[i].playerstate = PST_REBORN;
    }

	for(i=0; i<NUMMAPS; i++)
		mapcleared[i] = READBYTE(save_p);

    if( !P_SetupLevel (gameepisode, gamemap, gameskill, NULL) )
        return false;

    // get the time
    leveltime = READULONG(save_p);
    totalrings = READULONG(save_p);
	lastmap = READULONG(save_p);

	emeralds = READUSHORT(save_p);

	token = READULONG(save_p);
	sstimer = READULONG(save_p);
	bluescore = READULONG(save_p);
	redscore = READULONG(save_p);

	countdown = READULONG(save_p);
	countdown2 = READULONG(save_p);

	gravity = READFIXED(save_p);

	countdowntimer = READULONG(save_p);
	countdowntimeup = READBYTE(save_p);

    P_SetRandIndex(READBYTE(save_p));

    return true;
}

void P_SaveGame (void)
{
    P_ArchiveMisc();
    P_ArchivePlayers ();

    WRITEBYTE( save_p, 0x1d);           // consistancy marker
}

void P_SaveNetGame (void) // Do a full save for autojoin instead of SRB2 save Tails 01-22-2001
{
	extern boolean paused;

    CV_SaveNetVars((char**)&save_p);
    P_NetArchiveMisc();
    P_NetArchivePlayers ();
    P_NetArchiveWorld ();
    P_NetArchiveThinkers ();
    P_NetArchiveSpecials ();

	// Fixing the pause bug Graue 11-27-2003
	if(paused)
		WRITEBYTE( save_p, 0x2f);
	else
		WRITEBYTE( save_p, 0x2e);
    
    WRITEBYTE( save_p, 0x1d);           // consistancy marker
}

boolean P_LoadGame (void)
{
	int i;

    if( !P_UnArchiveMisc() )
        return false;
    P_UnArchivePlayers ();
	P_FinishMobjs();

	for(i=0;i<MAXPLAYERS;i++)
	{
		if(playeringame[i])
		{
			SetSavedSkin(i, players[i].skin, players[i].skincolor);
		}
	}

    return READBYTE(save_p)==0x1d;
}

boolean P_LoadNetGame (void) // Do a full save for autojoin instead of SRB2 save Tails 01-22-2001
{
	extern boolean paused;

    CV_LoadNetVars((char**)&save_p);
    if( !P_NetUnArchiveMisc() )
        return false;
    P_NetUnArchivePlayers ();
    P_NetUnArchiveWorld ();
    P_NetUnArchiveThinkers ();
    P_NetUnArchiveSpecials ();
	P_RelinkPointers();
	P_FinishMobjs();

	// Fixing the pause bug Graue 11-27-2003
	if(READBYTE(save_p) == 0x2f)
		paused = true;

    return READBYTE(save_p)==0x1d;
}