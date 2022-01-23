// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: wi_stuff.c,v 1.14 2001/06/30 15:06:01 bpereira Exp $
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
// $Log: wi_stuff.c,v $
// Revision 1.14  2001/06/30 15:06:01  bpereira
// fixed wronf next level name in intermission
//
// Revision 1.13  2001/05/16 21:21:15  bpereira
// no message
//
// Revision 1.12  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.11  2001/03/03 06:17:34  bpereira
// no message
//
// Revision 1.10  2001/02/24 13:35:21  bpereira
// no message
//
// Revision 1.9  2001/02/10 12:27:14  bpereira
// no message
//
// Revision 1.8  2001/01/27 11:02:36  bpereira
// no message
//
// Revision 1.7  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.6  2000/11/02 17:50:10  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.5  2000/09/21 16:45:09  bpereira
// no message
//
// Revision 1.4  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.3  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Intermission screens.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "wi_stuff.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "m_random.h"
#include "r_local.h"
#include "s_sound.h"
#include "st_stuff.h"
#include "i_video.h"
#include "v_video.h"
#include "z_zone.h"
#include "console.h"
#include "p_info.h"
#include "f_wipe.h"
#include "i_system.h"
#include "m_menu.h"

void I_PlayCD(); // proto Tails

extern consvar_t cv_inttime; // Tails 04-19-2002
char* GetPlayerFacePic(int skinnum); // Tails 07-12-2002

void P_AddPlayerScore(player_t* player, int amount);

//
// GENERAL DATA
//

//
// Locally used stuff.
//
#define FB 0

// States for the intermission

typedef enum
{
    NoState = -1,
    StatCount,
    ShowNextLoc

} stateenum_t;

// used to accelerate or skip a stage
static int              acceleratestage;

// wbs->pnum
static int              me;

 // specifies current state
static stateenum_t      state;

// contains information passed into intermission
static wbstartstruct_t* wbs;

static wbplayerstruct_t* plrs;  // wbs->plyr[]

// used for general timing
static int              cnt;

// used for timing of background animation
static int              bcnt;

static int              cnt_fscore[MAXPLAYERS]; // Tails 03-14-2000
static int              cnt_score[MAXPLAYERS]; // Score Tails 03-09-2000
static int              cnt_timebonus[MAXPLAYERS]; // Time Bonus Tails 03-10-2000
static int              cnt_ringbonus[MAXPLAYERS]; // Ring Bonus Tails 03-10-2000
static int              cnt_pause;

static boolean gottimebonus;

//
//      GRAPHICS
//

// background (map of levels).
//static patch_t*       bg;
static char             bgname[9];

// %, : graphics
static patch_t*         colon;

// 0-9 graphic
static patch_t*         num[10];

// "secret"
static patch_t*         sp_secret;

 // "Kills", "Scrt", "Items", "Frags"
static patch_t*         cscore; // Tails 08-11-2001
static patch_t*         cemerald; // Tails 08-12-2001
static patch_t*         gotemall; // Tails 08-12-2001
static patch_t*         nowsuper; // Tails 08-12-2001
static patch_t*         chaos1; // Tails 08-12-2001
static patch_t*         chaos2; // Tails 08-12-2001
static patch_t*         chaos3; // Tails 08-12-2001
static patch_t*         chaos4; // Tails 08-12-2001
static patch_t*         chaos5; // Tails 08-12-2001
static patch_t*         chaos6; // Tails 08-12-2001
static patch_t*         chaos7; // Tails 08-12-2001
static patch_t*         kills;
static patch_t*         secret;
static patch_t*         items;
static patch_t*         frags;
static patch_t*         fscore; // Tails 03-14-2000
static patch_t*			haspassed; // Tails
static patch_t*			soncpass; // SONIC letters Tails 11-15-2000
static patch_t*			tailpass; // TAILS letters Tails 11-15-2000
static patch_t*			knuxpass; // KNUCKLES letters Tails 11-15-2000
static patch_t*			youpass; // YOU letters Tails 11-15-2000

// Time sucks.
static patch_t*         time; // Tails

int blinker; // Tails 08-12-2001

//
// CODE
//

// slam background
// UNUSED static unsigned char *background=0;

static void WI_slamBackground(void)
{
    if (rendermode==render_soft) 
    {
		// Fudge the old method. I'm gonna do it my way!
		// Tails 04-02-2003
		V_DrawScaledPatch(0, 0, 0, W_CachePatchName(bgname, PU_CACHE));
        //memcpy(screens[0], screens[1], vid.width * vid.height);
        //V_MarkRect (0, 0, vid.width, vid.height);
    }
    else
        V_DrawScaledPatch(0, 0, 1+V_NOSCALESTART, W_CachePatchName(bgname, PU_CACHE));
}

// The ticker is used to detect keys
//  because of timing issues in netgames.
boolean WI_Responder(event_t* ev)
{
    return false;
}

//
// Draws a number.
// If digits > 0, then use that many digits minimum,
//  otherwise only use as many as necessary.
// Returns new x position.
//

static int WI_drawNum ( int           x,
                        int           y,
                        int           n,
                        int           digits )
{

    int         fontwidth = (num[0]->width);
    int         neg;
    int         temp;

    if (digits < 0)
    {
        if (!n)
        {
            // make variable-length zeros 1 digit long
            digits = 1;
        }
        else
        {
            // figure out # of digits in #
            digits = 0;
            temp = n;

            while (temp)
            {
                temp /= 10;
                digits++;
            }
        }
    }

    neg = n < 0;
    if (neg)
        n = -n;

    // if non-number, do not draw it
    if (n == 1994)
        return 0;

    // draw the new number
    while (digits--)
    {
        x -= fontwidth;
        V_DrawScaledPatch(x, y, FB, num[ n % 10 ]);
        n /= 10;
    }

    return x;

}

static void WI_drawPercent( int           x,
                            int           y,
                            int           p )
{
    if (p < 0)
        return;

    WI_drawNum(x, y, p, -1);
}

static void WI_unloadData(void);

void G_SaveGameData();

static void WI_End(void)
{
	if(!(multiplayer || netgame))
		G_SaveGameData(); // Tails 12-08-2002

	if(gottimebonus)
		grade |= 256;

    WI_unloadData();
}

// used for write introduce next level
static void WI_initNoState(void)
{
    state = NoState;
    acceleratestage = 0;
	cnt = 10;
}

void F_StartCredits(void);
void F_StartGameEvaluation(void);
void D_StartTitle(void);
void F_StartCustomCutscene(int cutscenenum);

static void WI_updateNoState(void) {

    if (--cnt==0)
    {
        WI_End();
		if(mapheaderinfo[gamemap-1].nextlevel == 1102)
		{
			if(mapheaderinfo[gamemap-1].cutscenenum != 0)
			{
				// Start custom cutscene here
				F_StartCustomCutscene(mapheaderinfo[gamemap-1].cutscenenum-1);
			}
			else
			{
				if(!modifiedgame && gameskill == sk_nightmare) // Very Hard cleared!
					veryhardcleared = true;

				if(cv_gametype.value == GT_COOP)
					F_StartCredits();
				else
					D_StartTitle();
			}
		}
		else if(mapheaderinfo[gamemap-1].nextlevel == 1101) // Cut to the chase, captain.
		{
			if(mapheaderinfo[gamemap-1].cutscenenum != 0)
			{
				// Start custom cutscene here
				F_StartCustomCutscene(mapheaderinfo[gamemap-1].cutscenenum-1);
			}
			else
			{
				if(cv_gametype.value == GT_COOP)
					F_StartGameEvaluation();
				else
					D_StartTitle();
			}
		}
		else if(mapheaderinfo[gamemap-1].nextlevel == 1100) // Cut to the chase, captain.
		{
			if(mapheaderinfo[gamemap-1].cutscenenum != 0)
			{
				// Start custom cutscene here
				F_StartCustomCutscene(mapheaderinfo[gamemap-1].cutscenenum-1);
			}
			else
				D_StartTitle();
		}
		else
			G_AfterIntermission();
    }

}

static boolean          snl_pointeron = false;

static void WI_drawNoState(void)
{
    snl_pointeron = true;
}


static int              dm_scores[MAXPLAYERS];

// Deathmatch intermission Tails 02-20-2002
static void WI_initDeathmatchStats(void)
{

    int         i;

    state = StatCount;
    acceleratestage = 0;

    cnt_pause = TICRATE*cv_inttime.value;

    for (i=0 ; i<MAXPLAYERS ; i++)
	{
         if (playeringame[i])
             dm_scores[i] = plrs[i].score;
	}
}

// Circuit intermission Tails 12-17-2003
static void WI_initCircuitStats(void)
{
    int         i;

    state = StatCount;
    acceleratestage = 0;

    cnt_pause = TICRATE*cv_inttime.value;

    for (i=0 ; i<MAXPLAYERS ; i++)
	{
         if (playeringame[i])
             dm_scores[i] = players[i].realtime;
	}
}

static void WI_updateDeathmatchStats(void)
{
    if( paused )
        return;
    if (cnt_pause>0) cnt_pause--;
    if (cnt_pause==0)
    {
        WI_initNoState();
    }
}

#define RANKINGY 60

void WI_drawIntRanking(char *title,int x,int y,fragsort_t *fragtable
                   , int scorelines, boolean large, int white);
void WI_drawCircuitRanking(char *title,int x,int y,fragsort_t *fragtable
                   , int scorelines, boolean large, int white); // Graue 12-24-2003

