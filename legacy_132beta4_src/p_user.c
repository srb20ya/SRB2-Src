// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_user.c,v 1.15 2001/05/27 13:42:48 bpereira Exp $
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
// $Log: p_user.c,v $
// Revision 1.15  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.14  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.13  2001/03/03 06:17:33  bpereira
// no message
//
// Revision 1.12  2001/01/27 11:02:36  bpereira
// no message
//
// Revision 1.11  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.10  2000/11/04 16:23:43  bpereira
// no message
//
// Revision 1.9  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.8  2000/10/21 08:43:31  bpereira
// no message
//
// Revision 1.7  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.6  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.5  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.4  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.3  2000/03/29 19:39:48  bpereira
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
//      Player related stuff.
//      Bobbing POV/weapon, movement.
//      Pending weapon.
//
//-----------------------------------------------------------------------------
#include "doomdef.h"
#include "d_event.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "s_sound.h"
#include "r_things.h" // Tails 03-01-2000
#include "i_sound.h" // Prototyping to remove warnings Tails
#include "d_think.h" // Tails 04-18-2001
#include "r_sky.h" // Tails 11-18-2001
#include "p_setup.h"
#include "p_inter.h"
#include "m_random.h"
#include "i_video.h"
#include "p_spec.h"
#include "r_splats.h"
#include "z_zone.h"

#include "hardware/hw3sound.h"

extern rendermode_t    rendermode; // Tails 09-08-2002
void HWR_SuperSonicLightToggle(boolean super); // Tails 09-08-2002
extern float grfovadjust;
extern consvar_t cv_grfovchange;

void P_XYMovement(mobj_t* mo);
boolean  P_NukeEnemies (player_t* player);
boolean  PIT_NukeEnemies (mobj_t* thing);
boolean  P_LookForEnemies (player_t* player);
void  P_HomingAttack (mobj_t* source, mobj_t* enemy);
void D_StartTitle(); // Tails
void P_MobjCheckWater(mobj_t* mobj);
void P_LightDash(mobj_t* source, mobj_t* enemy);
void P_LookForRings(player_t* player);
boolean P_RingNearby(player_t* player);

void V_DrawScaledPatch(int x, int y, int scrn, patch_t *patch);
void* W_CachePatchName(char *name, int tag);
void V_DrawString(int x, int y, int option, char *string);
boolean PTR_SlideTraverse (intercept_t* in);

// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP         32


//
// Movement.
//

// 16 pixels of bob
#define MAXBOB  0x100000

boolean         onground;

// Tails 08-13-2002
extern consvar_t cv_playerspeed;
extern consvar_t cv_ringslinger;

extern consvar_t cv_numlaps; // Graue 11-17-2003
extern consvar_t cv_circuit_ringthrow; // Graue 12-06-2003
extern consvar_t cv_circuit_spin; // Graue 12-08-2003
extern consvar_t cv_circuit_specmoves;

//
// P_Thrust
// Moves the given origin along a given angle.
//
void P_Thrust(mobj_t* mo, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;

	if(mo->player)
		move = FixedMul(move, cv_playerspeed.value);

    mo->momx += FixedMul(move, finecosine[angle]);

	if(!twodlevel)
		mo->momy += FixedMul(move, finesine[angle]);
}

// Tails 12-14-2003
void P_ThrustEvenIn2D(mobj_t* mo, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;

	if(mo->player)
		move = FixedMul(move, cv_playerspeed.value);

    mo->momx += FixedMul(move, finecosine[angle]);
	mo->momy += FixedMul(move, finesine[angle]);
}

void P_VectorInstaThrust(fixed_t xa, fixed_t xb, fixed_t xc,
					fixed_t ya, fixed_t yb, fixed_t yc,
					fixed_t za, fixed_t zb, fixed_t zc,
					fixed_t momentum, mobj_t* mo)
{
	fixed_t a1, b1, c1, a2, b2, c2, i, j, k;

	a1 = xb - xa;
	b1 = yb - ya;
	c1 = zb - za;
	a2 = xb - xc;
	b2 = yb - yc;
	c2 = zb - zc;
/*
	// Convert to unit vectors...
	a1 = FixedDiv(a1,sqrt(FixedMul(a1,a1) + FixedMul(b1,b1) + FixedMul(c1,c1)));
	b1 = FixedDiv(b1,sqrt(FixedMul(a1,a1) + FixedMul(b1,b1) + FixedMul(c1,c1)));
	c1 = FixedDiv(c1,sqrt(FixedMul(c1,c1) + FixedMul(c1,c1) + FixedMul(c1,c1)));

	a2 = FixedDiv(a2,sqrt(FixedMul(a2,a2) + FixedMul(c2,c2) + FixedMul(c2,c2)));
	b2 = FixedDiv(b2,sqrt(FixedMul(a2,a2) + FixedMul(c2,c2) + FixedMul(c2,c2)));
	c2 = FixedDiv(c2,sqrt(FixedMul(a2,a2) + FixedMul(c2,c2) + FixedMul(c2,c2)));
*/
	// Calculate the momx, momy, and momz
	i = FixedMul(momentum, FixedMul(b1,c2) - FixedMul(c1,b2));
	j = FixedMul(momentum, FixedMul(c1,a2) - FixedMul(a1,c2));
	k = FixedMul(momentum, FixedMul(a1,b2) - FixedMul(a1,c2));

	mo->momx = i;
	mo->momy = j;
	mo->momz = k;
}

//
// P_InstaThrust // Tails
// Moves the given origin along a given angle instantly.
//
void P_InstaThrust ( mobj_t*       mo,
                     angle_t       angle,
                     fixed_t       move )
{
    angle >>= ANGLETOFINESHIFT;

    mo->momx = FixedMul(move,finecosine[angle]);

	if(!twodlevel)
		mo->momy = FixedMul(move,finesine[angle]);
}

// Tails 12-14-2003
void P_InstaThrustEvenIn2D ( mobj_t*       mo,
                     angle_t       angle,
                     fixed_t       move )
{
    angle >>= ANGLETOFINESHIFT;

    mo->momx = FixedMul(move,finecosine[angle]);
	mo->momy = FixedMul(move,finesine[angle]);
}

//
// P_InstaZThrust // Tails
// Moves the given origin along a given angle instantly.
//
void P_InstaZThrust ( mobj_t*       mo,
                     angle_t       angle,
                     fixed_t       move )
{
    mo->momz = move;

	if(mo->player)
		mo->momz += (((angle >> FRACBITS)*FRACUNIT)/128)*(mo->player->speed/32.0);
}

// Returns a location (hard to explain - go see how it is used)
fixed_t P_ReturnThrustX ( mobj_t*       mo,
                     angle_t       angle,
                     fixed_t       move )
{
    angle >>= ANGLETOFINESHIFT;

    return FixedMul(move,finecosine[angle]);
}
fixed_t P_ReturnThrustY ( mobj_t*       mo,
                     angle_t       angle,
                     fixed_t       move )
{
    angle >>= ANGLETOFINESHIFT;

    return FixedMul(move,finesine[angle]);
}

#ifdef CLIENTPREDICTION2
//
// P_ThrustSpirit
// Moves the given origin along a given angle.
//
void P_ThrustSpirit(player_t *player, angle_t angle, fixed_t move)
{
    angle >>= ANGLETOFINESHIFT;
    if(player->spirit->subsector->sector->special == 15
    && !(player->powers[pw_flight] && !(player->spirit->z <= player->spirit->floorz))) // Friction_Low
    {
        player->spirit->momx += FixedMul(move>>2, finecosine[angle]);
        player->spirit->momy += FixedMul(move>>2, finesine[angle]);
    }
    else
    {
        player->spirit->momx += FixedMul(move, finecosine[angle]);
        player->spirit->momy += FixedMul(move, finesine[angle]);
    }
}
#endif

//
// P_CalcHeight
// Calculate the walking / running height adjustment
//
void P_CalcHeight (player_t* player)
{
    int         angle;
    fixed_t     bob;
    fixed_t     viewheight;
    mobj_t      *mo;

    // Regular movement bobbing
    // (needs to be calculated for gun swing
    // even if not on ground)
    // OPTIMIZE: tablify angle
    // Note: a LUT allows for effects
    //  like a ramp with low health.

	    mo = player->mo;
#ifdef CLIENTPREDICTION2
    if( player->spirit )
        mo = player->spirit;
#endif

    player->bob = ((FixedMul (player->rmomx,player->rmomx)
                   +FixedMul (player->rmomy,player->rmomy))*NEWTICRATERATIO)>>2;

    if (player->bob>MAXBOB)
        player->bob = MAXBOB;

    if ((player->cheats & CF_NOMOMENTUM) || mo->z > mo->floorz)
    {
        //added:15-02-98: it seems to be useless code!
        //player->viewz = player->mo->z + (cv_viewheight.value<<FRACBITS);

        //if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
        //    player->viewz = player->mo->ceilingz-4*FRACUNIT;
        player->viewz = mo->z + player->viewheight;
        return;
    }

    angle = (FINEANGLES/20*localgametic/NEWTICRATERATIO)&FINEMASK;
    bob = FixedMul ( player->bob/2, finesine[angle]);

    // move viewheight
    viewheight = cv_viewheight.value << FRACBITS; // default eye view height

    if (player->playerstate == PST_LIVE)
    {
        player->viewheight += player->deltaviewheight;

        if (player->viewheight > viewheight)
        {
            player->viewheight = viewheight;
            player->deltaviewheight = 0;
        }

        if (player->viewheight < viewheight/2)
        {
            player->viewheight = viewheight/2;
            if (player->deltaviewheight <= 0)
                player->deltaviewheight = 1;
        }

        if (player->deltaviewheight)
        {
            player->deltaviewheight += FRACUNIT/4;
            if (!player->deltaviewheight)
                player->deltaviewheight = 1;
        }
    }   

    player->viewz = mo->z + player->viewheight + bob;

    if (player->viewz > mo->ceilingz-4*FRACUNIT)
        player->viewz = mo->ceilingz-4*FRACUNIT;
    if (player->viewz < mo->floorz+4*FRACUNIT)
        player->viewz = mo->floorz+4*FRACUNIT;

}


extern int ticruned,ticmiss;

extern consvar_t cv_homing; // Tails 07-02-2001
extern consvar_t cv_lightdash;
extern consvar_t cv_numsnow; // Tails 12-25-2001
extern consvar_t cv_storm; // Tails 06-07-2002
extern consvar_t cv_shadow;

//
// P_ResetScore
//
// This is called when your chain is reset. If in
// Chaos mode, it displays what chain you got.
void P_ResetScore(player_t* player)
{
	if(cv_gametype.value == GT_CHAOS && player->scoreadd >= 5)
		CONS_Printf("%s got a chain of %d!\n", player_names[player-players], player->scoreadd);

	player->scoreadd = 0;
}

// P_TransferToNextMare
//
// Transfers the player to the next Mare.
// Returns true if successful, false if there is no other mare.
boolean P_TransferToNextMare(player_t* player)
{
	thinker_t*  th;
	mobj_t*     mo2;
	int first = 0;
	mobj_t* closestaxis;
	int axisnum;
	int mare;

	if(!player->mo->target)
		return false;

	axisnum = player->mo->target->health;
	mare = player->mare + 1;
/*
	player->axishit = true; // Hit an axis transfer point.

	if(player->axistransferred)
	{
		if(player->transferangle >= 0)
		{
			if((player->transferangle < 90 || player->transferangle >= 270)
				&& (player->flyangle < 90 || player->flyangle >= 270))
				return;

			if((player->transferangle >= 90 && player->transferangle < 270)
				&& (player->flyangle >= 90 && player->flyangle < 270))
				return;
		}
	}

	player->axistransferred = true;
	player->transferangle = player->anotherflyangle;

	if(cv_debug)
		CONS_Printf("Transferring to axis %d\nLeveltime: %d\n", axisnum,leveltime);
*/
	closestaxis = NULL;

	// scan the thinkers
	// to find the closest axis point
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;

		if(first == 0)
		{
			if(mo2->type == MT_AXIS1
				|| mo2->type == MT_AXIS2
				|| mo2->type == MT_AXIS3
				|| mo2->type == MT_AXIS1A
				|| mo2->type == MT_AXIS2A
				|| mo2->type == MT_AXIS3A)
			{
				if(mo2->health == axisnum && mo2->threshold == mare)
				{
					closestaxis = mo2;
					first++;
				}
			}
		}
		else
		{
			if(mo2->type == MT_AXIS1
				|| mo2->type == MT_AXIS2
				|| mo2->type == MT_AXIS3
				|| mo2->type == MT_AXIS1A
				|| mo2->type == MT_AXIS2A
				|| mo2->type == MT_AXIS3A)
			{
				fixed_t dist1, dist2;
				
				if(mo2->health == axisnum && mo2->threshold == mare)
				{
					dist1 = R_PointToDist2(player->mo->x, player->mo->y, mo2->x, mo2->y)-mo2->info->radius;
					dist2 = R_PointToDist2(player->mo->x, player->mo->y, closestaxis->x, closestaxis->y)-closestaxis->info->radius;

					if(dist1 < dist2)
						closestaxis = mo2;
				}
			}
		}
	}

	if(closestaxis == NULL)
		return false;

	player->mo->target = closestaxis;
	return true;
}

//
// P_TransferToAxis
//
// Finds the CLOSEST axis with the number specified.
void P_TransferToAxis(player_t* player, int axisnum)
{
	thinker_t*  th;
	mobj_t*     mo2;
	int first = 0;
	mobj_t* closestaxis;

	player->axishit = true; // Hit an axis transfer point.

	if(player->axistransferred)
	{
		if(player->transferangle >= 0)
		{
			if((player->transferangle < 90 || player->transferangle >= 270)
				&& (player->flyangle < 90 || player->flyangle >= 270))
				return;

			if((player->transferangle >= 90 && player->transferangle < 270)
				&& (player->flyangle >= 90 && player->flyangle < 270))
				return;
		}
	}

	player->axistransferred = true;
	player->transferangle = player->anotherflyangle;

	if(cv_debug)
		CONS_Printf("Transferring to axis %d\nLeveltime: %d\n", axisnum,leveltime);

	closestaxis = NULL;

	// scan the thinkers
	// to find the closest axis point
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;

		if(first == 0)
		{
			if(mo2->type == MT_AXIS1
				|| mo2->type == MT_AXIS2
				|| mo2->type == MT_AXIS3
				|| mo2->type == MT_AXIS1A
				|| mo2->type == MT_AXIS2A
				|| mo2->type == MT_AXIS3A)
			{
				if(mo2->health == axisnum)
				{
					closestaxis = mo2;
					first++;
				}
			}
		}
		else
		{
			if(mo2->type == MT_AXIS1
				|| mo2->type == MT_AXIS2
				|| mo2->type == MT_AXIS3
				|| mo2->type == MT_AXIS1A
				|| mo2->type == MT_AXIS2A
				|| mo2->type == MT_AXIS3A)
			{
				fixed_t dist1, dist2;
				
				if(mo2->health == axisnum)
				{
					dist1 = R_PointToDist2(player->mo->x, player->mo->y, mo2->x, mo2->y)-mo2->info->radius;
					dist2 = R_PointToDist2(player->mo->x, player->mo->y, closestaxis->x, closestaxis->y)-closestaxis->info->radius;

					if(dist1 < dist2)
						closestaxis = mo2;
				}
			}
		}
	}

	if(closestaxis == NULL)
		CONS_Printf("ERROR: Specified axis point to transfer to not found!\n%d\n", axisnum);

	player->mo->target = closestaxis;
}

//
// P_GetClosestAxis
//
// Finds the CLOSEST axis to the source mobj
mobj_t* P_GetClosestAxis(mobj_t* source)
{
	thinker_t*  th;
	mobj_t*     mo2;
	int first = 0;
	mobj_t* closestaxis;

	closestaxis = NULL;

	// scan the thinkers
	// to find the closest axis point
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;

		if(first == 0)
		{
			if(mo2->type == MT_AXIS1
				|| mo2->type == MT_AXIS2
				|| mo2->type == MT_AXIS3
				|| mo2->type == MT_AXIS1A
				|| mo2->type == MT_AXIS2A
				|| mo2->type == MT_AXIS3A)
			{
				closestaxis = mo2;
				first++;
			}
		}
		else
		{
			if(mo2->type == MT_AXIS1
				|| mo2->type == MT_AXIS2
				|| mo2->type == MT_AXIS3
				|| mo2->type == MT_AXIS1A
				|| mo2->type == MT_AXIS2A
				|| mo2->type == MT_AXIS3A)
			{
				fixed_t dist1, dist2;
				
				dist1 = R_PointToDist2(source->x, source->y, mo2->x, mo2->y)-mo2->info->radius;
				dist2 = R_PointToDist2(source->x, source->y, closestaxis->x, closestaxis->y)-closestaxis->info->radius;

				if(dist1 < dist2)
					closestaxis = mo2;
			}
		}
	}

	if(closestaxis == NULL)
		CONS_Printf("ERROR: No axis points found!\n");

	return closestaxis;
}

//
// P_DeNightserizePlayer
//
// Whoops! Ran out of NiGHTS time!
// Tails 10-20-2002
void P_DeNightserizePlayer(player_t* player)
{
	player->nightsmode = false;

	P_RemoveMobj(player->mo->tracer);
	player->usedown = false;
	player->jumpdown = false;
	player->attackdown = false;
	player->walking = 0;
	player->running = 0;
	player->spinning = 0;
	player->jumping = 0;
	player->homing = 0;
	player->mo->target = NULL;
	player->mo->fuse = 0;
	player->speed = 0;
	player->mfstartdash = 0;
	player->mfjumped = 0;
	player->thokked = false;
	player->mfspinning = 0;
	player->redxvi = 0;
	player->drilling = 0;

	player->mo->flags &= ~MF_NOGRAVITY;

	player->mo->flags2 &= ~MF2_DONTDRAW;

	if(cv_splitscreen.value && player == &players[secondarydisplayplayer])
	{
		if(cv_analog2.value)
			CV_SetValue(&cv_cam2_dist, 192);
		else
			CV_SetValue(&cv_cam2_dist, 128);
	}
	else if (player == &players[displayplayer])
	{
		if(cv_analog.value)
			CV_SetValue(&cv_cam_dist, 192);
		else
			CV_SetValue(&cv_cam_dist, 128);
	}

	P_SetMobjState(player->mo->tracer, S_DISS);
	P_SetMobjState(player->mo, S_PLAY_FALL1);
	player->nightsfall = true;
}
//
// P_NightserizePlayer
//
// NiGHTS Time!
// Tails 10-20-2002
void P_NightserizePlayer(player_t* player, int time, boolean nextmare)
{
	player->nightsmode = true;

	player->usedown = false;
	player->jumpdown = false;
	player->attackdown = false;
	player->walking = 0;
	player->running = 0;
	player->spinning = 0;
	player->homing = 0;

	if(!nextmare)
		player->mo->target = NULL;

	player->mo->fuse = 0;
	player->speed = 0;
	player->mfstartdash = 0;
	player->mfjumped = 0;
	player->thokked = false;
	player->mfspinning = 0;
	player->redxvi = 0;
	player->drilling = 0;

	player->powers[pw_blueshield] = 0;
	player->powers[pw_redshield] = 0;
	player->powers[pw_greenshield] = 0;
	player->powers[pw_blackshield] = 0;
	player->powers[pw_yellowshield] = 0;

	player->mo->flags |= MF_NOGRAVITY;

	player->mo->flags2 |= MF2_DONTDRAW;

	if(cv_splitscreen.value && player == &players[secondarydisplayplayer])
		CV_SetValue(&cv_cam2_dist, 320);
	else if (player == &players[displayplayer])
		CV_SetValue(&cv_cam_dist, 320);

	player->nightstime = time;
	player->bonustime = false;

	P_SetMobjState(player->mo->tracer, S_SUPERTRANS1);

	if(player->drillmeter < 40*20)
		player->drillmeter = 40*20;
}

// Useful when you want to kill everything the player is doing.
void P_ResetPlayer(player_t* player)
{
	player->mfspinning = 0;
	player->mfjumped = 0;
	player->gliding = 0;
	player->glidetime = 0;
	player->climbing = 0;
	player->powers[pw_tailsfly] = 0;
	player->thokked = false;
	player->onconveyor = 0; // Graue 12-26-2003
}

// Adds to the player's score
void P_AddPlayerScore(player_t* player, int amount)
{
	extern consvar_t cv_fraglimit, cv_ctf_scoring; // Graue 12-13-2003

	player->score += amount;

	if(cv_gametype.value == GT_CTF && cv_ctf_scoring.value == 0) // CTF normal scoring
	{
		if(player->ctfteam == 1) // Red
			redscore += amount;
		else if(player->ctfteam == 2) // Blue
			bluescore += amount;
	}

	if(cv_fraglimit.value)
		P_CheckFragLimit(player);
}

//
// P_DoPlayerExit
//
// Player exits the map via sector trigger Tails 12-03-2002
void P_DoPlayerExit(player_t* player)
{
	int i;

	if(player->exiting)
		return;

	if(cv_allowexitlevel.value == 0 && (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CTF || cv_gametype.value == GT_CHAOS))
	{
		return;
	}
	else if(cv_gametype.value == GT_RACE || cv_gametype.value == GT_CIRCUIT) // If in Race or Circuit Mode, allow
	{
		extern consvar_t cv_countdowntime;

		if(!countdown) // a 60-second wait ala Sonic 2. Tails 04-25-2001
			countdown = cv_countdowntime.value*TICRATE + 1; // Use cv_countdowntime Graue 11-20-2003

		player->exiting = 3*TICRATE;

		if(!countdown2)
			countdown2 = 71*TICRATE + 1;

		// Check if all the players in the race have finished. If so, end the level.
		for(i = 0; i < MAXPLAYERS; i++)
		{
			if(playeringame[i])
			{
				if(!players[i].exiting)
				{
					if(players[i].lives > 0)
						break;
				}
			}
		}

		if(i == MAXPLAYERS)  // finished
		{
			player->exiting = 2.8*TICRATE + 1;
			countdown = 0;
			countdown2 = 0;
		}
	}
	else
		player->exiting = 2.8*TICRATE + 1;

	player->gliding = 0;
	player->climbing = 0;
}

#define SPACESPECIAL 6
boolean P_InSpaceSector(mobj_t* mo) // Returns true if you are in space or quicksand
{
	sector_t* sector;

	sector = mo->subsector->sector;

	if(sector->special == SPACESPECIAL)
		return true;

	if(sector->ffloors)
	{
		ffloor_t* rover;

		for(rover = sector->ffloors; rover; rover = rover->next)
		{
			if(!(rover->flags & FF_QUICKSAND) && rover->master->frontsector->special != SPACESPECIAL)
			  continue;

			if(rover->flags & FF_QUICKSAND)
			{
				if(mo->z + (mo->height/2) > *rover->topheight)
					continue;
			}

			if(mo->z > *rover->topheight)
			  continue;

			if(mo->z + (mo->height/2) < *rover->bottomheight)
			  continue;
			
			return true;
		}
	}

	return false; // No vacuum here, Captain!
}

boolean P_InQuicksand(mobj_t* mo) // Returns true if you are in quicksand
{
	sector_t* sector;

	sector = mo->subsector->sector;

	if(sector->ffloors)
	{
		ffloor_t* rover;

		for(rover = sector->ffloors; rover; rover = rover->next)
		{
			if(!(rover->flags & FF_QUICKSAND))
			  continue;

			if(mo->z + (mo->height/2) > *rover->topheight)
			  continue;

			if(mo->z > *rover->topheight)
			  continue;

			if(mo->z + (mo->height/2) < *rover->bottomheight)
			  continue;
			
			return true;
		}
	}

	return false; // No sand here, Captain!
}

extern consvar_t cv_objectplace;
extern consvar_t cv_snapto;
extern consvar_t cv_speed;
extern consvar_t cv_objflags;
extern consvar_t cv_mapthingnum;
extern consvar_t cv_grid;
void P_SpawnHoopsAndRings(mapthing_t* mthing);

void P_GolfMovement(player_t* player)
{
    ticcmd_t*           cmd;

    cmd = &player->cmd;

	cmd->forwardmove = cmd->sidemove = 0; // Thou shalt not cheat!

// Start lots of cmomx/y stuff Tails 04-18-2001
// CMOMx stands for the conveyor belt speed.
// cmomx/y stuff modified to use onconveyor by Graue 12-26-2003
	if(player->onconveyor == 984)
	{
		if(player->mo->z > player->mo->watertop || player->mo->z + player->mo->height < player->mo->waterbottom)
			player->cmomx = player->cmomy = 0;
	}

	else if(player->onconveyor == 985 && player->mo->z > player->mo->floorz)
		player->cmomx = player->cmomy = 0;
	
	else if(player->onconveyor != 984 && player->onconveyor != 985)
		player->cmomx = player->cmomy = 0;

	player->rmomx = player->mo->momx - player->cmomx;
	player->rmomy = player->mo->momy - player->cmomy;
// End lots of cmomx/y stuff Tails 04-18-2001
// Start various movement calculations Tails 10-08-2000
// This determines if the player is facing the direction they are travelling or not.
// Didn't your teacher say to pay attention in Geometry/Trigonometry class? ;)

// Calculates player's speed based on distance-of-a-line formula
//	player->speed = R_PointToDist2(player->mo->x + player->rmomx, player->mo->y + player->rmomy, player->mo->x, player->mo->y) >> FRACBITS;
	player->speed = P_AproxDistance((player->mo->x + player->rmomx) - player->mo->x, (player->mo->y + player->rmomy) - player->mo->y) >> FRACBITS; // Player's Speed Tails 08-22-2000

	player->aiming = cmd->aiming<<16;

    onground = (player->mo->z <= player->mo->floorz) 
               || (player->cheats & CF_FLYAROUND)
               || (player->mo->flags2&(MF2_ONMOBJ));
}

// Control scheme for 2d levels. Tails 05-24-2003
void P_2dMovement(player_t* player)
{
    ticcmd_t*           cmd;
	int topspeed;
	int acceleration;
	int thrustfactor;
    fixed_t   movepushforward=0;
	angle_t   movepushangle=0;
	const int runspeed = 28;

    cmd = &player->cmd;

	if(player->exiting)
		cmd->forwardmove = cmd->sidemove = 0;

// Start lots of cmomx/y stuff Tails 04-18-2001
// CMOMx stands for the conveyor belt speed.
	if(player->onconveyor == 984)
	{
		if(player->mo->z > player->mo->watertop || player->mo->z + player->mo->height < player->mo->waterbottom)
			player->cmomx = player->cmomy = 0;
	}

	else if(player->onconveyor == 985 && player->mo->z > player->mo->floorz)
		player->cmomx = player->cmomy = 0;
	
	else if(player->onconveyor != 984 && player->onconveyor != 985)
		player->cmomx = player->cmomy = 0;

	player->rmomx = player->mo->momx - player->cmomx;
	player->rmomy = player->mo->momy - player->cmomy;
// End lots of cmomx/y stuff Tails 04-18-2001

// Start various movement calculations Tails 10-08-2000
// This determines if the player is facing the direction they are travelling or not.
// Didn't your teacher say to pay attention in Geometry/Trigonometry class? ;)

// Calculates player's speed based on distance-of-a-line formula
	player->speed = abs(player->rmomx >> FRACBITS);

// End various movement calculations Tails

	if(player->gliding)
	{
		if(cmd->sidemove > 0 && player->mo->angle != 0 && player->mo->angle >= ANG180)
			player->mo->angle += (640/NEWTICRATERATIO)<<16;
		else if(cmd->sidemove < 0 && player->mo->angle != ANG180 && (player->mo->angle > ANG180 || player->mo->angle == 0))
			player->mo->angle -= (640/NEWTICRATERATIO)<<16;
		else if(!cmd->sidemove)
		{
			if(player->mo->angle >= ANG270)
				player->mo->angle += (640/NEWTICRATERATIO)<<16;
			else if(player->mo->angle < ANG270 && player->mo->angle > ANG180)
				player->mo->angle -= (640/NEWTICRATERATIO)<<16;
		}
	}
	else
	{
		if((player->rmomx >> FRACBITS) > 0)
			player->mo->angle = 0;
		else if((player->rmomx >> FRACBITS) < 0)
			player->mo->angle = ANG180;
	}

	localangle = player->mo->angle;

	if(player->gliding)
	{
		movepushangle = player->mo->angle;
	}
	else
	{
		if(cmd->sidemove > 0)
			movepushangle = 0;
		else if(cmd->sidemove < 0)
			movepushangle = ANG180;
		else
			movepushangle = player->mo->angle;
	}

    // Do not let the player control movement
    //  if not onground.
    onground = (player->mo->z <= player->mo->floorz) 
               || (player->cheats & CF_FLYAROUND)
               || (player->mo->flags2&(MF2_ONMOBJ));

	player->aiming = cmd->aiming<<16;

	// Set the player speeds.
	if(player->powers[pw_super] || player->powers[pw_sneakers])
	{
		thrustfactor = player->thrustfactor*2;
		acceleration = player->accelstart/4 + player->speed*(player->acceleration/4);

		if(player->powers[pw_tailsfly])
		{
			topspeed = player->normalspeed;
		}
		else if(player->mo->eflags & MF_UNDERWATER)
		{
			topspeed = player->normalspeed;
			acceleration = (acceleration * 2) / 3;
		}
		else
		{
			topspeed = player->normalspeed * 2 > 50 ? 50 : player->normalspeed * 2;
		}
	}
	else
	{
		thrustfactor = player->thrustfactor;
		acceleration = player->accelstart + player->speed*player->acceleration;

		if(player->powers[pw_tailsfly])
		{
			topspeed = player->normalspeed/2;
		}
		else if(player->mo->eflags & MF_UNDERWATER)
		{
			topspeed = player->normalspeed/2;
			acceleration = (acceleration * 2) / 3;
		}
		else
		{
			topspeed = player->normalspeed;
		}
	}

//////////////////////////////////////
	if(player->climbing)
	{
		player->mo->momz = (cmd->forwardmove/10)*FRACUNIT;
	}

	if (cmd->sidemove
		&& !(player->climbing || player->gliding || player->exiting
		|| (player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_flashing]
		&& !onground)))
	{
		if(player->powers[pw_sneakers] || player->powers[pw_super]) // do you have super sneakers? Tails 02-28-2000
		{
			movepushforward = abs(cmd->sidemove) * ((thrustfactor*2)*acceleration); // then go faster!! Tails 02-28-2000
		}
		else // if not, then run normally Tails 02-28-2000
		{
			movepushforward = abs(cmd->sidemove) * (thrustfactor*acceleration); // Changed by Tails: 9-14-99
		}
        
		// allow very small movement while in air for gameplay
		if (!onground)
		{  
			movepushforward >>= 2; // Proper air movement - Changed by Tails: 9-13-99
		}

		// Allow a bit of movement while spinning Tails
		if (player->mfspinning)
		{
			if(!player->mfstartdash)
				movepushforward=movepushforward/48;
			else
				movepushforward = 0;
		}

		if(((player->rmomx >> FRACBITS) < topspeed) && (cmd->sidemove > 0)) // Sonic's Speed
			P_Thrust (player->mo, movepushangle, movepushforward);
		else if(((player->rmomx >> FRACBITS) > -topspeed) && (cmd->sidemove < 0))
			P_Thrust (player->mo, movepushangle, movepushforward);
	}		
}

