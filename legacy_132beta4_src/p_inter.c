// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_inter.c,v 1.23 2001/12/26 22:46:01 hurdler Exp $
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
// $Log: p_inter.c,v $
// Revision 1.23  2001/12/26 22:46:01  hurdler
// revert to beta 3 until it's fixed (there is at least a problem with saved game)
//
// Revision 1.22  2001/12/26 22:42:52  hurdler
// revert to beta 3 until it's fixed (there is at least a problem with saved game)
//
// Revision 1.18  2001/06/10 21:16:01  bpereira
// no message
//
// Revision 1.17  2001/05/27 13:42:47  bpereira
// no message
//
// Revision 1.16  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.15  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.14  2001/04/19 05:51:47  metzgermeister
// fixed 10 shells instead of 4 - bug
//
// Revision 1.13  2001/03/30 17:12:50  bpereira
// no message
//
// Revision 1.12  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.11  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.10  2000/11/02 17:50:07  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.9  2000/10/02 18:25:45  bpereira
// no message
//
// Revision 1.8  2000/10/01 10:18:17  bpereira
// no message
//
// Revision 1.7  2000/09/28 20:57:16  bpereira
// no message
//
// Revision 1.6  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.5  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.4  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      Handling interactions (i.e., collisions).
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "i_system.h"   //I_Tactile currently has no effect
#include "am_map.h"
#include "dstrings.h"
#include "g_game.h"
#include "m_random.h"
#include "p_local.h"
#include "p_inter.h"
#include "s_sound.h"
#include "r_main.h"
#include "st_stuff.h"

#define BONUSADD        6

//Prototype Tails
void P_ResetPlayer(player_t* player);
void P_PlayerRingBurst(player_t* player, int num_rings); // Added num_rings param Graue 12-06-2003
void P_Thrust ( mobj_t*       mo,
                angle_t       angle,
                fixed_t       move );
void P_InstaThrust ( mobj_t*       mo,
					angle_t       angle,
					fixed_t       move );
void I_PlayCD ();
void P_PlayerFlagBurst(player_t* player);
void P_AddPlayerScore(player_t* player, int amount);
// end protos Tails

//--------------------------------------------------------------------------
//
// PROC P_SetMessage
//
//--------------------------------------------------------------------------

boolean ultimatemsg;

void P_SetMessage(player_t *player, char *message, boolean ultmsg)
{
    if((ultimatemsg || !cv_showmessages.value) && !ultmsg)
        return;
    
    player->message = message;
    //player->messageTics = MESSAGETICS;
    //BorderTopRefresh = true;
    if( ultmsg )
        ultimatemsg = true;
}

//
// GET STUFF
//

//
// P_ClearStarPost
//
// Makes sure all previous star posts are cleared.
//
// Tails 07-05-2002
void P_ClearStarPost (player_t* player, int postnum)
{
    thinker_t*  th;
    mobj_t*     mo2;

	// In circuit mode, don't bother Graue 12-13-2003
	if(cv_gametype.value == GT_CIRCUIT)
		return;

    // scan the remaining thinkers
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;

        mo2 = (mobj_t *)th;

		if(mo2->type == MT_STARPOST)
		{
			if(mo2->health < postnum)
			{
				P_SetMobjState(mo2, S_STARPOST2);
				player->starpostbit |= (1<<(mo2->health-1)); // Removes need for switch() Graue 11-18-2003
			}
		}
    }
	return;
}