static void WI_drawDeathmatchStats(void)
{
    int          i;
/*    int          scorelines;
    int          whiteplayer;
    fragsort_t   fragtab[MAXPLAYERS];*/
    char         *timeleft;
	patch_t*     p;

	signed int topscore[15]; /* maximum value found -- OUTPUT VALUE */
	signed int etopscore[15]; /* element it was found in -- OUTPUT VALUE */

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
	{
		int scorebonus;
		V_DrawFill(0,0,vid.width, vid.height, 0);

		V_DrawCenteredString(BASEVIDWIDTH/2, 96, 0, va("Par: %d", par));
		V_DrawCenteredString(BASEVIDWIDTH/2, 112, 0, va("Your Putts: %d", players[me].health-1));
		V_DrawCenteredString(BASEVIDWIDTH/2, 128, V_WHITEMAP, "Score Bonus:");

		scorebonus = (par - (players[me].health - 2))*100;

		if(scorebonus < 0)
			scorebonus = 0;
		V_DrawCenteredString(BASEVIDWIDTH/2, 144, V_WHITEMAP, va("%d", scorebonus));

		return;
	}

    WI_slamBackground();

	p = W_CachePatchName("RESULT",PU_CACHE); // Tails
    V_DrawScaledPatch ((BASEVIDWIDTH-p->width)/2,2,0,p); // Tails

topscore[0] = -1; /* cut'n'paste rather than a loop, since it's faster */
topscore[1] = -1; /* 0s can change to -1 -- see later for details */
topscore[2] = -1;
topscore[3] = -1;
topscore[4] = -1;
topscore[5] = -1;
topscore[6] = -1;
topscore[7] = -1;
topscore[8] = -1;
topscore[9] = -1;
topscore[10] = -1;
topscore[11] = -1;
topscore[12] = -1;
topscore[13] = -1;
topscore[14] = -1;
etopscore[0] = -1; /* LEAVE THESE AS -1 !!! */
etopscore[1] = -1;
etopscore[2] = -1;
etopscore[3] = -1;
etopscore[4] = -1;
etopscore[5] = -1;
etopscore[6] = -1;
etopscore[7] = -1;
etopscore[8] = -1;
etopscore[9] = -1;
etopscore[10] = -1;
etopscore[11] = -1;
etopscore[12] = -1;
etopscore[13] = -1;
etopscore[14] = -1;

// Start Calculate Score Position First Tails 04-30-2001
for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
{
 if (playeringame[i])
 {
   if (topscore[0]<players[i].score)
    { topscore[0] = players[i].score; etopscore[0] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
{
 if (playeringame[i])
 {
   if (topscore[1]<players[i].score && i != etopscore[0]) /* ignore last player */
   { topscore[1] = players[i].score; etopscore[1] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
{
 if (playeringame[i])
 {
   if (topscore[2]<players[i].score && i != etopscore[0] && i != etopscore[1]) /* ignore last player */
   { topscore[2] = players[i].score; etopscore[2] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
{
 if (playeringame[i])
 {
   if (topscore[3]<players[i].score && i!=etopscore[0] && i!=etopscore[1] && i!=etopscore[2]) /* ignore last player */
   { topscore[3] = players[i].score; etopscore[3] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab fifth ranking */
{
 if (playeringame[i])
 {
   if (topscore[4]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]) /* ignore last player */
   { topscore[4] = players[i].score; etopscore[4] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab sixth ranking */
{
 if (playeringame[i])
 {
   if (topscore[5]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]) /* ignore last player */
   { topscore[5] = players[i].score; etopscore[5] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab seventh ranking */
{
 if (playeringame[i])
 {
   if (topscore[6]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]
	   && i!=etopscore[5]) /* ignore last player */
   { topscore[6] = players[i].score; etopscore[6] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab eighth ranking */
{
 if (playeringame[i])
 {
   if (topscore[7]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]
	   && i!=etopscore[5]
	   && i!=etopscore[6]) /* ignore last player */
   { topscore[7] = players[i].score; etopscore[7] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab ninth ranking */
{
 if (playeringame[i])
 {
   if (topscore[8]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]
	   && i!=etopscore[5]
	   && i!=etopscore[6]
	   && i!=etopscore[7]) /* ignore last player */
   { topscore[8] = players[i].score; etopscore[8] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab tenth ranking */
{
 if (playeringame[i])
 {
   if (topscore[9]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]
	   && i!=etopscore[5]
	   && i!=etopscore[6]
	   && i!=etopscore[7]
	   && i!=etopscore[8]) /* ignore last player */
   { topscore[9] = players[i].score; etopscore[9] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab eleventh ranking */
{
 if (playeringame[i])
 {
   if (topscore[10]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]
	   && i!=etopscore[5]
	   && i!=etopscore[6]
	   && i!=etopscore[7]
	   && i!=etopscore[8]
	   && i!=etopscore[9]) /* ignore last player */
   { topscore[10] = players[i].score; etopscore[10] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab twelfth ranking */
{
 if (playeringame[i])
 {
   if (topscore[11]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]
	   && i!=etopscore[5]
	   && i!=etopscore[6]
	   && i!=etopscore[7]
	   && i!=etopscore[8]
	   && i!=etopscore[9]
	   && i!=etopscore[10]) /* ignore last player */
   { topscore[11] = players[i].score; etopscore[11] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab thirteenth ranking */
{
 if (playeringame[i])
 {
   if (topscore[12]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]
	   && i!=etopscore[5]
	   && i!=etopscore[6]
	   && i!=etopscore[7]
	   && i!=etopscore[8]
	   && i!=etopscore[9]
	   && i!=etopscore[10]
	   && i!=etopscore[11]) /* ignore last player */
   { topscore[12] = players[i].score; etopscore[12] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab fourteenth ranking */
{
 if (playeringame[i])
 {
   if (topscore[13]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]
	   && i!=etopscore[5]
	   && i!=etopscore[6]
	   && i!=etopscore[7]
	   && i!=etopscore[8]
	   && i!=etopscore[9]
	   && i!=etopscore[10]
	   && i!=etopscore[11]
	   && i!=etopscore[12]) /* ignore last player */
   { topscore[13] = players[i].score; etopscore[13] = i; }
 }
}
for (i=0; i<MAXPLAYERS; i++) /* grab fifteenth ranking */
{
 if (playeringame[i])
 {
   if (topscore[13]<players[i].score
	   && i!=etopscore[0]
	   && i!=etopscore[1]
	   && i!=etopscore[2]
	   && i!=etopscore[3]
	   && i!=etopscore[4]
	   && i!=etopscore[5]
	   && i!=etopscore[6]
	   && i!=etopscore[7]
	   && i!=etopscore[8]
	   && i!=etopscore[9]
	   && i!=etopscore[10]
	   && i!=etopscore[11]
	   && i!=etopscore[12]
	   && i!=etopscore[13]) /* ignore last player */
   { topscore[14] = players[i].score; etopscore[14] = i; }
 }
}
// End Calculate Score Position First Tails 04-30-2001

// Now do all the fun score drawing stuff! Tails 05-01-2001
{
    fragsort_t   fragtab[15];
    int          i;
    int          scorelines;
    int          whiteplayer;

    // count frags for each present player
    scorelines = 0;
    for (i=0; i<11; i++) // Can only fit 11 ATM
    {
		if(!playeringame[etopscore[i]])
			continue;

		if(cv_gametype.value == GT_MATCH || cv_gametype.value == GT_CHAOS)
			fragtab[scorelines].count = players[etopscore[i]].score; // Tails 12-08-2000
		else if (cv_gametype.value == GT_TAG)
			fragtab[scorelines].count = players[etopscore[i]].tagcount; // Tails 05-09-2001

        fragtab[scorelines].num   = etopscore[i];
        fragtab[scorelines].color = players[etopscore[i]].skincolor;
        fragtab[scorelines].name  = player_names[etopscore[i]];
        scorelines++;


    }

    //Fab:25-04-98: when you play, you quickly see your frags because your
    //  name is displayed white, when playback demo, you quicly see who's the
    //  view.
    whiteplayer = demoplayback ? displayplayer : consoleplayer;

    WI_drawIntRanking(NULL,80,24,fragtab,scorelines,true,whiteplayer);
}

/*
    //Fab:25-04-98: when you play, you quickly see your frags because your
    //  name is displayed white, when playback demo, you quicly see who's the
    //  view.
    whiteplayer = demoplayback ? displayplayer : consoleplayer;

    // count frags for each present player
    scorelines = 0;
    for (i=0; i<MAXPLAYERS; i++)
    {
        if (playeringame[i])
        {
			if(cv_gametype.value == GT_MATCH)
				fragtab[scorelines].count = players[i].score; // Tails 12-08-2000
			else if (cv_gametype.value == GT_TAG)
				fragtab[scorelines].count = players[i].tagcount; // Tails 05-09-2001

            fragtab[scorelines].num   = i;
            fragtab[scorelines].color = players[i].skincolor;
            fragtab[scorelines].name  = player_names[i];
            scorelines++;
        }
    }

	// Tails 04-02-2001
    if (scorelines>14)
        scorelines = 14; //dont draw past bottom of screen, show the best only

    WI_drawRancking(NULL,80,48,fragtab,scorelines,true,whiteplayer);
*/
    timeleft=va("start in %d seconds",cnt_pause/TICRATE);
    //i=V_StringWidth(num);
    V_DrawString (160 - V_StringWidth(timeleft)/2, 188, V_WHITEMAP, timeleft);
}

// UGLEEEE!!!
// Tails 12-17-2003
static void WI_drawCircuitStats(void)
{
	boolean nodraw = false;
    char         *timeleft;
	//int      temp;
    int         i;
    //int         x;
    //int         y; // used as a dummy now Tails 05-01-2001
	//int         a; // x position for columns
	//int         b; // counter for times gone through
	//byte* colormap;

signed int topscore[15]; /* maximum value found -- OUTPUT VALUE */
signed int etopscore[15]; /* element it was found in -- OUTPUT VALUE */

			//char sscore[33];
			//char sseconds[33];
			//char sminutes[33];
			//char stics[33];

	static patch_t* act; // Tails 11-21-2000

			WI_slamBackground();

			act = W_CachePatchName("RESULT",PU_CACHE); // Tails
			V_DrawScaledPatch ((BASEVIDWIDTH-act->width)/2,2,0,act); // Tails

			// DO NOT BE FOOLED!
			// This is really finding the fastest time, not the
			// highest score. Cheap cut & paste from the Match Mode drawer.

			topscore[0] = MAXINT; // cut'n'paste rather than a loop, since it's faster
			topscore[1] = MAXINT; // 0s can change to -1 -- see later for details
			topscore[2] = MAXINT;
			topscore[3] = MAXINT;
			topscore[4] = MAXINT;
			topscore[5] = MAXINT;
			topscore[6] = MAXINT;
			topscore[7] = MAXINT;
			topscore[8] = MAXINT;
			topscore[9] = MAXINT;
			topscore[10] = MAXINT;
			topscore[11] = MAXINT;
			topscore[12] = MAXINT;
			topscore[13] = MAXINT;
			topscore[14] = MAXINT;
			etopscore[0] = -1; // LEAVE THESE AS -1 !!!
			etopscore[1] = -1;
			etopscore[2] = -1;
			etopscore[3] = -1;
			etopscore[4] = -1;
			etopscore[5] = -1;
			etopscore[6] = -1;
			etopscore[7] = -1;
			etopscore[8] = -1;
			etopscore[9] = -1;
			etopscore[10] = -1;
			etopscore[11] = -1;
			etopscore[12] = -1;
			etopscore[13] = -1;
			etopscore[14] = -1;

			// Start Calculate Time Position Tails 04-30-2001
			for (i=0; i<MAXPLAYERS; i++) // grab highest ranking
			{
			 if (playeringame[i])
			 {
			   if (topscore[0]>players[i].realtime)
				{ topscore[0] = players[i].realtime; etopscore[0] = i; }
			 }
			}

			for (i=0; i<MAXPLAYERS; i++) // grab second ranking
			{
			 if (playeringame[i])
			 {
			   if (topscore[1]>players[i].realtime && i != etopscore[0]) // ignore last player 
			   { topscore[1] = players[i].realtime; etopscore[1] = i; }
			 }
			}

			for (i=0; i<MAXPLAYERS; i++) // grab third ranking
			{
			 if (playeringame[i])
			 {
			   if (topscore[2]>players[i].realtime && i != etopscore[0] && i != etopscore[1]) // ignore last player 
			   { topscore[2] = players[i].realtime; etopscore[2] = i; }
			 }
			}

			for (i=0; i<MAXPLAYERS; i++) // grab fourth ranking
			{
			 if (playeringame[i])
			 {
			   if (topscore[3]>players[i].realtime && i!=etopscore[0] && i!=etopscore[1] && i!=etopscore[2]) /* ignore last player */
			   { topscore[3] = players[i].realtime; etopscore[3] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) // grab fifth ranking
			{
			 if (playeringame[i])
			 {
			   if (topscore[4]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]) // ignore last player
			   { topscore[4] = players[i].realtime; etopscore[4] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab sixth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[5]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]) /* ignore last player */
			   { topscore[5] = players[i].realtime; etopscore[5] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab seventh ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[6]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]) /* ignore last player */
			   { topscore[6] = players[i].realtime; etopscore[6] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab eighth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[7]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]) /* ignore last player */
			   { topscore[7] = players[i].realtime; etopscore[7] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab ninth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[8]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]) /* ignore last player */
			   { topscore[8] = players[i].realtime; etopscore[8] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab tenth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[9]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]) /* ignore last player */
			   { topscore[9] = players[i].realtime; etopscore[9] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab eleventh ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[10]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]) /* ignore last player */
			   { topscore[10] = players[i].realtime; etopscore[10] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab twelfth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[11]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]
				   && i!=etopscore[10]) /* ignore last player */
			   { topscore[11] = players[i].realtime; etopscore[11] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab thirteenth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[12]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]
				   && i!=etopscore[10]
				   && i!=etopscore[11]) /* ignore last player */
			   { topscore[12] = players[i].realtime; etopscore[12] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab fourteenth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[13]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]
				   && i!=etopscore[10]
				   && i!=etopscore[11]
				   && i!=etopscore[12]) /* ignore last player */
			   { topscore[13] = players[i].realtime; etopscore[13] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab fifteenth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[13]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]
				   && i!=etopscore[10]
				   && i!=etopscore[11]
				   && i!=etopscore[12]
				   && i!=etopscore[13]) /* ignore last player */
			   { topscore[14] = players[i].realtime; etopscore[14] = i; }
			 }
			}
			// End Calculate Score Position First Tails 04-30-2001

// Now do all the fun score drawing stuff! Tails 05-01-2001
{
    fragsort_t   fragtab[15];
    int          i;
    int          scorelines;
    int          whiteplayer;

    // count frags for each present player
    scorelines = 0;
    for (i=0; i<11; i++) // Can only fit 11 ATM
    {
		if(!playeringame[etopscore[i]])
			continue;

		fragtab[scorelines].count = players[etopscore[i]].realtime; // Tails 12-08-2000

        fragtab[scorelines].num   = etopscore[i];
        fragtab[scorelines].color = players[etopscore[i]].skincolor;
        fragtab[scorelines].name  = player_names[etopscore[i]];
        scorelines++;
    }

    //Fab:25-04-98: when you play, you quickly see your frags because your
    //  name is displayed white, when playback demo, you quicly see who's the
    //  view.
    whiteplayer = demoplayback ? displayplayer : consoleplayer;

    WI_drawCircuitRanking(NULL,80,24,fragtab,scorelines,true,whiteplayer);
}

    timeleft=va("start in %d seconds",cnt_pause/TICRATE);
    //i=V_StringWidth(num);
    V_DrawString (160 - V_StringWidth(timeleft)/2, 188, V_WHITEMAP, timeleft);
}

boolean teamingame(int teamnum)
{
   int i;

   if (cv_teamplay.value == 1)
   {
       for(i=0;i<MAXPLAYERS;i++)
          if(playeringame[i] && players[i].skincolor==teamnum)
              return true;
   }
   else
   if (cv_teamplay.value == 2)
   {
       for(i=0;i<MAXPLAYERS;i++)
          if(playeringame[i] && players[i].skin==teamnum)
              return true;
   }
   return false;
}

static int      cnt_frags[MAXPLAYERS];
static int      dofrags;
static int      ng_state;

static void WI_initNetgameStats(void)
{

    int i;

    state = StatCount;
    acceleratestage = 0;
    ng_state = 1;

    cnt_pause = TICRATE;

	if(cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CHAOS) // Match or Tag Tails 05-19-2001
		WI_initNoState();

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        if (!playeringame[i])
            continue;

		if (players[i].realtime / TICRATE < 30)
			players[i].timebonus = 50000;

		else if (players[i].realtime / TICRATE < 45 && players[i].realtime / TICRATE >= 30)
			players[i].timebonus = 10000;

		else if (players[i].realtime / TICRATE < 60 && players[i].realtime / TICRATE >= 45)
			players[i].timebonus = 5000;

		else if (players[i].realtime / TICRATE < 90 && players[i].realtime / TICRATE >= 60)
			players[i].timebonus = 4000;

		else if (players[i].realtime / TICRATE < 120 && players[i].realtime / TICRATE >= 90)
			players[i].timebonus = 3000;

		else if (players[i].realtime / TICRATE < 180 && players[i].realtime / TICRATE >= 120)
			players[i].timebonus = 2000;

		else if (players[i].realtime / TICRATE < 240 && players[i].realtime / TICRATE >= 180)
			players[i].timebonus = 1000;

		else if (players[i].realtime / TICRATE < 300 && players[i].realtime / TICRATE >= 240)
			players[i].timebonus = 500;

		else
			players[i].timebonus = 0;

// End Time Bonus & Ring Bonus count settings Tails 03-10-2000

        cnt_fscore[i] = (plrs[i].sscore);  // Tails 03-14-2000
        cnt_score[i] = 0; // Score Tails 03-09-2000
        cnt_timebonus[i] = (players[i].timebonus); // Time Bonus Tails 03-10-2000
        cnt_ringbonus[i] = (100*(players[i].health - 1)); // Ring Bonus Tails 03-10-2000

	//        cnt_kills[i] = 
	// cnt_items[i] = cnt_secret[i] = cnt_frags[i] = 0;

//        dofrags += ST_PlayerFrags(i);
    }

    dofrags = !!dofrags; // What the bloody hell is this? Graue 12-06-2003
}


// What does this do, what's ng_state, how do cnt_fscore and cnt_score differ? Graue 12-06-2003
static void WI_updateNetgameStats(void)
{

    int         i;
	int			z;
	int			numplayers;
	int			numdone;

    boolean     stillticking;

	// I'm still not quite sure what this does, but we only get into this function if the
	// gametype is 0, 2, or 4 (or 6, which I've added), and the next if statement used to
	// set ng_state if gametype was not 2 or 4, so it's effectively for anything but 0.
	// Presumably, setting ng_state to 10 somehow skips stuff that's only for coop mode.
	// Notes by Graue 12-06-2003
	if(cv_gametype.value != GT_COOP) // Originally by Tails 05-19-2001
		ng_state = 10; // Tails 05-19-2001

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
		ng_state = 10;

    if (acceleratestage && ng_state != 10)
    {
        acceleratestage = 0;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
                continue;

            cnt_fscore[i] = (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1))); // Tails 03-14-2000
            cnt_score[i] = (players[i].timebonus + (100*(players[i].health - 1))); // Score Tails 03-09-2000
            cnt_timebonus[i] = 0; // Time Bonus Tails 03-10-2000
            cnt_ringbonus[i] = 0; // Ring Bonus Tails 03-10-2000

	// Gives lives for score Tails 04-11-2001
			if(cnt_fscore[i] >= 50000+players[i].xtralife2)
			{
				players[i].lives++;

				if(mariomode)
					S_StartSound(0, sfx_marioa);
				else
				{
					if(i==consoleplayer)
					{
						S_StopMusic();
						S_ChangeMusic(mus_xtlife, false);
					}
				}
				players[i].xtralife2 += 50000;
			}
	// End score Tails 04-11-2001
        }
        S_StartSound(0, sfx_chchng); // Tails
        ng_state = 10;
    }

    if (ng_state == 2)
    {
		if(!(gamemap >= sstage_start && gamemap <= sstage_end))
		{	
			stillticking = true;

			for (i=0 ; i<MAXPLAYERS ; i++)
			{
				if (!playeringame[i])
					continue;

				if((cnt_timebonus[i] > 0) && (cnt_ringbonus[i] > 0))
				{
				  cnt_fscore[i] += 444; // Tails 03-14-2000
				  cnt_score[i] += 444; // Score Tails 03-09-2000
				}
				else
				{
					cnt_fscore[i] += 222; // Tails 03-14-2000
					cnt_score[i] += 222; // Score Tails 03-09-2000
				}

				cnt_timebonus[i] -= 222; // Time Bonus Tails 03-10-2000
				cnt_ringbonus[i] -= 222; // Ring Bonus Tails 03-10-2000

				if (cnt_fscore[i] >= (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1)))) // Score
					cnt_fscore[i] = (plrs[i].sscore + players[i].timebonus + (100*(players[i].health - 1)));  // Tails 03-09-2000

				if (cnt_score[i] >= (players[i].timebonus + (100*(players[i].health - 1)))) // Score
				    cnt_score[i] = (players[i].timebonus + (100*(players[i].health - 1)));  // Tails 03-09-2000

				// Gives lives for score Tails 04-11-2001
				if(cnt_fscore[i] >= 50000+players[i].xtralife2)
				{
					players[i].lives++;
					if(mariomode)
						S_StartSound(0, sfx_marioa);
					else
					{
						if(i==consoleplayer)
						{
							S_StopMusic();
							S_ChangeMusic(mus_xtlife, false);
						}
					}
					players[i].xtralife2 += 50000;
				}
				// End score Tails 04-11-2001			  
			  
				// start timebonus, ringbonus & stuff Tails 03-10-2000
				if (cnt_timebonus[i] <= 0)
				{
					cnt_timebonus[i] = 0;
				}

				if (cnt_ringbonus[i] <= 0)
				{
					cnt_ringbonus[i] = 0;
				}

				// end timebonus, ringbonus & stuff Tails 03-10-2000

				if(cnt_score[i] == (players[i].timebonus + (100*(players[i].health - 1))))  // Tails 03-09-2000
				{
					stillticking = false;
					players[i].score = cnt_fscore[i];
				}
				else
				{
					stillticking = true;
					if (!(bcnt&1) && !(cnt_score[me] == (players[me].timebonus + (100*(players[me].health - 1))))) // count sound faster! Tails 03-10-2000
						S_StartSound(0, sfx_menu1);
				}
			}
		
			if (!stillticking) // Wait for everybody! Tails 07-20-2001
			{
				numplayers = 0;
				numdone = 0;
				for(z=0; z<MAXPLAYERS; z++)
				{
					if(playeringame[z])
					{
						numplayers++;
						if((cnt_ringbonus[z] == 0) && (cnt_timebonus[z] == 0) && (cnt_score[z] == players[z].timebonus + (100*(players[z].health - 1))))
							numdone++;
					}
				}
				if(numdone == numplayers)
				{
					S_StartSound(0, sfx_chchng);
					ng_state = 10;
				}
			}
		}
		else // Special Stage Tails 08-12-2001
		{
			stillticking = true;

			for (i=0 ; i<MAXPLAYERS ; i++)
			{
				if (!playeringame[i])
					continue;

				  cnt_fscore[i] += 222; // Tails 03-14-2000
				  cnt_ringbonus[i] -= 222; // Ring Bonus Tails 03-10-2000

				  if (cnt_fscore[i] >= (plrs[i].sscore + (100*(players[i].health - 1)))) // Score
					  cnt_fscore[i] = (plrs[i].sscore + (100*(players[i].health - 1)));  // Tails 03-09-2000

				// Gives lives for score Tails 04-11-2001
				if(cnt_fscore[i] >= 50000+players[i].xtralife2)
				{
					players[i].lives++;
					if(mariomode)
						S_StartSound(0, sfx_marioa);
					else
					{
						if(i==consoleplayer)
						{
							S_StopMusic();
							S_ChangeMusic(mus_xtlife, false);
						}
					}
					players[i].xtralife2 += 50000;
				}
				// End score Tails 04-11-2001			  
			  
				if (cnt_ringbonus[i] <= 0)
					cnt_ringbonus[i] = 0;

					if(cnt_fscore[i] == (plrs[i].sscore + (100*(players[i].health - 1))))  // Tails 03-09-2000
					{
						stillticking = false;
						players[i].score = cnt_fscore[i];
					}
					else
					{
						stillticking = true;
						if (!(bcnt&1) && !(cnt_fscore[me] == (plrs[me].sscore + (100*(players[me].health - 1))))) // count sound faster! Tails 03-10-2000
							S_StartSound(0, sfx_menu1);
					}
				}
		
				if (!stillticking) // Wait for everybody! Tails 07-20-2001
				{
					numplayers = 0;
					numdone = 0;
					for(z=0; z<MAXPLAYERS; z++)
					{
						if(playeringame[z])
						{
							numplayers++;
							if((cnt_ringbonus[z] == 0) && (cnt_fscore[z] == plrs[z].sscore + (100*(players[z].health - 1))))
								numdone++;
						}
					}
					if(numdone == numplayers)
					{
						S_StartSound(0, sfx_chchng);
						ng_state = 10;
					}

				}
			}
		}

    else if (ng_state == 4)
    {
        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
                continue;
        }
        if (!stillticking)
        {
            //S_StartSound(0, sfx_chchng); // Tails
            ng_state++;
        }
    }
    else if (ng_state == 6)
    {
        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
                continue;
        }
    }
    else if (ng_state == 8)
    {
        if (!(bcnt&1)) // count sound faster! Tails 03-10-2000
            S_StartSound(0, sfx_menu1);

        stillticking = false;

        for (i=0 ; i<MAXPLAYERS ; i++)
        {
            if (!playeringame[i])
                continue;

            cnt_frags[i] += 1;
        }

        if (!stillticking)
        {
            ng_state++;
        }
    }
    else if (ng_state == 10)
    {
        if (acceleratestage)
        {
			if(cv_gametype.value == GT_CTF)
			{
				int rings;
				rings = 0;
				for(i=0; i<MAXPLAYERS; i++)
				{
					if(playeringame[i] && players[i].ctfteam == 1)
						rings += players[i].health - 1;
				}
				if(rings >= 100)
				{
					redscore += 100;
				}
				rings = 0;
				for(i=0; i<MAXPLAYERS; i++)
				{
					if(playeringame[i] && players[i].ctfteam == 2)
						rings += players[i].health - 1;
				}
				if(rings >= 100)
				{
					bluescore += 100;
				}
			}
			if(cv_gametype.value == GT_COOP)
			{
				for(i=0; i<MAXPLAYERS; i++) // Chuck the tallied score into the player's score Tails 08-02-2001
					players[i].score = cnt_fscore[i];
			}

			if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
			{
				int scorebonus;

				for(i=0; i<MAXPLAYERS;i++)
				{
					scorebonus = (par - (players[i].health - 2))*100;

					if(scorebonus < 0)
						scorebonus = 0;

					P_AddPlayerScore(&players[i], scorebonus);
				}
			}

			WI_initNoState();
        }
    }
    else if (ng_state & 1)
    {
        if (!--cnt_pause)
        {
            ng_state++;
            cnt_pause = TICRATE;
        }
    }
}

extern consvar_t cv_racetype; // Tails 08-29-2002

static void WI_drawNetgameStats(void)
{
	boolean nodraw = false;
    int         i;
    int         x;
    int         y; // used as a dummy now Tails 05-01-2001
	int         a; // x position for columns
	int         b; // counter for times gone through
	byte* colormap;

signed int topscore[15]; /* maximum value found -- OUTPUT VALUE */
signed int etopscore[15]; /* element it was found in -- OUTPUT VALUE */
signed int toptime[4]; /* maximum value found -- OUTPUT VALUE */
signed int etoptime[4]; /* element it was found in -- OUTPUT VALUE */
signed int topitembox[4]; /* maximum value found -- OUTPUT VALUE */
signed int etopitembox[4]; /* element it was found in -- OUTPUT VALUE */
signed int topring[4]; /* maximum value found -- OUTPUT VALUE */
signed int etopring[4]; /* element it was found in -- OUTPUT VALUE */
signed int toptotalring[4]; /* maximum value found -- OUTPUT VALUE */
signed int etoptotalring[4]; /* element it was found in -- OUTPUT VALUE */

signed int racerank[4]; // Tails 05-01-2001
signed int eracerank[4]; // Tails 05-01-2001

			char splaynum[33];
			char sscore[33];
			char sseconds[33];
			char sminutes[33];
			char stics[33];
			char sring[33];
			char stotring[33];
			char sitembox[33];
			char setopscore[33];
			char setoptime[33];
			char setopitembox[33];
			char setopring[33];
			char setoptotalring[33];
			char sracescore[33];

    int lh;
	static patch_t* act; // Tails 11-21-2000

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
	{
		int scorebonus;
		V_DrawFill(0,0,vid.width, vid.height, 0);

		V_DrawCenteredString(BASEVIDWIDTH/2, 96, 0, va("Par: %d", par));
		V_DrawCenteredString(BASEVIDWIDTH/2, 112, 0, va("Your Putts: %d", players[me].health-1));
		V_DrawCenteredString(BASEVIDWIDTH/2, 128, V_WHITEMAP, "Score Bonus:");

		scorebonus = (par - (players[me].health - 2))*100;

		if(scorebonus < 0)
			scorebonus = 0;
		V_DrawCenteredString(BASEVIDWIDTH/2, 144, V_WHITEMAP, va("%d", scorebonus));

		return;
	}

    lh = (3*SHORT(num[0]->height))/2;

    WI_slamBackground();

	if(gamemap >= sstage_start && gamemap <= sstage_end)
	{
		// Draw big CHAOS EMERALDS thing
		if(gameskill > sk_easy)
		{
			if(emeralds & EMERALD1 && emeralds & EMERALD2 && emeralds & EMERALD3 && emeralds & EMERALD4 && emeralds & EMERALD5 && emeralds & EMERALD6 && emeralds & EMERALD7)
			{
				if(gameskill > sk_easy)
				{
					if(ng_state == 10)
						V_DrawScaledPatch(48, 32, FB, nowsuper);
					else
						V_DrawScaledPatch(70, 26, FB, gotemall);
				}
			}
			else
				V_DrawScaledPatch(48, 26, FB, cemerald);
		}
		else
		{
			if(emeralds & EMERALD1 && emeralds & EMERALD2 && emeralds & EMERALD3)
			{
				V_DrawScaledPatch(70, 26, FB, gotemall);
				V_DrawCenteredString(BASEVIDWIDTH/2, BASEVIDHEIGHT/2, V_WHITEMAP, "Or have you? Play on Normal or higher to collect them all!");
			}
			else
				V_DrawScaledPatch(48, 26, FB, cemerald);
		}

		if(blinker & 1)
		{
			// Display Chaos Emeralds collected
			if(emeralds & EMERALD1)
				V_DrawScaledPatch(80, 92, FB, chaos1);
			if(emeralds & EMERALD2)
				V_DrawScaledPatch(104, 92, FB, chaos2);
			if(emeralds & EMERALD3)
				V_DrawScaledPatch(128, 92, FB, chaos3);
			if(emeralds & EMERALD4)
				V_DrawScaledPatch(152, 92, FB, chaos4);
			if(emeralds & EMERALD5)
				V_DrawScaledPatch(176, 92, FB, chaos5);
			if(emeralds & EMERALD6)
				V_DrawScaledPatch(200, 92, FB, chaos6);
			if(emeralds & EMERALD7)
				V_DrawScaledPatch(224, 92, FB, chaos7);
		}

    V_DrawScaledPatch(80, 132, FB, sp_secret);
    WI_drawPercent(232, 132, cnt_ringbonus[0]); // Ring Bonus Tails 03-10-2000

    V_DrawScaledPatch(80, 148, FB, cscore);
    WI_drawPercent(232, 148, cnt_fscore[0]); // Score Tails 03-10-2000
		blinker++; // Tails 08-12-2001
		return;
	}
	
	if(cv_gametype.value == GT_CTF)
	{
		char sscore[33];
		int rings;
		V_DrawString(160+64, 32, 0, "Red");
		V_DrawString(72+64, 32, 0, "Blue");
		V_DrawString(16, 48, 0, "Score");
		sprintf(sscore, "%i", redscore);
		V_DrawString(176+64, 48, 0, sscore);
		sprintf(sscore, "%i", bluescore);
		V_DrawString(92+64, 48, 0, sscore);
		V_DrawString(16, 64, 0, "Ring Bonus");
		V_DrawString(16, 80, 0, "Total Score");
		sprintf(sscore, "%i", redscore);
		V_DrawString(176+64, 48, 0, sscore);
		sprintf(sscore, "%i", bluescore);
		V_DrawString(92+64, 48, 0, sscore);
		rings = 0;
		// Do Red Team's Score First
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(playeringame[i] && players[i].ctfteam == 1)
				rings += players[i].health - 1;
		}
		if(rings >= 100)
		{
			sprintf(sscore, "%i", redscore+1);
			V_DrawString(176+64,64, 0, "1");
			V_DrawString(176+64,80, 0, sscore);
		}
		else
		{
			sprintf(sscore, "%i", redscore);
			V_DrawString(176+64,64, 0, "0");
			V_DrawString(176+64,80, 0, sscore);
		}
		rings = 0;
		// Now Do Blue Team's Score
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(playeringame[i] && players[i].ctfteam == 2)
				rings += players[i].health - 1;
		}
		if(rings >= 100)
		{
			sprintf(sscore, "%i", bluescore+1);
			V_DrawString(92+64,64, 0, "1");
			V_DrawString(92+64,80, 0, sscore);
		}
		else
		{
			sprintf(sscore, "%i", bluescore);
			V_DrawString(92+64,64, 0, "0");
			V_DrawString(92+64,80, 0, sscore);
		}
		return;
	}

	//FIXTHIS: badly modified by Graue 12-06-2003 for circuit mode
	else if(cv_gametype.value == GT_RACE)
	{
		if(cv_racetype.value == 1) // Time only
		{
			int temp; // My own little toss-around variable. =)

			act = W_CachePatchName("RESULT",PU_CACHE); // Tails
			V_DrawScaledPatch ((BASEVIDWIDTH-act->width)/2,2,0,act); // Tails

			// DO NOT BE FOOLED!
			// This is really finding the fastest time, not the
			// highest score. Cheap cut & paste from the Match Mode drawer.

			topscore[0] = MAXINT; /* cut'n'paste rather than a loop, since it's faster */
			topscore[1] = MAXINT; /* 0s can change to -1 -- see later for details */
			topscore[2] = MAXINT;
			topscore[3] = MAXINT;
			topscore[4] = MAXINT;
			topscore[5] = MAXINT;
			topscore[6] = MAXINT;
			topscore[7] = MAXINT;
			topscore[8] = MAXINT;
			topscore[9] = MAXINT;
			topscore[10] = MAXINT;
			topscore[11] = MAXINT;
			topscore[12] = MAXINT;
			topscore[13] = MAXINT;
			topscore[14] = MAXINT;
			etopscore[0] = -1; /* LEAVE THESE AS -1 !!! */
			etopscore[1] = -1;
			etopscore[2] = -1;
			etopscore[3] = -1;
			etopscore[4] = -1;
			etopscore[5] = -1;
			etopscore[6] = -1;
			etopscore[7] = -1;
			etopscore[8] = -1;
			etopscore[9] = -1;
			etopscore[10] = -1;
			etopscore[11] = -1;
			etopscore[12] = -1;
			etopscore[13] = -1;
			etopscore[14] = -1;

			// Start Calculate Time Position Tails 04-30-2001
			for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[0]>players[i].realtime)
				{ topscore[0] = players[i].realtime; etopscore[0] = i; }
			 }
			}

			for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[1]>players[i].realtime && i != etopscore[0]) /* ignore last player */
			   { topscore[1] = players[i].realtime; etopscore[1] = i; }
			 }
			}

			for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[2]>players[i].realtime && i != etopscore[0] && i != etopscore[1]) /* ignore last player */
			   { topscore[2] = players[i].realtime; etopscore[2] = i; }
			 }
			}

			for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[3]>players[i].realtime && i!=etopscore[0] && i!=etopscore[1] && i!=etopscore[2]) /* ignore last player */
			   { topscore[3] = players[i].realtime; etopscore[3] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab fifth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[4]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]) /* ignore last player */
			   { topscore[4] = players[i].realtime; etopscore[4] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab sixth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[5]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]) /* ignore last player */
			   { topscore[5] = players[i].realtime; etopscore[5] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab seventh ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[6]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]) /* ignore last player */
			   { topscore[6] = players[i].realtime; etopscore[6] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab eighth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[7]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]) /* ignore last player */
			   { topscore[7] = players[i].realtime; etopscore[7] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab ninth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[8]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]) /* ignore last player */
			   { topscore[8] = players[i].realtime; etopscore[8] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab tenth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[9]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]) /* ignore last player */
			   { topscore[9] = players[i].realtime; etopscore[9] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab eleventh ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[10]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]) /* ignore last player */
			   { topscore[10] = players[i].realtime; etopscore[10] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab twelfth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[11]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]
				   && i!=etopscore[10]) /* ignore last player */
			   { topscore[11] = players[i].realtime; etopscore[11] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab thirteenth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[12]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]
				   && i!=etopscore[10]
				   && i!=etopscore[11]) /* ignore last player */
			   { topscore[12] = players[i].realtime; etopscore[12] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab fourteenth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[13]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]
				   && i!=etopscore[10]
				   && i!=etopscore[11]
				   && i!=etopscore[12]) /* ignore last player */
			   { topscore[13] = players[i].realtime; etopscore[13] = i; }
			 }
			}
			for (i=0; i<MAXPLAYERS; i++) /* grab fifteenth ranking */
			{
			 if (playeringame[i])
			 {
			   if (topscore[13]>players[i].realtime
				   && i!=etopscore[0]
				   && i!=etopscore[1]
				   && i!=etopscore[2]
				   && i!=etopscore[3]
				   && i!=etopscore[4]
				   && i!=etopscore[5]
				   && i!=etopscore[6]
				   && i!=etopscore[7]
				   && i!=etopscore[8]
				   && i!=etopscore[9]
				   && i!=etopscore[10]
				   && i!=etopscore[11]
				   && i!=etopscore[12]
				   && i!=etopscore[13]) /* ignore last player */
			   { topscore[14] = players[i].realtime; etopscore[14] = i; }
			 }
			}
			// End Calculate Score Position First Tails 04-30-2001

			// Now do all the fun score drawing stuff! Tails 05-01-2001
			b = 1;
			a = 1;
			i = etopscore[0];
			while(true)
			{
				if(!playeringame[i])
					break;

				temp = (players[i].realtime/TICRATE) % 60;

				if(temp < 10) // A single-digit number
				{
					sprintf(sseconds, "%i", 0);
					sprintf(sscore, "%i", temp);
					strcat(sseconds, sscore); // Made it a two-digit string
				}
				else
					sprintf(sseconds, "%i", temp);

				temp = players[i].realtime % TICRATE;

				if(temp < 10) // A single-digit number
				{
					sprintf(stics, "%i", 0);
					sprintf(sscore, "%i", temp);
					strcat(stics, sscore); // Made it a two-digit string
				}
				else
					sprintf(stics, "%i", temp);

				// We don't care if minutes is two digits or not
				sprintf(sminutes, "%i", players[i].realtime/(60*TICRATE));


				// Make one big 0:00:00 string
				strcat(sminutes, ":\0");
				strcat(sminutes, sseconds);
				strcat(sminutes, ":\0");
				strcat(sminutes, stics);

				if(b == 6 || b == 11)
					a = 1;

					x = 48*a;

					if(a > 10)
						y = 142;
					else if(a > 5)
						y = 86;
					else
						y = 30;

					V_DrawString(x-(V_StringWidth(player_names[i])/2)+16*a,y-8, 0, player_names[i]);

					if(players[i].skincolor==0)
						colormap = colormaps;
					else
						colormap = (byte *) translationtables[players[i].skin] - 256 + (players[i].skincolor<<8);

					V_DrawMappedPatch(x-16+16*a,y,0, W_CachePatchName(GetPlayerFacePic(players[i].skin), PU_CACHE),colormap);
					V_DrawString(x-(V_StringWidth(sminutes)/2)+16*a, y+32, 0, sminutes);
					
					if(i == etopscore[14])
					{
						V_DrawString(x-8+16*a,y+12,V_WHITEMAP, "15");
						break;
					}
					else
					{
						if(i == etopscore[0])
						{
							i = etopscore[1];
							V_DrawString(x-4+16*a,y+12,V_WHITEMAP, "1");
						}
						else if (i == etopscore[1])
						{
							i = etopscore[2];
							V_DrawString(x-4+16*a,y+12,V_WHITEMAP, "2");
						}
						else if (i == etopscore[2])
						{
							i = etopscore[3];
							V_DrawString(x-4+16*a,y+12,V_WHITEMAP, "3");
						}
						else if (i == etopscore[3])
						{
							i = etopscore[4];
							V_DrawString(x-4+16*a,y+12,V_WHITEMAP, "4");
						}
						else if (i == etopscore[4])
						{
							i = etopscore[5];
							V_DrawString(x-4+16*a,y+12,V_WHITEMAP, "5");
						}
						else if (i == etopscore[5])
						{
							i = etopscore[6];
							V_DrawString(x-4+16*a,y+12,V_WHITEMAP, "6");
						}
						else if (i == etopscore[6])
						{
							i = etopscore[7];
							V_DrawString(x-4+16*a,y+12,V_WHITEMAP, "7");
						}
						else if (i == etopscore[7])
						{
							i = etopscore[8];
							V_DrawString(x-4+16*a,y+12,V_WHITEMAP, "8");
						}
						else if (i == etopscore[8])
						{
							i = etopscore[9];
							V_DrawString(x-4+16*a,y+12,V_WHITEMAP, "9");
						}
						else if (i == etopscore[9])
						{
							i = etopscore[10];
							V_DrawString(x-8+16*a,y+12,V_WHITEMAP, "10");
						}
						else if (i == etopscore[10])
						{
							i = etopscore[11];
							V_DrawString(x-8+16*a,y+12,V_WHITEMAP, "11");
						}
						else if (i == etopscore[11])
						{
							i = etopscore[12];
							V_DrawString(x-8+16*a,y+12,V_WHITEMAP, "12");
						}
						else if (i == etopscore[12])
						{
							i = etopscore[13];
							V_DrawString(x-8+16*a,y+12,V_WHITEMAP, "13");
						}
						else if (i == etopscore[13])
						{
							i = etopscore[14];
							V_DrawString(x-8+16*a,y+12,V_WHITEMAP, "14");
						}
						b++;
						a++;
					}
			}
			return;
		}

		// Otherwise, do a full evaulation of the player

		if(mapheaderinfo[gamemap-1].actnum != 0)
			V_DrawCenteredString(BASEVIDWIDTH/2, 32, 0, va("%s * %d *", mapheaderinfo[gamemap-1].lvlttl, mapheaderinfo[gamemap-1].actnum));
		else
			V_DrawCenteredString(BASEVIDWIDTH/2, 32, 0, va("* %s *", mapheaderinfo[gamemap-1].lvlttl));

		// Draw the side labels Tails 04-27-2001
		V_DrawString(8,80, 0, "SCORE");
		V_DrawString(8,96, 0, "TIME");
		V_DrawString(8,112,0, "RING");
		V_DrawString(8,128,0, "TOT. RING");
		V_DrawString(8,144,0, "ITEM BOX");
		V_DrawString(0,168,0, "* TOTAL *");

