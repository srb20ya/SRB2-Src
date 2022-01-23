// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_mobj.c,v 1.30 2001/08/12 15:21:04 bpereira Exp $
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
// MERCHANTABILITFY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: p_mobj.c,v $
// Revision 1.30  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.29  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.28  2001/07/16 22:35:41  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.27  2001/04/02 18:54:32  bpereira
// no message
//
// Revision 1.26  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.25  2001/03/30 17:12:50  bpereira
// no message
//
// Revision 1.24  2001/03/13 22:14:19  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.23  2001/03/03 06:17:33  bpereira
// no message
//
// Revision 1.22  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.21  2001/02/10 12:27:14  bpereira
// no message
//
// Revision 1.20  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.19  2000/11/04 16:23:43  bpereira
// no message
//
// Revision 1.18  2000/11/02 19:49:36  bpereira
// no message
//
// Revision 1.17  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.16  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.15  2000/10/16 20:02:30  bpereira
// no message
//
// Revision 1.14  2000/10/01 10:18:18  bpereira
// no message
//
// Revision 1.13  2000/09/28 20:57:16  bpereira
// no message
//
// Revision 1.12  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.11  2000/08/11 19:10:13  metzgermeister
// *** empty log message ***
//
// Revision 1.10  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.9  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.8  2000/04/08 17:29:25  stroggonmeth
// no message
//
// Revision 1.7  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.6  2000/04/05 15:47:46  stroggonmeth
// Added hack for Dehacked lumps. Transparent sprites are now affected by colormaps.
//
// Revision 1.5  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.4  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      Moving object handling. Spawn functions.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "g_input.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "p_local.h"
#include "p_inter.h"
#include "p_setup.h"    //levelflats to test if mobj in water sector
#include "r_main.h"
#include "r_things.h"
#include "r_sky.h"
#include "s_sound.h"
#include "z_zone.h"
#include "m_random.h"
#include "d_clisrv.h"
#include "r_splats.h"   //faB: in dev.

void P_ResetScore(player_t* player);

void P_InstaThrust(mobj_t* mo, angle_t angle, fixed_t move);

// protos.
CV_PossibleValue_t viewheight_cons_t[]={{16,"MIN"},{56,"MAX"},{0,NULL}};

consvar_t cv_viewheight = {"viewheight", VIEWHEIGHTS,0,viewheight_cons_t,NULL};

//Fab:26-07-98:
consvar_t cv_splats  = {"splats","1",CV_SAVE,CV_OnOff};

extern int numstarposts; // Graue 11-17-2003
extern unsigned bitstarposts; // Graue 11-18-2003

static const fixed_t FloatBobOffsets[64] =
{
        0, 51389, 102283, 152192,
        200636, 247147, 291278, 332604,
        370727, 405280, 435929, 462380,
        484378, 501712, 514213, 521763,
        524287, 521763, 514213, 501712,
        484378, 462380, 435929, 405280,
        370727, 332604, 291278, 247147,
        200636, 152192, 102283, 51389,
        -1, -51390, -102284, -152193,
        -200637, -247148, -291279, -332605,
        -370728, -405281, -435930, -462381,
        -484380, -501713, -514215, -521764,
        -524288, -521764, -514214, -501713,
        -484379, -462381, -435930, -405280,
        -370728, -332605, -291279, -247148,
        -200637, -152193, -102284, -51389
};

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
//SoM: 4/7/2000: Boom code...
boolean P_SetMobjState ( mobj_t*       mobj,
                         statenum_t    state )
{
    state_t*    st;
    
    //remember states seen, to detect cycles:
    
    static statenum_t seenstate_tab[NUMSTATES]; // fast transition table
    statenum_t *seenstate = seenstate_tab;      // pointer to table
    static int recursion;                       // detects recursion
    statenum_t i = state;                       // initial state
    boolean ret = true;                         // return value
    statenum_t tempstate[NUMSTATES];            // for use with recursion
    
    if (recursion++)                            // if recursion detected,
        memset(seenstate=tempstate,0,sizeof tempstate); // clear state table
    
    do
    {
        if (state == S_NULL)
        {
            mobj->state = (state_t *) S_NULL;
            P_RemoveMobj (mobj);
            ret = false;
            break;                 // killough 4/9/98
        }
        
        st = &states[state];
        mobj->state = st;
        mobj->tics = st->tics;
        mobj->sprite = st->sprite;
        mobj->frame = st->frame;
        
        // Modified handling.
        // Call action functions when the state is set
        
        if (st->action.acp1)
            st->action.acp1(mobj);
        
        seenstate[state] = 1 + st->nextstate;   // killough 4/9/98
        
        state = st->nextstate;
    } while (!mobj->tics && !seenstate[state]);   // killough 4/9/98
    
    if (ret && !mobj->tics)  // killough 4/9/98: detect state cycles
        CONS_Printf("Warning: State Cycle Detected");
    
    if (!--recursion)
        for (;(state=seenstate[i]);i=state-1)
            seenstate[i] = 0;  // killough 4/9/98: erase memory of states
        
    return ret;
}

//----------------------------------------------------------------------------
//
// FUNC P_SetMobjStateNF
//
// Same as P_SetMobjState, but does not call the state function.
//
//----------------------------------------------------------------------------

boolean P_SetMobjStateNF(mobj_t *mobj, statenum_t state)
{
    state_t *st;
    
    if(state == S_NULL)
    { // Remove mobj
        P_RemoveMobj(mobj);
        return(false);
    }
    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    return(true);
}

void P_RemovePrecipMobj(precipmobj_t* mobj);

// Tails 08-25-2002
boolean P_SetPrecipMobjState(precipmobj_t* mobj, statenum_t state)
{
    state_t *st;
    
    if(state == S_NULL)
    { // Remove mobj
        P_RemovePrecipMobj(mobj);
        return(false);
    }
    st = &states[state];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    return(true);
}

//
// P_ExplodeMissile
//
void P_ExplodeMissile (mobj_t* mo)
{
	mobj_t*	explodemo; // Tails
    mo->momx = mo->momy = mo->momz = 0;

    P_SetMobjState (mo, mobjinfo[mo->type].deathstate);

	if(mo->type == MT_DETON)
	{
		explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_EXPLODE);
		explodemo->momx += (P_Random() % 32) * FRACUNIT/8;
		explodemo->momy += (P_Random() % 32) * FRACUNIT/8;
		S_StartSound(explodemo, sfx_pop);
		explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_EXPLODE);
		explodemo->momx += (P_Random() % 64) * FRACUNIT/8;
		explodemo->momy -= (P_Random() % 64) * FRACUNIT/8;
		S_StartSound(explodemo, sfx_dmpain);
		explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_EXPLODE);
		explodemo->momx -= (P_Random() % 128) * FRACUNIT/8;
		explodemo->momy += (P_Random() % 128) * FRACUNIT/8;
		S_StartSound(explodemo, sfx_pop);
		explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_EXPLODE);
		explodemo->momx -= (P_Random() % 96) * FRACUNIT/8;
		explodemo->momy -= (P_Random() % 96) * FRACUNIT/8;
		S_StartSound(explodemo, sfx_cybdth);
	}

    mo->tics -= P_Random()&3;

    if (mo->tics < 1)
        mo->tics = 1;

    mo->flags &= ~MF_MISSILE;

    if (mo->info->deathsound)
        S_StartSound (mo, mo->info->deathsound);
}



//----------------------------------------------------------------------------
//
// PROC P_FloorBounceMissile
//
//----------------------------------------------------------------------------

void P_FloorBounceMissile(mobj_t *mo)
{
        mo->momz = -mo->momz;
        P_SetMobjState(mo, mobjinfo[mo->type].deathstate);
}

//----------------------------------------------------------------------------
//
// PROC P_ThrustMobj
//
//----------------------------------------------------------------------------

void P_ThrustMobj(mobj_t *mo, angle_t angle, fixed_t move)
{
        angle >>= ANGLETOFINESHIFT;
        mo->momx += FixedMul(move, finecosine[angle]);
        mo->momy += FixedMul(move, finesine[angle]);
}

// P_InsideANonSolidFFloor
//
// Returns TRUE if mobj is inside a non-solid 3d floor.
// Tails 10-01-2003
boolean P_InsideANonSolidFFloor (mobj_t* mobj, ffloor_t* rover)
{
	if((rover->flags & FF_SOLID))
		return false;

	if(mobj->z > *rover->topheight)
		return false;

	if(mobj->z + mobj->height < *rover->bottomheight)
		return false;

	return true;
}

//
// P_XYMovement
//
#define STOPSPEED               (0xffff/NEWTICRATERATIO)
#define FRICTION                0xe800/NEWTICRATERATIO   //0.90625
#define FRICTION_LOW            0xf900/NEWTICRATERATIO
#define FRICTION_FLY            0xeb00/NEWTICRATERATIO

//added:22-02-98: adds friction on the xy plane
void P_XYFriction (mobj_t* mo, fixed_t oldx, fixed_t oldy, boolean oldfriction)
{
    //valid only if player avatar
    player_t*   player = mo->player;

	if(player)
	{
		if (player->rmomx > -STOPSPEED
			&& player->rmomx < STOPSPEED
			&& player->rmomy > -STOPSPEED
			&& player->rmomy < STOPSPEED
			&& (player->cmd.forwardmove == 0
					&& player->cmd.sidemove == 0 && !player->mfspinning))
			{
				// if in a walking frame, stop moving
				if ( (player && player->walking==1) && (mo->type!=MT_SPIRIT)) // use my new walking variable! Tails 10-30-2000
				   {
		/*          if(player->powers[pw_super])
				   {
					P_SetMobjState (player->mo, S_PLAY_SPC1); // Super Sonic Stuff Tails 04-18-2000

				   }
				   else*/
					P_SetMobjState (player->mo, S_PLAY_STND);
								}
				mo->momx = player->cmomx;
				mo->momy = player->cmomy;
			}
		else
		{
			if(oldfriction)
			{
				mo->momx = FixedMul (mo->momx, FRICTION);
				mo->momy = FixedMul (mo->momy, FRICTION);
			}
			else
			{
				//SoM: 3/28/2000: Use boom friction.
				if ((oldx == mo->x) && (oldy == mo->y)) // Did you go anywhere?
				{
					mo->momx = FixedMul(mo->momx,ORIG_FRICTION);
					mo->momy = FixedMul(mo->momy,ORIG_FRICTION);
				}
				else
				{
					mo->momx = FixedMul(mo->momx,mo->friction);
					mo->momy = FixedMul(mo->momy,mo->friction);
				}
				mo->friction = ORIG_FRICTION;
			}
		}
	}
	else
    {
		if (mo->momx > -STOPSPEED
        && mo->momx < STOPSPEED
        && mo->momy > -STOPSPEED
        && mo->momy < STOPSPEED)
		{
			mo->momx = 0;
			mo->momy = 0;
		}
		else
		{
			if(oldfriction)
			{
				mo->momx = FixedMul (mo->momx, FRICTION);
				mo->momy = FixedMul (mo->momy, FRICTION);
			}
			else
			{
				//SoM: 3/28/2000: Use boom friction.
				if ((oldx == mo->x) && (oldy == mo->y)) // Did you go anywhere?
				{
					mo->momx = FixedMul(mo->momx,ORIG_FRICTION);
					mo->momy = FixedMul(mo->momy,ORIG_FRICTION);
				}
				else
				{
					mo->momx = FixedMul(mo->momx,mo->friction);
					mo->momy = FixedMul(mo->momy,mo->friction);
				}
				mo->friction = ORIG_FRICTION;
			}
		}
	}
}

//added:22-02-98: adds friction on the xy plane
void P_SceneryXYFriction (mobj_t* mo, fixed_t oldx, fixed_t oldy)
{
	if (mo->momx > -STOPSPEED
       && mo->momx < STOPSPEED
       && mo->momy > -STOPSPEED
       && mo->momy < STOPSPEED)
	{
		mo->momx = 0;
		mo->momy = 0;
	}
	else
	{
		//SoM: 3/28/2000: Use boom friction.
		if ((oldx == mo->x) && (oldy == mo->y)) // Did you go anywhere?
		{
			mo->momx = FixedMul(mo->momx,ORIG_FRICTION);
			mo->momy = FixedMul(mo->momy,ORIG_FRICTION);
		}
		else
		{
			mo->momx = FixedMul(mo->momx,mo->friction);
			mo->momy = FixedMul(mo->momy,mo->friction);
		}
		mo->friction = ORIG_FRICTION;
	}
}

void P_XYMovement (mobj_t* mo)
{
    fixed_t     ptryx;
    fixed_t     ptryy;
    player_t*   player;
    fixed_t     xmove;
    fixed_t     ymove;
    fixed_t     oldx, oldy; //reducing bobbing/momentum on ice
                            //when up against walls
    static int windTab[3] = {2048*5, 2048*10, 2048*25};
	boolean moved;

	moved = true;

	if((mo->type == MT_REDFLAG || mo->type == MT_BLUEFLAG
		|| (mo->flags & MF_PUSHABLE))
		&& (mo->subsector->sector->special == 16
		|| mo->subsector->sector->special == 5
		|| mo->subsector->sector->special == 7
		|| mo->subsector->sector->special == 4
		|| mo->subsector->sector->special == 11)
		&& mo->z == mo->floorz) // Remove CTF flag if in death pit
		mo->fuse = 1; // Tails 08-02-2001

    //added:18-02-98: if it's stopped
    if (!mo->momx && !mo->momy)
    {
        if (mo->flags2 & MF2_SKULLFLY)
        {
            // the skull slammed into something
            mo->flags2 &= ~MF2_SKULLFLY;
            mo->momx = mo->momy = mo->momz = 0;

            //added:18-02-98: comment: set in 'search new direction' state?
			if(mo->type != MT_EGGMOBILE)
				P_SetMobjState (mo, mo->info->spawnstate);

			return;
		}
		if(mo->flags2&MF2_WINDTHRUST)
		{
			int special = mo->subsector->sector->special;
			switch(special)
			{
			case 40: case 41: case 42: // Wind_East
				P_ThrustMobj(mo, 0, windTab[special-40]);
				break;
			case 43: case 44: case 45: // Wind_North
				P_ThrustMobj(mo, ANG90, windTab[special-43]);
				break;
			case 46: case 47: case 48: // Wind_South
				P_ThrustMobj(mo, ANG270, windTab[special-46]);
				break;
			case 49: case 50: case 51: // Wind_West
				P_ThrustMobj(mo, ANG180, windTab[special-49]);
				break;
			}
		}
	}

    player = mo->player;        //valid only if player avatar

    if (mo->momx > MAXMOVE)
        mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE)
        mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
        mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE)
        mo->momy = -MAXMOVE;

    xmove = mo->momx;
    ymove = mo->momy;

    oldx = mo->x;
    oldy = mo->y;

    do
    {
        if (xmove > MAXMOVE/2 || ymove > MAXMOVE/2)
        {
            ptryx = mo->x + xmove/2;
            ptryy = mo->y + ymove/2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }

        if (!P_TryMove (mo, ptryx, ptryy, true)) //SoM: 4/10/2000
        {
            // blocked move

            // gameplay issue : let the marine move forward while trying
            //                  to jump over a small wall
            //    (normally it can not 'walk' while in air)
            // BP:1.28 no more use Cf_JUMPOVER, but i leave it for backward lmps compatibility
            if (mo->player)
            {
				moved = false;

                if (tmfloorz - mo->z > MAXSTEPMOVE)
                {
                    if (mo->momz > 0)
                        mo->player->cheats |= CF_JUMPOVER;
                    else
                        mo->player->cheats &= ~CF_JUMPOVER;
				}
			}

			if(mo->flags & MF_BOUNCE) // Graue 12-31-2003
			{
				P_BounceMove(mo);
				xmove = ymove = 0;
			}
			else if ((mo->player && !twodlevel)
				|| (mo->flags & MF_SLIDEME)
				|| (mo->flags & MF_PUSHABLE)) // Tails 08-18-2001
			{   // try to slide along it
				P_SlideMove (mo);
				xmove = ymove = 0; // Graue 12-31-2003
			}
			else if (mo->flags & MF_MISSILE)
			{
				// explode a missile
				if (ceilingline &&
					ceilingline->backsector &&
					ceilingline->backsector->ceilingpic == skyflatnum &&
					ceilingline->frontsector &&
					ceilingline->frontsector->ceilingpic == skyflatnum &&
					mo->subsector->sector->ceilingheight == mo->ceilingz)
				{
					if (!boomsupport ||
						mo->z > ceilingline->backsector->ceilingheight)//SoM: 4/7/2000: DEMO'S
					{
						// Hack to prevent missiles exploding
						// against the sky.
						// Does not handle sky floors.
						//SoM: 4/3/2000: Check frontsector as well..

						P_SetMobjState(mo, S_DISS);
						//P_RemoveMobj (mo);
						return;
					}
				}

				// draw damage on wall
				//SPLAT TEST ----------------------------------------------------------
	#ifdef WALLSPLATS
				if (blockingline && mo->type != MT_REDRING && mo->type != MT_LASER
					&& !(mo->flags2 & MF2_AUTOMATIC) && !(mo->flags2 & MF2_RAILRING)
					&& !(mo->flags2 & MF2_HOMING) && !(mo->flags2 & MF2_EXPLOSION))   //set by last P_TryMove() that failed
				{ // Don't have redrings make splats Tails 03-04-2002
					divline_t   divl;
					divline_t   misl;
					fixed_t     frac;

					P_MakeDivline (blockingline, &divl);
					misl.x = mo->x;
					misl.y = mo->y;
					misl.dx = mo->momx;
					misl.dy = mo->momy;
					frac = P_InterceptVector (&divl, &misl);
					R_AddWallSplat (blockingline, P_PointOnLineSide(mo->x,mo->y,blockingline)
								   ,"A_DMG3", mo->z, frac, SPLATDRAWMODE_SHADE);
				}
	#endif
				// --------------------------------------------------------- SPLAT TEST

				P_ExplodeMissile (mo);
			}
			else if (mo->type == MT_FIREBALL)
			{
				// explode a missile
				if (ceilingline &&
					ceilingline->backsector &&
					ceilingline->backsector->ceilingpic == skyflatnum &&
					ceilingline->frontsector &&
					ceilingline->frontsector->ceilingpic == skyflatnum &&
					mo->subsector->sector->ceilingheight == mo->ceilingz)
				{
					if (!boomsupport ||
						mo->z > ceilingline->backsector->ceilingheight)//SoM: 4/7/2000: DEMO'S
					{
						// Hack to prevent missiles exploding
						// against the sky.
						// Does not handle sky floors.
						//SoM: 4/3/2000: Check frontsector as well..

						P_SetMobjState (mo, S_DISS);
						return;
					}
				}

				S_StartSound(mo, sfx_tink);

				P_ExplodeMissile (mo);
			}
			else
				mo->momx = mo->momy = 0;
		}
		else
		{
			// hack for playability : walk in-air to jump over a small wall
			if (mo->player)
			{
				mo->player->cheats &= ~CF_JUMPOVER;
				moved = true;
			}
		}

	} while (xmove || ymove);

	if(mo->player && moved == false && mo->player->nightsmode && mo->target)
	{
		int radius;
		const double deg2rad = 0.017453293;
		P_UnsetThingPosition(mo);
		mo->player->angle_pos = mo->player->old_angle_pos;
		mo->player->speed /= 5;
		mo->player->speed *= 4;
		player->flyangle += 180;
		player->flyangle %= 360;

		radius = mo->target->info->radius;

		mo->x = mo->target->x + cos(mo->player->old_angle_pos * deg2rad) * radius;
		mo->y = mo->target->y + sin(mo->player->old_angle_pos * deg2rad) * radius;

		mo->momx = mo->momy = 0;
		P_SetThingPosition(mo);
	}

	// Tails 09-12-2002
	if(mo->type == MT_FIREBALL || mo->type == MT_SHELL)
		return;

    if ((((mo->flags & MF_MISSILE) || (mo->flags2 & MF2_SKULLFLY))  || mo->type == MT_SNOWBALL) && !mo->type == MT_DETON) // Tails 12-12-2001
        return;         // no friction for missiles ever

	if(mo->flags & MF_MISSILE)
		return;

	if(mo->player) // No Friction
		if(mo->player->homing) // For Homing
			return; // Tails 09-02-2001

    if (mo->z > mo->floorz && !(mo->flags2&MF2_ONMOBJ))
        return;         // no friction when airborne

	// start spinning friction Tails 02-28-2000
	if(player)
	{
		if (player->mfspinning == 1 && (player->rmomx || player->rmomy) && !player->mfstartdash)
		{
			mo->momx = FixedMul (mo->momx, FRICTION*1.098);
			mo->momy = FixedMul (mo->momy, FRICTION*1.098);
			return;
		}
	}
	// end spinning friction Tails 02-28-2000
	// Ice on a ledge Tails 11-29-2000
	/*
	if (mo->subsector->sector->ffloors)
	{
		ffloor_t* rover;
	    for(rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
		{
			if(mo->z == *rover->topheight && !mo->momz && *rover->master->frontsector->special == 256)
			{
				mo->momx = FixedMul (mo->momx, FRICTION*1.1);
				mo->momy = FixedMul (mo->momy, FRICTION*1.1);
				return;
			}
		}
	}*/
    if (mo->z > mo->floorz && mo->type != MT_CRAWLACOMMANDER && mo->type != MT_EGGMOBILE && mo->type != MT_EGGMOBILE2)
		return;         // no friction when airborne

    P_XYFriction (mo, oldx, oldy, false);
}

boolean P_SceneryTryMove(mobj_t* thing, fixed_t x, fixed_t y);

void P_RingXYMovement (mobj_t* mo)
{
    fixed_t     ptryx;
    fixed_t     ptryy;
    fixed_t     xmove;
    fixed_t     ymove;
    fixed_t     oldx, oldy; //reducing bobbing/momentum on ice
                            //when up against walls

    if (mo->momx > MAXMOVE)
        mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE)
        mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
        mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE)
        mo->momy = -MAXMOVE;

    xmove = mo->momx;
    ymove = mo->momy;

    oldx = mo->x;
    oldy = mo->y;

    do
    {
        if (xmove > MAXMOVE/2 || ymove > MAXMOVE/2)
        {
            ptryx = mo->x + xmove/2;
            ptryy = mo->y + ymove/2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }

        if (!P_SceneryTryMove (mo, ptryx, ptryy)) //SoM: 4/10/2000
			P_SlideMove (mo);

	} while (xmove || ymove);

    if (mo->z > mo->floorz)
		return;         // no friction when airborne
}

void P_SceneryXYMovement (mobj_t* mo)
{
    fixed_t     ptryx;
    fixed_t     ptryy;
    fixed_t     xmove;
    fixed_t     ymove;
    fixed_t     oldx, oldy; //reducing bobbing/momentum on ice
                            //when up against walls

    if (mo->momx > MAXMOVE)
        mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE)
        mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
        mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE)
        mo->momy = -MAXMOVE;

    xmove = mo->momx;
    ymove = mo->momy;

    oldx = mo->x;
    oldy = mo->y;

    do
    {
        if (xmove > MAXMOVE/2 || ymove > MAXMOVE/2)
        {
            ptryx = mo->x + xmove/2;
            ptryy = mo->y + ymove/2;
            xmove >>= 1;
            ymove >>= 1;
        }
        else
        {
            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;
        }

        if (!P_SceneryTryMove (mo, ptryx, ptryy)) //SoM: 4/10/2000
        {
			// blocked move
			mo->momx = mo->momy = 0;
		}

	} while (xmove || ymove);

    if (mo->z > mo->floorz && !(mo->flags2&MF2_ONMOBJ))
        return;         // no friction when airborne

    if (mo->z > mo->floorz)
		return;         // no friction when airborne

    P_SceneryXYFriction (mo, oldx, oldy);
}

void P_RingZMovement (mobj_t* mo)
{
#ifdef FIXROVERBUGS
// Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
    if(mo->subsector->sector->ffloors)
    {
      ffloor_t*  rover;
      fixed_t    delta1;
      fixed_t    delta2;
      int        thingtop = mo->z + mo->height;

      for(rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
      {
		if(!(rover->flags & FF_EXISTS))
			continue;

        if((!(rover->flags & FF_SOLID || rover->flags & FF_QUICKSAND) || (rover->flags & FF_SWIMMABLE)))
			continue;

		if(rover->flags & FF_QUICKSAND)
		{
			if(mo->z < *rover->topheight && *rover->bottomheight < thingtop)
			{
				mo->floorz = mo->z;
				continue;
			}
		}

        delta1 = mo->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > mo->floorz && abs(delta1) < abs(delta2))
          mo->floorz = *rover->topheight;
        if(*rover->bottomheight < mo->ceilingz && abs(delta1) >= abs(delta2))
		{
			if(!(rover->flags & FF_PLATFORM))
				mo->ceilingz = *rover->bottomheight;
		}
      }
    }
#endif

    // adjust height
    mo->z += mo->momz;

	if(!mo->momx && !mo->momy)
		return;

    // clip movement
    if (mo->z <= mo->floorz)
    {
        mo->z = mo->floorz;
        mo->momz = 0;
    }
    else if (mo->z + mo->height > mo->ceilingz)
    {
		mo->momz = 0;
        mo->z = mo->ceilingz - mo->height;
    }
}