void P_3dMovement(player_t* player)
{
    ticcmd_t*           cmd;
	int					tag = 0; // Tails 05-08-2001
	angle_t				movepushangle; // Analog Test Tails 06-10-2001
	angle_t				movepushsideangle; // Analog Test Tails 06-10-2001
	int topspeed;
	int acceleration;
	int thrustfactor;
    fixed_t   movepushforward=0,movepushside=0;
	const int runspeed = 28;
	int mforward=0, mbackward=0;
	camera_t* thiscam;

	if(cv_splitscreen.value && player == &players[secondarydisplayplayer])
		thiscam = &camera2;
	else
		thiscam = &camera;

    cmd = &player->cmd;

	if(player->exiting)
		cmd->forwardmove = cmd->sidemove = 0;

	if(!netgame && ((player == &players[consoleplayer] && cv_analog.value) || (cv_splitscreen.value && player == &players[secondarydisplayplayer] && cv_analog2.value)))
	{
		movepushangle = thiscam->angle;
		movepushsideangle = thiscam->angle-ANG90;
	}
	else
	{
		movepushangle = player->mo->angle;
		movepushsideangle = player->mo->angle-ANG90;
	}

// Start lots of cmomx/y stuff Tails 04-18-2001
// CMOMx stands for the conveyor belt speed.
// cmomx/y stuff modified to use onconveyor by Graue 12-26-2003
	if(player->onconveyor == 984)
	{
		if(player->mo->z > player->mo->watertop || player->mo->z + player->mo->height < player->mo->waterbottom)
			player->cmomx = player->cmomy = 0;
	}

	else if(player->onconveyor == 985 && player->mo->z > player->mo->floorz)
		player->cmomx = player->cmomy = 0;
	
	else if(player->onconveyor != 984 && player->onconveyor != 985)
		player->cmomx = player->cmomy = 0;

	player->rmomx = player->mo->momx - player->cmomx;
	player->rmomy = player->mo->momy - player->cmomy;
// End lots of cmomx/y stuff Tails 04-18-2001
// Start various movement calculations Tails 10-08-2000
// This determines if the player is facing the direction they are travelling or not.
// Didn't your teacher say to pay attention in Geometry/Trigonometry class? ;)

// Calculates player's speed based on distance-of-a-line formula
//	player->speed = R_PointToDist2(player->mo->x + player->rmomx, player->mo->y + player->rmomy, player->mo->x, player->mo->y) >> FRACBITS;
	player->speed = P_AproxDistance((player->mo->x + player->rmomx) - player->mo->x, (player->mo->y + player->rmomy) - player->mo->y) >> FRACBITS; // Player's Speed Tails 08-22-2000

// forward
	if ((player->rmomx > 0 && player->rmomy > 0) && (player->mo->angle >= 0 && player->mo->angle < ANG90)) // Quadrant 1
		mforward = 1;
	else if ((player->rmomx < 0 && player->rmomy > 0) && (player->mo->angle >= ANG90 && player->mo->angle < ANG180)) // Quadrant 2
		mforward = 1;
	else if ((player->rmomx < 0 && player->rmomy < 0) && (player->mo->angle >= ANG180 && player->mo->angle < ANG270)) // Quadrant 3
		mforward = 1;
	else if ((player->rmomx > 0 && player->rmomy < 0) && ((player->mo->angle >= ANG270 && (player->mo->angle <= ANGLE_MAX << FRACBITS)) || (player->mo->angle >= 0 && player->mo->angle <= ANG45))) // Quadrant 4
		mforward = 1;
	else if (player->rmomx > 0 && ((player->mo->angle >= ANG270+ANG45 && player->mo->angle <= ANGLE_MAX << FRACBITS)))
		mforward = 1;
	else if (player->rmomx < 0 && (player->mo->angle >= ANG90+ANG45 && player->mo->angle <= ANG180+ANG45))
		mforward = 1;
	else if (player->rmomy > 0 && (player->mo->angle >= ANG45 && player->mo->angle <= ANG90+ANG45))
		mforward = 1;
	else if (player->rmomy < 0 && (player->mo->angle >= ANG180+ANG45 && player->mo->angle <= ANG270+ANG45))
		mforward = 1;
	else
		mforward = 0;
// backward
	if ((player->rmomx > 0 && player->rmomy > 0) && (player->mo->angle >= ANG180 && player->mo->angle < ANG270)) // Quadrant 3
		mbackward = 1;
	else if ((player->rmomx < 0 && player->rmomy > 0) && (player->mo->angle >= ANG270 && (player->mo->angle <= ANGLE_MAX << FRACBITS))) // Quadrant 4
		mbackward = 1;
	else if ((player->rmomx < 0 && player->rmomy < 0) && (player->mo->angle >= 0 && player->mo->angle < ANG90)) // Quadrant 1
		mbackward = 1;
	else if ((player->rmomx > 0 && player->rmomy < 0) && (player->mo->angle >= ANG90 && player->mo->angle < ANG180)) // Quadrant 2
		mbackward = 1;
	else if (player->rmomx < 0 && ((player->mo->angle >= ANG270+ANG45 && player->mo->angle <= ANGLE_MAX << FRACBITS) || (player->mo->angle >= 0 && player->mo->angle <= ANG45)))
		mbackward = 1;
	else if (player->rmomx > 0 && (player->mo->angle >= ANG90+ANG45 && player->mo->angle <= ANG180+ANG45))
		mbackward = 1;
	else if (player->rmomy < 0 && (player->mo->angle >= ANG45 && player->mo->angle <= ANG90+ANG45))
		mbackward = 1;
	else if (player->rmomy > 0 && (player->mo->angle >= ANG180+ANG45 && player->mo->angle <= ANG270+ANG45))
		mbackward = 1;
	else // Put in 'or' checks here!
		mbackward = 0;
		
// End various movement calculations Tails

    // Do not let the player control movement
    //  if not onground.
    onground = (player->mo->z <= player->mo->floorz) 
               || (player->cheats & CF_FLYAROUND)
               || (player->mo->flags2&(MF2_ONMOBJ));

	player->aiming = cmd->aiming<<16;

	// Set the player speeds.
	if(player->powers[pw_super] || player->powers[pw_sneakers])
	{
		thrustfactor = player->thrustfactor*2;
		acceleration = player->accelstart/4 + player->speed*(player->acceleration/4);

		if(player->powers[pw_tailsfly])
		{
			topspeed = player->normalspeed;
		}
		else if(player->mo->eflags & MF_UNDERWATER)
		{
			topspeed = player->normalspeed;
			acceleration = (acceleration * 2) / 3;
		}
		else
		{
			topspeed = player->normalspeed * 2 > 50 ? 50 : player->normalspeed * 2;
		}
	}
	else
	{
		thrustfactor = player->thrustfactor;
		acceleration = player->accelstart + player->speed*player->acceleration;

		if(player->powers[pw_tailsfly])
		{
			topspeed = player->normalspeed/2;
		}
		else if(player->mo->eflags & MF_UNDERWATER)
		{
			topspeed = player->normalspeed/2;
			acceleration = (acceleration * 2) / 3;
		}
		else
		{
			topspeed = player->normalspeed;
		}
	}

//////////////////////////////////////
	if ((netgame
		|| (player == &players[consoleplayer] && !cv_analog.value)
		|| (cv_splitscreen.value && player == &players[secondarydisplayplayer] && !cv_analog2.value))
		&& cmd->forwardmove
		&& !(player->gliding || player->exiting
		|| (player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_flashing]
		&& !onground)))
	{
		if(player->climbing)
		{
			player->mo->momz = (cmd->forwardmove/10)*FRACUNIT;
		}

		else if(player->powers[pw_sneakers] || player->powers[pw_super]) // do you have super sneakers? Tails 02-28-2000
		{
			movepushforward = cmd->forwardmove * ((thrustfactor*2)*acceleration); // then go faster!! Tails 02-28-2000
		}
		else // if not, then run normally Tails 02-28-2000
		{
			movepushforward = cmd->forwardmove * (thrustfactor*acceleration); // Changed by Tails: 9-14-99
		}
        
		// allow very small movement while in air for gameplay
		if (!onground)
		{  
			movepushforward >>= 2; // Proper air movement - Changed by Tails: 9-13-99
		}

		// Allow a bit of movement while spinning Tails
		if (player->mfspinning)
		{
			if((mforward && cmd->forwardmove > 0)
				|| (mbackward && cmd->forwardmove < 0))
				movepushforward = 0;
			else if(!player->mfstartdash)
				movepushforward=movepushforward/16;
			else
				movepushforward = 0;
		}

		if((player->speed < topspeed) && (mforward) && (cmd->forwardmove > 0)) // Sonic's Speed
			P_Thrust (player->mo, movepushangle, movepushforward);
		else if((mforward) && (cmd->forwardmove < 0))
			P_Thrust (player->mo, movepushangle, movepushforward);
		else if((player->speed < topspeed) && (mbackward) && (cmd->forwardmove < 0))
			P_Thrust (player->mo, movepushangle, movepushforward);
		else if((mbackward) && (cmd->forwardmove > 0))
			P_Thrust (player->mo, movepushangle, movepushforward);
		else if ((mforward == false) && (mbackward == false))
			P_Thrust (player->mo, movepushangle, movepushforward);
	}
// Analog movement control Tails
	if(!netgame && ((player == &players[consoleplayer] && cv_analog.value) || (cv_splitscreen.value && player == &players[secondarydisplayplayer] && cv_analog2.value)) && thiscam->chase)
	{
		if (!(player->gliding || player->exiting || (player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_flashing])))
		{
			angle_t controldirection;
			angle_t controllerdirection;
			angle_t controlplayerdirection;
			fixed_t tempx;
			fixed_t tempy;
			angle_t tempangle;
			boolean cforward; // controls pointing forward from the player
			boolean cbackward; // controls pointing backward from the player
			tempx = tempy = 0;

			cforward = cbackward = false;

			// Calculate the angle at which the controls are pointing
			// to figure out the proper mforward and mbackward.
			tempangle = thiscam->angle;
			tempangle >>= ANGLETOFINESHIFT;
			tempx += FixedMul(cmd->forwardmove,finecosine[tempangle]);
			tempy += FixedMul(cmd->forwardmove,finesine[tempangle]);

			tempangle = thiscam->angle-ANG90;
			tempangle >>= ANGLETOFINESHIFT;
			tempx += FixedMul(cmd->sidemove,finecosine[tempangle]);
			tempy += FixedMul(cmd->sidemove,finesine[tempangle]);

			tempx = tempx*FRACUNIT;
			tempy = tempy*FRACUNIT;

			controldirection = controllerdirection = R_PointToAngle2(player->mo->x, player->mo->y, player->mo->x + tempx, player->mo->y + tempy);

			controlplayerdirection = player->mo->angle;

			if(controlplayerdirection < ANG90)
			{
				controlplayerdirection += ANG90;
				controllerdirection += ANG90;
			}
			else if(controlplayerdirection > ANG270)
			{
				controlplayerdirection -= ANG90;
				controllerdirection -= ANG90;
			}

			// Controls pointing backwards from player
			if(controllerdirection > controlplayerdirection + ANG90
				&& controllerdirection < controlplayerdirection - ANG90)
			{
				cbackward = true;
			}
			else // Controls pointing in player's general direction
			{
				cforward = true;
			}

			if(player->climbing)
			{
				// Thrust in the direction of the controls
				P_InstaThrust(player->mo, player->mo->angle-ANG90, (cmd->sidemove/10)*FRACUNIT);
				player->mo->momz = (cmd->forwardmove/10)*FRACUNIT;
			}

			else if(player->powers[pw_sneakers] || player->powers[pw_super]) // do you have super sneakers? Tails 02-28-2000
			{
				movepushforward = sqrt(cmd->sidemove*cmd->sidemove + cmd->forwardmove*cmd->forwardmove) * ((thrustfactor*2)*acceleration); // then go faster!! Tails 02-28-2000
			}
			else // if not, then run normally Tails 02-28-2000
			{
				movepushforward = sqrt(cmd->sidemove*cmd->sidemove + cmd->forwardmove*cmd->forwardmove) * (thrustfactor*acceleration); // Changed by Tails: 9-14-99
			}
        
			// allow very small movement while in air for gameplay
			if (!onground)
			{  
				movepushforward >>= 2; // Proper air movement - Changed by Tails: 9-13-99
			}

			// Allow a bit of movement while spinning Tails
			if (player->mfspinning)
			{
				// Stupid little movement prohibitor hack
				// that REALLY shouldn't belong in analog code.
				if((mforward && cmd->forwardmove > 0)
				|| (mbackward && cmd->forwardmove < 0))
					movepushforward = 0;
				else if(!player->mfstartdash)
					movepushforward=movepushforward/16;
				else
					movepushforward = 0;
			}

			movepushsideangle = controldirection;

			if(player->speed < topspeed)
				P_Thrust (player->mo, controldirection, movepushforward);
			else if((mforward) && (cbackward))
				P_Thrust (player->mo, controldirection, movepushforward);
			else if((mbackward) && (cforward))
				P_Thrust (player->mo, controldirection, movepushforward);
		}
	}
	else if (netgame || (player == &players[consoleplayer] && !cv_analog.value) || (cv_splitscreen.value && player == &players[secondarydisplayplayer] && !cv_analog2.value))
	{
		if(player->climbing)
			P_InstaThrust (player->mo, player->mo->angle-ANG90, (cmd->sidemove/10)*FRACUNIT);

		else if (cmd->sidemove && !player->gliding && !player->exiting && !player->climbing && !(player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_flashing])) // Tails 04-12-2001
		{
			boolean mright;
			boolean mleft;
			angle_t sideangle;

			sideangle = player->mo->angle - ANG90;

			// forward
			if ((player->rmomx > 0 && player->rmomy > 0) && (sideangle >= 0 && sideangle < ANG90)) // Quadrant 1
				mright = 1;
			else if ((player->rmomx < 0 && player->rmomy > 0) && (sideangle >= ANG90 && sideangle < ANG180)) // Quadrant 2
				mright = 1;
			else if ((player->rmomx < 0 && player->rmomy < 0) && (sideangle >= ANG180 && sideangle < ANG270)) // Quadrant 3
				mright = 1;
			else if ((player->rmomx > 0 && player->rmomy < 0) && ((sideangle >= ANG270 && (sideangle <= ANGLE_MAX << FRACBITS)) || (sideangle >= 0 && sideangle <= ANG45))) // Quadrant 4
				mright = 1;
			else if (player->rmomx > 0 && ((sideangle >= ANG270+ANG45 && sideangle <= ANGLE_MAX << FRACBITS)))
				mright = 1;
			else if (player->rmomx < 0 && (sideangle >= ANG90+ANG45 && sideangle <= ANG180+ANG45))
				mright = 1;
			else if (player->rmomy > 0 && (sideangle >= ANG45 && sideangle <= ANG90+ANG45))
				mright = 1;
			else if (player->rmomy < 0 && (sideangle >= ANG180+ANG45 && sideangle <= ANG270+ANG45))
				mright = 1;
			else
				mright = 0;
			// backward
			if ((player->rmomx > 0 && player->rmomy > 0) && (sideangle >= ANG180 && sideangle < ANG270)) // Quadrant 3
				mleft = 1;
			else if ((player->rmomx < 0 && player->rmomy > 0) && (sideangle >= ANG270 && (sideangle <= ANGLE_MAX << FRACBITS))) // Quadrant 4
				mleft = 1;
			else if ((player->rmomx < 0 && player->rmomy < 0) && (sideangle >= 0 && sideangle < ANG90)) // Quadrant 1
				mleft = 1;
			else if ((player->rmomx > 0 && player->rmomy < 0) && (sideangle >= ANG90 && sideangle < ANG180)) // Quadrant 2
				mleft = 1;
			else if (player->rmomx < 0 && ((sideangle >= ANG270+ANG45 && sideangle <= ANGLE_MAX << FRACBITS) || (sideangle >= 0 && sideangle <= ANG45)))
				mleft = 1;
			else if (player->rmomx > 0 && (sideangle >= ANG90+ANG45 && sideangle <= ANG180+ANG45))
				mleft = 1;
			else if (player->rmomy < 0 && (sideangle >= ANG45 && sideangle <= ANG90+ANG45))
				mleft = 1;
			else if (player->rmomy > 0 && (sideangle >= ANG180+ANG45 && sideangle <= ANG270+ANG45))
				mleft = 1;
			else // Put in 'or' checks here!
				mleft = 0;

			if(player->powers[pw_sneakers] || player->powers[pw_super])
			{
				movepushside = cmd->sidemove * ((thrustfactor*2)*acceleration);
			}
			else
			{
				movepushside = cmd->sidemove * (thrustfactor*acceleration);
			}

			if (!onground)
			{
				movepushside >>= 2;
			}
					
			// Allow a bit of movement while spinning Tails
			if (player->mfspinning)
			{
				if(!player->mfstartdash)
					movepushside=movepushside/16;
				else
					movepushside = 0;
			}

			// Finally move the player now that his speed/direction has been decided.
			if(player->speed < topspeed)
				P_Thrust (player->mo, movepushsideangle, movepushside);
			else if((mright) && (cmd->sidemove < 0))
				P_Thrust (player->mo, movepushsideangle, movepushside);
			else if((mleft) && (cmd->sidemove > 0))
				P_Thrust (player->mo, movepushsideangle, movepushside);
		}
	}		
}

extern consvar_t cv_timeattacked;

//
// P_MovePlayer
//
void P_MovePlayer (player_t* player)
{
    ticcmd_t*           cmd;
	int					i; // Tails 04-25-2001
	int					tag = 0; // Tails 05-08-2001
	fixed_t tempx; // Tails 06-14-2001
	fixed_t tempy; // Tails 06-14-2001
	angle_t tempangle; // Tails 06-14-2001
	int thrustfactor;
	msecnode_t *node;
	const int runspeed = 28;
	fixed_t middleheight; // player->mo->z + player->info->height/2
	camera_t* thiscam;

	if(countdowntimeup)
		return;

	if(cv_splitscreen.value && player == &players[secondarydisplayplayer])
		thiscam = &camera2;
	else
		thiscam = &camera;

	if(player->superready && player->mo->tracer && player->mo->tracer->type == MT_SUPERTRANS)
	{
		if(player->mo->tracer->health > 0)
		{
			P_UnsetThingPosition(player->mo);
			player->mo->x = player->mo->tracer->x;
			player->mo->y = player->mo->tracer->y;
			player->mo->z = player->mo->tracer->z;
			P_SetThingPosition(player->mo);
			player->mo->momx = player->mo->momy = player->mo->momz = 0;
			player->mo->flags2 |= MF2_DONTDRAW;
			return;
		}
		else
		{
			player->mo->flags2 &= ~MF2_DONTDRAW;
			player->mo->tracer = NULL;

			// Sonic steps out of the phone booth...
			P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_CAPE)->target = player->mo; // A cape... "Super" Sonic, get it? Ha...ha...

			if(emeralds & EMERALD8) // 'Hyper' Sonic
			{
				mobj_t* mo;

				for(i=0; i<8; i++)
				{
					mo = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_HYPERSPARK);

					mo->target = player->mo;
					mo->angle = i*ANG45;
				}
			}
		}
	}

    cmd = &player->cmd;

	// Even if not NiGHTS, pull in nearby objects when walking around as John Q. Elliot.
	if((!player->nightsmode || player->powers[pw_nightshelper]) && ((mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS) || cv_timeattacked.value))
	{
		thinker_t* th;
		mobj_t* mo2;
		fixed_t x = player->mo->x;
		fixed_t y = player->mo->y;
		fixed_t z = player->mo->z;

		for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo2 = (mobj_t *)th;

			if(P_AproxDistance(P_AproxDistance(mo2->x - x, mo2->y - y), mo2->z - z) > 128*FRACUNIT)
				continue;

			if(!(mo2->type == MT_NIGHTSWING || mo2->type == MT_RING || mo2->type == MT_COIN))
				continue;

			// Yay! The thing's in reach! Pull it in!
			mo2->flags2 |= MF2_NIGHTSPULL;
			mo2->tracer = player->mo;
		}
	}

	if(player->bonustime > 1)
	{
		player->bonustime--;
		if(player->bonustime <= 1)
			player->bonustime = 1;
	}

	if(player->linktimer)
	{
		if(--player->linktimer <= 0) // Link timer
			player->linkcount = 0;
	}

	// Locate the capsule for this mare.
	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS
		&& !player->bonustime && !player->capsule)
	{
		thinker_t* th;
		mobj_t* mo2;

		for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo2 = (mobj_t *)th;

			if(mo2->type == MT_EGGCAPSULE
				&& mo2->threshold == player->mare)
				player->capsule = mo2;
		}
	}

	if(player->capsule && player->capsule->reactiontime)
	{
		if(player->nightsmode)
		{
			if(!(player->mo->tracer->state == &states[S_NIGHTSHURT1])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT2])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT3])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT4])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT5])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT6])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT7])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT8])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT9])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT10])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT11])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT12])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT13])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT14])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT15])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT16])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT17])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT18])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT19])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT20])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT21])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT22])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT23])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT24])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT25])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT26])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT27])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT28])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT29])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT30])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT31])
				&& !(player->mo->tracer->state == &states[S_NIGHTSHURT32]))
				P_SetMobjState(player->mo->tracer, S_NIGHTSHURT1);
		}

		if(player->mo->x <= player->capsule->x + 2*FRACUNIT
			&& player->mo->x >= player->capsule->x - 2*FRACUNIT)
		{
			P_UnsetThingPosition(player->mo);
			player->mo->x = player->capsule->x;
			P_SetThingPosition(player->mo);
			player->mo->momx = 0;
		}

		if(player->mo->y <= player->capsule->y + 2*FRACUNIT
			&& player->mo->y >= player->capsule->y - 2*FRACUNIT)
		{
			P_UnsetThingPosition(player->mo);
			player->mo->y = player->capsule->y;
			P_SetThingPosition(player->mo);
			player->mo->momy = 0;
		}

		if(player->mo->z <= player->capsule->z+(player->capsule->height/3) + 2*FRACUNIT
			&& player->mo->z >= player->capsule->z+(player->capsule->height/3) - 2*FRACUNIT)
		{
			player->mo->z = player->capsule->z+(player->capsule->height/3);
			player->mo->momz = 0;
		}

		if(player->mo->x > player->capsule->x)
			player->mo->momx = -2*FRACUNIT;
		else if(player->mo->x < player->capsule->x)
			player->mo->momx = 2*FRACUNIT;
			
		if(player->mo->y > player->capsule->y)
			player->mo->momy = -2*FRACUNIT;
		else if(player->mo->y < player->capsule->y)
			player->mo->momy = 2*FRACUNIT;

		if(player->mo->z > player->capsule->z+(player->capsule->height/3))
			player->mo->momz = -2*FRACUNIT;
		else if(player->mo->z < player->capsule->z+(player->capsule->height/3))
			player->mo->momz = 2*FRACUNIT;

		// Time to blow it up!
		if(player->mo->x == player->capsule->x
			&& player->mo->y == player->capsule->y
			&& player->mo->z == player->capsule->z+(player->capsule->height/3))
		{
			if(player->mo->health > 1)
			{
				player->mo->health--;
				player->health--;
				player->capsule->health--;

				// Spawn a 'pop' for each ring you deposit
				S_StartSound(P_SpawnMobj(player->capsule->x + ((P_SignedRandom()/3)<<FRACBITS), player->capsule->y + ((P_SignedRandom()/3)<<FRACBITS), player->capsule->z + ((P_SignedRandom()/3)<<FRACBITS), MT_EXPLODE), sfx_pop);

				if(player->capsule->health <= 0)
				{
					player->capsule->flags &= ~MF_NOGRAVITY;
					player->capsule->momz = 5*FRACUNIT;
					player->bonustime = 3*TICRATE;
					player->bonuscount = 10;
					{
						int i;
						fixed_t z;

						z = player->capsule->z + player->capsule->height/2;
						for(i = 0; i<16; i++)
							P_SpawnMobj(player->capsule->x, player->capsule->y, z, MT_BIRD);
					}
					player->capsule->reactiontime = 0;
					player->capsule = NULL;
					S_StartSound(player->mo, sfx_ngdone);
				}
			}
			else
			{
				if(player->capsule->health <= 0)
				{
					player->capsule->flags &= ~MF_NOGRAVITY;
					player->capsule->momz = 5*FRACUNIT;
					player->bonustime = 3*TICRATE;
					player->bonuscount = 10;
					{
						int i;
						fixed_t z;

						z = player->capsule->z + player->capsule->height/2;
						for(i = 0; i<16; i++)
							P_SpawnMobj(player->capsule->x, player->capsule->y, z, MT_BIRD);
						S_StartSound(player->mo, sfx_ngdone);
					}
				}
				player->capsule->reactiontime = 0;
				player->capsule = NULL;
			}
		}

		if(player->nightsmode)
		{
			P_UnsetThingPosition(player->mo->tracer);
			player->mo->tracer->x = player->mo->x;
			player->mo->tracer->y = player->mo->y;
			player->mo->tracer->z = player->mo->z;
			player->mo->tracer->floorz = player->mo->floorz;
			player->mo->tracer->ceilingz = player->mo->ceilingz;
			P_SetThingPosition(player->mo->tracer);
		}
		return;
	}