topscore[0] = -1; /* cut'n'paste rather than a loop, since it's faster */
topscore[1] = -1; /* 0s can change to -1 -- see later for details */
topscore[2] = -1;
topscore[3] = -1;
etopscore[0] = -1; /* LEAVE THESE AS -1 !!! */
etopscore[1] = -1;
etopscore[2] = -1;
etopscore[3] = -1;

toptime[0] = players[0].realtime+1; /* cut'n'paste rather than a loop, since it's faster */
toptime[1] = players[0].realtime+1; /* 0s can change to -1 -- see later for details */
toptime[2] = players[0].realtime+1;
toptime[3] = players[0].realtime+1;
etoptime[0] = -1; /* LEAVE THESE AS -1 !!! */
etoptime[1] = -1;
etoptime[2] = -1;
etoptime[3] = -1;

topitembox[0] = -1; /* cut'n'paste rather than a loop, since it's faster */
topitembox[1] = -1; /* 0s can change to -1 -- see later for details */
topitembox[2] = -1;
topitembox[3] = -1;
etopitembox[0] = -1; /* LEAVE THESE AS -1 !!! */
etopitembox[1] = -1;
etopitembox[2] = -1;
etopitembox[3] = -1;

topring[0] = -1; /* cut'n'paste rather than a loop, since it's faster */
topring[1] = -1; /* 0s can change to -1 -- see later for details */
topring[2] = -1;
topring[3] = -1;
etopring[0] = -1; /* LEAVE THESE AS -1 !!! */
etopring[1] = -1;
etopring[2] = -1;
etopring[3] = -1;

