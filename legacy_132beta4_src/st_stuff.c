// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: st_stuff.c,v 1.21 2001/08/20 21:37:35 hurdler Exp $
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
// $Log: st_stuff.c,v $
// Revision 1.21  2001/08/20 21:37:35  hurdler
// fix palette in splitscreen + hardware mode
//
// Revision 1.20  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.19  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.18  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.17  2001/04/01 17:35:07  bpereira
// no message
//
// Revision 1.16  2001/03/03 06:17:34  bpereira
// no message
//
// Revision 1.15  2001/02/24 13:35:21  bpereira
// no message
//
// Revision 1.14  2001/02/10 13:05:45  hurdler
// no message
//
// Revision 1.13  2001/01/31 17:14:07  hurdler
// Add cv_scalestatusbar in hardware mode
//
// Revision 1.12  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.11  2000/11/02 19:49:37  bpereira
// no message
//
// Revision 1.10  2000/10/04 16:34:51  hurdler
// Change a little the presentation of monsters/secrets numbers
//
// Revision 1.9  2000/10/02 18:25:45  bpereira
// no message
//
// Revision 1.8  2000/10/01 10:18:19  bpereira
// no message
//
// Revision 1.7  2000/10/01 01:12:00  hurdler
// Add number of monsters and secrets in overlay
//
// Revision 1.6  2000/09/28 20:57:18  bpereira
// no message
//
// Revision 1.5  2000/09/25 19:28:15  hurdler
// Enable Direct3D support as OpenGL
//
// Revision 1.4  2000/09/21 16:45:09  bpereira
// no message
//
// Revision 1.3  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Status bar code.
//      Does the face/direction indicator animatin.
//      Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

#include "doomdef.h"

#include "am_map.h"

#include "g_game.h"
#include "m_cheat.h"

#include "screen.h"
#include "r_local.h"
#include "p_local.h"
#include "p_inter.h"
#include "m_random.h"
#include "f_wipe.h"

#include "st_stuff.h"
#include "st_lib.h"
#include "i_video.h"
#include "v_video.h"

#include "keys.h"

#include "z_zone.h"
#include "hu_stuff.h"
#include "d_main.h"

#ifdef HWRENDER
#include "hardware/hw_drv.h"
#include "hardware/hw_main.h"
#endif

extern consvar_t cv_storm; // Tails 06-07-2002

//
// STATUS BAR DATA
//

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13

// main player in game
//Hurdler: no not static!
player_t*        plyr;

// lump number for PLAYPAL
//static int              lu_palette;

// used when in chat
static st_chatstateenum_t       st_chatstate;

// whether status bar chat is active
static boolean          st_chat;

// value of st_chat before message popped up
static boolean          st_oldchat;

// whether chat window has the cursor on
static boolean          st_cursoron;

// 0-9, tall numbers
static patch_t*         tallnum[10];

// NiGHTS timer numbers
static patch_t*         nightsnum[10];

// face status patches
patch_t*         faceprefix[MAXPLAYERS];

// face background
static patch_t*         facenameprefix[MAXPLAYERS];

// ------------------------------------------
//             status bar overlay
// ------------------------------------------

// icons for overlay
static   patch_t*   sboscore;
static   patch_t*   sbohealth;
static   patch_t*   sboarmor;
static   patch_t*   sboover; // Tails 03-11-2000
static   patch_t*   stlivex; // Tails 03-12-2000
static   patch_t*   rrings; // Tails 03-14-2000
static   patch_t*   sbotime; // Time logo Tails 06-12-2000
static   patch_t*   sbocolon; // Colon for time Tails 01-03-2001
static	 patch_t*	getall; // Special Stage HUD Tails 08-11-2001
static	 patch_t*	timeup; // Special Stage HUD Tails 08-11-2001
static   patch_t*   homing1; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing2; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing3; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing4; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing5; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   homing6; // Emerald hunt indicators Tails 12-20-2001
static   patch_t*   supersonic; // Tails 08-25-2002
static   patch_t*   ttlnum; // Tails 11-01-2000
static   patch_t*   nightslink; // Tails 12-21-2002
static   patch_t*   count5;
static   patch_t*   count4;
static   patch_t*   count3;
static   patch_t*   count2;
static   patch_t*   count1;
static   patch_t*   count0;
static   patch_t*   homingring;
static   patch_t*   autoring;
static   patch_t*   explosionring;
static   patch_t*   railring;
static   patch_t*   infinityring;
static   patch_t*   blueshield;
static   patch_t*   redshield;
static   patch_t*   yellowshield;
static   patch_t*   greenshield;
static   patch_t*   blackshield;
static   patch_t*   invincibility;
static   patch_t*   sneakers;
static   patch_t*   emblemicon;
static   patch_t*   emerald1;
static   patch_t*   emerald2;
static   patch_t*   emerald3;
static   patch_t*   emerald4;
static   patch_t*   emerald5;
static   patch_t*   emerald6;
static   patch_t*   emerald7;
static   patch_t*   bluestat;
static   patch_t*   byelstat;
static   patch_t*   orngstat;
static   patch_t*   redstat;
static   patch_t*   yelstat;
static   patch_t*   nbracket;
static   patch_t*   nhud[12];
static   patch_t*   narrow[9];
static   patch_t*   minicaps;

static boolean facefreed[MAXPLAYERS];
static boolean prefixfreed[MAXPLAYERS];

void S_StartSound(void *origin, int sfx_id); // Tails 12-21-2001
void S_ChangeMusic(int music_num, int loooping);
void I_PlayCD(int track, boolean looping);
void I_OsPolling(void);
tic_t inline I_GetTime(void);
void M_Drawer(void);
int wipe_StartScreen(int x, int y, int width, int height);
int wipe_EndScreen(int x, int y, int width, int height);
int wipe_ScreenWipe(int wipeno, int x, int y, int width, int height, int ticks);


//
// STATUS BAR CODE
//

boolean ST_SameTeam(player_t *a,player_t *b)
{
	if(cv_gametype.value == GT_CTF)
		return a->skincolor == b->skincolor;
	else
		return false;

    switch (cv_teamplay.value) {
       case 0 : return false;
       case 1 : return (a->skincolor == b->skincolor);
       case 2 : return (a->skin == b->skin);
    }
    return false;
}

static boolean  st_stopped = true;

void ST_Ticker (void)
{
    if( st_stopped )
        return;
}

static int st_palette = 0;

void ST_doPaletteStuff(void)
{
//	extern consvar_t cv_grfogdensity;
    int         palette;

    if (plyr->bonuscount)
    {
        palette = (plyr->bonuscount+7)>>3;

        if (palette >= NUMBONUSPALS)
            palette = NUMBONUSPALS-1;

        palette += STARTBONUSPALS;
    }
	// Check if the camera is underwater
	// (used for palette changes) Tails 11-02-2000
	/*else if((camera.chase && (camera.mo->z + 8*FRACUNIT) < camera.mo->watertop && (camera.mo->z + 8*FRACUNIT) > camera.mo->waterbottom)
		|| (!camera.chase && (plyr->mo->z + plyr->viewheight) < plyr->mo->watertop && (plyr->mo->z + plyr->viewheight) > plyr->mo->waterbottom))
		palette = RADIATIONPAL;*/
// Fixing dumb bug from previous underwater palette - Tails 10-31-99
    else
		palette = 0;

    if (palette != st_palette)
    {
        st_palette = palette;

#ifdef HWRENDER // not win32 only 19990829 by Kin
        if ( (rendermode == render_opengl) || (rendermode == render_d3d) )
        
        //faB - NOW DO ONLY IN SOFTWARE MODE, LETS FIX THIS FOR GOOD OR NEVER
        //      idea : use a true color gradient from frame to frame, because we
        //             are in true color in HW3D, we can have smoother palette change
        //             than the palettes defined in the wad

        {HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0x0);
            /*//CONS_Printf("palette: %d\n", palette);
            switch (palette) {
                case 0x00: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0x0); break;  // pas de changement
                case 0x01: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff373797); break; // red
                case 0x02: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff373797); break; // red
                case 0x03: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff3030a7); break; // red
                case 0x04: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff2727b7); break; // red
                case 0x05: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff2020c7); break; // red
                case 0x06: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff1717d7); break; // red
                case 0x07: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff1010e7); break; // red
                case 0x08: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0x50505050); break; // Storm
                case 0x09: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff6060); break; // blue
                case 0x0a: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffffffff); break; // light green
                case 0x0b: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffffffff); break; // light green
                case 0x0c: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffffffff); break; // light green
                case 0x0d: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff0808); break; // green
                case 0x0e: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff6060); break; // blue
                case 0x0f: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff6060); break; // blue
            }*/
        }
        else
#endif
        {
			if(palette == RADIATIONPAL)
				V_SetPaletteLump("WATERPAL");
			else if (palette >= STARTBONUSPALS && palette <= STARTBONUSPALS+NUMBONUSPALS)
				V_SetPaletteLump("FLASHPAL");
			else
				V_SetPaletteLump("PLAYPAL");

            if( !cv_splitscreen.value || !palette )
                V_SetPalette (palette);
        }
    }
}

void ST_overlayDrawer ();

void ST_Drawer ( boolean refresh )
{
    //added:30-01-98:force a set of the palette by doPaletteStuff()
    if (vid.recalc)
        st_palette = -1;

    // Do red-/gold-shifts from damage/items
#ifdef HWRENDER // not win32 only 19990829 by Kin
//25/08/99: Hurdler: palette changes is done for all players,
//                   not only player1 ! That's why this part 
//                   of code is moved somewhere else.
    if (rendermode==render_soft)
#endif
        ST_doPaletteStuff();

    if( st_overlay )
    {
		// No deadview! Tails
        plyr=&players[displayplayer];
        ST_overlayDrawer ();

        if( cv_splitscreen.value )
        {
            plyr=&players[secondarydisplayplayer];
            ST_overlayDrawer ();
        }
    }
}


static void ST_loadGraphics(void)
{

    int         i;
    char        namebuf[9];

    // Load the numbers, tall and short
    for (i=0;i<10;i++)
    {
        sprintf(namebuf, "STTNUM%d", i);
        tallnum[i] = (patch_t *) W_CachePatchName(namebuf, PU_STATIC);
		sprintf(namebuf, "NGTNUM%d", i);
		nightsnum[i] = (patch_t *) W_CachePatchName(namebuf, PU_STATIC);
    }

    // the original Doom uses 'STF' as base name for all face graphics
	for(i=0; i<MAXPLAYERS; i++)
	{
		ST_loadFaceGraphics ("SBOSLIFE", i); // Tails 03-15-2002
		ST_loadFaceNameGraphics ("STSONIC", i); // Tails 03-15-2002
	}
}