//
// P_ZMovement
//
void P_ZMovement (mobj_t* mo)
{
    fixed_t     dist;
    fixed_t     delta;

#ifdef FIXROVERBUGS
// Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
    if(mo->subsector->sector->ffloors)
    {
      ffloor_t*  rover;
      fixed_t    delta1;
      fixed_t    delta2;
      int        thingtop = mo->z + mo->height;

      for(rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
      {
		if(!(rover->flags & FF_EXISTS))
			continue;

        if((!(rover->flags & FF_SOLID || rover->flags & FF_QUICKSAND) || (rover->flags & FF_SWIMMABLE)))
			continue;

		if(rover->flags & FF_QUICKSAND)
		{
			if(mo->z < *rover->topheight && *rover->bottomheight < thingtop)
			{
				mo->floorz = mo->z;
				continue;
			}
		}

        delta1 = mo->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > mo->floorz && abs(delta1) < abs(delta2))
          mo->floorz = *rover->topheight;
        if(*rover->bottomheight < mo->ceilingz && abs(delta1) >= abs(delta2))
		{
			if(!(rover->flags & FF_PLATFORM))
				mo->ceilingz = *rover->bottomheight;
		}
      }
    }
#endif

    // adjust height
    mo->z += mo->momz;

	// Tails 12-18-2003
	if((mo->flags & MF_PUSHABLE) && mo->momz != 0)
	{
		fixed_t middleheight;
		boolean wasinwater;
		wasinwater = P_MobjCheckOldPosWater(mo);
		middleheight = mo->z + (mo->info->height>>1);
		if(((middleheight > mo->watertop && middleheight - mo->momz < mo->watertop)
			|| (middleheight < mo->watertop	&& middleheight - mo->momz > mo->watertop && wasinwater == false)))
		{
			int bubblecount;
			int i;

			P_SpawnMobj(mo->x, mo->y, mo->watertop, MT_SPLISH); // Spawn a splash
			S_StartSound(mo, sfx_splish); // And make a sound!

			bubblecount = abs(mo->momz)>>FRACBITS;
			// Create tons of bubbles
			for(i=0; i<bubblecount; i++)
			{
				if(P_Random() < 32)
					P_SpawnMobj(mo->x + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), mo->y + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), mo->z + (P_Random()<<FRACBITS)/4, MT_MEDIUMBUBBLE)->momz = mo->momz < 0 ? mo->momz/16 : 0;
				else
					P_SpawnMobj(mo->x + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), mo->y + (P_Random()<<FRACBITS)/8 * (P_Random()&1 ? 1 : -1), mo->z + (P_Random()<<FRACBITS)/4, MT_SMALLBUBBLE)->momz = mo->momz < 0 ? mo->momz/16 : 0;
			}
		}
	}

	switch(mo->type)
	{/*
		case MT_BLUEORB:
		case MT_GREENORB:
		case MT_BLACKORB:
		case MT_YELLOWORB:
		case MT_GTHOK:
		case MT_GRTHOK:
		case MT_PCTHOK:
		case MT_DRTHOK:
		case MT_STHOK:
		case MT_OTHOK:
		case MT_RTHOK:
		case MT_BTHOK:
		case MT_PTHOK:
		case MT_DBTHOK:
		case MT_BGTHOK:
			return; // Ignore these items Tails*/
		case MT_GOOP:
			if(mo->z <= mo->floorz && mo->momz)
			{
				P_SetMobjState(mo, S_GOOP3);
				mo->momx = mo->momy = 0;
				S_StartSound(mo, mo->info->painsound);
			}
			break;
		case MT_SMALLBUBBLE:
			if(mo->z <= mo->floorz) // Hit the floor, so POP!
			{
				int random;

				P_SetMobjState(mo, S_DISS);

				random = P_Random();

				if(mo->threshold == 42) // Don't make pop sound.
					break;

				if(random <= 51)
					S_StartSound(mo, sfx_bubbl1);
				else if (random <= 102)
					S_StartSound(mo, sfx_bubbl2);
				else if (random <= 153)
					S_StartSound(mo, sfx_bubbl3);
				else if (random <= 204)
					S_StartSound(mo, sfx_bubbl4);
				else
					S_StartSound(mo, sfx_bubbl5);
			}
			break;
		case MT_MEDIUMBUBBLE:
			if(mo->z <= mo->floorz) // Hit the floor, so split!
			{
				// split
				mobj_t* explodemo;

				explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_SMALLBUBBLE);
				explodemo->momx += (P_Random() % 32) * FRACUNIT/8;
				explodemo->momy += (P_Random() % 32) * FRACUNIT/8;
				explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_SMALLBUBBLE);
				explodemo->momx += (P_Random() % 64) * FRACUNIT/8;
				explodemo->momy -= (P_Random() % 64) * FRACUNIT/8;
				explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_SMALLBUBBLE);
				explodemo->momx -= (P_Random() % 128) * FRACUNIT/8;
				explodemo->momy += (P_Random() % 128) * FRACUNIT/8;
				explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_SMALLBUBBLE);
				explodemo->momx -= (P_Random() % 96) * FRACUNIT/8;
				explodemo->momy -= (P_Random() % 96) * FRACUNIT/8;
			}
			break;
		case MT_RING: // Ignore still rings Tails 09-02-2001
		case MT_COIN:
		case MT_FLINGCOIN:
		case MT_FLINGRING:
		case MT_HOMINGRING:
		case MT_AUTOMATICRING:
		case MT_SHIELDRING:
		case MT_RAILRING:
		case MT_EXPLOSIONRING:
		case MT_NIGHTSWING:
			if(!(mo->momx || mo->momy || mo->momz))
				return;
			break;
		default:
			break;
	}

    if ( mo->flags & MF_FLOAT
			&& mo->target && mo->health && !(mo->type == MT_DETON || mo->type == MT_JETTBOMBER || mo->type == MT_JETTGUNNER || mo->type == MT_CRAWLACOMMANDER || mo->type == MT_EGGMOBILE2)) // Tails 07-21-2001
    {
        // float down towards target if too close
        if ( !(mo->flags2 & MF2_SKULLFLY)
             && !(mo->flags & MF_INFLOAT) )
        {
            dist = P_AproxDistance (mo->x - mo->target->x,
                                    mo->y - mo->target->y);

            delta =(mo->target->z + (mo->height>>1)) - mo->z;

            if (delta<0 && dist < -(delta*3) )
                mo->z -= FLOATSPEED;
            else if (delta>0 && dist < (delta*3) )
                mo->z += FLOATSPEED;
        }

    }

    // clip movement

    if (mo->z <= mo->floorz)
    {
        // hit the floor
        if(mo->flags&MF_MISSILE)
        {
            mo->z = mo->floorz;
            if( (mo->flags& MF_NOCLIP)==0 )
            {
				P_ExplodeMissile(mo);
				return;
            }
        }
		else if(mo->type == MT_FIREBALL)
		{
			mo->momz = 5*FRACUNIT;
		}

        mo->z = mo->floorz;

        // Note (id):
        //  somebody left this after the setting momz to 0,
        //  kinda useless there.
        if (mo->flags2 & MF2_SKULLFLY)
        {
            // the skull slammed into something
            mo->momz = -mo->momz;
        }

		// Mine explodes upon ground contact Tails 06-13-2000
		if((mo->type==MT_MINE) && (mo->z <= mo->floorz) && !(mo->state == &states[S_MINE_BOOM1]
		   || mo->state == &states[S_MINE_BOOM2] || mo->state == &states[S_MINE_BOOM3]
		   || mo->state == &states[S_MINE_BOOM4] || mo->state == &states[S_DISS]))
		{
			P_ExplodeMissile(mo);
		}

        if (mo->momz < 0) // falling
        {
			if(mo->type == MT_SEED)
			{
				byte random;
				random = P_Random();
				if(random < 64)
					P_SpawnMobj(mo->x, mo->y, mo->floorz, MT_GFZFLOWER3);
				else if(random < 192)
					P_SpawnMobj(mo->x, mo->y, mo->floorz, MT_GFZFLOWER1);
				else
					P_SpawnMobj(mo->x, mo->y, mo->floorz, MT_GFZFLOWER2);

//				P_RemoveMobj(mo);
				P_SetMobjState(mo, S_DISS);
				return;
			}
            // set it once and not continuously
// Tails
			if(tmfloorthing)
			{
				// Bouncing boxes Tails 09-28-2001
				if(tmfloorthing->z > tmfloorthing->floorz)
				{
					if((tmfloorthing->flags & MF_MONITOR) || (tmfloorthing->flags & MF_PUSHABLE))
						mo->momz = 4*FRACUNIT;
				}
			}
            if ((mo->z <= mo->floorz) && (!(tmfloorthing) || (((tmfloorthing->flags & MF_PUSHABLE) || (tmfloorthing->flags2 & MF2_STANDONME)) || tmfloorthing->type == MT_PLAYER || tmfloorthing->type == MT_FLOORSPIKE))) // Tails 9-15-99 Spin Attack
			{
				if(tmfloorthing && mo->momz)
					mo->eflags |= MF_JUSTHITFLOOR;
				else if(!tmfloorthing)
					mo->eflags |= MF_JUSTHITFLOOR; // Tails 9-15-99 Spin Attack
			}
// end Tails

            //SOM: Flingrings bounce
            if(mo->type == MT_FLINGRING
				|| mo->type == MT_FLINGCOIN
				|| mo->type == MT_HOMINGRING
				|| mo->type == MT_AUTOMATICRING
				|| mo->type == MT_SHIELDRING
				|| mo->type == MT_RAILRING
				|| mo->type == MT_EXPLOSIONRING)
			{
				if(mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS)
					mo->momz = -mo->momz * 0.1;
				else
					mo->momz = -mo->momz * 0.85;
			}
            else if (!(tmfloorthing) || (((tmfloorthing->flags & MF_PUSHABLE) || (tmfloorthing->flags2 & MF2_STANDONME)) || tmfloorthing->type == MT_PLAYER || tmfloorthing->type == MT_FLOORSPIKE))
			{
              mo->momz = 0;
			}
        }

	if(mo->type == MT_STEAM) // Tails 05-29-2001
		return; // Tails 05-29-2001

        mo->z = mo->floorz;
    }
    else if(mo->flags2&MF2_LOGRAV)
    {
		fixed_t gravityadd=0;

        if(mo->momz == 0)
            mo->momz = -(gravityadd>>3)*2;
        else
            mo->momz -= gravityadd>>3;
    }
    else if (! (mo->flags & MF_NOGRAVITY) )             // Gravity here!
    {
        fixed_t     gravityadd=0;
		boolean no3dfloorgrav;
        
        //Fab: NOT SURE WHETHER IT IS USEFUL, just put it here too
        //     TO BE SURE there is no problem for the release..
        //     (this is done in P_Mobjthinker below normally)
        mo->eflags &= ~MF_JUSTHITFLOOR;

		// Custom gravity! Tails 06-14-2002
		no3dfloorgrav = true;
		if(mo->subsector->sector->ffloors) // Check for 3D floor gravity too. Tails 10-01-2003
		{
			ffloor_t* rover;

			for(rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
			{
				if(!(rover->flags & FF_EXISTS))
					continue;

				if(P_InsideANonSolidFFloor(mo, rover))
				{
					if(rover->master->frontsector->gravity != NULL)
					{
						gravityadd = -FixedMul(gravity, (FixedDiv(*rover->master->frontsector->gravity >> FRACBITS,1000))); // Graue 12-28-2003
						no3dfloorgrav = false;
						break;
					}
				}
			}
		}

		if(no3dfloorgrav)
		{
			if(mo->subsector->sector->gravity != NULL)
				gravityadd = -FixedMul(gravity, (FixedDiv(*mo->subsector->sector->gravity >> FRACBITS,1000)));
			else
				gravityadd = -gravity;
		}

		// Less gravity underwater.
		if(mo->eflags & MF_UNDERWATER)
			gravityadd = gravityadd/3; // Tails

		if (mo->momz==0) // mobj at stop, no floor, so feel the push of gravity!
            gravityadd <<= 1;

		if(mo->type == MT_CEILINGSPIKE)
		{
			gravityadd = -gravityadd; // Reverse gravity for ceiling spikes Tails
			if(mo->z + mo->height >= mo->ceilingz)
				gravityadd = 0;
		}

        mo->momz += gravityadd/NEWTICRATERATIO; // Tails
    }

    if (mo->z + mo->height > mo->ceilingz)
    {
		if(mo->momz > 0)
		{
			// hit the ceiling
			mo->momz = 0;
		}

        mo->z = mo->ceilingz - mo->height;

        if (mo->flags2 & MF2_SKULLFLY)
        {       // the skull slammed into something
            mo->momz = -mo->momz;
        }

		if (mo->type == MT_FIREBALL)
        {
            //SoM: 4/3/2000: Don't explode on the sky!
            if(mo->subsector->sector->ceilingpic == skyflatnum &&
               mo->subsector->sector->ceilingheight == mo->ceilingz)
            {
                P_SetMobjState(mo, S_DISS);
                return;
            }

			S_StartSound(mo, sfx_tink);

            P_ExplodeMissile (mo);
            return;
        }
        else if ( (mo->flags & MF_MISSILE)
             && !(mo->flags & MF_NOCLIP) )
        {
            //SoM: 4/3/2000: Don't explode on the sky!
            if(mo->subsector->sector->ceilingpic == skyflatnum &&
               mo->subsector->sector->ceilingheight == mo->ceilingz)
            {
				P_SetMobjState(mo, S_DISS);
//                P_RemoveMobj(mo);
                return;
            }

            P_ExplodeMissile (mo);
            return;
        }
    }
}

void P_PlayerZMovement (mobj_t* mo)
{
	if (!mo->player)
        return;             // mobj was removed

#ifdef FIXROVERBUGS
// Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
    if(mo->subsector->sector->ffloors)
    {
      ffloor_t*  rover;
      fixed_t    delta1;
      fixed_t    delta2;
      int        thingtop = mo->z + mo->height;

      for(rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
      {
		if(!(rover->flags & FF_EXISTS))
			continue;

        if((!(rover->flags & FF_SOLID || rover->flags & FF_QUICKSAND) && !(mo->player && !mo->player->nightsmode && mo->player->skin == 1 && !mo->player->mfspinning && mo->player->speed > 28 && mo->ceilingz - *rover->topheight >= mo->info->height && mo->z < *rover->topheight + 30*FRACUNIT && mo->z > *rover->topheight - 30*FRACUNIT))) continue;

		if(rover->flags & FF_QUICKSAND)
		{
			if(mo->z < *rover->topheight && *rover->bottomheight < thingtop)
			{
				mo->floorz = mo->z;
				continue;
			}
		}

        delta1 = mo->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > mo->floorz && abs(delta1) < abs(delta2))
          mo->floorz = *rover->topheight;
        if(*rover->bottomheight < mo->ceilingz && abs(delta1) >= abs(delta2))
		{
			if(!(rover->flags & FF_PLATFORM))
				mo->ceilingz = *rover->bottomheight;
		}
      }
    }
#endif

    // check for smooth step up
#ifdef CLIENTPREDICTION2
    if (mo->player && mo->z < mo->floorz && mo->type!=MT_PLAYER)
#else
    if (mo->z < mo->floorz && mo->type!=MT_SPIRIT)
#endif
    {
        mo->player->viewheight -= mo->floorz - mo->z;

        mo->player->deltaviewheight
            = ((cv_viewheight.value<<FRACBITS) - mo->player->viewheight)>>3;
    }

    // adjust height
    mo->z += mo->momz;

	// Have player fall through floor? 10-05-2001 Tails
	if (mo->player->playerstate == PST_DEAD)
		goto playergravity;
/*
	if(mo->flags2&MF2_FLY && !(mo->z <= mo->floorz)
		&& leveltime&2)
	{
		mo->z += finesine[(FINEANGLES/20*leveltime>>2)&FINEMASK];
	}*/

    // clip movement

    if (mo->z <= mo->floorz)
    {
		if(mo->player && mo->player->nightsmode)
		{
			if(mo->player->flyangle < 90 || mo->player->flyangle >= 270)
				mo->player->flyangle += 90;
			else
				mo->player->flyangle -= 90;
			mo->z = mo->floorz;
			mo->player->speed /= 5;
			mo->player->speed *= 4;
			goto nightsdone;
		}
		// Get up if you fell. Tails 07-24-2002
		if(mo->state == &states[S_PLAY_PAIN])
			P_SetMobjState(mo, S_PLAY_STND);

        mo->z = mo->floorz;

        if (mo->momz < 0) // falling
        {
            if (mo->player && (mo->momz < -8*FRACUNIT))
//                && !(mo->flags2&MF2_FLY))
            {
                // Squat down.
                // Decrease viewheight for a moment
                // after hitting the ground (hard),
                // and utter appropriate sound.
                mo->player->deltaviewheight = mo->momz>>3;
            }

			if((mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE) && mo->momz < -2*FRACUNIT)
				S_StartSound(mo, mo->info->activesound);

            // set it once and not continuously
// Tails
			if(tmfloorthing)
			{
				if((tmfloorthing->flags & MF_MONITOR) || (tmfloorthing->flags & MF_PUSHABLE)
					|| (tmfloorthing->flags2 & MF2_STANDONME))
				{
					if(mo->player)
					{
						if(!(mo->player->mfjumped))
						tmfloorthing = 0;
					}
				}
			}
            if ((mo->z <= mo->floorz) && (!(tmfloorthing) || (((tmfloorthing->flags & MF_PUSHABLE) || (tmfloorthing->flags2 & MF2_STANDONME)) || tmfloorthing->type == MT_PLAYER || tmfloorthing->type == MT_FLOORSPIKE))) // Tails 9-15-99 Spin Attack
			{
				if(tmfloorthing && mo->momz)
					mo->eflags |= MF_JUSTHITFLOOR;
				else if(!tmfloorthing)
					mo->eflags |= MF_JUSTHITFLOOR; // Tails 9-15-99 Spin Attack

				if(mo->eflags & MF_JUSTHITFLOOR)
				{
					// Cut momentum in half when you hit the ground and
					// aren't pressing any controls. Tails 03-03-2000
					if (!(mo->player->cmd.forwardmove || mo->player->cmd.sidemove) && !mo->player->cmomx && !mo->player->cmomy && !mo->player->mfspinning)
					{
						mo->momx = mo->momx/2;
						mo->momy = mo->momy/2;
					}
				}

				if(mo->player)
				{
					if(mo->health)
					{
						if(false/*mo->player->powers[pw_super]*/)
						{
							if(mo->player->rmomx || mo->player->rmomy)
								P_SetMobjState (mo, S_PLAY_ABL1);  // Super Sonic Stuff Tails 04-18-2000
							else
								P_SetMobjState (mo, S_PLAY_SPC1);  // Super Sonic Stuff Tails 04-18-2000
						}
						else if (mo->player->mfspinning == 1 && mo->player->usedown && !mo->player->mfjumped);
						else
						{
							const int runspeed = 28;

							if(mo->player->cmomx || mo->player->cmomy)
							{
								if(mo->player->speed > runspeed && !mo->player->running)
									P_SetMobjState (mo, S_PLAY_SPD1);
								else if ((mo->player->rmomx > STOPSPEED || mo->player->rmomy > STOPSPEED) && !mo->player->walking)
									P_SetMobjState (mo, S_PLAY_RUN1);
								else if ((mo->player->rmomx < -STOPSPEED || mo->player->rmomy < -STOPSPEED) && !mo->player->walking)
									P_SetMobjState (mo, S_PLAY_RUN1);
								else if ((mo->player->rmomx < FRACUNIT && mo->player->rmomx > -FRACUNIT && mo->player->rmomy < FRACUNIT && mo->player->rmomy > -FRACUNIT) && !(mo->player->walking || mo->player->running))
									P_SetMobjState (mo, S_PLAY_STND);
							}
							else
							{
								if(mo->player->speed > runspeed && !mo->player->running)
									P_SetMobjState (mo, S_PLAY_SPD1);
								else if ((mo->momx || mo->momy) && !mo->player->walking)
									P_SetMobjState (mo, S_PLAY_RUN1);
								else if (!mo->momx && !mo->momy && !(mo->player->walking || mo->player->running))
									P_SetMobjState (mo, S_PLAY_STND);
							}
						}

						if(mo->player->mfjumped)
							mo->player->mfspinning = 0;
						else if(!mo->player->usedown)
							mo->player->mfspinning = 0;

						P_ResetScore(mo->player);
						mo->player->mfjumped = 0; // Tails 9-15-99 Spin Attack
						mo->player->thokked = false;
						mo->player->gliding = 0;
						mo->player->glidetime = 0;
						mo->player->climbing = 0;
					}
				}
			}
// end Tails
			if(mo->player)
			{
				if(mo->player->mfspinning == 0)
                {
					mo->player->mfstartdash = 0; // dashing stuff Tails 02-27-2000
                }
			}

            if (!(tmfloorthing) || (((tmfloorthing->flags & MF_PUSHABLE) || (tmfloorthing->flags2 & MF2_STANDONME))|| tmfloorthing->type == MT_PLAYER || tmfloorthing->type == MT_FLOORSPIKE))
			{
              mo->momz = 0;
			}
        }

        mo->z = mo->floorz;
    }
    else if(mo->flags2&MF2_LOGRAV)
    {
		fixed_t gravityadd=0;

        if(mo->momz == 0)
            mo->momz = -(gravityadd>>3)*2;
        else
            mo->momz -= gravityadd>>3;
    }
    else if (! (mo->flags & MF_NOGRAVITY) )             // Gravity here!
    {
        fixed_t     gravityadd=0;
		boolean no3dfloorgrav;
        
        //Fab: NOT SURE WHETHER IT IS USEFUL, just put it here too
        //     TO BE SURE there is no problem for the release..
        //     (this is done in P_Mobjthinker below normally)
        mo->eflags &= ~MF_JUSTHITFLOOR;

		// Custom gravity! Tails 06-14-2002
		no3dfloorgrav = true;
		if(mo->subsector->sector->ffloors) // Check for 3D floor gravity too. Tails 10-01-2003
		{
			ffloor_t* rover;

			for(rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
			{
				if(!(rover->flags & FF_EXISTS))
					continue;

				if(P_InsideANonSolidFFloor(mo, rover))
				{
					if(rover->master->frontsector->gravity != NULL)
					{
						gravityadd = -FixedMul(gravity, (FixedDiv(*rover->master->frontsector->gravity >> FRACBITS,1000))); // Graue 12-28-2003
						no3dfloorgrav = false;
						break;
					}
				}
			}
		}
		
		if(no3dfloorgrav)
		{
			if(mo->subsector->sector->gravity != NULL)
				gravityadd = -FixedMul(gravity, (FixedDiv(*mo->subsector->sector->gravity >> FRACBITS,1000)));
			else
				gravityadd = -gravity;
		}

		// Less gravity underwater.
		if(mo->eflags & MF_UNDERWATER)
			gravityadd = gravityadd/3; // Tails

		if (mo->momz==0) // mobj at stop, no floor, so feel the push of gravity!
            gravityadd <<= 1;

playergravity:
		if ((mo->player->charability==1) && ((mo->player->powers[pw_tailsfly]) || (mo->player->mo->state == &states[S_PLAY_SPC1]) || (mo->player->mo->state == &states[S_PLAY_SPC2]) || (mo->player->mo->state == &states[S_PLAY_SPC3]) || (mo->player->mo->state == &states[S_PLAY_SPC4])))
			gravityadd = gravityadd/3; // less gravity while flying
		if(mo->player->gliding)
			gravityadd = gravityadd/3; // less gravity while gliding
		if(mo->player->climbing)
			gravityadd = 0;
		if(mo->player->nightsmode)
			gravityadd = 0; // Tails 07-02-2001
		if(mo->player->playerstate == PST_DEAD) // Added crash check Tails 11-16-2001)
		{
			// Custom gravity! Tails 06-14-2002
			if(mo->subsector->sector->gravity != NULL)
				gravityadd = -FixedMul(gravity, (FixedDiv(*mo->subsector->sector->gravity >> FRACBITS,1000)));
			else
				gravityadd = -gravity;

			mo->momz += gravityadd/NEWTICRATERATIO;
			return;
		}

        mo->momz += gravityadd/NEWTICRATERATIO; // Tails
    }

nightsdone:

    if (mo->z + mo->height > mo->ceilingz)
    {
		if(mo->player && mo->player->nightsmode)
		{
			if(mo->player->flyangle < 90 || mo->player->flyangle >= 270)
				mo->player->flyangle -= 90;
			else
				mo->player->flyangle += 90;
			mo->player->flyangle %= 360;
			mo->z = mo->ceilingz - mo->height;
			mo->player->speed /= 5;
			mo->player->speed *= 4;
		}

		// Check for "Mario" blocks to hit.
		// and bounce them Tails 03-11-2002
		if(mo->momz > 0)
		{
			msecnode_t *node;

			if(mo->player) // Only let the player punch
			{
				// Search the touching sectors, from side-to-side...
				for (node = mo->touching_sectorlist; node; node = node->m_snext)
				{
					if (node->m_sector->ffloors)
					{
	 					ffloor_t* rover;

						for(rover = node->m_sector->ffloors; rover; rover = rover->next)
						{
							// Come on, it's time to go...
							if(rover->flags & FF_MARIO
								&& *rover->bottomheight == mo->ceilingz) // The player's head hit the bottom!
							{
								// DO THE MARIO!
								EV_MarioBlock(rover->master->frontsector, node->m_sector, *rover->topheight, rover->master, mo);
							}
						}
					}
				} // Ugly ugly billions of braces! Argh! Tails
			}

			// hit the ceiling
			if(mariomode)
				S_StartSound(mo, sfx_mario1);

			mo->momz = 0;
		}

        mo->z = mo->ceilingz - mo->height;
    }
}

void P_SceneryZMovement (mobj_t* mo)
{
#ifdef FIXROVERBUGS
// Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
    if(mo->subsector->sector->ffloors)
    {
      ffloor_t*  rover;
      fixed_t    delta1;
      fixed_t    delta2;
      int        thingtop = mo->z + mo->height;

      for(rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
      {
		if(!(rover->flags & FF_EXISTS))
			continue;

        if((!(rover->flags & FF_SOLID || rover->flags & FF_QUICKSAND) || (rover->flags & FF_SWIMMABLE)))
			continue;

		if(rover->flags & FF_QUICKSAND)
		{
			if(mo->z < *rover->topheight && *rover->bottomheight < thingtop)
			{
				mo->floorz = mo->z;
				continue;
			}
		}

        delta1 = mo->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > mo->floorz && abs(delta1) < abs(delta2))
          mo->floorz = *rover->topheight;
        if(*rover->bottomheight < mo->ceilingz && abs(delta1) >= abs(delta2))
		{
			if(!(rover->flags & FF_PLATFORM))
				mo->ceilingz = *rover->bottomheight;
		}
      }
    }
#endif

    // adjust height
    mo->z += mo->momz;

	switch(mo->type)
	{
		case MT_SMALLBUBBLE:
			if(mo->z <= mo->floorz) // Hit the floor, so POP!
			{
				int random;

				P_SetMobjState(mo, S_DISS);

				if(mo->threshold == 42) // Don't make pop sound.
					break;

				random = P_Random();

				if(random <= 51)
					S_StartSound(mo, sfx_bubbl1);
				else if (random <= 102)
					S_StartSound(mo, sfx_bubbl2);
				else if (random <= 153)
					S_StartSound(mo, sfx_bubbl3);
				else if (random <= 204)
					S_StartSound(mo, sfx_bubbl4);
				else
					S_StartSound(mo, sfx_bubbl5);
			}
			break;
		case MT_MEDIUMBUBBLE:
			if(mo->z <= mo->floorz) // Hit the floor, so split!
			{
				// split
				mobj_t* explodemo;

				explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_SMALLBUBBLE);
				explodemo->momx += (P_Random() % 32) * FRACUNIT/8;
				explodemo->momy += (P_Random() % 32) * FRACUNIT/8;
				explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_SMALLBUBBLE);
				explodemo->momx += (P_Random() % 64) * FRACUNIT/8;
				explodemo->momy -= (P_Random() % 64) * FRACUNIT/8;
				explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_SMALLBUBBLE);
				explodemo->momx -= (P_Random() % 128) * FRACUNIT/8;
				explodemo->momy += (P_Random() % 128) * FRACUNIT/8;
				explodemo = P_SpawnMobj(mo->x, mo->y, mo->z, MT_SMALLBUBBLE);
				explodemo->momx -= (P_Random() % 96) * FRACUNIT/8;
				explodemo->momy -= (P_Random() % 96) * FRACUNIT/8;
			}
			break;
		default:
			break;
	}

    // clip movement
    if (mo->z <= mo->floorz)
    {
        mo->z = mo->floorz;

        if (mo->momz < 0) // falling
        {
            if ((!(tmfloorthing) || (((tmfloorthing->flags & MF_PUSHABLE) || (tmfloorthing->flags2 & MF2_STANDONME)) || tmfloorthing->type == MT_PLAYER || tmfloorthing->type == MT_FLOORSPIKE))) // Tails 9-15-99 Spin Attack
			{
				if(tmfloorthing && mo->momz)
					mo->eflags |= MF_JUSTHITFLOOR;
				else if(!tmfloorthing)
					mo->eflags |= MF_JUSTHITFLOOR; // Tails 9-15-99 Spin Attack
			}
// end Tails

            if (!tmfloorthing)
			{
              mo->momz = 0;
			}
        }

        mo->z = mo->floorz;
    }
    else if (! (mo->flags & MF_NOGRAVITY) )             // Gravity here!
    {
        fixed_t     gravityadd=0;
        
        //Fab: NOT SURE WHETHER IT IS USEFUL, just put it here too
        //     TO BE SURE there is no problem for the release..
        //     (this is done in P_Mobjthinker below normally)
        mo->eflags &= ~MF_JUSTHITFLOOR;

		// Custom gravity! Tails 06-14-2002
		if(mo->subsector->sector->gravity != NULL)
			gravityadd = -FixedMul(gravity, (FixedDiv(*mo->subsector->sector->gravity >> FRACBITS,1000)));
		else
			gravityadd = -gravity;

		// Less gravity underwater.
		if(mo->eflags & MF_UNDERWATER)
			gravityadd = gravityadd/3; // Tails

		if (mo->momz==0) // mobj at stop, no floor, so feel the push of gravity!
            gravityadd <<= 1;

        mo->momz += gravityadd/NEWTICRATERATIO; // Tails
    }

    if (mo->z + mo->height > mo->ceilingz)
    {
		if(mo->momz > 0)
		{
			// hit the ceiling
			mo->momz = 0;
		}

        mo->z = mo->ceilingz - mo->height;
    }
}