#define MAXDRILLSPEED 14000
#define MAXNORMALSPEED 6000

	// Test revamped NiGHTS movement.
	if(player->nightsmode)
	{
		int radius;
		const double deg2rad = 0.017453293;
		int drillamt = 0;
		boolean still = false;
		boolean moved = false;
		signed short newangle;
		double xspeed, yspeed;
	    thinker_t*  th;
		mobj_t*     mo2;
		mobj_t*  closestaxis;
		int first = 0;
		boolean backwardaxis = false;
		fixed_t newx;
		fixed_t newy;
		boolean firstdrill;
		angle_t movingangle;

		player->drilling = false;

		firstdrill = false;

		if(player->drillmeter > 96*20)
			player->drillmeter = 96*20;

		if(player->drilldelay)
			player->drilldelay--;

		if(!(cmd->buttons & BT_JUMP))
		{
			if(player->drillmeter <= 0)
				player->drillmeter = (TICRATE/10)/NEWTICRATERATIO;
		}

		if(!player->mo->tracer)
		{
			P_DeNightserizePlayer(player);
			return;
		}

		if(leveltime % 35 == 1)
			player->nightstime--;

		if(!player->nightstime)
		{
			P_DeNightserizePlayer(player);
			S_StartSound(player->mo, sfx_timeup);
			return;
		}

		closestaxis = NULL;

		newx = P_ReturnThrustX(player->mo, player->mo->angle, 3*FRACUNIT)+player->mo->x;
		newy = P_ReturnThrustY(player->mo, player->mo->angle, 3*FRACUNIT)+player->mo->y;

		if(player->transfertoclosest || !player->mo->target)
		{
			// scan the thinkers
			// to find the closest axis point
			for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
			{
				if (th->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;

				mo2 = (mobj_t *)th;

				if(first == 0)
				{
					if(mo2->type == MT_AXIS1
						|| mo2->type == MT_AXIS2
						|| mo2->type == MT_AXIS3
						|| mo2->type == MT_AXIS1A
						|| mo2->type == MT_AXIS2A
						|| mo2->type == MT_AXIS3A)
					{
						closestaxis = mo2;
						first++;
					}
				}
				else
				{
					if(mo2->type == MT_AXIS1
						|| mo2->type == MT_AXIS2
						|| mo2->type == MT_AXIS3
						|| mo2->type == MT_AXIS1A
						|| mo2->type == MT_AXIS2A
						|| mo2->type == MT_AXIS3A)
					{
						fixed_t dist1, dist2;

						dist1 = R_PointToDist2(newx, newy, mo2->x, mo2->y)-mo2->info->radius;
						dist2 = R_PointToDist2(newx, newy, closestaxis->x, closestaxis->y)-closestaxis->info->radius;

						if(dist1 < dist2)
							closestaxis = mo2;
					}
				}
			}

			player->mo->target = closestaxis;
		}

		if(!player->mo->target) // Uh-oh!
		{
			CONS_Printf("No axis points found!\n");
			return;
		}

		player->mare = player->mo->target->threshold;

		// The 'ambush' flag says you should rotate
		// the other way around the axis.
		if(player->mo->target->flags & MF_AMBUSH)
		{
			backwardaxis = true;
		}

		player->angle_pos = atan2((player->mo->y - player->mo->target->y), (player->mo->x - player->mo->target->x))/deg2rad;

		if(player->angle_pos < 0.0)
		{
			player->angle_pos += 360.0;
		}
		else if(player->angle_pos >= 360.0)
		{
			player->angle_pos -= 360.0;
		}

		player->old_angle_pos = player->angle_pos;

		radius = player->mo->target->info->radius;

		player->mo->flags |= MF_NOGRAVITY;

		player->mo->flags2 |= MF2_DONTDRAW;

		// Currently reeling from being hit.
		if(player->powers[pw_flashing] > (flashingtics/3)*2)
		{
			xspeed = ((cos(player->flyangle*deg2rad))*(player->speed/50.0));
			yspeed = ((sin(player->flyangle*deg2rad))*(player->speed/50.0));

			xspeed *= player->mo->target->info->speed/10.0;

			if(backwardaxis)
				xspeed *= -1;

			player->angle_speed = (xspeed/200.0);

			player->angle_pos += player->angle_speed;

			if(player->angle_pos < 0.0 && player->old_angle_pos > 0.0)
				player->angle_pos += 360.0;
			else if(player->angle_pos >= 360.0 && player->old_angle_pos < 360.0)
				player->angle_pos -= 360.0;

			player->mo->momx = player->mo->target->x + cos(player->angle_pos * deg2rad) * radius - player->mo->x;
			player->mo->momy = player->mo->target->y + sin(player->angle_pos * deg2rad) * radius - player->mo->y;

			player->mo->momz = 0;
			P_UnsetThingPosition(player->mo->tracer);
			player->mo->tracer->x = player->mo->x;
			player->mo->tracer->y = player->mo->y;
			player->mo->tracer->z = player->mo->z;
			player->mo->tracer->floorz = player->mo->floorz;
			player->mo->tracer->ceilingz = player->mo->ceilingz;
			P_SetThingPosition(player->mo->tracer);
			return;
		}

		if((player->mo->tracer->state == &states[S_SUPERTRANS1])
			|| (player->mo->tracer->state == &states[S_SUPERTRANS2])
			|| (player->mo->tracer->state == &states[S_SUPERTRANS3])
			|| (player->mo->tracer->state == &states[S_SUPERTRANS4])
			|| (player->mo->tracer->state == &states[S_SUPERTRANS5])
			|| (player->mo->tracer->state == &states[S_SUPERTRANS6])
			|| (player->mo->tracer->state == &states[S_SUPERTRANS7])
			|| (player->mo->tracer->state == &states[S_SUPERTRANS8])
			|| (player->mo->tracer->state == &states[S_SUPERTRANS9]))
		{
			player->mo->momx = player->mo->momy = player->mo->momz = 0;

			P_UnsetThingPosition(player->mo->tracer);
			player->mo->tracer->x = player->mo->x;
			player->mo->tracer->y = player->mo->y;
			player->mo->tracer->z = player->mo->z;
			player->mo->tracer->floorz = player->mo->floorz;
			player->mo->tracer->ceilingz = player->mo->ceilingz;
			P_SetThingPosition(player->mo->tracer);
			return;
		}

		if(player->exiting > 0 && player->exiting < 2*TICRATE)
		{
			player->mo->momx = player->mo->momy = 0;

			player->mo->momz = 30*FRACUNIT;

			player->mo->tracer->angle += ANG45/4;
			
			P_UnsetThingPosition(player->mo->tracer);
			player->mo->tracer->x = player->mo->x;
			player->mo->tracer->y = player->mo->y;
			player->mo->tracer->z = player->mo->z;
			player->mo->tracer->floorz = player->mo->floorz;
			player->mo->tracer->ceilingz = player->mo->ceilingz;
			P_SetThingPosition(player->mo->tracer);
			return;
		}

		if(leveltime & 1)
		{
			mobj_t* firstmobj;
			mobj_t* secondmobj;

			firstmobj = P_SpawnMobj(player->mo->x + P_ReturnThrustX(player->mo, player->mo->angle+ANG90, 16*FRACUNIT), player->mo->y + P_ReturnThrustY(player->mo, player->mo->angle+ANG90, 16*FRACUNIT), player->mo->z + player->mo->height/2, MT_NIGHTSPARKLE);
			secondmobj = P_SpawnMobj(player->mo->x + P_ReturnThrustX(player->mo, player->mo->angle-ANG90, 16*FRACUNIT), player->mo->y + P_ReturnThrustY(player->mo, player->mo->angle-ANG90, 16*FRACUNIT), player->mo->z + player->mo->height/2, MT_NIGHTSPARKLE);

			if(firstmobj)
			{
				firstmobj->fuse = leveltime;
				firstmobj->target = player->mo;
			}

			if(secondmobj)
			{
				secondmobj->fuse = leveltime;
				secondmobj->target = player->mo;
			}

			player->mo->fuse = leveltime;
		}

		if(cmd->buttons & BT_JUMP && player->drillmeter && player->drilldelay == 0)
		{
			if(!player->jumping)
				firstdrill = true;

			player->jumping = 1;
			player->drilling = true;
		}
		else
		{
			player->jumping = 0;

			if(cmd->sidemove != 0)
				moved = true;

			if(player->drillmeter & 1)
				player->drillmeter++; // I'll be nice and give them one.
		}

		if(cmd->forwardmove != 0)
			moved = true;

		if(moved == true)
		{
			if(player->drilling)
			{
				drillamt += 50;
			}
			else
			{
				int distance;

				distance = sqrt(abs(cmd->forwardmove)*abs(cmd->forwardmove) + abs(cmd->sidemove)*abs(cmd->sidemove));

				drillamt += distance > 50 ? 50 : distance;

				drillamt *= 1.25;
			}
		}
						
		player->speed += drillamt;

		if(player->drilling == false)
		{
			if(player->speed > MAXNORMALSPEED)
				player->speed -= (player->speed - drillamt > MAXNORMALSPEED) ? 55 : 50;
		}
		else
		{
			player->speed += 75;
			if(player->speed > MAXDRILLSPEED)
				player->speed = MAXDRILLSPEED;

			if(--player->drillmeter == 0)
				player->drilldelay = TICRATE*2;
		}

		if(drillamt == 0 && player->speed > 0)
			player->speed -= 25;

		if(player->speed < 0)
			player->speed = 0;

		if(cmd->sidemove != 0)
		{
			newangle = atan2(cmd->forwardmove, cmd->sidemove)/deg2rad;
		}
		else if(cmd->forwardmove > 0)
			newangle = 89;
		else if(cmd->forwardmove < 0)
			newangle = 269;

		if(newangle < 0 && moved)
			newangle = 360+newangle;


		if(player->drilling)
		{
			thrustfactor = 1;
		}
		else
			thrustfactor = 6;

		for(i=0; i<thrustfactor; i++)
		{
			if(moved && player->flyangle != newangle)
			{
				// player->flyangle is the one to move
				// newangle is the "move to"
				if ((((newangle-player->flyangle)+360)%360)>(((player->flyangle-newangle)+360)%360))
				{
					player->flyangle--;
					if(player->flyangle < 0)
					{
						player->flyangle = 360 + player->flyangle;
					}
				}
				else
				{
					player->flyangle++;
				}
			}

			player->flyangle %= 360;
		
		}

		if(!(player->speed)
			&& cmd->forwardmove == 0)
			still = true;

		if((cmd->buttons & BT_CAMLEFT) && (cmd->buttons & BT_CAMRIGHT))
		{
			if(!player->skiddown && player->speed > 2000)
			{
				player->speed /= 10;
				S_StartSound(player->mo, sfx_ngskid);
			}
			player->skiddown = true;
		}
		else
			player->skiddown = false;

		xspeed = ((cos(player->flyangle*deg2rad))*(player->speed/50.0));
		yspeed = ((sin(player->flyangle*deg2rad))*(player->speed/50.0));

		xspeed *= player->mo->target->info->speed/10.0;

		if(backwardaxis)
			xspeed *= -1;

		player->angle_speed = (xspeed/200.0);

		player->angle_pos += player->angle_speed;

		if(player->angle_pos < 0.0 && player->old_angle_pos > 0.0)
			player->angle_pos += 360.0;
		else if(player->angle_pos >= 360.0 && player->old_angle_pos < 360.0)
			player->angle_pos -= 360.0;

		player->mo->momx = player->mo->target->x + cos(player->angle_pos * deg2rad) * radius - player->mo->x;
		player->mo->momy = player->mo->target->y + sin(player->angle_pos * deg2rad) * radius - player->mo->y;

		if(still)
			player->mo->momz = -FRACUNIT;
		else
			player->mo->momz = (yspeed/11.0)*FRACUNIT;

		if(player->mo->momz > 20*FRACUNIT)
			player->mo->momz = 20*FRACUNIT;
		else if(player->mo->momz < -20*FRACUNIT)
			player->mo->momz = -20*FRACUNIT;

		middleheight = player->mo->z + (player->mo->info->height>>1);

		if(player->mo->momz != 0)
		{
			boolean wasinwater;
			wasinwater = P_MobjCheckOldPosWater(player->mo);
			if(((middleheight > player->mo->watertop && middleheight - player->mo->momz < player->mo->watertop)
				|| (middleheight < player->mo->watertop	&& middleheight - player->mo->momz > player->mo->watertop && wasinwater == false)))
			{
				int bubblecount;
				int i;

				if(player->mo->momz > 0)
				{
					player->mo->momz = player->mo->momz*1.706783369803; // Give the player a little out-of-water boost.
					player->mo->momz *= (player->jumpfactor/100.0);
				}

				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->watertop, MT_SPLISH); // Spawn a splash
				S_StartSound(player->mo, sfx_splish); // And make a sound!

				bubblecount = abs(player->mo->momz)>>FRACBITS;
				// Create tons of bubbles
				for(i=0; i<bubblecount; i++)
				{
					if(P_Random() < 32)
						P_SpawnMobj(player->mo->x + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), player->mo->y + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), player->mo->z + (P_Random()<<FRACBITS)/4, MT_MEDIUMBUBBLE)->momz = player->mo->momz < 0 ? player->mo->momz/16 : 0;
					else
						P_SpawnMobj(player->mo->x + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), player->mo->y + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), player->mo->z + (P_Random()<<FRACBITS)/4, MT_SMALLBUBBLE)->momz = player->mo->momz < 0 ? player->mo->momz/16 : 0;
				}
			}
		}

		// You can create splashes as you fly across water.
		if(player->mo->z + player->mo->info->height >= player->mo->watertop && player->mo->z <= player->mo->watertop && player->speed > 9000 && leveltime % 5 == 1)
		{
			S_StartSound(P_SpawnMobj(player->mo->x, player->mo->y, player->mo->watertop, MT_SPLISH), sfx_wslap);
		}

		// Spawn Sonic's bubbles Tails 03-07-2000
		if(player->mo->eflags & MF_UNDERWATER && !(P_Random() % 16))
		{
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height / 1.25), MT_SMALLBUBBLE)->threshold = 42;
		}
		else if(player->mo->eflags & MF_UNDERWATER && !(P_Random() % 96))
		{
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height / 1.25), MT_MEDIUMBUBBLE)->threshold = 42;
		}

		if(player->mo->momx || player->mo->momy)
			player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, player->mo->x + player->mo->momx, player->mo->y + player->mo->momy);

		if(still)
		{
			player->anotherflyangle = 0;
			movingangle = 0;
		}
		else if(backwardaxis)
		{
			// Special cases to prevent the angle from being
			// calculated incorrectly when wrapped.
			if(player->old_angle_pos > 350.0 && player->angle_pos < 10.0)
			{
				movingangle = R_PointToAngle2(0, player->mo->z, -R_PointToDist2(player->mo->momx, player->mo->momy, 0, 0), player->mo->z + player->mo->momz);
				player->anotherflyangle = (movingangle >> ANGLETOFINESHIFT) * 360/FINEANGLES;
			}
			else if(player->old_angle_pos < 10.0 && player->angle_pos > 350.0)
			{
				movingangle = R_PointToAngle2(0, player->mo->z, R_PointToDist2(player->mo->momx, player->mo->momy, 0, 0), player->mo->z + player->mo->momz);
				player->anotherflyangle = (movingangle >> ANGLETOFINESHIFT) * 360/FINEANGLES;
			}
			else if(player->angle_pos > player->old_angle_pos)
			{
				movingangle = R_PointToAngle2(0, player->mo->z, -R_PointToDist2(player->mo->momx, player->mo->momy, 0, 0), player->mo->z + player->mo->momz);
				player->anotherflyangle = (movingangle >> ANGLETOFINESHIFT) * 360/FINEANGLES;
			}
			else
			{
				movingangle = R_PointToAngle2(0, player->mo->z, R_PointToDist2(player->mo->momx, player->mo->momy, 0, 0), player->mo->z + player->mo->momz);
				player->anotherflyangle = (movingangle >> ANGLETOFINESHIFT) * 360/FINEANGLES;
			}
		}
		else
		{
			// Special cases to prevent the angle from being
			// calculated incorrectly when wrapped.
			if(player->old_angle_pos > 350.0 && player->angle_pos < 10.0)
			{
				movingangle = R_PointToAngle2(0, player->mo->z, R_PointToDist2(player->mo->momx, player->mo->momy, 0, 0), player->mo->z + player->mo->momz);
				player->anotherflyangle = (movingangle >> ANGLETOFINESHIFT) * 360/FINEANGLES;
			}
			else if(player->old_angle_pos < 10.0 && player->angle_pos > 350.0)
			{
				movingangle = R_PointToAngle2(0, player->mo->z, -R_PointToDist2(player->mo->momx, player->mo->momy, 0, 0), player->mo->z + player->mo->momz);
				player->anotherflyangle = (movingangle >> ANGLETOFINESHIFT) * 360/FINEANGLES;
			}
			else if(player->angle_pos < player->old_angle_pos)
			{
				movingangle = R_PointToAngle2(0, player->mo->z, -R_PointToDist2(player->mo->momx, player->mo->momy, 0, 0), player->mo->z + player->mo->momz);
				player->anotherflyangle = (movingangle >> ANGLETOFINESHIFT) * 360/FINEANGLES;
			}
			else
			{
				movingangle = R_PointToAngle2(0, player->mo->z, R_PointToDist2(player->mo->momx, player->mo->momy, 0, 0), player->mo->z + player->mo->momz);
				player->anotherflyangle = (movingangle >> ANGLETOFINESHIFT) * 360/FINEANGLES;
			}
		}

		if(player->anotherflyangle >= 349 || player->anotherflyangle <= 11)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL1A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL1B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL1C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL1D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY1A : S_NIGHTSFLY1B);
		}
		else if(player->anotherflyangle >= 12 && player->anotherflyangle <= 33)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL2A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL2B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL2C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL2D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY2A : S_NIGHTSFLY2B);
		}
		else if(player->anotherflyangle >= 34 && player->anotherflyangle <= 56)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL3A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL3B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL3C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL3D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY3A : S_NIGHTSFLY3B);
		}
		else if(player->anotherflyangle >= 57 && player->anotherflyangle <= 79)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL4A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL4B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL4C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL4D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY4A : S_NIGHTSFLY4B);
		}
		else if(player->anotherflyangle >= 80 && player->anotherflyangle <= 101)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL5A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL5B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL5C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL5D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL5B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL5C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL5D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL5A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL5A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY5A : S_NIGHTSFLY5B);
		}
		else if(player->anotherflyangle >= 102 && player->anotherflyangle <= 123)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL4A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL4B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL4C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL4D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL4A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY4A : S_NIGHTSFLY4B);
		}
		else if(player->anotherflyangle >= 124 && player->anotherflyangle <= 146)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL3A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL3B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL3C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL3D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL3A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY3A : S_NIGHTSFLY3B);
		}
		else if(player->anotherflyangle >= 147 && player->anotherflyangle <= 168)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL2A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL2B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL2C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL2D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL2A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY2A : S_NIGHTSFLY2B);
		}
		else if(player->anotherflyangle >= 169 && player->anotherflyangle <= 191)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL1A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL1B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL1C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL1D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL1A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY1A : S_NIGHTSFLY1B);
		}
		else if(player->anotherflyangle >= 192 && player->anotherflyangle <= 213)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL6A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL6B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL6C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL6D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY6A : S_NIGHTSFLY6B);
		}
		else if(player->anotherflyangle >= 214 && player->anotherflyangle <= 236)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL7A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL7B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL7C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL7D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY7A : S_NIGHTSFLY7B);
		}
		else if(player->anotherflyangle >= 237 && player->anotherflyangle <= 258)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL8A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL8B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL8C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL8D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY8A : S_NIGHTSFLY8B);
		}
		else if(player->anotherflyangle >= 259 && player->anotherflyangle <= 281)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL9A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL9B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL9C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL9D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL9B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL9C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL9D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL9A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL9A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY9A : S_NIGHTSFLY9B);
		}
		else if(player->anotherflyangle >= 282 && player->anotherflyangle <= 304)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL8A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL8B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL8C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL8D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL8A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY8A : S_NIGHTSFLY8B);
		}
		else if(player->anotherflyangle >= 305 && player->anotherflyangle <= 326)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL7A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL7B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL7C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL7D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL7A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY7A : S_NIGHTSFLY7B);
		}
		else if(player->anotherflyangle >= 327 && player->anotherflyangle <= 348)
		{
			if(player->drilling)
			{
				if(!(player->mo->tracer->state == &states[S_NIGHTSDRILL6A]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL6B]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL6C]
					|| player->mo->tracer->state == &states[S_NIGHTSDRILL6D]))
				{
					if(!(player->mo->tracer->state >= &states[S_NIGHTSFLY1A]
						&& player->mo->tracer->state <= &states[S_NIGHTSFLY9B]))
					{
						short framenum;

						framenum = player->mo->tracer->state->frame % 4;

						if(framenum == 0) // Drilla
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6B);
						else if(framenum == 1) // Drillb
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6C);
						else if(framenum == 2) // Drillc
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6D);
						else // Drilld
							P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6A);
					}
					else
						P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRILL6A);
				}
			}
			else
				P_SetMobjStateNF(player->mo->tracer, leveltime & 1 ? S_NIGHTSFLY6A : S_NIGHTSFLY6B);
		}

		if(player == &players[consoleplayer])
			localangle = player->mo->angle;
		else if(cv_splitscreen.value && player == &players[secondarydisplayplayer])
			localangle2 = player->mo->angle;

		player->transfertoclosest = false;

		if(still)
		{
			P_SetMobjStateNF(player->mo->tracer, S_NIGHTSDRONE1);
			player->mo->tracer->angle = player->mo->angle;
		}

		// Sets the minutes/seconds, and synchronizes the "real"
		// amount of time spent in the level. Tails 02-29-2000
		if(!player->exiting)
		{
			player->minutes = (leveltime/(60*TICRATE));
			player->seconds = (leveltime/TICRATE) % 60;
			player->realtime = leveltime;
		}

		P_UnsetThingPosition(player->mo->tracer);
		player->mo->tracer->x = player->mo->x;
		player->mo->tracer->y = player->mo->y;
		player->mo->tracer->z = player->mo->z;
		player->mo->tracer->floorz = player->mo->floorz;
		player->mo->tracer->ceilingz = player->mo->ceilingz;
		P_SetThingPosition(player->mo->tracer);

		if(movingangle >= ANG90 && movingangle <= ANG180)
			movingangle = -(movingangle - ANG180);
		else if(movingangle >= ANG180 && movingangle <= ANG270)
			movingangle = -(movingangle - ANG180);
		else if(movingangle >= ANG270)
			movingangle = (movingangle - ANGLE_MAX);

		if(player == &players[consoleplayer])
			localaiming = movingangle;
		else if(cv_splitscreen.value && player == &players[secondarydisplayplayer])
			localaiming2 = movingangle;

		player->mo->tracer->angle = player->mo->angle;

		if(player->drilling)
		{
			if(firstdrill)
			{
				S_StartSound(player->mo, sfx_drill1);
				player->drilltimer = 32 * NEWTICRATERATIO;
			}
			else if(--player->drilltimer <= 0)
			{
				player->drilltimer = 10 * NEWTICRATERATIO;
				S_StartSound(player->mo, sfx_drill2);
			}
		}

		if(player->powers[pw_extralife] == 1 && player==&players[consoleplayer]) // Extra Life!
		{
			if(player->powers[pw_super])
			{
				S_ChangeMusic(mus_supers, true);
			}
			else if(player->powers[pw_invulnerability] > 1)
			{
				if(mariomode)
					S_ChangeMusic(mus_minvnc, false);
				else
					S_ChangeMusic(mus_invinc, false);
			}
			else if(player->powers[pw_sneakers] > 1)
			{
				S_ChangeMusic(mus_shoes, false);
			}
			else if(player->powers[pw_underwater] <= 11*TICRATE + 1 && player->powers[pw_underwater] > 0)
			{
				S_ChangeMusic(mus_drown, false); // Tails 03-14-2000
			}
			else if(player->powers[pw_spacetime] > 1)
			{
				S_ChangeMusic(mus_drown, false); // Tails 03-14-2000
			}
			else
			{
				S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, 1); // Tails 03-14-2000
			}
		}

		if(cv_objectplace.value)
		{
			player->nightstime = 3;
			player->drillmeter = TICRATE;

			// This places a hoop!
			if(cmd->buttons & BT_ATTACK && !player->attackdown)
			{
				mapthing_t*         mt;
				mapthing_t*    oldmapthings;
				unsigned short angle;

				angle = (360-player->anotherflyangle) % 360;
				if(angle > 90 && angle < 270)
				{
					angle += 180;
					angle %= 360;
				}

				if(player->mo->target->flags & MF_AMBUSH)
					angle = player->anotherflyangle;
				else
				{
					angle = (360-player->anotherflyangle) % 360;
					if(angle > 90 && angle < 270)
					{
						angle += 180;
						angle %= 360;
					}
				}

				oldmapthings = mapthings;
				nummapthings++;
				mapthings        = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);

				memcpy(mapthings, oldmapthings, sizeof(mapthing_t)*(nummapthings-1));

				Z_Free(oldmapthings);

				mt = mapthings+nummapthings-1;

				mt->x = player->mo->x >> FRACBITS;
				mt->y = player->mo->y >> FRACBITS;
				mt->angle = angle;
				mt->type = 57;

				mt->options = (player->mo->z - player->mo->subsector->sector->floorheight) >> FRACBITS;

				P_SpawnHoopsAndRings(mt);

				player->attackdown = true;
			}
			else if(!(cmd->buttons & BT_ATTACK))
				player->attackdown = false;

			// This places a ring!
			if(cmd->buttons & BT_CAMRIGHT && !player->redxvi)
			{
				mapthing_t*				 mt;
				mapthing_t*    oldmapthings;

				oldmapthings = mapthings;
				nummapthings++;
				mapthings        = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);

				memcpy(mapthings, oldmapthings, sizeof(mapthing_t)*(nummapthings-1));

				Z_Free(oldmapthings);

				mt = mapthings+nummapthings-1;

				mt->x = player->mo->x >> FRACBITS;
				mt->y = player->mo->y >> FRACBITS;
				mt->angle = 0;
				mt->type = 2014;

				mt->options = (player->mo->z - player->mo->subsector->sector->floorheight) >> FRACBITS;
				mt->options <<= 4;

				mt->options += cv_objflags.value;
				P_SpawnHoopsAndRings(mt);

				player->redxvi = true;
			}
			else if(!(cmd->buttons & BT_CAMRIGHT))
				player->redxvi = false;

			// This places a wing item!
			if(cmd->buttons & BT_CAMLEFT && !player->mfjumped)
			{
				mapthing_t*				 mt;
				mapthing_t*    oldmapthings;

				oldmapthings = mapthings;
				nummapthings++;
				mapthings        = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);

				memcpy(mapthings, oldmapthings, sizeof(mapthing_t)*(nummapthings-1));

				Z_Free(oldmapthings);

				mt = mapthings+nummapthings-1;

				mt->x = player->mo->x >> FRACBITS;
				mt->y = player->mo->y >> FRACBITS;
				mt->angle = 0;
				mt->type = 37;

				mt->options = (player->mo->z - player->mo->subsector->sector->floorheight) >> FRACBITS;

				CONS_Printf("Z is %d\n", mt->options);

				mt->options <<= 4;

				CONS_Printf("Z is %d\n", mt->options);

				mt->options += cv_objflags.value;

				P_SpawnHoopsAndRings(mt);

				player->mfjumped = true;
			}
			else if(!(cmd->buttons & BT_CAMLEFT))
				player->mfjumped = false;

			// This places a custom object as defined in the console cv_mapthingnum.
			if(cmd->buttons & BT_USE && !player->usedown && cv_mapthingnum.value)
			{
				mapthing_t*				 mt;
				mapthing_t*    oldmapthings;
				int shift;

				if((cv_mapthingnum.value == 16 || cv_mapthingnum.value == 2008) && ((player->mo->z - player->mo->subsector->sector->floorheight) >> FRACBITS) >= 2048)
				{
					CONS_Printf("Sorry, you're too high to place this object (max: 2047 above bottom floor).\n");
					return;
				}
				else if(((player->mo->z - player->mo->subsector->sector->floorheight) >> FRACBITS) >= 4096)
				{
					CONS_Printf("Sorry, you're too high to place this object (max: 4095 above bottom floor).\n");
					return;
				}

				oldmapthings = mapthings;
				nummapthings++;
				mapthings        = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);

				memcpy(mapthings, oldmapthings, sizeof(mapthing_t)*(nummapthings-1));

				Z_Free(oldmapthings);

				mt = mapthings+nummapthings-1;

				mt->x = player->mo->x >> FRACBITS;
				mt->y = player->mo->y >> FRACBITS;
				mt->angle = 0;
				mt->type = cv_mapthingnum.value;

				mt->options = (player->mo->z - player->mo->subsector->sector->floorheight) >> FRACBITS;
				
				if(mt->type == 16 || mt->type == 2008) // Eggmobile 1 & 2
					shift = 5; // Why you would want to place these in a NiGHTS map, I have NO idea!
				else if(mt->type == 3006) // Stupid starpost...
					shift = 0;
				else
					shift = 4;

				if(shift)
					mt->options <<= shift;
				else
					mt->options = 0;

				mt->options += cv_objflags.value;

				if (mt->type == 57 || mt->type == 84
					|| mt->type == 44 || mt->type == 76
					|| mt->type == 77 || mt->type == 47
					|| mt->type == 2014 || mt->type == 47
					|| mt->type == 2007 || mt->type == 2048
					|| mt->type == 2010 || mt->type == 2046
					|| mt->type == 2047 || mt->type == 37)
					P_SpawnHoopsAndRings(mt);
				else
					P_SpawnMapThing(mt);

				CONS_Printf("Spawned at %d\n", mt->options >> shift);

				player->usedown = true;
			}
			else if(!(cmd->buttons & BT_USE))
				player->usedown = false;
		}

		return;
	}

	if(player->nightsfall && player->mo->z <= player->mo->floorz)
	{
		if(player->health > 1)
			P_DamageMobj(player->mo, NULL, NULL, 1);
		else
			player->nightsfall = false;
	}

	if(cv_objectplace.value)
	{
		mobj_t* currentitem;

		if (netgame || (player == &players[consoleplayer] && !cv_analog.value)
		 || (cv_splitscreen.value && player == &players[secondarydisplayplayer] && !cv_analog2.value) || player->mfspinning)
		{
			#ifndef ABSOLUTEANGLE
				player->mo->angle += (cmd->angleturn<<16);
			#else
				if(!player->climbing)
						player->mo->angle = (cmd->angleturn<<16);
			#endif
		}

		ticruned++;
		if( (cmd->angleturn & TICCMD_RECEIVED) == 0)
			ticmiss++;

		if(cmd->buttons & BT_JUMP)
			player->mo->z += FRACUNIT*cv_speed.value;
		else if(cmd->buttons & BT_USE)
			player->mo->z -= FRACUNIT*cv_speed.value;

		if(cmd->buttons & BT_CAMLEFT && !player->usedown)
		{
			do
			{
				player->currentthing--;
				if(player->currentthing <= 0)
					player->currentthing = NUMMOBJTYPES-1;
			}while(mobjinfo[player->currentthing].doomednum == -1
				|| (player->currentthing >= MT_NIGHTSPARKLE
					&& player->currentthing <= MT_NIGHTSCHAR)
				|| player->currentthing == MT_PUSH
				|| player->currentthing == MT_PULL
				|| player->currentthing == MT_SANTA
				|| player->currentthing == MT_LASER
				|| player->currentthing == MT_BOSSFLYPOINT
				|| player->currentthing == MT_EGGTRAP
				|| player->currentthing == MT_CHAOSSPAWNER
				|| player->currentthing == MT_STREETLIGHT
				|| player->currentthing == MT_TELEPORTMAN // Graue 12-12-2003
				|| (player->currentthing >= MT_AWATERA
					&& player->currentthing <= MT_AWATERH));

			CONS_Printf("Current mapthing is %d\n", mobjinfo[player->currentthing].doomednum);
			player->usedown = true;
		}
		else if(cmd->buttons & BT_CAMRIGHT && !player->jumpdown)
		{
			do
			{
				player->currentthing++;
				if(player->currentthing >= NUMMOBJTYPES)
					player->currentthing = 0;
			}while(mobjinfo[player->currentthing].doomednum == -1
				|| (player->currentthing >= MT_NIGHTSPARKLE
					&& player->currentthing <= MT_NIGHTSCHAR)
				|| player->currentthing == MT_PUSH
				|| player->currentthing == MT_PULL
				|| player->currentthing == MT_SANTA
				|| player->currentthing == MT_LASER
				|| player->currentthing == MT_BOSSFLYPOINT
				|| player->currentthing == MT_EGGTRAP
				|| player->currentthing == MT_CHAOSSPAWNER
				|| player->currentthing == MT_STREETLIGHT
				|| player->currentthing == MT_TELEPORTMAN // Graue 12-12-2003
				|| (player->currentthing >= MT_AWATERA
					&& player->currentthing <= MT_AWATERH));

			CONS_Printf("Current mapthing is %d\n", mobjinfo[player->currentthing].doomednum);
			player->jumpdown = true;
		}

		// Place an object and add it to the maplist
		if(player->mo->target)
			if(cmd->buttons & BT_ATTACK && !player->attackdown)
			{
				mapthing_t*         mt;
				mapthing_t*    oldmapthings;
				mobj_t* newthing;
				short x,y,z;
				byte zshift;

				z = 0;

				if(cv_snapto.value)
				{
					if(cv_snapto.value == 1) // Snap to floor
						z = (player->mo->floorz - player->mo->subsector->sector->floorheight) >> FRACBITS;
					else if(cv_snapto.value == 2) // Snap to ceiling
						z = (player->mo->ceilingz - player->mo->target->height - player->mo->subsector->sector->floorheight)>>FRACBITS;
					else if(cv_snapto.value == 3) // Snap to middle
						z = (((player->mo->ceilingz - player->mo->floorz)/2)-player->mo->subsector->sector->floorheight)>>FRACBITS;
				}
				else
				{
					if(cv_grid.value)
					{
						double zpos;

						zpos = ((double)(player->mo->z - player->mo->subsector->sector->floorheight))/FRACUNIT;

						zpos = (zpos/cv_grid.value);

						zpos = (zpos-(int)zpos) < 0.5 ? (int)zpos : (int)zpos+1;

						zpos *= cv_grid.value;

						z = (int)zpos;
					}
					else
						z = (player->mo->z - player->mo->subsector->sector->floorheight) >> FRACBITS;
				}

				// Bosses have height limitations because of their 5th bit usage.
				if((player->mo->target->flags & MF_BOSS)
					|| (cv_mapthingnum.value >= 4001 && cv_mapthingnum.value <= 4028)
					|| (cv_mapthingnum.value == 11 || cv_mapthingnum.value == 87 || cv_mapthingnum.value == 89)
					|| (cv_mapthingnum.value >= 1 && cv_mapthingnum.value <= 4))
				{
					if(z >= 2048)
					{
						CONS_Printf("Sorry, you're too high to place this object (max: 2047 above bottom floor).\n");
						return;
					}
					zshift = 5; // Shift it over 5 bits to make room for the flag info.
				}
				else
				{
					if(z >= 4096)
					{
						CONS_Printf("Sorry, you're too high to place this object (max: 4095 above bottom floor).\n");
						return;
					}
					zshift = 4;
				}

				z <<= zshift;

				// Currently only the Starpost uses this
				if(player->mo->target->flags & MF_SPECIALFLAGS)
				{
					if(player->mo->target->type == MT_STARPOST)
						z = cv_objflags.value;

					CONS_Printf("Starposts do not currently support Z height.\nNote that it will appear normally on the floor if it does not right now.");
				}
				else
					z += cv_objflags.value; // Easy/med/hard/ambush/etc.

				oldmapthings = mapthings;
				nummapthings++;
				mapthings        = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);

				memcpy(mapthings, oldmapthings, sizeof(mapthing_t)*(nummapthings-1));

				Z_Free(oldmapthings);

				mt = mapthings+nummapthings-1;

				if(cv_grid.value)
				{
					double pos;

					pos = ((double)player->mo->x)/FRACUNIT;

					pos = (pos/cv_grid.value);

					pos = (pos-(int)pos) < 0.5 ? (int)pos : (int)pos+1;

					pos *= cv_grid.value;

					x = (int)pos;
				}
				else
					x = player->mo->x >> FRACBITS;

				if(cv_grid.value)
				{
					double pos;

					pos = ((double)player->mo->y)/FRACUNIT;

					pos = (pos/cv_grid.value);

					pos = (pos-(int)pos) < 0.5 ? (int)pos : (int)pos+1;

					pos *= cv_grid.value;

					y = (int)pos;
				}
				else
					y = player->mo->y >> FRACBITS;

				mt->x = x;
				mt->y = y;
				mt->angle = player->mo->angle/ANGLE_1;
				if(cv_mapthingnum.value != 0)
				{
					mt->type = cv_mapthingnum.value;
					CONS_Printf("Placed object mapthingum %d, not the one below.\n", mt->type);
				}
				else
					mt->type = mobjinfo[player->currentthing].doomednum;

				mt->options = z;

				newthing = P_SpawnMobj(x << FRACBITS, y << FRACBITS, player->mo->subsector->sector->floorheight + ((z>>zshift)<<FRACBITS), player->currentthing);
				newthing->angle = player->mo->angle;
				newthing->spawnpoint = mt;
				CONS_Printf("Placed object type %d at %d, %d, %d, %d\n", newthing->info->doomednum, mt->x, mt->y, newthing->z >> FRACBITS, mt->angle);

				player->attackdown = true;
			}

		if(cmd->buttons & BT_TAUNT) // Remove any objects near you
		{
			thinker_t*  th;
			mobj_t*     mo2;
			boolean done = false;

			// scan the thinkers
			for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
			{
				if (th->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;

				mo2 = (mobj_t *)th;

				if(mo2 == player->mo->target)
					continue;

				if(mo2 == player->mo)
					continue;

				if(P_AproxDistance(P_AproxDistance(mo2->x - player->mo->x, mo2->y - player->mo->y), mo2->z - player->mo->z) < player->mo->radius)
				{
					if(mo2->spawnpoint)
					{
						mapthing_t* mt;
						int i;

						P_SetMobjState(mo2, S_DISS);
						mt = mapthings;
						for (i=0 ; i<nummapthings ; i++, mt++)
						{
							if(done)
								continue;

							if(mt->mobj == mo2) // Found it! Now to delete...
							{
								mapthing_t*    oldmapthings;
								mapthing_t*    oldmt;
								mapthing_t*    newmt;
								int z;

								CONS_Printf("Deleting...\n");

								oldmapthings = mapthings;
								nummapthings--;
								mapthings        = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);


								// Gotta rebuild the WHOLE MAPTHING LIST,
								// otherwise it doesn't work!
								oldmt = oldmapthings;
								newmt = mapthings;
								for(z=0; z<nummapthings+1; z++, oldmt++, newmt++)
								{
									if(oldmt->mobj == mo2)
									{
										CONS_Printf("Deleted.\n");
										newmt--;
										continue;
									}

									newmt->x = oldmt->x;
									newmt->y = oldmt->y;
									newmt->angle = oldmt->angle;
									newmt->type = oldmt->type;
									newmt->options = oldmt->options;

									newmt->z = oldmt->z;
									newmt->mobj = oldmt->mobj;
								}

								Z_Free(oldmapthings);
								done = true;
							}
						}
					}
					else
						CONS_Printf("You cannot delete this item because it doesn't have a mapthing!\n");
				}
				done = false;
			}
		}

		if(!(cmd->buttons & BT_ATTACK))
			player->attackdown = false;

		if(!(cmd->buttons & BT_CAMLEFT))
			player->usedown = false;

		if(!(cmd->buttons & BT_CAMRIGHT))
			player->jumpdown = false;

		if(cmd->forwardmove)
		{
			P_Thrust(player->mo, player->mo->angle, cmd->forwardmove*FRACUNIT/4);
			P_UnsetThingPosition(player->mo);
			player->mo->x += player->mo->momx;
			player->mo->y += player->mo->momy;
			P_SetThingPosition(player->mo);
			player->mo->momx = player->mo->momy = 0;
		}
		if(cmd->sidemove)
		{
			P_Thrust(player->mo, player->mo->angle-ANG90, cmd->sidemove*FRACUNIT/4);
			P_UnsetThingPosition(player->mo);
			player->mo->x += player->mo->momx;
			player->mo->y += player->mo->momy;
			P_SetThingPosition(player->mo);
			player->mo->momx = player->mo->momy = 0;
		}

		if(!player->mo->target || player->currentthing != player->mo->target->type)
		{
			if(player->mo->target)
				P_SetMobjState(player->mo->target, S_DISS);

			currentitem = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, player->currentthing);
			currentitem->flags2 |= MF2_NOTHINK;
			currentitem->angle = player->mo->angle;
			currentitem->tics = -1;

			player->mo->target = currentitem;
			P_UnsetThingPosition(currentitem);
			currentitem->flags |= MF_NOBLOCKMAP;
			currentitem->flags |= MF_NOCLIP;
			P_SetThingPosition(currentitem);
		}
		else if(player->mo->target)
		{
			P_UnsetThingPosition(player->mo->target);
			player->mo->target->x = player->mo->x;
			player->mo->target->y = player->mo->y;
			player->mo->target->z = player->mo->z;
			P_SetThingPosition(player->mo->target);
			player->mo->target->angle = player->mo->angle;
		}

		return;
	}