// made separate so that skins code can reload custom face graphics
void ST_loadFaceGraphics (char *facestr, int playernum)
{
    char  namelump[9];

    //hack: make sure base face name is no more than 8 chars
    // bug: core dump fixed 19990220 by Kin
    if( strlen(facestr)>8 )
        facestr[8]='\0';
    strcpy (namelump, facestr);  // copy base name

    faceprefix[playernum] = W_CachePatchName(namelump, PU_STATIC);
	facefreed[playernum] = false;
}

// Tails 03-15-2002
// made separate so that skins code can reload custom face graphics
void ST_loadFaceNameGraphics (char *facestr, int playernum)
{
    char  namelump[9];

    //hack: make sure base face name is no more than 8 chars
    // bug: core dump fixed 19990220 by Kin
    if( strlen(facestr)>8 )
        facestr[8]='\0';
    strcpy (namelump, facestr);  // copy base name

    facenameprefix[playernum] = W_CachePatchName(namelump, PU_STATIC);
	prefixfreed[playernum] = false;
}


static void ST_loadData(void)
{
    ST_loadGraphics();
}

void ST_unloadGraphics(void)
{

    int i;

    //faB: GlidePatch_t are always purgeable
    if (rendermode==render_soft)
    {
        // unload the numbers, tall and short
        for (i=0;i<10;i++)
        {
            Z_ChangeTag(tallnum[i], PU_CACHE);
			Z_ChangeTag(nightsnum[i], PU_CACHE);
        }
    }

    ST_unloadFaceGraphics ();
    ST_unloadFaceNameGraphics (); // Tails 03-15-2002

    // Note: nobody ain't seen no unloading
    //   of stminus yet. Dude.

}

// made separate so that skins code can reload custom face graphics
void ST_unloadFaceGraphics (void)
{
    //faB: GlidePatch_t are always purgeable
    if (rendermode==render_soft)
    {
		int i;
		for(i=0;i<MAXPLAYERS;i++)
		{
			if(facefreed[i] == false)
			{
				Z_Free(faceprefix[i]);
				facefreed[i] = true;
			}
		}
    }
}

// Tails 03-15-2002
// made separate so that skins code can reload custom face graphics
void ST_unloadFaceNameGraphics (void)
{
    //faB: GlidePatch_t are always purgeable
    if (rendermode==render_soft)
    {
		int i;
		for(i=0;i<MAXPLAYERS;i++)
		{
			if(prefixfreed[i] == false)
			{
				Z_Free(facenameprefix[i]);
				prefixfreed[i] = true;
			}
		}
    }
}

// made separate so that skins code can reload custom face graphics
void ST_unloadFaceGraphic (int playernum)
{
	if(facefreed[playernum] = true)
		return;

    //faB: GlidePatch_t are always purgeable
    if (rendermode==render_soft)
    {
		Z_Free(faceprefix[playernum]);
		facefreed[playernum] = true;
    }
}

// Tails 03-15-2002
// made separate so that skins code can reload custom face graphics
void ST_unloadFaceNameGraphic (int playernum)
{
	if(prefixfreed[playernum] = true)
		return;

    //faB: GlidePatch_t are always purgeable
    if (rendermode==render_soft)
    {
		Z_Free(facenameprefix[playernum]);
		prefixfreed[playernum] = true;
    }
}

void ST_unloadData(void)
{
    ST_unloadGraphics();
}

void ST_initData(void)
{
    //added:16-01-98:'link' the statusbar display to a player, which could be
    //               another player than consoleplayer, for example, when you
    //               change the view in a multiplayer demo with F12.
	// Now works for gametypes 2 and 6, race and circuit Graue 12-06-2003
    if (singledemo || cv_gametype.value == GT_RACE || cv_gametype.value == GT_CIRCUIT) // Tails
        statusbarplayer = displayplayer;
    else
        statusbarplayer = consoleplayer;

    plyr = &players[statusbarplayer];

    st_chatstate = StartChatState;

    st_oldchat = st_chat = false;
    st_cursoron = false;

    st_palette = -1;
}

static void ST_Stop (void)
{
    if (st_stopped)
        return;

    V_SetPalette (0);

    st_stopped = true;
}

void ST_Start (void)
{
    if (!st_stopped)
        ST_Stop();

    ST_initData();
    st_stopped = false;
}

//
//  Initializes the status bar,
//  sets the defaults border patch for the window borders.
//

//faB: used by Glide mode, holds lumpnum of flat used to fill space around the viewwindow
int  st_borderpatchnum;