// Checks for water at the old position.
// Returns 'true' if the MF_UNDERWATER flag would have been set before.
// Tails 10-01-2002
boolean P_MobjCheckOldPosWater (mobj_t* mobj)
{
    sector_t* sector;
    fixed_t   z;
	fixed_t   oldz; // Old z pos of mobj

	oldz = mobj->z - mobj->momz;

    //
    // see if we are in water, and set some flags for later
    //
    sector = mobj->subsector->sector;
    z = sector->floorheight;

    //SoM: 3/28/2000: Only use 280 water type of water. Some boom levels get messed up.
    if ((sector->heightsec > -1 && sector->altheightsec == 1) ||
        (sector->floortype==FLOOR_WATER && sector->heightsec == -1))
    {
        if (sector->heightsec > -1)  //water hack
            z = (sectors[sector->heightsec].floorheight);
        else
            z = sector->floorheight + (FRACUNIT/4); // water texture

        if (oldz+(mobj->info->height>>1) <= z) // Tails
		{
            return true;
		}
        else
		{
            return false;
		}
    }
    if(sector->ffloors)
    {
      ffloor_t*  rover;

      for(rover = sector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SWIMMABLE) || rover->flags & FF_SOLID)
          continue;
        if(*rover->topheight <= oldz || *rover->bottomheight > (oldz + (mobj->info->height >> 1)))
          continue;

        if(oldz + (mobj->info->height >> 1) < *rover->topheight)
		{
            return true;
		}
        else
		{
            return false;
		}
      }
    }

    return false;

}

//
// P_MobjCheckWater : check for water, set stuff in mobj_t struct for
//                    movement code later, this is called either by P_MobjThinker() or P_PlayerThink()
void P_MobjCheckWater (mobj_t* mobj)
{
    sector_t* sector;
    fixed_t   z;
    int       oldeflags;

    if(mobj->type==MT_SPLASH || mobj->type==MT_SPIRIT) // splash don't do splash
        return;

	// Default if no water exists.
	mobj->watertop = mobj->waterbottom = mobj->subsector->sector->floorheight - 1000*FRACUNIT;
    //
    // see if we are in water, and set some flags for later
    //
    sector = mobj->subsector->sector;
    z = sector->floorheight;
    oldeflags = mobj->eflags;

    //SoM: 3/28/2000: Only use 280 water type of water. Some boom levels get messed up.
    if ((sector->heightsec > -1 && sector->altheightsec == 1) ||
        (sector->floortype==FLOOR_WATER && sector->heightsec == -1))
    {
        if (sector->heightsec > -1)  //water hack
            z = (sectors[sector->heightsec].floorheight);
        else
            z = sector->floorheight + (FRACUNIT/4); // water texture

        if (mobj->z<=z && mobj->z+mobj->info->height > z)
            mobj->eflags |= MF_TOUCHWATER;
        else
            mobj->eflags &= ~MF_TOUCHWATER;

		// Set the watertop and waterbottom Tails 02-03-2002
		mobj->watertop = z;
		mobj->waterbottom = sector->floorheight;

        if (mobj->z+(mobj->info->height>>1) <= z) // Tails
		{
            mobj->eflags |= MF_UNDERWATER;

			if(mobj->player) // Underwater player stuff Tails
			{
				if(!((mobj->player->powers[pw_super]) || (mobj->player->powers[pw_invulnerability])))
				{
					if(sector->special != 7)
						mobj->player->powers[pw_redshield] = false;

					mobj->player->powers[pw_yellowshield] = false;
				}
				if (mobj->player->powers[pw_underwater] <= 0 && !(mobj->player->powers[pw_greenshield])) // Tails 03-06-2000
				{// Tails 03-06-2000
					if(mobj->player->powers[pw_underwater] < underwatertics + 1)
						mobj->player->powers[pw_underwater] = underwatertics + 1; // Tails 03-06-2000
				}// Tails 03-06-2000
			}
		}
        else
		{
            mobj->eflags &= ~MF_UNDERWATER;
			// Return of WaterZ! Tails 10-31-2000
//			mobj->waterz = mobj->subsector->sector->floorheight - 10000*FRACUNIT;
		}
    }
    else if(sector->ffloors)
    {
      ffloor_t*  rover;

      mobj->eflags &= ~(MF_UNDERWATER|MF_TOUCHWATER);

      for(rover = sector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SWIMMABLE) || rover->flags & FF_SOLID)
          continue;
        if(*rover->topheight < mobj->z || *rover->bottomheight > (mobj->z + (mobj->info->height >> 1)))
          continue;

        if(mobj->z + mobj->info->height > *rover->topheight)
            mobj->eflags |= MF_TOUCHWATER;
        else
            mobj->eflags &= ~MF_TOUCHWATER;

		// Set the watertop and waterbottom Tails 02-03-2002
		mobj->watertop = *rover->topheight;
		mobj->waterbottom = *rover->bottomheight;

        if(mobj->z + (mobj->info->height >> 1) < *rover->topheight)
		{ // Tails
            mobj->eflags |= MF_UNDERWATER;

			if(mobj->player)
			{
				if(!((mobj->player->powers[pw_super]) || (mobj->player->powers[pw_invulnerability])))
				{
					if(rover->master->frontsector->special != 7)
						mobj->player->powers[pw_redshield] = false;

					mobj->player->powers[pw_yellowshield] = false;
				}
				if (mobj->player->powers[pw_underwater] <= 0 && !(mobj->player->powers[pw_greenshield])) // Tails 03-06-2000
				{// Tails 03-06-2000
					if(mobj->player->powers[pw_underwater] < underwatertics + 1)
						mobj->player->powers[pw_underwater] = underwatertics + 1; // Tails 03-06-2000
				}// Tails 03-06-2000
			}
		} // Tails
        else
		{
            mobj->eflags &= ~MF_UNDERWATER;
		}
      }
    }
    else
	{
        mobj->eflags &= ~(MF_UNDERWATER|MF_TOUCHWATER);
		// Return of WaterZ! Tails 10-31-2000
//		mobj->waterz = mobj->subsector->sector->floorheight - 10000*FRACUNIT;
	}

/*
    if( (mobj->eflags ^ oldeflags) & MF_TOUCHWATER)
        CONS_Printf("touchewater %d\n",mobj->eflags & MF_TOUCHWATER ? 1 : 0);
    if( (mobj->eflags ^ oldeflags) & MF_UNDERWATER)
        CONS_Printf("underwater %d\n",mobj->eflags & MF_UNDERWATER ? 1 : 0);
*/
}

void P_SceneryCheckWater (mobj_t* mobj)
{
    sector_t* sector;
    fixed_t   z;

	// Default if no water exists.
	mobj->watertop = mobj->waterbottom = mobj->subsector->sector->floorheight - 10000*FRACUNIT;
    //
    // see if we are in water, and set some flags for later
    //
    sector = mobj->subsector->sector;
    z = sector->floorheight;

    //SoM: 3/28/2000: Only use 280 water type of water. Some boom levels get messed up.
    if ((sector->heightsec > -1 && sector->altheightsec == 1) ||
        (sector->floortype==FLOOR_WATER && sector->heightsec == -1))
    {
        if (sector->heightsec > -1)  //water hack
            z = (sectors[sector->heightsec].floorheight);
        else
            z = sector->floorheight + (FRACUNIT/4); // water texture

		// Set the watertop and waterbottom Tails 02-03-2002
		mobj->watertop = z;
		mobj->waterbottom = sector->floorheight;

        if (mobj->z+(mobj->info->height>>1) <= z) // Tails
		{
            mobj->eflags |= MF_UNDERWATER;
		}
        else
		{
            mobj->eflags &= ~MF_UNDERWATER;
		}
    }
    else if(sector->ffloors)
    {
      ffloor_t*  rover;

      mobj->eflags &= ~(MF_UNDERWATER|MF_TOUCHWATER);

      for(rover = sector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SWIMMABLE) || rover->flags & FF_SOLID)
          continue;
        if(*rover->topheight <= mobj->z || *rover->bottomheight > (mobj->z + (mobj->info->height >> 1)))
          continue;

        if(mobj->z + mobj->info->height > *rover->topheight)
            mobj->eflags |= MF_TOUCHWATER;
        else
            mobj->eflags &= ~MF_TOUCHWATER;

		// Set the watertop and waterbottom Tails 02-03-2002
		mobj->watertop = *rover->topheight;
		mobj->waterbottom = *rover->bottomheight;

        if(mobj->z + (mobj->info->height >> 1) < *rover->topheight)
		{ // Tails
            mobj->eflags |= MF_UNDERWATER;
		} // Tails
        else
		{
            mobj->eflags &= ~MF_UNDERWATER;
		}
      }
    }
    else
	{
        mobj->eflags &= ~(MF_UNDERWATER|MF_TOUCHWATER);
	}
}

// Tails 08-13-2002
void P_DestroyRobots(void)
{
	// Search through all the thinkers for enemies.
    int count;
    mobj_t *mo;
    thinker_t *think;

    count = 0;
    for(think = thinkercap.next; think != &thinkercap; think = think->next)
    {
        if(think->function.acp1 != (actionf_p1)P_MobjThinker)
        { // Not a mobj thinker
                continue;
        }
        mo = (mobj_t *)think;
        if(!(mo->flags&MF_COUNTKILL) || (mo == players[consoleplayer].mo) || (mo->health <= 0))
        { // Not a valid monster
		    continue;
        }

		if(mo->type == MT_PLAYER) // Don't chase after other players!
			continue;

        // Found a target monster
        P_DamageMobj(mo, players[consoleplayer].mo, players[consoleplayer].mo, 10000);
	}
}


//===========================================================================
//
// PlayerLandedOnThing
//
//===========================================================================
static void PlayerLandedOnThing(mobj_t *mo, mobj_t *onmobj)
{
    mo->player->deltaviewheight = mo->momz>>3;
/*
    if(mo->momz < -23*FRACUNIT)
    {
        //P_FallingDamage(mo->player);
        P_NoiseAlert(mo, mo);
    }
    else if(mo->momz < -8*FRACUNIT)
    {
        S_StartSound(mo, sfx_pop);
    }*/
}

extern boolean P_TryCameraMove(fixed_t x, fixed_t y, camera_t* thiscam);
extern void P_SlideCameraMove(camera_t* thiscam);

// P_CameraThinker
//
// Process the mobj-ish required functions of the camera
// Tails 09-29-2002
void P_CameraThinker(camera_t* thiscam)
{
    if ( thiscam->momx || thiscam->momy )
	{
		fixed_t     ptryx;
		fixed_t     ptryy;
		fixed_t     xmove;
		fixed_t     ymove;
		fixed_t     oldx, oldy; //reducing bobbing/momentum on ice
									//when up against walls

		if (thiscam->momx > MAXMOVE)
			thiscam->momx = MAXMOVE;
		else if (thiscam->momx < -MAXMOVE)
			thiscam->momx = -MAXMOVE;

		if (thiscam->momy > MAXMOVE)
			thiscam->momy = MAXMOVE;
		else if (thiscam->momy < -MAXMOVE)
			thiscam->momy = -MAXMOVE;

		xmove = thiscam->momx;
		ymove = thiscam->momy;

		oldx = thiscam->x;
		oldy = thiscam->y;

		do
		{
			if (xmove > MAXMOVE/2 || ymove > MAXMOVE/2)
			{
				ptryx = thiscam->x + xmove/2;
				ptryy = thiscam->y + ymove/2;
				xmove >>= 1;
				ymove >>= 1;
			}
			else
			{
				ptryx = thiscam->x + xmove;
				ptryy = thiscam->y + ymove;
				xmove = ymove = 0;
			}

			if (!P_TryCameraMove (ptryx, ptryy, thiscam)) //SoM: 4/10/2000
				P_SlideCameraMove(thiscam);

		} while (xmove || ymove);
	}

	thiscam->subsector = R_PointInSubsector(thiscam->x, thiscam->y);

    //added:28-02-98: always do the gravity bit now, that's simpler
    //                BUT CheckPosition only if wasn't do before.
    if ( thiscam->momz )
	{
 	    #ifdef FIXROVERBUGS
	    // Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
		if(thiscam->subsector->sector->ffloors)
		{
		  ffloor_t*  rover;
		  fixed_t    delta1;
		  fixed_t    delta2;
		  int        thingtop = thiscam->z + thiscam->height;

		  for(rover = thiscam->subsector->sector->ffloors; rover; rover = rover->next)
		  {
			if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS)) continue;

			delta1 = thiscam->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
			delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
			if(*rover->topheight > thiscam->floorz && abs(delta1) < abs(delta2))
			  thiscam->floorz = *rover->topheight;
			if(*rover->bottomheight < thiscam->ceilingz && abs(delta1) >= abs(delta2))
				thiscam->ceilingz = *rover->bottomheight;
		  }
		}
	    #endif

		// adjust height
		thiscam->z += thiscam->momz;

		// clip movement
		if (thiscam->z <= thiscam->floorz)
		{
			// hit the floor
			thiscam->z = thiscam->floorz;
		}

		if (thiscam->z + thiscam->height > thiscam->ceilingz)
		{
			if(thiscam->momz > 0)
			{
				// hit the ceiling
				thiscam->momz = 0;
			}

			thiscam->z = thiscam->ceilingz - thiscam->height;
		}
	}
}

//
// P_PlayerMobjThinker
//
void P_PlayerMobjThinker (mobj_t* mobj)
{
	msecnode_t *node; // Tails 02-08-2002
/*	fixed_t zatpoint = 0;

	zatpoint = R_SecplaneZatPoint(&mobj->subsector->sector->floorplane, mobj->x, mobj->y);

	CONS_Printf("Zatpoint is %d\n", zatpoint);*/

	// Make sure player shows dead Tails 03-15-2000
    if(mobj->health <= 0)
	{
		if(mobj->state == &states[S_DISS])
		{
			P_RemoveMobj(mobj);
			return;
		}

		P_SetMobjState (mobj, S_PLAY_DIE3);
		mobj->flags2 &= ~MF2_DONTDRAW;
		P_PlayerZMovement(mobj);
		return;
	}

	P_MobjCheckWater(mobj);

    //
    // momentum movement
    //
#ifdef CLIENTPREDICTION2
    // move player mobj (not the spirit) to spirit position (sent by ticcmd)
    if((mobj->type==MT_PLAYER) && (mobj->player) && 
        ((mobj->player->cmd.angleturn&(TICCMD_XY|TICCMD_RECEIVED))==(TICCMD_XY|TICCMD_RECEIVED)) && 
        (mobj->player->playerstate == PST_LIVE))
    {
        int oldx = mobj->x, oldy = mobj->y;

        if( oldx!=mobj->player->cmd.x || oldy!=mobj->player->cmd.y )
        {
            mobj->eflags |= MF_NOZCHECKING;
            // cross special lines and pick up things
            if(!P_TryMove (mobj, mobj->player->cmd.x, mobj->player->cmd.y, true))
            {
                // P_TryMove fail mean cannot change mobj position to requestied position
                // the mobj is blocked by something
                if (mobj->player-players==consoleplayer)
                {
                    // reset spirit possition
                    CL_ResetSpiritPosition(mobj);

                    //if(devparm)
                    CONS_Printf("\2MissPrediction\n");
                }
            }
            mobj->eflags &= ~MF_NOZCHECKING;
        }
        P_XYFriction (mobj, oldx, oldy, false);

    }
    else
#endif
    if ( mobj->momx || mobj->momy )
    {
        P_XYMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if ((mobj->thinker.function.acv == (actionf_v) (-1)))
            return;             // mobj was removed
    }
	else
		P_TryMove(mobj, mobj->x, mobj->y, true);

	// Crumbling platforms
	for (node = mobj->touching_sectorlist; node; node = node->m_snext)
	{
		if (node->m_sector->ffloors)
		{
			ffloor_t* rover;

			for(rover = node->m_sector->ffloors; rover; rover = rover->next)
			{
				if(*rover->topheight == mobj->z)
				{
					if(rover->flags & FF_CRUMBLE) // No, you shouldn't be using both of these together, you nut.
					{
						if(rover->flags & FF_FLOATBOB)
						{
							if(rover->flags & FF_NORETURN)
								EV_StartNoReturnCrumble(rover->master->frontsector, node->m_sector, *rover->topheight, true, mobj->player);
							else
								EV_StartCrumble(rover->master->frontsector, node->m_sector, *rover->topheight, true, mobj->player);
						}
						else
						{
							if(rover->flags & FF_NORETURN)
								EV_StartNoReturnCrumble(rover->master->frontsector, node->m_sector, *rover->topheight, false, mobj->player);
							else
								EV_StartCrumble(rover->master->frontsector, node->m_sector, *rover->topheight, false, mobj->player);
						}
					}
				}
			}
		}
	}

	// Check for floating water platforms
	// and bounce them Tails 02-08-2002
	if(mobj->momz < 0)
	{
		msecnode_t *node; // Tails 02-08-2002
		fixed_t watertop;
		fixed_t waterbottom;
		boolean roverfound;

		watertop = waterbottom = 0;
		roverfound = false;

		for (node = mobj->touching_sectorlist; node; node = node->m_snext)
		{
			if (node->m_sector->ffloors)
			{
				ffloor_t* rover;
				// Get water boundaries first
				for(rover = node->m_sector->ffloors; rover; rover = rover->next)
				{
					if(rover->flags & FF_SWIMMABLE) // Is there water?
					{
						watertop = *rover->topheight;
						waterbottom = *rover->bottomheight;
						roverfound = true;
						break;
					}
				}

				// Support for bobbing platforms in "old" water Tails 02-15-2002
				if (!roverfound && node->m_sector->heightsec > -1 && node->m_sector->altheightsec == 1)
				{
					// Set the watertop and waterbottom Tails 02-03-2002
					watertop = (sectors[node->m_sector->heightsec].floorheight);
					waterbottom = node->m_sector->floorheight;
				}
			}
		}
		if(watertop)
		{
			for (node = mobj->touching_sectorlist; node; node = node->m_snext)
			{
				if (node->m_sector->ffloors)
				{
					ffloor_t* rover;
					for(rover = node->m_sector->ffloors; rover; rover = rover->next)
					{
						if(rover->flags & FF_FLOATBOB
							&& *rover->topheight <= mobj->z+abs(mobj->momz)
							&& *rover->topheight >= mobj->z-abs(mobj->momz)) // The player is landing on the cheese!
						{
							// Initiate a 'bouncy' elevator function
							// which slowly diminishes.
							EV_BounceSector(rover->master->frontsector, mobj->momz, node->m_sector, true);
						}
					}
				}
			}
		} // Ugly ugly billions of braces! Argh! Tails
	}

    //added:28-02-98: always do the gravity bit now, that's simpler
    //                BUT CheckPosition only if wasn't do before.
    if ((mobj->eflags & MF_ONGROUND)==0 || 
		(mobj->z != mobj->floorz) ||
          mobj->momz )
    {
		mobj_t *onmo;
        onmo = P_CheckOnmobj(mobj);
        if(!onmo)
        {
			P_PlayerZMovement(mobj);
			P_CheckPosition (mobj, mobj->x, mobj->y); // Need this to pick up objects! Tails 02-01-2002
			if(mobj->flags&MF2_ONMOBJ )
				mobj->flags2 &= ~MF2_ONMOBJ;
		}
        else
        {
			if(mobj->momz < -8*FRACUNIT )//&& !(mobj->flags2&MF2_FLY))
            {
				PlayerLandedOnThing(mobj, onmo);
            }
            if(onmo->z+onmo->height-mobj->z <= 24*FRACUNIT)
            {
				mobj->player->viewheight -= onmo->z+onmo->height
				-mobj->z;
				mobj->player->deltaviewheight = 
				(VIEWHEIGHT-mobj->player->viewheight)>>3;
				mobj->z = onmo->z+onmo->height;
				mobj->flags2 |= MF2_ONMOBJ;
				mobj->momz = 0;
            }                               
            else
            { // hit the bottom of the blocking mobj
				mobj->momz = 0;
            }
        }

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v) (-1))
            return;             // mobj was removed
    }
    else
        mobj->eflags &= ~MF_JUSTHITFLOOR;
/*
    // SoM: Floorhuggers stay on the floor allways...
    // BP: tested here but never set ?!
    if(mobj->info->flags & MF_FLOORHUGGER)
    {
      mobj->z = mobj->floorz;
    }
*/
    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic
        if (!mobj->tics)
            if (!P_SetMobjState (mobj, mobj->state->nextstate) )
                return;         // freed itself
    }
    else
    {
        // check for nightmare respawn (Not anymore! Tails)
        return;
    }

//	P_CheckPosition (mobj, mobj->x, mobj->y);
}

void P_SnowThinker(precipmobj_t* mobj)
{
	#ifdef FIXROVERBUGS
	// Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
	if(mobj->subsector->sector->ffloors)
	{
	  ffloor_t*  rover;
	  fixed_t    delta1;
	  fixed_t    delta2;
	  int        thingtop = mobj->z + 4*FRACUNIT;

	  for(rover = mobj->subsector->sector->ffloors; rover; rover = rover->next)
	  {
	    if(!(rover->flags & FF_EXISTS)) continue;

		delta1 = mobj->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
		delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
		if(*rover->topheight > mobj->floorz && abs(delta1) < abs(delta2))
		  mobj->floorz = *rover->topheight;
	  }
	}
	#endif

		//SoM: 3/28/2000: Only use 270 water type of water. Some boom levels get messed up.
		if (mobj->subsector->sector->heightsec > -1 && mobj->subsector->sector->altheightsec == 1)
		{
			if (mobj->z <= sectors[mobj->subsector->sector->heightsec].floorheight) // Tails
			{
				mobj->floorz = sectors[mobj->subsector->sector->heightsec].floorheight;
			}
		}
		
		// adjust height
		mobj->z += mobj->momz;

		if(mobj->z <= mobj->floorz)
		{
			mobj->z = mobj->subsector->sector->ceilingheight;
		}

		return;
}

void P_RemovePrecipMobj(precipmobj_t* mobj);

void P_RainThinker(precipmobj_t* mobj)
{
	#ifdef FIXROVERBUGS
	// Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
	if(mobj->subsector->sector->ffloors)
	{
	  ffloor_t*  rover;
	  fixed_t    delta1;
	  fixed_t    delta2;
	  int        thingtop = mobj->z + 4*FRACUNIT;

	  for(rover = mobj->subsector->sector->ffloors; rover; rover = rover->next)
	  {
	    if(!(rover->flags & FF_EXISTS)) continue;

		delta1 = mobj->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
		delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
		if(*rover->topheight > mobj->floorz && abs(delta1) < abs(delta2))
		  mobj->floorz = *rover->topheight;
	  }
	}
	#endif

		//SoM: 3/28/2000: Only use 270 water type of water. Some boom levels get messed up.
		if (mobj->subsector->sector->heightsec > -1 && mobj->subsector->sector->altheightsec == 1)
		{
			if (mobj->z <= sectors[mobj->subsector->sector->heightsec].floorheight) // Tails
			{
				mobj->floorz = sectors[mobj->subsector->sector->heightsec].floorheight;
			}
		}
		
		// adjust height
		mobj->z += mobj->momz;

		if(mobj->state != &states[S_RAIN1])
		{
		    // cycle through states,
			// calling action functions at transitions
			if (mobj->tics != -1)
			{
				mobj->tics--;

				// you can cycle through multiple states in a tic
				if (!mobj->tics)
					if (!P_SetPrecipMobjState (mobj, mobj->state->nextstate) )
						return;         // freed itself
			}

			if(mobj->state == &states[S_RAINRETURN])
			{
				mobj->z = mobj->subsector->sector->ceilingheight;
				mobj->momz = mobjinfo[MT_RAIN].speed;
				P_SetPrecipMobjState(mobj, S_RAIN1);
			}
		}
		else if(mobj->z <= mobj->floorz && mobj->momz)
		{
			mobj->momz = 0;
			mobj->z = mobj->floorz;
			P_SetPrecipMobjState(mobj, S_SPLASH1);
		}

		return;
}
void P_SplashThinker(precipmobj_t* mobj)
{

}