void P_SpawnHoopOfSomething(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, int number, mobjtype_t type, int rotangle);
void P_SpawnParaloop(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, int number, mobjtype_t type, int rotangle);
void P_NightserizePlayer(player_t* player, int time, boolean nextmare);
extern consvar_t cv_objectplace;
extern consvar_t cv_timeattacked;
void P_DoPlayerExit(player_t* player);
boolean P_TransferToNextMare(player_t* player);
//
// P_TouchSpecialThing
//
void P_TouchSpecialThing ( mobj_t*       special,
                           mobj_t*       toucher,
						   boolean   heightcheck )
{                  
    player_t*   player;
//    int         i;
    int         sound;
	int emercount; // Tails 12-12-2001
	int i; // Tails 12-12-2001
	mobj_t*   dummymo;
	emercount = 0; // Tails 12-12-2001

	if(cv_objectplace.value)
		return;

// Don't do this Tails 10-31-2000
/*    delta = special->z - toucher->z;

	//SoM: 3/27/2000: For some reason, the old code allowed the player to
    //grab items that were out of reach...
    if (delta > toucher->height
        || delta < -special->height)
    {
        // out of reach
        return;
    }
*/
    // Dead thing touching.
    // Can happen with a sliding player corpse.
    if (toucher->health <= 0)
        return;

	if(heightcheck)
	{
		if(toucher->z > (special->z + special->height))
			return;
		if(special->z > (toucher->z + toucher->height))
			return;
	}

	if(special->health<=0)
		return;

    sound = sfx_itemup;
    player = toucher->player;

	if(!player) // Only players can touch stuff!
		return;

	if(special->state == &states[S_DISS]) // Don't collect if in "disappearing" mode Tails 04-29-2001
		return;

	// Ignore eggman in "ouchie" mode
	if((special->flags & MF_BOSS) && (special->flags2 & MF2_FRET))
	{
		return;
	}



	if(special->type == MT_EGGMOBILE2)
	{
		if(special->movecount)
			return;
	}

	if(special->flags & MF_BOSS)
	{
		  if((toucher->z <= special->z + special->height && toucher->z + toucher->height >= special->z) // Are you touching the side of it?
			&& ((toucher->player->nightsmode && toucher->player->drilling) || toucher->player->mfjumped || toucher->player->mfspinning
			|| toucher->player->powers[pw_invulnerability] || toucher->player->powers[pw_super])) // Do you possess the ability to subdue the object?
		  {
			if(toucher->momz < 0)
				toucher->momz = -toucher->momz;
				toucher->momx = -toucher->momx;
				toucher->momy = -toucher->momy;
			P_DamageMobj(special, toucher, toucher, 1);
			return;
		  }
		  else if(toucher->z + toucher->height >= special->z
			  && toucher->z < special->z
			  && toucher->player->charability == 1
			  && (toucher->player->powers[pw_tailsfly]
			  || toucher->state == &states[S_PLAY_SPC1]
			  || toucher->state == &states[S_PLAY_SPC2]
			  || toucher->state == &states[S_PLAY_SPC3]
			  || toucher->state == &states[S_PLAY_SPC4])) // Tails can shred stuff with his propeller.
		  {
				toucher->momz = -toucher->momz/2;

			P_DamageMobj(special, toucher, toucher, 1);
			return;
		  }
		  else
			  P_DamageMobj(toucher, special, special, 1);
		  return;
	}
	else if((special->flags & MF_ENEMY)
		&& !(special->flags & MF_MISSILE))
	{
	  ////////////////////////////////////////////////////////
	  /////ENEMIES!!//////////////////////////////////////////
	  ////////////////////////////////////////////////////////
		  if((toucher->z <= special->z + special->height && toucher->z + toucher->height >= special->z) // Are you touching the side of it?
			&& ((toucher->player->nightsmode && toucher->player->drilling) || toucher->player->mfjumped || toucher->player->mfspinning
			|| toucher->player->powers[pw_invulnerability] || toucher->player->powers[pw_super])) // Do you possess the ability to subdue the object?
		  {
			if(toucher->momz < 0)
				toucher->momz = -toucher->momz;
			P_DamageMobj(special, toucher, toucher, 1);
			return;
		  }
		  else if(toucher->z + toucher->height >= special->z
			  && toucher->z < special->z
			  && toucher->player->charability == 1
			  && (toucher->player->powers[pw_tailsfly]
			  || toucher->state == &states[S_PLAY_SPC1]
			  || toucher->state == &states[S_PLAY_SPC2]
			  || toucher->state == &states[S_PLAY_SPC3]
			  || toucher->state == &states[S_PLAY_SPC4])) // Tails can shred stuff with his propeller.
		  {
			if(toucher->momz < 0)
				toucher->momz = -toucher->momz/2;

			P_DamageMobj(special, toucher, toucher, 1);
			return;
		  }
		  else
			  P_DamageMobj(toucher, special, special, 1);
		  return;
	}
	else
	{
	// We now identify by object type, not sprite! Tails 04-11-2001
    switch (special->type)
    {
	case MT_EMBLEM: // Secret emblem thingy
		P_SetMobjState(special, S_DISS);
		P_SpawnMobj(special->x, special->y, special->z, MT_SPARK);
		S_StartSound(toucher, sfx_ncitem);
		gottenemblems |= special->health;
		return;
	case MT_EASTEREGG: // Easter Egg!
		P_SetMobjState(special, S_DISS);
		P_SpawnMobj(special->x, special->y, special->z, MT_SPARK);
		S_StartSound(toucher, sfx_ncitem);
		foundeggs |= special->health;
		if(foundeggs == 4095)
			grade |= 512;
		return;
	case MT_EGGCAPSULE:
		if(!(toucher->player->health > 1))
			return;

		if(toucher->player->nightsmode && !(toucher->target))
			return;

		if(toucher->player->mare != special->threshold)
			return;

		// Mark the player as 'pull into the capsule'
		toucher->player->capsule = special;
		toucher->player->capsule->reactiontime = 1;
		return;
	case MT_NIGHTSSUPERLOOP:
		player->powers[pw_superparaloop] = paralooptics;
		S_StartSound(toucher, sfx_ncspec);
		P_SetMobjState(special, S_DISS);
		return;
	case MT_NIGHTSDRILLREFILL:
		player->drillmeter = 96*20;
		S_StartSound(toucher, sfx_ncspec);
		P_SetMobjState(special, S_DISS);
		return;
	case MT_NIGHTSHELPER:
		player->powers[pw_nightshelper] = helpertics;
		S_StartSound(toucher, sfx_ncspec);
		P_SetMobjState(special, S_DISS);
		return;
	case MT_NIGHTSWING:
		P_SpawnMobj(special->x, special->y, special->z, MT_SPARK);
		player->linkcount++;
		player->linktimer = 2*TICRATE;

		S_StartSound(toucher, sfx_ncitem);

		dummymo = P_SpawnMobj(special->x, special->y, special->z, MT_NIGHTSCORE);

		if(player->linkcount < 10)
		{
			if(player->bonustime)
			{
				P_AddPlayerScore(player, player->linkcount*20);
				P_SetMobjState(dummymo, dummymo->info->xdeathstate+player->linkcount-1);
			}
			else
			{
				P_AddPlayerScore(player, player->linkcount*10);
				P_SetMobjState(dummymo, dummymo->info->spawnstate+player->linkcount-1);
			}
		}
		else
		{
			if(player->bonustime)
			{
				P_AddPlayerScore(player, 200);
				P_SetMobjState(dummymo, dummymo->info->xdeathstate+9);
			}
			else
			{
				P_AddPlayerScore(player, 100);
				P_SetMobjState(dummymo, dummymo->info->spawnstate+9);
			}
		}
		player->drillmeter += TICRATE;
		dummymo->momz = FRACUNIT;
		dummymo->fuse = 3*TICRATE;
		P_SetMobjState(special, S_DISS);
		special->flags &= ~MF_SPECIAL;
		if(toucher->target)
		{
			if(toucher->target->flags & MF_AMBUSH)
				P_InstaThrust(dummymo, R_PointToAngle2(dummymo->x, dummymo->y, toucher->target->x, toucher->target->y), 3*FRACUNIT);
			else
				P_InstaThrust(dummymo, R_PointToAngle2(toucher->target->x, toucher->target->y, dummymo->x, dummymo->y), 3*FRACUNIT);
		}
		return;
	case MT_HOOPCOLLIDE:
		// This produces a kind of 'domino effect' with the hoop's pieces.
		for(; special->bprev != NULL; special = special->bprev); // Move to the first sprite in the hoop
		i = 0;
		for(; special->type == MT_HOOP; special = special->bnext)
		{
			special->fuse = 11;
			special->movedir = i;
			i++;
		}
		// Make the collision detectors disappear.
		for(; special != NULL; special = special->bnext)
		{
			special->flags &= ~MF_SPECIAL;
			P_RemoveMobj(special);
		}
		// Play hoop sound -- pick one depending on the current link.
		player->linkcount++;
		player->linktimer = 2*TICRATE;

		if(player->linkcount < 5)
			S_StartSound(toucher, sfx_hoop1);
		else if(player->linkcount < 10)
			S_StartSound(toucher, sfx_hoop2);
		else
			S_StartSound(toucher, sfx_hoop3);

		dummymo = P_SpawnMobj(toucher->x, toucher->y, toucher->z, MT_NIGHTSCORE);

		if(player->linkcount < 10)
		{
			if(player->bonustime)
			{
				P_AddPlayerScore(player, player->linkcount*20);
				P_SetMobjState(dummymo, dummymo->info->xdeathstate+player->linkcount-1);
			}
			else
			{
				P_AddPlayerScore(player, player->linkcount*10);
				P_SetMobjState(dummymo, dummymo->info->spawnstate+player->linkcount-1);
			}
		}
		else
		{
			if(player->bonustime)
			{
				P_AddPlayerScore(player, 200);
				P_SetMobjState(dummymo, dummymo->info->xdeathstate+9);
			}
			else
			{
				P_AddPlayerScore(player, 100);
				P_SetMobjState(dummymo, dummymo->info->spawnstate+9);
			}
		}
		player->drillmeter += TICRATE;
		dummymo->momz = FRACUNIT;
		dummymo->fuse = 3*TICRATE;
		if(toucher->target)
		{
			if(toucher->target->flags & MF_AMBUSH)
				P_InstaThrust(dummymo, R_PointToAngle2(dummymo->x, dummymo->y, toucher->target->x, toucher->target->y), 3*FRACUNIT);
			else
				P_InstaThrust(dummymo, R_PointToAngle2(toucher->target->x, toucher->target->y, dummymo->x, dummymo->y), 3*FRACUNIT);
		}
		return;
	case MT_NIGHTSDRONE:
		if(player->bonustime && !player->exiting)
		{
			if(!player->nightsmode)
			{
				if(!(netgame || multiplayer))
				{
					special->flags2 |= MF2_DONTDRAW;
					special->tracer = toucher;
				}

				toucher->tracer = P_SpawnMobj(toucher->x, toucher->y, toucher->z, MT_NIGHTSCHAR);
			}

			P_NightserizePlayer(player, special->health, true);
			S_StartSound(toucher, sfx_ideya);

			if(P_TransferToNextMare(player) == false)
			{
				for(i=0; i<MAXPLAYERS; i++)
					P_DoPlayerExit(&players[i]);
			}
			return;
		}
		if(!player->nightsmode)
		{
			if(!(netgame || multiplayer))
			{
				special->flags2 |= MF2_DONTDRAW;
				special->tracer = toucher;
			}

			S_StartSound(toucher, sfx_supert);
			toucher->tracer = P_SpawnMobj(toucher->x, toucher->y, toucher->z, MT_NIGHTSCHAR);
			P_NightserizePlayer(player, special->health, false);
		}
		return;
	case MT_NIGHTSPARKLE:
		if(special->fuse < player->mo->fuse - TICRATE)
		{
		    thinker_t*  th;
			mobj_t*     mo2;
			int count;
			int x,y,z;
			fixed_t gatherradius;
			double d;

			if(special->target != toucher) // These ain't your sparkles, pal!
				return;

			x = special->x >> FRACBITS;
			y = special->y >> FRACBITS;
			z = special->z >> FRACBITS;
			count = 1;

			// scan the remaining thinkers
			for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
			{
				if (th->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;

				mo2 = (mobj_t *)th;

				if(mo2 == special)
					continue;

				if(mo2->type == MT_NIGHTSPARKLE && mo2->fuse >= special->fuse
					&& mo2->target == toucher
					&& (mo2->flags & MF_SPECIAL))
				{
					mo2->tics = 1;
					mo2->flags &= ~MF_SPECIAL;
					count++;
					x += mo2->x >> FRACBITS;
					y += mo2->y >> FRACBITS;
					z += mo2->z >> FRACBITS;
				}
				else if(mo2->type == MT_NIGHTSPARKLE && mo2->target == toucher
					&& (mo2->flags & MF_SPECIAL))
				{
					mo2->tics = 1;
					mo2->flags &= ~MF_SPECIAL;
				}
			}
			x/=count;
			y/=count;
			z/=count;
			x <<= FRACBITS;
			y <<= FRACBITS;
			z <<= FRACBITS;
			P_SetMobjState(special, S_DISS);
			gatherradius = P_AproxDistance(P_AproxDistance(special->x - x, special->y - y), special->z - z);

			if(player->powers[pw_superparaloop])
				gatherradius *= 2;

			if(gatherradius < 30*FRACUNIT) // Player is probably just sitting there.
				return;

			for (d=0; d<360.0; d+= 22.5)
				P_SpawnParaloop(x, y, z, gatherradius, 16, MT_NIGHTSPARKLE, d);

			S_StartSound(toucher, sfx_prloop);

			// Now we RE-scan all the thinkers to find close objects to pull
			// in from the paraloop. Isn't this just so efficient?
			for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
			{
				if (th->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;

				mo2 = (mobj_t *)th;

				if(P_AproxDistance(P_AproxDistance(mo2->x - x, mo2->y - y), mo2->z - z) > gatherradius)
					continue;

				if(mo2->flags & MF_SHOOTABLE)
				{
					P_DamageMobj(mo2, toucher, toucher, 1);
					continue;
				}

				// Make these APPEAR!
				// Tails 12-15-2003
				if(mo2->type == MT_NIGHTSSUPERLOOP
					|| mo2->type == MT_NIGHTSDRILLREFILL
					|| mo2->type == MT_NIGHTSHELPER)
				{
					if(!(mo2->flags & MF_SPECIAL))
					{
						P_SetMobjState(mo2, mo2->info->seestate);
						mo2->flags |= MF_SPECIAL;
						S_StartSound(toucher, sfx_hidden);
						continue;
					}
				}

				if(!(mo2->type == MT_NIGHTSWING || mo2->type == MT_RING || mo2->type == MT_COIN))
					continue;

				// Yay! The thing's in reach! Pull it in!
				mo2->flags2 |= MF2_NIGHTSPULL;
				mo2->tracer = toucher;
			}
		}
		return;
	case MT_STARPOST:
		// In circuit mode, player must have touched all previous starposts Graue 11-18-2003
		if(cv_gametype.value == GT_CIRCUIT && special->health - player->starpostnum > 1)
		{
			if(!S_SoundPlaying(special, -1))
				S_StartSound(special, sfx_lose); // Graue 11-19-2003
			return;
		}

		// Make another switch() unnecessary Graue 12-06-2003
		if(special->health > 10)
		{
			CONS_Printf("Bad Starpost Number!\n");
			return;
		}
		
		if(player->starpostbit & (1<<(special->health-1)))
			return; // Already hit this post

		player->starpostbit |= (1<<(special->health-1));

		// Save the player's time and position.
		player->starposttime = leveltime;
		player->starpostx = player->mo->x >> FRACBITS;
		player->starposty = player->mo->y >> FRACBITS;
		player->starpostz = special->z >> FRACBITS;
		player->starpostangle = special->angle;
		player->starpostnum = special->health;
		S_StartSound(special, sfx_strpst);
		P_ClearStarPost(player, special->health);
		P_SetMobjState(special, S_STARPOST4);
		return;
	case MT_SPIKEBALL:
	case MT_FLAME:
	case MT_GOOP:
		P_DamageMobj(toucher, special, special, 1);
		return;
	case MT_SPECIALSPIKEBALL:
		if(!(gamemap >= sstage_start && gamemap <= sstage_end))
		{
			P_DamageMobj(toucher, special, special, 1);
			return;
		}

		if(player->powers[pw_flashing])
			return;
		player->powers[pw_flashing] = flashingtics;
		P_PlayRinglossSound(toucher);
		if(toucher->health > 10)
			toucher->health -= 10;
		else
			toucher->health = 1;
		player->health = toucher->health;
		toucher->z++;
		if(toucher->eflags & MF_UNDERWATER)
			toucher->momz = 4.04269230769230769230769230769231*FRACUNIT;
		else
			toucher->momz = 6.9*FRACUNIT;
		P_InstaThrust (toucher, -toucher->angle, 4*FRACUNIT);
		P_ResetPlayer(player);
		P_SetMobjState(toucher, S_PLAY_PAIN);
		return;
	case MT_SANTA: // Not used anymore. But oh well.
		if(player->exiting)
			return;
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(!playeringame[i])
				continue;
			if(players[i].emeraldhunt > 0)
				emercount += players[i].emeraldhunt;
		}
		if(emercount >= 3)
		{
			for(i=0; i<MAXPLAYERS; i++)
			{
				if(!playeringame[i])
					continue;

				players[i].exiting = 2.8*TICRATE + 1;
				players[i].snowbuster = true;
			}
			S_StartSound(0, sfx_lvpass);
		}
		return;

	// Emerald Hunt Tails 12-12-2001
	case MT_EMERHUNT:
	case MT_EMESHUNT:
	case MT_EMETHUNT:
		player->emeraldhunt++;
		P_SetMobjState(special, S_DISS);
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		if(hunt1 == special)
		{
			hunt1->health = 0;
			hunt1 = NULL;
		}
		else if(hunt2 == special)
		{
			hunt2->health = 0;
			hunt2 = NULL;
		}
		else if(hunt3 == special)
		{
			hunt3->health = 0;
			hunt3 = NULL;
		}
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(!playeringame[i])
				continue;

			if(players[i].emeraldhunt > 0)
				emercount += players[i].emeraldhunt;
		}
		if(emercount >= 3)
		{
			for(i=0; i<MAXPLAYERS; i++)
			{
				if(!playeringame[i])
					continue;

				players[i].exiting = 2.8*TICRATE + 1;
			}
			S_StartSound(0, sfx_lvpass);
		}
		return;

	case MT_PUMA:
	case MT_HAMMER:
	case MT_KOOPA:
	case MT_KOOPAFLAME:
		P_DamageMobj(toucher, special, special, 1);
		return;
	case MT_SHELL:
		if(special->state == &states[S_SHELL]) // Resting anim
		{
			// Kick that sucker around!
			special->angle = toucher->angle;
			P_InstaThrust(special, special->angle, special->info->speed);
			S_StartSound(toucher, sfx_lose);
			P_SetMobjState(special, S_SHELL1);
			special->target = toucher;
			special->threshold = 1.5*TICRATE;
		}
		return;
	case MT_AXE:
		{
			line_t junk;
			thinker_t*  th;
			mobj_t*     mo2;

			junk.tag = 649;
			EV_DoElevator(&junk, bridgeFall, false);
    
			// scan the remaining thinkers
			// to find koopa
			for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
			{
				if (th->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;

				mo2 = (mobj_t *)th;
				if(mo2->type == MT_KOOPA)
				{
					mo2->momz = 5*FRACUNIT;
					break;
				}
			}
			P_SetMobjState(special, S_DISS);
			return;
		}
	case MT_FIREFLOWER:
		if(!mariomode)
			return;
		toucher->player->powers[pw_fireflower] = true;
		toucher->flags =  (toucher->flags & ~MF_TRANSLATION)
                     | ((13)<<MF_TRANSSHIFT);
		sound = sfx_shield;
		break;
        // rings
	case MT_COIN:
	case MT_FLINGCOIN:
        if(toucher->player->powers[pw_flashing] > flashingtics/2)
			return;
		if(mariomode)
			P_SpawnMobj (special->x,special->y,special->z, MT_COINSPARKLE)->momz = special->momz;
		else
			P_SpawnMobj (special->x,special->y,special->z, MT_COINSPARKLE);
        player->health++;
        player->mo->health = player->health;
		player->totalring++;

		if(player->lightdash)
			player->lightdash = TICRATE;

		if((mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS) || cv_timeattacked.value)
		{
			player->linkcount++;
			player->linktimer = 2*TICRATE;

			dummymo = P_SpawnMobj(special->x, special->y, special->z, MT_NIGHTSCORE);

			if(player->linkcount < 10)
			{
				if(player->bonustime)
				{
					P_AddPlayerScore(player, player->linkcount*20);
					P_SetMobjState(dummymo, dummymo->info->xdeathstate+player->linkcount-1);
				}
				else
				{
					P_AddPlayerScore(player, player->linkcount*10);
					P_SetMobjState(dummymo, dummymo->info->spawnstate+player->linkcount-1);
				}
			}
			else
			{
				if(player->bonustime)
				{
					P_AddPlayerScore(player, 200);
					P_SetMobjState(dummymo, dummymo->info->xdeathstate+9);
				}
				else
				{
					P_AddPlayerScore(player, 100);
					P_SetMobjState(dummymo, dummymo->info->spawnstate+9);
				}
			}

			dummymo->momz = FRACUNIT;
			dummymo->fuse = 3*TICRATE;
			if(toucher->target)
			{
				if(toucher->target->flags & MF_AMBUSH)
					P_InstaThrust(dummymo, R_PointToAngle2(dummymo->x, dummymo->y, toucher->target->x, toucher->target->y), 3*FRACUNIT);
				else
					P_InstaThrust(dummymo, R_PointToAngle2(toucher->target->x, toucher->target->y, dummymo->x, dummymo->y), 3*FRACUNIT);
			}
		}
        break;

      case MT_RING:
	  case MT_FLINGRING:
        if(toucher->player->powers[pw_flashing] > flashingtics/2)
			return;
		if(mariomode)
			P_SpawnMobj (special->x,special->y,special->z, MT_SPARK)->momz = special->momz;
		else
			P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
        player->health++;
        player->mo->health = player->health;
		player->totalring++;

		if(player->lightdash)
			player->lightdash = TICRATE;

		if((mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS) || cv_timeattacked.value)
		{
			player->linkcount++;
			player->linktimer = 2*TICRATE;

			dummymo = P_SpawnMobj(special->x, special->y, special->z, MT_NIGHTSCORE);

			if(player->linkcount < 10)
			{
				if(player->bonustime)
				{
					P_AddPlayerScore(player, player->linkcount*20);
					P_SetMobjState(dummymo, dummymo->info->xdeathstate+player->linkcount-1);
				}
				else
				{
					P_AddPlayerScore(player, player->linkcount*10);
					P_SetMobjState(dummymo, dummymo->info->spawnstate+player->linkcount-1);
				}
			}
			else
			{
				if(player->bonustime)
				{
					P_AddPlayerScore(player, 200);
					P_SetMobjState(dummymo, dummymo->info->xdeathstate+9);
				}
				else
				{
					P_AddPlayerScore(player, 100);
					P_SetMobjState(dummymo, dummymo->info->spawnstate+9);
				}
			}
			dummymo->momz = FRACUNIT;
			dummymo->fuse = 3*TICRATE;
			if(toucher->target)
			{
				if(toucher->target->flags & MF_AMBUSH)
					P_InstaThrust(dummymo, R_PointToAngle2(dummymo->x, dummymo->y, toucher->target->x, toucher->target->y), 3*FRACUNIT);
				else
					P_InstaThrust(dummymo, R_PointToAngle2(toucher->target->x, toucher->target->y, dummymo->x, dummymo->y), 3*FRACUNIT);
			}
		}
        break;

	  case MT_HOMINGRING:
		if(toucher->player->powers[pw_flashing] > flashingtics/2)
			return;
		player->health++;               // can go over 100%
        if (player->health > 1000) // go up to 999 rings Tails 11-01-99
            player->health = 1000; // go up to 999 rings Tails 11-01-99
        player->mo->health = player->health;
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		player->powers[pw_homingring] += special->health;
		break;
	  case MT_RAILRING:
		if(toucher->player->powers[pw_flashing] > flashingtics/2)
			return;
		player->health++;               // can go over 100%
        if (player->health > 1000) // go up to 999 rings Tails 11-01-99
            player->health = 1000; // go up to 999 rings Tails 11-01-99
        player->mo->health = player->health;
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		player->powers[pw_railring] += special->health;
		break;
	  case MT_SHIELDRING:
		if(toucher->player->powers[pw_flashing] > flashingtics/2)
			return;
		player->health++;               // can go over 100%
        if (player->health > 1000) // go up to 999 rings Tails 11-01-99
            player->health = 1000; // go up to 999 rings Tails 11-01-99
        player->mo->health = player->health;
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		player->powers[pw_shieldring] += special->health;
		break;
	  case MT_AUTOMATICRING:
		if(toucher->player->powers[pw_flashing] > flashingtics/2)
			return;
		player->health++;               // can go over 100%
        if (player->health > 1000) // go up to 999 rings Tails 11-01-99
            player->health = 1000; // go up to 999 rings Tails 11-01-99
        player->mo->health = player->health;
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		player->powers[pw_automaticring] += special->health;
		break;
	  case MT_EXPLOSIONRING:
		if(toucher->player->powers[pw_flashing] > flashingtics/2)
			return;
		player->health++;               // can go over 100%
        if (player->health > 1000) // go up to 999 rings Tails 11-01-99
            player->health = 1000; // go up to 999 rings Tails 11-01-99
        player->mo->health = player->health;
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		player->powers[pw_explosionring] += special->health;
        break;

        // Special Stage Token Tails 08-11-2001
	  case MT_EMMY:
		P_SpawnMobj (special->x,special->y,special->z, MT_SPARK);
		tokenlist += special->health;
        token++; // I've got a token!

		if(emeralds == 127) // Got all 7
		{
			player->mo->health += 50;
			player->health += 50;
		}
        break;

// start bubble grab Tails 03-07-2000
      case MT_EXTRALARGEBUBBLE:
		if(player->powers[pw_greenshield])
			return;
		else if(special->z < player->mo->z + player->mo->height / 3
			|| special->z > player->mo->z + (player->mo->height*2/3))
			return; // Only go in the mouth
		else
		{
      if(player->powers[pw_underwater] <= 12*TICRATE + 1)
        {
		if(player->powers[pw_super])
		{
			S_ChangeMusic(mus_supers, true);
		}
		else if(player->powers[pw_invulnerability])
		{
			if(mariomode)
				S_ChangeMusic(mus_minvnc, false);
			else
				S_ChangeMusic(mus_invinc, false);
		}
		else if(player->powers[pw_sneakers])
		{
			S_ChangeMusic(mus_shoes, false);
		}
		else
		{
			S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, true);
        }
	  }

	  if(player->powers[pw_underwater] < underwatertics + 1)
        player->powers[pw_underwater] = underwatertics + 1;

        P_SpawnMobj (special->x,special->y,special->z, MT_POP);
           player->message = NULL;
           sound = sfx_gasp;
		P_SetMobjState(player->mo, S_PLAY_GASP);
		P_ResetPlayer(player);
		player->mo->momx = player->mo->momy = player->mo->momz = 0;
		}
          break;
// end bubble grab Tails 03-07-2000

/*
      case MT_EMMY:
    if(!(player->emerald1)){
        player->emerald1 = true;
        sound = sfx_getpow;
        break;}
  else if((player->emerald1) && !(player->emerald2)){
        player->emerald2 = true;
        sound = sfx_getpow;
        break;}
  else if((player->emerald2) && !(player->emerald3)){
        player->emerald3 = true;
        sound = sfx_getpow;
        break;}
   else if((player->emerald3) && !(player->emerald4)){
        player->emerald4 = true;
        sound = sfx_getpow;
        break;}
   else if((player->emerald4) && !(player->emerald5)){
        player->emerald5 = true;
        sound = sfx_getpow;
        break;}
   else if((player->emerald5) && !(player->emerald6)){
        player->emerald6 = true;
        sound = sfx_getpow;
        break;}
   else if((player->emerald6) && !(player->emerald7)){
        player->emerald7 = true;
        sound = sfx_getpow;
        break;}
   else
     break;
*/

	  case MT_REDFLAG:
		  if(toucher->player->powers[pw_flashing] > flashingtics/2)
			return;
		  if(special->fuse == 1)
			  return;
		  if(player->ctfteam == 1 && player->specialsector != 988) // Player is on the Red Team
		  {
				if(special->x != special->spawnpoint->x << FRACBITS
					&& special->y != special->spawnpoint->y << FRACBITS
					&& special->z != special->spawnpoint->z << FRACBITS)
				{
					special->fuse = 1;

					CONS_Printf("%s returned the red flag to base.\n", player_names[player-players]);
				}
		  /*
			player->gotflag |= MF_REDFLAG;
			rflagpoint = special->spawnpoint;
			S_StartSound (player->mo, sfx_lvpass);
			P_SetMobjState(special, S_DISS);*/
		  }
		  else if(player->ctfteam == 2) // Player is on the Blue Team
		  {
			player->gotflag |= MF_REDFLAG;
			rflagpoint = special->spawnpoint;
			S_StartSound (player->mo, sfx_lvpass);
			P_SetMobjState(special, S_DISS);
			CONS_Printf("%s picked up the red flag!\n", player_names[player-players]);
		  }
		  return;

	  case MT_BLUEFLAG:
		  if(toucher->player->powers[pw_flashing] > flashingtics/2)
			return;
		  if(special->fuse == 1)
			  return;
			if(player->ctfteam == 2 && player->specialsector != 989) // Player is on the Blue Team
		  {
				if(special->x != special->spawnpoint->x << FRACBITS
					&& special->y != special->spawnpoint->y << FRACBITS
					&& special->z != special->spawnpoint->z << FRACBITS)
				{
					special->fuse = 1;

					CONS_Printf("%s returned the blue flag to base.\n", player_names[player-players]);
				}
		/*	player->gotflag |= MF_BLUEFLAG;
			bflagpoint = special->spawnpoint;
			S_StartSound (player->mo, sfx_lvpass);
			P_SetMobjState(special, S_DISS);*/
		  }
		  else if(player->ctfteam == 1) // Player is on the Red Team
		  {
			player->gotflag |= MF_BLUEFLAG;
			bflagpoint = special->spawnpoint;
			S_StartSound (player->mo, sfx_lvpass);
			P_SetMobjState(special, S_DISS);
			CONS_Printf("%s picked up the blue flag!\n", player_names[player-players]);
		  }
		  return;

	  case MT_DISS:
		  break;

	  case MT_TOKEN: // Tails 08-18-2001
		  P_SetMobjState(special, S_DISS);
		  return; // Tails 08-18-2001

      default:
		  if(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE && special->flags & MF_MONITOR)
		  {
			  P_DamageMobj(special, toucher, toucher, 1);
			  sound = sfx_None;
		  }
//		  else
//			CONS_Printf ("\2P_TouchSpecialThing: Unknown gettable thing\n");
        break;
    }
	}

    if (special->flags & MF_COUNTITEM)
        player->itemcount++;
 // Buncha stuff here Tails
    P_SetMobjState(special, S_DISS);
	special->flags &= ~MF_NOBLOCKMAP;
//    player->bonuscount += BONUSADD;

    //added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
//    if (player == &players[displayplayer] || (cv_splitscreen.value && player==&players[secondarydisplayplayer]))
	if(sound != sfx_None)
		S_StartSound (player->mo, sound); // was NULL, but changed to player so you could hear others pick up rings Tails 01-11-2001
}