void ST_Init (void)
{
	int i;

	for(i=0; i<MAXPLAYERS; i++)
	{
		facefreed[i] = true;
		prefixfreed[i] = true;
	}

//    if(dedicated)
//		return;
    
    //added:26-01-98:screens[4] is allocated at videomode setup, and
    //               set at V_Init(), the first time being at SCR_Recalc()

    // DOOM II border patch, original was GRNROCK
	st_borderpatchnum = W_GetNumForName ("FLOOR0_3"); // Tails 10-26-99

    scr_borderpatch = W_CacheLumpNum (st_borderpatchnum, PU_STATIC);

    ST_loadData();

    //
    // cache the status bar overlay icons  (fullscreen mode)
    //
    sbohealth = W_CachePatchName ("SBOHEALT", PU_STATIC); // Tails
    sboscore  = W_CachePatchName ("SBOFRAGS", PU_STATIC); // Tails
    sboarmor  = W_CachePatchName ("SBOARMOR", PU_STATIC); // Tails
    sboover   = W_CachePatchName ("SBOOVER", PU_STATIC); // Tails 03-11-2000
    stlivex   = W_CachePatchName ("STLIVEX", PU_STATIC); // Tails 03-12-2000
    rrings    = W_CachePatchName ("SBORINGS", PU_STATIC); // Tails 03-14-2000
//    colon     = W_CachePatchName ("WICOLON", PU_STATIC); // Tails 03-14-2000
    sbotime     = W_CachePatchName ("SBOTIME", PU_STATIC); // Time logo Tails 06-12-2000
	sbocolon = W_CachePatchName ("SBOCOLON", PU_STATIC); // Colon for time Tails 01-03-2001
	getall     = W_CachePatchName ("GETALL", PU_STATIC); // Special Stage HUD Tails 08-11-2001
	timeup	   = W_CachePatchName ("TIMEUP", PU_STATIC); // Special Stage HUD Tails 08-11-2001
	homing1	   = W_CachePatchName ("HOMING1", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing2	   = W_CachePatchName ("HOMING2", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing3	   = W_CachePatchName ("HOMING3", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing4	   = W_CachePatchName ("HOMING4", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing5	   = W_CachePatchName ("HOMING5", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	homing6	   = W_CachePatchName ("HOMING6", PU_STATIC); // Emerald hunt indicators Tails 12-20-2001
	supersonic = W_CachePatchName ("SUPERICO", PU_STATIC); // Tails 08-25-2002
	nightslink = W_CachePatchName ("NGHTLINK", PU_STATIC);
	count5     = W_CachePatchName("CNTFA0", PU_STATIC);
	count4     = W_CachePatchName("CNTEA0", PU_STATIC);
	count3     = W_CachePatchName("CNTDA0", PU_STATIC);
	count2     = W_CachePatchName("CNTCA0", PU_STATIC);
	count1     = W_CachePatchName("CNTBA0", PU_STATIC);
	count0     = W_CachePatchName("CNTAA0", PU_STATIC);

	homingring    = W_CachePatchName("HOMNIND",PU_STATIC);
	autoring      = W_CachePatchName("AUTOIND",PU_STATIC);
	explosionring = W_CachePatchName("BOMBIND",PU_STATIC);
	railring      = W_CachePatchName("RAILIND",PU_STATIC);
	infinityring  = W_CachePatchName("INFNIND",PU_STATIC);
	blueshield    = W_CachePatchName("BLTVB0",PU_STATIC);
	redshield     = W_CachePatchName("RDTVB0",PU_STATIC);
	yellowshield  = W_CachePatchName("YLTVB0",PU_STATIC);
	greenshield   = W_CachePatchName("GRTVB0",PU_STATIC);
	blackshield   = W_CachePatchName("BKTVB0",PU_STATIC);
	invincibility = W_CachePatchName("PINVB0",PU_STATIC);
	sneakers      = W_CachePatchName("SHTVB0",PU_STATIC);
	emblemicon    = W_CachePatchName("EMBLICON", PU_STATIC);
	emerald1      = W_CachePatchName("CHAOS1", PU_STATIC);
	emerald2      = W_CachePatchName("CHAOS2", PU_STATIC);
	emerald3      = W_CachePatchName("CHAOS3", PU_STATIC);
	emerald4      = W_CachePatchName("CHAOS4", PU_STATIC);
	emerald5      = W_CachePatchName("CHAOS5", PU_STATIC);
	emerald6      = W_CachePatchName("CHAOS6", PU_STATIC);
	emerald7      = W_CachePatchName("CHAOS7", PU_STATIC);

	// NiGHTS HUD things Tails 12-17-2003
	bluestat = W_CachePatchName("BLUESTAT", PU_STATIC);
	byelstat = W_CachePatchName("BYELSTAT", PU_STATIC);
	orngstat = W_CachePatchName("ORNGSTAT", PU_STATIC);
	redstat = W_CachePatchName("REDSTAT", PU_STATIC);
	yelstat = W_CachePatchName("YELSTAT", PU_STATIC);
	nbracket = W_CachePatchName("NBRACKET", PU_STATIC);
	nhud[0] = W_CachePatchName("NHUD1", PU_STATIC);
	nhud[1] = W_CachePatchName("NHUD2", PU_STATIC);
	nhud[2] = W_CachePatchName("NHUD3", PU_STATIC);
	nhud[3] = W_CachePatchName("NHUD4", PU_STATIC);
	nhud[4] = W_CachePatchName("NHUD5", PU_STATIC);
	nhud[5] = W_CachePatchName("NHUD6", PU_STATIC);
	nhud[6] = W_CachePatchName("NHUD7", PU_STATIC);
	nhud[7] = W_CachePatchName("NHUD8", PU_STATIC);
	nhud[8] = W_CachePatchName("NHUD9", PU_STATIC);
	nhud[9] = W_CachePatchName("NHUD10", PU_STATIC);
	nhud[10] = W_CachePatchName("NHUD11", PU_STATIC);
	nhud[11] = W_CachePatchName("NHUD12", PU_STATIC);
	minicaps = W_CachePatchName("MINICAPS", PU_STATIC);

	narrow[0] = W_CachePatchName("NARROW1", PU_STATIC);
	narrow[1] = W_CachePatchName("NARROW2", PU_STATIC);
	narrow[2] = W_CachePatchName("NARROW3", PU_STATIC);
	narrow[3] = W_CachePatchName("NARROW4", PU_STATIC);
	narrow[4] = W_CachePatchName("NARROW5", PU_STATIC);
	narrow[5] = W_CachePatchName("NARROW6", PU_STATIC);
	narrow[6] = W_CachePatchName("NARROW7", PU_STATIC);
	narrow[7] = W_CachePatchName("NARROW8", PU_STATIC);

	// non-animated version
	narrow[8] = W_CachePatchName("NARROW9", PU_STATIC);
}

 //added:16-01-98: change the status bar too, when pressing F12 while viewing
//                 a demo.
void ST_changeDemoView (void)
{
    //the same routine is called at multiplayer deathmatch spawn
    // so it can be called multiple times
    ST_Start();
}


// =========================================================================
//                         STATUS BAR OVERLAY
// =========================================================================

boolean   st_overlay;

static inline int SCY( int y )
{ 
    //31/10/99: fixed by Hurdler so it _works_ also in hardware mode
    // do not scale to resolution for hardware accelerated
    // because these modes always scale by default
    y = y * vid.fdupy;     // scale to resolution
    if ( cv_splitscreen.value ) {
        y >>= 1;
        if (plyr != &players[statusbarplayer])
            y += vid.height / 2;
    }
    return y;
}

static inline int STRINGY( int y )
{ 
    //31/10/99: fixed by Hurdler so it _works_ also in hardware mode
    // do not scale to resolution for hardware accelerated
    // because these modes always scale by default
    if ( cv_splitscreen.value ) {
        y >>= 1;
        if (plyr != &players[statusbarplayer])
            y += BASEVIDHEIGHT / 2;
    }
    return y;
}

static inline int SCX( int x )
{
    return x * vid.fdupx;
}


//  Draw a number, scaled, over the view
//  Always draw the number completely since it's overlay
//
void ST_drawOverlayNum (int       x,            // right border!
                        int       y,
                        int       num,
                        patch_t** numpat,
                        patch_t*  percent )
{
    int       w = (numpat[0]->width);
    boolean   neg;

    // in the special case of 0, you draw 0
    if (!num)
    {
        V_DrawScaledPatch(x - (w*vid.dupx), y, FG|V_NOSCALESTART, numpat[ 0 ]);
        return;
    }

    neg = num < 0;

    if (neg)
        num = -num;

    // draw the number
    while (num)
    {
        x -= (w * vid.dupx);
        V_DrawScaledPatch(x, y, FG|V_NOSCALESTART, numpat[ num % 10 ]);
        num /= 10;
    }

    // draw a minus sign if necessary
    if (neg)
        V_DrawScaledPatch(x - (8*vid.dupx), y, FG|V_NOSCALESTART, (patch_t*)W_CachePatchName("STTMINUS", PU_STATIC)); // Tails
}

//  Draw a number, scaled, over the view
//  Always draw the number completely since it's overlay
//
// Supports different colors! woo! Tails 12-21-2002
void ST_drawNightsOverlayNum (int       x,            // right border!
                        int       y,
                        int       num,
                        patch_t** numpat,
                        int colornum )
{
    int       w = (numpat[0]->width);
    boolean   neg;
	byte* colormap;
	int flags = 0;

	flags = (flags & ~MF_TRANSLATION) | (colornum<<MF_TRANSSHIFT);

	// Uses the player colors.
	colormap = (byte *) defaulttranslationtables - 256 + ( (flags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) );

    // in the special case of 0, you draw 0
    if (!num)
    {
        V_DrawMappedPatch(x - (w*vid.dupx), y, FG|V_NOSCALESTART, numpat[ 0 ], colormap);
        return;
    }

    neg = num < 0;

    if (neg)
        num = -num;

    // draw the number
    while (num)
    {
        x -= (w * vid.dupx);
        V_DrawMappedPatch(x, y, FG|V_NOSCALESTART, numpat[ num % 10 ], colormap);
        num /= 10;
    }

	// Sorry chum, this function only draws UNSIGNED values!
}

void WI_drawIntRanking(char *title,int x,int y,fragsort_t *fragtable
                   , int scorelines, boolean large, int white)
{
    int   i,j;
    char  num[12];
    int   plnum;
    int   frags;
    int   colornum;
    fragsort_t temp;

    colornum = 0x78;

    // sort the frags count
    for (i=0; i<scorelines; i++)
        for(j=0; j<scorelines-1-i; j++)
            if( fragtable[j].count < fragtable[j+1].count )
            {
                temp = fragtable[j];
                fragtable[j] = fragtable[j+1];
                fragtable[j+1] = temp;
            }

    if(title)
        V_DrawString (x, y-14, 0, title);
    // draw rankings
    for (i=0; i<scorelines; i++)
    {
        frags = fragtable[i].count;
        plnum = fragtable[i].num;

		if(cv_gametype.value != GT_CTF)
		{
			byte* colormap;

			if(players[plnum].skincolor==0)
				colormap = colormaps;
			else
				colormap = (byte *) translationtables[players[plnum].skin] - 256 + (players[plnum].skincolor<<8);

			V_DrawSmallScaledPatch (SCX(x-24),(y-4)*vid.fdupy, V_NOSCALESTART,faceprefix[plnum], colormap); // Tails 03-11-2000
		}

        // draw frags count
        sprintf(num,"%3i", frags );
        V_DrawString (x+(large ? 32+144 : 24+160)-V_StringWidth(num), y, 0, num);

        // draw name
		if (cv_gametype.value != GT_CTF) // Tails 08-03-2001
			V_DrawString (x+(large ? 0 : 29), y, plnum == white ? V_WHITEMAP : 0, fragtable[i].name);
		else
			V_DrawString (x+(large ? 0 : 29), y, i == 0 ? V_WHITEMAP : 0, fragtable[i].color == 6 ? "RED" : "BLUE"); 

        y += 16;
        if (y>=BASEVIDHEIGHT)
            break;            // dont draw past bottom of screen
    }
}

void WI_drawCircuitRanking(char *title,int x,int y,fragsort_t *fragtable
                   , int scorelines, boolean large, int white)
{
    int   i,j;
    char  num[33];
    int   plnum;
    int   frags;
    int   colornum;
    fragsort_t temp;

    colornum = 0x78;

    // sort the frags count
    for (i=0; i<scorelines; i++)
        for(j=0; j<scorelines-1-i; j++)
            if( fragtable[j].count > fragtable[j+1].count )
            {
                temp = fragtable[j];
                fragtable[j] = fragtable[j+1];
                fragtable[j+1] = temp;
            }

    if(title)
        V_DrawString (x, y-14, 0, title);
    // draw rankings
    for (i=0; i<scorelines; i++)
    {
		byte* colormap;
        frags = fragtable[i].count;
        plnum = fragtable[i].num;


		if(players[plnum].skincolor==0)
			colormap = colormaps;
		else
			colormap = (byte *) translationtables[players[plnum].skin] - 256 + (players[plnum].skincolor<<8);

		V_DrawSmallScaledPatch (SCX(x-24),(y-4)*vid.fdupy, V_NOSCALESTART,faceprefix[plnum], colormap); // Tails 03-11-2000

		// draw frags count
		sprintf(num,"%3i:%s%i:%s%i", frags / (TICRATE * 60),                  // minutes
			((frags / TICRATE) % 60 < 10) ? "0" : "", (frags / TICRATE) % 60, // seconds
			((frags % TICRATE) < 10) ? "0" : "", frags % TICRATE);            // tics

        V_DrawString (x+(large ? 32+144 : 24+160)-V_StringWidth(num), y, 0, num);

        // draw name
		V_DrawString (x+(large ? 0 : 29), y, plnum == white ? V_WHITEMAP : 0, fragtable[i].name);

        y += 16;
        if (y>=BASEVIDHEIGHT)
            break;            // dont draw past bottom of screen
    }
}

extern consvar_t cv_objectplace;
extern consvar_t cv_snapto;
extern consvar_t cv_objflags;

//  Quick-patch for the Cave party 19-04-1998 !!
//
void WI_drawRancking(char *title,int x,int y,fragsort_t *fragtable
                   , int scorelines, boolean large, int white)
{
    int   i,j;
    char  num[12];
    int   plnum;
    int   frags;
    int   colornum;
    fragsort_t temp;
	byte* colormap;

    colornum = 0x78;

    // sort the frags count
    for (i=0; i<scorelines; i++)
        for(j=0; j<scorelines-1-i; j++)
            if( fragtable[j].count < fragtable[j+1].count )
            {
                temp = fragtable[j];
                fragtable[j] = fragtable[j+1];
                fragtable[j+1] = temp;
            }

    if(title)
        V_DrawString (x, y-14, 0, title);
    // draw rankings

	if(cv_gametype.value == GT_CTF)
	{
		V_DrawFill (127-16,3, 32, 10,176);
		V_DrawCenteredString (128, 4, redscore > bluescore ? V_WHITEMAP : 0, "RED"); 
        sprintf(num,"%i", redscore );
		V_DrawFill(128-(V_StringWidth(num))/2, 12, V_StringWidth(num), 8, 176);
        V_DrawCenteredString (128, 12, 0, num);

		V_DrawFill (BASEVIDWIDTH-128-16,3, 32, 10,200);
		V_DrawCenteredString (BASEVIDWIDTH-128, 4, bluescore > redscore ? V_WHITEMAP : 0, "BLUE"); 
        sprintf(num,"%i", bluescore );
		V_DrawFill(BASEVIDWIDTH-128-(V_StringWidth(num))/2, 12, V_StringWidth(num), 8, 200);
        V_DrawCenteredString (BASEVIDWIDTH-128, 12, 0, num);
	}


    for (i=0; i<scorelines; i++)
    {
        frags = fragtable[i].count;
        plnum = fragtable[i].num;

		if(players[plnum].skincolor==0)
			colormap = colormaps;
		else
			colormap = (byte *) translationtables[players[plnum].skin] - 256 + (players[plnum].skincolor<<8);

		V_DrawSmallScaledPatch (SCX(x-24),(y-4)*vid.fdupy, V_NOSCALESTART,faceprefix[plnum], colormap); // Tails 03-11-2000

        // draw frags count
        sprintf(num,"%3i", frags );
        V_DrawString (x+(large ? 32+144 : 24+160)-V_StringWidth(num), y, 0, num);

        // draw name
		V_DrawString (x+(large ? 0 : 29), y, plnum == white ? V_WHITEMAP : 0, fragtable[i].name);

        y += 16;
        if (y>=BASEVIDHEIGHT)
            break;            // dont draw past bottom of screen
    }

	if(cv_timelimit.value)
	{
		V_DrawString(240,0, 0, "Time Left");
		V_DrawString(264-strlen(va("%d", ((cv_timelimit.value*TICRATE*60)-leveltime)/TICRATE))/2,8, 0, va("%d", ((cv_timelimit.value*TICRATE*60)-leveltime)/TICRATE));
	}
}

extern consvar_t cv_chasecam;
extern consvar_t cv_chasecam2;
extern consvar_t cv_lapcounteridiocy; // Graue 12-25-2003
extern boolean circintrodone; // Graue 12-24-2003

//  Draw the status bar overlay, customisable : the user choose which
//  kind of information to overlay
//
void ST_overlayDrawer ()
{
	int splity;

	if(cv_splitscreen.value)
		splity = 24;
	else
		splity = 0;

	if(cv_gametype.value == GT_CHAOS) // Chaos Chain value
	{
		char chains[33];
		sprintf(chains, "CHAINS: %d", plyr->scoreadd);
		V_DrawString(8, STRINGY(184), 0, chains);
	}
	else if(plyr->linkcount)
	{
		int colornum;

		colornum = (plyr->linkcount / 5)%14;

		if(cv_splitscreen.value)
		{
			ST_drawNightsOverlayNum(SCX(256), SCY(160),
				plyr->linkcount, nightsnum, colornum);
			V_DrawMappedPatch(SCX(264), SCY(160), FG|V_NOSCALESTART, nightslink
				, (byte *) defaulttranslationtables - 256 + ( ((0 & ~MF_TRANSLATION) | (colornum<<MF_TRANSSHIFT) & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) ));
		}
		else
		{
			ST_drawNightsOverlayNum(SCX(160), SCY(176),
				plyr->linkcount, nightsnum, colornum);
			V_DrawMappedPatch(SCX(168), SCY(176), FG|V_NOSCALESTART, nightslink
				, (byte *) defaulttranslationtables - 256 + ( ((0 & ~MF_TRANSLATION) | (colornum<<MF_TRANSSHIFT) & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) ));
		}
	}

	if(plyr->nightsmode)
	{
		if(!(plyr->drillmeter & 1))
		{
			V_DrawFill(14, STRINGY(142), 100, 8, 64);
			V_DrawFill(16, STRINGY(144), 96, 4, 0);
			V_DrawFill(16, STRINGY(144), plyr->drillmeter/20, 4, 112);
		}
		else
		{
			V_DrawFill(14, STRINGY(142), 100, 8, 144);
			V_DrawFill(16, STRINGY(144), 96, 4, 88);
			V_DrawFill(16, STRINGY(144), plyr->drillmeter/20, 4, 116);
		}
	}

	if(plyr->bonustime > 1)
	{
		V_DrawCenteredString(BASEVIDWIDTH/2, STRINGY(100), 0, "BONUS TIME START!");
	}

	// start GAME OVER pic Tails 03-11-2000
	if((cv_gametype.value == GT_COOP || cv_gametype.value == GT_RACE) && plyr->lives <= 0)
	{
		V_DrawScaledPatch (SCX(32),SCY(100)-((sboover->height/2)*vid.dupy), FG | V_NOSCALESTART, sboover); // Tails 03-11-2000
	}
	// end GAME OVER pic Tails 03-11-2000

	// start lives status Tails 03-12-2000
	if((cv_gametype.value == GT_COOP || cv_gametype.value == GT_RACE))
	{
		V_DrawScaledPatch (SCX(52),SCY(192)-(faceprefix[plyr-players]->height*vid.dupy), FG | V_NOSCALESTART,facenameprefix[plyr-players]); // Tails 03-11-2000

		if(plyr->powers[pw_super])
			V_DrawScaledPatch (SCX(16),SCY(192)-(faceprefix[plyr-players]->height*vid.dupy), FG | V_NOSCALESTART,supersonic); // Tails 03-11-2000	
		else
		{
			byte* colormap;

			if(plyr->skincolor==0)
				colormap = colormaps;
			else
				colormap = (byte *) translationtables[plyr->skin] - 256 + (plyr->skincolor<<8);

			V_DrawMappedPatch (SCX(16),SCY(192)-(faceprefix[plyr-players]->height*vid.dupy), FG | V_NOSCALESTART,faceprefix[plyr-players], colormap); // Tails 03-11-2000
		}

		// draw the number of lives
		ST_drawOverlayNum(SCX(88), // was 50 Tails 10-31-99
						SCY(192)-(11*vid.dupy),
						plyr->lives,
						tallnum,NULL);

		// now draw the "x"
		if(cv_splitscreen.value)
			V_DrawScaledPatch (SCX(56),SCY(176), FG | V_NOSCALESTART, stlivex);
		else
			V_DrawScaledPatch (SCX(56),SCY(192)-(stlivex->height*vid.dupy), FG | V_NOSCALESTART, stlivex);
	}

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS)
	{
		V_DrawScaledPatch (SCX(16), SCY(8), FG | V_NOSCALESTART, nbracket);

		V_DrawScaledPatch (SCX(24), SCY(8)+8*vid.fdupy, FG | V_NOSCALESTART, nhud[(leveltime/2)%12]);

		if(plyr->capsule)
		{
			int amount;
			int origamount;
			const int length = 88;

			V_DrawScaledPatch (SCX(72), SCY(8), FG | V_NOSCALESTART, nbracket);
			V_DrawScaledPatch(SCX(74), SCY(8)+4*vid.fdupy, FG | V_NOSCALESTART, minicaps);


			if(plyr->capsule->reactiontime != 0)
			{
				int r;
				const int orblength = 20;

				for(r=0; r<5; r++)
				{
					V_DrawScaledPatch(SCX(230-(7*r)), SCY(144), FG |V_NOSCALESTART, redstat);
					V_DrawScaledPatch(SCX(188-(7*r)), SCY(144), FG |V_NOSCALESTART, orngstat);
					V_DrawScaledPatch(SCX(146-(7*r)), SCY(144), FG |V_NOSCALESTART, yelstat);
					V_DrawScaledPatch(SCX(104-(7*r)), SCY(144), FG |V_NOSCALESTART, byelstat);
				}
				origamount = plyr->capsule->spawnpoint->angle&1023;

				amount = (origamount - plyr->capsule->health);

				amount = amount * ((float)orblength/origamount);

				if(amount > 0)
				{
					int t;

					// Fill up the bar with blue orbs... in reverse! (yuck)
					for(r = amount; r >= 0; r--)
					{
						t = r;

						if(r > 14)
							t += 1;
						if(r > 9)
							t += 1;
						if(r > 4)
							t += 1;

						V_DrawScaledPatch(SCX(76+(7*t)), SCY(144), FG |V_NOSCALESTART, bluestat);
					}
				}
			}				
			else
			{
				// Lil' white box!
				V_DrawFill(15, STRINGY(8)+34, length+2, 5, 4);

				V_DrawFill(16, STRINGY(8)+35, length/4, 3, 231);
				V_DrawFill(16+length/4, STRINGY(8)+35, length/4, 3, 248);
				V_DrawFill(16+(length/4)*2, STRINGY(8)+35, length/4, 3, 215);
				V_DrawFill(16+(length/4)*3, STRINGY(8)+35, length/4, 3, 179);
				origamount = plyr->capsule->spawnpoint->angle&1023;

				amount = (origamount - plyr->capsule->health);

				amount = amount * ((float)length/origamount);

				if(amount > 0)
					V_DrawFill(16, STRINGY(8)+35, amount, 3, 197);
			}
			V_DrawScaledPatch (SCX(40), SCY(8)+5*vid.fdupy, FG | V_NOSCALESTART, narrow[(leveltime/2)%8]);
		}
		else
			V_DrawScaledPatch (SCX(40), SCY(8)+5*vid.fdupy, FG | V_NOSCALESTART, narrow[8]);

		ST_drawOverlayNum(SCX(68), SCY(8)+11*vid.fdupy, plyr->health-1, tallnum, NULL);

		ST_drawNightsOverlayNum(SCX(288), SCY(12), plyr->score, nightsnum, 7); // Blue
	}
	else
	{
		if(cv_splitscreen.value)
		{
			ST_drawOverlayNum(SCX(288), // was 50 Tails 10-31-99
							 SCY(10), // Ring score location Tails 10-31-99
							 plyr->health > 0 ? plyr->health-1 : 0, 
							 tallnum,NULL);

			if(plyr->health <= 1 && leveltime/5 & 1)
			{
				V_DrawScaledPatch (SCX(220),SCY(10), FG | V_NOSCALESTART,rrings); // Tails 03-14-2000
			}
			else if(plyr->health <= 1)
			{
				V_DrawScaledPatch (SCX(220),SCY(10), FG | V_NOSCALESTART,sbohealth); // Was a number I forget and 198 =) Tails 10-31-99
			}
			else
			{
				V_DrawScaledPatch (SCX(220),SCY(10), FG | V_NOSCALESTART,sbohealth); // Was a number I forget and 198 =) Tails 10-31-99
			}
		}
		else
		{
			if(gamemap >= sstage_start && gamemap <= sstage_end)
			{
				int ringscollected; // Total # everyone has collected
				int i;

				ringscollected = 0;

				for(i=0; i<MAXPLAYERS; i++)
				{
					if(playeringame[i] && players[i].mo && players[i].mo->health > 1)
					{
						ringscollected += players[i].mo->health - 1;
					}
				}

					ST_drawOverlayNum(SCX(112), // was 50 Tails 10-31-99
									SCY(42), // Ring score location Tails 10-31-99
									ringscollected,
									tallnum,NULL);
			}
			else
			{
				ST_drawOverlayNum(SCX(112), // was 50 Tails 10-31-99
								SCY(42), // Ring score location Tails 10-31-99
								plyr->health > 0 ? plyr->health-1 : 0, 
								tallnum,NULL);
			}

			if(plyr->health <= 1 && leveltime/5 & 1)
			{
				V_DrawScaledPatch (SCX(16),SCY(42), FG | V_NOSCALESTART,rrings); // Tails 03-14-2000
			}
			else
			{
				V_DrawScaledPatch (SCX(16),SCY(42), FG | V_NOSCALESTART,sbohealth); // Was a number I forget and 198 =) Tails 10-31-99
			}
		}

		if (cv_splitscreen.value)
		{
			ST_drawOverlayNum(SCX(128), // Score Tails 03-01-2000
							  SCY(10), // Location Tails 03-01-2000
							  plyr->score,
							  tallnum,NULL);
			V_DrawScaledPatch (SCX(16),SCY(10), FG | V_NOSCALESTART,sboscore); // Draw SCORE Tails 03-01-2000
		}
		else
		{
			ST_drawOverlayNum(SCX(128), // Score Tails 03-01-2000
							  SCY(10), // Location Tails 03-01-2000
							  plyr->score,
							  tallnum,NULL);
           
			V_DrawScaledPatch (SCX(16),SCY(10), FG | V_NOSCALESTART,sboscore); // Draw SCORE Tails 03-01-2000
		}

		if (cv_splitscreen.value)
		{
			if(plyr->seconds < 10)
			{
				ST_drawOverlayNum(SCX(204), // Tails 02-29-2000
								  SCY(10), // Draw the current single seconds time Tails 02-29-2000
								  0,
								  tallnum,NULL);
			}

			ST_drawOverlayNum(SCX(212), // Tails 02-29-2000
							  SCY(10), // Draw the current single seconds time Tails 02-29-2000
							  plyr->seconds,
							  tallnum,NULL);

			ST_drawOverlayNum(SCX(188), // Tails 02-29-2000
							  SCY(10), // Draw the current minutes time Tails 02-29-2000
							  plyr->minutes,
							  tallnum,NULL);

			V_DrawScaledPatch (SCX(188),SCY(10), FG | V_NOSCALESTART,sbocolon); // colon location Tails 02-29-2000

			V_DrawScaledPatch (SCX(136),SCY(10), FG | V_NOSCALESTART,sbotime); // TIME location Tails 02-29-2000
		}
		else
		{
			if(cv_timetic.value) // Special option to show tics instead of MM:SS
			{
				ST_drawOverlayNum(SCX(112), // Tails 02-29-2000
								  SCY(26), // Draw the current single seconds time Tails 02-29-2000
								  plyr->realtime,
								  tallnum,NULL);
			}
			else
			{
				if(plyr->seconds < 10)
				{
					ST_drawOverlayNum(SCX(104), // Tails 02-29-2000
									  SCY(26), // Draw the current single seconds time Tails 02-29-2000
											0,
									  tallnum,NULL);
				}

				ST_drawOverlayNum(SCX(112), // Tails 02-29-2000
								  SCY(26), // Draw the current single seconds time Tails 02-29-2000
								  plyr->seconds,
								  tallnum,NULL);

				ST_drawOverlayNum(SCX(88), // Tails 02-29-2000
								  SCY(26), // Draw the current single seconds time Tails 02-29-2000
								  plyr->minutes,
								  tallnum,NULL);

				V_DrawScaledPatch (SCX(88),SCY(26), FG | V_NOSCALESTART,sbocolon); // colon location Tails 02-29-2000
			}

			V_DrawScaledPatch (SCX(17),SCY(26), FG | V_NOSCALESTART,sbotime); // TIME location Tails 02-29-2000

		}
	}
/*
	// Show other player's names in network play
	if(!cv_splitscreen.value && netgame)
	{
		int q;
		int closestplayernum;
		closestplayernum = -1;

		for(q = 0; q < MAXPLAYERS; q++)
		{
			angle_t an;
			angle_t max_angle;
			angle_t min_angle;
			angle_t playerangle;

			if(!playeringame[q])
				continue;

			if(&players[q] == plyr) // Don't look at self
				continue;

			if(!players[q].mo)
				continue;

			if (players[q].mo->health <= 0)
				continue; // dead

			an = R_PointToAngle2 (plyr->mo->x,
									plyr->mo->y,
									players[q].mo->x,
									players[q].mo->y);

			// Check sideways angle
			if(!(plyr->mo->angle < an + ANG45/5
				&& plyr->mo->angle > an - ANG45/5)) // within 5 degrees left or right
				continue;

			// Check vertical angle
			an = R_PointToAngle2(0, plyr->mo->z + plyr->viewheight, P_AproxDistance(plyr->mo->x - players[q].mo->x, plyr->mo->y - players[q].mo->y), players[q].mo->z + players[q].viewheight);

			min_angle = max_angle = an <= ANG90 ? an + ANG90 : an - ANG270;

			max_angle += ANG45/5;
			min_angle -= ANG45/5;

			max_angle -= ANG45/9;
			min_angle -= ANG45/9;

			playerangle = plyr->aiming <= ANG90 ? plyr->aiming + ANG90 : plyr->aiming - ANG270;
			an += ANG90;

			if(playerangle < min_angle)
				continue;

			if(playerangle > max_angle)
				continue;

			if(P_CheckSight(plyr->mo, players[q].mo) == false) // Hiding behind something? Sonic ain't Superman!
				continue;

			if(closestplayernum == -1)
				closestplayernum = q;
			else
			{
				if(P_AproxDistance(plyr->mo->x - players[q].mo->x, plyr->mo->y - players[q].mo->y)
					< P_AproxDistance(plyr->mo->x - players[closestplayernum].mo->x, plyr->mo->y - players[closestplayernum].mo->y))
					closestplayernum = q;
			}
		}

		if(closestplayernum != -1)
		{
			// If you're not on the same CTF team, show the opponent's name in yellow.
			if(cv_gametype.value == GT_CTF && plyr->ctfteam != players[closestplayernum].ctfteam)
				V_DrawString((BASEVIDWIDTH/2) - (strlen(player_names[closestplayernum])*8)/2,80, V_TRANSLUCENT|V_WHITEMAP, player_names[closestplayernum]);
			else
				V_DrawString((BASEVIDWIDTH/2) - (strlen(player_names[closestplayernum])*8)/2,80, V_TRANSLUCENT, player_names[closestplayernum]);
		}
	}
*/

	if(cv_debug)
	{
		extern int numstarposts; // Graue 11-17-2003
		char smomx[33];
		char smomy[33];
		char smomz[33];
		char sspeed[33];
		char scontinues[33];
		char ssuperready[33];
		char semerald1[33];
		char semerald2[33];
		char semerald3[33];
		char semerald4[33];
		char semerald5[33];
		char semerald6[33];
		char semerald7[33];
		char sx[33];
		char sy[33];
		char sz[33];
		char sangle[33];
		char sunderwater[33];
		char smfjumped[33];
		char smfspinning[33];
		char smfstartdash[33];
		char sjumping[33];
		char sscoreadd[33];
		sprintf(smomx, "%i", plyr->rmomx >> FRACBITS);
		sprintf(smomy, "%i", plyr->rmomy >> FRACBITS);
		sprintf(smomz, "%i", plyr->mo->momz >> FRACBITS);
		sprintf(sspeed, "%i", plyr->speed);
		sprintf(scontinues, "%i", plyr->mo->floorz >> FRACBITS);
		sprintf(ssuperready, "%i", plyr->superready);
		sprintf(semerald1, "%i", plyr->charability);
		sprintf(semerald2, "%i", plyr->normalspeed);
		sprintf(semerald3, "%i", plyr->charspin);
		sprintf(semerald4, "%i", plyr->starttranscolor);
		sprintf(semerald5, "%i", plyr->endtranscolor);
		sprintf(semerald6, "%i", plyr->deadtimer);
		sprintf(semerald7, "%i", plyr->jumpfactor);
		sprintf(sx, "%i", plyr->mo->x >> FRACBITS);
		sprintf(sy, "%i", plyr->mo->y >> FRACBITS);
		sprintf(sz, "%i", plyr->mo->z >> FRACBITS);
		sprintf(sangle, "%i", plyr->mo->angle >> FRACBITS);
		sprintf(sunderwater, "%i", plyr->powers[pw_underwater]);
		sprintf(smfjumped, "%i", plyr->mfjumped);
		sprintf(smfspinning, "%i", plyr->mfspinning);
		sprintf(smfstartdash, "%i", plyr->mfstartdash);
		sprintf(sjumping, "%i", plyr->jumping);
		sprintf(sscoreadd, "%i", plyr->scoreadd);
		V_DrawString(248, 0, 0, "MOMX =");
		V_DrawString(296, 0, 0, smomx);
		V_DrawString(248, 8, 0, "MOMY =");
		V_DrawString(296, 8, 0, smomy);
		V_DrawString(248, 16, 0, "MOMZ =");
		V_DrawString(296, 16, 0, smomz);
		V_DrawString(240, 24, 0, "SPEED =");
		V_DrawString(296, 24, 0, sspeed);
		V_DrawString(208, 32, 0, "FLOORZ =");
		V_DrawString(288, 32, 0, scontinues);
		V_DrawString(200, 40, 0, "SUPERREADY =");
		V_DrawString(296, 40, 0, ssuperready);
		V_DrawString(216, 48, 0, "CABILITY =");
		V_DrawString(296, 48, 0, semerald1);
		V_DrawString(216, 56, 0, "CHARSPED =");
		V_DrawString(296, 56, 0, semerald2);
		V_DrawString(216, 64, 0, "CHARSPIN =");
		V_DrawString(296, 64, 0, semerald3);
		V_DrawString(216, 72, 0, "STRCOLOR =");
		V_DrawString(296, 72, 0, semerald4);
		V_DrawString(216, 80, 0, "ENDCOLOR =");
		V_DrawString(296, 80, 0, semerald5);
		V_DrawString(216, 88, 0, "DEDTIMER =");
		V_DrawString(296, 88, 0, semerald6);
		V_DrawString(216, 96, 0, "JUMPFACT =");
		V_DrawString(296, 96, 0, semerald7);
		V_DrawString(240, 104, 0, "X =");
		V_DrawString(264, 104, 0, sx);
		V_DrawString(240, 112, 0, "Y =");
		V_DrawString(264, 112, 0, sy);
		V_DrawString(240, 120, 0, "Z =");
		V_DrawString(264, 120, 0, sz);
		V_DrawString(216, 128, 0, "Angle =");
		V_DrawString(272, 128, 0, sangle);
		V_DrawString(192, 152, 0, "Underwater =");
		V_DrawString(288, 152, 0, sunderwater);
		V_DrawString(192, 160, 0, "MF_JUMPED =");
		V_DrawString(288, 160, 0, smfjumped);
		V_DrawString(192, 168, 0, "MF_SPINNING =");
		V_DrawString(296, 168, 0, smfspinning);
		V_DrawString(192, 176, 0, "MF_STARDASH =");
		V_DrawString(296, 176, 0, smfstartdash);
		V_DrawString(192, 184, 0, "Jumping =");
		V_DrawString(288, 184, 0, sjumping);
		V_DrawString(192, 192, 0, "Scoreadd =");
		V_DrawString(288, 192, 0, sscoreadd);
	}

	// Show number of laps completed in circuit mode Graue 11-19-2003
	if(cv_gametype.value == GT_CIRCUIT)
	{
		extern consvar_t cv_numlaps, cv_splitscreen;
		char lapsdone[25];

		// Print the number of laps wrong and make it stupidly write "Done" because Mystic said so Graue 12-24-2003
		if(cv_lapcounteridiocy.value)
		{
			if(players[displayplayer].laps < cv_numlaps.value)
				sprintf(lapsdone, "%d/%d", players[displayplayer].laps + 1, cv_numlaps.value);
			else
				strcpy(lapsdone, "DONE");
		}
		else
			sprintf(lapsdone, "%d/%d", players[displayplayer].laps, cv_numlaps.value);

		// Draw locations fixed by Graue 11-27-2003
		if(cv_splitscreen.value)
		{
			V_DrawString(304 - V_StringWidth(lapsdone), 84, 0, lapsdone);
			if(cv_lapcounteridiocy.value)
			{
				if(players[secondarydisplayplayer].laps < cv_numlaps.value)
					sprintf(lapsdone, "%d/%d", players[secondarydisplayplayer].laps + 1, cv_numlaps.value);
				else
					strcpy(lapsdone, "DONE");
			}
			else
				sprintf(lapsdone, "%d/%d", players[secondarydisplayplayer].laps, cv_numlaps.value);
			V_DrawString(304 - V_StringWidth(lapsdone), 184, 0, lapsdone);
		}
		else
			V_DrawString(304 - V_StringWidth(lapsdone), 10, 0, lapsdone);
	}

	if(cv_objectplace.value && plyr->mo && plyr->mo->target)
	{
		char x[8];
		char y[8];
		char z[8];
		char doomednum[8];
		char thingflags[8];
		sprintf(x, "%i", plyr->mo->x >> FRACBITS);
		sprintf(y, "%i", plyr->mo->y >> FRACBITS);
		sprintf(z, "%i", plyr->mo->z >> FRACBITS);
		sprintf(doomednum, "%i", plyr->mo->target->info->doomednum);
		sprintf(thingflags, "%i", cv_objflags.value);
		V_DrawString(16, 98, 0, "X =");
		V_DrawString(48, 98, 0, x);
		V_DrawString(16, 108, 0, "Y =");
		V_DrawString(48, 108, 0, y);
		V_DrawString(16, 118, 0, "Z =");
		V_DrawString(48, 118, 0, z);
		V_DrawString(16, 128, 0, "thing # =");
		V_DrawString(16+84, 128, 0, doomednum);
		V_DrawString(16, 138, 0, "flags =");
		V_DrawString(16+56, 138, 0, thingflags);
		V_DrawString(16, 148, 0, "snap =");
		V_DrawString(16+48, 148, 0, cv_snapto.string);
	}

	if(!(netgame || multiplayer) && eastermode && !modifiedgame)
	{
		char eggsfound[3];
		int found = 0;

		if(foundeggs & 1)
			found++;
		if(foundeggs & 2)
			found++;
		if(foundeggs & 4)
			found++;
		if(foundeggs & 8)
			found++;
		if(foundeggs & 16)
			found++;
		if(foundeggs & 32)
			found++;
		if(foundeggs & 64)
			found++;
		if(foundeggs & 128)
			found++;
		if(foundeggs & 256)
			found++;
		if(foundeggs & 512)
			found++;
		if(foundeggs & 1024)
			found++;
		if(foundeggs & 2048)
			found++;

		sprintf(eggsfound, "Eggs Found: %i/%i", found, NUMEGGS);
		V_DrawString(184, STRINGY(168), 0, eggsfound);
	}

	if(hu_showscores && cv_gametype.value == GT_COOP)
	{
		if(!netgame && !modifiedgame && plyr == &players[consoleplayer])
		{
			char emblemsfound[3];
			int found = 0;

			if(gottenemblems & 1)
				found++;
			if(gottenemblems & 2)
				found++;
			if(gottenemblems & 4)
				found++;
			if(gottenemblems & 8)
				found++;
			if(gottenemblems & 16)
				found++;
			if(gottenemblems & 32)
				found++;
			if(gottenemblems & 64)
				found++;
			if(gottenemblems & 128)
				found++;
			if(gottenemblems & 256)
				found++;
			if(gottenemblems & 512)
				found++;
			if(gottenemblems & 1024)
				found++;
			if(gottenemblems & 2048)
				found++;
			if(gottenemblems & 4096)
				found++;
			if(gottenemblems & 8192)
				found++;
			if(gottenemblems & 16384)
				found++;
			if(gottenemblems & 32768)
				found++;
			if(gottenemblems & 65536)
				found++;
			if(gottenemblems & 131072)
				found++;
			if(gottenemblems & 262144)
				found++;
			if(gottenemblems & 524288)
				found++;

			sprintf(emblemsfound, "- %i/%i", found, NUMEMBLEMS);
			V_DrawString(160, 96, 0, emblemsfound);
			V_DrawScaledPatch(SCX(128), (96-emblemicon->height/4)*vid.fdupy, FG|V_NOSCALESTART, emblemicon);
		}

		if(emeralds & EMERALD1)
			V_DrawScaledPatch(SCX(64), 128*vid.fdupy, FG|V_NOSCALESTART, emerald1);
		if(emeralds & EMERALD2)
			V_DrawScaledPatch(SCX(96), 128*vid.fdupy, FG|V_NOSCALESTART, emerald2);
		if(emeralds & EMERALD3)
			V_DrawScaledPatch(SCX(128), 128*vid.fdupy, FG|V_NOSCALESTART, emerald3);
		if(emeralds & EMERALD4)
			V_DrawScaledPatch(SCX(160), 128*vid.fdupy, FG|V_NOSCALESTART, emerald4);
		if(emeralds & EMERALD5)
			V_DrawScaledPatch(SCX(192), 128*vid.fdupy, FG|V_NOSCALESTART, emerald5);
		if(emeralds & EMERALD6)
			V_DrawScaledPatch(SCX(224), 128*vid.fdupy, FG|V_NOSCALESTART, emerald6);
		if(emeralds & EMERALD7)
			V_DrawScaledPatch(SCX(256), 128*vid.fdupy, FG|V_NOSCALESTART, emerald7);
	}

	// Countdown timer for Race Mode Tails 04-25-2001
	if(countdown)
	{
		char scountdown[33];
		sprintf(scountdown, "%i", countdown/TICRATE);
		V_DrawString(154, STRINGY(176), 0, scountdown);
	}
	// End Countdown timer for Race Mode Tails 04-25-2001

	// Start Tag display for Tag Mode Tails 05-09-2001
	if(cv_gametype.value == GT_TAG)
	{
		if(plyr->tagit)
		{
			char stagit[33];
			sprintf(stagit, "%i", plyr->tagit/TICRATE);

			if(cv_splitscreen.value)
				V_DrawString(120, STRINGY(168), 0, "YOU'RE IT!");
			else
				V_DrawString(120, STRINGY(176), 0, "YOU'RE IT!");

			V_DrawString(158-(V_StringWidth(stagit))/2, STRINGY(184), 0, stagit);
		}

		if(plyr->tagzone)
		{
			char stagzone[33];
			sprintf(stagzone, "%i", plyr->tagzone/TICRATE);
			if(cv_splitscreen.value)
			{
				V_DrawString(201, STRINGY(168), 0, "IN NO-TAG ZONE");
				V_DrawString(254-(V_StringWidth(stagzone))/2, STRINGY(184), 0, stagzone);
			}
			else
			{
				V_DrawString(104, STRINGY(160), 0, "IN NO-TAG ZONE");
				V_DrawString(158-(V_StringWidth(stagzone))/2, STRINGY(168), 0, stagzone);
			}
		}
		else if(plyr->taglag)
		{
			char staglag[33];
			sprintf(staglag, "%i", plyr->taglag/TICRATE);

			if(cv_splitscreen.value)
			{
				V_DrawString(30, STRINGY(168), 0, "NO-TAG LAG");
				V_DrawString(62-(V_StringWidth(staglag))/2, STRINGY(184), 0, staglag);
			}
			else
			{
				V_DrawString(120, STRINGY(160), 0, "NO-TAG LAG");
				V_DrawString(158-(V_StringWidth(staglag))/2, STRINGY(168), 0, staglag);
			}
		}
	}
	// End Tag display for Tag Mode Tails 05-09-2001

	// CTF HUD Stuff Tails 07-31-2001
	if(cv_gametype.value == GT_CTF)
	{
		int team;
		unsigned short whichflag;
		int i;
		team = whichflag = 0;

		for(i=0; i<MAXPLAYERS; i++)
		{
			if(players[i].gotflag & MF_REDFLAG)
			{
				team = players[i].ctfteam;
				whichflag = players[i].gotflag;
				break; // break, don't continue.
			}
		}

		if(plyr->ctfteam != team && team > 0 && ((plyr->ctfteam == 1 && whichflag & MF_REDFLAG) || (plyr->ctfteam == 2 && whichflag & MF_BLUEFLAG)))
		{
			if(cv_splitscreen.value)
				V_DrawString(128, STRINGY(120), V_WHITEMAP|V_TRANSLUCENT, "OTHER TEAM HAS YOUR FLAG!");
			else
				V_DrawString(128, STRINGY(160), V_WHITEMAP, "OTHER TEAM HAS YOUR FLAG!");
		}
		else if (plyr->ctfteam == team && team > 0)
		{
			if((plyr->ctfteam == 1 && whichflag & MF_REDFLAG) || (plyr->ctfteam == 2 && whichflag & MF_BLUEFLAG))
			{
				if(cv_splitscreen.value)
					V_DrawString(128, STRINGY(120), V_TRANSLUCENT, "YOUR TEAM HAS YOUR FLAG!");
				else
					V_DrawString(128, STRINGY(160), 0, "YOUR TEAM HAS YOUR FLAG!");
			}
			else
			{
				if(cv_splitscreen.value)
					V_DrawString(128, STRINGY(136), V_TRANSLUCENT, "YOUR TEAM HAS ENEMY FLAG!");
				else
					V_DrawString(128, STRINGY(168), 0, "YOUR TEAM HAS ENEMY FLAG!");
			}
		}

		team = whichflag = 0;

		for(i=0; i<MAXPLAYERS; i++)
		{
			if(players[i].gotflag & MF_BLUEFLAG)
			{
				team = players[i].ctfteam;
				whichflag = players[i].gotflag;
				break; // break, don't continue.
			}
		}
		if(plyr->ctfteam != team && team > 0 && ((plyr->ctfteam == 1 && whichflag & MF_REDFLAG) || (plyr->ctfteam == 2 && whichflag & MF_BLUEFLAG)))
		{
			if(cv_splitscreen.value)
				V_DrawString(128, STRINGY(120), V_WHITEMAP|V_TRANSLUCENT, "OTHER TEAM HAS YOUR FLAG!");
			else
				V_DrawString(128, STRINGY(160), V_WHITEMAP, "OTHER TEAM HAS YOUR FLAG!");
		}
		else if (plyr->ctfteam == team && team > 0)
		{
			if((plyr->ctfteam == 1 && whichflag & MF_REDFLAG) || (plyr->ctfteam == 2 && whichflag & MF_BLUEFLAG))
			{
				if(cv_splitscreen.value)
					V_DrawString(128, STRINGY(120), V_TRANSLUCENT, "YOUR TEAM HAS YOUR FLAG!");
				else
					V_DrawString(128, STRINGY(160), 0, "YOUR TEAM HAS YOUR FLAG!");
			}
			else
			{
				if(cv_splitscreen.value)
					V_DrawString(128, STRINGY(136), V_TRANSLUCENT, "YOUR TEAM HAS ENEMY FLAG!");
				else
					V_DrawString(128, STRINGY(168), 0, "YOUR TEAM HAS ENEMY FLAG!");
			}
		}

		if(plyr->gotflag & MF_REDFLAG)
		{
			if(cv_splitscreen.value)
				V_DrawString(128, STRINGY(152), V_TRANSLUCENT, "YOU HAVE THE RED FLAG");
			else
				V_DrawString(128, STRINGY(176), 0, "YOU HAVE THE RED FLAG");
		}
		else if (plyr->gotflag & MF_BLUEFLAG)
		{
			if(cv_splitscreen.value)
				V_DrawString(128, STRINGY(168), V_TRANSLUCENT, "YOU HAVE THE BLUE FLAG");
			else
				V_DrawString(128, STRINGY(184), 0, "YOU HAVE THE BLUE FLAG");
		}
		if(plyr->ctfteam == 1)
		{
			if(cv_splitscreen.value)
				V_DrawString(128, STRINGY(184), V_TRANSLUCENT, "YOU'RE ON THE RED TEAM");
			else
				V_DrawString(128, STRINGY(192), 0, "YOU'RE ON THE RED TEAM");
		}
		else if(plyr->ctfteam == 2)
		{
			if(cv_splitscreen.value)
				V_DrawString(128, STRINGY(184), V_TRANSLUCENT, "YOU'RE ON THE BLUE TEAM");
			else
				V_DrawString(128, STRINGY(192), 0, "YOU'RE ON THE BLUE TEAM");
		}
	}

	// Special Stage HUD Tails 08-11-2001
	if(gamemap >= sstage_start && gamemap <= sstage_end)
	{

		if(cv_splitscreen.value)
			ST_drawOverlayNum(SCX(288),SCY(40), totalrings, tallnum, NULL);
		else
			ST_drawOverlayNum(SCX(112),SCY(56), totalrings, tallnum, NULL);

		if(leveltime < 5*TICRATE)
		{
			V_DrawScaledPatch (SCX(100),SCY(90), FG | V_NOSCALESTART,getall); // Tails 08-11-2001
			ST_drawOverlayNum(SCX(160), SCY(93), totalrings, tallnum, NULL);
		}

		if(sstimer)
		{
			V_DrawString(124,STRINGY(160), 0, "TIME LEFT");
			ST_drawOverlayNum(SCX(168), // Tails 02-29-2000
							 SCY(176), // Draw the current single seconds time Tails 02-29-2000
							 sstimer/TICRATE,
							 tallnum,NULL);
		}
		else
			V_DrawScaledPatch (SCX(125),SCY(90), FG | V_NOSCALESTART,timeup); // Tails 08-11-2001
	}

	if(plyr->nightstime > 0)
	{
		int numbersize;

		if(plyr->nightstime < 10)
			numbersize = SCX(16)/2;
		else if(plyr->nightstime < 100)
			numbersize = SCX(32)/2;
		else
			numbersize = SCX(48)/2;

		if(plyr->nightstime < 10)
			ST_drawNightsOverlayNum(SCX(160) + numbersize, SCY(32), plyr->nightstime, nightsnum, 6); // Red
		else
			ST_drawNightsOverlayNum(SCX(160) + numbersize, SCY(32), plyr->nightstime, nightsnum, MAXSKINCOLORS-1); // Yellow
	}

	// Emerald Hunt Indicators Tails 12-20-2001
	if(hunt1 && hunt1->health)
	{
		fixed_t dist;
		dist = P_AproxDistance(P_AproxDistance(plyr->mo->x - hunt1->x, plyr->mo->y - hunt1->y), plyr->mo->z - hunt1->z);

		if(dist < 128<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(132), SCY(168), FG | V_NOSCALESTART, homing6);
			if(leveltime % 5 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 512<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(132), SCY(168), FG | V_NOSCALESTART, homing5);
			if(leveltime % 10 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 1024<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(132), SCY(168), FG | V_NOSCALESTART, homing4);
			if(leveltime % 20 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 2048<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(132), SCY(168), FG | V_NOSCALESTART, homing3);
			if(leveltime % 30 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 3072<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(132), SCY(168), FG | V_NOSCALESTART, homing2);
			if(leveltime % 35 ==1)
				S_StartSound(0, sfx_shotgn);
		}
		else
		{
			V_DrawScaledPatch(SCX(132), SCY(168), FG | V_NOSCALESTART, homing1);
		}
	}
	if(hunt2 && hunt2->health)
	{
		fixed_t dist;
		dist = P_AproxDistance(P_AproxDistance(plyr->mo->x - hunt2->x, plyr->mo->y - hunt2->y), plyr->mo->z - hunt2->z);
		if(dist < 128<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(152), SCY(168), FG | V_NOSCALESTART, homing6);
			if(leveltime % 5 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 512<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(152), SCY(168), FG | V_NOSCALESTART, homing5);
			if(leveltime % 10 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 1024<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(152), SCY(168), FG | V_NOSCALESTART, homing4);
			if(leveltime % 20 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 2048<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(152), SCY(168), FG | V_NOSCALESTART, homing3);
			if(leveltime % 30 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 3072<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(152), SCY(168), FG | V_NOSCALESTART, homing2);
			if(leveltime % 35 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else
		{
			V_DrawScaledPatch(SCX(152), SCY(168), FG | V_NOSCALESTART, homing1);
		}
	}
	if(hunt3 && hunt3->health)
	{
		fixed_t dist;
		dist = P_AproxDistance(P_AproxDistance(plyr->mo->x - hunt3->x, plyr->mo->y - hunt3->y), plyr->mo->z - hunt3->z);
		if(dist < 128<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(172), SCY(168), FG | V_NOSCALESTART, homing6);
			if(leveltime % 5 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 512<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(172), SCY(168), FG | V_NOSCALESTART, homing5);
			if(leveltime % 10 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 1024<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(172), SCY(168), FG | V_NOSCALESTART, homing4);
			if(leveltime % 20 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 2048<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(172), SCY(168), FG | V_NOSCALESTART, homing3);
			if(leveltime % 30 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else if(dist < 3072<<FRACBITS)
		{
			V_DrawScaledPatch(SCX(172), SCY(168), FG | V_NOSCALESTART, homing2);
			if(leveltime % 35 == 1)
				S_StartSound(0, sfx_shotgn);
		}
		else
		{
			V_DrawScaledPatch(SCX(172), SCY(168), FG | V_NOSCALESTART, homing1);
		}
	}

	if(plyr->powers[pw_railring] > 5*TICRATE || (plyr->powers[pw_railring] && leveltime & 1))
	{
		char railringpower[4];
		V_DrawScaledPatch(SCX(8), SCY(56-splity), FG|V_NOSCALESTART, railring);
		sprintf(railringpower, "%i", plyr->powers[pw_railring]/TICRATE);
		V_DrawString(32, STRINGY(56-splity+4), V_TRANSLUCENT, railringpower);
	}

	if(plyr->powers[pw_homingring] > 5*TICRATE || (plyr->powers[pw_homingring] && leveltime & 1))
	{
		char homingringpower[4];
		V_DrawScaledPatch(SCX(8), SCY(88-splity), FG|V_NOSCALESTART, homingring);
		sprintf(homingringpower, "%i", plyr->powers[pw_homingring]/TICRATE);
		V_DrawString(32, STRINGY(88-splity+4), V_TRANSLUCENT, homingringpower);
	}

	if(plyr->powers[pw_shieldring] > 5*TICRATE || (plyr->powers[pw_shieldring] && leveltime & 1))
	{
		char infinityringpower[4];
		V_DrawScaledPatch(SCX(8), SCY(120-splity), FG|V_NOSCALESTART, infinityring);
		sprintf(infinityringpower, "%i", plyr->powers[pw_shieldring]/TICRATE);
		V_DrawString(32, STRINGY(120-splity+4), V_TRANSLUCENT, infinityringpower);
	}

	if(plyr->powers[pw_explosionring] > 5*TICRATE || (plyr->powers[pw_explosionring] && leveltime & 1))
	{
		char explosionringpower[4];
		V_DrawScaledPatch(SCX(8), SCY(152-splity), FG|V_NOSCALESTART, explosionring);
		sprintf(explosionringpower, "%i", plyr->powers[pw_explosionring]/TICRATE);
		V_DrawString(32, STRINGY(152-splity+4), V_TRANSLUCENT, explosionringpower);
	}

	if(plyr->powers[pw_automaticring] > 5*TICRATE || (plyr->powers[pw_automaticring] && leveltime & 1))
	{
		char automaticringpower[4];
		V_DrawScaledPatch(SCX(8), SCY(184-splity), FG|V_NOSCALESTART, autoring);
		sprintf(automaticringpower, "%i", plyr->powers[pw_automaticring]/TICRATE);
		V_DrawString(32, STRINGY(184-splity+4), V_TRANSLUCENT, automaticringpower);
	}

	if(plyr->lightdashallowed)
	{
		V_DrawFill(224, 6, 72, 12, 119);
		V_DrawCenteredString(292-32, 8, 0, "ACTIVATE");
		V_DrawCenteredString(292-32, 20, 0, "LIGHT DASH");
	}

	// This is where we draw all the fun cheese if you have the chasecam off!
	if((!cv_chasecam.value && plyr == &players[consoleplayer])
		|| (!cv_chasecam2.value && plyr == &players[secondarydisplayplayer] && cv_splitscreen.value))
	{
		player_t* player = plyr;
		fixed_t yheight;

		yheight = SCY(120);

		if(player->powers[pw_blueshield])
			V_DrawScaledPatch(SCX(304), SCY(32), FG|V_NOSCALESTART, blueshield);
		else if(player->powers[pw_redshield])
			V_DrawScaledPatch(SCX(304), SCY(32), FG|V_NOSCALESTART, redshield);
		else if(player->powers[pw_greenshield])
			V_DrawScaledPatch(SCX(304), SCY(32), FG|V_NOSCALESTART, greenshield);
		else if(player->powers[pw_blackshield])
			V_DrawScaledPatch(SCX(304), SCY(32), FG|V_NOSCALESTART, blackshield);
		else if(player->powers[pw_yellowshield])
			V_DrawScaledPatch(SCX(304), SCY(32), FG|V_NOSCALESTART, yellowshield);

		if(player->powers[pw_invulnerability] > 3*TICRATE || (player->powers[pw_invulnerability] && leveltime & 1))
			V_DrawScaledPatch(SCX(304), SCY(56), FG|V_NOSCALESTART, invincibility);

		if(player->powers[pw_sneakers] > 3*TICRATE || (player->powers[pw_sneakers] && leveltime & 1))
			V_DrawScaledPatch(SCX(304), SCY(80), FG|V_NOSCALESTART, sneakers);

		// Display the countdown drown numbers!
		if(!player->nightsmode)
		{
			if ((player->powers[pw_underwater] <= 11*TICRATE + 1
				&& player->powers[pw_underwater] >= 10*TICRATE + 1)
				||
				(player->powers[pw_spacetime] <= 11*TICRATE + 1
				&& player->powers[pw_spacetime] >= 10*TICRATE + 1))
			{      
				V_DrawTranslucentPatch(SCX((BASEVIDWIDTH/2)-(count5->width/2)), yheight, FG|V_NOSCALESTART, count5); 
			}
			else if ((player->powers[pw_underwater] <= 9*TICRATE + 1
				&& player->powers[pw_underwater] >= 8*TICRATE + 1)
				||
				(player->powers[pw_spacetime] <= 9*TICRATE + 1
				&& player->powers[pw_spacetime] >= 8*TICRATE + 1))
			{    
				V_DrawTranslucentPatch(SCX((BASEVIDWIDTH/2)-(count4->width/2)), yheight, FG|V_NOSCALESTART, count4); 
			}
			else if ((player->powers[pw_underwater] <= 7*TICRATE + 1
				&& player->powers[pw_underwater] >= 6*TICRATE + 1)
				||
				(player->powers[pw_spacetime] <= 7*TICRATE + 1
				&& player->powers[pw_spacetime] >= 6*TICRATE + 1))
			{
				V_DrawTranslucentPatch(SCX((BASEVIDWIDTH/2)-(count3->width/2)), yheight, FG|V_NOSCALESTART, count3); 
			}
			else if ((player->powers[pw_underwater] <= 5*TICRATE + 1
				&& player->powers[pw_underwater] >= 4*TICRATE + 1)
				||
				(player->powers[pw_spacetime] <= 5*TICRATE + 1
				&& player->powers[pw_spacetime] >= 4*TICRATE + 1))
			{
				V_DrawTranslucentPatch(SCX((BASEVIDWIDTH/2)-(count2->width/2)), yheight, FG|V_NOSCALESTART, count2); 
			}
			else if ((player->powers[pw_underwater] <= 3*TICRATE + 1
				&& player->powers[pw_underwater] >= 2*TICRATE + 1)
				||
				(player->powers[pw_spacetime] <= 3*TICRATE + 1
				&& player->powers[pw_spacetime] >= 2*TICRATE + 1))
			{
				V_DrawTranslucentPatch(SCX((BASEVIDWIDTH/2)-(count1->width/2)), yheight, FG|V_NOSCALESTART, count1); 
			}
			else if ((player->powers[pw_underwater] <= 1*TICRATE + 1
				&& player->powers[pw_underwater] > 1)
				||
				(player->powers[pw_spacetime] <= 1*TICRATE + 1
				&& player->powers[pw_spacetime] > 1))
			{
				V_DrawTranslucentPatch(SCX((BASEVIDWIDTH/2)-(count0->width/2)), yheight, FG|V_NOSCALESTART, count0); 
			}
		}
	}

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
		V_DrawCenteredString(BASEVIDWIDTH/2, 176, 0, va("PAR : %d", par));

	if(mariomode && plyr->exiting)
	{
		thinker_t*  th;
		mobj_t*     mo2;
		boolean foundtoad = false;

		// scan the remaining thinkers
		// to find toad
		for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo2 = (mobj_t *)th;
			if(mo2->type == MT_TOAD)
			{
				foundtoad = true;
				break;
			}
		}

		if(foundtoad)
		{
			V_DrawCenteredString(160, 32+16, 0, "Thank you!");
			V_DrawCenteredString(160, 44+16, 0, "But our earless leader is in");
			V_DrawCenteredString(160, 56+16, 0, "another castle!");
		}
	}

	// draw level title Tails
	if(leveltime < 111)
	{
#define ZONE "ZONE"
		char* lvlttl = mapheaderinfo[gamemap-1].lvlttl;
		int lvlttlxpos;
		int ttlnumxpos;
		int zonexpos;
		int actnum = mapheaderinfo[gamemap-1].actnum;
		boolean nonumber = false;

		if(*lvlttl == '\0' || (cv_gametype.value ==
			GT_CIRCUIT && circintrodone))
			goto jimmy; // No title. Oh well!

		if(actnum > 0)
			ttlnum = W_CachePatchName (actnum > 9 ? va("TTL%d",actnum) : va("TTL0%d",actnum), PU_CACHE); // Tails 11-01-2000
		else
		{
			nonumber = true;
			ttlnum = W_CachePatchName("TTL01", PU_CACHE);
		}

		if(nonumber)
			lvlttlxpos = ((BASEVIDWIDTH/2) - (V_LevelNameWidth(lvlttl)/2));
		else
			lvlttlxpos = ((BASEVIDWIDTH/2) - (V_LevelNameWidth(lvlttl)/2)) - ttlnum->width;

		ttlnumxpos = lvlttlxpos + V_LevelNameWidth(lvlttl);
		zonexpos = ttlnumxpos - V_LevelNameWidth(ZONE);
	
		if(leveltime == 2)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(200), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 0, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 200, 0, ZONE);
		}
		else if (leveltime == 3)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(188), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 12, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 188, 0, ZONE);
		}
		else if (leveltime == 4)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(0),SCY(176), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 24, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 176, 0, ZONE);
		}
		else if (leveltime == 5)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(164), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 36, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 164, 0, ZONE);
		}
		else if (leveltime == 6)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(152), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 48, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 152, 0, ZONE);
		}
		else if (leveltime == 7)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(140), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 60, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 140, 0, ZONE);
		}
		else if (leveltime == 8)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(128), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 72, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 128, 0, ZONE);
		}
		else if (leveltime >= 9 && leveltime < 106)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(104), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 80, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 104, 0, ZONE);
		}
		else if (leveltime == 106)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(80), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 104, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 80, 0, ZONE);
		}
		else if (leveltime == 107)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(56), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 128, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 56, 0, ZONE);
		}
		else if (leveltime == 108)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(32), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 152, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 32, 0, ZONE);
		}
		else if (leveltime == 109)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(8), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 176, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 8, 0, ZONE);
		}
		else if (leveltime == 110)
		{
			if(!nonumber)
				V_DrawScaledPatch (SCX(ttlnumxpos),SCY(0), FG | V_NOSCALESTART,ttlnum); // Tails 11-01-2000
			V_DrawLevelTitle(lvlttlxpos, 200, 0, lvlttl);

			if(mapheaderinfo[gamemap-1].nozone == false)
				V_DrawLevelTitle(zonexpos, 0, 0, ZONE);
		}
	}