void P_RingThinker (mobj_t* mobj)
{
    if ( mobj->momx || mobj->momy )
    {
        P_RingXYMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if ((mobj->thinker.function.acv == (actionf_v) (-1)))
            return;             // mobj was removed
    }

    //added:28-02-98: always do the gravity bit now, that's simpler
    //                BUT CheckPosition only if wasn't do before.
    if ((mobj->eflags & MF_ONGROUND)==0 || 
		(mobj->z != mobj->floorz) ||
          mobj->momz )
    {
		P_RingZMovement(mobj);
		P_CheckPosition (mobj, mobj->x, mobj->y); // Need this to pick up objects! Tails 02-01-2002

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v) (-1))
            return;             // mobj was removed
    }

    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic
        if (!mobj->tics)
            if (!P_SetMobjState (mobj, mobj->state->nextstate) )
                return;         // freed itself
    }
}

void P_LaserThinker (precipmobj_t* mobj)
{
    // FIXME: decent NOP/NULL/Nil function pointer please.
    if (mobj->thinker.function.acv == (actionf_v) (-1))
        return;             // mobj was removed

    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic
        if (!mobj->tics)
            if (!P_SetPrecipMobjState (mobj, mobj->state->nextstate) )
                return;         // freed itself
    }
}

void A_Boss1Chase(mobj_t* actor);
//
// P_Look4Players
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
boolean P_Look4Players ( mobj_t*       actor,
                                  boolean       allaround )
{
    int         c;
    int         stop;
    player_t*   player;
    sector_t*   sector;
    angle_t     an;
    fixed_t     dist;

    sector = actor->subsector->sector;

    // BP: first time init, this allow minimum lastlook changes
    if(actor->lastlook<0)
        actor->lastlook = P_Random () % MAXPLAYERS;

    c = 0;
    stop = (actor->lastlook-1)&PLAYERSMASK;

    for ( ; ; actor->lastlook = (actor->lastlook+1)&PLAYERSMASK )
    {
        // done looking
        if (actor->lastlook == stop)
            return false;

        if (!playeringame[actor->lastlook])
            continue;

        if (c++ == 2)
            return false;

        player = &players[actor->lastlook];

        if (player->health <= 0)
            continue;           // dead

		if(!player->mo)
			continue;

        if (!P_CheckSight (actor, player->mo))
            continue;           // out of sight

        if (!allaround)
        {
            an = R_PointToAngle2 (actor->x,
                                  actor->y,
                                  player->mo->x,
                                  player->mo->y)
                - actor->angle;

            if (an > ANG90 && an < ANG270)
            {
                dist = P_AproxDistance (player->mo->x - actor->x,
                                        player->mo->y - actor->y);
                // if real close, react anyway
                if (dist > MELEERANGE)
                    continue;   // behind back
            }
        }

        actor->target = player->mo;
        return true;
    }

    return false;
}

// Finds the player no matter what they're hiding behind (even lead!)
boolean P_SupermanLook4Players ( mobj_t*       actor)
{
    int         c;
    int         stop;
    player_t*   playersinthegame[MAXPLAYERS];

	stop = 0;

	for(c=0;c<MAXPLAYERS;c++)
	{
		if(playeringame[c])
		{
			if (players[c].health <= 0)
				continue;           // dead

			if(!players[c].mo)
				continue;

			playersinthegame[stop] = &players[c];
			stop++;
		}
	}

	if(stop == 0)
		return false;

    actor->target = playersinthegame[P_Random()%stop]->mo;
    return true;
}

fixed_t P_ReturnThrustX(mobj_t* mo, angle_t angle, fixed_t move);
fixed_t P_ReturnThrustY(mobj_t* mo, angle_t angle, fixed_t move);

// AI for the first boss. (Moved here and improved)
// Tails 10-28-2002
void P_Boss1Thinker(mobj_t* mobj)
{
	if(mobj->state->nextstate == mobj->info->spawnstate && mobj->tics == 1)
	{
		mobj->flags2 &= ~MF2_FRET;
	}

	if(!mobj->tracer)
	{
		fixed_t jetx, jety;
		mobj_t* filler;

		jetx = mobj->x + P_ReturnThrustX(mobj, mobj->angle, -56*FRACUNIT);
		jety = mobj->y + P_ReturnThrustY(mobj, mobj->angle, -56*FRACUNIT);

		// Spawn the jets here
		filler = P_SpawnMobj(jetx, jety, mobj->z + 8*FRACUNIT, MT_JETFUME1);
		filler->target = mobj;
		filler->fuse = 56;
		
		filler = P_SpawnMobj(jetx + P_ReturnThrustX(mobj, mobj->angle-ANG90, 24*FRACUNIT), jety + P_ReturnThrustY(mobj, mobj->angle-ANG90, 24*FRACUNIT), mobj->z + 32*FRACUNIT, MT_JETFUME1);
		filler->target = mobj;
		filler->fuse = 57;

		filler = P_SpawnMobj(jetx + P_ReturnThrustX(mobj, mobj->angle+ANG90, 24*FRACUNIT), jety + P_ReturnThrustY(mobj, mobj->angle+ANG90, 24*FRACUNIT), mobj->z + 32*FRACUNIT, MT_JETFUME1);
		filler->target = mobj;
		filler->fuse = 58;

		mobj->tracer = filler;
	}

    if (!mobj->target
        || !(mobj->target->flags&MF_SHOOTABLE))
    {
		if(mobj->health <= 0)
		{
			// look for a new target
			if(P_Look4Players(mobj,true))
			{
				if(mobj->info->mass != 0) // Bid farewell!
					S_StartSound(mobj, mobj->info->mass);
			}
			return;
		}

        // look for a new target
        if(P_Look4Players(mobj,true))
			S_StartSound(mobj, mobj->info->seesound);

        return;
    }

	if(mobj->state == &states[mobj->info->spawnstate])
		A_Boss1Chase(mobj);

	if(mobj->state == &states[mobj->info->meleestate]
		|| mobj->state == &states[mobj->info->missilestate] && mobj->health > mobj->info->damage)
		mobj->angle = R_PointToAngle2(mobj->x, mobj->y, mobj->target->x, mobj->target->y);
}

void A_Boss2Chase(mobj_t* actor);
void A_Boss2Pogo(mobj_t* actor);
// AI for the second boss.
// No, it does NOT convert "Boss" to a "Thinker". =P
// Tails 11-02-2002
void P_Boss2Thinker(mobj_t* mobj)
{
	if(mobj->movecount)
		mobj->movecount--;

	if (!(mobj->movecount)) 
		mobj->flags2 &= ~MF2_FRET;

	if(!mobj->tracer && cv_gametype.value != GT_CHAOS)
	{
		mobj_t* filler;

		// Spawn the jets here
		filler = P_SpawnMobj(mobj->x, mobj->y, mobj->z - 15*FRACUNIT, MT_JETFUME2);
		filler->target = mobj;

		mobj->tracer = filler;
	}

    if (!mobj->target
        || !(mobj->target->flags&MF_SHOOTABLE))
    {
		if(mobj->health <= 0)
		{
			// look for a new target
			if(P_Look4Players(mobj,true))
			{
				if(mobj->info->mass != 0) // Bid farewell!
					S_StartSound(mobj, mobj->info->mass);
			}
			return;
		}

        // look for a new target
        if(P_Look4Players(mobj,true))
			S_StartSound(mobj, mobj->info->seesound);

        return;
    }

	if(cv_gametype.value == GT_CHAOS && (mobj->state == &states[S_EGGMOBILE2_POGO1] || mobj->state == &states[S_EGGMOBILE2_POGO2]
			|| mobj->state == &states[S_EGGMOBILE2_POGO3] || mobj->state == &states[S_EGGMOBILE2_POGO4]
			|| mobj->state == &states[S_EGGMOBILE2_STND])) // Chaos mode, he pogos only
	{
		mobj->flags &= ~MF_NOGRAVITY;
		A_Boss2Pogo(mobj);
	}
	else if (cv_gametype.value != GT_CHAOS)
	{
		if(mobj->state == &states[mobj->info->spawnstate] && mobj->health > mobj->info->damage)
		{
			A_Boss2Chase(mobj);
		}
		else if(mobj->state == &states[mobj->info->raisestate] || mobj->state == &states[S_EGGMOBILE2_POGO2]
			|| mobj->state == &states[S_EGGMOBILE2_POGO3] || mobj->state == &states[S_EGGMOBILE2_POGO4]
			|| mobj->state == &states[mobj->info->spawnstate])
		{
			mobj->flags &= ~MF_NOGRAVITY;
			A_Boss2Pogo(mobj);
		}
	}
}

// Fun function stuff to make NiGHTS hoops!
// Thanks a TON, Hurdler!
typedef float TVector[4];
typedef float TMatrix[4][4];
const double deg2rad = 0.017453293;

TVector *VectorMatrixMultiply(TVector v, TMatrix m)
{
    static TVector ret;

    ret[0] = v[0]*m[0][0] + v[1]*m[1][0] + v[2]*m[2][0] + 
v[3]*m[3][0];
    ret[1] = v[0]*m[0][1] + v[1]*m[1][1] + v[2]*m[2][1] + 
v[3]*m[3][1];
    ret[2] = v[0]*m[0][2] + v[1]*m[1][2] + v[2]*m[2][2] + 
v[3]*m[3][2];
    ret[3] = v[0]*m[0][3] + v[1]*m[1][3] + v[2]*m[2][3] + 
v[3]*m[3][3];

    return &ret;
}

//ok, now, here is how to compute the transformation regarding the 
//tilt:
TMatrix *RotateXMatrix(float rad)
{
    static TMatrix ret;

    ret[0][0] = 1; ret[0][1] = 0;          ret[0][2] = 0;         
ret[0][3] = 0;
    ret[1][0] = 0; ret[1][1] = cos(rad);   ret[1][2] = sin(rad);  
ret[1][3] = 0;
    ret[2][0] = 0; ret[2][1] = -sin(rad);  ret[2][2] = cos(rad);  
ret[2][3] = 0;
    ret[3][0] = 0; ret[3][1] = 0;          ret[3][2] = 0;         
ret[3][3] = 1;

    return &ret;
}

TMatrix *RotateYMatrix(float rad)
{
    static TMatrix ret;

    ret[0][0] = cos(rad);  ret[0][1] = 0; ret[0][2] = -sin(rad);  
ret[0][3] = 0;
    ret[1][0] = 0;         ret[1][1] = 1; ret[1][2] = 0;          
ret[1][3] = 0;
    ret[2][0] = sin(rad);  ret[2][1] = 0; ret[2][2] = cos(rad);   
ret[2][3] = 0;
    ret[3][0] = 0;         ret[3][1] = 0; ret[3][2] = 0;          
ret[3][3] = 1;

    return &ret;
}

TMatrix *RotateZMatrix(float rad)
{
    static TMatrix ret;

    ret[0][0] = cos(rad);   ret[0][1] = sin(rad);  ret[0][2] = 0; 
ret[0][3] = 0;
    ret[1][0] = -sin(rad);  ret[1][1] = cos(rad);  ret[1][2] = 0; 
ret[1][3] = 0;
    ret[2][0] = 0;          ret[2][1] = 0;         ret[2][2] = 1; 
ret[2][3] = 0;
    ret[3][0] = 0;          ret[3][1] = 0;         ret[3][2] = 0; 
ret[3][3] = 1;

    return &ret;
}

//Hurdler: some new math functions
fixed_t double2fixed(double t)
{
    double fl = floor(t);
    return ((int)fl << 16) | (int)((t-fl)*65536.0);
}

mobj_t* P_GetClosestAxis(mobj_t* source);

degenmobj_t P_GimmeAxisXYPos(mobj_t* closestaxis, fixed_t x, fixed_t y)
{
	degenmobj_t mobj;
	double angle_pos;

	mobj.x = x;
	mobj.y = y;

	angle_pos = atan2((mobj.y - closestaxis->y), (mobj.x - closestaxis->x))/deg2rad;
	if(angle_pos < 0.0)
	{
		angle_pos += 360.0;
	}
	else if(angle_pos >= 360.0)
	{
		angle_pos -= 360.0;
	}

	mobj.x = closestaxis->x + cos(angle_pos * deg2rad) * closestaxis->info->radius;
	mobj.y = closestaxis->y + sin(angle_pos * deg2rad) * closestaxis->info->radius;

	return mobj;
}

void P_MoveHoop(mobj_t* mobj)
{
	TVector v;
	TVector *res;
	fixed_t finalx, finaly, finalz;
	fixed_t mthingx, mthingy, mthingz;

	mthingx = mobj->target->x;
	mthingy = mobj->target->y;
	mthingz = mobj->target->z+mobj->target->height/2;

	// Make the sprite travel towards the center of the hoop
	v[0] = cos(mobj->movedir*11.25*deg2rad) * (mobj->fuse * 8);
	v[1] = 0;
	v[2] = sin(mobj->movedir*11.25*deg2rad) * (mobj->fuse * 8);
	v[3] = 1;

	res = VectorMatrixMultiply(v, *RotateXMatrix(mobj->target->movedir*deg2rad));
	memcpy(&v, res, sizeof(v));
	res = VectorMatrixMultiply(v, *RotateZMatrix(mobj->target->movecount*deg2rad));
	memcpy(&v, res, sizeof(v));

	finalx = mthingx + double2fixed(v[0]);
	finaly = mthingy + double2fixed(v[1]);
	finalz = mthingz + double2fixed(v[2]);

	P_UnsetThingPosition(mobj);
	mobj->x = finalx;
	mobj->y = finaly;
	P_SetThingPosition(mobj);
	mobj->z = finalz - mobj->height/2;
}

void P_SpawnHoopOfSomething(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, int number, mobjtype_t type, int rotangle)
{
	mobj_t* mobj;
	int i;
	TVector v;
	TVector *res;
	fixed_t finalx, finaly, finalz;
	degenmobj_t hoopcenter;
	mobj_t* axis;
	short closestangle;
	degenmobj_t xypos;
	double degrees;

	hoopcenter.x = x;
	hoopcenter.y = y;
	hoopcenter.z = z;

	axis = P_GetClosestAxis((mobj_t*)&hoopcenter);

	if(axis == NULL)
	{
		CONS_Printf("You forgot to put axis points in the map!\n");
		return;
	}

	xypos = P_GimmeAxisXYPos(axis, x, y);

	x = xypos.x;
	y = xypos.y;

	hoopcenter.z = z - mobjinfo[type].height/2;

	hoopcenter.x = x;
	hoopcenter.y = y;

	closestangle = R_PointToAngle2(x, y, axis->x, axis->y)/ANGLE_1;

	degrees = 360.0/number;

	radius >>= FRACBITS;

	// Create the hoop!
	for(i = 0; i<number; i++)
	{
		v[0] = cos(i*(degrees)*deg2rad) * radius;
		v[1] = 0;
		v[2] = sin(i*(degrees)*deg2rad) * radius;
		v[3] = 1;

		res = VectorMatrixMultiply(v, *RotateXMatrix(rotangle*deg2rad));
		memcpy(&v, res, sizeof(v));
		res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
		memcpy(&v, res, sizeof(v));

		finalx = x + double2fixed(v[0]);
		finaly = y + double2fixed(v[1]);
		finalz = z + double2fixed(v[2]);

		mobj = P_SpawnMobj(finalx, finaly, finalz, type);
		mobj->z -= mobj->height/2;
	}
}
void P_SpawnParaloop(fixed_t x, fixed_t y, fixed_t z, fixed_t radius, int number, mobjtype_t type, int rotangle)
{
	mobj_t* mobj;
	int i;
	TVector v;
	TVector *res;
	fixed_t finalx, finaly, finalz;
	degenmobj_t hoopcenter;
	mobj_t* axis;
	short closestangle;
	degenmobj_t xypos;
	double degrees;
	fixed_t dist;

	hoopcenter.x = x;
	hoopcenter.y = y;
	hoopcenter.z = z;

	axis = P_GetClosestAxis((mobj_t*)&hoopcenter);

	if(axis == NULL)
	{
		CONS_Printf("You forgot to put axis points in the map!\n");
		return;
	}

	xypos = P_GimmeAxisXYPos(axis, x, y);

	x = xypos.x;
	y = xypos.y;

	hoopcenter.z = z - mobjinfo[type].height/2;

	hoopcenter.x = x;
	hoopcenter.y = y;

	closestangle = R_PointToAngle2(x, y, axis->x, axis->y)/ANGLE_1;

	degrees = 360.0/number;

	radius >>= FRACBITS;
	radius *= 0.8;

	// Create the hoop!
	for(i = 0; i<number; i++)
	{
		v[0] = cos(i*(degrees)*deg2rad) * radius;
		v[1] = 0;
		v[2] = sin(i*(degrees)*deg2rad) * radius;
		v[3] = 1;

		res = VectorMatrixMultiply(v, *RotateXMatrix(rotangle*deg2rad));
		memcpy(&v, res, sizeof(v));
		res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
		memcpy(&v, res, sizeof(v));

		finalx = x + double2fixed(v[0]);
		finaly = y + double2fixed(v[1]);
		finalz = z + double2fixed(v[2]);

		mobj = P_SpawnMobj(finalx, finaly, finalz, type);
		mobj->z -= mobj->height/2;

		// change angle
		mobj->angle = R_PointToAngle2(mobj->x, mobj->y, x, y);

		// change slope
		dist = P_AproxDistance(P_AproxDistance(x - mobj->x, y - mobj->y), z - mobj->z);

		if (dist < 1)
			dist = 1;

		mobj->momx = FixedMul(FixedDiv(x - mobj->x, dist), 5*FRACUNIT);
		mobj->momy = FixedMul(FixedDiv(y - mobj->y, dist), 5*FRACUNIT);
		mobj->momz = FixedMul(FixedDiv(z - mobj->z, dist), 5*FRACUNIT);
		mobj->fuse = radius/4 + 1;

		if(mobj->fuse <= 1)
			mobj->fuse = 2;

		mobj->flags |= MF_NOCLIPTHING;
		mobj->flags &= ~MF_SPECIAL;

		if(mobj->fuse > 7)
			mobj->tics = mobj->fuse - 7;
		else
			mobj->tics = 1;
	}
}

// Returns true if a boss with health is in the level.
// Used for Chaos mode
// Tails 12-19-2002
boolean P_BossDoesntExist(void)
{
	thinker_t*  th;
	mobj_t*     mo2;

	// scan the thinkers
	// to find the closest axis point
	for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
	{
		if (th->function.acp1 != (actionf_p1)P_MobjThinker)
			continue;

		mo2 = (mobj_t *)th;

		if((mo2->flags & MF_BOSS)
			&& mo2->health)
			return false;
	}

	// No boss found!
	return true;
}

extern consvar_t cv_chaos_bluecrawla;
extern consvar_t cv_chaos_redcrawla;
extern consvar_t cv_chaos_crawlacommander;
extern consvar_t cv_chaos_jettysynbomber;
extern consvar_t cv_chaos_jettysyngunner;
extern consvar_t cv_chaos_eggmobile1;
extern consvar_t cv_chaos_eggmobile2;
extern consvar_t cv_chaos_spawnrate;
extern consvar_t cv_blueshield;
extern consvar_t cv_yellowshield;
extern consvar_t cv_greenshield;
extern consvar_t cv_redshield;
extern consvar_t cv_blackshield;
extern consvar_t cv_teleporters;
extern consvar_t cv_superring;
extern consvar_t cv_silverring;
extern consvar_t cv_1up;
extern consvar_t cv_eggmanbox;
extern consvar_t cv_supersneakers;
extern consvar_t cv_invincibility;
void A_NightsItemChase(mobj_t* actor);
boolean A_BlueLook(mobj_t* actor);
boolean A_RedLook(mobj_t* actor);
boolean A_YellowLook(mobj_t* actor);
boolean A_BlackLook(mobj_t* actor);
boolean A_GreenLook(mobj_t* actor);

void P_ShieldPos ( mobj_t*       thing )
{
    fixed_t tempfloorz;
	fixed_t tempceilingz;

	if(!thing->target)
		return;

    // The base floor / ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tempfloorz = thing->subsector->sector->floorheight;
    tempceilingz = thing->subsector->sector->ceilingheight;

    //SoM: 3/23/2000: Check list of fake floors and see if
    //tmfloorz/tmceilingz need to be altered.
    if(thing->subsector->sector->ffloors)
    {
      ffloor_t*  rover;
      fixed_t    delta1;
      fixed_t    delta2;
      int        thingtop = thing->z + thing->height;

      for(rover = thing->subsector->sector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS)) continue;

        delta1 = thing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > tempfloorz && abs(delta1) < abs(delta2))
          tempfloorz = *rover->topheight;
        if(*rover->bottomheight < tempceilingz && abs(delta1) >= abs(delta2))
		{
			if(!(rover->flags & FF_PLATFORM))
				tempceilingz = *rover->bottomheight;
		}
      }
    }

	thing->floorz = tempfloorz;
	thing->ceilingz = tempceilingz;
}

void A_BossDeath(mobj_t* mo);
// AI for the Koopa boss.
void P_KoopaThinker(mobj_t* koopa)
{
	P_MobjCheckWater(koopa);

	if(koopa->watertop > koopa->z + koopa->height + 128*FRACUNIT && koopa->health > 0)
	{
		A_BossDeath(koopa);
		P_SetMobjState(koopa, S_DISS);
		koopa->health = 0;
		return;
	}

	// Koopa moves ONLY on the X axis!
	if(koopa->threshold > 0)
	{
		koopa->threshold--;
		koopa->momx = FRACUNIT;

		if(koopa->threshold == 0)
			koopa->threshold = -TICRATE*2;
	}
	else if(koopa->threshold < 0)
	{
		koopa->threshold++;
		koopa->momx = -FRACUNIT;

		if(koopa->threshold == 0)
			koopa->threshold = TICRATE*2;
	}
	else
		koopa->threshold = TICRATE*2;

	P_XYMovement(koopa);

	if(P_Random() < 8 && koopa->z <= koopa->floorz)
		koopa->momz = 5*FRACUNIT;

	if(koopa->z > koopa->floorz)
		koopa->momz += FRACUNIT/4;

	if(P_Random() < 4)
	{
		mobj_t* flame;
		flame = P_SpawnMobj(koopa->x - koopa->radius + 5*FRACUNIT, koopa->y, koopa->z + (P_Random() >> FRACBITS)/4, MT_KOOPAFLAME);
		if(!flame)
			return;
		flame->momx = -flame->info->speed;
		S_StartSound(flame, sfx_koopfr);
	}
	else if(P_Random() > 250 && gameskill >= sk_hard)
	{
		mobj_t* hammer;
		hammer = P_SpawnMobj(koopa->x - koopa->radius, koopa->y, koopa->z + koopa->height, MT_HAMMER);
		if(!hammer)
			return;
		hammer->momx = -5*FRACUNIT;
		hammer->momz = 7*FRACUNIT;
	}
}