//////////////////////
// MOVEMENT CODE	//
//////////////////////

	// "THERE IS ONE (control scheme for) NiGHTS!"
	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
	{
		if (netgame || (player == &players[consoleplayer] && !cv_analog.value)
		 || (cv_splitscreen.value && player == &players[secondarydisplayplayer] && !cv_analog2.value) || player->mfspinning)
		{
			#ifndef ABSOLUTEANGLE
				player->mo->angle += (cmd->angleturn<<16);
			#else
				if(!player->climbing)
						player->mo->angle = (cmd->angleturn<<16);
			#endif
		}

		ticruned++;
		if( (cmd->angleturn & TICCMD_RECEIVED) == 0)
			ticmiss++;

		P_GolfMovement(player);
	}
	else if(twodlevel) // 2d-level, so special control applies.
	{
		P_2dMovement(player);
	}
	else
	{
		if (netgame || (player == &players[consoleplayer] && !cv_analog.value)
		 || (cv_splitscreen.value && player == &players[secondarydisplayplayer] && !cv_analog2.value) || player->mfspinning)
		{
			#ifndef ABSOLUTEANGLE
				player->mo->angle += (cmd->angleturn<<16);
			#else
				if(!player->climbing)
						player->mo->angle = (cmd->angleturn<<16);
			#endif
		}

		ticruned++;
		if( (cmd->angleturn & TICCMD_RECEIVED) == 0)
			ticmiss++;

		P_3dMovement(player);
	}

/////////////////////////
// MOVEMENT ANIMATIONS //
/////////////////////////

// Flag variables so it's easy to check
// what state the player is in. Tails 08-19-2000
	if (player->mo->state == &states[S_PLAY_RUN1] || player->mo->state == &states[S_PLAY_RUN2] || player->mo->state == &states[S_PLAY_RUN3] || player->mo->state == &states[S_PLAY_RUN4] || player->mo->state == &states[S_PLAY_RUN5] || player->mo->state == &states[S_PLAY_RUN6] || player->mo->state == &states[S_PLAY_RUN7] || player->mo->state == &states[S_PLAY_RUN8])
	{
		player->walking = 1;
		player->running = player->spinning = 0;
	}
	else if (player->mo->state == &states[S_PLAY_SPD1] || player->mo->state == &states[S_PLAY_SPD2] || player->mo->state == &states[S_PLAY_SPD3] || player->mo->state == &states[S_PLAY_SPD4])
	{
		player->running = 1;
		player->walking = player->spinning = 0;
	}
	else if (player->mo->state == &states[S_PLAY_ATK1] || player->mo->state == &states[S_PLAY_ATK2] || player->mo->state == &states[S_PLAY_ATK3] || player->mo->state == &states[S_PLAY_ATK4])
	{
		player->spinning = 1;
		player->running = player->walking = 0;
	}
	else
		player->walking = player->running = player->spinning = 0;

	if(cmd->forwardmove || cmd->sidemove)
	{
	// If the player is moving fast enough,
	// break into a run!
		if((player->speed > runspeed) && player->walking && (onground))
			P_SetMobjState (player->mo, S_PLAY_SPD1);

	// Otherwise, just walk.
		else if((player->rmomx || player->rmomy) && (player->mo->state == &states[S_PLAY_STND] || player->mo->state == &states[S_PLAY_CARRY] || player->mo->state == &states[S_PLAY_TAP1] || player->mo->state == &states[S_PLAY_TAP2] || player->mo->state == &states[S_PLAY_TEETER1] || player->mo->state == &states[S_PLAY_TEETER2])/* && !(player->powers[pw_super])*/)
			P_SetMobjState (player->mo, S_PLAY_RUN1);
	}

// Prevent Super Sonic from showing Tails 03-25-2001
	if(player->skin==0 && (player->mo->state == &states[S_PLAY_SPC1] || player->mo->state == &states[S_PLAY_SPC2] || player->mo->state == &states[S_PLAY_SPC3] || player->mo->state == &states[S_PLAY_SPC4] || player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL2]))
		P_SetMobjState(player->mo, S_PLAY_STND);

// Adjust the player's animation speed to
// match their velocity.
	if(onground) // Only if on the ground.
	{
		if(player->walking)
		{
			if(player->speed > 12)
				states[player->mo->state->nextstate].tics = 2*NEWTICRATERATIO;
			else if(player->speed > 6)
				states[player->mo->state->nextstate].tics = 3*NEWTICRATERATIO;
			else
				states[player->mo->state->nextstate].tics = 4*NEWTICRATERATIO;
		}
		else if(player->running)
		{
			if(player->speed > 52)
				states[player->mo->state->nextstate].tics = 1*NEWTICRATERATIO;
			else
				states[player->mo->state->nextstate].tics = 2*NEWTICRATERATIO;
		}
	}
	else if(player->mo->state == &states[S_PLAY_FALL1] || player->mo->state == &states[S_PLAY_FALL2])
	{
		fixed_t speed;
		speed = abs(player->mo->momz);
		if(speed < 10*FRACUNIT)
			states[player->mo->state->nextstate].tics = 4*NEWTICRATERATIO;
		else if(speed < 20*FRACUNIT)
			states[player->mo->state->nextstate].tics = 3*NEWTICRATERATIO;
		else if(speed < 30*FRACUNIT)
			states[player->mo->state->nextstate].tics = 2*NEWTICRATERATIO;
		else/* if(speed < 20*FRACUNIT)*/
			states[player->mo->state->nextstate].tics = 1*NEWTICRATERATIO;
	}

	if(player->spinning)
	{
		if(player->speed > 16)
			states[player->mo->state->nextstate].tics = 1*NEWTICRATERATIO;
		else
			states[player->mo->state->nextstate].tics = 2*NEWTICRATERATIO;
	}

	// "If the player is Super Sonic and is pressing the
	// forward/back or left/right keys and running, play
	// Super Sonic's running animation."
/*	if  (player->powers[pw_super] && (cmd->forwardmove || cmd->sidemove)
		&& player->running)
        P_SetMobjState (player->mo, S_PLAY_ABL1);*/

	// If your running animation is playing, and you're
	// going too slow, switch back to the walking frames.
	if(player->running && !(player->speed > runspeed))
		P_SetMobjState (player->mo, S_PLAY_RUN1);

	// If Springing, but travelling DOWNWARD, change back!
	if(player->mo->state == &states[S_PLAY_PLG1] && player->mo->momz < 0)
		P_SetMobjState (player->mo, S_PLAY_FALL1);

	// If Springing but on the ground, change back!
	else if((player->mo->state == &states[S_PLAY_PLG1] || player->mo->state == &states[S_PLAY_FALL1] || player->mo->state == &states[S_PLAY_FALL2] || player->mo->state == &states[S_PLAY_CARRY]) && player->mo->z == player->mo->floorz && !player->mo->momz)
		P_SetMobjState(player->mo, S_PLAY_STND);

	// If you are stopped and are still walking, stand still!
	if(!player->mo->momx && !player->mo->momy && !player->mo->momz && player->walking)
		P_SetMobjState(player->mo, S_PLAY_STND);


//////////////////
//GAMEPLAY STUFF//
//////////////////

	// Make sure you're not "jumping" on the ground Tails 11-05-2000
	if(onground && player->mfjumped == 1 && !player->mo->momz)
	{
		player->mfjumped = 0;
		player->thokked = false;
		P_SetMobjState(player->mo, S_PLAY_STND);
	}
/*
	// Make sure player is in a ball when jumped Tails 03-13-2000
	if (player->mfjumped && !(player->gliding) && !(player->mo->state == &states[S_PLAY_ATK1] || player->mo->state == &states[S_PLAY_ATK2] || player->mo->state == &states[S_PLAY_ATK3] || player->mo->state == &states[S_PLAY_ATK4]))
	{
		P_SetMobjState(player->mo, S_PLAY_ATK1);
	}   
*/

	// Cap the speed limit on a spindash Tails 11-01-2000
	// Up the 60*FRACUNIT number to boost faster, you speed demon you!
	// Note: You must change the MAXMOVE variable in p_local.h to see any effect over 60.
	if(player->dashspeed > MAXMOVE)
		player->dashspeed = MAXMOVE;

	else if(player->dashspeed > 0 && player->dashspeed < 15*FRACUNIT/NEWTICRATERATIO) // Don't let the spindash
		player->dashspeed = 15*FRACUNIT/NEWTICRATERATIO;						 // counter get too high!

	// Fly counter for Tails.
	if(player->fly1 && player->powers[pw_tailsfly] && !(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE))
	{
		if(player->mo->momz < 5*FRACUNIT/NEWTICRATERATIO)
			player->mo->momz += FRACUNIT/2/NEWTICRATERATIO;
		player->fly1--;
	}

	if(cmd->buttons & BT_JUMP && player->powers[pw_tailsfly] && mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE)
	{
		player->mo->momz += FRACUNIT/4/NEWTICRATERATIO;
	}

// Glide MOMZ Tails 11-17-2000
// AKA my own gravity. =)
	if(player->gliding)
	{
		if(player->mo->momz == -2*FRACUNIT/NEWTICRATERATIO)
			player->mo->momz = -2*FRACUNIT/NEWTICRATERATIO;
		else if(player->mo->momz < -2*FRACUNIT/NEWTICRATERATIO)
			player->mo->momz += FRACUNIT*3/4/NEWTICRATERATIO;

		P_InstaThrust(player->mo, player->mo->angle, (20*FRACUNIT + player->glidetime*1000)/NEWTICRATERATIO);
		player->glidetime++;

		if(!player->jumpdown)    // If not holding the jump button
		{
			player->gliding = 0; // down, stop gliding.
			P_SetMobjState(player->mo, S_PLAY_ATK1);
		}
	}
	else if(player->climbing) // 'Deceleration' for climbing on walls.
	{
		if(player->mo->momz > 0)
			player->mo->momz -= .5*FRACUNIT/NEWTICRATERATIO;
		else if(player->mo->momz < 0)
			player->mo->momz += .5*FRACUNIT/NEWTICRATERATIO;
	}

	if(player->charability != 2) // If you can't glide, then why
	{							  // the heck would you be gliding?
		player->gliding = 0;
		player->glidetime = 0;
		player->climbing = 0;
	}

// Jump out of water stuff Tails 12-06-2000
		// Use the info->height for water detection.
	middleheight = player->mo->z + (player->mo->info->height>>1);

	if(player->mo->momz != 0)
	{
		boolean wasinwater;
		wasinwater = P_MobjCheckOldPosWater(player->mo);
		if(((middleheight > player->mo->watertop && middleheight - player->mo->momz < player->mo->watertop)
			|| (middleheight < player->mo->watertop	&& middleheight - player->mo->momz > player->mo->watertop && wasinwater == false)))
		{
			int bubblecount;
			int i;

			if(player->mo->momz > 0)
			{
				player->mo->momz = player->mo->momz*1.706783369803; // Give the player a little out-of-water boost.
				player->mo->momz *= (player->jumpfactor/100.0);
			}

			P_SpawnMobj(player->mo->x, player->mo->y, player->mo->watertop, MT_SPLISH); // Spawn a splash
			S_StartSound(player->mo, sfx_splish); // And make a sound!

			bubblecount = abs(player->mo->momz)>>FRACBITS;
			// Create tons of bubbles
			for(i=0; i<bubblecount; i++)
			{
				if(P_Random() < 32)
					P_SpawnMobj(player->mo->x + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), player->mo->y + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), player->mo->z + (P_Random()<<FRACBITS)/4, MT_MEDIUMBUBBLE)->momz = player->mo->momz < 0 ? player->mo->momz/16 : 0;
				else
					P_SpawnMobj(player->mo->x + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), player->mo->y + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), player->mo->z + (P_Random()<<FRACBITS)/4, MT_SMALLBUBBLE)->momz = player->mo->momz < 0 ? player->mo->momz/16 : 0;
			}
		}
	}

	// If you're running fast enough, you can create splashes as you run in shallow water.
	if(player->mo->z + player->mo->info->height >= player->mo->watertop && player->mo->z <= player->mo->watertop && player->speed > runspeed && leveltime % 5 == 1 && player->mo->momz == 0)
	{
		S_StartSound(P_SpawnMobj(player->mo->x, player->mo->y, player->mo->watertop, MT_SPLISH), sfx_wslap);
	}

//////////////////////////
// RING & SCORE			//
// EXTRA LIFE BONUSES	//
//////////////////////////

	// Ahh ahh! No ring shields in special stages!
	if(player->powers[pw_yellowshield] && gamemap >= sstage_start && gamemap <= sstage_end)
		P_DamageMobj(player->mo, NULL, NULL, 1);

	if(!(gamemap >= sstage_start && gamemap <= sstage_end)
		&& (cv_gametype.value != GT_MATCH && cv_gametype.value != GT_TAG
		&& cv_gametype.value != GT_CTF && cv_gametype.value != GT_CIRCUIT)) // Don't do it in special stages. (or circuit mode Graue 12-22-2003)
	{
		if ((player->health > 100) && (!player->xtralife))
		{
			player->lives += 1;

			if(mariomode)
				S_StartSound(player->mo, sfx_marioa);
			else
			{
				if(player==&players[consoleplayer])
				{
					S_StopMusic();
					S_ChangeMusic(mus_xtlife, false);
				}
				player->powers[pw_extralife] = extralifetics + 1;
			}
			player->xtralife = 1;
		}

		if ((player->health > 200) && (player->xtralife > 0 && player->xtralife < 2))
		{
			player->lives += 1;

			if(mariomode)
				S_StartSound(player->mo, sfx_marioa);
			else
			{
				if(player==&players[consoleplayer])
				{
					S_StopMusic();
					S_ChangeMusic(mus_xtlife, false);
				}
				player->powers[pw_extralife] = extralifetics + 1;
			}
			player->xtralife = 2;
		}
	}

	if(player->score >= 50000+player->xtralife2 && !(cv_gametype.value == GT_MATCH
													|| cv_gametype.value == GT_TAG
													|| cv_gametype.value == GT_CTF
													|| cv_gametype.value == GT_CHAOS
													|| cv_gametype.value == GT_CIRCUIT))
	{
		player->lives++;

		if(mariomode)
			S_StartSound(player->mo, sfx_marioa);
		else
		{
			if(player==&players[consoleplayer])
			{
				S_StopMusic();
				S_ChangeMusic(mus_xtlife, false);
			}
			player->powers[pw_extralife] = extralifetics + 1;
		}
		player->xtralife2 += 50000;
	}

//////////////////////////
// SUPER SONIC STUFF	//
//////////////////////////

// Does player have all emeralds? If so, flag the "Ready For Super!" Tails 04-08-2000
	if((emeralds & EMERALD1) && (emeralds & EMERALD2) && (emeralds & EMERALD3) && (emeralds & EMERALD4) && (emeralds & EMERALD5) && (emeralds & EMERALD6) && (emeralds & EMERALD7) && (player->health > 50))
		player->superready = true;
	else
		player->superready = false;

	if(player->powers[pw_super])
	{
		// If you're super and not Sonic, de-superize!
		if(!(player->skin == 0))
			player->powers[pw_super] = 0;

		// Deplete one ring every second while super
		if((leveltime % TICRATE == 0) && !(player->exiting))
		{
			player->health--;
			player->mo->health--;
		}

		// Ran out of rings while super!
		if((player->powers[pw_super]) && (player->health <= 1))
		{
			player->powers[pw_super] = false;
			player->health = 1;
			player->mo->health = 1;
//			P_SetMobjState(player->mo, S_PLAY); // Return to normal

			// Resume normal music
			if(player->powers[pw_invulnerability] > 1)
			{
				if(mariomode)
					S_ChangeMusic(mus_minvnc, false);
				else
					S_ChangeMusic(mus_invinc, false);
			}
			else if(player->powers[pw_sneakers] > 1)
			{
				S_ChangeMusic(mus_shoes, false);
			}
			else if(!(player->powers[pw_extralife] > 1) && (!player->powers[pw_spacetime]) && ((player->powers[pw_underwater] > 12*TICRATE + 1) || (!player->powers[pw_underwater])))
			{
				S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, 1);
			}

			// If you had a shield, restore its visual significance.
			if(player->powers[pw_blueshield])
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BLUEORB)->target = player->mo;
			else if(player->powers[pw_yellowshield])
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_YELLOWORB)->target = player->mo;
			else if(player->powers[pw_greenshield])
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GREENORB)->target = player->mo;
			else if(player->powers[pw_blackshield])
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BLACKORB)->target = player->mo;
			else if(player->powers[pw_redshield])
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_REDORB)->target = player->mo;
		}

		// If Super Sonic is moving fast enough, run across the water!
/*		if((player->powers[pw_super]) && (player->mo->z < player->mo->waterz+10*FRACUNIT) && (player->mo->z > player->mo->waterz-10*FRACUNIT) && (cmd->forwardmove) && (player->rmomx) && (player->rmomy) && (player->mo->momz < 0) && (player->speed > 10))
		{
			 player->mo->z = player->mo->waterz;
			 player->mo->momz = 0;
//			 P_SpawnSplash (player->mo, (player->mo->z));
		}*/
	}

/////////////////////////
//Special Music Changes//
/////////////////////////

	if(player->powers[pw_extralife] == 1 && player==&players[consoleplayer]) // Extra Life!
	{
		if(player->powers[pw_super])
		{
			S_ChangeMusic(mus_supers, true);
		}
		else if(player->powers[pw_invulnerability] > 1)
		{
			if(mariomode)
				S_ChangeMusic(mus_minvnc, false);
			else
				S_ChangeMusic(mus_invinc, false);
		}
		else if(player->powers[pw_sneakers] > 1)
		{
			S_ChangeMusic(mus_shoes, false);
		}
		else if(player->powers[pw_underwater] <= 11*TICRATE + 1 && player->powers[pw_underwater] > 0)
		{
			S_ChangeMusic(mus_drown, false); // Tails 03-14-2000
		}
		else if(player->powers[pw_spacetime] > 1)
		{
			S_ChangeMusic(mus_drown, false); // Tails 03-14-2000
		}
		else
		{
			S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, 1); // Tails 03-14-2000
		}
	}