#ifdef thatsbuggycode
//
//  Tell each supported thing to check again its position,
//  because the 'base' thing has vanished or diminished,
//  the supported things might fall.
//
//added:28-02-98:
void P_CheckSupportThings (mobj_t* mobj)
{
    fixed_t   supportz = mobj->z + mobj->height;

    while ((mobj = mobj->supportthings))
    {
        // only for things above support thing
        if (mobj->z > supportz)
            mobj->eflags |= MF_CHECKPOS;
    }
}


//
//  If a thing moves and supportthings,
//  move the supported things along.
//
//added:28-02-98:
void P_MoveSupportThings (mobj_t* mobj, fixed_t xmove, fixed_t ymove, fixed_t zmove)
{
    fixed_t   supportz = mobj->z + mobj->height;
    mobj_t    *mo = mobj->supportthings;

    while (mo)
    {
        //added:28-02-98:debug
        if (mo==mobj)
        {
            mobj->supportthings = NULL;
            break;
        }

        // only for things above support thing
        if (mobj->z > supportz)
        {
            mobj->eflags |= MF_CHECKPOS;
            mobj->momx += xmove;
            mobj->momy += ymove;
            mobj->momz += zmove;
        }

        mo = mo->supportthings;
    }
}


//
//  Link a thing to it's 'base' (supporting) thing.
//  When the supporting thing will move or change size,
//  the supported will then be aware.
//
//added:28-02-98:
void P_LinkFloorThing(mobj_t*   mobj)
{
    mobj_t*     mo;
    mobj_t*     nmo;

    // no supporting thing
    if (!(mo = mobj->floorthing))
        return;

    // link mobj 'above' the lower mobjs, so that lower supporting
    // mobjs act upon this mobj
    while ( (nmo = mo->supportthings) &&
            (nmo->z<=mobj->z) )
    {
        // dont link multiple times
        if (nmo==mobj)
            return;

        mo = nmo;
    }
    mo->supportthings = mobj;
    mobj->supportthings = nmo;
}