boolean P_CheckHoopPosition(mobj_t* hoopthing, fixed_t x, fixed_t y, fixed_t z, fixed_t radius);
//
// P_MobjThinker
//
void P_MobjThinker (mobj_t* mobj)
{
	if(mobj->flags2 & MF2_NOTHINK)
		return;

	// Special thinker for scenery objects Tails 07-24-2002
	if(mobj->flags & MF_SCENERY)
	{
		switch(mobj->type)
		{
			case MT_BLUEORB:
				if(A_BlueLook(mobj))
					P_ShieldPos(mobj);
				else
					return;
				break;
			case MT_BLACKORB:
				if(A_BlackLook(mobj))
					P_ShieldPos(mobj);
				else
					return;
				break;
			case MT_REDORB:
				if(A_RedLook(mobj))
					P_ShieldPos(mobj);
				else
					return;
				break;
			case MT_YELLOWORB:
				if(A_YellowLook(mobj))
					P_ShieldPos(mobj);
				else
					return;
				break;
			case MT_GREENORB:
				if(A_GreenLook(mobj))
					P_ShieldPos(mobj);
				else
					return;
				break;
			default:
				break;
		}

		P_SceneryCheckWater(mobj);
		P_SceneryThinker(mobj);

		switch(mobj->type)
		{
			case MT_SMALLBUBBLE:
			case MT_MEDIUMBUBBLE:
			case MT_EXTRALARGEBUBBLE:	// start bubble dissipate Tails
				if(!(mobj->eflags & MF_UNDERWATER)
					|| mobj->z+mobj->height >= mobj->ceilingz)
				{
					int random;

					P_SetMobjState (mobj, S_DISS);

					if(mobj->threshold == 42) // Don't make pop sound.
						break;

					random = P_Random();

					if(random <= 51)
						S_StartSound(mobj, sfx_bubbl1);
					else if (random <= 102)
						S_StartSound(mobj, sfx_bubbl2);
					else if (random <= 153)
						S_StartSound(mobj, sfx_bubbl3);
					else if (random <= 204)
						S_StartSound(mobj, sfx_bubbl4);
					else
						S_StartSound(mobj, sfx_bubbl5);
				}
				break;
			case MT_SIGN: // Start Level end sign stuff Tails 01-14-2001
				if(players[consoleplayer].exiting)
				{
					if (mobj->state == &states[S_SIGN52])
					{
						P_SetMobjState (mobj, S_SIGN1);
						S_StartSound(mobj, sfx_lvpass);
					}
				}
				break;
			case MT_ZERO:
			case MT_ONE:
			case MT_TWO:
			case MT_THREE:
			case MT_FOUR:
			case MT_FIVE:
					if(!mobj->target)
					{
						P_SetMobjState(mobj, S_DISS);
						break;
					}
					if(!mobj->target->player || !(mobj->target->player->powers[pw_underwater] || mobj->target->player->powers[pw_spacetime]))
					{
						P_SetMobjState(mobj, S_DISS);
						break;
					}
					mobj->x = mobj->target->x;
					mobj->y = mobj->target->y;
					mobj->z = mobj->target->z + (mobj->target->height) + 8*FRACUNIT; // Adjust height for height changes
					if(mobj->threshold <= 35)
						mobj->flags2 |= MF2_DONTDRAW;
					else
						mobj->flags2 &= ~MF2_DONTDRAW;
					if(mobj->threshold <= 30)
						mobj->threshold = 40;
					mobj->threshold--;
					break;
			default:
				break;
		}
		return;
	}

switch(mobj->type)
{
	case MT_HOOP:
		if(mobj->fuse > 1)
		{
			P_MoveHoop(mobj);
		}
		else if(mobj->fuse == 1)
		{
			mobj->movecount = 1;
		}
		if(mobj->movecount)
		{
			mobj->fuse++;

			if(mobj->fuse > 32)
				P_RemoveMobj(mobj);
		}
		else
			mobj->fuse--;
		return;
		break;
	case MT_HOOPCOLLIDE:
		// The hoop finds YOU, not vice versa.
		P_CheckHoopPosition (mobj, mobj->x, mobj->y, mobj->z+(mobj->height/2), mobj->radius);
		return;
		break;
	case MT_NIGHTSPARKLE:
		if (mobj->tics != -1)
		{
			mobj->tics--;

			// you can cycle through multiple states in a tic
			if (!mobj->tics)
				if (!P_SetMobjState (mobj, mobj->state->nextstate) )
					return;         // freed itself
		}
		if(mobj->flags & MF_SPECIAL)
			return;

		if ( mobj->momx || mobj->momy )
		{
			P_XYMovement (mobj);

			// FIXME: decent NOP/NULL/Nil function pointer please.
			if ((mobj->thinker.function.acv == (actionf_v) (-1)))
				return;             // mobj was removed
		}

		//added:28-02-98: always do the gravity bit now, that's simpler
		//                BUT CheckPosition only if wasn't done before.
		if ( mobj->momz )
		{
			P_ZMovement(mobj);
		}
		return;
		break;
	case MT_ROCKCRUMBLE:
		if(mobj->z <= mobj->floorz)
		{
			P_SetMobjState(mobj, mobj->info->deathstate);
			return;
		}
		break;
	case MT_SUPERTRANS:
		if(mobj->state == &states[S_NIGHTSDRONE1])
		{
			mobj->health = 0;
			P_SetMobjState(mobj, S_DISS);
		}
		break;
	case MT_SPINFIRE:
		mobj->z = mobj->floorz+1;
		break;
	case MT_EGGCAPSULE:
	case MT_HAMMER:
		if(mobj->z <= mobj->floorz)
			P_SetMobjState(mobj,S_DISS);
		break;
	case MT_KOOPA:
		P_KoopaThinker(mobj);
		break;
	case MT_STREETLIGHT:
		if(mobj->z + mobj->height != mobj->ceilingz && leveltime > 0)
		{
			mobj->momz = FRACUNIT;
			P_ZMovement(mobj);
			P_CheckPosition(mobj, mobj->x, mobj->y);
			mobj->z = mobj->ceilingz - mobj->height;
		}
		return;
		break;
	case MT_REDRING:
		if(((mobj->z < mobj->floorz) || (mobj->z + mobj->height > mobj->ceilingz)) && mobj->flags & MF_MISSILE)
			P_ExplodeMissile (mobj);
		break;
	case MT_BOSSFLYPOINT:
		return;
		break;
	case MT_NIGHTSCORE:
		mobj->flags =  (mobj->flags & ~MF_TRANSLATION)
						 | ((leveltime % 13)<<MF_TRANSSHIFT);
		break;
	case MT_JETFUME1:
		{
			fixed_t jetx, jety;

			if(!mobj->target)
			{
				P_RemoveMobj(mobj);
				return;
			}

			if(cv_gametype.value == GT_CHAOS && mobj->target->health <= 0)
				P_SetMobjState(mobj, S_DISS);

			jetx = mobj->target->x + P_ReturnThrustX(mobj->target, mobj->target->angle, -56*FRACUNIT);
			jety = mobj->target->y + P_ReturnThrustY(mobj->target, mobj->target->angle, -56*FRACUNIT);

			if(mobj->fuse == 56) // First one
			{
				P_UnsetThingPosition(mobj);
				mobj->x = jetx;
				mobj->y = jety;
				mobj->z = mobj->target->z + 8*FRACUNIT;
				P_SetThingPosition(mobj);
			}
			else if(mobj->fuse == 57)
			{
				P_UnsetThingPosition(mobj);
				mobj->x = jetx + P_ReturnThrustX(mobj->target, mobj->target->angle-ANG90, 24*FRACUNIT);
				mobj->y = jety + P_ReturnThrustY(mobj->target, mobj->target->angle-ANG90, 24*FRACUNIT);
				mobj->z = mobj->target->z + 32*FRACUNIT;
				P_SetThingPosition(mobj);
			}
			else if(mobj->fuse == 58)
			{
				P_UnsetThingPosition(mobj);
				mobj->x = jetx + P_ReturnThrustX(mobj->target, mobj->target->angle+ANG90, 24*FRACUNIT);
				mobj->y = jety + P_ReturnThrustY(mobj->target, mobj->target->angle+ANG90, 24*FRACUNIT);
				mobj->z = mobj->target->z + 32*FRACUNIT;
				P_SetThingPosition(mobj);
			}
			mobj->fuse++;
		}
		break;
	case MT_JETFUME2:
		if(!mobj->target)
		{
			P_RemoveMobj(mobj);
			return;
		}
		if(cv_gametype.value == GT_CHAOS)
			P_SetMobjState(mobj, S_DISS);
		P_UnsetThingPosition(mobj);
		mobj->x = mobj->target->x;
		mobj->y = mobj->target->y;
		mobj->z = mobj->target->z - 15*FRACUNIT;
		if(mobj->target->health < 3)
			mobj->flags |= MF_NOSECTOR;
		if(mobj->target->health <= 0)
			mobj->flags &= ~MF_NOSECTOR;
		P_SetThingPosition(mobj);
		break;
	case MT_SEED:
		mobj->momz = mobj->info->speed;
		break;
	case MT_NIGHTSDRONE:
		if(mobj->tracer && mobj->tracer->player && !mobj->tracer->player->nightsmode)
			mobj->flags2 &= ~MF2_DONTDRAW;
		mobj->angle += ANGLE_1*10;
		if(mobj->z <= mobj->floorz)
			mobj->momz = 5*FRACUNIT;
		break;
	case MT_LASER:
		mobj->flags &= ~MF_NOBLOCKMAP;
		break;
	case MT_PLAYER:
		P_PlayerMobjThinker(mobj);
		return;
		break;
	case MT_FAN: // Fans spawn bubbles underwater Tails 02-28-2001
		// check mobj against possible water content, before movement code
		P_MobjCheckWater (mobj);
		if(mobj->eflags & MF_UNDERWATER)
		{
			if(!(P_Random() % 16))
			{
				P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_SMALLBUBBLE);
			}
			else if(!(P_Random() % 96))
			{
				P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_MEDIUMBUBBLE);
			}
		}
		break;
	case MT_EGGMOBILE:
		if(mobj->health < (mobj->info->damage+1) && leveltime & 1 && mobj->health > 0)
			P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_SMOK);
		if(mobj->flags2 & MF2_SKULLFLY)
		{
			mobj_t* spawnmobj;
			spawnmobj = P_SpawnMobj(mobj->x, mobj->y, mobj->z, mobj->info->painchance);
			spawnmobj->flags = (spawnmobj->flags & ~MF_TRANSLATION)
								| ((1)<<MF_TRANSSHIFT); // Tails 08-20-2002
		}
		P_Boss1Thinker(mobj);
		if(mobj->flags2 & MF2_BOSSFLEE)
			P_InstaThrust(mobj, mobj->angle, 12*FRACUNIT);
		break;
	case MT_EGGMOBILE2:
		P_Boss2Thinker(mobj);
		if(mobj->flags2 & MF2_BOSSFLEE)
			P_InstaThrust(mobj, mobj->angle, 12*FRACUNIT);
		break;
	case MT_SKIM:
		// check mobj against possible water content, before movement code
		P_MobjCheckWater (mobj);
		// Keep Skim at water surface Tails 06-13-2000
		if(mobj->z != mobj->watertop)
			mobj->z = mobj->watertop;
		break;
	case MT_RING:
	case MT_COIN:
		// No need to check water. Who cares?
		P_RingThinker(mobj);
		if(mobj->flags2 & MF2_NIGHTSPULL)
			A_NightsItemChase(mobj);
		return;
		break;
	case MT_NIGHTSWING:
		if(mobj->flags2 & MF2_NIGHTSPULL)
			A_NightsItemChase(mobj);
		break;
	case MT_SHELL:
		if(mobj->threshold > TICRATE)
			mobj->threshold--;

		if(mobj->state != &states[S_SHELL])
		{
			P_InstaThrust(mobj, mobj->angle, mobj->info->speed);
		}
		break;
	case MT_TURRET:
		P_MobjCheckWater (mobj);
		if((mobj->eflags & MF_UNDERWATER) && mobj->health > 0)
		{
			P_SetMobjState(mobj, mobj->info->deathstate);
			mobj->health = 0;
			mobj->flags2 &= ~MF2_FIRING;
		}
		break;
	case MT_GARGOYLE: // Graue 12-31-2003
	case MT_SNOWMAN: // Graue 12-31-2003:
		if(!(mobj->flags2 & MF2_SLIDEPUSH) || !(mobj->flags & MF_PUSHABLE) || cv_gametype.value != GT_CTF)
			break;

		{
			extern consvar_t cv_ctf_scoring, cv_fraglimit;
			int goaltype = 0;
	
			if(mobj->subsector->sector->special == 994) goaltype = 1; // Red Team's Goal
			else if(mobj->subsector->sector->special == 995) goaltype = 2; // Blue Team's Goal
			else {
				int spec;

				spec = P_ThingOnSpecial3DFloor(mobj);
				if(spec == 994) goaltype = 1; // Red Team's Goal
				if(spec == 995) goaltype = 2; // Blue Team's Goal
			}

			if(!goaltype)
				break;

			if(goaltype == 1)
			{
				// Blue team scores a point, or two, or three, or 1000...
				if(cv_ctf_scoring.value == 1)
					bluescore += 1;
				else
					bluescore += 1000;

				if(cv_fraglimit.value)
					P_CheckFragLimit(NULL); // Safe because only team scores are checked

				CONS_Printf("Blue team scored a point.\n");
			}
			else // goaltype == 2
			{
				// Red team scores a point, or two, or three, or 1000...
				if(cv_ctf_scoring.value == 1)
					redscore += 1;
				else
					redscore += 1000;

				if(cv_fraglimit.value)
					P_CheckFragLimit(NULL); // Safe because only team scores are checked

				CONS_Printf("Red team scored a point.\n");
			}

			mobj->flags &= ~MF_PUSHABLE;
			mobj->fuse = 2; // Makes it disappear and respawn at its first spawnpoint later on
		}
	default:
		// check mobj against possible water content, before movement code
		P_MobjCheckWater (mobj);
		break;
}

if(mobj->flags2 & MF2_FIRING && mobj->target)
{
	if(mobj->health > 0 && (leveltime & 1)) // Fire mode
	{
		mobj->angle = R_PointToAngle2(mobj->x, mobj->y, mobj->target->x, mobj->target->y);
		S_StartSound(P_SpawnMissile (mobj, mobj->target, MT_TURRETLASER), sfx_trfire);
	}
	else if(mobj->health > 0)
		mobj->angle = R_PointToAngle2(mobj->x, mobj->y, mobj->target->x, mobj->target->y);
}

if(mobj->flags & MF_PUSHABLE)
{
	if (!( mobj->momx || mobj->momy))
		P_TryMove(mobj, mobj->x, mobj->y, true);
}

if(mobj->flags & MF_AMBIENT)
{
	if(leveltime % mobj->health == 0)
		S_StartSound(mobj, mobj->info->seesound);
	return;
}

    //SOM: Check fuse
    if(mobj->fuse) {
      mobj->fuse--;
      if(!mobj->fuse) { // Well, whaddaya know, the fuse ran out! Hehehe... Graue 12-31-2003

		  subsector_t* ss;
		  fixed_t             x;
		  fixed_t             y;
		  fixed_t             z;
		  mobj_t*			flagmo;

		switch(mobj->type)
		{
			int random; // Tails 08-09-2001

			case MT_GARGOYLE: // Graue 12-31-2003
			case MT_SNOWMAN: // Graue 12-31-2003
				x = mobj->spawnpoint->x << FRACBITS;
				y = mobj->spawnpoint->y << FRACBITS;
				ss = R_PointInSubsector(x, y);
				z = ss->sector->floorheight;
				if(mobj->spawnpoint->z != 0)
					z += mobj->spawnpoint->z << FRACBITS;
				flagmo = P_SpawnMobj(x, y, z, mobj->type);
				flagmo->spawnpoint = mobj->spawnpoint;
				P_SetMobjState(mobj, S_DISS);
				flagmo->flags = mobj->flags; // Graue 12-31-2003
				flagmo->flags2 = mobj->flags2;
				break;
			case MT_BLUEFLAG:
			case MT_REDFLAG:
				x = mobj->spawnpoint->x << FRACBITS;
				y = mobj->spawnpoint->y << FRACBITS;
				ss = R_PointInSubsector(x, y);
				z = ss->sector->floorheight;
				if(mobj->spawnpoint->z != 0)
					z += mobj->spawnpoint->z << FRACBITS;
				flagmo = P_SpawnMobj(x, y, z, mobj->type);
				flagmo->spawnpoint = mobj->spawnpoint;
				P_SetMobjState(mobj, S_DISS);
				break;
			case MT_BLUETV: // Blue shield box
			case MT_YELLOWTV: // Yellow shield box
			case MT_REDTV: // Red shield box
			case MT_GREENTV: // Green shield box
			case MT_BLACKTV: // Black shield box
			case MT_SNEAKERTV: // Super Sneaker box
			case MT_SUPERRINGBOX: // 10-Ring box
			case MT_GREYRINGBOX: // 25-Ring box
			case MT_INV: // Invincibility box
			case MT_MIXUPBOX: // Teleporter Mixup box
			case MT_PRUP: // 1up!
			case MT_EGGMANBOX:
				P_SetMobjState(mobj, S_DISS); // make sure they dissapear Tails

				if(mobj->flags & MF_AMBUSH)
				{
					mobjtype_t spawnchance[30];
					int i = 0;
					int oldi = 0;
					int numchoices = 0;

					random = P_Random(); // Gotta love those random numbers!

					if(cv_superring.value)
					{
						oldi = i;
						for(; i < oldi + cv_superring.value; i++)
						{
							spawnchance[i] = MT_SUPERRINGBOX;
							numchoices++;
						}
					}
					if(cv_silverring.value)
					{
						oldi = i;
						for(; i < oldi + cv_silverring.value; i++)
						{
							spawnchance[i] = MT_GREYRINGBOX;
							numchoices++;
						}
					}
					if(cv_supersneakers.value)
					{
						oldi = i;
						for(; i < oldi + cv_supersneakers.value; i++)
						{
							spawnchance[i] = MT_SNEAKERTV;
							numchoices++;
						}
					}
					if(cv_invincibility.value)
					{
						oldi = i;
						for(; i < oldi + cv_invincibility.value; i++)
						{
							spawnchance[i] = MT_INV;
							numchoices++;
						}
					}
					if(cv_blueshield.value)
					{
						oldi = i;
						for(; i < oldi + cv_blueshield.value; i++)
						{
							spawnchance[i] = MT_BLUETV;
							numchoices++;
						}
					}
					if(cv_greenshield.value)
					{
						oldi = i;
						for(; i < oldi + cv_greenshield.value; i++)
						{
							spawnchance[i] = MT_GREENTV;
							numchoices++;
						}
					}
					if(cv_yellowshield.value)
					{
						oldi = i;
						for(; i < oldi + cv_yellowshield.value; i++)
						{
							spawnchance[i] = MT_YELLOWTV;
							numchoices++;
						}
					}
					if(cv_redshield.value)
					{
						oldi = i;
						for(; i < oldi + cv_redshield.value; i++)
						{
							spawnchance[i] = MT_REDTV;
							numchoices++;
						}
					}
					if(cv_blackshield.value)
					{
						oldi = i;
						for(; i < oldi + cv_blackshield.value; i++)
						{
							spawnchance[i] = MT_BLACKTV;
							numchoices++;
						}
					}
					if(cv_1up.value && cv_gametype.value == GT_RACE)
					{
						oldi = i;
						for(; i < oldi + cv_1up.value; i++)
						{
							spawnchance[i] = MT_PRUP;
							numchoices++;
						}
					}
/*					if(cv_eggmanbox.value)
					{
						oldi = i;
						for(; i < oldi + cv_eggmanbox.value; i++)
						{
							spawnchance[i] = MT_EGGMANBOX;
							numchoices++;
						}
					}*/
					if(cv_teleporters.value)
					{
						oldi = i;
						for(; i < oldi + cv_teleporters.value; i++)
						{
							spawnchance[i] = MT_MIXUPBOX;
							numchoices++;
						}
					}

					if(!(cv_1up.value || cv_blackshield.value || cv_redshield.value
						|| cv_yellowshield.value || cv_greenshield.value || cv_blueshield.value
						|| cv_invincibility.value || cv_supersneakers.value || cv_silverring.value
						|| cv_superring.value || /*cv_eggmanbox.value || */cv_teleporters.value))
						CONS_Printf("Note: All monitors turned off.\n");
					else
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, spawnchance[random%numchoices])->flags |= MF_AMBUSH;
				}
				else
				{
					P_SpawnMobj(mobj->x, mobj->y, mobj->z, mobj->type);
				}
				break;
			case MT_QUESTIONBOX:
				P_SetMobjState(mobj, S_DISS);
				P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_QUESTIONBOX);
				break;
			case MT_EGGTRAP: // Egg Capsule animal release
			{
				int i,j;
				fixed_t x, y, z;
				mobj_t* mo2;

				z = mobj->subsector->sector->floorheight + 64*FRACUNIT;
				for(j = 0; j<2; j++)
				{
					for(i = 0; i<32; i++)
					{
						x = mobj->x + sin(i*22.5) * 64 * FRACUNIT;
						y = mobj->y + cos(i*22.5) * 64 * FRACUNIT;
						
						mo2 = P_SpawnMobj(x, y, z, MT_EXPLODE);
						mo2->momx = sin(i*22.5) * 4 * FRACUNIT;
						mo2->momy = cos(i*22.5) * 4 * FRACUNIT;

						if(i&1)
						{
							P_SpawnMobj(x, y, z, MT_BIRD);
							S_StartSound(mo2, sfx_pop);
						}
						else if (i < 16)
							P_SpawnMobj(x, y, z, MT_SQRL);
						else
							P_SpawnMobj(x, y, z, MT_MOUSE);
					}
					z -= 32*FRACUNIT;
				}
				// Mark all players with the time to exit thingy!
				for(i=0; i<MAXPLAYERS; i++)
					players[i].exiting = 2.8*TICRATE+1;
			}
			case MT_CHAOSSPAWNER: // Chaos Mode spawner thingy
			{
				mobjtype_t enemy;
				mobjtype_t spawnchance[30];
				mobj_t* spawnedthing;
				int i = 0;
				int oldi = 0;
				int numchoices = 0;

				if(cv_gametype.value != GT_CHAOS)
					return;
				mobj->fuse = cv_chaos_spawnrate.value * TICRATE;
				random = P_Random(); // Gotta love those random numbers!

				if(cv_chaos_bluecrawla.value)
				{
					oldi = i;
					for(; i < oldi + cv_chaos_bluecrawla.value; i++)
					{
						spawnchance[i] = MT_BLUECRAWLA;
						numchoices++;
					}
				}
				if(cv_chaos_redcrawla.value)
				{
					oldi = i;
					for(; i < oldi + cv_chaos_redcrawla.value; i++)
					{
						spawnchance[i] = MT_REDCRAWLA;
						numchoices++;
					}
				}
				if(cv_chaos_crawlacommander.value)
				{
					oldi = i;
					for(; i < oldi + cv_chaos_crawlacommander.value; i++)
					{
						spawnchance[i] = MT_CRAWLACOMMANDER;
						numchoices++;
					}
				}
				if(cv_chaos_jettysynbomber.value)
				{
					oldi = i;
					for(; i < oldi + cv_chaos_jettysynbomber.value; i++)
					{
						spawnchance[i] = MT_JETTBOMBER;
						numchoices++;
					}
				}
				if(cv_chaos_jettysyngunner.value)
				{
					oldi = i;
					for(; i < oldi + cv_chaos_jettysyngunner.value; i++)
					{
						spawnchance[i] = MT_JETTGUNNER;
						numchoices++;
					}
				}
				if(cv_chaos_eggmobile1.value && P_BossDoesntExist())
				{
					oldi = i;
					for(; i < oldi + cv_chaos_eggmobile1.value; i++)
					{
						spawnchance[i] = MT_EGGMOBILE;
						numchoices++;
					}
				}
				if(cv_chaos_eggmobile2.value && P_BossDoesntExist())
				{
					oldi = i;
					for(; i < oldi + cv_chaos_eggmobile2.value; i++)
					{
						spawnchance[i] = MT_EGGMOBILE2;
						numchoices++;
					}
				}

				if((!cv_chaos_bluecrawla.value && !cv_chaos_redcrawla.value
					&& !cv_chaos_crawlacommander.value && !cv_chaos_jettysynbomber.value
					&& !cv_chaos_jettysyngunner.value && !cv_chaos_eggmobile1.value
					&& !cv_chaos_eggmobile2.value))
				{
					CONS_Printf("Geez, man. Turn some of the enemies on!\n");
					enemy = NUMMOBJTYPES+1;
				}
				else
				{
					if(numchoices > 0)
						enemy = spawnchance[random%numchoices];
				}

				if(enemy < NUMMOBJTYPES)
				{
					int i = 0;
					spawnedthing = P_SpawnMobj(mobj->x, mobj->y, mobj->z+32*FRACUNIT, enemy);
					P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_TFOG);

					if(P_SupermanLook4Players(spawnedthing) == false)
					{
						spawnedthing->target = players[0].mo;
						P_SetMobjState (spawnedthing, spawnedthing->info->seestate);
					}
					else
						P_SetMobjState (spawnedthing, spawnedthing->info->seestate);

					if(spawnedthing->flags & MF_BOSS)
					{
						spawnedthing->flags2 |= MF2_CHAOSBOSS;
						spawnedthing->momx = spawnedthing->momy = 0;
					}
				}
				break;
			}
			default:
				if(mobj->info->deathstate)
					P_ExplodeMissile(mobj);
				else
					P_SetMobjState(mobj, S_DISS); // make sure they dissapear tails
				break;
			}
        }
      }

    if ( mobj->momx ||
         mobj->momy ||
        (mobj->flags2&MF2_SKULLFLY) )
    {
        P_XYMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if ((mobj->thinker.function.acv == (actionf_v) (-1)))
            return;             // mobj was removed
    }

    //added:28-02-98: always do the gravity bit now, that's simpler
    //                BUT CheckPosition only if wasn't do before.
    if ((mobj->eflags & MF_ONGROUND)==0 || 
		(mobj->z != mobj->floorz) ||
          mobj->momz )
    {
		mobj_t *onmo;
        onmo = P_CheckOnmobj(mobj);
        if(!onmo)
        {
			P_ZMovement(mobj);
			P_CheckPosition (mobj, mobj->x, mobj->y); // Need this to pick up objects! Tails 02-01-2002
		}

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v) (-1))
            return;             // mobj was removed
    }
    else
        mobj->eflags &= ~MF_JUSTHITFLOOR;
/*
    // SoM: Floorhuggers stay on the floor allways...
    // BP: tested here but never set ?!
    if(mobj->info->flags & MF_FLOORHUGGER)
    {
      mobj->z = mobj->floorz;
    }
*/
    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic
        if (!mobj->tics)
            if (!P_SetMobjState (mobj, mobj->state->nextstate) )
                return;         // freed itself
    }
    else
    {
        // check for nightmare respawn (Not anymore! Tails)
        return;
    }

//	P_CheckPosition (mobj, mobj->x, mobj->y);
}

void A_ThrownRing(mobj_t* actor);
// Quick, optimized function for the Rail Rings Tails 07-12-2002
void P_RailThinker (mobj_t* mobj)
{
    //
    // momentum movement
    //
    if ( mobj->momx || mobj->momy )
    {
        P_XYMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if ((mobj->thinker.function.acv == (actionf_v) (-1)))
            return;             // mobj was removed
    }

    //added:28-02-98: always do the gravity bit now, that's simpler
    //                BUT CheckPosition only if wasn't do before.
    if ( (mobj->eflags & MF_ONGROUND)==0 ||
         (mobj->z != mobj->floorz) ||
          mobj->momz
       )
    {
		mobj_t *onmo;
        onmo = P_CheckOnmobj(mobj);
        if(!onmo)
        {
			P_ZMovement(mobj);
			P_CheckPosition (mobj, mobj->x, mobj->y); // Need this to pick up objects! Tails 02-01-2002
		}

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v) (-1))
            return;             // mobj was removed
    }
	if(mobj->flags2 & MF2_HOMING)
		A_ThrownRing(mobj);
}

// Quick, optimized function for scenery Tails 07-24-2002
void P_SceneryThinker (mobj_t* mobj)
{
	if(mobj->flags & MF_MONITORICON)
	{
		if(mobj->z < mobj->floorz + mobj->info->damage)
		{
			mobj->momz = mobj->info->speed;
		}
		else
			mobj->momz = 0;
	}
    //
    // momentum movement
    //
    if ( mobj->momx || mobj->momy )
    {
        P_SceneryXYMovement (mobj);

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if ((mobj->thinker.function.acv == (actionf_v) (-1)))
            return;             // mobj was removed
    }

    //added:28-02-98: always do the gravity bit now, that's simpler
    //                BUT CheckPosition only if wasn't do before.
    if ( (mobj->eflags & MF_ONGROUND)==0 ||
         (mobj->z != mobj->floorz) ||
          mobj->momz
       )
    {
		mobj_t *onmo;
        onmo = P_CheckOnmobj(mobj);
        if(!onmo)
        {
			P_SceneryZMovement(mobj);
			P_CheckPosition (mobj, mobj->x, mobj->y); // Need this to pick up objects! Tails 02-01-2002
		}

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if (mobj->thinker.function.acv == (actionf_v) (-1))
            return;             // mobj was removed
    }
    else
        mobj->eflags &= ~MF_JUSTHITFLOOR;

    // cycle through states,
    // calling action functions at transitions
    if (mobj->tics != -1)
    {
        mobj->tics--;

        // you can cycle through multiple states in a tic
        if (!mobj->tics)
            if (!P_SetMobjState (mobj, mobj->state->nextstate) )
                return;         // freed itself
    }
    else
    {
        // check for nightmare respawn (Not anymore! Tails)
        return;
    }
}

void P_MobjNullThinker (mobj_t* mobj)
{}