///////////////////////////
//LOTS OF UNDERWATER CODE//
///////////////////////////

	// Spawn Sonic's bubbles Tails 03-07-2000
	if(player->mo->eflags & MF_UNDERWATER && !(P_Random() % 16) && !(player->powers[pw_greenshield]))
	{
		P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height / 1.25), MT_SMALLBUBBLE)->threshold = 42;
	}
	else if(player->mo->eflags & MF_UNDERWATER && !(P_Random() % 96) && !(player->powers[pw_greenshield]))
	{
		P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height / 1.25), MT_MEDIUMBUBBLE)->threshold = 42;
	}

	// Display the countdown drown numbers!
	if (player->powers[pw_underwater] == 11*TICRATE + 1 || player->powers[pw_spacetime] == 11*TICRATE + 1)
	{      
		mobj_t* numbermobj;
		numbermobj = P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height) + 8*FRACUNIT, MT_FIVE);
		numbermobj->target = player->mo;
		numbermobj->threshold = 40;
	}
	else if (player->powers[pw_underwater] == 9*TICRATE + 1 || player->powers[pw_spacetime] == 9*TICRATE + 1)
	{    
        mobj_t* numbermobj;
		numbermobj = P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height) + 8*FRACUNIT, MT_FOUR);
		numbermobj->target = player->mo;
		numbermobj->threshold = 40;
	}
	else if (player->powers[pw_underwater] == 7*TICRATE + 1 || player->powers[pw_spacetime] == 7*TICRATE + 1)
	{
		mobj_t* numbermobj;
		numbermobj = P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height) + 8*FRACUNIT, MT_THREE);
		numbermobj->target = player->mo;
		numbermobj->threshold = 40;
	}
	else if (player->powers[pw_underwater] == 5*TICRATE + 1 || player->powers[pw_spacetime] == 5*TICRATE + 1)
	{
		mobj_t* numbermobj;
		numbermobj = P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height) + 8*FRACUNIT, MT_TWO);
		numbermobj->target = player->mo;
		numbermobj->threshold = 40;
	}
	else if (player->powers[pw_underwater] == 3*TICRATE + 1 || player->powers[pw_spacetime] == 3*TICRATE + 1)
	{
		mobj_t* numbermobj;
		numbermobj = P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height) + 8*FRACUNIT, MT_ONE);
		numbermobj->target = player->mo;
		numbermobj->threshold = 40;
	}
	else if (player->powers[pw_underwater] == 1*TICRATE + 1 || player->powers[pw_spacetime] == 1*TICRATE + 1)
	{
		mobj_t* numbermobj;
		numbermobj = P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height) + 8*FRACUNIT, MT_ZERO);
		numbermobj->target = player->mo;
		numbermobj->threshold = 40;
	}
	// Underwater timer runs out Tails 03-05-2000
	else if (player->powers[pw_underwater] == 1)
	{
		mobj_t* killer;

		if(player == &players[consoleplayer])
		{
			S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, 1);
		}

		killer = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DISS);
		killer->threshold = 42; // Special flag that it was drowning which killed you.

		P_DamageMobj(player->mo, killer, killer, 10000);
	}
	else if (player->powers[pw_spacetime] == 1)
	{
		if(player == &players[consoleplayer])
		{
			S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, 1);
		}

		P_DamageMobj(player->mo, NULL, NULL, 10000);
	}

	if(!(player->mo->eflags & MF_UNDERWATER) && player->powers[pw_underwater])
	{
		if(player == &players[consoleplayer] && player->powers[pw_underwater] <= 12*TICRATE + 1)
		{
			if (player->powers[pw_super])
			{
				S_ChangeMusic(mus_supers, true);
			}
			else if (player->powers[pw_invulnerability] > 1
				&& player->powers[pw_extralife] <= 1)
			{
				if(mariomode)
					S_ChangeMusic(mus_minvnc, false);
				else
					S_ChangeMusic(mus_invinc, false);
			}
			else if(player->powers[pw_sneakers] > 1)
			{
				S_ChangeMusic(mus_shoes, false);
			}
			else if(!(player->powers[pw_extralife] > 1))
			{
				S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, 1); // Tails 04-04-2000
			}
		}

		player->powers[pw_underwater] = 0;
	}

	if(player == &players[consoleplayer] && player->powers[pw_spacetime] > 1 && P_InSpaceSector(player->mo) == false)
	{
		if (player->powers[pw_super])
		{
			S_ChangeMusic(mus_supers, true);
		}
		else if (player->powers[pw_invulnerability] > 1
			&& player->powers[pw_extralife] <= 1)
		{
			if(mariomode)
				S_ChangeMusic(mus_minvnc, false);
			else
				S_ChangeMusic(mus_invinc, false);
		}
		else if(player->powers[pw_sneakers] > 1)
		{
			S_ChangeMusic(mus_shoes, false);
		}
		else if(!(player->powers[pw_extralife] > 1))
		{
			S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, 1); // Tails 04-04-2000
		}
	
		player->powers[pw_spacetime] = 0;
	}

	if(player == &players[consoleplayer])
	{
		if (player->powers[pw_underwater] == 11*TICRATE + 1)
		{            
			S_StopMusic();
			S_ChangeMusic(mus_drown, false);
		}

		if (player->powers[pw_underwater] == 25*TICRATE + 1)
		{            
			S_StartSound (0, sfx_wtrdng);
		}
		else if (player->powers[pw_underwater] == 20*TICRATE + 1)
		{            
			S_StartSound (0, sfx_wtrdng);
		}
		else if (player->powers[pw_underwater] == 15*TICRATE + 1)
		{            
			S_StartSound (0, sfx_wtrdng);
		}
	}

////////////////
//TAILS FLYING//
////////////////

	// If not in a fly position, don't think you're flying! Tails 03-05-2000
	if (!(player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL2]))
		player->powers[pw_tailsfly] = 0;

	if(player->charability == 1)
	{
		// Tails Put-Put noise Tails 03-05-2000
		if (player->mo->state == &states[S_PLAY_ABL1] && player->powers[pw_tailsfly])
		{
			S_StartSound (player->mo, sfx_putput);
		}

		// Tails-gets-tired Stuff
		if ((player->powers[pw_tailsfly] == 1) || (player->powers[pw_tailsfly]== 0 && (player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL2])))
		{
			P_SetMobjState (player->mo, S_PLAY_SPC4);
		}
		if ((player->mo->state->nextstate == S_PLAY_SPC1 || player->mo->state->nextstate == S_PLAY_SPC3) && !player->powers[pw_tailsfly])
		{
			S_StartSound (player->mo, sfx_pudpud);
		}
	}

	// Uncomment this to invoke a 10-minute time limit on levels.
	/*if(leveltime > 20999) // one tic off so the time doesn't display 10:00
		P_DamageMobj(player->mo, NULL, NULL, 10000);*/

	// Spawn Invincibility Sparkles
	if(mariomode)
	{
		if(player->powers[pw_invulnerability] && player->powers[pw_super] == false)
			player->mo->flags =  (player->mo->flags & ~MF_TRANSLATION)
						 | ((leveltime % 13)<<MF_TRANSSHIFT);
	}
	else
	{
		if (player->powers[pw_invulnerability] && leveltime % 5 == 1 && player->powers[pw_super] == false)
		{
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z, MT_IVSP);
		}
		else if(player->powers[pw_super] && emeralds & 8) // 'Hyper' Sonic
		{
			// Fancy effect already exists
		}
		else if ((player->powers[pw_super]) && (cmd->forwardmove || cmd->sidemove) && (leveltime % TICRATE == 0) && (player->mo->momx || player->mo->momy))
		{
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z, MT_SUPERSPARK);
		}
	}

	// Resume normal music stuff. Tails
	if ((player->powers[pw_invulnerability] == 1) && (player->powers[pw_super] == false))
	{
		if(mariomode)
		{
			if(player->powers[pw_fireflower])
				player->mo->flags =  (player->mo->flags & ~MF_TRANSLATION)
						 | ((13)<<MF_TRANSSHIFT);
			else
				player->mo->flags =  (player->mo->flags & ~MF_TRANSLATION)
						 | ((player->skincolor)<<MF_TRANSSHIFT);
		}

		if(player == &players[consoleplayer] && !(player->powers[pw_extralife] > 1) && !player->powers[pw_sneakers] && !player->powers[pw_spacetime] && ((player->powers[pw_underwater] > 12*TICRATE + 1) || (!player->powers[pw_underwater])))
		{
			S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, 1);
		}

		// If you have a shield, restore the visual significance.
		if(player->powers[pw_blueshield])
			P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BLUEORB)->target = player->mo;
		else if(player->powers[pw_yellowshield])
			P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_YELLOWORB)->target = player->mo;
		else if(player->powers[pw_greenshield])
			P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GREENORB)->target = player->mo;
		else if(player->powers[pw_blackshield])
			P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BLACKORB)->target = player->mo;
		else if(player->powers[pw_redshield])
			P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_REDORB)->target = player->mo;
	}

	if ((player->powers[pw_sneakers] == 1) && player == &players[consoleplayer])
	{
		if(!(player->powers[pw_extralife] > 1) && !player->powers[pw_invulnerability] && !player->powers[pw_spacetime] && ((player->powers[pw_underwater] > 12*TICRATE + 1) || (!player->powers[pw_underwater])))
		{
			S_ChangeMusic(mapheaderinfo[gamemap-1].musicslot, 1);
		}
	}

	// Show the "THOK!" graphic when spinning quickly across the ground. Tails 11-01-2000
	if(player->mfspinning && player->speed > 15 && !player->mfjumped)
	{
		if(player->skincolor != 0)
		{
			mobj_t* mobj;
			mobj = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z	- (player->mo->info->height - player->mo->height)/3, player->mo->info->damage);
			mobj->flags = (mobj->flags & ~MF_TRANSLATION) | ((player->skincolor)<<MF_TRANSSHIFT); // Tails 08-20-2002
		}
	}


////////////////////////////
//SPINNING AND SPINDASHING//
////////////////////////////

	if(cv_gametype.value != GT_CIRCUIT || cv_circuit_spin.value == 1) // Graue 12-08-2003
	{
		// If the player isn't on the ground, make sure they aren't in a "starting dash" position.
		if (!onground)
		{
			player->mfstartdash = 0;
			player->dashspeed = 0;
		}

		if(player->powers[pw_redshield] && player->mfspinning && (player->rmomx || player->rmomy) && onground && leveltime & 1
			&& !(player->mo->eflags & MF_UNDERWATER) && !(player->mo->eflags & MF_TOUCHWATER))
		{
			fixed_t newx;
			fixed_t newy;
			mobj_t* flame;
			angle_t travelangle;

			travelangle = R_PointToAngle2(player->mo->x, player->mo->y, player->rmomx + player->mo->x, player->rmomy + player->mo->y);

			newx = player->mo->x + P_ReturnThrustX(player->mo, travelangle + ANG45 + ANG90, 24*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, travelangle + ANG45 + ANG90, 24*FRACUNIT);
			flame = P_SpawnMobj(newx, newy, player->mo->floorz+1, MT_SPINFIRE);
			flame->target = player->mo;
			flame->angle = travelangle;
			flame->fuse = TICRATE*6;

			flame->momx = 8;
			P_XYMovement(flame);

			if(flame->z > flame->floorz+1)
				P_SetMobjState(flame, S_DISS);
		
			newx = player->mo->x + P_ReturnThrustX(player->mo, travelangle - ANG45 - ANG90, 24*FRACUNIT);
			newy = player->mo->y + P_ReturnThrustY(player->mo, travelangle - ANG45 - ANG90, 24*FRACUNIT);
			flame = P_SpawnMobj(newx, newy, player->mo->floorz+1, MT_SPINFIRE);
			flame->target = player->mo;
			flame->angle = travelangle;
			flame->fuse = TICRATE*6;

			flame->momx = 8;
			P_XYMovement(flame);

			if(flame->z > flame->floorz+1)
				P_SetMobjState(flame, S_DISS);
		}

		//Spinning and Spindashing
		if(player->charspin && cmd->buttons & BT_USE && !player->exiting && !(player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_flashing])) // subsequent revs
		{
			if(player->speed < 5 && !player->mo->momz && onground && !player->usedown && !player->mfspinning)
			{
				P_ResetScore(player);
				player->mo->momx = player->cmomx;
				player->mo->momy = player->cmomy;
				player->mfstartdash = 1;
				player->mfspinning = 1;
				if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
					player->dashspeed+=((FRACUNIT/NEWTICRATERATIO)>>2); // Graue 12-31-2003
				else
					player->dashspeed+=FRACUNIT/NEWTICRATERATIO; // more speed as you rev more Tails 03-01-2000
				P_SetMobjState (player->mo, S_PLAY_ATK1);
			    player->usedown = true;
			}
			else if(player->mfstartdash)
			{
				if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
					player->dashspeed+=((FRACUNIT/NEWTICRATERATIO)>>2); // Graue 12-31-2003
				else
					player->dashspeed+=FRACUNIT/NEWTICRATERATIO;
				if (leveltime & 1)
				{
					S_StartSound (player->mo, sfx_spndsh); // Make the rev sound!
					// Now spawn the color thok circle.
					if(player->skincolor != 0)
					{
						mobj_t* mobj;
						mobj = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z	- (player->mo->info->height - player->mo->height)/3, player->mo->info->raisestate);
						mobj->flags = (mobj->flags & ~MF_TRANSLATION) | ((player->skincolor)<<MF_TRANSSHIFT); // Tails 08-20-2002
					}
				}
			}
			// If not moving up or down, and travelling faster than a speed of four while not holding down the spin button and not spinning.
			// AKA Just go into a spin on the ground, you idiot. ;)
			else if(!player->climbing && !player->mo->momz && player->speed > 4 && !player->usedown && !player->mfspinning)
			{
				P_ResetScore(player);
				player->mfspinning = 1;
				P_SetMobjState (player->mo, S_PLAY_ATK1);
				S_StartSound (player->mo, sfx_spin);
				player->usedown = true;
			}
		}

		// If you're on the ground and spinning too slowly, get up.
		if(onground && player->mfspinning && (player->rmomx < 5*FRACUNIT/NEWTICRATERATIO && player->rmomx > -5*FRACUNIT/NEWTICRATERATIO) && (player->rmomy < 5*FRACUNIT/NEWTICRATERATIO && player->rmomy > -5*FRACUNIT/NEWTICRATERATIO) && !player->mfstartdash)
		{
			player->mfspinning = 0;
			P_SetMobjState(player->mo, S_PLAY_STND);
			player->mo->momx = player->cmomx;
			player->mo->momy = player->cmomy;
			P_ResetScore(player);
		}

		// Catapult the player from a spindash rev!
		if(onground && !player->usedown && player->dashspeed && player->mfstartdash && player->mfspinning)
		{
			player->mfstartdash = 0;
			P_InstaThrust (player->mo, player->mo->angle, player->dashspeed); // catapult forward ho!! Tails 02-27-2000
			S_StartSound (player->mo, sfx_zoom);
			player->dashspeed = 0;
		
			if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
			{
				player->health++;
				player->mo->health++;
			}
		}
	} // end of spinning/spindashing code

	//added:22-02-98: jumping
	if (!(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF) && cmd->buttons & BT_JUMP && !player->jumpdown && !player->exiting && !(player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_flashing]))
	{
		// can't jump while in air, can't jump while jumping
		if (onground || player->climbing || player->carried)// || (player->mo->eflags & MF_UNDERWATER)) )
		{
			if(player->climbing)
			{
				// Jump this high.
				if(player->powers[pw_super])
					player->mo->momz = 5*FRACUNIT;
				else if (player->mo->eflags & MF_UNDERWATER)
					player->mo->momz = 2*FRACUNIT;
				else
					player->mo->momz = 3.75*FRACUNIT;

				player->mo->angle = player->mo->angle - ANG180; // Turn around from the wall you were climbing.

				if (player==&players[consoleplayer])
					localangle = player->mo->angle; // Adjust the local control angle.
				else if(cv_splitscreen.value && player==&players[secondarydisplayplayer])
					localangle2 = player->mo->angle;

				player->climbing = 0; // Stop climbing, duh!
				P_InstaThrust(player->mo, player->mo->angle, 6*FRACUNIT); // Jump off the wall.
			}
			else if(!(player->mfjumped)) // Tails 9-15-99 Spin Attack
			{
				// Jump this high.
				if(player->carried)
				{
					player->mo->momz = 9*FRACUNIT;
					player->carried = false;
				}
				else if(mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS)
				{
					if(player->mo->eflags & MF_UNDERWATER)
						player->mo->momz = 14.0615384615384615384615384615385*FRACUNIT;
					else
						player->mo->momz = 24*FRACUNIT;
				}
				else if(player->powers[pw_super])
				{
					if(player->mo->eflags & MF_UNDERWATER)
						player->mo->momz = 7.61666666666666666666666666666667*FRACUNIT;
					else
						player->mo->momz = 13*FRACUNIT;

					if(P_InQuicksand(player->mo))
						player->mo->momz /= 2;
				}
				else if(player->mo->eflags & MF_UNDERWATER)
					player->mo->momz = 5.7125*FRACUNIT; // jump this high
				else
				{
					//P_VectorInstaThrust(1*FRACUNIT, 2*FRACUNIT, 3*FRACUNIT, 4*FRACUNIT, 5*FRACUNIT, 6*FRACUNIT, FRACUNIT, FRACUNIT, FRACUNIT, 9.75*FRACUNIT, player->mo);
							
					player->mo->momz = 9.75*FRACUNIT; // Ramp Test Tails

					if(P_InQuicksand(player->mo))
						player->mo->momz /= 2;
				}

				player->jumping = 1;
			}
			player->mo->momz *= (player->jumpfactor/100.0); // Custom height
			player->mo->z++; // set just an eensy above the ground
			P_ResetScore(player);
			player->mfjumped = 1; // Tails 9-15-99 Spin Attack
			S_StartSound (player->mo, sfx_jump); // Play jump sound!
//			if(player->powers[pw_super])
//				P_SetMobjState (player->mo, S_PLAY_ABL1); // Tails 9-24-99
//			else
				P_SetMobjState (player->mo, S_PLAY_ATK1);
		}
		else
		{
			if(cv_gametype.value != GT_CIRCUIT || cv_circuit_specmoves.value == 1) // Graue 12-08-2003
			{
				switch(player->charability)
				{
					case 0:
						// Now it's Sonic's abilities turn!
						if (player->mfjumped)
						{
							if(player->superready && !player->powers[pw_super]) // If you can turn into Super
							{							// and aren't, do it!
								player->powers[pw_super] = true;
								if(player==&players[consoleplayer])
								{
									S_StopMusic();
									S_ChangeMusic(mus_supers, true);
								}
								S_StartSound(player->mo, sfx_supert);
								player->mo->tracer = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_SUPERTRANS);
								player->mo->flags2 |= MF2_DONTDRAW;
								player->mo->momx = player->mo->momy = player->mo->momz = 0;
								player->mfjumped = 0;
								P_SetMobjState(player->mo, S_PLAY_RUN1);
							}
							else // Otherwise, THOK!
							{
								if(!player->thokked)
								{
									P_InstaThrust (player->mo, player->mo->angle, MAXMOVE); // Catapult the player
									S_StartSound (player->mo, player->mo->info->attacksound); // Play the THOK sound
										
									// Now check the player's color so the right THOK object is displayed.
									if(player->skincolor != 0)
									{
										mobj_t* mobj;
										mobj = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z	- (player->mo->info->height - player->mo->height)/3, player->mo->info->painchance);
										mobj->flags = (mobj->flags & ~MF_TRANSLATION) | ((player->skincolor)<<MF_TRANSSHIFT); // Tails 08-20-2002
									}

									// Must press jump while holding down spin to activate.
									if(cv_homing.value && !player->homing && player->mfjumped)
									{
										if(P_LookForEnemies(player))
											if(player->mo->tracer)
												player->homing = 3*TICRATE;
									}

									if(player->powers[pw_super] && (emeralds & EMERALD8))
										player->blackow = 1;

									player->thokked = true;
								}
							}
						}
						break;

					case 1:
						// If currently in the air from a jump, and you pressed the
						// button again and have the ability to fly, do so!
						if(!(player->powers[pw_tailsfly]) && (player->mfjumped))
						{
							P_SetMobjState (player->mo, S_PLAY_ABL1); // Change to the flying animation

							if(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE)
								player->powers[pw_tailsfly] = tailsflytics/2 + 1;
							else
								player->powers[pw_tailsfly] = tailsflytics + 1; // Set the fly timer

							player->mfjumped = player->mfspinning = player->mfstartdash = 0;
						}
						// If currently flying, give an ascend boost.
						else if (player->powers[pw_tailsfly])
						{
							if(player->fly1 == 0)
								player->fly1 = 20;
							else
								player->fly1 = 2;
						}
						break;

					case 2:
						// Now Knuckles-type abilities are checked.
						if (player->mfjumped)
						{
							player->gliding = 1;
							player->glidetime = 0;
							P_SetMobjState(player->mo, S_PLAY_ABL1);
							P_InstaThrust(player->mo, player->mo->angle, 20*FRACUNIT/NEWTICRATERATIO);
							player->mfspinning = 0;
							player->mfstartdash = 0;
						}
						break;
					default:
						break;
				}
			} // end of special moves code
		}
		player->jumpdown = true;
	}
	else if(!(cmd->buttons & BT_JUMP))// If not pressing the jump button
		player->jumpdown = false;

	// If letting go of the jump button while still on ascent, cut the jump height.
	if(player->jumpdown == false && player->mfjumped && player->mo->momz > 0 && player->jumping == 1)
	{
		player->mo->momz = player->mo->momz/2;
		player->jumping = 0;
	}

	// If you're not spinning, you'd better not be spindashing!
	if(!player->mfspinning)
		player->mfstartdash = 0;

	// Sets the minutes/seconds, and synchronizes the "real"
	// amount of time spent in the level. Tails 02-29-2000
	if(!player->exiting)
	{
	    player->minutes = (leveltime/(60*TICRATE));
        player->seconds = (leveltime/TICRATE) % 60;
		player->realtime = leveltime;
	}


//////////////////
//TAG MODE STUFF//
//////////////////
	// Tails 05-08-2001
	if(cv_gametype.value == GT_TAG)
	{
		for(i=0; i<MAXPLAYERS; i++)
		{
			if(tag == 1 && playeringame[i]) // If "IT"'s time is up, make the next player in line "IT" Tails 05-08-2001
			{
				players[i].tagit = 300*TICRATE + 1;
				player->tagit = 0;
				tag = 0;
				CONS_Printf("%s is it!\n", player_names[i]); // Tell everyone who is it! Tails 05-08-2001
			}
			if(players[i].tagit == 1)
				tag = 1;
		}

		// If nobody is it... find someone! Tails 05-08-2001
		if(!players[0].tagit && !players[1].tagit && !players[2].tagit && !players[3].tagit && !players[4].tagit && !players[5].tagit && !players[6].tagit && !players[7].tagit && !players[8].tagit && !players[9].tagit && !players[10].tagit && !players[11].tagit && !players[12].tagit && !players[13].tagit && !players[14].tagit && !players[15].tagit && !players[16].tagit && !players[17].tagit && !players[18].tagit && !players[19].tagit && !players[20].tagit && !players[21].tagit && !players[22].tagit && !players[23].tagit && !players[24].tagit && !players[25].tagit && !players[26].tagit && !players[27].tagit && !players[28].tagit && !players[29].tagit && !players[30].tagit && !players[31].tagit)
		{
			for(i = 0; i<MAXPLAYERS; i++)
			{
				if(playeringame[i])
				{
					players[i].tagit = 300*TICRATE + 1;
					CONS_Printf("%s is it!\n", player_names[i]); // Tell everyone who is it! Tails 05-08-2001
					break;
				}
			}
		}

		// If you're "IT", show a big "IT" over your head for others to see.
		if(player->tagit)
		{
			if(!(player == &players[consoleplayer])) // Don't display it on your own view.
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_TAG);
			player->tagit--;
		}

		// "No-Tag-Zone" Stuff Tails 05-11-2001
		// If in the No-Tag sector and don't have any "tagzone lag",
		// protect the player for 10 seconds.
		if(player->mo->subsector->sector->special == 987 && !player->tagzone && !player->taglag && !player->tagit)
			player->tagzone = 10*TICRATE;

		// If your time is up, set a certain time that you aren't
		// allowed back in, known as "tagzone lag".
		if(player->tagzone == 1)
			player->taglag = 60*TICRATE;

		// Or if you left the no-tag sector, do the same.
		if(player->mo->subsector->sector->special != 987 && player->tagzone)
			player->taglag = 60*TICRATE;

		// If you have "tagzone lag", you shouldn't be protected.
		if(player->taglag)
			player->tagzone = 0;
	}
//////////////////////////
//CAPTURE THE FLAG STUFF//
//////////////////////////

	else if(cv_gametype.value == GT_CTF)
	{
		if(player->gotflag & MF_REDFLAG || player->gotflag & MF_BLUEFLAG) // If you have the flag (duh).
		{
			// Spawn a got-flag message over the head of the player that
			// has it (but not on your own screen if you have the flag).
			if(cv_splitscreen.value)
			{
				if(player->gotflag & MF_REDFLAG)
					P_SpawnMobj(player->mo->x+player->mo->momx, player->mo->y+player->mo->momy, player->mo->z + player->mo->info->height+ player->mo->momz, MT_GOTFLAG);
				if(player->gotflag & MF_BLUEFLAG)
					P_SpawnMobj(player->mo->x+player->mo->momx, player->mo->y+player->mo->momy, player->mo->z + player->mo->info->height+16*FRACUNIT+ player->mo->momz, MT_GOTFLAG2);
			}
			else if((player != &players[consoleplayer]))
			{
				if(player->gotflag & MF_REDFLAG)
					P_SpawnMobj(player->mo->x+player->mo->momx, player->mo->y+player->mo->momy, player->mo->z + player->mo->info->height + player->mo->momz, MT_GOTFLAG);
				if(player->gotflag & MF_BLUEFLAG)
					P_SpawnMobj(player->mo->x+player->mo->momx, player->mo->y+player->mo->momy, player->mo->z + player->mo->info->height+16*FRACUNIT + player->mo->momz, MT_GOTFLAG2);
			}
		}

		// If the player isn't on a team, put them on one! Tails 08-04-2001
		if(player->ctfteam != 1 && player->ctfteam != 2)
		{
			if(cv_splitscreen.value && player == &players[secondarydisplayplayer])
			{
				delayoverride = true;
				CV_SetValue(&cv_preferredteam2, cv_preferredteam2.value);
				delayoverride = true;
				CV_SetValue(&cv_playercolor2, 0);
			}
			else if(player == &players[consoleplayer])
			{
				delayoverride = true;
				CV_SetValue(&cv_preferredteam, cv_preferredteam.value);
				delayoverride = true;
				CV_SetValue(&cv_playercolor, 0);
			}
		}

		if(cv_splitscreen.value)
		{
			// Make sure you are your team's color
			if(players[consoleplayer].ctfteam == 1 && cv_playercolor.value != 6)
			{
				delayoverride = true;
				CV_SetValue(&cv_playercolor, 6);
			}
			else if(players[consoleplayer].ctfteam == 2 && cv_playercolor.value != 7)
			{
				delayoverride = true;
				CV_SetValue(&cv_playercolor, 7);
			}

			if(players[secondarydisplayplayer].ctfteam == 1 && cv_playercolor2.value != 6)
			{
				delayoverride = true;
				CV_SetValue(&cv_playercolor2, 6);
			}
			else if(players[secondarydisplayplayer].ctfteam == 2 && cv_playercolor2.value != 7)
			{
				delayoverride = true;
				CV_SetValue(&cv_playercolor2, 7);
			}
		}
		else
		{
			// Make sure you are your team's color
			if(players[consoleplayer].ctfteam == 1 && cv_playercolor.value != 6)
			{
				delayoverride = true;
				CV_SetValue(&cv_playercolor, 6);
			}
			else if(players[consoleplayer].ctfteam == 2 && cv_playercolor.value != 7)
			{
				delayoverride = true;
				CV_SetValue(&cv_playercolor, 7);
			}
		}
	}

