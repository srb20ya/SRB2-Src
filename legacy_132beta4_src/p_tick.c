// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_tick.c,v 1.6 2001/01/25 22:15:44 bpereira Exp $
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
// $Log: p_tick.c,v $
// Revision 1.6  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.5  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.4  2000/10/21 08:43:31  bpereira
// no message
//
// Revision 1.3  2000/10/08 13:30:01  bpereira
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
//      Archiving: SaveGame I/O.
//      Thinker, Ticker.
//
//-----------------------------------------------------------------------------


#include "doomstat.h"
#include "g_game.h"
#include "p_local.h"
#include "z_zone.h"
#include "t_script.h"
#include "s_sound.h"

tic_t     leveltime;
int verifyplayer = 4242; // Tails 10-05-2003

//
// THINKERS
// All thinkers should be allocated by Z_Malloc
// so they can be operated on uniformly.
// The actual structures will vary in size,
// but the first element must be thinker_t.
//



// Both the head and tail of the thinker list.
thinker_t       thinkercap;


//
// P_InitThinkers
//
void P_InitThinkers (void)
{
    thinkercap.prev = thinkercap.next  = &thinkercap;
}




//
// P_AddThinker
// Adds a new thinker at the end of the list.
//
void P_AddThinker (thinker_t* thinker)
{
    thinkercap.prev->next = thinker;
    thinker->next = &thinkercap;
    thinker->prev = thinkercap.prev;
    thinkercap.prev = thinker;
}



//
// P_RemoveThinker
// Deallocation is lazy -- it will not actually be freed
// until its thinking turn comes up.
//
void P_RemoveThinker (thinker_t* thinker)
{
  // FIXME: NOP.
  thinker->function.acv = (actionf_v)(-1);
}



//
// P_AllocateThinker
// Allocates memory and adds a new thinker at the end of the list.
//
void P_AllocateThinker (thinker_t*      thinker)
{
}



//
// P_RunThinkers
//
void P_RunThinkers (void)
{
    thinker_t*  currentthinker;

    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
        if ( currentthinker->function.acv == (actionf_v)(-1) )
        {
            void *removeit;
            // time to remove it
            currentthinker->next->prev = currentthinker->prev;
            currentthinker->prev->next = currentthinker->next;
            removeit = currentthinker;
            currentthinker = currentthinker->next;
            Z_Free (removeit);
        }
        else
        {
			if (currentthinker->function.acp1)
                currentthinker->function.acp1 (currentthinker);
            currentthinker = currentthinker->next;
        }
    }
}

boolean circintrodone = false; // Graue 12-24-2003

//
// P_Ticker
//