//
// P_SpawnMobj
//
mobj_t* P_SpawnMobj ( fixed_t       x,
                      fixed_t       y,
                      fixed_t       z,
                      mobjtype_t    type )
{
    mobj_t*     mobj;
    state_t*    st;
    mobjinfo_t* info;

    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));
    info = &mobjinfo[type];

    mobj->type = type;
    mobj->info = info;

    mobj->x = x;
    mobj->y = y;

	if(mobj->type == MT_RING || mobj->type == MT_COIN)
	{
		const double deg2rad = 0.017453293;
		if(mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS)
		{
			thinker_t*  th;
			mobj_t*     mo2;
			mobj_t*  closestaxis = NULL;
			unsigned short first = 0;
			double angle_pos;

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

						dist1 = R_PointToDist2(mobj->x, mobj->y, mo2->x, mo2->y)-mo2->info->radius;
						dist2 = R_PointToDist2(mobj->x, mobj->y, closestaxis->x, closestaxis->y)-closestaxis->info->radius;

						if(dist1 <= dist2)
							closestaxis = mo2;
					}
				}
			}

			if(closestaxis != NULL)
			{
				angle_pos = atan2((mobj->y - closestaxis->y), (mobj->x - closestaxis->x))/deg2rad;
				if(angle_pos < 0.0)
				{
					angle_pos += 360.0;
				}
				else if(angle_pos >= 360.0)
				{
					angle_pos -= 360.0;
				}

				mobj->x = closestaxis->x + cos(angle_pos * deg2rad) * closestaxis->info->radius;
				mobj->y = closestaxis->y + sin(angle_pos * deg2rad) * closestaxis->info->radius;
				mobj->angle = R_PointToAngle2(mobj->x, mobj->y, closestaxis->x, closestaxis->y);
			}
		}
	}

    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;

    mobj->health = info->spawnhealth;

    if (gameskill < sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    mobj->lastlook = -1;  // stuff moved in P_enemy.P_LookForPlayer

    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[info->spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame; // FF_FRAMEMASK for frame, and other bits..
    mobj->touching_sectorlist = NULL; //SoM: 4/7/2000
    mobj->friction = ORIG_FRICTION; //SoM: 4/7/2000

    // BP: SoM right ? if not ajust in p_saveg line 625 and 979
    mobj->movefactor = ORIG_FRICTION_FACTOR;

    // set subsector and/or block links
    P_SetThingPosition (mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    //added:27-02-98: if ONFLOORZ, stack the things one on another
    //                so they do not occupy the same 3d space
    //                allow for some funny thing arrangements!
    if (z == ONFLOORZ)
    {
        //if (!P_CheckPosition(mobj,x,y))
            // we could send a message to the console here, saying
            // "no place for spawned thing"...

        //added:28-02-98: defaults onground
        mobj->eflags |= MF_ONGROUND;

        //added:28-02-98: dirty hack : dont stack monsters coz it blocks
        //                moving floors and anyway whats the use of it?
        /*if (mobj->flags & MF_NOBLOOD)
        {
            mobj->z = mobj->floorz;

            // first check the tmfloorz
            P_CheckPosition(mobj,x,y);
            mobj->z = tmfloorz+FRACUNIT;

            // second check at the good z pos
            P_CheckPosition(mobj,x,y);

            mobj->floorz = tmfloorz;
            mobj->ceilingz = tmsectorceilingz;
            mobj->z = tmfloorz;
            // thing not on solid ground
            if (tmfloorthing)
                mobj->eflags &= ~MF_ONGROUND;

            //if (mobj->type == MT_BARREL)
            //   fprintf(stderr,"barrel at z %d floor %d ceiling %d\n",mobj->z,mobj->floorz,mobj->ceilingz);

        }
        else*/
		if(((mobj->type == MT_RING || mobj->type == MT_COIN) && mobj->flags & MF_AMBUSH)
			|| mobj->type == MT_DETON || mobj->type == MT_JETTBOMBER || mobj->type == MT_JETTGUNNER) // Special flag for rings Tails 06-03-2001
			mobj->z = mobj->floorz + 32*FRACUNIT;
		else
            mobj->z = mobj->floorz;

    }
    else if (z == ONCEILINGZ)
        mobj->z = mobj->ceilingz - mobj->height;
    else if(z == FLOATRANDZ)
    {
        fixed_t space = ((mobj->ceilingz)-(mobj->height))-mobj->floorz;
        if(space > 48*FRACUNIT)
        {
            space -= 40*FRACUNIT;
            mobj->z = ((space*P_Random())>>8)+mobj->floorz+40*FRACUNIT;
        }
        else
             mobj->z = mobj->floorz;
    }
    else
    {
        //CONS_Printf("mobj spawned at z %d\n",z>>16);
        mobj->z = z;
    }

    // added 16-6-98: special hack for spirit
    if(mobj->type == MT_SPIRIT)
        mobj->thinker.function.acv = (actionf_p1)P_MobjNullThinker;
    else
    {
        mobj->thinker.function.acp1 = (actionf_p1)P_MobjThinker;
        P_AddThinker (&mobj->thinker);
    }

    //SOM: Fuse for bunnies, squirls, and flingrings
      if(mobj->type == MT_BIRD || mobj->type == MT_SQRL || mobj->type == MT_MOUSE)
        mobj->fuse = 300 + (P_Random() % 50);

    return mobj;
}

void P_SetPrecipitationThingPosition(precipmobj_t* thing);

// Tails 08-15-2002
precipmobj_t* P_SpawnRainMobj ( fixed_t       x,
                      fixed_t       y,
                      fixed_t       z,
                      mobjtype_t    type )
{
    precipmobj_t*     mobj;
    state_t*    st;

    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));

    mobj->x = x;
    mobj->y = y;
    mobj->flags = mobjinfo[type].flags;

    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[mobjinfo[type].spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame; // FF_FRAMEMASK for frame, and other bits..
    mobj->touching_sectorlist = NULL; //SoM: 4/7/2000

    // set subsector and/or block links
    P_SetPrecipitationThingPosition (mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;

    mobj->z = z;
	mobj->momz = mobjinfo[type].speed;

    mobj->thinker.function.acp1 = (actionf_p1)P_RainThinker;
    P_AddThinker (&mobj->thinker);

    return mobj;
}
precipmobj_t* P_SpawnSnowMobj ( fixed_t       x,
                      fixed_t       y,
                      fixed_t       z,
                      mobjtype_t    type )
{
    precipmobj_t*     mobj;
    state_t*    st;

    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));

    mobj->x = x;
    mobj->y = y;
	mobj->flags = mobjinfo[type].flags;

    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[mobjinfo[type].spawnstate];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame; // FF_FRAMEMASK for frame, and other bits..
    mobj->touching_sectorlist = NULL; //SoM: 4/7/2000

    // set subsector and/or block links
    P_SetPrecipitationThingPosition (mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;

    mobj->z = z;
	mobj->momz = mobjinfo[type].speed;

    mobj->thinker.function.acp1 = (actionf_p1)P_SnowThinker;
    P_AddThinker (&mobj->thinker);

    return mobj;
}
/*
precipmobj_t* P_SpawnLaserMobj ( fixed_t       x,
                      fixed_t       y,
                      fixed_t       z)
{
    precipmobj_t*     mobj;
    state_t*    st;

    mobj = Z_Malloc (sizeof(*mobj), PU_LEVEL, NULL);
    memset (mobj, 0, sizeof (*mobj));

    mobj->x = x;
    mobj->y = y;
    mobj->flags |= (MF_NOBLOCKMAP|MF_NOGRAVITY);

    // do not set the state with P_SetMobjState,
    // because action routines can not be called yet
    st = &states[S_LASER3];

    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame; // FF_FRAMEMASK for frame, and other bits..
    mobj->touching_sectorlist = NULL; //SoM: 4/7/2000

    // set subsector and/or block links
    P_SetPrecipitationThingPosition (mobj);

    mobj->floorz = mobj->subsector->sector->floorheight;

    mobj->z = z;

    mobj->thinker.function.acp1 = (actionf_p1)P_LaserThinker;
    P_AddThinker (&mobj->thinker);

    return mobj;
}
*/
//
// P_RemoveMobj
//
mapthing_t     *itemrespawnque[ITEMQUESIZE];
tic_t           itemrespawntime[ITEMQUESIZE];
int             iquehead;
int             iquetail;


void P_RemoveMobj (mobj_t* mobj)
{
	// Rings only, please! Tails 05-27-2002
	if(mobj->spawnpoint && (mobj->type == MT_RING || mobj->type == MT_COIN)
		|| ((mobj->type == MT_HOMINGRING
		|| mobj->type == MT_RAILRING
		|| mobj->type == MT_SHIELDRING
		|| mobj->type == MT_AUTOMATICRING
		|| mobj->type == MT_EXPLOSIONRING) && !(mobj->flags2 & MF2_DONTRESPAWN)))
	{
		itemrespawnque[iquehead] = mobj->spawnpoint;
		itemrespawntime[iquehead] = leveltime;
		iquehead = (iquehead+1)&(ITEMQUESIZE-1);
		// lose one off the end?
		if (iquehead == iquetail)
			iquetail = (iquetail+1)&(ITEMQUESIZE-1);
	}

    // unlink from sector and block lists
    P_UnsetThingPosition (mobj);

    //SoM: 4/7/2000: Remove touching_sectorlist from mobj.
    if(sector_list)
    {
      P_DelSeclist(sector_list);
      sector_list = NULL;
    }

    // stop any playing sound
    S_StopSound (mobj);

    // free block
    P_RemoveThinker ((thinker_t*)mobj);
}

void P_UnsetPrecipThingPosition(precipmobj_t* thing);

// Tails 08-25-2002
void P_RemovePrecipMobj (precipmobj_t* mobj)
{
    // unlink from sector and block lists
    P_UnsetPrecipThingPosition (mobj);

	if(precipsector_list)
	{
	  P_DelPrecipSeclist(precipsector_list);
	  precipsector_list = NULL;
	}

    // free block
    P_RemoveThinker ((thinker_t*)mobj);
}

// Clearing out stuff for savegames Tails 02-02-2002
void P_RemoveSavegameMobj (mobj_t* mobj)
{
    // unlink from sector and block lists
    P_UnsetThingPosition (mobj);

    //SoM: 4/7/2000: Remove touching_sectorlist from mobj.
    if(sector_list)
    {
      P_DelSeclist(sector_list);
      sector_list = NULL;
    }

    // stop any playing sound
    S_StopSound (mobj);

    // free block
    P_RemoveThinker ((thinker_t*)mobj);
}


consvar_t cv_itemrespawntime={"respawnitemtime","30",CV_NETVAR,CV_Unsigned};
consvar_t cv_itemrespawn    ={"respawnitem"    , "0",CV_NETVAR,CV_OnOff};
consvar_t cv_flagtime={"flagtime","30",CV_NETVAR,CV_Unsigned}; // Tails 08-03-2001
consvar_t cv_suddendeath={"suddendeath","0",CV_NETVAR,CV_OnOff}; // Tails 11-18-2002

extern consvar_t cv_numsnow;
extern consvar_t cv_snow;
extern consvar_t cv_storm;
extern consvar_t cv_precipdist;
extern consvar_t cv_raindensity;

void P_SpawnLightningFlash(sector_t* sector);

void P_SpawnPrecipitation()
{
	int i;
	fixed_t x;
	fixed_t y;
	fixed_t height;

	if(cv_snow.value)
	{
		int z;
		subsector_t* snowsector;
		z = 0;

		for(i=0; i < (1048576/cv_numsnow.value); i++)
		{
			x = ((rand() * 65535) - 32768) << FRACBITS;
			y = ((rand() * 65535) - 32768) << FRACBITS;
			height = ((rand() * 65535) - 32768) << FRACBITS;

			if((rand()*65535) & 1)
				x = -x;

			if((rand()*65535) & 1)
				y = -y;

			snowsector = R_IsPointInSubsector(x, y);

			if(snowsector == 0)
				continue;

			if(mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS) // Spawn snow from normal ceilings
			{
				if(snowsector->sector->floorheight < snowsector->sector->ceilingheight)
				{
					while(!(height < snowsector->sector->ceilingheight &&
						height > snowsector->sector->floorheight))
						height = ((rand() * 65535) - 32768) << FRACBITS;

					z = P_Random();
					if(z < 64)
						P_SetPrecipMobjState(P_SpawnSnowMobj(x, y, height, MT_SNOWFLAKE), S_SNOW3);
					else if (z < 144)
						P_SetPrecipMobjState(P_SpawnSnowMobj(x, y, height, MT_SNOWFLAKE), S_SNOW2);
					else
						P_SpawnSnowMobj(x, y, height, MT_SNOWFLAKE);
				}
			}
			else
			{
				if(snowsector->sector->ceilingpic == skyflatnum && snowsector->sector->floorheight < snowsector->sector->ceilingheight)
				{
					while(!(height < snowsector->sector->ceilingheight &&
						height > snowsector->sector->floorheight))
						height = ((rand() * 65535) - 32768) << FRACBITS;

					z = P_Random();
					if(z < 64)
						P_SetPrecipMobjState(P_SpawnSnowMobj(x, y, height, MT_SNOWFLAKE), S_SNOW3);
					else if (z < 144)
						P_SetPrecipMobjState(P_SpawnSnowMobj(x, y, height, MT_SNOWFLAKE), S_SNOW2);
					else
						P_SpawnSnowMobj(x, y, height, MT_SNOWFLAKE);
				}
			}
		}
	}
	else if(cv_storm.value)
	{

		subsector_t* rainsector;

		for(i=0; i < (1048576/cv_raindensity.value); i++)
		{
			x = ((rand() * 65535) - 32768) << FRACBITS;
			y = ((rand() * 65535) - 32768) << FRACBITS;
			height = ((rand() * 65535) - 32768) << FRACBITS;

			if((rand()*65535) & 1)
				x = -x;

			if((rand()*65535) & 1)
				y = -y;

			rainsector = R_IsPointInSubsector(x, y);

			if(rainsector == 0)
				continue;

			if(rainsector->sector->ceilingpic == skyflatnum && rainsector->sector->floorheight < rainsector->sector->ceilingheight)
			{
				while(!(height < rainsector->sector->ceilingheight &&
					height > rainsector->sector->floorheight))
					height = ((rand() * 65535) - 32768) << FRACBITS;

				P_SpawnRainMobj(x, y, height, MT_RAIN);
			}
		}
	}
}

//
// P_RespawnSpecials
//
void P_RespawnSpecials (void)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    mobj_t*             mo;
    mapthing_t*         mthing;

    int                 i;

	// Rain spawning Tails 06-07-2002
	if(cv_storm.value)
	{
		int volume;

		if(netgame)
			return;

		volume = 255;

		if(players[displayplayer].mo->subsector->sector->ceilingpic == skyflatnum);
		else
		{
			fixed_t x,y,yl,yh,xl,xh,closex,closey,closedist,newdist,adx,ady;

			// Essentially check in a 1024 unit radius of the player for an outdoor area.
			yl = players[displayplayer].mo->y - 1024*FRACUNIT;
			yh = players[displayplayer].mo->y + 1024*FRACUNIT;
			xl = players[displayplayer].mo->x - 1024*FRACUNIT;
			xh = players[displayplayer].mo->x + 1024*FRACUNIT;
			closex = players[displayplayer].mo->x + 2048*FRACUNIT;
			closey = players[displayplayer].mo->y + 2048*FRACUNIT;
			closedist = 2048*FRACUNIT;
			for (y=yl ; y<=yh ; y += FRACUNIT*64)
				for (x=xl ; x<=xh ; x += FRACUNIT*64)
				{
					if(R_PointInSubsector(x,y)->sector->ceilingpic == skyflatnum) // Found the outdoors!
					{
						adx = abs(players[displayplayer].mo->x - x);
						ady = abs(players[displayplayer].mo->y - y);
						newdist = adx + ady - ((adx < ady ? adx : ady)>>1);
						if(newdist < closedist)
						{
							closex = x;
							closey = y;
							closedist = newdist;
						}
					}
				}
			volume = 255 - (closedist>>FRACBITS)/4;

		}
		if(volume < 0)
			volume = 0;
		else if(volume > 255)
			volume = 255;

		if(!netgame && (leveltime == 0 || leveltime % 80 == 1))
			S_StartSoundAtVolume(players[displayplayer].mo, sfx_rainin, volume);

		if(P_Random() < 2)
		{
			sector_t* ss;
			int i;
			ss = sectors;

			for(i=0; i<numsectors; i++, ss++)
			{
				if(ss->ceilingpic == skyflatnum) // Only for the sky.
					P_SpawnLightningFlash(ss); // Spawn a quick flash thinker here which fades out a bit like real lightning.
			}
			i = P_Random();

			if(i < 128 && leveltime & 1)
				S_StartSoundAtVolume(players[displayplayer].mo, sfx_litng1, volume);
			else if(i < 128)
				S_StartSoundAtVolume(players[displayplayer].mo, sfx_litng2, volume);
			else if(leveltime & 1)
				S_StartSoundAtVolume(players[displayplayer].mo, sfx_litng3, volume);
			else
				S_StartSoundAtVolume(players[displayplayer].mo, sfx_litng4, volume);
		}
		else if(leveltime & 1)
		{
			int random;

			random = P_Random();

			if(random > 253)
			{
				if(random & 1)
					S_StartSoundAtVolume(players[displayplayer].mo, sfx_athun1, volume);
				else
					S_StartSoundAtVolume(players[displayplayer].mo, sfx_athun2, volume);
			}
		}
	}

    // only respawn items when cv_itemrespawn is on
    if (!cv_itemrespawn.value)
        return; //

	// Don't respawn in special stages! Tails 05-26-2002
	if(gamemap >= sstage_start && gamemap <= sstage_end)
		return;

    // nothing left to respawn?
    if (iquehead == iquetail)
        return;

    // the first item in the queue is the first to respawn
    // wait at least 30 seconds
    if (leveltime - itemrespawntime[iquetail] < (tic_t)cv_itemrespawntime.value*TICRATE)
        return;

    mthing = itemrespawnque[iquetail];
/*
	if(mthing->type != 2014) // Only use this to respawn rings Tails 03-03-2002
		return;
*/
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    // find which type to spawn
    for (i=0 ; i< NUMMOBJTYPES ; i++)
        if (mthing->type == mobjinfo[i].doomednum)
            break;

    // spawn it
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
//	else if(mthing->z)// && mthing->options & MTF_AMBUSH) // Tails 08-05-2001
//		z = mthing->z << FRACBITS;//ss->sector->floorheight + 32*FRACUNIT; // Tails 08-05-2001
    else
        z = mthing->z << FRACBITS;//ONFLOORZ;

    mo = P_SpawnMobj (x,y,z, i);
    mo->spawnpoint = mthing;
    mo->angle = ANG45 * (mthing->angle/45);

    // pull it from the que
    iquetail = (iquetail+1)&(ITEMQUESIZE-1);
}

extern consvar_t cv_chasecam;
extern consvar_t cv_chasecam2;

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
// BP: spawn it at a playerspawn mapthing
void P_SpawnPlayer (mapthing_t* mthing, int playernum)
{
    player_t*           p;
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    mobj_t*             mobj;

    // not playing?
    if (!playeringame[playernum])
        return;

#ifdef PARANOIA
    if(playernum<0 && playernum>=MAXPLAYERS)
        I_Error("P_SpawnPlayer : playernum not in bound (%d)",playernum);
#endif

    p = &players[playernum];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn (playernum);

    x           = mthing->x << FRACBITS;
    y           = mthing->y << FRACBITS;

	// Flagging a player's ambush will make them start on the ceiling Tails 02-27-2002
	if(mthing->options & MTF_AMBUSH)
		z = ONCEILINGZ;
	else if((mthing->options >> 5) != 0)
		z = R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 5) << FRACBITS);
	else
		z = mthing->z << FRACBITS; // ONFLOORZ; // Use the Z! Tails 05-26-2002

	mthing->z = z >> FRACBITS;
    mobj        = P_SpawnMobj (x,y,z, MT_PLAYER);
    //SoM:
    mthing->mobj = mobj;

    // set color translations for player sprites
    // added 6-2-98 : change color : now use skincolor (befor is mthing->type-1
	// Some new stuff here Tails 06-10-2001

    mobj->flags |= (p->skincolor)<<MF_TRANSSHIFT;

    //
    // set 'spritedef' override in mobj for player skins.. (see ProjectSprite)
    // (usefulness : when body mobj is detached from player (who respawns),
    //  the dead body mobj retain the skin through the 'spritedef' override).
    mobj->skin = &skins[p->skin];

    mobj->angle = ANGLE_1*mthing->angle;
    if (playernum == consoleplayer)
        localangle = mobj->angle;
    else
    if (cv_splitscreen.value && playernum == secondarydisplayplayer)
        localangle2 = mobj->angle;
    mobj->player = p;
	if(cv_gametype.value == GT_CIRCUIT)
		mobj->health = p->health = 50 + 1; // Graue 12-06-2003
	else
		mobj->health = p->health;

    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->message = NULL;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = cv_viewheight.value<<FRACBITS;
    // added 2-12-98
    p->viewz = p->mo->z + p->viewheight;

    if (playernum == consoleplayer)
    {
        // wake up the status bar
        ST_Start ();
        // wake up the heads up text
        HU_Start ();
    }

#ifdef CLIENTPREDICTION2
    //added 1-6-98 : for movement prediction
    if(p->spirit)
        CL_ResetSpiritPosition(mobj);   // reset spirit possition
    else
        p->spirit = P_SpawnMobj (x,y,z, MT_SPIRIT);
        
    p->spirit->skin    = mobj->skin;
    p->spirit->angle   = mobj->angle;
    p->spirit->player  = mobj->player;
    p->spirit->health  = mobj->health;
    p->spirit->movedir = weapontobutton[p->readyweapon];
    p->spirit->flags2 |= MF2_DONTDRAW;
#endif
    SV_SpawnPlayer(playernum, mobj->x, mobj->y, mobj->angle);

    if(cv_chasecam.value)
	{
		if(displayplayer==playernum)
			P_ResetCamera(p, &camera);
	}
	if(cv_chasecam2.value && cv_splitscreen.value)
	{
		if(secondarydisplayplayer==playernum)
			P_ResetCamera(p, &camera2);
	}

	// Make players bounce in golf mode Graue 12-31-2003
	//if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
	//	p->mo->flags2 |= MF2_BOUNCE;
	//else
	//	p->mo->flags2 &= ~MF2_BOUNCE;
}

void P_SpawnStarpostPlayer (mobj_t* mobj, int playernum)
{
    player_t*           p;
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
	angle_t         angle;
	int      starposttime;

    // not playing?
    if (!playeringame[playernum])
        return;

#ifdef PARANOIA
    if(playernum<0 && playernum>=MAXPLAYERS)
        I_Error("P_SpawnPlayer : playernum not in bound (%d)",playernum);
#endif

    p = &players[playernum];

    x           = p->starpostx << FRACBITS;
    y           = p->starposty << FRACBITS;
	z           = p->starpostz << FRACBITS;
	angle       = p->starpostangle;
	starposttime= p->starposttime;

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn (playernum);

    mobj        = P_SpawnMobj (x,y,z, MT_PLAYER);

    // set color translations for player sprites
    // added 6-2-98 : change color : now use skincolor (befor is mthing->type-1
	// Some new stuff here Tails 06-10-2001
    mobj->flags |= (p->skincolor)<<MF_TRANSSHIFT;

    //
    // set 'spritedef' override in mobjy for player skins.. (see ProjectSprite)
    // (usefulness : when body mobjy is detached from player (who respawns),
    //  the dead body mobjy retain the skin through the 'spritedef' override).
    mobj->skin = &skins[p->skin];

	mobj->angle = angle;
    if (playernum == consoleplayer)
        localangle = mobj->angle;
    else
    if (cv_splitscreen.value && playernum == secondarydisplayplayer)
        localangle2 = mobj->angle;
    mobj->player = p;
	if(cv_gametype.value == GT_CIRCUIT)
		mobj->health = p->health = 50 + 1; // Graue 12-06-2003
	else
		mobj->health = p->health;

    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->message = NULL;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = cv_viewheight.value<<FRACBITS;
    // added 2-12-98
    p->viewz = p->mo->z + p->viewheight;

    if (playernum == consoleplayer)
    {
        // wake up the status bar
        ST_Start ();
        // wake up the heads up text
        HU_Start ();
    }

    SV_SpawnPlayer(playernum, mobj->x, mobj->y, mobj->angle);

    if(cv_chasecam.value)
	{
		if(displayplayer==playernum)
			P_ResetCamera(p, &camera);
	}
	if(cv_chasecam2.value && cv_splitscreen.value)
	{
		if(secondarydisplayplayer==playernum)
			P_ResetCamera(p, &camera2);
	}

	if(!(netgame || multiplayer))
		leveltime = starposttime;

	// Make players bounce in golf mode Graue 12-31-2003
	//if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
	//	p->mo->flags2 |= MF2_BOUNCE;
	//else
	//	p->mo->flags2 &= ~MF2_BOUNCE;
}

extern consvar_t cv_raceitemboxes;
extern consvar_t cv_matchboxes; // Race cars! =P
extern consvar_t cv_specialrings;
extern consvar_t cv_ringslinger;

// Graue 12-06-2003
extern consvar_t cv_circuit_itemboxes;
extern consvar_t cv_circuit_ringthrow;