//
//  Unlink a thing from it's support,
//  when it's 'floorthing' has changed,
//  before linking with the new 'floorthing'.
//
//added:28-02-98:
void P_UnlinkFloorThing(mobj_t*   mobj)
{
    mobj_t*     mo;

    if (!(mo = mobj->floorthing))      // just to be sure (may happen)
       return;

    while (mo->supportthings)
    {
        if (mo->supportthings == mobj)
        {
            mo->supportthings = NULL;
            break;
        }
        mo = mo->supportthings;
    }
}
#endif


// Death messages relating to the target (dying) player
//
static void P_DeathMessages ( mobj_t*       target,
                              mobj_t*       inflictor,
                              mobj_t*       source )
{
    int     w;
    char    *str;

	if(cv_gametype.value == GT_COOP || cv_gametype.value == GT_RACE) // Tails
		return; // Tails

    if (!target || !target->player)
        return;

	str = "%s died.\n";

    if (source && source->player)
    {
        if (source->player==target->player)
            CONS_Printf("%s suicides\n", player_names[target->player-players]);
        else
        {
			str = "%s was killed by %s\n";

            CONS_Printf(str,player_names[target->player-players],
                            player_names[source->player-players]);
        }
		return;
    }
	else if(source)
	{
		switch(source->type)
		{
			case MT_DISS:
				if(source->threshold == 42)
					str = "%s drowned.\n";
				break;
			case MT_BLUECRAWLA:
				if(cv_gametype.value == GT_CHAOS)
					str = "%s was killed by a blue crawla!\n";
				break;
			case MT_REDCRAWLA:
				if(cv_gametype.value == GT_CHAOS)
					str = "%s was killed by a red crawla!\n";
				break;
			case MT_JETTGUNNER:
				if(cv_gametype.value == GT_CHAOS)
					str = "%s was killed by a jetty-syn gunner!\n";
				break;
			case MT_JETTBOMBER:
				if(cv_gametype.value == GT_CHAOS)
					str = "%s was killed by a jetty-syn bomber!\n";
				break;
			case MT_CRAWLACOMMANDER:
				if(cv_gametype.value == GT_CHAOS)
					str = "%s was killed by a crawla commander!\n";
				break;
			case MT_EGGMOBILE:
				if(cv_gametype.value == GT_CHAOS)
					str = "%s was killed by the Egg Mobile!\n";
				break;
			case MT_EGGMOBILE2:
				if(cv_gametype.value == GT_CHAOS)
					str = "%s was killed by the Egg Slimer!\n"; // Graue 12-13-2003
				break;
			default:
				break;
		}
	}
	else if((inflictor->flags & MF_PUSHABLE) && source && source->player)
	{
		CONS_Printf("%s crushed %s with a heavy object!\n", player_names[source->player-players], player_names[target->player-players]);
	}
    else
    {
        if (!source)
        {
            // environment kills
            w = target->player->specialsector;      //see p_spec.c

            if (w==5)
				str = "%s fell into a bottomless pit.\n";
            else if (w==7)
                str = "%s fell in some nasty goop!\n";
        }
    }
    CONS_Printf(str, player_names[target->player-players]);

	if((cv_gametype.value == GT_COOP || cv_gametype.value == GT_RACE) && target->player->lives - 1 <= 0)
		CONS_Printf("%s got a game over.\n",player_names[target->player-players]);
}