jimmy:

	// Graue 12-24-2003
	if(!circintrodone && leveltime > 1)
	{
		// If gametype is not GT_CIRCUIT, then circintrodone should be true anyway
		switch(leveltime/CIRCINTROTIME)
		{
			case 0: // Print a red 3
				ST_drawNightsOverlayNum(SCX(160), SCY(40), 3, nightsnum, 6);
				break;
			case 1: // Print a yellow 2
				ST_drawNightsOverlayNum(SCX(160), SCY(40), 2, nightsnum, 15);
				break;
			case 2: // Print a green 1
				ST_drawNightsOverlayNum(SCX(160), SCY(40), 1, nightsnum, 12);
				break;
		}
	}

	if(!(netgame || multiplayer) && plyr->deadtimer > 0 && (plyr->deadtimer < 45*TICRATE) && plyr->lives <= 0) // Tails 11-21-2000
	{
		if(plyr->continues > 0) // Player has continues, so let's use them!
		{
			char stimeleft[33];
			patch_t* contsonic;
			// Do continue screen here.
			// Initialize music
			if(plyr->deadtimer < 45*TICRATE && plyr->deadtimer > 45*TICRATE - 2) // For some reason the code doesn't like a simple ==...
			{
				tic_t                       wipestart;
				tic_t                       tics;
				tic_t                       nowtime;
				int                         y;
				boolean                     done;
				// Force a screen wipe

				// First, read the current screen
				if(rendermode == render_soft)
				{
					wipe_StartScreen(0, 0, vid.width, vid.height);

					// Then, draw what the new screen will look like.
					V_DrawFill(0,0,vid.width,vid.height,0);

					contsonic = W_CachePatchName ("CONT1", PU_CACHE); // Tails 11-01-2000
					V_DrawScaledPatch((320-contsonic->width)/2, 64, FG, contsonic);
					V_DrawString(128,128,0, "CONTINUE?");
					sprintf(stimeleft, "%i", (plyr->deadtimer - 34*TICRATE)/TICRATE);
					V_DrawString(plyr->deadtimer >= 44*TICRATE ? 152 : 160,144,0, stimeleft);

					// Now, read the end screen we want to fade to.
					wipe_EndScreen(0, 0, vid.width, vid.height);

					S_ChangeMusic(mus_contsc, false);

					// Do the wipe-io!
					wipestart = I_GetTime () - 1;
					y=wipestart+2*TICRATE; // init a timeout
					do
					{
						do
						{
							nowtime = I_GetTime ();
							tics = nowtime - wipestart;
						} while (!tics);
						wipestart = nowtime;
						done = wipe_ScreenWipe (wipe_ColorXForm
												, 0, 0, vid.width, vid.height, tics);
						I_OsPolling ();
						I_UpdateNoBlit ();
						M_Drawer ();            // menu is drawn even on top of wipes
						I_FinishUpdate ();      // page flip or blit buffer
					} while (!done && I_GetTime()<(unsigned)y);
				}
			}
			V_DrawFill(0,0,vid.width,vid.height,0);
			V_DrawString(128,128,0, "CONTINUE?");
			// Draw a Sonic!
			contsonic = W_CachePatchName ("CONT1", PU_CACHE); // Tails 11-01-2000
			V_DrawScaledPatch((320-contsonic->width)/2, 64, FG, contsonic);
			sprintf(stimeleft, "%i", (plyr->deadtimer - 34*TICRATE)/TICRATE);
			V_DrawString(plyr->deadtimer >= 44*TICRATE ? 152 : 160,144,0, stimeleft);
			if(plyr->deadtimer < 35*TICRATE)
				COM_BufAddText("exitgame\n");
			if(plyr->cmd.buttons & BT_JUMP
				|| plyr->cmd.buttons & BT_USE)
			{
				plyr->continues--;

				// Reset # of lives
				switch(gameskill)
				{
					case sk_insane:
						plyr->lives = 1;
						break;
					case sk_nightmare:
					case sk_hard:
					case sk_medium:
						plyr->lives = 3;
						break;
					case sk_easy:
						plyr->lives = 5;
						break;
					case sk_baby:
						plyr->lives = 9;
						break;
					default: // Oops!?
						CONS_Printf("ERROR: PLAYER SKILL UNDETERMINED!");
						break;
				}
				// Clear any starpost data
				plyr->starpostangle = 0;
				plyr->starpostbit = 0;
				plyr->starpostnum = 0;
				plyr->starposttime = 0;
				plyr->starpostx = 0;
				plyr->starposty = 0;
				plyr->starpostz = 0;
				contsonic = W_CachePatchName ("CONT2", PU_CACHE); // Tails 11-01-2000
				V_DrawScaledPatch((320-contsonic->width)/2, 64, FG, contsonic);
			}
		}
		else // Just go to the title screen
			COM_BufAddText("exitgame\n");
	}
}