//
// P_SpawnMapThing
// The fields of the mapthing should
// already be in host byte order.
//
void P_SpawnMapThing (mapthing_t* mthing)
{
    int                 i;
    int                 bit;
    mobj_t*             mobj;
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
    subsector_t*        ss; // Tails 08-30-2001
	
    if(!mthing->type)
      return; //SoM: 4/7/2000: Ignore type-0 things as NOPs

    // count deathmatch start positions
    if (mthing->type == 11)
    {
        if (numdmstarts < MAX_DM_STARTS)
        {
            deathmatchstarts[numdmstarts] = mthing;
            mthing->type = 0;
            numdmstarts++;
        }
        return;
    }

	else if (mthing->type == 87) // CTF Startz! Tails 08-04-2001
    {
        if (numredctfstarts < MAXPLAYERS)
        {
			redctfstarts[numredctfstarts] = mthing;
            mthing->type=0;
            numredctfstarts++;
        }
        return;
    }

	else if (mthing->type == 89) // CTF Startz! Tails 08-04-2001
    {
        if (numbluectfstarts < MAXPLAYERS)
        {
			bluectfstarts[numbluectfstarts] = mthing;
            mthing->type=0;
            numbluectfstarts++;
        }
        return;
    }

	else if (mthing->type == 57 || mthing->type == 84
		|| mthing->type == 44 || mthing->type == 76
		|| mthing->type == 77 || mthing->type == 47
		|| mthing->type == 2014 || mthing->type == 47
		|| mthing->type == 2007 || mthing->type == 2048
		|| mthing->type == 2010 || mthing->type == 2046
		|| mthing->type == 2047 || mthing->type == 37) // Don't spawn hoops, wings or rings yet!
		return;
	// cv_specialrings.value check for spawning special weapon rings moved down Graue 12-13-2003

    // check for players specially
    // added 9-2-98 type 5 -> 8 player[x] starts for cooperative
    //              support ctfdoom cooperative playerstart
    //SoM: 4/7/2000: Fix crashing bug.
    if ((mthing->type > 0 && mthing->type <=4) ||
        (mthing->type<=4028 && mthing->type>=4001) )
    {
        if(mthing->type>4000)                     // This screws up my 'debug'
             mthing->type=mthing->type-4001+5;    // code! Tails 04-03-2003

        // save spots for respawning in network games
        playerstarts[mthing->type-1] = mthing;
        return;
    }

    // Ambient sound sequences
    if(mthing->type >= 1200 && mthing->type < 1300)
    {
//        P_AddAmbientSfx(mthing->type-1200);
        return;
    }


	if(mthing->type != 52 && mthing->type != 53 && mthing->type != 59
		&& mthing->type != 61 && mthing->type != 62 && mthing->type != 15
		&& mthing->type != 45 && mthing->type != 46 && mthing->type != 55
		&& mthing->type != 82 && mthing->type != 85)
	{
		if (gameskill == sk_baby)
			bit = 1;
		else if (gameskill >= sk_nightmare)
			bit = 4;
		else
			bit = 1<<(gameskill-1);

		if (!(mthing->options & bit) )
			return;
	}

    // find which type to spawn
    for (i=0 ; i< NUMMOBJTYPES ; i++)
        if (mthing->type == mobjinfo[i].doomednum)
            break;

    if (i==NUMMOBJTYPES)
    {
        CONS_Printf ("\2P_SpawnMapThing: Unknown type %i at (%i, %i)\n",
                      mthing->type,
                      mthing->x, mthing->y);
        return;
    }

	// Spawn special rings in circuit mode with circuit_ringthrow on Graue 12-12-2003
	if(!(cv_gametype.value == GT_MATCH || cv_gametype.value == GT_CTF || cv_gametype.value == GT_TAG)
		&& !cv_ringslinger.value && !(cv_gametype.value == GT_CIRCUIT && cv_circuit_ringthrow.value)
		|| !cv_specialrings.value) // Graue 12-13-2003
	{
		switch(i)
		{
			case MT_HOMINGRING:
			case MT_RAILRING:
			case MT_SHIELDRING:
			case MT_AUTOMATICRING:
			case MT_EXPLOSIONRING:
				return;
			default:
				break;
		}
	}

	if(cv_gametype.value != GT_COOP) // Hunt should only work on CoOp. Makes sense?
	{
		switch(i)
		{
			case MT_EMERHUNT:
			case MT_EMESHUNT:
			case MT_EMETHUNT:
				return;
			default:
				break;
		}
	}

	if(cv_gametype.value == GT_MATCH) // No enemies in match mode
	{
		if((mobjinfo[i].flags & MF_ENEMY)
			|| (mobjinfo[i].flags & MF_BOSS))
			return;
	}

	// Set powerup boxes to user settings for race. Tails 08-28-2002
	if(cv_gametype.value == GT_RACE)
	{
		if(cv_raceitemboxes.value != 0)
		{
			switch(i)
			{
				case MT_BLUETV: // Blue shield box
				case MT_REDTV: // Red shield box
				case MT_YELLOWTV: // Yellow shield box
				case MT_GREENTV: // Green shield box
				case MT_BLACKTV: // Black shield box
				case MT_SNEAKERTV: // Super Sneaker box
				case MT_PRUP: // 1-Up box
				case MT_SUPERRINGBOX: // 10-Ring box
				case MT_GREYRINGBOX: // 25-Ring box
				case MT_INV: // Invincibility box
				case MT_MIXUPBOX: // Teleporter Mixup box
				case MT_QUESTIONBOX: // Random box
				case MT_EGGMANBOX:
					if(cv_raceitemboxes.value == 1) // Random
						i = MT_QUESTIONBOX;
					else if(cv_raceitemboxes.value == 2)
						i = MT_MIXUPBOX;
					else if(cv_raceitemboxes.value == 3)
						return; // Don't spawn!
					break;
				default:
					break;
			}
		}
		else
		{
			switch(i)
			{
				case MT_BLUETV: // Blue shield box
					if(!cv_blueshield.value)
						return;
					break;
				case MT_REDTV: // Red shield box
					if(!cv_redshield.value)
						return;
					break;
				case MT_YELLOWTV: // Yellow shield box
					if(!cv_yellowshield.value)
						return;
					break;
				case MT_GREENTV: // Green shield box
					if(!cv_greenshield.value)
						return;
					break;
				case MT_BLACKTV: // Black shield box
					if(!cv_blackshield.value)
						return;
					break;
				case MT_SNEAKERTV: // Super Sneaker box
					if(!cv_supersneakers.value)
						return;
					break;
				case MT_PRUP: // 1-Up box
					if(!cv_1up.value)
						return;
					break;
				case MT_SUPERRINGBOX: // 10-Ring box
					if(!cv_superring.value)
						return;
					break;
				case MT_GREYRINGBOX: // 25-Ring box
					if(!cv_silverring.value)
						return;
					break;
				case MT_INV: // Invincibility box
					if(!cv_invincibility.value)
						return;
					break;
				case MT_EGGMANBOX:
					if(!cv_eggmanbox.value)
						return;
					break;
				case MT_MIXUPBOX: // Yeah, it's not a spawnmapthing, but so what?
					if(!cv_teleporters.value)
						return;
					break;
				default:
					break;
			}
		}
	}

	// Set powerup boxes to user settings for circuit mode. Graue 12-06-2003
	// FIXTHIS: this is almost exactly the same as the above race mode powerup box setting
	if(cv_gametype.value == GT_CIRCUIT)
	{
		if(cv_circuit_itemboxes.value != 0)
		{
			// Removed switch Tails 12-14-2003
			if(mobjinfo[i].flags & MF_MONITOR)
			{
				if(cv_raceitemboxes.value == 1) // Random
					i = MT_QUESTIONBOX;
				else if(cv_raceitemboxes.value == 2)
					return; // Don't spawn!
			}
		}
		else
		{
			switch(i)
			{
				case MT_BLUETV: // Blue shield box
					if(!cv_blueshield.value)
						return;
					break;
				case MT_REDTV: // Red shield box
					if(!cv_redshield.value)
						return;
					break;
				case MT_YELLOWTV: // Yellow shield box
					if(!cv_yellowshield.value)
						return;
					break;
				case MT_GREENTV: // Green shield box
					if(!cv_greenshield.value)
						return;
					break;
				case MT_BLACKTV: // Black shield box
					if(!cv_blackshield.value)
						return;
					break;
				case MT_SNEAKERTV: // Super Sneaker box
					if(!cv_supersneakers.value)
						return;
					break;
				case MT_PRUP: // 1-Up box
					if(!cv_1up.value)
						return;
					break;
				case MT_SUPERRINGBOX: // 10-Ring box
					if(!cv_superring.value)
						return;
					break;
				case MT_GREYRINGBOX: // 25-Ring box
					if(!cv_silverring.value)
						return;
					break;
				case MT_INV: // Invincibility box
					if(!cv_invincibility.value)
						return;
					break;
				case MT_EGGMANBOX:
					if(!cv_eggmanbox.value)
						return;
					break;
				case MT_MIXUPBOX: // Yeah, it's not a spawnmapthing, but so what?
					if(!cv_teleporters.value)
						return;
					break;
				default:
					break;
			}
		}
	}
	
	// Set powerup boxes to user settings for other netplay modes. Tails 11-20-2002
	else if(cv_gametype.value == GT_MATCH
		|| cv_gametype.value == GT_TAG
		|| cv_gametype.value == GT_CTF
		|| cv_gametype.value == GT_CHAOS)
	{
		if(cv_matchboxes.value != 0)
		{
			if(cv_matchboxes.value == 1) // Random
			{
				switch(i)
				{
					case MT_BLUETV: // Blue shield box
					case MT_REDTV: // Red shield box
					case MT_YELLOWTV: // Yellow shield box
					case MT_GREENTV: // Green shield box
					case MT_BLACKTV: // Black shield box
					case MT_SNEAKERTV: // Super Sneaker box
					case MT_PRUP: // 1-Up box
					case MT_SUPERRINGBOX: // 10-Ring box
					case MT_GREYRINGBOX: // 25-Ring box
					case MT_INV: // Invincibility box
					case MT_MIXUPBOX: // Teleporter Mixup box
					case MT_QUESTIONBOX: // Random box
					case MT_EGGMANBOX:
						i = MT_QUESTIONBOX;
						break;
					default:
						break;
				}
			}
			else if(cv_matchboxes.value == 3) // Don't spawn
			{
				switch(i)
				{
					case MT_BLUETV: // Blue shield box
					case MT_REDTV: // Red shield box
					case MT_YELLOWTV: // Yellow shield box
					case MT_GREENTV: // Green shield box
					case MT_BLACKTV: // Black shield box
					case MT_SNEAKERTV: // Super Sneaker box
					case MT_PRUP: // 1-Up box
					case MT_SUPERRINGBOX: // 10-Ring box
					case MT_GREYRINGBOX: // 25-Ring box
					case MT_INV: // Invincibility box
					case MT_MIXUPBOX: // Teleporter Mixup box
					case MT_QUESTIONBOX: // Random box
					case MT_EGGMANBOX:
						return;
						break;
					default:
						break;
				}
			}
			else
			{
				switch(i)
				{
					case MT_BLUETV: // Blue shield box
						if(!cv_blueshield.value)
							return;
						break;
					case MT_REDTV: // Red shield box
						if(!cv_redshield.value)
							return;
						break;
					case MT_YELLOWTV: // Yellow shield box
						if(!cv_yellowshield.value)
							return;
						break;
					case MT_GREENTV: // Green shield box
						if(!cv_greenshield.value)
							return;
						break;
					case MT_BLACKTV: // Black shield box
						if(!cv_blackshield.value)
							return;
						break;
					case MT_SNEAKERTV: // Super Sneaker box
						if(!cv_supersneakers.value)
							return;
						break;
					case MT_PRUP: // 1-Up box
						if(!cv_1up.value)
							return;
						break;
					case MT_SUPERRINGBOX: // 10-Ring box
						if(!cv_superring.value)
							return;
						break;
					case MT_GREYRINGBOX: // 25-Ring box
						if(!cv_silverring.value)
							return;
						break;
					case MT_INV: // Invincibility box
						if(!cv_invincibility.value)
							return;
						break;
					case MT_EGGMANBOX:
						if(!cv_eggmanbox.value)
							return;
						break;
					case MT_MIXUPBOX: // Teleporter Mixup box
						if(!cv_teleporters.value)
							return;
						if(cv_matchboxes.value == 2)
							mthing->options &= ~MTF_AMBUSH;
						break;
					default:
						break;
				}
			}
		}
		else
		{
			switch(i)
			{
				case MT_BLUETV: // Blue shield box
					if(!cv_blueshield.value)
						return;
					break;
				case MT_REDTV: // Red shield box
					if(!cv_redshield.value)
						return;
					break;
				case MT_YELLOWTV: // Yellow shield box
					if(!cv_yellowshield.value)
						return;
					break;
				case MT_GREENTV: // Green shield box
					if(!cv_greenshield.value)
						return;
					break;
				case MT_BLACKTV: // Black shield box
					if(!cv_blackshield.value)
						return;
					break;
				case MT_SNEAKERTV: // Super Sneaker box
					if(!cv_supersneakers.value)
						return;
					break;
				case MT_PRUP: // 1-Up box
					if(!cv_1up.value)
						return;
					break;
				case MT_SUPERRINGBOX: // 10-Ring box
					if(!cv_superring.value)
						return;
					break;
				case MT_GREYRINGBOX: // 25-Ring box
					if(!cv_silverring.value)
						return;
					break;
				case MT_EGGMANBOX:
					if(!cv_eggmanbox.value)
						return;
					break;
				case MT_INV: // Invincibility box
					if(!cv_invincibility.value)
						return;
					break;
				case MT_MIXUPBOX:
					if(!cv_teleporters.value)
						return;
					break;
				default:
					break;
			}
		}

	}

	if(cv_gametype.value != GT_COOP && cv_gametype.value != GT_RACE) // Coop and race only Graue 12-13-2003
	{
		if(i==MT_SIGN) // Don't spawn the level exit sign when it isn't needed.
			return;
	}

	// Do spawn weapons in Chaos! Graue 12-13-2003
	// Previously bailed out for weapon rings in chaos mode here, but that's already handled
	// above, unless cv_ringslinger == 1

	if ((i == MT_SUPERRINGBOX || i == MT_GREYRINGBOX || i == MT_EGGMANBOX || i == MT_BLUETV || i == MT_REDTV || i == MT_YELLOWTV || i == MT_GREENTV || i == MT_BLACKTV)
		&& gameskill == sk_insane
		&& !(gamemap >= sstage_start && gamemap <= sstage_end)) // Don't have rings in Ultimate mode Tails 03-26-2001
		return;

	if((i == MT_BLUEFLAG || i == MT_REDFLAG) && cv_gametype.value != GT_CTF)
		return; // Don't spawn flags if you aren't in CTF Mode! Tails 09-03-2001

	// You already got this token. Tails 12-18-2003
	if(i == MT_EMMY)
	{
		if (tokenlist & (1 << (tokenbits)))
			return;
	}
    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    ss = R_PointInSubsector (x,y);

	if(mthing->type != 52 && mthing->type != 53 && mthing->type != 59
		&& mthing->type != 61 && mthing->type != 62 && mthing->type != 15
		&& mthing->type != 45 && mthing->type != 46 && mthing->type != 55
		&& mthing->type != 82 && mthing->type != 85)
	{
		if (mobjinfo[i].flags & MF_SPAWNCEILING)
			z = ONCEILINGZ;
		else if((mthing->options >> 4) != 0)
			z = R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 4) << FRACBITS);
		else if (i == MT_SPECIALSPIKEBALL
			|| i == MT_HOMINGRING || i == MT_RAILRING
			|| i == MT_SHIELDRING || i == MT_AUTOMATICRING
			|| i == MT_EXPLOSIONRING)
		{
			if(mthing->options & MTF_AMBUSH) // Special flag for rings Tails 06-03-2001
				z = ss->sector->floorheight + 32*FRACUNIT;
			else
				z = ss->sector->floorheight;

			mthing->z = z >> FRACBITS;
		}
		else if (i == MT_CRAWLACOMMANDER || i == MT_DETON || i == MT_JETTBOMBER || i == MT_JETTGUNNER || i == MT_EGGMOBILE || i == MT_EGGMOBILE2)
			z = ss->sector->floorheight + 33*FRACUNIT;
		else
			z = ONFLOORZ;

		mthing->z = z >> FRACBITS;
	}
	else
		z = ONFLOORZ;

	mobj = P_SpawnMobj (x,y,z, i);
	mobj->spawnpoint = mthing;

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE && mobj->flags & MF_MONITOR)
	{
		mobj->flags |= MF_SPECIAL;
		mobj->flags &= ~MF_SOLID;
	}

	if(mobj->type == MT_FAN)
	{
		if(mthing->angle != 0)
		{
			mobj->health = mthing->angle;
		}
		else
		{
			mobj->health = (ss->sector->ceilingheight - .25*(ss->sector->ceilingheight - ss->sector->floorheight));
			mobj->health -= ss->sector->floorheight;
			mobj->health >>= FRACBITS;
		}
	}

	if(mobj->flags & MF_BOSS)
	{
		if(mthing->options & MTF_MULTI) // No egg trap for this boss
			mobj->flags2 |= MF2_BOSSNOTRAP;

		// Special case - Bosses use the 5th bit, so rip it off!!!
		if((mthing->options >> 5) != 0)
			z = R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 5) << FRACBITS);

		mthing->z = z >> FRACBITS;
	}
	else if(mobj->type == MT_EGGCAPSULE)
	{
		mobj->health = mthing->angle&1023;

		if(mthing->angle >> 10 != 0)
			mobj->threshold = mthing->angle >> 10;
	}

	// Special condition for the 2nd boss.
	if(mobj->type == MT_EGGMOBILE2)
		mobj->watertop = mobj->info->speed;
	else if(mobj->type == MT_CHAOSSPAWNER)
		mobj->fuse = P_Random()*2;

	if(mthing->type == 52 || mthing->type == 53 || mthing->type == 59
		|| mthing->type == 61 || mthing->type == 62 || mthing->type == 15
		|| mthing->type == 45 || mthing->type == 46 || mthing->type == 55
		|| mthing->type == 82 || mthing->type == 85)
	{
		if(mthing->options >> 10 != 0)
			mobj->threshold = mthing->options >> 10;

		mobj->health = mthing->options & 1023;
	}
	else if(i == MT_EMMY)
	{
		mobj->health = 1 << tokenbits++;
		P_SpawnMobj(x,y,z, MT_TOKEN);
	}
	else if(i == MT_EGGMOBILE && mthing->options & MTF_AMBUSH)
	{
		mobj_t* spikemobj;
		spikemobj = P_SpawnMobj(x,y,z, MT_SPIKEBALL);
		spikemobj->target = mobj;
		spikemobj->angle = 0;
		spikemobj = P_SpawnMobj(x,y,z, MT_SPIKEBALL);
		spikemobj->target = mobj;
		spikemobj->angle = ANG90;
		spikemobj = P_SpawnMobj(x,y,z, MT_SPIKEBALL);
		spikemobj->target = mobj;
		spikemobj->angle = ANG180;
		spikemobj = P_SpawnMobj(x,y,z, MT_SPIKEBALL);
		spikemobj->target = mobj;
		spikemobj->angle = ANG270;
	}
	else if(i == MT_STARPOST)
	{
		unsigned int spbit;

		if(mthing->options & 16) // Star Post #2
			mobj->health = 2;
		else if(mthing->options & 32) // Star Post #3
			mobj->health = 3;
		else if(mthing->options & 64) // Star Post #4
			mobj->health = 4;
		else if(mthing->options & 128) // Star Post #5
			mobj->health = 5;
		else if(mthing->options & 256) // Star Post #6
			mobj->health = 6;
		else if(mthing->options & 512) // Star Post #7
			mobj->health = 7;
		else if(mthing->options & 1024) // Star Post #8
			mobj->health = 8;
		else if(mthing->options & 2048) // Star Post #9
			mobj->health = 9;
		else if(mthing->options & 4096) // Star Post #10
			mobj->health = 10;
		else // Star Post #1
			mobj->health = 1;

		// Check starpost numbers and bits Graue 11-18-2003
		spbit = 1 << (mobj->health - 1);
		if(!(bitstarposts & spbit)) // We haven't already found a starpost with this number
		{
			bitstarposts |= spbit;
			numstarposts++;
		}
	}

	if (mobj->tics > 0)
		mobj->tics = 1 + (P_Random () % mobj->tics);
	if (mobj->flags & MF_COUNTKILL)
		totalkills++;
	if (mobj->flags & MF_COUNTITEM)
		totalitems++;

	mobj->angle = mthing->angle*ANGLE_1;
	if (mthing->options & MTF_AMBUSH)
	{
		switch (i)
		{
			case MT_YELLOWDIAG:
			case MT_YELLOWDIAGDOWN:
			case MT_REDDIAG:
			case MT_REDDIAGDOWN:
				mobj->angle += ANG45/2;
				break;
			default:
				break;
		}

		// Tails 12-16-2003
		if(mobj->flags & MF_PUSHABLE)
		{
			mobj->flags &= ~MF_PUSHABLE;
			mobj->flags2 |= MF2_STANDONME;
		}

		if(mthing->type != 52 && mthing->type != 53 && mthing->type != 59
			&& mthing->type != 61 && mthing->type != 62 && mthing->type != 15
			&& mthing->type != 45 && mthing->type != 46 && mthing->type != 82
			&& mthing->type != 85)
			mobj->flags |= MF_AMBUSH;
	}

	// New behaviour for MF_PUSHABLEs with multi flag set Graue 12-31-2003
	if((mthing->options & MTF_MULTI) && (mobj->flags & MF_PUSHABLE))
	{
		mobj->flags2 |= MF2_SLIDEPUSH;
		mobj->flags |= MF_BOUNCE;
	}

	mthing->mobj = mobj;
}

void P_SpawnHoopsAndRings(mapthing_t* mthing)
{
	mobj_t* mobj;
	int r;
	fixed_t x,y,z;

	x = mthing->x << FRACBITS;
	y = mthing->y << FRACBITS;

	if(mthing->type == 57) // NiGHTS hoop!
	{
		int i;
		TVector v;
		TVector *res;
		fixed_t finalx, finaly, finalz;
		mobj_t* hoopcenter;
		mobj_t* nextmobj;
		mobj_t* axis;
		short closestangle;
		fixed_t mthingx, mthingy, mthingz;
		degenmobj_t xypos;

		nextmobj = axis = NULL;

		mthingx = mthing->x << FRACBITS;
		mthingy = mthing->y << FRACBITS;
			
		mthingz = mthing->options << FRACBITS;

		hoopcenter = P_SpawnMobj(mthingx, mthingy, mthingz, MT_HOOPCENTER);

		hoopcenter->flags2 |= MF2_NOTHINK;

		axis = P_GetClosestAxis(hoopcenter);

		if(axis == NULL)
		{
			CONS_Printf("You forgot to put axis points in the map!\n");
			return;
		}

		xypos = P_GimmeAxisXYPos(axis, mthingx, mthingy);

		mthingx = xypos.x;
		mthingy = xypos.y;

		mthingz += R_PointInSubsector(mthingx, mthingy)->sector->floorheight;

		hoopcenter->z = mthingz - hoopcenter->height/2;

		P_UnsetThingPosition(hoopcenter);
		hoopcenter->x = mthingx;
		hoopcenter->y = mthingy;
		P_SetThingPosition(hoopcenter);

		closestangle = R_PointToAngle2(mthingx, mthingy, axis->x, axis->y)/ANGLE_1;

		hoopcenter->movedir = mthing->angle;
		hoopcenter->movecount = closestangle;

		// Create the hoop!
		for(i = 0; i<32; i++)
		{
			v[0] = cos(i*11.25*deg2rad) * 96;
			v[1] = 0;
			v[2] = sin(i*11.25*deg2rad) * 96;
			v[3] = 1;

			res = VectorMatrixMultiply(v, *RotateXMatrix(mthing->angle*deg2rad));
			memcpy(&v, res, sizeof(v));
			res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
			memcpy(&v, res, sizeof(v));

			finalx = mthingx + double2fixed(v[0]);
			finaly = mthingy + double2fixed(v[1]);
			finalz = mthingz + double2fixed(v[2]);
/*
			CONS_Printf("finalx is %d \t(%f)\n", finalx >> FRACBITS, v[0]);
			CONS_Printf("finaly is %d \t(%f)\n", finaly >> FRACBITS, v[1]);
			CONS_Printf("finalz is %d \t(%f)\n", finalz >> FRACBITS, v[2]);
*/
			mobj = P_SpawnMobj(finalx, finaly, finalz, MT_HOOP);
			mobj->z -= mobj->height/2;
			mobj->target = hoopcenter; // Link the sprite to the center.

			if(xmasmode && (i&1))
				mobj->flags =  (mobj->flags & ~MF_TRANSLATION)
						 | (MAXSKINCOLORS-1<<MF_TRANSSHIFT); // Yellow

			// Link all the sprites in the hoop together
			if(nextmobj != NULL)
			{
				mobj->bprev = nextmobj;
				mobj->bprev->bnext = mobj;
			}

			nextmobj = mobj;
		}

		// Create the collision detectors!
		for(i = 0; i<16; i++)
		{
			v[0] = cos(i*22.5*deg2rad) * 32;
			v[1] = 0;
			v[2] = sin(i*22.5*deg2rad) * 32;
			v[3] = 1;
			res = VectorMatrixMultiply(v, *RotateXMatrix(mthing->angle*deg2rad));
			memcpy(&v, res, sizeof(v));
			res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
			memcpy(&v, res, sizeof(v));

			finalx = mthingx + double2fixed(v[0]);
			finaly = mthingy + double2fixed(v[1]);
			finalz = mthingz + double2fixed(v[2]);
/*
			CONS_Printf("finalx is %d \t(%f)\n", finalx >> FRACBITS, v[0]);
			CONS_Printf("finaly is %d \t(%f)\n", finaly >> FRACBITS, v[1]);
			CONS_Printf("finalz is %d \t(%f)\n", finalz >> FRACBITS, v[2]);
*/
			mobj = P_SpawnMobj(finalx, finaly, finalz, MT_HOOPCOLLIDE);
			mobj->z -= mobj->height/2;

			// Link all the collision sprites together.
			mobj->bprev = nextmobj;
			mobj->bprev->bnext = mobj;

			nextmobj = mobj;
		}
		// Create the collision detectors!
		for(i = 0; i<16; i++)
		{
			v[0] = cos(i*22.5*deg2rad) * 64;
			v[1] = 0;
			v[2] = sin(i*22.5*deg2rad) * 64;
			v[3] = 1;
			res = VectorMatrixMultiply(v, *RotateXMatrix(mthing->angle*deg2rad));
			memcpy(&v, res, sizeof(v));
			res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
			memcpy(&v, res, sizeof(v));

			finalx = mthingx + double2fixed(v[0]);
			finaly = mthingy + double2fixed(v[1]);
			finalz = mthingz + double2fixed(v[2]);
/*
			CONS_Printf("finalx is %d \t(%f)\n", finalx >> FRACBITS, v[0]);
			CONS_Printf("finaly is %d \t(%f)\n", finaly >> FRACBITS, v[1]);
			CONS_Printf("finalz is %d \t(%f)\n", finalz >> FRACBITS, v[2]);
*/
			mobj = P_SpawnMobj(finalx, finaly, finalz, MT_HOOPCOLLIDE);
			mobj->z -= mobj->height/2;

			// Link all the collision sprites together.
			mobj->bprev = nextmobj;
			mobj->bprev->bnext = mobj;

			nextmobj = mobj;
		}
		return;
	}
	else if(mthing->type == 37) // Wing logo item.
	{
		if((mthing->options >> 4) != 0)
			mthing->z = (R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 4) << FRACBITS)) >> FRACBITS;
		else
			mthing->z = R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight >> FRACBITS;

		mobj = P_SpawnMobj (mthing->x << FRACBITS, mthing->y << FRACBITS,mthing->z << FRACBITS, MT_NIGHTSWING);
		mobj->spawnpoint = mthing;

		if (mobj->tics > 0)
			mobj->tics = 1 + (P_Random () % mobj->tics);
		mobj->angle = ANGLE_1*mthing->angle;
		mobj->flags |= MF_AMBUSH;
		mthing->mobj = mobj;
	}
	else if(mthing->type == 2048) // A ring of wing items (NiGHTS stuff)
	{
		int i;
		TVector v;
		TVector *res;
		fixed_t finalx, finaly, finalz;
		mobj_t* hoopcenter;
		mobj_t* axis;
		short closestangle;
		fixed_t mthingx, mthingy, mthingz;
		degenmobj_t xypos;

		axis = NULL;

		mthingx = mthing->x << FRACBITS;
		mthingy = mthing->y << FRACBITS;
				
		if((mthing->options >> 4) != 0)
			mthingz = (R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 4) << FRACBITS));
		else
			mthingz = R_PointInSubsector(mthingx, mthing->y << FRACBITS)->sector->floorheight;

		hoopcenter = P_SpawnMobj(mthingx, mthingy, mthingz, MT_DISS);

		axis = P_GetClosestAxis(hoopcenter);

		P_RemoveMobj(hoopcenter);

		if(axis == NULL)
		{
			CONS_Printf("You forgot to put axis points in the map!\n");
			return;
		}

		xypos = P_GimmeAxisXYPos(axis, mthingx, mthingy);

		mthingx = xypos.x;
		mthingy = xypos.y;