// WARNING : check cv_fraglimit>0 before call this function !
void P_CheckFragLimit(player_t *p)
{
/*    if(cv_teamplay.value)
    {
        int fragteam=0,i;

        for(i=0;i<MAXPLAYERS;i++)
            if(ST_SameTeam(p,&players[i]))
                fragteam += ST_PlayerFrags(i);

        if(cv_fraglimit.value<=fragteam)
            G_ExitLevel();
    }
    else
    {
        if(cv_fraglimit.value<=ST_PlayerFrags(p-players))
            G_ExitLevel();
    }*/

	// Change fraglimit to pointlimit Graue 12-13-2003
	if(cv_gametype.value == GT_CTF)
	{
		// Just check both teams Graue 12-16-2003
		if(cv_fraglimit.value <= redscore || cv_fraglimit.value <= bluescore)
			G_ExitLevel();
	}
	else if(cv_fraglimit.value <= p->score)
		G_ExitLevel();
}

extern consvar_t cv_soniccd;
// P_KillMobj
//
//      source is the attacker,
//      target is the 'target' of the attack, target dies...
//                                          113
void P_KillMobj ( mobj_t*       target,
                  mobj_t*       inflictor,
                  mobj_t*       source )
{
    mobjtype_t  item;
    mobj_t*     mo;

	if(mariomode && inflictor && (inflictor->type == MT_SHELL || inflictor->type == MT_FIREBALL))
		target->tracer = inflictor;

    // dead target is no more shootable
    target->flags &= ~MF_SHOOTABLE;

	target->flags2 &= ~MF2_SKULLFLY;

    target->flags &= ~MF_FLOAT;

	if(target->flags & MF_BOSS)
		target->momx = target->momy = target->momz = 0;
	else if(target->flags & MF_ENEMY)
		target->momz = 0;

//    if (target->type != MT_SKULL)
//        target->flags &= ~MF_NOGRAVITY;
    if (target->type != MT_PLAYER
		&& !(target->flags & MF_MONITOR))
        target->flags |= MF_NOGRAVITY; // Don't drop Tails 03-08-2000

	// Let EVERYONE know what happened to a player! 01-29-2002 Tails
    if (target->player)
	{
		// Graue 12-13-2003: suicide penalty no longer applies to CTF,
		//	                 nor match if cv_match_scoring == 1
		extern consvar_t cv_match_scoring;

		if(cv_gametype.value == GT_CHAOS)
			target->player->score /= 2; // Halve the player's score in Chaos Mode
		else if((cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG)
			&& ((target == source) || (source == NULL && inflictor == NULL))
			&& target->player->score >= 50 && cv_match_scoring.value == 0) // Suicide penalty
			target->player->score -= 50;


        P_DeathMessages (target, inflictor, source);

		target->flags2 &= ~MF2_DONTDRAW;
	}

    // if killed by a player
    if (source && source->player)
    {
		if(target->flags & MF_MONITOR)
		{
			target->target = source;
			source->player->numboxes++;
			// Graue 12-06-2003: cv_itemrespawn works in race mode now too
			if((cv_itemrespawn.value && (modifiedgame || netgame || multiplayer))) //|| cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CTF || cv_gametype.value == GT_CHAOS || cv_gametype.value == GT_CIRCUIT) // Random box generation Tails 08-09-2001
				target->fuse = cv_itemrespawntime.value*TICRATE;
		}

		// Time Attacked Thingy Tails 12-14-2003
		if(cv_timeattacked.value)
		{
			source->player->linkcount++;
			source->player->linktimer = 2*TICRATE;
		}

		// Award Score Tails
		{
			int score = 0;

			if(cv_gametype.value == GT_CHAOS)
			{
				if((target->flags & MF_ENEMY)
					&& !(target->flags & MF_MISSILE))
				{
						source->player->scoreadd++; // Tails 11-03-2000
				}

				switch(target->type)
				{
					case MT_BLUECRAWLA:
					case MT_GOOMBA:
						score = 100*source->player->scoreadd;
						break;
					case MT_REDCRAWLA:
					case MT_BLUEGOOMBA:
						score = 150*source->player->scoreadd;
						break;
					case MT_JETTBOMBER:
						score = 400*source->player->scoreadd;
						break;
					case MT_JETTGUNNER:
						score = 500*source->player->scoreadd;
						break;
					case MT_CRAWLACOMMANDER:
						score = 300*source->player->scoreadd;
						break;
					default:
						score = 100*source->player->scoreadd;
						break;
				}
			}
			else
			{
				if(target->flags & MF_BOSS)
					score = 1000;
				else if((target->flags & MF_ENEMY)
					&& !(target->flags & MF_MISSILE))
				{
					source->player->scoreadd++; // Tails 11-03-2000
					if(source->player->scoreadd == 1)
					{
						score = 100; // Score! Tails 03-01-2000
						P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), MT_SCRA);
					}
					if(source->player->scoreadd == 2)
					{
						score = 200; // Score! Tails 03-01-2000
						P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), MT_SCRB);
					}
					if(source->player->scoreadd == 3)
					{
						score = 500; // Score! Tails 03-01-2000
						P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), MT_SCRC);
					}
					if(source->player->scoreadd >= 4)
					{
						score = 1000; // Score! Tails 03-01-2000
						P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), MT_SCRD);
					}
				}
			}

			P_AddPlayerScore(source->player, score);
		}

        // count for intermission
        if (target->flags & MF_COUNTKILL)
            source->player->killcount++;

    }
    else if (!multiplayer && (target->flags & MF_COUNTKILL))
    {
        // count all monster deaths,
        // even those caused by other monsters
        players[0].killcount++;
    }

    // if a player avatar dies...
    if (target->player)
    {
        target->flags &= ~MF_SOLID;                     // does not block
//        target->flags2 &= ~MF2_FLY;

		if(cv_gametype.value == GT_COOP || cv_gametype.value == GT_RACE) // Coop and race only Graue 12-13-2003
		{
			target->player->lives -= 1; // Lose a life Tails 03-11-2000

			if(target->player->lives <= 0) // Tails 03-14-2000
			{
				if(target->player==&players[consoleplayer])
				{
					S_StopMusic(); // Stop the Music! Tails 03-14-2000
					S_ChangeMusic(mus_gmover, false); // Yousa dead now, Okieday? Tails 03-14-2000
				}
			}
		}
        target->player->playerstate = PST_DEAD;

        if (target->player == &players[consoleplayer])
        {
            // don't die in auto map,
            // switch view prior to dying
            if (automapactive)
                AM_Stop ();

            //added:22-02-98: recenter view for next live...
            localaiming = 0;
        }
        if (cv_splitscreen.value && target->player == &players[secondarydisplayplayer])
        {
            //added:22-02-98: recenter view for next live...
            localaiming2 = 0;
        }
/* HERETODO
        if(target->flags2&MF2_FIREDAMAGE)
        { // Player flame death
            P_SetMobjState(target, S_PLAY_FDTH1);
            //S_StartSound(target, sfx_hedat1); // Burn sound
            return;
        }
*/
    }

    P_SetMobjState (target, target->info->deathstate);
	/* FIXTHIS: For player, the above is redundant because of P_SetMobjState (target, S_PLAY_DIE1)
	   in P_DamageMobj()
	   Graue 12-22-2003 */

	if(source && target && target->player && source->player)
		P_PlayVictorySound(source); // Killer laughs at you. LAUGHS! BWAHAHAHA!