//////////////////
//ANALOG CONTROL//
//////////////////

	if(!netgame && ((player == &players[consoleplayer] && cv_analog.value) || (cv_splitscreen.value && player == &players[secondarydisplayplayer] && cv_analog2.value)) 
		&& (cmd->forwardmove || cmd->sidemove) && !player->climbing)
	{
		// If travelling slow enough, face the way the controls
		// point and not your direction of movement.
		if(player->speed < 5 || player->gliding || player->mo->z > player->mo->floorz)
		{
			tempx = tempy = 0;

			tempangle = thiscam->angle;
			tempangle >>= ANGLETOFINESHIFT;
			tempx += FixedMul(cmd->forwardmove,finecosine[tempangle]);
			tempy += FixedMul(cmd->forwardmove,finesine[tempangle]);

			tempangle = thiscam->angle-ANG90;
			tempangle >>= ANGLETOFINESHIFT;
			tempx += FixedMul(cmd->sidemove,finecosine[tempangle]);
			tempy += FixedMul(cmd->sidemove,finesine[tempangle]);

			tempx = tempx*FRACUNIT;
			tempy = tempy*FRACUNIT;

			player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, player->mo->x + tempx, player->mo->y + tempy);
		}
		// Otherwise, face the direction you're travelling.
		else if (player->walking || player->running || player->spinning || ((player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL2] || player->mo->state == &states[S_PLAY_SPC1] || player->mo->state == &states[S_PLAY_SPC2] || player->mo->state == &states[S_PLAY_SPC3] || player->mo->state == &states[S_PLAY_SPC4]) && player->charability == 1))
			player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, player->rmomx + player->mo->x, player->rmomy + player->mo->y);

		// Update the local angle control.
		if (player==&players[consoleplayer])
			localangle = player->mo->angle;
		else if(cv_splitscreen.value && player==&players[secondarydisplayplayer])
			localangle2 = player->mo->angle;
	}

////////////////////////////
//BLACK SHIELD ACTIVATION,//
//HOMING, AND OTHER COOL  //
//STUFF!                  //
////////////////////////////

	// Black shield activation and Super Sonic move Tails 01-11-2001
	if(cmd->buttons & BT_USE && (player->mfjumped || player->powers[pw_tailsfly])) // Let Tails use it while he's flying...can be useful!
	{
		if(player->powers[pw_super])
			player->mo->momz = 0;
		else if(player->powers[pw_blackshield] && !player->usedown)
		{
			// Don't let Super Sonic or invincibility use it
			if(!(player->powers[pw_super] || player->powers[pw_invulnerability]))
			{
			   player->blackow = 1; // This signals for the BOOM to take effect, as seen below.
			   player->powers[pw_blackshield] = false;
			}
		}
	}

	// This is separate so that P_DamageMobj in p_inter.c can call it, too.
	if(player->blackow)
	{
		P_NukeEnemies(player); // Search for all nearby enemies and nuke their pants off!
		S_StartSound (player->mo, sfx_bkpoof); // Sound the BANG!
		for(i=0;i<MAXPLAYERS;i++)
		{
			if(playeringame[i] && P_AproxDistance(player->mo->x - players[i].mo->x, player->mo->y - players[i].mo->y) < 1536*FRACUNIT)
				players[i].bonuscount += 10; // Flash the palette.
		}
		player->blackow = 0;
	}

	// Uber-secret HOMING option. Experimental!
	if(cv_homing.value)
	{
		// If you've got a target, chase after it!
		if(player->homing && player->mo->tracer)
		{
			mobj_t* mobj;
			mobj = P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z	- (player->mo->info->height - player->mo->height)/3, player->mo->info->painchance);
			mobj->flags = (mobj->flags & ~MF_TRANSLATION) | ((player->skincolor)<<MF_TRANSSHIFT); // Tails 08-20-2002
			P_HomingAttack(player->mo, player->mo->tracer);
		}

		// But if you don't, then stop homing.
		if(player->mo->tracer && (player->mo->tracer->health <= 0 || (player->mo->tracer->flags2 & MF2_FRET)) && player->homing)
		{
			if(player->mo->eflags & MF_UNDERWATER)
				player->mo->momz = 6.3472222222222222222222222222222*FRACUNIT;
			else
				player->mo->momz = 10*FRACUNIT/NEWTICRATERATIO;

			player->mo->momx = player->mo->momy = player->homing = 0;

			if(player->mo->tracer->flags2 & MF2_FRET)
				P_InstaThrust(player->mo, player->mo->angle, -(player->speed << FRACBITS)/8);

			if(!(player->mo->tracer->flags & MF_BOSS))
				player->thokked = false;
		}

		// If you're not jumping, then you obviously wouldn't be homing.
		if(!player->mfjumped && player->homing)
			player->homing = 0;
	}

	if(player->climbing == 1)
	{
		fixed_t platx;
		fixed_t platy;
		subsector_t* glidesector;
		boolean climb = true;

		platx = P_ReturnThrustX(player->mo, player->mo->angle, player->mo->radius + 8*FRACUNIT);
		platy = P_ReturnThrustY(player->mo, player->mo->angle, player->mo->radius + 8*FRACUNIT);

		glidesector = R_PointInSubsector(player->mo->x + platx, player->mo->y + platy);

		if(glidesector->sector != player->mo->subsector->sector)
		{
			boolean floorclimb;
			boolean thrust;
			boolean boostup;
			boolean skyclimber;
			thrust = false;
			floorclimb = false;
			boostup = false;
			skyclimber = false;
				
			if(glidesector->sector->ffloors)
			{
 				ffloor_t* rover;
				for(rover = glidesector->sector->ffloors; rover; rover = rover->next)
				{
					if(!(rover->flags & FF_SOLID))
						continue;

					floorclimb = true;

					// Only supports rovers who are moving like an 'elevator', not just the top or bottom.
					if(rover->master->frontsector->floorspeed && rover->master->frontsector->ceilspeed == 42)
					{
						if(cmd->forwardmove)
							player->mo->momz += rover->master->frontsector->floorspeed;
						else
						{
							player->mo->momz = rover->master->frontsector->floorspeed;
							climb = false;
						}
					}

					if((*rover->bottomheight > player->mo->z) && ((player->mo->z - player->mo->momz) > *rover->bottomheight))
					{
						floorclimb = true;
						boostup = false;
						player->mo->momz = 0;
					}
					if(*rover->bottomheight > player->mo->z + player->mo->height) // Waaaay below the ledge.
					{
						floorclimb = false;
						boostup = false;
						thrust = false;
					}
					if(*rover->topheight < player->mo->z + 16*FRACUNIT)
					{
						floorclimb = false;
						thrust = true;
						boostup = true;
					}

					if(floorclimb)
						break;
				}
			}

			if((glidesector->sector->ceilingheight >= player->mo->z) && ((player->mo->z - player->mo->momz) >= glidesector->sector->ceilingheight))
			{
				floorclimb = true;
				player->mo->momz = 0;
			}
			if(floorclimb == false && glidesector->sector->floorheight < player->mo->z + 16*FRACUNIT && (glidesector->sector->ceilingpic == skyflatnum || glidesector->sector->ceilingheight > (player->mo->z + player->mo->height + 8*FRACUNIT)))
			{
				thrust = true;
				boostup = true;
				// Play climb-up animation here
			}
			if((glidesector->sector->ceilingheight < player->mo->z) && glidesector->sector->ceilingpic == skyflatnum)
			{
				skyclimber = true;
				// Play climb-up animation here
			}

			if(player->mo->z + 16*FRACUNIT < glidesector->sector->floorheight)
			{
				floorclimb = true;

				if(glidesector->sector->floorspeed)
				{
					if(cmd->forwardmove)
						player->mo->momz += glidesector->sector->floorspeed;
					else
					{
						player->mo->momz = glidesector->sector->floorspeed;
						climb = false;
					}
				}
			}
			else if(player->mo->z >= glidesector->sector->ceilingheight)
			{
				floorclimb = true;

				if(glidesector->sector->ceilspeed)
				{
					if(cmd->forwardmove)
						player->mo->momz += glidesector->sector->ceilspeed;
					else
					{
						player->mo->momz = glidesector->sector->ceilspeed;
						climb = false;
					}
				}
			}

			if(player->lastsidehit != -1 && player->lastlinehit != -1)
			{
				thinker_t* think;
				scroll_t* scroller;
				angle_t sideangle;

				for(think = thinkercap.next; think != &thinkercap; think = think->next)
				{
					if(think->function.acp1 != (actionf_p1)T_Scroll)
						continue;

					scroller = (scroll_t *)think;

					if(scroller->type != sc_side)
						continue;

					if(scroller->affectee != player->lastsidehit)
						continue;

					if(cmd->forwardmove)
					{
						player->mo->momz += scroller->dy;
						climb = true;
					}
					else
					{
						player->mo->momz = scroller->dy;
						climb = false;
					}

					sideangle = R_PointToAngle2(lines[player->lastlinehit].v2->x,lines[player->lastlinehit].v2->y,lines[player->lastlinehit].v1->x,lines[player->lastlinehit].v1->y);

					if(cmd->sidemove)
					{
						P_Thrust(player->mo, sideangle, scroller->dx);
						climb = true;
					}
					else
					{
						P_InstaThrust(player->mo, sideangle, scroller->dx);
						climb = false;
					}					
				}
			}

//			P_Thrust(player->mo, player->mo->angle, FRACUNIT);

			if(cmd->sidemove || cmd->forwardmove)
				climb = true;
			else
				climb = false;

			if(player->climbing && climb && (player->mo->momx || player->mo->momy || player->mo->momz)
				&& !(player->mo->state == &states[S_PLAY_CLIMB2]
					|| player->mo->state == &states[S_PLAY_CLIMB3]
					|| player->mo->state == &states[S_PLAY_CLIMB4]
					|| player->mo->state == &states[S_PLAY_CLIMB5]))
				P_SetMobjState(player->mo, S_PLAY_CLIMB2);
			else if ((!(player->mo->momx || player->mo->momy || player->mo->momz) || !climb) && player->mo->state != &states[S_PLAY_CLIMB1])
				P_SetMobjState(player->mo, S_PLAY_CLIMB1);

			if(floorclimb == false)
			{
				if(boostup)
					player->mo->momz += 2*FRACUNIT/NEWTICRATERATIO;
				if(thrust)
					P_InstaThrust(player->mo, player->mo->angle, 4*FRACUNIT); // Lil' boost up.

				player->climbing = 0;
				player->mfjumped = 1;
				P_SetMobjState(player->mo, S_PLAY_ATK1);
			}

			if(skyclimber == true)
			{
				player->climbing = 0;
				player->mfjumped = 1;
				P_SetMobjState(player->mo, S_PLAY_ATK1);
			}
		}
		else
		{
			player->climbing = 0;
			player->mfjumped = 1;
			P_SetMobjState(player->mo, S_PLAY_ATK1);
		}

		if(cmd->sidemove || cmd->forwardmove)
			climb = true;
		else
			climb = false;

		if(player->climbing && climb && (player->mo->momx || player->mo->momy || player->mo->momz)
			&& !(player->mo->state == &states[S_PLAY_CLIMB2]
				|| player->mo->state == &states[S_PLAY_CLIMB3]
				|| player->mo->state == &states[S_PLAY_CLIMB4]
				|| player->mo->state == &states[S_PLAY_CLIMB5]))
			P_SetMobjState(player->mo, S_PLAY_CLIMB2);
		else if ((!(player->mo->momx || player->mo->momy || player->mo->momz) || !climb) && player->mo->state != &states[S_PLAY_CLIMB1])
			P_SetMobjState(player->mo, S_PLAY_CLIMB1);

		if(cmd->buttons & BT_USE)
		{
			player->climbing = 0;
			player->mfjumped = 1;
			P_SetMobjState(player->mo, S_PLAY_ATK1);
			player->mo->momz = 4*FRACUNIT;
			P_InstaThrust(player->mo, player->mo->angle, -4*FRACUNIT);
		}

		if (player==&players[consoleplayer])
			localangle = player->mo->angle;
		else if(cv_splitscreen.value && player==&players[secondarydisplayplayer])
			localangle2 = player->mo->angle;

		if(player->climbing == 0)
			P_SetMobjState(player->mo, S_PLAY_ATK1);
	}

	if(player->climbing > 1)
	{
		P_InstaThrust(player->mo, player->mo->angle, 4*FRACUNIT); // Shove up against the wall
		player->climbing--;
	}

	if(!player->climbing)
	{
		player->lastsidehit = -1;
		player->lastlinehit = -1;
	}

	if(player->bustercount > 0 && (player->mo->health <= 5 || !player->snowbuster))
		player->bustercount = 0;

	// Make sure you're not teetering when you shouldn't be.
	if((player->mo->state == &states[S_PLAY_TEETER1] || player->mo->state == &states[S_PLAY_TEETER2])
		&& (player->mo->momx || player->mo->momy || player->mo->momz))
		P_SetMobjState(player->mo, S_PLAY_STND);

	// Check for teeter!
	if(!player->mo->momz)
	{
		boolean teeter;
		boolean roverfloor; // solid 3d floors?
		boolean checkedforteeter;
		teeter = false;
		checkedforteeter = false;

		for (node = player->mo->touching_sectorlist; node; node = node->m_snext)
		{
			// Ledge teetering. Check if any nearby sectors are low enough from your current one.
			if(!(player->mo->momx && player->mo->momy) && (player->mo->state == &states[S_PLAY_STND]))
			{
				checkedforteeter = true;
				roverfloor = false;
				if (node->m_sector->ffloors)
				{
 					ffloor_t* rover;
					for(rover = node->m_sector->ffloors; rover; rover = rover->next)
					{
						if(!(rover->flags & FF_SOLID))
							continue; // intangible 3d floor

						if(*rover->topheight < node->m_sector->floorheight) // Below the floor
							continue;

						if(*rover->topheight < player->mo->z - 32*FRACUNIT
							|| (*rover->bottomheight > player->mo->z + player->mo->height
							&& player->mo->z > node->m_sector->floorheight + 32*FRACUNIT))
						{
							teeter = true;
							roverfloor = true;
						}
						else
						{
							teeter = false;
							roverfloor = true;
							break;
						}
					}
				}
					
				if(teeter == false && roverfloor == false
					&& (node->m_sector->floorheight < player->mo->z - 32*FRACUNIT))
						teeter = true;
			}

			// Let's check for airbobby platforms while we're at it.
			if(node->m_sector->ffloors)
			{
 				ffloor_t* rover;

				for(rover = node->m_sector->ffloors; rover; rover = rover->next)
				{
					if(rover->flags & FF_AIRBOB)
					{
						if(*rover->topheight == player->mo->z)
						{
							// Graue 11-04-2003 adjustable bob amounts, 11-07-2003 reversability
							if(rover->master->special == 38)
								EV_AirBob(rover->master->frontsector, player, 16, false); // AUGH! GET THIS HEAVY PLAYER OFFA ME!
							if(rover->master->special == 68)
								EV_AirBob(rover->master->frontsector, player, P_AproxDistance(rover->master->dx, rover->master->dy)>>FRACBITS, false);
							if(rover->master->special == 72) // Graue 11-15-2003 change 71 to 72
								EV_AirBob(rover->master->frontsector, player, P_AproxDistance(rover->master->dx, rover->master->dy)>>FRACBITS, true);
						}
					}
				}
			}
		}

		if(checkedforteeter == true && teeter == false) // Backup code
		{
			subsector_t* a = R_PointInSubsector(player->mo->x + 5*FRACUNIT, player->mo->y + 5*FRACUNIT);
			subsector_t* b = R_PointInSubsector(player->mo->x - 5*FRACUNIT, player->mo->y + 5*FRACUNIT);
			subsector_t* c = R_PointInSubsector(player->mo->x + 5*FRACUNIT, player->mo->y - 5*FRACUNIT);
			subsector_t* d = R_PointInSubsector(player->mo->x - 5*FRACUNIT, player->mo->y - 5*FRACUNIT);
			teeter = false;
			roverfloor = false;
			if (a->sector->ffloors)
			{
	 			ffloor_t* rover;
				for(rover = a->sector->ffloors; rover; rover = rover->next)
				{
					if(!(rover->flags & FF_SOLID))
						continue; // intangible 3d floor

					if(*rover->topheight < a->sector->floorheight) // Below the floor
						continue;

					if(*rover->topheight < player->mo->z - 32*FRACUNIT
						|| (*rover->bottomheight > player->mo->z + player->mo->height
						&& player->mo->z > a->sector->floorheight + 32*FRACUNIT))
					{
						teeter = true;
						roverfloor = true;
					}
					else
					{
						teeter = false;
						roverfloor = true;
						break;
					}
				}
			}
			else if (b->sector->ffloors)
			{
	 			ffloor_t* rover;
				for(rover = b->sector->ffloors; rover; rover = rover->next)
				{
					if(!(rover->flags & FF_SOLID))
						continue; // intangible 3d floor

					if(*rover->topheight < b->sector->floorheight) // Below the floor
						continue;

					if(*rover->topheight < player->mo->z - 32*FRACUNIT
						|| (*rover->bottomheight > player->mo->z + player->mo->height
						&& player->mo->z > b->sector->floorheight + 32*FRACUNIT))
					{
						teeter = true;
						roverfloor = true;
					}
					else
					{
						teeter = false;
						roverfloor = true;
						break;
					}
				}
			}
			else if (c->sector->ffloors)
			{
	 			ffloor_t* rover;
				for(rover = c->sector->ffloors; rover; rover = rover->next)
				{
					if(!(rover->flags & FF_SOLID))
						continue; // intangible 3d floor

					if(*rover->topheight < c->sector->floorheight) // Below the floor
						continue;

					if(*rover->topheight < player->mo->z - 32*FRACUNIT
						|| (*rover->bottomheight > player->mo->z + player->mo->height
						&& player->mo->z > c->sector->floorheight + 32*FRACUNIT))
					{
						teeter = true;
						roverfloor = true;
					}
					else
					{
						teeter = false;
						roverfloor = true;
						break;
					}
				}
			}
			else if (d->sector->ffloors)
			{
	 			ffloor_t* rover;
				for(rover = d->sector->ffloors; rover; rover = rover->next)
				{
					if(!(rover->flags & FF_SOLID))
						continue; // intangible 3d floor

					if(*rover->topheight < d->sector->floorheight) // Below the floor
						continue;

					if(*rover->topheight < player->mo->z - 32*FRACUNIT
						|| (*rover->bottomheight > player->mo->z + player->mo->height
						&& player->mo->z > d->sector->floorheight + 32*FRACUNIT))
					{
						teeter = true;
						roverfloor = true;
					}
					else
					{
						teeter = false;
						roverfloor = true;
						break;
					}
				}
			}
			
			
			if(teeter == false && roverfloor == false && (a->sector->floorheight < player->mo->floorz - 32*FRACUNIT
				|| b->sector->floorheight < player->mo->floorz - 32*FRACUNIT
				|| c->sector->floorheight < player->mo->floorz - 32*FRACUNIT
				|| d->sector->floorheight < player->mo->floorz - 32*FRACUNIT))
					teeter = true;
		}

		if(teeter)
			P_SetMobjState(player->mo, S_PLAY_TEETER1);
		else if(checkedforteeter == true && !(player->mo->state == &states[S_PLAY_STND] || player->mo->state == &states[S_PLAY_TAP1] || player->mo->state == &states[S_PLAY_TAP2]))
			P_SetMobjState(player->mo, S_PLAY_STND);
	}

/////////////////
// FIRING CODE //
/////////////////