//		mthingz += R_PointInSubsector(mthingx, mthingy)->sector->floorheight;

		closestangle = (R_PointToAngle2(mthingx, mthingy, axis->x, axis->y)+ANG90)/ANGLE_1;

		// Create the hoop!
		for(i = 0; i<8; i++)
		{
			v[0] = cos(i*45*deg2rad) * 96;
			v[1] = 0;
			v[2] = sin(i*45*deg2rad) * 96;
			v[3] = 1;

			res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
			memcpy(&v, res, sizeof(v));

			finalx = mthingx + double2fixed(v[0]);
			finaly = mthingy + double2fixed(v[1]);
			finalz = mthingz + double2fixed(v[2]);
/*
			CONS_Printf("finalx is %d \t(%f)\n", finalx >> FRACBITS, v[0]);
			CONS_Printf("finaly is %d \t(%f)\n", finaly >> FRACBITS, v[1]);
			CONS_Printf("finalz is %d \t(%f)\n", finalz >> FRACBITS, v[2]);
*/
			mobj = P_SpawnMobj(finalx, finaly, finalz, MT_NIGHTSWING);
			mobj->z -= mobj->height/2;
		}
		return;
	}
	else if(mthing->type == 2010) // A BIGGER ring of wing items (NiGHTS stuff)
	{
		int i;
		TVector v;
		TVector *res;
		fixed_t finalx, finaly, finalz;
		mobj_t* hoopcenter;
		mobj_t* axis;
		short closestangle;
		fixed_t mthingx, mthingy, mthingz;
		degenmobj_t xypos;

		axis = NULL;

		mthingx = mthing->x << FRACBITS;
		mthingy = mthing->y << FRACBITS;
				
		if((mthing->options >> 4) != 0)
			mthingz = (R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 4) << FRACBITS));
		else
			mthingz = R_PointInSubsector(mthingx, mthing->y << FRACBITS)->sector->floorheight;

		hoopcenter = P_SpawnMobj(mthingx, mthingy, mthingz, MT_DISS);

		axis = P_GetClosestAxis(hoopcenter);

		P_RemoveMobj(hoopcenter);

		if(axis == NULL)
		{
			CONS_Printf("You forgot to put axis points in the map!\n");
			return;
		}

		xypos = P_GimmeAxisXYPos(axis, mthingx, mthingy);

		mthingx = xypos.x;
		mthingy = xypos.y;

//		mthingz += R_PointInSubsector(mthingx, mthingy)->sector->floorheight;

		closestangle = (R_PointToAngle2(mthingx, mthingy, axis->x, axis->y)+ANG90)/ANGLE_1;

		// Create the hoop!
		for(i = 0; i<16; i++)
		{
			v[0] = cos(i*45*deg2rad) * 192;
			v[1] = 0;
			v[2] = sin(i*45*deg2rad) * 192;
			v[3] = 1;

			res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
			memcpy(&v, res, sizeof(v));

			finalx = mthingx + double2fixed(v[0]);
			finaly = mthingy + double2fixed(v[1]);
			finalz = mthingz + double2fixed(v[2]);
/*
			CONS_Printf("finalx is %d \t(%f)\n", finalx >> FRACBITS, v[0]);
			CONS_Printf("finaly is %d \t(%f)\n", finaly >> FRACBITS, v[1]);
			CONS_Printf("finalz is %d \t(%f)\n", finalz >> FRACBITS, v[2]);
*/
			mobj = P_SpawnMobj(finalx, finaly, finalz, MT_NIGHTSWING);
			mobj->z -= mobj->height/2;
		}
		return;
	}
	else
	{
		if(gameskill >= sk_insane && !((gamemap >= sstage_start && gamemap <= sstage_end) || (mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS))) // No rings in Ultimate!
			return;

		if(mthing->type == 2014) // Your basic ring.
		{
			if(mthing->options & MTF_AMBUSH) // Special flag for rings Tails 06-03-2001
				mthing->z = (R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + 32*FRACUNIT) >> FRACBITS;
			else if((mthing->options >> 4) != 0)
				mthing->z = (R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + ((mthing->options >> 4) << FRACBITS)) >> FRACBITS;
			else
				mthing->z = R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight >> FRACBITS;

			mobj = P_SpawnMobj (mthing->x << FRACBITS, mthing->y << FRACBITS,mthing->z << FRACBITS, MT_RING);
			mobj->spawnpoint = mthing;

			if (mobj->tics > 0)
				mobj->tics = 1 + (P_Random () % mobj->tics);
			mobj->angle = ANGLE_1*mthing->angle;
			mobj->flags |= MF_AMBUSH;
			mthing->mobj = mobj;
		}
		else if(mthing->type == 10005) // Your basic coin.
		{
			if(mthing->options & MTF_AMBUSH) // Special flag for rings Tails 06-03-2001
				mthing->z = (R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + 32*FRACUNIT) >> FRACBITS;
			else if((mthing->options >> 4) != 0)
				mthing->z = (R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + ((mthing->options >> 4) << FRACBITS)) >> FRACBITS;
			else
				mthing->z = R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight >> FRACBITS;

			mobj = P_SpawnMobj (mthing->x << FRACBITS, mthing->y << FRACBITS,mthing->z << FRACBITS, MT_COIN);
			mobj->spawnpoint = mthing;

			if (mobj->tics > 0)
				mobj->tics = 1 + (P_Random () % mobj->tics);
			mobj->angle = ANGLE_1*mthing->angle;
			mobj->flags |= MF_AMBUSH;
			mthing->mobj = mobj;
		}
		else if(mthing->type == 84) // Vertical Rings - Stack of 5 Tails 08-05-2001
		{
			for(r=1; r<6; r++)
			{
				if((mthing->options >> 4) != 0)
					z = (R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + ((mthing->options >> 4) << FRACBITS)) + 64*FRACUNIT*r;
				else
					z = R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + 64*FRACUNIT*r;

				mobj = P_SpawnMobj (x,y,z, MT_RING);
//				mobj->spawnpoint = mthing;

				if (mobj->tics > 0)
				  mobj->tics = 1 + (P_Random () % mobj->tics);

				mobj->angle = ANGLE_1*mthing->angle;
				if (mthing->options & MTF_AMBUSH)
					mobj->flags |= MF_AMBUSH;
			}
		}
		else if(mthing->type == 44) // Vertical Rings - Stack of 5 (suitable for Red Spring) Tails 08-05-2001
		{
			for(r=1; r<6; r++)
			{
				if((mthing->options >> 4) != 0)
					z = (R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + ((mthing->options >> 4) << FRACBITS)) + 128*FRACUNIT*r;
				else
					z = R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + 128*FRACUNIT*r;
				mobj = P_SpawnMobj (x,y,z, MT_RING);
//				mobj->spawnpoint = mthing;

				if (mobj->tics > 0)
				  mobj->tics = 1 + (P_Random () % mobj->tics);

				mobj->angle = ANGLE_1*mthing->angle;
				if (mthing->options & MTF_AMBUSH)
					mobj->flags |= MF_AMBUSH;
			}
		}
		else if(mthing->type == 76) // Diagonal rings (5)
		{
			angle_t angle = ANG45 * (mthing->angle/45);
			angle >>= ANGLETOFINESHIFT;

			for(r=1; r<6; r++)
			{
				x += FixedMul(64*FRACUNIT, finecosine[angle]);
				y += FixedMul(64*FRACUNIT, finesine[angle]);
				if((mthing->options >> 4) != 0)
					z = (R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + ((mthing->options >> 4) << FRACBITS)) + 64*FRACUNIT*r;
				else
					z = R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + 64*FRACUNIT*r;
				mobj = P_SpawnMobj (x,y,z, MT_RING);
//				mobj->spawnpoint = mthing;

				if (mobj->tics > 0)
				  mobj->tics = 1 + (P_Random () % mobj->tics);

				mobj->angle = ANGLE_1*mthing->angle;
				if (mthing->options & MTF_AMBUSH)
					mobj->flags |= MF_AMBUSH;
			}
		}
		else if(mthing->type == 77) // Diagonal rings (10)
		{
			angle_t angle = ANG45 * (mthing->angle/45);
			angle >>= ANGLETOFINESHIFT;

			for(r=1; r<11; r++)
			{
				x += FixedMul(64*FRACUNIT, finecosine[angle]);
				y += FixedMul(64*FRACUNIT, finesine[angle]);
				if((mthing->options >> 4) != 0)
					z = (R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + ((mthing->options >> 4) << FRACBITS)) + 64*FRACUNIT*r;
				else
					z = R_PointInSubsector(mthing->x << FRACBITS, mthing->y << FRACBITS)->sector->floorheight + 64*FRACUNIT*r;
				mobj = P_SpawnMobj (x,y,z, MT_RING);
//				mobj->spawnpoint = mthing;

				if (mobj->tics > 0)
				  mobj->tics = 1 + (P_Random () % mobj->tics);

				mobj->angle = ANGLE_1*mthing->angle;
				if (mthing->options & MTF_AMBUSH)
					mobj->flags |= MF_AMBUSH;
			}
		}
		else if(mthing->type == 47) // A ring of rings (NiGHTS stuff)
		{
			int i;
			TVector v;
			TVector *res;
			fixed_t finalx, finaly, finalz;
			mobj_t* hoopcenter;
			mobj_t* axis;
			short closestangle;
			fixed_t mthingx, mthingy, mthingz;
			degenmobj_t xypos;

			axis = NULL;

			mthingx = mthing->x << FRACBITS;
			mthingy = mthing->y << FRACBITS;

			if((mthing->options >> 4) != 0)
				mthingz = (R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 4) << FRACBITS));
			else
				mthingz = R_PointInSubsector(mthingx, mthing->y << FRACBITS)->sector->floorheight;

			hoopcenter = P_SpawnMobj(mthingx, mthingy, mthingz, MT_DISS);

			axis = P_GetClosestAxis(hoopcenter);

			P_RemoveMobj(hoopcenter);

			if(axis == NULL)
			{
				CONS_Printf("You forgot to put axis points in the map!\n");
				return;
			}

			xypos = P_GimmeAxisXYPos(axis, mthingx, mthingy);

			mthingx = xypos.x;
			mthingy = xypos.y;

//			mthingz += R_PointInSubsector(mthingx, mthingy)->sector->floorheight;

			closestangle = (R_PointToAngle2(mthingx, mthingy, axis->x, axis->y)+ANG90)/ANGLE_1;

			// Create the hoop!
			for(i = 0; i<8; i++)
			{
				v[0] = cos(i*45*deg2rad) * 96;
				v[1] = 0;
				v[2] = sin(i*45*deg2rad) * 96;
				v[3] = 1;

				res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
				memcpy(&v, res, sizeof(v));

				finalx = mthingx + double2fixed(v[0]);
				finaly = mthingy + double2fixed(v[1]);
				finalz = mthingz + double2fixed(v[2]);
	/*
				CONS_Printf("finalx is %d \t(%f)\n", finalx >> FRACBITS, v[0]);
				CONS_Printf("finaly is %d \t(%f)\n", finaly >> FRACBITS, v[1]);
				CONS_Printf("finalz is %d \t(%f)\n", finalz >> FRACBITS, v[2]);
	*/
				mobj = P_SpawnMobj(finalx, finaly, finalz, MT_RING);
				mobj->z -= mobj->height/2;
			}

			return;
		}
		else if(mthing->type == 2007) // A BIGGER ring of rings (NiGHTS stuff)
		{
			int i;
			TVector v;
			TVector *res;
			fixed_t finalx, finaly, finalz;
			mobj_t* hoopcenter;
			mobj_t* axis;
			short closestangle;
			fixed_t mthingx, mthingy, mthingz;
			degenmobj_t xypos;

			axis = NULL;

			mthingx = mthing->x << FRACBITS;
			mthingy = mthing->y << FRACBITS;
				
			if((mthing->options >> 4) != 0)
				mthingz = (R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 4) << FRACBITS));
			else
				mthingz = R_PointInSubsector(mthingx, mthing->y << FRACBITS)->sector->floorheight;

			hoopcenter = P_SpawnMobj(mthingx, mthingy, mthingz, MT_DISS);

			axis = P_GetClosestAxis(hoopcenter);

			P_RemoveMobj(hoopcenter);

			if(axis == NULL)
			{
				CONS_Printf("You forgot to put axis points in the map!\n");
				return;
			}

			xypos = P_GimmeAxisXYPos(axis, mthingx, mthingy);

			mthingx = xypos.x;
			mthingy = xypos.y;

//			mthingz += R_PointInSubsector(mthingx, mthingy)->sector->floorheight;

			closestangle = (R_PointToAngle2(mthingx, mthingy, axis->x, axis->y)+ANG90)/ANGLE_1;

			// Create the hoop!
			for(i = 0; i<16; i++)
			{
				v[0] = cos(i*22.5*deg2rad) * 192;
				v[1] = 0;
				v[2] = sin(i*22.5*deg2rad) * 192;
				v[3] = 1;

				res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
				memcpy(&v, res, sizeof(v));

				finalx = mthingx + double2fixed(v[0]);
				finaly = mthingy + double2fixed(v[1]);
				finalz = mthingz + double2fixed(v[2]);
	/*
				CONS_Printf("finalx is %d \t(%f)\n", finalx >> FRACBITS, v[0]);
				CONS_Printf("finaly is %d \t(%f)\n", finaly >> FRACBITS, v[1]);
				CONS_Printf("finalz is %d \t(%f)\n", finalz >> FRACBITS, v[2]);
	*/
				mobj = P_SpawnMobj(finalx, finaly, finalz, MT_RING);
				mobj->z -= mobj->height/2;
			}

			return;
		}
		else if(mthing->type == 2046) // A ring of rings and wings (alternating) (NiGHTS stuff)
		{
			int i;
			TVector v;
			TVector *res;
			fixed_t finalx, finaly, finalz;
			mobj_t* hoopcenter;
			mobj_t* axis;
			short closestangle;
			fixed_t mthingx, mthingy, mthingz;
			degenmobj_t xypos;

			axis = NULL;

			mthingx = mthing->x << FRACBITS;
			mthingy = mthing->y << FRACBITS;
				
			if((mthing->options >> 4) != 0)
				mthingz = (R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 4) << FRACBITS));
			else
				mthingz = R_PointInSubsector(mthingx, mthing->y << FRACBITS)->sector->floorheight;

			hoopcenter = P_SpawnMobj(mthingx, mthingy, mthingz, MT_DISS);

			axis = P_GetClosestAxis(hoopcenter);

			P_RemoveMobj(hoopcenter);

			if(axis == NULL)
			{
				CONS_Printf("You forgot to put axis points in the map!\n");
				return;
			}

			xypos = P_GimmeAxisXYPos(axis, mthingx, mthingy);

			mthingx = xypos.x;
			mthingy = xypos.y;

//			mthingz += R_PointInSubsector(mthingx, mthingy)->sector->floorheight;

			closestangle = (R_PointToAngle2(mthingx, mthingy, axis->x, axis->y)+ANG90)/ANGLE_1;

			// Create the hoop!
			for(i = 0; i<8; i++)
			{
				v[0] = cos(i*45*deg2rad) * 96;
				v[1] = 0;
				v[2] = sin(i*45*deg2rad) * 96;
				v[3] = 1;

				res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
				memcpy(&v, res, sizeof(v));

				finalx = mthingx + double2fixed(v[0]);
				finaly = mthingy + double2fixed(v[1]);
				finalz = mthingz + double2fixed(v[2]);
	/*
				CONS_Printf("finalx is %d \t(%f)\n", finalx >> FRACBITS, v[0]);
				CONS_Printf("finaly is %d \t(%f)\n", finaly >> FRACBITS, v[1]);
				CONS_Printf("finalz is %d \t(%f)\n", finalz >> FRACBITS, v[2]);
	*/
				if(i & 1)
					mobj = P_SpawnMobj(finalx, finaly, finalz, MT_RING);
				else
					mobj = P_SpawnMobj(finalx, finaly, finalz, MT_NIGHTSWING);

				mobj->z -= mobj->height/2;
			}

			return;
		}
		else if(mthing->type == 2047) // A BIGGER ring of rings and wings (alternating) (NiGHTS stuff)
		{
			int i;
			TVector v;
			TVector *res;
			fixed_t finalx, finaly, finalz;
			mobj_t* hoopcenter;
			mobj_t* axis;
			short closestangle;
			fixed_t mthingx, mthingy, mthingz;
			degenmobj_t xypos;

			axis = NULL;

			mthingx = mthing->x << FRACBITS;
			mthingy = mthing->y << FRACBITS;
				
			if((mthing->options >> 4) != 0)
				mthingz = (R_PointInSubsector(x, y)->sector->floorheight + ((mthing->options >> 4) << FRACBITS));
			else
				mthingz = R_PointInSubsector(mthingx, mthing->y << FRACBITS)->sector->floorheight;

			hoopcenter = P_SpawnMobj(mthingx, mthingy, mthingz, MT_DISS);

			axis = P_GetClosestAxis(hoopcenter);

			P_RemoveMobj(hoopcenter);

			if(axis == NULL)
			{
				CONS_Printf("You forgot to put axis points in the map!\n");
				return;
			}

			xypos = P_GimmeAxisXYPos(axis, mthingx, mthingy);

			mthingx = xypos.x;
			mthingy = xypos.y;

//			mthingz += R_PointInSubsector(mthingx, mthingy)->sector->floorheight;

			closestangle = (R_PointToAngle2(mthingx, mthingy, axis->x, axis->y)+ANG90)/ANGLE_1;

			// Create the hoop!
			for(i = 0; i<16; i++)
			{
				v[0] = cos(i*22.5*deg2rad) * 192;
				v[1] = 0;
				v[2] = sin(i*22.5*deg2rad) * 192;
				v[3] = 1;

				res = VectorMatrixMultiply(v, *RotateZMatrix(closestangle*deg2rad));
				memcpy(&v, res, sizeof(v));

				finalx = mthingx + double2fixed(v[0]);
				finaly = mthingy + double2fixed(v[1]);
				finalz = mthingz + double2fixed(v[2]);
	/*
				CONS_Printf("finalx is %d \t(%f)\n", finalx >> FRACBITS, v[0]);
				CONS_Printf("finaly is %d \t(%f)\n", finaly >> FRACBITS, v[1]);
				CONS_Printf("finalz is %d \t(%f)\n", finalz >> FRACBITS, v[2]);
	*/
				if(i & 1)
					mobj = P_SpawnMobj(finalx, finaly, finalz, MT_RING);
				else
					mobj = P_SpawnMobj(finalx, finaly, finalz, MT_NIGHTSWING);
				mobj->z -= mobj->height/2;
			}

			return;
		}
	}
}

//
// GAME SPAWN FUNCTIONS
//

/* Crummy function Tails 12-05-2001
//
// P_SpawnSplash
//
// when player moves in water
// SoM: Passing the Z height saves extra calculations...
void P_SpawnSplash (mobj_t* mo, fixed_t  z)
                                // flatwater : old water FWATER flat texture
{
    mobj_t*     th;
    //fixed_t     z;
*/
    // we are supposed to be in water sector and my current
    // hack uses negative tag as water height
    /*if (flatwater)
        z = mo->subsector->sector->floorheight + (FRACUNIT/4);
    else
        z = sectors[mo->subsector->sector->heightsec].floorheight; *///SoM: 3/17/2000
/*
    // need to touch the surface because the splashes only appear at surface
    if (mo->z > z || mo->z + mo->height < z)
        return;

    // note pos +1 +1 so it doesn't eat the sound of the player..
    th = P_SpawnMobj (mo->x+1,mo->y+1,z, MT_SPLASH);
    //if( z - mo->subsector->sector->floorheight > 4*FRACUNIT)
        S_StartSound (th, sfx_gloop);
    //else
    //    S_StartSound (th,sfx_splash);
    th->tics -= P_Random()&3;

    if (th->tics < 1)
        th->tics = 1;

*/
    // get rough idea of speed
    /*
    thrust = (mo->momx + mo->momy) >> FRACBITS+1;

    if (thrust >= 2 && thrust<=3)
        P_SetMobjState (th,S_SPLASH2);
    else
    if (thrust < 2)
        P_SetMobjState (th,S_SPLASH3);
    */
//}


// --------------------------------------------------------------------------
// P_SpawnSmoke
// --------------------------------------------------------------------------
// when player gets hurt by lava/slime, spawn at feet
void P_SpawnSmoke ( fixed_t       x,
                    fixed_t       y,
                    fixed_t       z )
{
    mobj_t*     th;

    x = x - ((P_Random()&8) * FRACUNIT) - 4*FRACUNIT;
    y = y - ((P_Random()&8) * FRACUNIT) - 4*FRACUNIT;
    z += (P_Random()&3) * FRACUNIT;


    th = P_SpawnMobj (x,y,z, MT_SMOK);
    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;

    if (th->tics < 1)
        th->tics = 1;
}

//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
boolean P_CheckMissileSpawn (mobj_t* th)
{
	th->tics -= P_Random()&3;
    if (th->tics < 1)
		th->tics = 1;

    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx>>1);
    th->y += (th->momy>>1);
    th->z += (th->momz>>1);

    if (!P_TryMove (th, th->x, th->y, true))
    {
        P_ExplodeMissile (th);
        return false;
    }
    return true;
}

//
// P_SpawnXYZMissile
//
// Spawns missile at specific coords
// Tails 10-28-2002
mobj_t* P_SpawnXYZMissile ( mobj_t*       source,
                         mobj_t*       dest,
                         mobjtype_t    type,
						 fixed_t x,
						 fixed_t y,
						 fixed_t z)
{
    mobj_t*     th;
    angle_t     an;
    int         dist;

#ifdef PARANOIA
    if(!source)
        I_Error("P_SpawnMissile : no source");
    if(!dest)
        I_Error("P_SpawnMissile : no dest");
#endif

    th = P_SpawnMobj (x,
                      y,
                      z, type);

    if (th->info->seesound)
        S_StartSound (th, th->info->seesound);

    th->target = source;        // where it came from
    an = R_PointToAngle2 (x, y, dest->x, dest->y);

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul (th->info->speed, finecosine[an]);
    th->momy = FixedMul (th->info->speed, finesine[an]);

    dist = P_AproxDistance (dest->x - x, dest->y - y);
    dist = dist / th->info->speed;

    if (dist < 1)
        dist = 1;

    th->momz = (dest->z - z) / dist;

    dist = P_CheckMissileSpawn (th);
    return dist ? th : NULL;
}

//
// P_SpawnMissile
//
mobj_t* P_SpawnMissile ( mobj_t*       source,
                         mobj_t*       dest,
                         mobjtype_t    type )
{
    mobj_t*     th;
    angle_t     an;
    int         dist;
    fixed_t     z;

#ifdef PARANOIA
    if(!source)
        I_Error("P_SpawnMissile : no source");
    if(!dest)
        I_Error("P_SpawnMissile : no dest");
#endif
    switch(type)
    {
		case MT_JETTGUNNER:
			z = source->z - 12*FRACUNIT;
			break;
		case MT_TURRET:
			z = source->z + 16*FRACUNIT;
        default:
           z = source->z+32*FRACUNIT;
           break;
    }

    th = P_SpawnMobj (source->x,
                      source->y,
                      z, type);

    if (th->info->seesound)
        S_StartSound (th, th->info->seesound);

    th->target = source;        // where it came from

	if(source->type == MT_TURRET) // More accurate!
		an = R_PointToAngle2 (source->x, source->y, dest->x+(dest->momx*(3*gameskill)), dest->y+(dest->momy*(3*gameskill)));
	else
		an = R_PointToAngle2 (source->x, source->y, dest->x, dest->y);

// Invis shouldn't matter Tails 01-06-2001
/*
    // fuzzy player
    if (dest->flags2 & MF2_SHADOW)
    {
        an += (P_Random()<<20); // WARNING: don't put this in one line 
        an -= (P_Random()<<20); // else this expretion is ambiguous (evaluation order not diffined)
    }
*/

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul (th->info->speed, finecosine[an]);
    th->momy = FixedMul (th->info->speed, finesine[an]);

	if(source->type == MT_TURRET) // More accurate!
		dist = P_AproxDistance (dest->x+(dest->momx*(3*gameskill)) - source->x, dest->y+(dest->momy*(3*gameskill)) - source->y);
	else
		dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);

    dist = dist / th->info->speed;

    if (dist < 1)
        dist = 1;

	if(source->type == MT_TURRET) // More accurate!
		th->momz = (dest->z+(dest->momz*(3*gameskill)) - source->z) / dist;
	else
		th->momz = (dest->z - source->z) / dist;

    dist = P_CheckMissileSpawn (th);
    return dist ? th : NULL;
}


//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//
mobj_t *P_SPMAngle ( mobj_t*       source,
                     mobjtype_t    type,
                     angle_t       angle )
{
    mobj_t*     th;
    angle_t     an;

    fixed_t     x;
    fixed_t     y;
    fixed_t     z;
    fixed_t     slope=0;

    // angle at which you fire, is player angle
    an = angle;

    if (source->player->autoaim_toggle && cv_allowautoaim.value
		&& !source->player->powers[pw_railring])
    {
        // see which target is to be aimed at
        slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

        if (!linetarget)
        {
            an += 1<<26;
            slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);

            if (!linetarget)
            {
                an -= 2<<26;
                slope = P_AimLineAttack (source, an, 16*64*FRACUNIT);
            }
            if (!linetarget)
            {
                an = angle;
                slope = 0;
            }
        }
    }

    //added:18-02-98: if not autoaim, or if the autoaim didnt aim something,
    //                use the mouseaiming
    if ((!(source->player->autoaim_toggle && cv_allowautoaim.value)
                                || (!linetarget)) || source->player->powers[pw_railring])
    {
            slope = AIMINGTOSLOPE(source->player->aiming);
    }

    x = source->x;
    y = source->y;
    z = source->z + source->height/3; // Tails 03-25-2001

    th = P_SpawnMobj (x,y,z, type);

    if (th->info->seesound)
        S_StartSound (th, th->info->seesound);

    th->target = source;

    th->angle = an;
    th->momx = FixedMul( th->info->speed, finecosine[an>>ANGLETOFINESHIFT]);
    th->momy = FixedMul( th->info->speed, finesine[an>>ANGLETOFINESHIFT]);
    
    th->momx = FixedMul(th->momx,finecosine[source->player->aiming>>ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->momy,finecosine[source->player->aiming>>ANGLETOFINESHIFT]);
    th->momz = FixedMul( th->info->speed, slope);

    slope = P_CheckMissileSpawn (th);

    return slope ? th : NULL;
}