//    target->tics -= P_Random()&3;

    if (target->tics < 1)
        target->tics = 1;

	if(mariomode // Don't show birds, etc. in Mario Mode Tails 12-23-2001
		|| cv_gametype.value == GT_CHAOS) // Or Chaos Mode!
		return;

    // Drop stuff.
    // This determines the kind of object spawned
    // during the death frame of a thing.
	if(target->flags & MF_ENEMY)
	{
		if(cv_soniccd.value)
		{
			item = MT_SEED;
		}
		else
		{
			int random;

			switch (target->type)
			{
			  case MT_REDCRAWLA:
				item = MT_SQRL;
				break;

			  case MT_BLUECRAWLA:
			  case MT_JETTBOMBER:
			  case MT_GFZFISH:
				item = MT_BIRD;
				break;

			  case MT_JETTGUNNER:
			  case MT_CRAWLACOMMANDER:
				item = MT_MOUSE;
				break;

			  default:
				random = P_Random();

				if(random < 86)
				  item = MT_SQRL;
				else if(random < 172)
				  item = MT_BIRD;
				else
				  item = MT_MOUSE;
				  				  
				break;
			}
		}

    // SoM: Damnit! Why not use the target's floorz?
    mo = P_SpawnMobj (target->x,target->y,target->z + (target->height / 2), item);
//    mo->flags |= MF_DROPPED;    // special versions of items
	}
}


//---------------------------------------------------------------------------
//
// FUNC P_MinotaurSlam
//
//---------------------------------------------------------------------------

void P_MinotaurSlam(mobj_t *source, mobj_t *target)
{
    angle_t angle;
    fixed_t thrust;
    
    angle = R_PointToAngle2(source->x, source->y, target->x, target->y);
    angle >>= ANGLETOFINESHIFT;
    thrust = 16*FRACUNIT+(P_Random()<<10);
    target->momx += FixedMul(thrust, finecosine[angle]);
    target->momy += FixedMul(thrust, finesine[angle]);
    P_DamageMobj(target, NULL, NULL, HITDICE(6));
    if(target->player)
    {
        target->reactiontime = 14+(P_Random()&7);
    }
}

//---------------------------------------------------------------------------
//
// FUNC P_TouchWhirlwind
//
//---------------------------------------------------------------------------

boolean P_TouchWhirlwind(mobj_t *target)
{
/*    int randVal;
    
    target->angle += P_SignedRandom()<<20;
    target->momx += P_SignedRandom()<<10;
    target->momy += P_SignedRandom()<<10;
    if(leveltime&16 && !(target->flags2&MF2_BOSS))
    {
        randVal = P_Random();
        if(randVal > 160)
        {
            randVal = 160;
        }
        target->momz += randVal<<10;
        if(target->momz > 12*FRACUNIT)
        {
            target->momz = 12*FRACUNIT;
        }
    }
    if(!(leveltime&7))
    {
        return P_DamageMobj(target, NULL, NULL, 3);
    }*/
    return false;
}

// Tails 09-06-2002
// P_PlayRinglossSound
//
// Plays the sound when you lose rings. Put here for easy
// changes and multiplayer custom skin sound sorting.
//
void P_PlayRinglossSound(mobj_t* source)
{
	int random;

	random = P_Random();

	if(random <= 63)
		S_StartSound(source, sfx_altow1);
	else if(random <= 127)
		S_StartSound(source, sfx_altow2);
	else if(random <= 191)
		S_StartSound(source, sfx_altow3);
	else
		S_StartSound(source, sfx_altow4);
}

// Tails 09-06-2002
// P_PlayDeathSound
//
// Plays the sound when you kick the bucket, buy the farm,
// etc. Put here for easy changes and multiplayer custom
// skin sound sorting.
//
void P_PlayDeathSound(mobj_t* source)
{
	int random;

	random = P_Random();

	if(random <= 63)
		S_StartSound(source, sfx_altdi1);
	else if(random <= 127)
		S_StartSound(source, sfx_altdi2);
	else if(random <= 191)
		S_StartSound(source, sfx_altdi3);
	else
		S_StartSound(source, sfx_altdi4);
}

// Tails 09-06-2002
// P_PlayVictorySound
//
// Plays the sound when you make someone buy the farm.
// Put here for easy changes and multiplayer custom
// skin sound sorting.
void P_PlayVictorySound(mobj_t* source)
{
	int random;

	random = P_Random();

	if(random <= 63)
		S_StartSound(source, sfx_victr1);
	else if(random <= 127)
		S_StartSound(source, sfx_victr2);
	else if(random <= 191)
		S_StartSound(source, sfx_victr3);
	else
		S_StartSound(source, sfx_victr4);
}

// Tails 09-06-2002
// P_PlayTauntSound
//
// Plays the sound when you hit the taunt key.
// Put here for easy changes and multiplayer
// custom skin sound sorting.
void P_PlayTauntSound(mobj_t* source)
{
	int random;

	random = P_Random();

	if(random <= 63)
		S_StartSound(source, sfx_taunt1);
	else if(random <= 127)
		S_StartSound(source, sfx_taunt2);
	else if(random <= 191)
		S_StartSound(source, sfx_taunt3);
	else
		S_StartSound(source, sfx_taunt4);

	// In the future... taunt animation?
}

extern consvar_t cv_killingdead;