// These make stuff WAAAAYY easier to understand!
#define HOMING player->powers[pw_homingring]
#define RAIL player->powers[pw_railring]
#define AUTOMATIC player->powers[pw_automaticring]
#define EXPLOSION player->powers[pw_explosionring]

	// check for fire
	//  the missile launcher and bfg do not auto fire
	if (cmd->buttons & BT_ATTACK) // Tails 12-03-2000
	{
		player->bustercount++; // Tails 12-12-2001
		if(mariomode)
		{
			if(!player->attackdown && player->powers[pw_fireflower])
			{
				player->attackdown = true;
				P_SpawnPlayerMissile (player->mo, MT_FIREBALL);
				S_StartSound(player->mo, sfx_thok);
			}
		}
		else if((((cv_gametype.value == GT_MATCH || cv_gametype.value == GT_CTF || cv_ringslinger.value
			|| (cv_circuit_ringthrow.value && cv_gametype.value == GT_CIRCUIT)) && player->mo->health > 1
			&& ((!player->attackdown && !player->weapondelay) || (AUTOMATIC && leveltime & 1)))
			|| (cv_gametype.value == GT_TAG && player->mo->health > 1 && (!player->attackdown &&
			!player->weapondelay) && player->tagit))
			&& !player->exiting) // Graue 12-13-2003: don't fire when you're already done
		{
			player->attackdown = true;

			if(!player->powers[pw_shieldring])
			{
				player->mo->health--;
				player->health--;
			}

			if (HOMING && RAIL && AUTOMATIC && EXPLOSION)
			{
				// Don't even TRY stopping this guy!
				mobj_t* mo;
				int      i;

				player->weapondelay = 2;

				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNAUTOMATICEXPLOSIONHOMING); // Tails 03-13-2001
				if(mo)
				{
					mo->flags2 |= MF2_RAILRING;
					mo->flags2 |= MF2_HOMING;
					mo->flags2 |= MF2_EXPLOSION;
					mo->flags2 |= MF2_AUTOMATIC;
					mo->flags2 |= MF2_DONTDRAW;
				}

				for(i=0; i<256; i++)
				{
					if(mo && mo->flags & MF_MISSILE)
					{
						if(!(mo->flags & MF_NOBLOCKMAP))
						{
							P_UnsetThingPosition(mo);
							mo->flags |= MF_NOBLOCKMAP;
							P_SetThingPosition(mo);
						}

						if(i&1)
							P_SpawnMobj(mo->x, mo->y, mo->z, MT_SPARK);

						P_RailThinker(mo);
					}
					else
						return;
				}
			}
			else if (HOMING && RAIL && AUTOMATIC)
			{
				// Automatic homing rail
				mobj_t* mo;
				int      i;

				player->weapondelay = 2;

				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNAUTOMATICHOMING); // Tails 03-13-2001
				if(mo)
				{
					mo->flags2 |= MF2_RAILRING;
					mo->flags2 |= MF2_HOMING;
					mo->flags2 |= MF2_AUTOMATIC;
					mo->flags2 |= MF2_DONTDRAW;
				}

				for(i=0; i<256; i++)
				{
					if(mo && mo->flags & MF_MISSILE)
					{
						if(!(mo->flags & MF_NOBLOCKMAP))
						{
							P_UnsetThingPosition(mo);
							mo->flags |= MF_NOBLOCKMAP;
							P_SetThingPosition(mo);
						}

						if(i&1)
							P_SpawnMobj(mo->x, mo->y, mo->z, MT_SPARK);

						P_RailThinker(mo);
					}
					else
						return;
				}
			}
			else if (RAIL && AUTOMATIC && EXPLOSION)
			{
				// Automatic exploding rail
				mobj_t* mo;
				int      i;

				player->weapondelay = 2;

				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNAUTOMATICEXPLOSION); // Tails 03-13-2001
				if(mo)
				{
					mo->flags2 |= MF2_RAILRING;
					mo->flags2 |= MF2_EXPLOSION;
					mo->flags2 |= MF2_AUTOMATIC;
					mo->flags2 |= MF2_DONTDRAW;
				}

				for(i=0; i<256; i++)
				{
					if(mo && mo->flags & MF_MISSILE)
					{
						if(!(mo->flags & MF_NOBLOCKMAP))
						{
							P_UnsetThingPosition(mo);
							mo->flags |= MF_NOBLOCKMAP;
							P_SetThingPosition(mo);
						}

						if(i&1)
							P_SpawnMobj(mo->x, mo->y, mo->z, MT_SPARK);

						P_RailThinker(mo);
					}
					else
						return;
				}
			}
			else if (AUTOMATIC && EXPLOSION && HOMING)
			{
				// Automatic exploding homing
				mobj_t* mo;

				player->weapondelay = 2;

				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNAUTOMATICEXPLOSIONHOMING); // Supercalifragilisticespialladocious!
				if(mo)
				{
					mo->flags2 |= MF2_HOMING;
					mo->flags2 |= MF2_EXPLOSION;
					mo->flags2 |= MF2_AUTOMATIC;
				}
			}
			else if (EXPLOSION && HOMING && RAIL)
			{
				// Exploding homing rail
				mobj_t* mo;
				int      i;

				player->weapondelay = 1.5*TICRATE;
				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNHOMINGEXPLOSION); // Tails 03-13-2001
				if(mo)
				{
					mo->flags2 |= MF2_RAILRING;
					mo->flags2 |= MF2_EXPLOSION;
					mo->flags2 |= MF2_HOMING;
					mo->flags2 |= MF2_DONTDRAW;
				}

				for(i=0; i<256; i++)
				{
					if(mo && mo->flags & MF_MISSILE)
					{
						if(!(mo->flags & MF_NOBLOCKMAP))
						{
							P_UnsetThingPosition(mo);
							mo->flags |= MF_NOBLOCKMAP;
							P_SetThingPosition(mo);
						}

						if(i&1)
							P_SpawnMobj(mo->x, mo->y, mo->z, MT_SPARK);

						P_RailThinker(mo);
					}
					else
						return;
				}
			}
			else if (HOMING && RAIL)
			{
				// Homing rail
				mobj_t* mo;
				int      i;

				player->weapondelay = 1.5*TICRATE;
				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNHOMING); // Tails 03-13-2001
				if(mo)
				{
					mo->flags2 |= MF2_RAILRING;
					mo->flags2 |= MF2_HOMING;
					mo->flags2 |= MF2_DONTDRAW;
				}

				for(i=0; i<256; i++)
				{
					if(mo && mo->flags & MF_MISSILE)
					{
						if(!(mo->flags & MF_NOBLOCKMAP))
						{
							P_UnsetThingPosition(mo);
							mo->flags |= MF_NOBLOCKMAP;
							P_SetThingPosition(mo);
						}

						if(i&1)
							P_SpawnMobj(mo->x, mo->y, mo->z, MT_SPARK);

						P_RailThinker(mo);
					}
					else
						return;
				}
			}
			else if (EXPLOSION && RAIL)
			{
				// Explosion rail (Graue 11-04-2003 desc fixed)
				mobj_t* mo;
				int      i;

				player->weapondelay = 1.5*TICRATE;
				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNEXPLOSION); // Tails 03-13-2001
				if(mo)
				{
					mo->flags2 |= MF2_RAILRING;
					mo->flags2 |= MF2_EXPLOSION;
					mo->flags2 |= MF2_DONTDRAW;
				}

				for(i=0; i<256; i++)
				{
					if(mo && mo->flags & MF_MISSILE)
					{
						if(!(mo->flags & MF_NOBLOCKMAP))
						{
							P_UnsetThingPosition(mo);
							mo->flags |= MF_NOBLOCKMAP;
							P_SetThingPosition(mo);
						}

						if(i&1)
							P_SpawnMobj(mo->x, mo->y, mo->z, MT_SPARK);

						P_RailThinker(mo);
					}
					else
						return;
				}
			}
			else if (RAIL && AUTOMATIC)
			{
				// Automatic rail
				mobj_t* mo;
				int      i;

				player->weapondelay = 2;

				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNAUTOMATIC); // Tails 03-13-2001
				if(mo)
				{
					mo->flags2 |= MF2_RAILRING;
					mo->flags2 |= MF2_AUTOMATIC;
					mo->flags2 |= MF2_DONTDRAW;
				}

				for(i=0; i<256; i++)
				{
					if(mo && mo->flags & MF_MISSILE)
					{
						if(!(mo->flags & MF_NOBLOCKMAP))
						{
							P_UnsetThingPosition(mo);
							mo->flags |= MF_NOBLOCKMAP;
							P_SetThingPosition(mo);
						}

						if(i&1)
							P_SpawnMobj(mo->x, mo->y, mo->z, MT_SPARK);

						P_RailThinker(mo);
					}
					else
						return;
				}
			}
			else if (AUTOMATIC && EXPLOSION)
			{
				// Automatic exploding
				mobj_t* mo;

				player->weapondelay = 5;

				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNAUTOMATICEXPLOSION);
				if(mo)
				{
					mo->flags2 |= MF2_AUTOMATIC;
					mo->flags2 |= MF2_EXPLOSION;
				}
			}
			else if (EXPLOSION && HOMING)
			{
				// Homing exploding
				mobj_t* mo;

				player->weapondelay = TICRATE;
				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNHOMINGEXPLOSION);
				if(mo)
				{
					mo->flags2 |= MF2_EXPLOSION;
					mo->flags2 |= MF2_HOMING;
				}
			}
			else if (AUTOMATIC && HOMING)
			{
				// Automatic homing
				mobj_t* mo;

				player->weapondelay = 2;

				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNAUTOMATICHOMING);
				if(mo)
				{
					mo->flags2 |= MF2_AUTOMATIC;
					mo->flags2 |= MF2_HOMING;
				}
			}
			else if (HOMING)
			{
				// Homing ring
				mobj_t* mo;

				player->weapondelay = TICRATE/4;
				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNHOMING);
				if(mo)
				{
					mo->flags2 |= MF2_HOMING;
				}
			}
			else if (RAIL)
			{
				// Rail ring
				mobj_t* mo;
				int      i;
				int      z;

				z = 0;

				player->weapondelay = 1.5*TICRATE;
				mo = P_SpawnPlayerMissile (player->mo, MT_REDRING); // Tails 03-13-2001
				if(mo)
				{
					mo->flags2 |= MF2_RAILRING;
					mo->flags2 |= MF2_DONTDRAW;
				}

				for(i=0; i<256; i++)
				{
					if(mo && mo->flags & MF_MISSILE)
					{
						if(!(mo->flags & MF_NOBLOCKMAP))
						{
							P_UnsetThingPosition(mo);
							mo->flags |= MF_NOBLOCKMAP;
							P_SetThingPosition(mo);
						}

						if(i&1)
							P_SpawnMobj(mo->x, mo->y, mo->z, MT_SPARK);

						P_RailThinker(mo);
					}
					else
						return;
				}
				
			}
			else if (AUTOMATIC)
			{
				// Automatic
				mobj_t* mo;

				player->weapondelay = 2;

				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNAUTOMATIC);
				if(mo)
					mo->flags2 |= MF2_AUTOMATIC;
			}
			else if (EXPLOSION)
			{
				// Exploding
				mobj_t* mo;

				player->weapondelay = TICRATE;
				mo = P_SpawnPlayerMissile (player->mo, MT_THROWNEXPLOSION);
				if(mo)
				{
					mo->flags2 |= MF2_EXPLOSION;
				}
			}
			else // No powers, just a regular ring.
			{
				player->weapondelay = TICRATE/4;
				P_SpawnPlayerMissile (player->mo, MT_REDRING); // Tails 03-13-2001
			}
			return;
		}
	}
	else if (player->bustercount > 2*TICRATE && player->mo->health > 5) // Tails 12-12-2001
	{
		P_SpawnPlayerMissile (player->mo, MT_SNOWBALL); // Tails 12-12-2001
		player->mo->health -= 5; // Tails 12-12-2001
		player->health -= 5; // Tails 12-12-2001
		player->bustercount = 0; // Tails 12-12-2001
		player->attackdown = false; // Tails 12-12-2001
	}
	else
	{
		player->attackdown = false;
		player->bustercount = 0; // Tails 12-12-2001
	}

	// Less height while spinning. Good for spinning under things...?
	if((player->mfspinning) || (player->mfjumped) || (player->powers[pw_tailsfly]) || (player->gliding) || (player->charability == 1 && (player->mo->state == &states[S_PLAY_SPC1] || player->mo->state == &states[S_PLAY_SPC2] || player->mo->state == &states[S_PLAY_SPC3] || player->mo->state == &states[S_PLAY_SPC4])))
		player->mo->height = player->mo->info->height/14*8; // 32
	else
		player->mo->height = player->mo->info->height;

	// Little water sound while touching water - just a nicety.
	if((player->mo->eflags & MF_TOUCHWATER) && !(player->mo->eflags & MF_UNDERWATER))
	{
		if(P_Random() & 1 && leveltime % 35 == 0)
			S_StartSound(player->mo, sfx_floush);
	}

	if(player->taunttimer)
		player->taunttimer--;

	// Check for taunt button Tails 09-06-2002
	if((netgame || multiplayer) && (cmd->buttons & BT_TAUNT) && !player->taunttimer)
	{
		P_PlayTauntSound(player->mo);
		player->taunttimer = 3*TICRATE; // Don't you just hate people who hammer the taunt key?
	}

	if(player->powers[pw_super] && rendermode != render_soft)
		HWR_SuperSonicLightToggle(true);
	else
		HWR_SuperSonicLightToggle(false);

	if(rendermode != render_soft && cv_grfovchange.value)
	{
		fixed_t speed;
		const int runnyspeed = 20;

		speed = R_PointToDist2(player->mo->x + player->rmomx, player->mo->y + player->rmomy, player->mo->x, player->mo->y);
	
		if(speed > (player->normalspeed-5)*FRACUNIT)
			speed = (player->normalspeed-5)*FRACUNIT;

		if(speed >= runnyspeed*FRACUNIT)
			grfovadjust = ((float)speed/FRACUNIT)-runnyspeed;
		else
			grfovadjust = 0.0;

		if(grfovadjust < 0.0)
			grfovadjust = 0.0;
	}
	else
		grfovadjust = 0.0;

	if(player->carried && !(cmd->buttons & BT_JUMP))
	{
		player->mo->height = player->mo->info->height/14*10; // 40
		if((player->mo->tracer->z - player->mo->height - FRACUNIT) >= player->mo->floorz)
			player->mo->z = player->mo->tracer->z - player->mo->height - FRACUNIT;
		else
			player->carried = false;

		player->mo->momx = player->mo->tracer->momx;
		player->mo->momy = player->mo->tracer->momy;
		player->mo->momz = player->mo->tracer->momz;

		if(cv_gametype.value != GT_MATCH && cv_gametype.value != GT_CTF)
		{
			player->mo->angle = player->mo->tracer->angle;

			if(player == &players[consoleplayer])
				localangle = player->mo->angle;
			else if(cv_splitscreen.value && player == &players[secondarydisplayplayer])
				localangle2 = player->mo->angle;
		}

		if(P_AproxDistance(player->mo->x - player->mo->tracer->x, player->mo->y - player->mo->tracer->y) > player->mo->radius)
			player->carried = false;

		P_SetMobjState(player->mo, S_PLAY_CARRY);
	}

	if(cv_shadow.value)
		R_AddFloorSplat (player->mo->subsector, "SHADOW", player->mo->x, player->mo->y, player->mo->floorz, SPLATDRAWMODE_SHADE);

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE)
	{
		if(player->mo->z > player->mo->floorz && player->mo->momz < 0 && !player->powers[pw_flashing])
		{
			if(!(player->mfjumped || player->mfspinning || player->powers[pw_tailsfly] || player->gliding || player->climbing
				|| player->mo->state == &states[S_PLAY_SPC1]
				|| player->mo->state == &states[S_PLAY_SPC2]
				|| player->mo->state == &states[S_PLAY_SPC3]
				|| player->mo->state == &states[S_PLAY_SPC4]))
			{
				if(!(player->mo->state == &states[S_PLAY_FALL1] || player->mo->state == &states[S_PLAY_FALL2]))
					P_SetMobjState(player->mo, S_PLAY_FALL1);
			}
		}
	}

	if(cv_lightdash.value && player->charability == 0 && P_RingNearby(player))
	{
		player->lightdashallowed = true;

		if(cmd->buttons & BT_LIGHTDASH)
		{
			if(!player->attackdown)
			{
				player->lightdash = TICRATE;
				player->attackdown = true;
			}
		}
		else
			player->attackdown = false;
	}
	else
		player->lightdashallowed = false;

	if(cv_lightdash.value && player->lightdash)
	{
		P_LookForRings(player);
		player->powers[pw_flashing] = 2;
	}

	// Look for blocks to bust up
	if(player->mfspinning || player->charability == 2)
	{
		fixed_t oldx;
		fixed_t oldy;

		oldx = player->mo->x;
		oldy = player->mo->y;

		P_UnsetThingPosition(player->mo);
		player->mo->x += player->mo->momx;
		player->mo->y += player->mo->momy;
		P_SetThingPosition(player->mo);

		for (node = player->mo->touching_sectorlist; node; node = node->m_snext)
		{
			if(!node->m_sector)
				break;

			if(node->m_sector->ffloors)
			{
				ffloor_t* rover;

				for(rover = node->m_sector->ffloors; rover; rover = rover->next)
				{
					if((rover->flags & FF_BUSTUP) && !rover->master->frontsector->teamstartsec)
					{
						EV_CrumbleChain(node->m_sector);
						goto bustupdone;
					}
				}
			}
		}
bustupdone:
		P_UnsetThingPosition(player->mo);
		player->mo->x = oldx;
		player->mo->y = oldy;
		P_SetThingPosition(player->mo);
	}

	if(twodlevel && player->gliding)
	{
		fixed_t oldx;
		fixed_t oldy;
		boolean climbed = false;

		oldx = player->mo->x;
		oldy = player->mo->y;

		P_UnsetThingPosition(player->mo);
		player->mo->x += player->mo->momx;
		player->mo->y += player->mo->momy;
		P_SetThingPosition(player->mo);

		for (node = player->mo->touching_sectorlist; node; node = node->m_snext)
		{
			if(!node->m_sector)
				break;

			if(node->m_sector->floorheight > player->mo->z
				|| node->m_sector->ceilingheight < player->mo->z)
			{
				P_ResetPlayer(player);
				player->climbing = 5;
				player->mo->momx = player->mo->momy = player->mo->momz = 0;
				climbed = true;
			}
			else if(node->m_sector->ffloors)
			{
				ffloor_t* rover;

				for(rover = node->m_sector->ffloors; rover; rover = rover->next)
				{
					if((rover->flags & FF_SOLID))
					{
						if(*rover->topheight > player->mo->z && *rover->bottomheight < player->mo->z)
						{
							P_ResetPlayer(player);
							player->climbing = 5;
							player->mo->momx = player->mo->momy = player->mo->momz = 0;
							climbed = true;
						}
					}
				}
			}
		}
		P_UnsetThingPosition(player->mo);
		player->mo->x = oldx;
		player->mo->y = oldy;
		P_SetThingPosition(player->mo);
	}

	// Check for a BOUNCY sector!
	{
		fixed_t oldx;
		fixed_t oldy;
		fixed_t oldz;

		oldx = player->mo->x;
		oldy = player->mo->y;
		oldz = player->mo->z;

		P_UnsetThingPosition(player->mo);
		player->mo->x += player->mo->momx;
		player->mo->y += player->mo->momy;
		player->mo->z += player->mo->momz;
		P_SetThingPosition(player->mo);

		for (node = player->mo->touching_sectorlist; node; node = node->m_snext)
		{
			if(!node->m_sector)
				break;

			if(node->m_sector->ffloors)
			{
				ffloor_t* rover;
				boolean top = true;

				for(rover = node->m_sector->ffloors; rover; rover = rover->next)
				{
					if(player->mo->z > *rover->topheight)
					  continue;

					if(player->mo->z + player->mo->height < *rover->bottomheight)
					  continue;

					if(oldz < *rover->topheight && oldz > *rover->bottomheight)
						top = false;

					if(rover->master->frontsector->special == 14)
					{
						double linedist;

						linedist = P_AproxDistance(rover->master->v1->x-rover->master->v2->x, rover->master->v1->y-rover->master->v2->y) >> FRACBITS;

						linedist /= 100.0;

						if(top)
							player->mo->momz = -player->mo->momz*linedist;
						else
						{
							player->mo->momx = -player->mo->momx*linedist;
							player->mo->momy = -player->mo->momy*linedist;
						}
						goto bouncydone;
					}
				}
			}
		}
bouncydone:
		P_UnsetThingPosition(player->mo);
		player->mo->x = oldx;
		player->mo->y = oldy;
		player->mo->z = oldz;
		P_SetThingPosition(player->mo);
	}


	// Look for Quicksand!
	if(player->mo->subsector->sector->ffloors)
	{
		ffloor_t* rover;
		fixed_t sinkspeed;

		for(rover = player->mo->subsector->sector->ffloors; rover; rover = rover->next)
		{
			if(!(rover->flags & FF_QUICKSAND))
				continue;

			if(*rover->topheight >= player->mo->z && *rover->bottomheight < player->mo->z + player->mo->height)
			{
				sinkspeed = P_AproxDistance(rover->master->v1->x - rover->master->v2->x, rover->master->v1->y - rover->master->v2->y)/8;

				sinkspeed /= TICRATE;

				player->mo->z -= sinkspeed;

				player->mo->momx /= sinkspeed;
				player->mo->momy /= sinkspeed;

				if(!player->powers[pw_spacetime] && player->mo->z + player->mo->height/2 < *rover->topheight)
				{
					player->powers[pw_spacetime] = spacetimetics + 2;
					S_ChangeMusic(mus_drown, false);
				}
			}
		}
	}
}

boolean P_RingNearby(player_t *player) // Is a ring in range?
{
        mobj_t *mo;
        thinker_t *think;
		mobj_t* closest = NULL;

        for(think = thinkercap.next; think != &thinkercap; think = think->next)
        {
                if(think->function.acp1 != (actionf_p1)P_MobjThinker)
                { // Not a mobj thinker
                        continue;
                }
                mo = (mobj_t *)think;
                if((mo->health <= 0) || (mo->state == &states[S_DISS]))
                { // Not a valid ring
                        continue;
                }
				
				if(!(mo->type == MT_RING || mo->type == MT_COIN))
				{
					continue;
				}

                if(P_AproxDistance(P_AproxDistance(player->mo->x-mo->x, player->mo->y-mo->y), player->mo->z-mo->z)
                        > 192*FRACUNIT)
                { // Out of range
                        continue;
                }

                if(!P_CheckSight(player->mo, mo))
                { // Out of sight
                        continue;
                }

				if(closest && P_AproxDistance(P_AproxDistance(player->mo->x-mo->x, player->mo->y-mo->y), player->mo->z-mo->z) > P_AproxDistance(P_AproxDistance(player->mo->x-closest->x, player->mo->y-closest->y), player->mo->z-closest->z))
					continue;

                // Found a target
				closest = mo;
        }

		if(closest)
			return true;

        return false;
}

void P_LookForRings(player_t *player)
{
        mobj_t *mo;
        thinker_t *think;
		boolean found = false;

		player->mo->target = NULL;
		player->mo->tracer = NULL;

        for(think = thinkercap.next; think != &thinkercap; think = think->next)
        {
                if(think->function.acp1 != (actionf_p1)P_MobjThinker)
                { // Not a mobj thinker
                        continue;
                }
                mo = (mobj_t *)think;
                if((mo->health <= 0) || (mo->state == &states[S_DISS]))
                { // Not a valid ring
                        continue;
                }
				
				if(!(mo->type == MT_RING || mo->type == MT_COIN))
				{
					continue;
				}

                if(P_AproxDistance(P_AproxDistance(player->mo->x-mo->x, player->mo->y-mo->y), player->mo->z-mo->z)
                        > 192*FRACUNIT)
                { // Out of range
                        continue;
                }

                if(!P_CheckSight(player->mo, mo))
                { // Out of sight
                        continue;
                }

				if(player->mo->target && P_AproxDistance(P_AproxDistance(player->mo->x-mo->x, player->mo->y-mo->y), player->mo->z-mo->z) > P_AproxDistance(P_AproxDistance(player->mo->x-player->mo->target->x, player->mo->y-player->mo->target->y), player->mo->z-player->mo->target->z))
					continue;

                // Found a target
				found = true;
                player->mo->target = mo;
				player->mo->tracer = mo;
        }

		if(found)
		{
			P_ResetPlayer(player);
			P_SetMobjState(player->mo, S_PLAY_FALL1);
			P_ResetScore(player);
			P_LightDash(player->mo, player->mo->target);
			return;
		}
		player->mo->momx /= 2;
		player->mo->momy /= 2;
		player->mo->momz /= 2;
		player->lightdash = false;
        return;
}

void P_LightDash (mobj_t* source, mobj_t* enemy) // Home in on your target Tails 06-19-2001
{
    fixed_t     dist;
    mobj_t*     dest;

	if(!source->tracer)
		return; // Nothing to home in on!

    // adjust direction
    dest = source->tracer;

    if (!dest)
        return;

    // change angle
	source->angle = R_PointToAngle2(source->x, source->y, enemy->x, enemy->y);
	    if (source->player)
		{
			if(source->player==&players[consoleplayer])
				localangle = source->angle;
			else if(cv_splitscreen.value && source->player == &players[secondarydisplayplayer])
				localangle2 = source->angle;
		}

    // change slope
	dist = P_AproxDistance(P_AproxDistance(dest->x - source->x, dest->y - source->y), dest->z - source->z);

    if (dist < 1)
        dist = 1;

	source->momx = FixedMul(FixedDiv(dest->x - source->x, dist), (MAXMOVE));
	source->momy = FixedMul(FixedDiv(dest->y - source->y, dist), (MAXMOVE));
	source->momz = FixedMul(FixedDiv(dest->z - source->z, dist), (MAXMOVE));

	return; // Tails 06-20-2001
}

//
// P_NukeEnemies
// Looks for something you can hit - Used for black shield Tails 07-12-2001
//
mobj_t*         bombsource;
mobj_t*         bombspot;
boolean P_NukeEnemies (player_t* player)
{
    int         x;
    int         y;

    int         xl;
    int         xh;
    int         yl;
    int         yh;

	int			i;
	mobj_t*		mo;

    fixed_t     dist;

    dist = 1536<<FRACBITS;
    yh = (player->mo->y + dist - bmaporgy)>>MAPBLOCKSHIFT;
    yl = (player->mo->y - dist - bmaporgy)>>MAPBLOCKSHIFT;
    xh = (player->mo->x + dist - bmaporgx)>>MAPBLOCKSHIFT;
    xl = (player->mo->x - dist - bmaporgx)>>MAPBLOCKSHIFT;
    bombspot = player->mo;
    bombsource = player->mo;

	for(i=0; i<16; i++)
	{
    mo = P_SpawnMobj(player->mo->x,
                     player->mo->y,
                     player->mo->z,
                     MT_SUPERSPARK);
	mo->momx = (sin(i*22.5) * 60 * FRACUNIT) /NEWTICRATERATIO;
	mo->momy = (cos(i*22.5) * 60 * FRACUNIT) /NEWTICRATERATIO;
	}

    for (y=yl ; y<=yh ; y++)
        for (x=xl ; x<=xh ; x++)
            P_BlockThingsIterator (x, y, PIT_NukeEnemies );

		return true;
}

boolean PIT_NukeEnemies (mobj_t* thing)
{
    fixed_t     dx;
    fixed_t     dy;
    fixed_t     dist;

    if (!(thing->flags & MF_SHOOTABLE) )
        return true;

	if(thing->flags & MF_MONITOR)
		return true; // Monitors cannot be 'nuked'.

    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);

    dist = dx>dy ? dx : dy;
    dist -= thing->radius;

	if(abs(thing->z+(thing->height>>1) - bombspot->z) > 1024*FRACUNIT)
		return true;

    dist >>= FRACBITS;

    if (dist < 0)
        dist = 0;

    if (dist > 1536)
        return true;    // out of range

	if(cv_gametype.value == GT_COOP && thing->type == MT_PLAYER)
		return true; // Don't hurt players in Co-Op!

	if(thing == bombsource) // Don't hurt yourself!!
			return true;

// Uncomment P_CheckSight to prevent black shield from going through walls Tails 07-13-2001
//    if ( P_CheckSight (thing, bombspot) )
	P_DamageMobj (thing, bombspot, bombsource, 2); // Tails 01-11-2001

    return true;
}


//
// P_LookForEnemies
// Looks for something you can hit - Used for homing attack Tails 06-20-2001
// Includes monitors and springs!
//
boolean P_LookForEnemies(player_t *player)
{
        int count;
        mobj_t *mo;
        thinker_t *think;
		mobj_t* closestmo = NULL;
		angle_t an;

        count = 0;
        for(think = thinkercap.next; think != &thinkercap; think = think->next)
        {
                if(think->function.acp1 != (actionf_p1)P_MobjThinker)
                { // Not a mobj thinker
                        continue;
                }
                mo = (mobj_t *)think;
                if((!(mo->flags&MF_COUNTKILL) && !(mo->flags & MF_MONITOR) && !(mo->flags & MF_SPRING)))
                { // Not a valid monster
                        continue;
                }

				if(mo->health <= 0)
					continue;

				if(mo == player->mo)
					continue;

				if(mo->flags2 & MF2_FRET)
					continue;

				if(mo->type == MT_DETON) // Don't be STUPID, Sonic!
					continue;

				if(mo->flags & MF_MONITOR)
				{
					if(mo->state == &states[S_MONITOREXPLOSION5])
						continue;
				}

                if(P_AproxDistance(P_AproxDistance(player->mo->x-mo->x, player->mo->y-mo->y), player->mo->z-mo->z)
                        > RING_DIST)
                { // Out of range
                        continue;
                }

				if(mo->type == MT_PLAYER) // Don't chase after other players!
					continue;

				if(closestmo && P_AproxDistance(P_AproxDistance(player->mo->x-mo->x, player->mo->y-mo->y), player->mo->z-mo->z)
					> P_AproxDistance(P_AproxDistance(player->mo->x-closestmo->x, player->mo->y-closestmo->y), player->mo->z-closestmo->z))
					continue;

				an = R_PointToAngle2 (player->mo->x,
									  player->mo->y,
									  mo->x,
									  mo->y) - player->mo->angle;

				if (an > ANG90 && an < ANG270)
					continue;   // behind back

				player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, mo->x, mo->y);

                if(!P_CheckSight(player->mo, mo))
                { // Out of sight
                        continue;
                }

				closestmo = mo;
        }

		if(closestmo)
		{
            // Found a target monster
            player->mo->target = closestmo;
			player->mo->tracer = closestmo;
            return(true);
		}
        return(false);
}

void P_HomingAttack (mobj_t* source, mobj_t* enemy) // Home in on your target Tails 06-19-2001
{
    fixed_t     dist;
    mobj_t*     dest;

	if(!(enemy->health))
		return;

	if(!source->tracer)
		return; // Nothing to home in on!

    // adjust direction
    dest = source->tracer;

    if (!dest || dest->health <= 0)
        return;

    // change angle
	source->angle = R_PointToAngle2(source->x, source->y, enemy->x, enemy->y);
	    if (source->player)
		{
			if(source->player==&players[consoleplayer])
				localangle = source->angle;
			else if(cv_splitscreen.value && source->player == &players[secondarydisplayplayer])
				localangle2 = source->angle;
		}

    // change slope
	dist = P_AproxDistance(P_AproxDistance(dest->x - source->x, dest->y - source->y), dest->z - source->z);

    if (dist < 1)
        dist = 1;

	if(source->type == MT_DETON && enemy->player) // For Deton Chase
	{
		source->momx = FixedMul(FixedDiv(dest->x - source->x, dist), ((enemy->player->normalspeed * FRACUNIT)* 0.85));
		source->momy = FixedMul(FixedDiv(dest->y - source->y, dist), ((enemy->player->normalspeed * FRACUNIT)* 0.85));
		source->momz = FixedMul(FixedDiv(dest->z - source->z, dist), ((enemy->player->normalspeed * FRACUNIT)* 0.85));
	}
	else if(source->type != MT_PLAYER)
	{
		if(source->threshold == 32000)
		{
			source->momx = FixedMul(FixedDiv(dest->x - source->x, dist), source->info->speed/3);
			source->momy = FixedMul(FixedDiv(dest->y - source->y, dist), source->info->speed/3);
			source->momz = FixedMul(FixedDiv(dest->z - source->z, dist), source->info->speed/3);
		}
		else
		{
			source->momx = FixedMul(FixedDiv(dest->x - source->x, dist), source->info->speed);
			source->momy = FixedMul(FixedDiv(dest->y - source->y, dist), source->info->speed);
			source->momz = FixedMul(FixedDiv(dest->z - source->z, dist), source->info->speed);
		}
	}
	else
	{
		source->momx = FixedMul(FixedDiv(dest->x - source->x, dist), (40*FRACUNIT));
		source->momy = FixedMul(FixedDiv(dest->y - source->y, dist), (40*FRACUNIT));
		source->momz = FixedMul(FixedDiv(dest->z - source->z, dist), (40*FRACUNIT));
	}

	return; // Tails 06-20-2001
}

// Search for emeralds Tails 12-20-2001
void P_FindEmerald (player_t* player)
{
    thinker_t*  th;
    mobj_t*     mo2;

    // scan the remaining thinkers
    // to find all emeralds
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;

        mo2 = (mobj_t *)th;
        if (!hunt1 && (mo2->type == MT_EMERHUNT || mo2->type == MT_EMESHUNT || mo2->type == MT_EMETHUNT))
        {
            hunt1 = mo2; // Found it!
        }
		else if(!hunt2 && (mo2->type == MT_EMERHUNT || mo2->type == MT_EMESHUNT || mo2->type == MT_EMETHUNT))
        {
            hunt2 = mo2; // Found it!
        }
		else if(!hunt3 && (mo2->type == MT_EMERHUNT || mo2->type == MT_EMESHUNT || mo2->type == MT_EMETHUNT))
        {
            hunt3 = mo2; // Found it!
        }
    }
	return;
}

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
#define ANG5    (ANG90/18)

void P_DeathThink (player_t* player)
{
	ticcmd_t*           cmd;

	cmd = &player->cmd;

    // fall to the ground
    if (player->viewheight > 6*FRACUNIT)
        player->viewheight -= FRACUNIT;

    if (player->viewheight < 6*FRACUNIT)
        player->viewheight = 6*FRACUNIT;

    player->deltaviewheight = 0;
    onground = player->mo->z <= player->mo->floorz;

    P_CalcHeight (player);

	if(!player->deadtimer)
		player->deadtimer = 60*TICRATE;

	player->deadtimer--;

	if ((cmd->buttons & BT_JUMP) && (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_CHAOS || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CTF || cv_gametype.value == GT_CIRCUIT)) // Tails 05-06-2001
		player->playerstate = PST_REBORN; // Tails 05-06-2001
	else if(player->deadtimer < 30*TICRATE && cv_gametype.value == GT_TAG) // Tails 08-17-2001
		player->playerstate = PST_REBORN; // Tails 08-17-2001
	else if(player->lives > 0)
	{
		if ((cmd->buttons & BT_JUMP) && player->deadtimer < 59*TICRATE && !(cv_gametype.value == GT_RACE)) // Respawn with Jump button Tails 12-04-99
			player->playerstate = PST_REBORN;

		if ((cmd->buttons & BT_JUMP) && (cv_gametype.value == GT_RACE)) // Tails 05-06-2001
			player->playerstate = PST_REBORN; // Tails 05-06-2001

		if(player->deadtimer < 56*TICRATE && cv_gametype.value == GT_COOP) // Tails 05-06-2001
			player->playerstate = PST_REBORN; // Tails 05-06-2001

		if(player->mo->z < R_PointInSubsector(player->mo->x, player->mo->y)->sector->floorheight - 10000*FRACUNIT)
			player->playerstate = PST_REBORN;
	}

	if(player->mo->momz < -30*FRACUNIT)
		player->mo->momz = -30*FRACUNIT;

	if(player->mo->z + player->mo->momz < player->mo->subsector->sector->floorheight - 5120*FRACUNIT)
	{
		player->mo->momz = 0;
		player->mo->z = player->mo->subsector->sector->floorheight - 5120*FRACUNIT;
	}

	// Keep time rolling in race mode Tails 08-29-2002
	if((cv_gametype.value == GT_RACE || cv_gametype.value == GT_CIRCUIT) && !(countdown2 && !countdown) && !player->exiting)
	{
		player->minutes = (leveltime/(60*TICRATE));
		player->seconds = (leveltime/TICRATE) % 60;
		player->realtime = leveltime;
	}
}

//
// P_MoveCamera : make sure the camera is not outside the world
//                and looks at the player avatar
//

camera_t camera; // Two cameras.. one for split!
camera_t camera2;

//#define VIEWCAM_DIST    (128<<FRACBITS)
//#define VIEWCAM_HEIGHT  (20<<FRACBITS)

void CV_CamRotate_OnChange(void)
{
	if(cv_cam_rotate.value > 359)
		CV_SetValue (&cv_cam_rotate, 0);
}

void CV_CamRotate2_OnChange(void)
{
	if(cv_cam2_rotate.value > 359)
		CV_SetValue (&cv_cam2_rotate, 0);
}

// Both cameras share these variables... hey, I'm lazy!
// Not anymore! New variables for the split player!
CV_PossibleValue_t rotation_cons_t[]={{1,"MIN"},{45,"MAX"},{0,NULL}};

consvar_t cv_cam_dist   = {"cam_dist"  ,"128"  ,CV_FLOAT,NULL};
consvar_t cv_cam_height = {"cam_height", "20"   ,CV_FLOAT,NULL};
consvar_t cv_cam_still  = {"cam_still"  ,"0", 0,CV_OnOff}; // Tails 07-02-2001
consvar_t cv_cam_speed  = {"cam_speed" ,  "0.25",CV_FLOAT,NULL};
consvar_t cv_cam_rotate = {"cam_rotate","0", CV_CALL|CV_NOINIT, CV_Unsigned, CV_CamRotate_OnChange };
consvar_t cv_cam_rotspeed = {"cam_rotspeed","10", 0,rotation_cons_t};
consvar_t cv_cam2_dist   = {"cam2_dist"  ,"128"  ,CV_FLOAT,NULL};
consvar_t cv_cam2_height = {"cam2_height", "20"   ,CV_FLOAT,NULL};
consvar_t cv_cam2_still  = {"cam2_still"  ,"0", 0,CV_OnOff};
consvar_t cv_cam2_speed  = {"cam2_speed" ,  "0.25",CV_FLOAT,NULL};
consvar_t cv_cam2_rotate = {"cam2_rotate","0", CV_CALL|CV_NOINIT, CV_Unsigned, CV_CamRotate2_OnChange };
consvar_t cv_cam2_rotspeed = {"cam2_rotspeed","10", 0,rotation_cons_t};