toptotalring[0] = -1; /* cut'n'paste rather than a loop, since it's faster */
toptotalring[1] = -1; /* 0s can change to -1 -- see later for details */
toptotalring[2] = -1;
toptotalring[3] = -1;
etoptotalring[0] = -1; /* LEAVE THESE AS -1 !!! */
etoptotalring[1] = -1;
etoptotalring[2] = -1;
etoptotalring[3] = -1;

// Start Calculate Score Position First Tails 04-30-2001
for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
{
 if (playeringame[i])
 {
   if (topscore[0]<players[i].score)
    { topscore[0] = players[i].score; etopscore[0] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
{
 if (playeringame[i])
 {
   if (topscore[1]<players[i].score && i != etopscore[0]) /* ignore last player */
   { topscore[1] = players[i].score; etopscore[1] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
{
 if (playeringame[i])
 {
   if (topscore[2]<players[i].score && i != etopscore[0] && i != etopscore[1]) /* ignore last player */
   { topscore[2] = players[i].score; etopscore[2] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
{
 if (playeringame[i])
 {
   if (topscore[3]<players[i].score && i!=etopscore[0] && i!=etopscore[1] && i!=etopscore[2]) /* ignore last player */
   { topscore[3] = players[i].score; etopscore[3] = i; }
 }
}
// End Calculate Score Position First Tails 04-30-2001

// Start Calculate Time Position Tails 04-30-2001
for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
{
 if (playeringame[i])
 {
   if (toptime[0]>players[i].realtime)
    { toptime[0] = players[i].realtime; etoptime[0] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
{
 if (playeringame[i])
 {
   if (toptime[1]>players[i].realtime && i!=etoptime[0]) /* ignore last player */
   { toptime[1] = players[i].realtime; etoptime[1] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
{
 if (playeringame[i])
 {
   if (toptime[2]>players[i].realtime && i!=etoptime[0] && i!=etoptime[1]) /* ignore last player */
   { toptime[2] = players[i].realtime; etoptime[2] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
{
 if (playeringame[i])
 {
   if (toptime[3]>players[i].realtime && i!= etoptime[0] && i!=etoptime[1] && i!=etoptime[2]) /* ignore last player */
   { toptime[3] = players[i].realtime; etoptime[3] = i; }
 }
}
// End Calculate Time Position Tails 04-30-2001

// Start Calculate Item Box Position Tails 04-30-2001
for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
{
 if (playeringame[i])
 {
   if (topitembox[0]<players[i].numboxes)
    { topitembox[0] = players[i].numboxes; etopitembox[0] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
{
 if (playeringame[i])
 {
   if (topitembox[1]<players[i].numboxes && i!=etopitembox[0]) /* ignore last player */
   { topitembox[1] = players[i].numboxes; etopitembox[1] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
{
 if (playeringame[i])
 {
   if (topitembox[2]<players[i].numboxes && i!=etopitembox[0] && i!=etopitembox[1]) /* ignore last player */
     { topitembox[2] = players[i].numboxes; etopitembox[2] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
{
 if (playeringame[i])
 {
   if (topitembox[3]<players[i].numboxes && i!=etopitembox[0] && i!=etopitembox[1] && i!=etopitembox[2]) /* ignore last player */
   { topitembox[3] = players[i].numboxes; etopitembox[3] = i; }
 }
}
// End Calculate Item Box Position Tails 04-30-2001

// Start Calculate Ring Position Tails 04-30-2001
for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
{
 if (playeringame[i])
 {
   if (topring[0]<players[i].health)
    { topring[0] = players[i].health; etopring[0] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
{
 if (playeringame[i])
 {
   if (topring[1]<players[i].health && i!=etopring[0]) /* ignore last player */
   { topring[1] = players[i].health; etopring[1] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
{
 if (playeringame[i])
 {
   if (topring[2]<players[i].health && i!=etopring[0] && i!=etopring[1]) /* ignore last player */
   { topring[2] = players[i].health; etopring[2] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
{
 if (playeringame[i])
 {
   if (topring[3]<players[i].health && i!=etopring[0] && i!=etopring[1] && i!=etopring[2]) /* ignore last player */
   { topring[3] = players[i].health; etopring[3] = i; }
 }
}
// End Calculate Ring Position Tails 04-30-2001

// Start Calculate Total Ring Position Tails 04-30-2001
for (i=0; i<MAXPLAYERS; i++) /* grab highest ranking */
{
 if (playeringame[i])
 {
   if (toptotalring[0]<players[i].totalring)
    { toptotalring[0] = players[i].totalring; etoptotalring[0] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab second ranking */
{
 if (playeringame[i])
 {
   if (toptotalring[1]<players[i].totalring && i!=etoptotalring[0]) /* ignore last player */
   { toptotalring[1] = players[i].totalring; etoptotalring[1] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab third ranking */
{
 if (playeringame[i])
 {
   if (toptotalring[2]<players[i].totalring && i!=etoptotalring[0] && i!=etoptotalring[1]) /* ignore last player */
   { toptotalring[2] = players[i].totalring; etoptotalring[2] = i; }
 }
}

for (i=0; i<MAXPLAYERS; i++) /* grab fourth ranking */
{
 if (playeringame[i])
 {
   if (toptotalring[3]<players[i].totalring && i!=etoptotalring[0] && i!=etoptotalring[1] && i!=etoptotalring[2]) /* ignore last player */
   { toptotalring[3] = players[i].totalring; etoptotalring[3] = i; }
 }
}
// End Calculate Total Ring Position Tails 04-30-2001

	for (i=0; i<MAXPLAYERS; i++)
	{
		if(playeringame[i])
		{

			// Draw "Top 4" if i > 3
			if(i > 3)
				V_DrawString(140, 48, 0, "TOP 4");

		players[i].racescore = 0;

		if((i == etopscore[0]) && !(topscore[0] == topscore[1]))
			players[i].racescore++;
		if((i == etoptime[0]) && !(toptime[0] == toptime[1]))
			players[i].racescore++;
		if((i == etopring[0]) && !(topring[0] == topring[1]))
			players[i].racescore++;
		if((i == etoptotalring[0]) && !(toptotalring[0] == toptotalring[1]))
			players[i].racescore++;
		if((i == etopitembox[0]) && !(topitembox[0] == topitembox[1]))
			players[i].racescore++;

		racerank[0] = -1;
		racerank[1] = - 1;
		racerank[2] = - 1;
		racerank[3] = - 1;
		eracerank[0] = -1;
		eracerank[1] = -1;
		eracerank[2] = -1;
		eracerank[3] = -1;
		}

// Start Calculate Player Position Tails 04-30-2001
for (y=0; y<MAXPLAYERS; y++) // grab highest ranking
{
 if (playeringame[y])
 {
   if (racerank[0]<players[y].racescore)
    { racerank[0] = players[y].racescore; eracerank[0] = y; }
 }
}

for (y=0; y<MAXPLAYERS; y++) // grab second ranking
{
 if (playeringame[y])
 {
   if (racerank[1]<players[y].racescore && y!=eracerank[0]) // ignore last player
   { racerank[1] = players[y].racescore; eracerank[1] = y; }
 }
}

for (y=0; y<MAXPLAYERS; y++) // grab third ranking
{
 if (playeringame[y])
 {
   if (racerank[2]<players[y].racescore && y!=eracerank[0] && y!=eracerank[1]) // ignore last player
   { racerank[2] = players[y].racescore; eracerank[2] = y; }
 }
}

for (y=0; y<MAXPLAYERS; y++) // grab fourth ranking
{
 if (playeringame[y])
 {
   if (racerank[3]<players[y].racescore && y!=eracerank[0] && y!=eracerank[1] && y!=eracerank[2]) // ignore last player
   { racerank[3] = players[y].racescore; eracerank[3] = y; }
 }
}
// End Calculate Player Position Tails 04-30-2001
}
// Now do all the fun score drawing stuff! Tails 05-01-2001
V_DrawScaledPatch(112,8, FB, W_CachePatchName("RESULT", PU_STATIC)); // Tails 05-06-2001
y = 0;
i = eracerank[0];
while(true)
{
	if(!playeringame[i])
		break;

			sprintf(splaynum, "%i", i+1);
			sprintf(sscore, "%i", players[i].score);
			sprintf(sseconds, "%i", ((players[i].realtime/TICRATE) % 60));
			sprintf(sminutes, "%i", players[i].realtime/(60*TICRATE));
			sprintf(stics, "%i", players[i].realtime % TICRATE);

			if(players[i].health > 0)
				sprintf(sring, "%i", players[i].health - 1);
			else
				sprintf(sring, "%i", players[i].health);

			sprintf(stotring, "%i", players[i].totalring);
			sprintf(sitembox, "%i", players[i].numboxes);

			sprintf(setopscore, "%i", etopscore[0]+1);
			sprintf(setoptime, "%i", etoptime[0]+1);
			sprintf(setopitembox, "%i", etopitembox[0]+1);
			sprintf(setopring, "%i", etopring[0]+1);
			sprintf(setoptotalring, "%i", etoptotalring[0]+1);
			sprintf(sracescore, "%i", players[i].racescore);

		x = 104+64*y-8;

		V_DrawString(104-((int)strlen(player_names[i])*8)+64*y,60, 0, player_names[i]);
		V_DrawString(104-((int)strlen(splaynum)*8)+64*y-8,68, 0, splaynum);
		V_DrawString(104-((int)strlen("P")*8)+64*y,68, 0, "P");

		V_DrawString(x-(int)strlen(sscore)*8+8,80, 0, sscore);
		if((players[i].realtime % TICRATE) < 10)
		{
			V_DrawString(x-8, 96, 0, "0");
			V_DrawString(x-(int)strlen(stics)*8+8,96, 0, stics);
		}
		else
		{
		V_DrawString(x-(int)strlen(stics)*8+8,96, 0, stics);
		}
		V_DrawString(x-16,96, 0, ":");
		if(players[i].seconds < 10)
		{
			V_DrawString(x-32, 96, 0, "0");
			V_DrawString(x-24-(int)strlen(sseconds)*8+8,96, 0, sseconds);
		}
		else
		{
		V_DrawString(x-24-(int)strlen(sseconds)*8+8,96, 0, sseconds);
		}
		V_DrawString(x-40,96, 0, ":");
		V_DrawString(x-56-(int)strlen(sminutes)*8+16,96, 0, sminutes);
		V_DrawString(x-(int)strlen(sring)*8+8,112, 0, sring);
		V_DrawString(x-(int)strlen(stotring)*8+8,128, 0, stotring);
		V_DrawString(x-(int)strlen(sitembox)*8+8,144, 0, sitembox);

		V_DrawString(x-(int)strlen(sracescore)*8+8,168, 0, sracescore);

		V_DrawString(304,64, 0, "W");

		// if 0 and 1 are the same, draw TIED instead
		if(topscore[0] == topscore[1])
			V_DrawString(304,80, V_WHITEMAP, "T");
		else
		{
			V_DrawString(300,80, V_WHITEMAP, setopscore);
			V_DrawString(308,80, V_WHITEMAP, "P");
		}
		if(toptime[0] == toptime[1])
			V_DrawString(304,96, V_WHITEMAP, "T");
		else
		{
			V_DrawString(300,96, V_WHITEMAP, setoptime);
			V_DrawString(308,96, V_WHITEMAP, "P");
		}
		if(topring[0] == topring[1])
			V_DrawString(304,112, V_WHITEMAP, "T");
		else
		{
			V_DrawString(300,112, V_WHITEMAP, setopring);
			V_DrawString(308,112, V_WHITEMAP, "P");
		}
		if(toptotalring[0] == toptotalring[1])
			V_DrawString(304,128, V_WHITEMAP, "T");
		else
		{
			V_DrawString(300,128, V_WHITEMAP, setoptotalring);
			V_DrawString(308,128, V_WHITEMAP, "P");
		}
		if(topitembox[0] == topitembox[1])
			V_DrawString(304,144, V_WHITEMAP, "T");
		else
		{
			V_DrawString(300,144, V_WHITEMAP,setopitembox);
			V_DrawString(308,144, V_WHITEMAP, "P");
		}

		if(i == eracerank[3])
			break;
		else
		{
			if(i == eracerank[0])
				i = eracerank[1];
			else if (i == eracerank[1])
				i = eracerank[2];
			else if (i == eracerank[2])
				i = eracerank[3];
			y++;
		}
}

if(players[eracerank[0]].racescore == players[eracerank[1]].racescore)
	V_DrawString(160-((int)strlen("TIED")/2)*8, 184, V_WHITEMAP, "TIED");
else
{
	V_DrawString(160-((int)strlen(player_names[eracerank[0]])/2)*8, 180, V_WHITEMAP, player_names[eracerank[0]]);
	V_DrawString(160-((int)strlen(" WINS")/2)*8, 188, V_WHITEMAP, " WINS");
}
		return;
	}

    // draw stat titles (top line)

	if(players[me].skin == 0)
		V_DrawScaledPatch(88,48, FB, soncpass); // Tails
	else if(players[me].skin == 1)
		V_DrawScaledPatch(88,48, FB, tailpass); // Tails
	else if(players[me].skin == 2)
		V_DrawScaledPatch(40,48, FB, knuxpass); // Tails
	else
		V_DrawScaledPatch(96,48, FB, youpass); // Tails

	V_DrawScaledPatch(72,48, FB, haspassed); // Tails

// Start act selection Tails 11-21-2000
	switch(mapheaderinfo[gamemap-1].actnum)
	{
		case 0:
			nodraw = true;
			break;
		default:
			act = W_CachePatchName (mapheaderinfo[gamemap-1].actnum > 9 ? va("TTL%d",mapheaderinfo[gamemap-1].actnum) : va("TTL0%d",mapheaderinfo[gamemap-1].actnum), PU_CACHE); // Tails 11-01-2000
			break;
	}

	if(!nodraw)
		V_DrawScaledPatch (244, 56, FB, act); // Draw the act numeral Tails 11-21-2000

// End act selection Tails 11-21-2000

    V_DrawScaledPatch(68, 84+lh, FB, items);
    WI_drawPercent(BASEVIDWIDTH - 68, 84+lh, cnt_timebonus[me]); // Time Bonus Tails 03-10-2000

    V_DrawScaledPatch(68, 84+2*lh, FB, sp_secret);
    WI_drawPercent(BASEVIDWIDTH - 68, 84+2*lh, cnt_ringbonus[me]); // Ring Bonus Tails 03-10-2000

    V_DrawScaledPatch(88, 84+4*lh, FB, kills);
    WI_drawPercent(BASEVIDWIDTH - 68, 84+4*lh, cnt_score[me]); // Total Tails 03-10-2000

    V_DrawScaledPatch(16, 10, FB, fscore);
    WI_drawPercent(128, 10, cnt_fscore[me]); // Score Tails 03-10-2000

    V_DrawScaledPatch(17, 26, FB, time);

// Draw the time Tails 11-21-2000
          if(players[me].seconds < 10)
           {
			WI_drawPercent(104, 26, 0);
           }
			WI_drawPercent(112, 26, players[me].seconds);

			WI_drawPercent(88, 26, players[me].minutes);

            V_DrawScaledPatch (88,26, FB, colon); // colon location Tails 02-29-2000
}

static int sp_state;
static int waittime; // Tails 02-20-2002

static void WI_initStats(void)
{
    state = StatCount;
    acceleratestage = 0;
    sp_state = 1;
	if(!(gamemap >= sstage_start && gamemap <= sstage_end))
	{
if (players[me].realtime / TICRATE < 30)
    players[me].timebonus = 50000;

else if (players[me].realtime / TICRATE < 45 && players[me].realtime / TICRATE >= 30)
    players[me].timebonus = 10000;

else if (players[me].realtime / TICRATE < 60 && players[me].realtime / TICRATE >= 45)
    players[me].timebonus = 5000;

else if (players[me].realtime / TICRATE < 90 && players[me].realtime / TICRATE >= 60)
    players[me].timebonus = 4000;

else if (players[me].realtime / TICRATE < 120 && players[me].realtime / TICRATE >= 90)
    players[me].timebonus = 3000;

else if (players[me].realtime / TICRATE < 180 && players[me].realtime / TICRATE >= 120)
    players[me].timebonus = 2000;

else if (players[me].realtime / TICRATE < 240 && players[me].realtime / TICRATE >= 180)
    players[me].timebonus = 1000;

else if (players[me].realtime / TICRATE < 300 && players[me].realtime / TICRATE >= 240)
    players[me].timebonus = 500;

else
	players[me].timebonus = 0;

	if(!modifiedgame)
	{
		if(((unsigned)players[me].realtime < timedata[gamemap-1].time) ||
			(timedata[gamemap-1].time == 0))
			timedata[gamemap-1].time = players[me].realtime;
	}

    cnt_timebonus[0] = (players[me].timebonus); // Time Bonus Tails 03-10-2000
	}
    cnt_fscore[0] = (plrs[me].sscore); // Tails 03-14-2000
    cnt_score[0] = 0; // Score Tails 03-09-2000
    cnt_ringbonus[0] = (100*(players[me].health - 1)); // Ring Bonus Tails 03-10-2000
    cnt_pause = TICRATE;
}

static void WI_updateStats(void)
{
	if(sp_state == 10 && !(grade & 256) && !modifiedgame)
	{
		int seconds = 0;
		int i;

		for(i=0; i<9; i++)
		{
			if(timedata[i].time > 0)
				seconds += timedata[i].time;
			else
				seconds += 800*TICRATE;
		}

		seconds /= TICRATE;

		if(seconds <= 360)
		{
			V_DrawCenteredString(BASEVIDWIDTH/2, 128, 0, "TIME BONUS UNLOCKED!");
			gottimebonus = true;
		}
	}

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
		sp_state = 10;

    if (acceleratestage && sp_state != 10)
    {
        acceleratestage = 0;
        cnt_fscore[me] = (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1))); // Tails 03-14-2000
        cnt_score[me] = (players[me].timebonus + (100*(players[me].health - 1))); // Score Tails 03-09-2000
        cnt_timebonus[me] = 0; // Time Bonus Tails 03-10-2000
        cnt_ringbonus[me] = 0; // Ring Bonus Tails 03-10-2000
// Gives lives for score Tails 04-11-2001
		if(cnt_fscore[me] >= 50000+players[me].xtralife2)
		{
			players[me].lives++;
			if(mariomode)
				S_StartSound(0, sfx_marioa);
			else
			{
				S_StopMusic();
				S_ChangeMusic(mus_xtlife, false);
			}
			players[me].xtralife2 += 50000;
		}
// End score Tails 04-11-2001
        S_StartSound(0, sfx_chchng); // Tails
        sp_state = 10;
    }

    if (sp_state == 2)
    {
	if(!(gamemap >= sstage_start && gamemap <= sstage_end))
	{
			if((cnt_timebonus[me] > 0) && (cnt_ringbonus[me] > 0))
			{
          cnt_fscore[me] += 444; // Tails 03-14-2000
          cnt_score[me] += 444; // Score Tails 03-09-2000
			}
			else
			{
          cnt_fscore[me] += 222; // Tails 03-14-2000
          cnt_score[me] += 222; // Score Tails 03-09-2000
			}
          cnt_timebonus[me] -= 222; // Time Bonus Tails 03-10-2000
          cnt_ringbonus[me] -= 222; // Ring Bonus Tails 03-10-2000

        if (!(bcnt&1)) // count sound faster! Tails 03-10-2000
            S_StartSound(0, sfx_menu1);

// Gives lives for score Tails 04-11-2001
		if(cnt_fscore[me] >= 50000+players[me].xtralife2)
		{
			players[me].lives++;
			if(mariomode)
				S_StartSound(0, sfx_marioa);
			else
			{
				S_StopMusic();
				S_ChangeMusic(mus_xtlife, false);
			}
			players[me].xtralife2 += 50000;
		}
// End score Tails 04-11-2001			  
// start score, time bonus, ring bonus Tails 03-09-2000

        if (cnt_fscore[me] >= (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1))))
        {
            cnt_fscore[me] = (plrs[me].sscore + players[me].timebonus + (100*(players[me].health - 1)));
        }

        if (cnt_score[me] >= (players[me].timebonus + (100*(players[me].health - 1))))
        {
            cnt_score[me] = (players[me].timebonus + (100*(players[me].health - 1)));
        }

        if (cnt_timebonus[me] <= 0)
        {
            cnt_timebonus[me] = 0;
        }

        if (cnt_ringbonus[me] <= 0)
        {
            cnt_ringbonus[me] = 0;
        }

        if ((cnt_ringbonus[me] == 0) && (cnt_timebonus[me] == 0) && (cnt_score[me] == players[me].timebonus + (100*(players[me].health - 1))))
        {
            S_StartSound(0, sfx_chchng); // Tails
			sp_state = 10;
			waittime = 3*TICRATE;
		}
	}
	else
	{
          cnt_fscore[me] += 222; // Tails 03-14-2000
          cnt_ringbonus[me] -= 222; // Ring Bonus Tails 03-10-2000

        if (!(bcnt&1)) // count sound faster! Tails 03-10-2000
            S_StartSound(0, sfx_menu1);

// Gives lives for score Tails 04-11-2001
		if(cnt_fscore[me] >= 50000+players[me].xtralife2)
		{
			players[me].lives++;
			if(mariomode)
				S_StartSound(0, sfx_marioa);
			else
			{
					S_StopMusic();
					S_ChangeMusic(mus_xtlife, false);
			}
			players[me].xtralife2 += 50000;
		}
// End score Tails 04-11-2001			  
// start score, time bonus, ring bonus Tails 03-09-2000

        if (cnt_fscore[me] >= (plrs[me].sscore + (100*(players[me].health - 1))))
        {
            cnt_fscore[me] = (plrs[me].sscore + (100*(players[me].health - 1)));
        }

        if (cnt_ringbonus[me] <= 0)
        {
            cnt_ringbonus[me] = 0;
        }

        if ((cnt_ringbonus[me] == 0) && (cnt_fscore[me] == plrs[me].sscore + (100*(players[me].health - 1))))
        {
            S_StartSound(0, sfx_chchng); // Tails
			sp_state = 10;
			waittime = 3*TICRATE;
		}
	}

    }
    else if (sp_state == 4)
    {
        if (!(bcnt&3))
            S_StartSound(0, sfx_menu1);
    }
    else if (sp_state == 6)
    {
        if (!(bcnt&3))
            S_StartSound(0, sfx_menu1);
    }
    else if (sp_state == 10)
    {
        if (acceleratestage || (--waittime == 0 && !(multiplayer || netgame)))
        {
			if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
			{
				int scorebonus;

				scorebonus = (par - (players[me].health - 2))*100;

				if(scorebonus < 0)
					scorebonus = 0;

				P_AddPlayerScore(&players[me], scorebonus);
			}
			else
				players[me].score = cnt_fscore[me];

                WI_initNoState();
        }
    }
    else if (sp_state & 1)
    {
        if (!--cnt_pause)
        {
            sp_state++;
            cnt_pause = TICRATE;
        }
    }
}

static void WI_drawStats(void)
{
	boolean nodraw = false;
    // line height
    int lh;
	static patch_t* act; // Tails 11-21-2000

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
	{
		int scorebonus;
		V_DrawFill(0,0,vid.width, vid.height, 0);

		V_DrawCenteredString(BASEVIDWIDTH/2, 96, 0, va("Par: %d", par));
		V_DrawCenteredString(BASEVIDWIDTH/2, 112, 0, va("Your Putts: %d", players[me].health-1));
		V_DrawCenteredString(BASEVIDWIDTH/2, 128, V_WHITEMAP, "Score Bonus:");

		scorebonus = (par - (players[me].health - 2))*100;

		if(scorebonus < 0)
			scorebonus = 0;
		V_DrawCenteredString(BASEVIDWIDTH/2, 144, V_WHITEMAP, va("%d", scorebonus));

		return;
	}

    lh = (3*SHORT(num[0]->height))/2;

    WI_slamBackground();

	// Don't show 'Has Passed' stuff in Race Mode Tails 04-27-2001
	if(cv_gametype.value == GT_RACE)
		return;

	if(!(gamemap >= sstage_start && gamemap <= sstage_end))
	{
	if(players[me].skin == 0)
		V_DrawScaledPatch(88,48, FB, soncpass); // Tails
	else if(players[me].skin == 1)
		V_DrawScaledPatch(88,48, FB, tailpass); // Tails
	else if(players[me].skin == 2)
		V_DrawScaledPatch(40,48, FB, knuxpass); // Tails
	else
		V_DrawScaledPatch(96,48, FB, youpass); // Tails

	V_DrawScaledPatch(72,48, FB, haspassed); // Tails

// Start act selection Tails 11-21-2000
	switch(mapheaderinfo[gamemap-1].actnum)
	{
		case 0:
			nodraw = true;
			break;
		default:
			act = W_CachePatchName (mapheaderinfo[gamemap-1].actnum > 9 ? va("TTL%d",mapheaderinfo[gamemap-1].actnum) : va("TTL0%d",mapheaderinfo[gamemap-1].actnum), PU_CACHE); // Tails 11-01-2000
			break;
	}

	if(!nodraw)
		V_DrawScaledPatch (244, 56, FB, act); // Draw the act numeral Tails 11-21-2000

// End act selection Tails 11-21-2000

    V_DrawScaledPatch(68, 84+lh, FB, items);
    WI_drawPercent(BASEVIDWIDTH - 68, 84+lh, cnt_timebonus[0]); // Time Bonus Tails 03-10-2000

    V_DrawScaledPatch(68, 84+2*lh, FB, sp_secret);
    WI_drawPercent(BASEVIDWIDTH - 68, 84+2*lh, cnt_ringbonus[0]); // Ring Bonus Tails 03-10-2000

    V_DrawScaledPatch(88, 84+4*lh, FB, kills);
    WI_drawPercent(BASEVIDWIDTH - 68, 84+4*lh, cnt_score[0]); // Total Tails 03-10-2000

    V_DrawScaledPatch(16, 10, FB, fscore);
    WI_drawPercent(128, 10, cnt_fscore[0]); // Score Tails 03-10-2000

    V_DrawScaledPatch(17, 26, FB, time);

	if(cv_timetic.value) // new TimeTic option Tails 04-02-2001
	{
		WI_drawPercent(112, 26, players[me].realtime);
	}
	else
	{
          if(players[me].seconds < 10)
           {
			WI_drawPercent(104, 26, 0);
           }
			WI_drawPercent(112, 26, players[me].seconds);

			WI_drawPercent(88, 26, players[me].minutes);

            V_DrawScaledPatch (88,26, FB, colon); // colon location Tails 02-29-2000
	}
	}
	else // Special Stage Drawer Tails 08-12-2001
	{
		// Draw big CHAOS EMERALDS thing
		if(emeralds & EMERALD1 && emeralds & EMERALD2 && emeralds & EMERALD3 && emeralds & EMERALD4 && emeralds & EMERALD5 && emeralds & EMERALD6 && emeralds & EMERALD7)
		{
			if(sp_state == 10 && players[me].skin == 0)
				V_DrawScaledPatch(48, 32, FB, nowsuper);
			else
				V_DrawScaledPatch(70, 26, FB, gotemall);
		}
		else
			V_DrawScaledPatch(48, 26, FB, cemerald);

		if(blinker & 1)
		{
			// Display Chaos Emeralds collected
			if(emeralds & EMERALD1)
				V_DrawScaledPatch(80, 92, FB, chaos1);
			if(emeralds & EMERALD2)
				V_DrawScaledPatch(104, 92, FB, chaos2);
			if(emeralds & EMERALD3)
				V_DrawScaledPatch(128, 92, FB, chaos3);
			if(emeralds & EMERALD4)
				V_DrawScaledPatch(152, 92, FB, chaos4);
			if(emeralds & EMERALD5)
				V_DrawScaledPatch(176, 92, FB, chaos5);
			if(emeralds & EMERALD6)
				V_DrawScaledPatch(200, 92, FB, chaos6);
			if(emeralds & EMERALD7)
				V_DrawScaledPatch(224, 92, FB, chaos7);
		}

    V_DrawScaledPatch(80, 132, FB, sp_secret);
    WI_drawPercent(232, 132, cnt_ringbonus[0]); // Ring Bonus Tails 03-10-2000

    V_DrawScaledPatch(80, 148, FB, cscore);
    WI_drawPercent(232, 148, cnt_fscore[0]); // Score Tails 03-10-2000
	}
	
	blinker++; // Tails 08-12-2001
}

static void WI_checkForAccelerate(void)
{
    int   i;
    player_t  *player;

    // check for button presses to skip delays
    for (i=0, player = players ; i<MAXPLAYERS ; i++, player++)
    {
        if (playeringame[i])
        {
            if (player->cmd.buttons & BT_JUMP) // Tails 07-20-2001
            {
                if (!player->jumpdown) // Tails 07-20-2001
                    acceleratestage = 1;
                player->jumpdown = true; // Tails 07-20-2001
            }
            else
                player->jumpdown = false; // Tails 07-20-2001
            if (player->cmd.buttons & BT_USE)
            {
                if (!player->usedown)
                    acceleratestage = 1;
                player->usedown = true;
            }
            else
                player->usedown = false;
        }
    }
}



// Updates stuff each tick
void WI_Ticker(void)
{
    // counter for general background animation
    bcnt++;

    if (bcnt == 1)
    {
        // intermission music
          S_ChangeMusic(mus_lclear, false); // Tails 03-14-2000
    }

    WI_checkForAccelerate();

    switch (state)
    {
      case StatCount:
        if (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CHAOS || cv_gametype.value == GT_CIRCUIT) WI_updateDeathmatchStats(); // Deathmatch intermission Tails 02-20-2002
        else if (multiplayer) WI_updateNetgameStats();
        else WI_updateStats();
        break;

      case NoState:
        WI_updateNoState();
        break;

	  default:
		  break;
    }

}

static void WI_loadData(void)
{
    int         i;
    char        name[9];

	if(rendermode == render_soft)
		wipe_StartScreen(0, 0, vid.width, vid.height);

	V_DrawFill(0,0, vid.width, vid.height, 0);

	if(rendermode == render_soft)
	{
		tic_t                       nowtime;
		tic_t                       tics;
		tic_t                       wipestart;
		int                         y;
		boolean                     done;

		wipe_EndScreen(0, 0, vid.width, vid.height);

		wipestart = I_GetTime () - 1;
		y=wipestart+TICRATE; // init a timeout
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

    // choose the background of the intermission
	if(cv_gametype.value != GT_COOP) // Tails 04-27-2001
	{
		strcpy(bgname, "SRB2BACK"); // Tails 04-27-2001
	}
	else
	{
		if(gamemap >= sstage_start && gamemap <= sstage_end)
			strcpy(bgname, "SPECIALB"); // Tails 08-11-2001
		else
		{
			if(mapheaderinfo[gamemap-1].interscreen[0] != '#')
				strcpy(bgname, va("%s", mapheaderinfo[gamemap-1].interscreen));
			else
				strcpy(bgname, "INTERSCR"); // Tails 12-02-99
		}
	}

    if (rendermode == render_soft)
    {
        memset(screens[0],0,vid.width*vid.height*vid.bpp);

        // clear backbuffer from status bar stuff and borders
        memset(screens[1],0,vid.width*vid.height*vid.bpp);
  
        // background stored in backbuffer        
        V_DrawScaledPatch(0, 0, 1, W_CachePatchName(bgname, PU_CACHE));
    }

    for (i=0;i<10;i++)
    {
        sprintf(name, "WINUM%d", i);
        num[i] = W_CachePatchName(name, PU_STATIC);
    }

	cscore = W_CachePatchName("CSCORE", PU_STATIC); // Tails 08-11-2001

	gotemall = W_CachePatchName("GOTEMALL", PU_STATIC); // Tails 08-12-2001
	cemerald = W_CachePatchName("CEMERALD", PU_STATIC); // Tails 08-12-2001
	nowsuper = W_CachePatchName("NOWSUPER", PU_STATIC); // Tails 08-12-2001
	chaos1 = W_CachePatchName("CHAOS1", PU_STATIC); // Tails 08-12-2001
	chaos2 = W_CachePatchName("CHAOS2", PU_STATIC); // Tails 08-12-2001
	chaos3 = W_CachePatchName("CHAOS3", PU_STATIC); // Tails 08-12-2001
	chaos4 = W_CachePatchName("CHAOS4", PU_STATIC); // Tails 08-12-2001
	chaos5 = W_CachePatchName("CHAOS5", PU_STATIC); // Tails 08-12-2001
	chaos6 = W_CachePatchName("CHAOS6", PU_STATIC); // Tails 08-12-2001
	chaos7 = W_CachePatchName("CHAOS7", PU_STATIC); // Tails 08-12-2001
        
    // "kills"
    kills = W_CachePatchName("WIOSTK", PU_STATIC);

    fscore = W_CachePatchName("SBOFRAGS", PU_STATIC); // Tails 03-14-2000
        
    // "scrt"
    secret = W_CachePatchName("WIOSTS", PU_STATIC);
        
    // "secret"
    sp_secret = W_CachePatchName("WISCRT2", PU_STATIC);
        
    // "items"
    items = W_CachePatchName("WIOSTI", PU_STATIC);
        
    // "frgs"
    frags = W_CachePatchName("WIFRGS", PU_STATIC);
        
    // "time"
    time = W_CachePatchName("SBOARMOR", PU_STATIC); // Tails
    
    // ":"
    colon = W_CachePatchName("WICOLON", PU_STATIC);

	haspassed = W_CachePatchName("HPASSED", PU_STATIC); // Tails
	soncpass = W_CachePatchName("SONCPASS", PU_STATIC); // Tails
	knuxpass = W_CachePatchName("KNUXPASS", PU_STATIC); // Tails
	tailpass = W_CachePatchName("TAILPASS", PU_STATIC); // Tails
	youpass = W_CachePatchName("YOUPASS", PU_STATIC); // Tails
}

static void WI_unloadData(void)
{
    int         i;

    //faB: never Z_ChangeTag() a pointer returned by W_CachePatchxxx()
    //     it doesn't work and is unecessary
    if (rendermode==render_soft)
    {
		for (i=0 ; i<10 ; i++)
			Z_ChangeTag(num[i], PU_CACHE);
    }

    if (rendermode==render_soft)
    {
        Z_ChangeTag(colon, PU_CACHE);

		Z_ChangeTag(cscore, PU_CACHE); // Tails 08-11-2001
		Z_ChangeTag(cemerald, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(gotemall, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(nowsuper, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos1, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos2, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos3, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos4, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos5, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos6, PU_CACHE); // Tails 08-12-2001
		Z_ChangeTag(chaos7, PU_CACHE); // Tails 08-12-2001
        Z_ChangeTag(kills, PU_CACHE);
		Z_ChangeTag(fscore, PU_CACHE); // Oops! Forgot this! Tails 08-11-2001
        Z_ChangeTag(secret, PU_CACHE);
        Z_ChangeTag(sp_secret, PU_CACHE);
        Z_ChangeTag(items, PU_CACHE);
        Z_ChangeTag(frags, PU_CACHE);
        Z_ChangeTag(time, PU_CACHE);

		// Extra ChangeTags Tails 08-11-2001
		Z_ChangeTag(haspassed, PU_CACHE);
		Z_ChangeTag(soncpass, PU_CACHE);
		Z_ChangeTag(knuxpass, PU_CACHE);
		Z_ChangeTag(tailpass, PU_CACHE);
		Z_ChangeTag(youpass, PU_CACHE);
    }
}

void WI_Drawer (void)
{
    switch (state)
    {
      case StatCount:
        if (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CHAOS) // Deathmatch intermission Tails 02-20-2002
        {
            WI_drawDeathmatchStats();
        }
		else if(cv_gametype.value == GT_CIRCUIT)
			WI_drawCircuitStats();
        else if (multiplayer)
            WI_drawNetgameStats();
        else
            WI_drawStats();
        break;

      case NoState:
        WI_drawNoState();
        break;

	  default:
		break;
    }
}


static void WI_initVariables(wbstartstruct_t* wbstartstruct)
{

    wbs = wbstartstruct;

#ifdef RANGECHECKING
    RNGCHECK(wbs->last, 0, 8);
    RNGCHECK(wbs->next, 0, 8);

    RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
    RNGCHECK(wbs->pnum, 0, MAXPLAYERS);
#endif

    acceleratestage = 0;
    cnt = bcnt = 0;
    me = wbs->pnum;
    plrs = wbs->plyr;

    if (!wbs->maxkills)
        wbs->maxkills = 1;

    if (!wbs->maxitems)
        wbs->maxitems = 1;

    if (!wbs->maxsecret)
        wbs->maxsecret = 1;

    if (wbs->epsd > 2)
        wbs->epsd -= 3;
}

void WI_Start(wbstartstruct_t* wbstartstruct)
{
	blinker = 0; // Tails 08-17-2001
	gottimebonus = false;

    WI_initVariables(wbstartstruct);
    WI_loadData();

    if (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CHAOS) // Deathmatch intermission Tails 02-20-2002
        WI_initDeathmatchStats();
	else if(cv_gametype.value == GT_CIRCUIT)
		WI_initCircuitStats();
    else if (multiplayer)
        WI_initNetgameStats();
    else
        WI_initStats();
}