//
// P_DamageMobj
// Damages both enemies and players
// "inflictor" is the thing that caused the damage
//  creature or missile, can be NULL (slime, etc)
// "source" is the thing to target after taking damage
//  creature or NULL
// Source and inflictor are the same for melee attacks.
// Source can be NULL for slime, barrel explosions
// and other environmental stuff.
//
boolean P_DamageMobj ( mobj_t*   target,
                       mobj_t*   inflictor,
                       mobj_t*   source,
                       int       damage )
{
    unsigned    ang;
    player_t*   player;
    boolean     takedamage;  // false on some case in teamplay

	if(cv_objectplace.value)
		return false;

    if ( !(target->flags & MF_SHOOTABLE) )
        return false; // shouldn't happen...

	if(target->player && target->player->nightsmode && source == target)
		return false;

	// Make sure that boxes CANNOT be popped by enemies!
	// Removed the switch Tails 12-14-2003
	if(target->flags & MF_MONITOR)
	{
		if(!source || !(source->player) || (inflictor && !inflictor->player)) // Graue 12-06-2003
			return false;
	}

    if (target->health <= 0)
        return false;

    if ( target->flags2 & MF2_SKULLFLY )
    {
        // Minotaur is invulnerable during charge attack
//        if(target->type == MT_MINOTAUR)
//            return false;

        target->momx = target->momy = target->momz = 0;
    }

	if(target->flags & MF_BOSS)
	{
		if(target->flags2 & MF2_FRET)
			return false;
	}

	// Special case for Crawla Commander Tails 09-11-2002
	if(target->type == MT_CRAWLACOMMANDER)
	{
		if(target->fuse) // Invincible
			return false;

		if(target->health > 1)
		{
			if(cv_gametype.value == GT_CHAOS && source && source->player)
			{
				source->player->scoreadd++;
				P_AddPlayerScore(source->player, 300*source->player->scoreadd);
			}
			S_StartSound(target, target->info->painsound);
			target->fuse = TICRATE/2;
			target->flags2 |= MF2_FRET;
		}
		else
		{
			target->flags |= MF_NOGRAVITY;
			target->fuse = 0;
		}

		target->momx = target->momy = target->momz = 0;

		P_InstaThrust(target, -target->angle, 5*FRACUNIT);
	}
	else if(target->flags & MF_BOSS)
	{
		if(target->health > 1)
			target->flags2 |= MF2_FRET;

		if(cv_gametype.value == GT_CHAOS && source && source->player)
		{
			source->player->scoreadd++;
			P_AddPlayerScore(source->player, 300*source->player->scoreadd);
		}
	}

    player = target->player;

	if(player && player->exiting)
		return false;

    if(player) // Player is the target
	{
		if(player->nightsmode && !player->powers[pw_flashing])
		{
			int radius;
			const double deg2rad = 0.017453293;
			P_UnsetThingPosition(target);
			player->angle_pos = player->old_angle_pos;
			player->speed /= 5;
			player->flyangle += 180;
			player->flyangle %= 360;

			if(source && source->player)
			{
				if(player->nightstime > 20)
					player->nightstime -= 20;
				else
					player->nightstime = 1;
			}
			else
			{
				if(player->nightstime > 5)
					player->nightstime -= 5;
				else
					player->nightstime = 1;
			}

			radius = target->target->info->radius;

			target->x = target->target->x + cos(player->old_angle_pos * deg2rad) * radius;
			target->y = target->target->y + sin(player->old_angle_pos * deg2rad) * radius;

			target->momx = target->momy = 0;
			P_SetThingPosition(target);
			player->powers[pw_flashing] = flashingtics;
			P_SetMobjState(target->tracer, S_NIGHTSHURT1);
			S_StartSound(target, sfx_nghurt);			
		}

		if(inflictor && (inflictor->flags & MF_FIRE) && player->powers[pw_redshield])
			return false; // Invincible to fire objects

		if(source && source->player) // Player hits another player
		{
			extern consvar_t cv_friendlyfire; // Graue 12-28-2003

			if(source == target) // You can't kill yourself, idiot...
				return false;

			// Graue 12-28-2003
			if(!cv_friendlyfire.value && (cv_gametype.value == GT_COOP ||
				cv_gametype.value == GT_RACE || cv_gametype.value == GT_CHAOS ||
				cv_gametype.value == GT_CIRCUIT))
				return false;

			if((cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CTF) && cv_suddendeath.value
				&& !player->powers[pw_flashing] && !player->powers[pw_invulnerability])
				damage = 10000; // Instant-death!

			if(cv_gametype.value == GT_TAG) // Tag Mode!
			{
				int i;

				// If flashing, or in the tag zone, or invulnerable, ignore the tag.
				if(target->player->powers[pw_flashing] || target->player->tagzone || target->player->powers[pw_invulnerability])
					return false;

				if(player->health > 0)
					CONS_Printf("%s was hit by %s\n",player_names[target->player-players],
                            player_names[source->player-players]);

				// Make the player IT!
				if(source->player->tagit < 298*TICRATE && source->player->tagit > 0)
				{
					target->player->tagit = 300*TICRATE + 1;
					source->player->tagit = 0;
				}

				// Award points to all those who didn't get tagged.
				for(i=0;i<MAXPLAYERS;i++)
				{
					if(playeringame[i])
					{
						if(&players[i] != source->player
							&& &players[i] != target->player)
							players[i].tagcount++;
					}
				}

				target->z++;

				if(target->eflags & MF_UNDERWATER)
					target->momz = 4.04269230769230769230769230769231*FRACUNIT;
				else
					target->momz = 6.9*FRACUNIT;

				if(source->player->mfjumped && source->momz < 0) // Bounce tagger off target
					source->momz = -source->momz;

				ang = R_PointToAngle2 ( inflictor->x,
										inflictor->y,
									    target->x,
								        target->y);

				P_InstaThrust (target, ang, 4*FRACUNIT);
				P_ResetPlayer(target->player);
				target->player->powers[pw_flashing] = flashingtics;
				P_SetMobjState(target, S_PLAY_PAIN);

				// Check for a shield
				if(target->player->powers[pw_redshield] || target->player->powers[pw_blueshield] || target->player->powers[pw_yellowshield] || target->player->powers[pw_blackshield] || target->player->powers[pw_greenshield])
				{
					target->player->powers[pw_redshield] = false;
					target->player->powers[pw_blueshield] = false;      //Get rid of shield
					target->player->powers[pw_yellowshield] = false;
					target->player->powers[pw_blackshield] = false;
					target->player->powers[pw_greenshield] = false;
					S_StartSound (target, sfx_shldls);
					return true;
				}

				if(target->health <= 1) // Death
				{
					P_PlayDeathSound(target);
					P_PlayVictorySound(source); // Killer laughs at you! LAUGHS! BWAHAHAHHAHAA!!
				}
				else if(target->health > 1) // Ring loss
				{
					P_PlayRinglossSound(target);
					P_PlayerRingBurst(target->player, target->player->mo->health - 1);
				}

				target->player->health = target->health = 1;
				return true;
			}
			else if(cv_gametype.value == GT_CTF) // CTF
			{
				// Don't allow players on the same team to hurt one another.
				if(target->player->ctfteam == source->player->ctfteam)
				{
					target->health++;
					target->player->health++;
					return false;
				}
			}

			if(player->health > 0 && !(player->powers[pw_flashing] || player->powers[pw_invulnerability]))
				CONS_Printf("%s was hit by %s\n",player_names[target->player-players],
					player_names[source->player-players]);
		}

		if(damage == 10000)
		{
			player->powers[pw_redshield] = false;
			player->powers[pw_blueshield] = false;      //Get rid of shield
			player->powers[pw_yellowshield] = false;
			player->powers[pw_blackshield] = false;
			player->powers[pw_greenshield] = false;
			player->mo->momx = player->mo->momy = player->mo->momz = 0;

			player->powers[pw_fireflower] = false;
			player->mo->flags =  (player->mo->flags & ~MF_TRANSLATION)
                     | ((player->skincolor)<<MF_TRANSSHIFT);

			if(player->powers[pw_underwater] != 1) // Don't jump up when drowning
				player->mo->momz = 18*FRACUNIT;
			else
				player->mo->momz++;

			if(source && source->type == MT_DISS && source->threshold == 42) // drowned
				S_StartSound(target, sfx_drown);
			else
				P_PlayDeathSound(target);

			P_SetMobjState(target, S_PLAY_DIE1);
			if(cv_gametype.value == GT_CTF && (player->gotflag & MF_REDFLAG || player->gotflag & MF_BLUEFLAG))
				P_PlayerFlagBurst(player);
			if(source && source->player) // Tails 03-13-2001
			{
				P_AddPlayerScore(source->player, 100); // Tails 03-13-2001
			}
		}
		else if ( damage < 10000 // start ignore bouncing & such in invulnerability Tails 02-26-2000
			&& ( (player->cheats&CF_GODMODE)
			|| player->powers[pw_invulnerability] || player->powers[pw_flashing] || player->powers[pw_super]) )
		{
			return false;
		} // end ignore bouncing & such in invulnerability Tails 02-26-2000

		else if ( damage < 10000 && (player->powers[pw_redshield] || player->powers[pw_blueshield] || player->powers[pw_yellowshield] || player->powers[pw_blackshield] || player->powers[pw_greenshield]))  //If One-Hit Shield
		{
			player->powers[pw_blueshield]   = false;      //Get rid of shield
			player->powers[pw_yellowshield] = false;
			player->powers[pw_greenshield]  = false;
			player->powers[pw_redshield] = false;

			if(player->powers[pw_blackshield]) // Give them what's coming to them!
			{
				player->blackow = 1; // BAM!
				player->powers[pw_blackshield] = false;
				player->jumpdown = true;
			}
			damage = 0;                 //Don't take rings away
			player->mo->z++;

			if(player->mo->eflags & MF_UNDERWATER)
					player->mo->momz = 4.04269230769230769230769230769231*FRACUNIT;
			else
					player->mo->momz = 6.9*FRACUNIT;

			if(inflictor == NULL)
				P_InstaThrust (player->mo, -player->mo->angle, 4*FRACUNIT);
			else
			{
				ang = R_PointToAngle2 ( inflictor->x,
										inflictor->y,
										target->x,
										target->y);
				P_InstaThrust (target, ang, 4*FRACUNIT);
			}

			P_SetMobjState(target, target->info->painstate);
			target->player->powers[pw_flashing] = flashingtics; // Tails

			player->powers[pw_fireflower] = false;
			player->mo->flags =  (player->mo->flags & ~MF_TRANSLATION)
                     | ((player->skincolor)<<MF_TRANSSHIFT);

			P_ResetPlayer(player);

			if(source && (source->type == MT_DISS || source->type == MT_FLOORSPIKE || source->type == MT_CEILINGSPIKE) && source->threshold == 43) // spikes
				S_StartSound(target, sfx_spkdth);
			else
				S_StartSound (target, sfx_shldls); // Ba-Dum! Shield loss.

			if(cv_gametype.value == GT_CTF && (player->gotflag & MF_REDFLAG || player->gotflag & MF_BLUEFLAG))
				P_PlayerFlagBurst(player);
			if(source && source->player) // Tails 03-13-2001
			{
				extern consvar_t cv_match_scoring;
				P_AddPlayerScore(source->player, cv_match_scoring.value == 1 ? 25 : 50); // Graue 12-13-2003
			}
			return true;
		}
		else if(player->mo->health > 1) // No shield but have rings.
		{
			// Special stage style damage for circuit mode Graue 12-06-2003
			if(cv_gametype.value == GT_CIRCUIT && player->mo->health > 10) // You idiot, Graue! The health is number of rings PLUS ONE! Nasty bug fixed by Graue 12-22-2003
				damage = 10;
			else
				damage = player->mo->health - 1;

			player->mo->z++;

			player->powers[pw_fireflower] = false;
			player->mo->flags =  (player->mo->flags & ~MF_TRANSLATION)
                     | ((player->skincolor)<<MF_TRANSSHIFT);

			if(player->mo->eflags & MF_UNDERWATER)
				player->mo->momz = 4.04269230769230769230769230769231*FRACUNIT;
			else
				player->mo->momz = 6.9*FRACUNIT;

			if(inflictor == NULL)
				P_InstaThrust (player->mo, -player->mo->angle, 4*FRACUNIT);
			else
			{
				ang = R_PointToAngle2 ( inflictor->x,
										inflictor->y,
										target->x,
										target->y);
				P_InstaThrust (target, ang, 4*FRACUNIT);
			}

			P_ResetPlayer(player);

			if(source && (source->type == MT_DISS || source->type == MT_FLOORSPIKE || source->type == MT_CEILINGSPIKE) && source->threshold == 43) // spikes
				S_StartSound(target, sfx_spkdth);

			// Ring loss sound plays despite hitting spikes
			P_PlayRinglossSound(target); // Ringledingle!

			if(cv_gametype.value == GT_CTF && (player->gotflag & MF_REDFLAG || player->gotflag & MF_BLUEFLAG))
				P_PlayerFlagBurst(player);
			if(source && source->player) // Tails 03-13-2001
			{
				P_AddPlayerScore(source->player, 50); // Tails 03-13-2001
			}
		}
		else // No shield, no rings, no invincibility.
		{
			damage = 1;
			player->mo->momz = 18*FRACUNIT;
			player->mo->momx = player->mo->momy = 0;

			player->powers[pw_fireflower] = false;
			player->mo->flags =  (player->mo->flags & ~MF_TRANSLATION)
                     | ((player->skincolor)<<MF_TRANSSHIFT);

			P_ResetPlayer(player);

			if(source && (source->type == MT_DISS || source->type == MT_FLOORSPIKE || source->type == MT_CEILINGSPIKE) && source->threshold == 43) // Spikes
				S_StartSound(target, sfx_spkdth);
			else
				P_PlayDeathSound(target);

			if(cv_gametype.value == GT_CTF && (player->gotflag & MF_REDFLAG || player->gotflag & MF_BLUEFLAG))
				P_PlayerFlagBurst(player);
			if(source && source->player) // Tails 03-13-2001
			{
				P_AddPlayerScore(source->player, 100); // Tails 03-13-2001
			}
		}
	}

	takedamage = false;
    // player specific
    if (player)
    {
        player->health -= damage;   // mirror mobj health here for Dave

        if (player->health < 0)
            player->health = 0;

        takedamage = true;

		if(damage < 1000)
		{
			P_PlayerRingBurst(player, damage); // SoM

			target->player->powers[pw_flashing] = flashingtics; // Tails
		}

        //added:22-02-98: force feedback ??? electro-shock???
        if (player == &players[consoleplayer])
            I_Tactile (40,10,40+min(damage, 100)*2);
    }
    else
        takedamage = true;

	if(cv_killingdead.value && source && source->player && P_Random() < 192)
		P_DamageMobj(source, target, target, 1);

    if( takedamage )
    {
        // do the damage
        target->health -= damage;
        if (target->health <= 0)
        {
            P_KillMobj ( target, inflictor, source );
            return true;
        }

//        target->flags |= MF_JUSTHIT;    // fight back!

        P_SetMobjState (target, target->info->painstate);

		if(target->player)
			P_ResetPlayer(target->player);

		target->reactiontime = 0;           // we're awake now...
    }

    if (!(target->flags2&(MF2_SKULLFLY)) )
    {
//        target->flags |= MF_JUSTHIT;    // fight back!

		P_SetMobjState (target, target->info->painstate);
    }

    target->reactiontime = 0;           // we're awake now...

	if(source && source != target) // Tails 06-17-2001
    {
        // if not intent on another player,
        // chase after this one
        target->target = source;
        target->threshold = BASETHRESHOLD;
        if (target->state == &states[target->info->spawnstate]
            && target->info->seestate != S_NULL)
            P_SetMobjState (target, target->info->seestate);
    }

    return takedamage;
}