void P_Ticker (void)
{
    int         i;

//	Z_CheckHeap(-42);

	if(server && verifyplayer != 4242 && verifyplayer < MAXPLAYERS)
	{
		CONS_Printf("Verified...\n");
		COM_BufAddText(va("verify %d\n", verifyplayer));
		verifyplayer = 4242;
	}

	if(server && verifyplayer >= 100 && verifyplayer != 4242)
	{
		verifyplayer -= 100;
	}

    // run the tic
    if (paused || (!netgame && menuactive && !demoplayback))
        return;

    for (i=0 ; i<MAXPLAYERS ; i++)
        if (playeringame[i])
            P_PlayerThink (&players[i]);

	// Keep track of how long they've been playing! Tails 12-08-2002
	if(!(netgame || multiplayer))
		totalplaytime++;

///////////////////////
//SPECIAL STAGE STUFF//
///////////////////////

	if(gamemap >= sstage_start && gamemap <= sstage_end)
	{
		boolean inwater;
		inwater = false;

		// Can't drown in a special stage Tails 07-04-2002
		for(i=0;i<MAXPLAYERS;i++)
		{
			if(!playeringame[i])
				continue;
			
			if(players[i].powers[pw_underwater] != 0)
				players[i].powers[pw_underwater] = 0;

			if(players[i].powers[pw_spacetime] != 0)
				players[i].powers[pw_spacetime] = 0;
		}

		if(sstimer < 7 && sstimer > 0) // The special stage time is up! Tails 08-11-2001
		{
			sstimer = 0;
			for(i=0;i<MAXPLAYERS;i++)
			{
				if(playeringame[i])
				{
					players[i].exiting = 2.8*TICRATE + 1;
					players[i].gliding = 0;
				}

				if(i == consoleplayer)
					S_StartSound(0, sfx_lose);
			}
		}

		if(sstimer > 1) // As long as time isn't up...
		{
			int ssrings = 0;
			// Count up the rings of all the players and see if
			// they've collected the required amount.
			for(i=0; i<MAXPLAYERS; i++)
			{
				if(playeringame[i])
					ssrings += (players[i].mo->health-1);
			}

			if(ssrings >= totalrings)
			{
				S_StartSound(0, sfx_cgot); // Got the emerald!

				// Halt all the players
				for(i=0;i<MAXPLAYERS;i++)
				{
					if(playeringame[i])
					{
						players[i].mo->momx = players[i].mo->momy = 0;
						players[i].exiting = 2.8*TICRATE + 1;
						sstimer = 0;
					}
				}

				// Check what emeralds the player has so you know which one to award next.
				if(!(emeralds & EMERALD1))
				{
					emeralds |= EMERALD1;
					for(i=0;i<MAXPLAYERS;i++)
					{
						if(playeringame[i])
							P_SpawnMobj(players[i].mo->x, players[i].mo->y, players[i].mo->z + players[i].mo->info->height, MT_GREENEMERALD);
					}
				}
				else if((emeralds & EMERALD1) && !(emeralds & EMERALD2))
				{
					emeralds |= EMERALD2;
					for(i=0;i<MAXPLAYERS;i++)
					{
						if(playeringame[i])
							P_SpawnMobj(players[i].mo->x, players[i].mo->y, players[i].mo->z + players[i].mo->info->height, MT_ORANGEEMERALD);
					}
				}
				else if((emeralds & EMERALD2) && !(emeralds & EMERALD3))
				{
					emeralds |= EMERALD3;
					for(i=0;i<MAXPLAYERS;i++)
					{
						if(playeringame[i])
							P_SpawnMobj(players[i].mo->x, players[i].mo->y, players[i].mo->z + players[i].mo->info->height, MT_PINKEMERALD);
					}
				}
				else if((emeralds & EMERALD3) && !(emeralds & EMERALD4))
				{
					emeralds |= EMERALD4;
					for(i=0;i<MAXPLAYERS;i++)
					{
						if(playeringame[i])
							P_SpawnMobj(players[i].mo->x, players[i].mo->y, players[i].mo->z + players[i].mo->info->height, MT_BLUEEMERALD);
					}
				}
				else if((emeralds & EMERALD4) && !(emeralds & EMERALD5))
				{
					emeralds |= EMERALD5;
					for(i=0;i<MAXPLAYERS;i++)
					{
						if(playeringame[i])
							P_SpawnMobj(players[i].mo->x, players[i].mo->y, players[i].mo->z + players[i].mo->info->height, MT_REDEMERALD);
					}
				}
				else if((emeralds & EMERALD5) && !(emeralds & EMERALD6))
				{
					emeralds |= EMERALD6;
					for(i=0;i<MAXPLAYERS;i++)
					{
						if(playeringame[i])
							P_SpawnMobj(players[i].mo->x, players[i].mo->y, players[i].mo->z + players[i].mo->info->height, MT_LIGHTBLUEEMERALD);
					}
				}
				else if((emeralds & EMERALD6) && !(emeralds & EMERALD7))
				{
					emeralds |= EMERALD7;
					for(i=0;i<MAXPLAYERS;i++)
					{
						if(playeringame[i])
							P_SpawnMobj(players[i].mo->x, players[i].mo->y, players[i].mo->z + players[i].mo->info->height, MT_GREYEMERALD);
					}
				}
			}

			for(i=0;i<MAXPLAYERS;i++)
			{
				if(playeringame[i])
				{
					// If in water, deplete timer 3x as fast.
					if((players[i].mo->z <= players[i].mo->watertop
						&& players[i].mo->z + players[i].mo->height >= players[i].mo->watertop) // Feet in the water, but head above
						|| (players[i].mo->z + players[i].mo->height > players[i].mo->waterbottom
						&& players[i].mo->z <= players[i].mo->waterbottom) // player's head in water, but feet not (defies natural physics?!?!)
						|| (players[i].mo->z >= players[i].mo->waterbottom
						&& players[i].mo->z + players[i].mo->height <= players[i].mo->watertop)) // player is ENTIRELY in water
					{
						inwater = true;
					}
				}
			}
			
			// Decrement the timer
			if(inwater)
			{
				sstimer--; // No idea why player->sstimer -= 3; doesn't work here...
				sstimer--;
				sstimer--;
				sstimer--;
				sstimer--;
				sstimer--;
			}
			else
				sstimer--;
		}
	}
/////////////////////////////////////////

    P_RunThinkers ();
    P_UpdateSpecials ();
    P_RespawnSpecials ();
//    P_AmbientSound();

    // for par times
    leveltime++;

#ifdef CIRCUITMODE
	// Graue 12-24-2003 and 12-29-2003
	if(!circintrodone) // This implies cv_gametype.value == GT_CIRCUIT
	{
		if(leveltime == 3*CIRCINTROTIME)
		{
			for(i=0;i<MAXPLAYERS;i++)
				if(playeringame[i])
					players[i].exiting = 0;
			leveltime = 0;
			circintrodone = true;
		}

		if(leveltime == 2*CIRCINTROTIME+1 || leveltime == CIRCINTROTIME+1 || leveltime == 1)
			S_StartSound(0, sfx_shotgn);
	}
	else if(leveltime == 1 && cv_gametype.value == GT_CIRCUIT)
		S_StartSound(0, sfx_shotgn);
#endif

	if(countdowntimer)
	{
		countdowntimer--;
		if(countdowntimer <= 0)
		{
			int i;
			countdowntimer = 0;
			countdowntimeup = true;
			for(i=0; i<MAXPLAYERS; i++)
			{
				if(!playeringame[i])
					continue;

				if(!players[i].mo)
					continue;

				P_DamageMobj(players[i].mo, NULL, NULL, 10000);
			}

		}
	}

	if(countdown)
		countdown--;

	if(countdown2)
		countdown2--;

	if(playerchangedelay)
		playerchangedelay--;

#ifdef FRAGGLESCRIPT
    // SoM: Update FraggleScript...
    T_DelayedScripts();
#endif
}