fixed_t t_cam_dist = -42;
fixed_t t_cam_height = -42;
fixed_t t_cam_rotate = -42;
fixed_t t_cam2_dist = -42;
fixed_t t_cam2_height = -42;
fixed_t t_cam2_rotate = -42;


void P_ResetCamera (player_t *player, camera_t* thiscam)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

	if(!player->mo)
		return;

	if(player->mo->health <= 0)
		return;

    thiscam->chase = true;
    x = player->mo->x;
    y = player->mo->y;
    z = player->mo->z + (cv_viewheight.value<<FRACBITS);

    // hey we should make sure that the sounds are heard from the camera
    // instead of the marine's head : TO DO

    // set bits for the camera
    thiscam->x = x;
    thiscam->y = y;
    thiscam->z = z;

    thiscam->angle = player->mo->angle;
    thiscam->aiming = 0;

    thiscam->subsector = R_PointInSubsector(thiscam->x,thiscam->y);

//	P_CheckCameraPosition(camera.x, camera.y);

	thiscam->radius = mobjinfo[MT_CHASECAM].radius;
	thiscam->height = mobjinfo[MT_CHASECAM].height;
}

boolean PTR_FindCameraPoint (intercept_t* in)
{
/*    int         side;
    fixed_t             slope;
    fixed_t             dist;
    line_t*             li;

    li = in->d.line;

    if ( !(li->flags & ML_TWOSIDED) )
        return false;

    // crosses a two sided line
    //added:16-02-98: Fab comments : sets opentop, openbottom, openrange
    //                lowfloor is the height of the lowest floor
    //                         (be it front or back)
    P_LineOpening (li);

    dist = FixedMul (attackrange, in->frac);

    if (li->frontsector->floorheight != li->backsector->floorheight)
    {
        //added:18-02-98: comments :
        // find the slope aiming on the border between the two floors
        slope = FixedDiv (openbottom - cameraz , dist);
        if (slope > aimslope)
            return false;
    }

    if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
    {
        slope = FixedDiv (opentop - shootz , dist);
        if (slope < aimslope)
            goto hitline;
    }

    return true;

    // hit line
  hitline:*/
    // stop the search
    return false;
}


fixed_t cameraz;
extern consvar_t cv_chasecam; // Tails
extern consvar_t cv_chasecam2;
extern void P_CameraThinker(camera_t* camera);

void P_MoveChaseCamera (player_t *player, camera_t* thiscam)
{
    angle_t             angle;
    fixed_t             x,y,z ,viewpointx,viewpointy;
    fixed_t             dist;
    mobj_t*             mo;
    subsector_t*        newsubsec;
    float               f1,f2;

	if(!cv_chasecam.value && thiscam == &camera)
		return;

	if(!cv_chasecam2.value && thiscam == &camera2)
		return;

    if (!thiscam->chase)
        P_ResetCamera (player, thiscam);

	if(!player)
		return;

    mo = player->mo;

	if(!mo)
		return;

	P_CameraThinker(thiscam);

	if(twodlevel) // 2d level Tails 04-02-2003
		angle = ANG90;
	else if (thiscam == &camera ? cv_cam_still.value == true : cv_cam2_still.value == true) // Tails 07-02-2001
		angle = thiscam->angle;
	else if(player->nightsmode && player->mo->target && player->mo->target->flags & MF_AMBUSH)
		angle = R_PointToAngle2(player->mo->target->x, player->mo->target->y, thiscam->x, thiscam->y);
	else if(player->nightsmode && player->mo->target) // NiGHTS Level Tails 06-10-2001
		angle = R_PointToAngle2(thiscam->x, thiscam->y, player->mo->target->x, player->mo->target->y);
	else if(((player == &players[consoleplayer] && cv_analog.value)
		|| (cv_splitscreen.value && player == &players[secondarydisplayplayer] && cv_analog2.value))) // Analog Test Tails 06-10-2001
	{
		angle = R_PointToAngle2(thiscam->x, thiscam->y, mo->x, mo->y);

		if(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE)
		{
			thinker_t*  th;
			mobj_t*     mo2;
			boolean foundhim = false;

			// scan the remaining thinkers
			// to find all emeralds
			for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
			{
				if (th->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;

				mo2 = (mobj_t *)th;
				if((mo2->flags & MF_BOSS)
					&& mo2->health > 0)
				{
					foundhim = true;
					break;
				}
			}
			
			if(foundhim)
				angle = R_PointToAngle2(thiscam->x, thiscam->y, mo2->x, mo2->y);
		}
	}
	else
		angle = mo->angle + ((thiscam == &camera ? cv_cam_rotate.value : cv_cam2_rotate.value)*ANGLE_1);

	if(cv_analog.value && ((thiscam == &camera && t_cam_rotate != -42) || (thiscam == &camera2 && t_cam2_rotate != -42)))
	{
		angle = ((thiscam == &camera ? cv_cam_rotate.value : cv_cam2_rotate.value)*ANGLE_1);
		thiscam->angle = angle;
	}

	if(!cv_objectplace.value && !twodlevel)
	{
		if(player->cmd.buttons & BT_CAMLEFT)
		{
			if(thiscam == &camera)
			{
				if(cv_analog.value)
					angle-=cv_cam_rotspeed.value*ANGLE_1;
				else
					CV_SetValue(&cv_cam_rotate, cv_cam_rotate.value==0 ? 358 : cv_cam_rotate.value - 2);
			}
			else
			{
				if(cv_analog2.value)
					angle-=cv_cam2_rotspeed.value*ANGLE_1;
				else
					CV_SetValue(&cv_cam2_rotate, cv_cam2_rotate.value==0 ? 358 : cv_cam2_rotate.value - 2);
			}
		}
		else if(player->cmd.buttons & BT_CAMRIGHT)
		{
			if(thiscam == &camera)
			{
				if(cv_analog.value)
					angle+=cv_cam_rotspeed.value*ANGLE_1;
				else
					CV_SetValue(&cv_cam_rotate, cv_cam_rotate.value + 2);
			}
			else
			{
				if(cv_analog2.value)
					angle+=cv_cam2_rotspeed.value*ANGLE_1;
				else
					CV_SetValue(&cv_cam2_rotate, cv_cam2_rotate.value + 2);
			}
		}
	}

    // sets ideal cam pos
	if(twodlevel)
		dist = 320<<FRACBITS;
	else
		dist  = (thiscam == &camera ? cv_cam_dist.value : cv_cam2_dist.value);

    x = mo->x - FixedMul( finecosine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
    y = mo->y - FixedMul(   finesine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
    z = mo->z + (cv_viewheight.value<<FRACBITS) + (thiscam == &camera ? cv_cam_height.value : cv_cam2_height.value);

/*    P_PathTraverse ( mo->x, mo->y, x, y, PT_ADDLINES, PTR_UseTraverse );*/

    // move camera down to move under lower ceilings
    newsubsec = R_IsPointInSubsector ((mo->x + thiscam->x)>>1,(mo->y + thiscam->y)>>1);

    if (!newsubsec)
    {
        // use player sector 
        if (mo->subsector->sector->ceilingheight - thiscam->height < z)
            z = mo->subsector->sector->ceilingheight - thiscam->height-11*FRACUNIT; // don't be blocked by a opened door
    }
    else
    // camera fit ?
    if (newsubsec->sector->ceilingheight != newsubsec->sector->floorheight && newsubsec->sector->ceilingheight - thiscam->height < z)
        // no fit
        z = newsubsec->sector->ceilingheight - thiscam->height-11*FRACUNIT;
        // is the camera fit is there own sector

	// Make the camera a tad smarter with 3d floors
	if(newsubsec && newsubsec->sector->ffloors)
	{
		ffloor_t* rover;

		for(rover = newsubsec->sector->ffloors; rover; rover = rover->next)
		{
			if(rover->flags & FF_SOLID)
			{
				if(*rover->bottomheight - thiscam->height < z
					&& thiscam->z < *rover->bottomheight)
					z = *rover->bottomheight - thiscam->height-11*FRACUNIT;

				else if(*rover->topheight + thiscam->height > z
					&& thiscam->z > *rover->topheight)
					z = *rover->topheight;

				if(mo->z >= *rover->topheight && thiscam->z < *rover->bottomheight
					|| mo->z < *rover->bottomheight && thiscam->z >= *rover->topheight) // Can't...SEE!
					P_ResetCamera(player, thiscam);
			}
		}
	}

    newsubsec = R_PointInSubsector (thiscam->x,thiscam->y);
    if (newsubsec->sector->ceilingheight - thiscam->height < z)
        z = newsubsec->sector->ceilingheight - thiscam->height-11*FRACUNIT;


    // point viewed by the camera
    // this point is just 64 unit forward the player
    dist = 64 << FRACBITS;
    viewpointx = mo->x + FixedMul( finecosine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
    viewpointy = mo->y + FixedMul( finesine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);

	if((thiscam == &camera && !cv_cam_still.value)
		|| (thiscam == &camera2 && !cv_cam2_still.value)) // Tails 07-02-2001
		thiscam->angle = R_PointToAngle2(thiscam->x,thiscam->y,
											viewpointx,viewpointy);

	if(twodlevel)
		thiscam->angle = angle;

    // follow the player
    if(!player->playerstate == PST_DEAD && (thiscam == &camera ? cv_cam_speed.value : cv_cam2_speed.value) != 0 && (abs(thiscam->x - mo->x) > (thiscam == &camera ? cv_cam_dist.value : cv_cam2_dist.value)*3 || 
       abs(thiscam->y - mo->y) > (thiscam == &camera ? cv_cam_dist.value : cv_cam2_dist.value)*3 || abs(thiscam->z - mo->z) > (thiscam == &camera ? cv_cam_dist.value : cv_cam2_dist.value)*3))
                P_ResetCamera(player, thiscam); // Tails

	if(twodlevel)
	{
		thiscam->momx = x-thiscam->x;
		thiscam->momy = y-thiscam->y;
		thiscam->momz = z-thiscam->z;
	}
	else
	{
		thiscam->momx = FixedMul(x - thiscam->x,(thiscam == &camera ? cv_cam_speed.value : cv_cam2_speed.value));
		thiscam->momy = FixedMul(y - thiscam->y,(thiscam == &camera ? cv_cam_speed.value : cv_cam2_speed.value));

		if(thiscam->subsector->sector->special == 16
			&& thiscam->z < thiscam->subsector->sector->floorheight + 256*FRACUNIT
			&& FixedMul(z - thiscam->z,(thiscam == &camera ? cv_cam_speed.value : cv_cam2_speed.value)) < 0)
			thiscam->momz = 0; // Don't go down a death pit
		else
			thiscam->momz = FixedMul(z - thiscam->z,(thiscam == &camera ? cv_cam_speed.value : cv_cam2_speed.value));
	}

    // compute aming to look the viewed point
    f1=FIXED_TO_FLOAT(viewpointx-thiscam->x);
    f2=FIXED_TO_FLOAT(viewpointy-thiscam->y);
    dist=sqrt(f1*f1+f2*f2)*FRACUNIT;

    angle=R_PointToAngle2(0,thiscam->z, dist
                         ,mo->z+(mo->info->height>>1)+finesine[(player->aiming>>ANGLETOFINESHIFT) & FINEMASK] * 64);

	if(twodlevel || (thiscam == &camera ? cv_cam_still.value == false : cv_cam2_still.value == false)) // Keep the view still... Tails
	{
		G_ClipAimingPitch(&angle);
		dist=thiscam->aiming-angle;
		thiscam->aiming-=(dist>>3);
	}

	// Make player translucent if camera is too close (only in single player).
	if(!(multiplayer || netgame) && !cv_splitscreen.value && P_AproxDistance(thiscam->x - player->mo->x, thiscam->y - player->mo->y) < 48*FRACUNIT)
	{
		player->mo->flags2 |= MF2_SHADOW;
	}
	else
	{
		player->mo->flags2 &= ~MF2_SHADOW;
	}
}

#ifdef CLIENTPREDICTION2

void CL_ResetSpiritPosition(mobj_t *mobj)
{
    P_UnsetThingPosition(mobj->player->spirit);
    mobj->player->spirit->x=mobj->x;
    mobj->player->spirit->y=mobj->y;
    mobj->player->spirit->z=mobj->z;
    mobj->player->spirit->momx=0;
    mobj->player->spirit->momy=0;
    mobj->player->spirit->momz=0;
    mobj->player->spirit->angle=mobj->angle;
    P_SetThingPosition(mobj->player->spirit);
}

void P_ProcessCmdSpirit (player_t* player,ticcmd_t *cmd)
{
    fixed_t   movepushforward=0,movepushside=0;
#ifdef PARANOIA
    if(!player)
        I_Error("P_MoveSpirit : player null");
    if(!player->spirit)
        I_Error("P_MoveSpirit : player->spirit null");
    if(!cmd)
        I_Error("P_MoveSpirit : cmd null");
#endif

    // don't move if dead
    if( player->playerstate != PST_LIVE )
    {
        cmd->angleturn &= ~TICCMD_XY;
        return;
    }
    onground = (player->spirit->z <= player->spirit->floorz) ||
               (player->cheats & CF_FLYAROUND);

    if (player->spirit->reactiontime)
    {
        player->spirit->reactiontime--;
        return;
    }

    player->spirit->angle = cmd->angleturn<<16;
    cmd->angleturn |= TICCMD_XY;
/*
    // now weapon is allways send change is detected at receiver side
    if(cmd->buttons & BT_CHANGE) 
    {
        player->spirit->movedir = cmd->buttons & (BT_WEAPONMASK | BT_EXTRAWEAPON);
        cmd->buttons &=~BT_CHANGE;
    }
    else
    {
        if( player->pendingweapon!=wp_nochange )
            player->spirit->movedir=weapontobutton[player->pendingweapon];
        cmd->buttons&=~(BT_WEAPONMASK | BT_EXTRAWEAPON);
        cmd->buttons|=player->spirit->movedir;
    }
*/
    if (cmd->forwardmove)
    {
        movepushforward = cmd->forwardmove * movefactor;
        
        if (player->spirit->eflags & MF_UNDERWATER)
        {
            // half forward speed when waist under water
            // a little better grip if feets touch the ground
            if (!onground)
                movepushforward >>= 1;
            else
                movepushforward = movepushforward *3/4;
        }
        else
        {
            // allow very small movement while in air for gameplay
            if (!onground)
                movepushforward >>= 3;
        }
        
        P_ThrustSpirit (player->spirit, player->spirit->angle, movepushforward);
    }
    
    if (cmd->sidemove)
    {
        movepushside = cmd->sidemove * movefactor;
        if (player->spirit->eflags & MF_UNDERWATER)
        {
            if (!onground)
                movepushside >>= 1;
            else
                movepushside = movepushside *3/4;
        }
        else 
            if (!onground)
                movepushside >>= 3;
            
        P_ThrustSpirit (player->spirit, player->spirit->angle-ANG90, movepushside);
    }
    
    // mouselook swim when waist underwater
    player->spirit->eflags &= ~MF_SWIMMING;
    if (player->spirit->eflags & MF_UNDERWATER)
    {
        fixed_t a;
        // swim up/down full move when forward full speed
        a = FixedMul( movepushforward*50, finesine[ (cmd->aiming>>(ANGLETOFINESHIFT-16)) ] >>5 );
        
        if ( a != 0 ) {
            player->spirit->eflags |= MF_SWIMMING;
            player->spirit->momz += a;
        }
    }

    //added:22-02-98: jumping
    if (cmd->buttons & BT_JUMP)
    {
        // can't jump while in air, can't jump while jumping
        if (!(player->jumpdown & 2) &&
             (onground || (player->spirit->eflags & MF_UNDERWATER)) )
        {
            if (onground)
                player->spirit->momz = 6*FRACUNIT;
            else //water content
                player->spirit->momz = 3*FRACUNIT/2;

            //TODO: goub gloub when push up in water
            
            if ( !(player->cheats & CF_FLYAROUND) && onground && !(player->spirit->eflags & MF_UNDERWATER))
            {
                S_StartSound(player->spirit, sfx_jump);

                // keep jumping ok if FLY mode.
                player->jumpdown |= 2;
            }
        }
    }
    else
        player->jumpdown &= ~2;

}

void P_MoveSpirit (player_t* p,ticcmd_t *cmd, int realtics)
{
    if( gamestate != GS_LEVEL )
        return;
    if(p->spirit)
    {
        extern boolean supdate;
        int    i;

        p->spirit->flags|=MF_SOLID;
        for(i=0;i<realtics;i++)
        {
            P_ProcessCmdSpirit(p,cmd);
            P_MobjThinker(p->spirit);
        }                 
        p->spirit->flags&=~MF_SOLID;
        P_CalcHeight (p);                 // z-bobing of player
        A_TicWeapon(p, &p->psprites[0]);  // bobing of weapon
        cmd->x=p->spirit->x;
        cmd->y=p->spirit->y;
        supdate=true;
    }
    else
    if(p->mo)
    {
        cmd->x=p->mo->x;
        cmd->y=p->mo->y;
    }
}

#endif

//
// P_PlayerThink
//

boolean playerdeadview; //Fab:25-04-98:show dm rankings while in death view

void P_PlayerThink (player_t* player)
{
    ticcmd_t*           cmd;
//    weapontype_t        newweapon;
//    int                 waterz;

#ifdef PARANOIA
    if(!player->mo) I_Error("p_playerthink : players[%d].mo == NULL",player-players);
#endif

    if (player->bonuscount)
        player->bonuscount--;

    // fixme: do this in the cheat code
    if (player->cheats & CF_NOCLIP)
        player->mo->flags |= MF_NOCLIP;
    else if (!cv_objectplace.value && !player->nightsmode)
        player->mo->flags &= ~MF_NOCLIP;

    // chain saw run forward
    cmd = &player->cmd;

    if (player->mo->flags2 & MF2_JUSTATTACKED)
    {
// added : now angle turn is a absolute value not relative
#ifndef ABSOLUTEANGLE
        cmd->angleturn = 0;
#endif
        cmd->forwardmove = 0xc800/512;
        cmd->sidemove = 0;
        player->mo->flags2 &= ~MF2_JUSTATTACKED;
    }

    if (player->playerstate == PST_REBORN)
#ifdef PARANOIA
        I_Error("player %d is in PST_REBORN\n");
#else
        // it is not "normal" but far to be critical
        return;
#endif

if(cv_gametype.value == GT_RACE || cv_gametype.value == GT_CIRCUIT) // Circuit also! Graue 12-06-2003
{
	int i;

	// Check if all the players in the race have finished. If so, end the level.
	for(i = 0; i < MAXPLAYERS; i++)
	{
		if(playeringame[i])
		{
			if(!players[i].exiting && players[i].lives > 0)
				break;
		}
	}

	if(i == MAXPLAYERS && player->exiting == 3*TICRATE)  // finished
		player->exiting = 2.8*TICRATE + 1;

	// If 10 seconds are left on the timer,
	// begin the drown music for countdown! // Tails 04-25-2001
	if(countdown == 11*TICRATE - 1)
	{
		if(player==&players[consoleplayer])
			{
				S_ChangeMusic(mus_drown, false);
			}
	}

	// If you've hit the countdown and you haven't made
	//  it to the exit, you're a goner! Tails 04-25-2001
	else if(countdown == 1 && !player->exiting)
	{
		player->lives = 1;
		if(player->playerstate == PST_DEAD) // Already dead.. guess he decided not to get back up.
		{
			if(player==&players[consoleplayer])
			{
				S_StopMusic(); // Stop the Music! Tails 03-14-2000
				S_ChangeMusic(mus_gmover, false); // Yousa dead now, Okieday? Tails 03-14-2000
			}
		}
		P_DamageMobj(player->mo, NULL, NULL, 10000);
		player->lives = 0;
	}
}

// Check if player is in the exit sector.
// If so, begin the level end process. Tails 12-15-2000
	if(player->mo->subsector->sector->special == 982 && !player->exiting)
	{
		// Note by Graue 12-13-2003: if you edit this exit stuff, also edit the FOF exit code
		//                           in p_spec.c
		extern int numstarposts;
		int lineindex;
		boolean skipexit = false;

		if(cv_gametype.value == GT_CIRCUIT) // Graue 11-15-2003
		{
			if(player->starpostnum == numstarposts) // Must have touched all the starposts
			{
				extern consvar_t cv_lapcounteridiocy; // Graue 12-25-2003

				player->laps++;
				if(cv_lapcounteridiocy.value)
				{
					if(player->laps == cv_numlaps.value)
						CONS_Printf("%s finished the final lap\n",player_names[player-players]);
					else
						CONS_Printf("%s started lap %d\n",player_names[player-players],player->laps+1);
				}
				else
					CONS_Printf("%s finished lap %d\n",player_names[player-players],player->laps);

				// Reset starposts (checkpoints) info
				player->starpostangle = player->starposttime = player->starpostnum = player->starpostbit = 0;
				player->starpostx = player->starposty = player->starpostz = 0;
			}

			if(player->laps < cv_numlaps.value)
				skipexit = true;
		}

		if(!skipexit) {
			P_DoPlayerExit(player);
			lineindex = P_FindSpecialLineFromTag(71, player->mo->subsector->sector->tag); // Tails 07-20-2003

			if(lineindex != -1) // Custom exit!
			{
				// Special goodies with the PASSUSE flag depending on emeralds collected Graue 12-31-2003
				if(lines[lineindex].flags & ML_PASSUSE)
				{
					if(emeralds & (EMERALD1 | EMERALD2 | EMERALD3 | EMERALD4 | EMERALD5 | EMERALD6 | EMERALD7))
					{
						if(emeralds & EMERALD8) // The secret eighth emerald. Use linedef length.
							nextmapoverride = P_AproxDistance(lines[lineindex].dx, lines[lineindex].dy);
						else // The first seven, not bad. Use front sector's ceiling.
							nextmapoverride = lines[lineindex].frontsector->ceilingheight;
					}
					else // Don't have all eight emeralds, or even the first seven? Use front sector's floor.
						nextmapoverride = lines[lineindex].frontsector->floorheight;
				}
				else {
					nextmapoverride = P_AproxDistance(lines[lineindex].dx, lines[lineindex].dy);
				}

				nextmapoverride >>= FRACBITS; // Graue 12-31-2003

				if(lines[lineindex].flags & ML_NOCLIMB)
					skipstats = true;
			}
		}
	}

	// If it is set, start subtracting Tails 12-15-2000
	if(player->exiting)
	{
		if(cv_gametype.value == GT_CIRCUIT)
		{
			player->exiting--;
		}
		else if(player->exiting < 3*TICRATE)
		{
			player->exiting--;
		}
	}

	if(player->exiting && countdown2)
		player->exiting = 5;

	if(player->exiting == 2 || countdown2 == 2)
	{
		SendNetXCmd(XD_EXITLEVEL,NULL,0); // Let everybody know it's time to exit
		if(cv_gametype.value != GT_RACE) // so an inconsistency doesn't occur!
			leveltime -= 2.8*TICRATE;     // Tails 12-15-2000
		// FIXTHIS: Since the above leveltime setting is broken (bad logic) and will never
		// activate, I'm not touching it for now. Graue 12-06-2003
		// FIXED. Tails 12-14-2003
	}

    if (player->playerstate == PST_DEAD)
    {
		player->mo->flags2 &= ~MF2_SHADOW; // Tails 05-06-2001
        //Fab:25-04-98: show the dm rankings while dead, only in deathmatch
        if (player==&players[displayplayer])
            playerdeadview = true;

        P_DeathThink (player);

        //added:26-02-98:camera may still move when guy is dead
        if (cv_splitscreen.value && player==&players[secondarydisplayplayer] && camera2.chase)
		{
            P_MoveChaseCamera (player,&camera2);
		}
		else if(camera.chase && player==&players[displayplayer])
			P_MoveChaseCamera(player, &camera);

        return;
    }

	if(cv_gametype.value == GT_RACE)
	{
		if(player->lives <= 0)
			player->lives = 3;
	}
	else if(cv_gametype.value == GT_COOP && (netgame || multiplayer))
	{
		// In Co-Op, replenish a user's continues and lives if they are depleted.
		if(player->lives <= 0)
		{
			switch(gameskill)
			{
				case sk_insane:
					player->lives = 1;
					break;
				case sk_nightmare:
				case sk_hard:
				case sk_medium:
					player->lives = 3;
					break;
				case sk_easy:
					player->lives = 5;
					break;
				case sk_baby:
					player->lives = 9;
					break;
				default: // Oops!?
					CONS_Printf("ERROR: GAME SKILL UNDETERMINED!");
					break;
			}
		}

		if(player->continues <= 0)
		{
			switch(gameskill)
			{
				case sk_insane:
					player->continues = 0;
					break;
				case sk_nightmare:
				case sk_hard:
				case sk_medium:
					player->continues = 1;
					break;
				case sk_easy:
					player->continues = 2;
					break;
				case sk_baby:
					player->continues = 5;
					break;
				default: // Oops!?
					CONS_Printf("ERROR: GAME SKILL UNDETERMINED!");
					break;
			}
		}
	}

    if (player==&players[displayplayer])
        playerdeadview = false;

    // check water content, set stuff in mobj
    P_MobjCheckWater (player->mo);

    // check special sectors : damage & secrets
    P_PlayerInSpecialSector (player);

    // Move around.
    // Reactiontime is used to prevent movement
    //  for a bit after a teleport.
    if (player->mo->reactiontime)
        player->mo->reactiontime--;
    else
        P_MovePlayer (player);

    //added:22-02-98: bob view only if looking by the marine's eyes
#ifndef CLIENTPREDICTION2
	if(cv_splitscreen.value && player==&players[secondarydisplayplayer] && !camera2.chase)
	{
		P_CalcHeight (player);
	}
	else if(!camera.chase)
		P_CalcHeight (player);
#endif

    //added:26-02-98: calculate the camera movement
	if(cv_splitscreen.value && player==&players[secondarydisplayplayer] && camera2.chase)
	{
		P_MoveChaseCamera (player, &camera2);
	}
	else if(camera.chase && player==&players[displayplayer])
		P_MoveChaseCamera(player, &camera);

    // check for use
	if(!player->nightsmode)
	{
		if (cmd->buttons & BT_USE)
		{
			if (!player->usedown)
			{
				player->usedown = true;
			}
		}
		else
			player->usedown = false;
	}

    // Counters, time dependent power ups.
	// Start Time Bonus & Ring Bonus count settings Tails 03-10-2000
	player->fscore = player->score; // Tails 03-12-2000

	if(player->splish)
		player->splish--;

	if(player->tagzone)
		player->tagzone--;

	if(player->taglag)
		player->taglag--;

    // Strength counts up to diminish fade.
    if (player->powers[pw_sneakers])
        player->powers[pw_sneakers]--;

    if (player->powers[pw_invulnerability])
        player->powers[pw_invulnerability]--;

    if (player->powers[pw_flashing])
        player->powers[pw_flashing]--;

    if (player->powers[pw_tailsfly]) // tails fly
        player->powers[pw_tailsfly]--; // counter Tails 03-05-2000

    if (player->powers[pw_underwater]) // underwater
        player->powers[pw_underwater]--; // timer Tails 03-06-2000

	if (player->powers[pw_spacetime])
		player->powers[pw_spacetime]--;

    if (player->powers[pw_extralife]) // what's it look like pal?
        player->powers[pw_extralife]--; // duuuuh Tails 03-14-2000

	if (player->powers[pw_homingring])
		player->powers[pw_homingring]--;

	if (player->powers[pw_railring])
		player->powers[pw_railring]--;

	if(player->powers[pw_shieldring])
		player->powers[pw_shieldring]--;

	if(player->powers[pw_automaticring])
		player->powers[pw_automaticring]--;

	if(player->powers[pw_explosionring])
		player->powers[pw_explosionring]--;

	// Tails 12-15-2003
	if(player->powers[pw_superparaloop])
		player->powers[pw_superparaloop]--;

	// Tails 12-15-2003
	if(player->powers[pw_nightshelper])
		player->powers[pw_nightshelper]--;

	if(player->weapondelay)
		player->weapondelay--;

	if(player->homing)
		player->homing--;

	if(player->lightdash)
		player->lightdash--;

	// Flash player after being hit. Tails 03-07-2000
	if(!player->nightsmode)
	{
		if(player->powers[pw_flashing] && (leveltime & 1))
			player->mo->flags2 |= MF2_DONTDRAW;
		else if(!cv_objectplace.value)
		{
			if(!(player->mo->tracer && player->mo->tracer->type == MT_SUPERTRANS && player->mo->tracer->health > 0))
				player->mo->flags2 &= ~MF2_DONTDRAW;
		}
	}
	else
	{
		if(player->powers[pw_flashing] & 1)
			player->mo->tracer->flags2 |= MF2_DONTDRAW;
		else
			player->mo->tracer->flags2 &= ~MF2_DONTDRAW;
	}
}