// Made num_rings a parameter Graue 12-06-2003
void P_PlayerRingBurst(player_t* player, int num_rings)
{
	//int       num_rings;
	int       i;
	mobj_t*   mo;

	//If no health, don't spawn ring!
	if(player->mo->health <= 1)
		return;

	// If in sk_nightmare or higher, don't spill rings.
	if(gameskill >= sk_nightmare && !(mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS))
	{
		player->nightsfall = false;
		return;
	}

	/*
	if(player->nightsfall)
		num_rings = player->mo->health - 1;
	else if(player->mo->health > 33)
		num_rings = 32;
	else
		num_rings = player->mo->health - 1; */

	if(num_rings > 32 && !player->nightsfall)
		num_rings = 32;

	for(i = 0; i<num_rings; i++)
	{
		if(player->powers[pw_shieldring])
		{
			mo = P_SpawnMobj(player->mo->x,
							player->mo->y,
							player->mo->z,
							MT_SHIELDRING);
			mo->health = player->powers[pw_shieldring];
			mo->flags2 |= MF2_DONTRESPAWN;
			mo->flags &= ~MF_NOGRAVITY;
			player->powers[pw_shieldring] = 0;
			mo->fuse = 12*TICRATE;
		}
		else if(player->powers[pw_homingring])
		{
			mo = P_SpawnMobj(player->mo->x,
							player->mo->y,
							player->mo->z,
							MT_HOMINGRING);
			mo->health = player->powers[pw_homingring];
			mo->flags2 |= MF2_DONTRESPAWN;
			mo->flags &= ~MF_NOGRAVITY;
			player->powers[pw_homingring] = 0;
			mo->fuse = 12*TICRATE;
		}
		else if(player->powers[pw_railring])
		{
			mo = P_SpawnMobj(player->mo->x,
							player->mo->y,
							player->mo->z,
							MT_RAILRING);
			mo->health = player->powers[pw_railring];
			mo->flags2 |= MF2_DONTRESPAWN;
			mo->flags &= ~MF_NOGRAVITY;
			player->powers[pw_railring] = 0;
			mo->fuse = 12*TICRATE;
		}
		else if(player->powers[pw_automaticring])
		{
			mo = P_SpawnMobj(player->mo->x,
							player->mo->y,
							player->mo->z,
							MT_AUTOMATICRING);
			mo->health = player->powers[pw_automaticring];
			mo->flags2 |= MF2_DONTRESPAWN;
			mo->flags &= ~MF_NOGRAVITY;
			player->powers[pw_automaticring] = 0;
			mo->fuse = 12*TICRATE;
		}
		else if(player->powers[pw_explosionring])
		{
			mo = P_SpawnMobj(player->mo->x,
							player->mo->y,
							player->mo->z,
							MT_EXPLOSIONRING);
			mo->health = player->powers[pw_explosionring];
			mo->flags2 |= MF2_DONTRESPAWN;
			mo->flags &= ~MF_NOGRAVITY;
			player->powers[pw_explosionring] = 0;
			mo->fuse = 12*TICRATE;
		}
		else if (mariomode)
		{
			mo = P_SpawnMobj(player->mo->x,
							 player->mo->y,
							 player->mo->z,
							 MT_FLINGCOIN);

			mo->fuse = 8*TICRATE;
		}
		else
		{
			mo = P_SpawnMobj(player->mo->x,
							 player->mo->y,
							 player->mo->z,
							 MT_FLINGRING);

			mo->fuse = 8*TICRATE;
		}

		// Make rings spill out around the player in 16 directions like SA, but spill like Sonic 2.
		// Technically a non-SA way of spilling rings. They just so happen to be a little similar.
		// Tails 05-11-2001
		if(player->nightsfall)
		{
			mo->momx = sin(i*22.5) * (i/16+2) * FRACUNIT;

			if(!twodlevel)
				mo->momy = cos(i*22.5) * (i/16+2) * FRACUNIT;

			mo->momz = 8*FRACUNIT;
			mo->fuse = 20*TICRATE; // Adjust fuse for NiGHTS
		}
		else
		{
			if(i>15)
			{
				mo->momx = sin(i*22.5) * 3 * FRACUNIT;

				if(!twodlevel)
					mo->momy = cos(i*22.5) * 3 * FRACUNIT;

				mo->momz = 4*FRACUNIT;

				if((i&1) && !(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE))
					mo->momz += 4*FRACUNIT;
			}
			else
			{
				mo->momx = sin(i*22.5) * 2 * FRACUNIT;

				if(!twodlevel)
					mo->momy = cos(i*22.5) * 2 * FRACUNIT;

				mo->momz = 3*FRACUNIT;

				if((i&1) && !(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE))
					mo->momz += 3*FRACUNIT;
			}
		}
	}
	player->nightsfall = false;
	return;
}

// Flag Burst for CTF Tails 08-02-2001
void P_PlayerFlagBurst(player_t* player)
{
  mobj_t*   redflag;
  mobj_t*  blueflag;

  if(!(player->gotflag & MF_REDFLAG || player->gotflag & MF_BLUEFLAG))
	  return;

  if(player->gotflag & MF_REDFLAG)
  {
    redflag = P_SpawnMobj(player->mo->x,
                     player->mo->y,
                     player->mo->z,
                     MT_REDFLAG);

	if(!redflag) // Safety check!!
	  return;

	redflag->momx = sin(P_Random()) * 6 * FRACUNIT;
	redflag->momy = cos(P_Random()) * 6 * FRACUNIT;
    redflag->momz = 8*FRACUNIT;

	redflag->spawnpoint = rflagpoint;
	redflag->fuse = cv_flagtime.value * TICRATE;
	rflagpoint = NULL;

	CONS_Printf("%s dropped the red flag.\n", player_names[player-players]);
  }

  if (player->gotflag & MF_BLUEFLAG)
  {
    blueflag = P_SpawnMobj(player->mo->x,
                     player->mo->y,
                     player->mo->z,
                     MT_BLUEFLAG);

	if(!blueflag) // Safety check!!
	  return;

	blueflag->momx = sin(P_Random()) * 6 * FRACUNIT;
	blueflag->momy = cos(P_Random()) * 6 * FRACUNIT;
    blueflag->momz = 8*FRACUNIT;

	blueflag->spawnpoint = bflagpoint;
	blueflag->fuse = cv_flagtime.value * TICRATE;
	bflagpoint = NULL;

	CONS_Printf("%s dropped the blue flag.\n", player_names[player-players]);
  }

  player->gotflag = 0;
  return;
}