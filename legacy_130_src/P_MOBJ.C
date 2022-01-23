// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_mobj.c,v 1.10 2000/04/16 18:38:07 bpereira Exp $
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
// $Log: p_mobj.c,v $
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

	player_t  *plyr;
	extern consvar_t cv_nights; // Tails 07-02-2001
// protos.
void CV_ViewHeight_OnChange (void);

CV_PossibleValue_t viewheight_cons_t[]={{16,"MIN"},{56,"MAX"},{0,NULL}};

consvar_t cv_viewheight = {"viewheight","41",0,viewheight_cons_t,NULL};

//Fab:26-07-98:
consvar_t cv_gravity = {"gravity","0.5",CV_NETVAR|CV_FLOAT|CV_SHOWMODIF}; // No longer required in autoexec.cfg! Tails 12-01-99
consvar_t cv_splats  = {"splats","1",CV_SAVE,CV_OnOff};

//
// P_SetMobjState
// Returns true if the mobj is still present.
//
//SoM: 4/7/2000: Boom code...
boolean P_SetMobjState ( mobj_t*       mobj,
                         statenum_t    state )
{
  state_t*  st;

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


//
// P_ExplodeMissile
//
// Added some stuff here Tails 08-26-2001
void P_ExplodeMissile (mobj_t* mo)
{
	mobj_t*	explodemo;

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


//
// P_XYMovement
//
#define STOPSPEED               0xffff
#define FRICTION                0xe800   //0.90625

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
            P_SetMobjState (player->mo, S_PLAY);
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
	else if (!player)
    {
		if (mo->momx > -STOPSPEED
        && mo->momx < STOPSPEED
        && mo->momy > -STOPSPEED
        && mo->momy < STOPSPEED)
    {
        // if in a walking frame, stop moving
        if ( (player && player->walking==1) && (mo->type!=MT_SPIRIT)) // use my new walking variable! Tails 10-30-2000
           {
/*           if(player->powers[pw_super])
		   {
            P_SetMobjState (player->mo, S_PLAY_SPC1); // Super Sonic Stuff Tails 04-18-2000
		   }
           else*/
            P_SetMobjState (player->mo, S_PLAY);
                        }
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
}}



void P_XYMovement (mobj_t* mo)
{
    fixed_t     ptryx;
    fixed_t     ptryy;
    player_t*   player;
    fixed_t     xmove;
    fixed_t     ymove;
    fixed_t     oldx, oldy; //reducing bobbing/momentum on ice
                            //when up against walls

	if((mo->type == MT_REDFLAG || mo->type == MT_BLUEFLAG) && mo->subsector->sector->special == 16 && mo->z == mo->floorz) // Remove CTF flag if in death pit
		mo->fuse = 1; // Tails 08-02-2001

    //added:18-02-98: if it's stopped
    if (!mo->momx && !mo->momy)
    {
        if (mo->flags & MF_SKULLFLY)
        {
            // the skull slammed into something
            mo->flags &= ~MF_SKULLFLY;
            mo->momx = mo->momy = mo->momz = 0;

            //added:18-02-98: comment: set in 'search new direction' state?
			if(mo->type != MT_EGGMOBILE)
            P_SetMobjState (mo, mo->info->spawnstate);
        }
        return;
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

            //added:26-02-98: slidemove also for ChaseCam
            // note : the SPIRIT have a valide player field
            if (mo->player || mo->type==MT_CHASECAM || mo->type == MT_JETTBOMBER || mo->type == MT_JETTGUNNER || mo->type == MT_MISC2 || mo->type == MT_FLINGRING || mo->type == MT_GARGOYLE) // Tails 08-18-2001
            {   // try to slide along it
                P_SlideMove (mo);
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
                  if (!boomsupport ||
                    mo->z > ceilingline->backsector->ceilingheight)//SoM: 4/7/2000: DEMO'S
                {
                    // Hack to prevent missiles exploding
                    // against the sky.
                    // Does not handle sky floors.
                    //SoM: 4/3/2000: Check frontsector as well..
                    P_RemoveMobj (mo);
                    return;
                }

                // draw damage on wall
                //SPLAT TEST ----------------------------------------------------------
                #ifdef WALLSPLATS
                if (blockingline && demoversion>=129)   //set by last P_TryMove() that failed
                {
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
            else
                mo->momx = mo->momy = 0;
        }
        else
            // hack for playability : walk in-air to jump over a small wall
            if (mo->player)
                mo->player->cheats &= ~CF_JUMPOVER;


    } while (xmove || ymove);

    // slow down
    if (player)
    {
        if (player->cheats & CF_NOMOMENTUM || cv_nights.value)
        {
            // debug option for no sliding at all
            mo->momx = mo->momy = 0;
            return;
        }
        else
        if (player->cheats & CF_FLYAROUND)
        {
            P_XYFriction (mo, oldx, oldy, true);
            return;
        }
        if(mo->friction != ORIG_FRICTION && mo->z <= mo->subsector->sector->floorheight)
          P_XYFriction (mo, oldx, oldy, false);
    }

    if ((mo->flags & (MF_MISSILE | MF_SKULLFLY) || mo->type == MT_SNOWBALL) && !mo->type == MT_DETON) // Tails 12-12-2001
        return;         // no friction for missiles ever

	if(mo->player) // No Friction
		if(mo->player->homing) // For Homing
			return; // Tails 09-02-2001

    // slow down in water, not too much for playability issues
/*    if (demoversion>=128 && (mo->eflags & MF_UNDERWATER))
    {
        mo->momx = FixedMul (mo->momx, FRICTION*3/4);
        mo->momy = FixedMul (mo->momy, FRICTION*3/4);
        return;
    }
*/
// start spinning friction Tails 02-28-2000
if(player)
{
   if (player->mfspinning == 1 && (player->rmomx || player->rmomy) && !player->mfstartdash)
      {
        mo->momx = FixedMul (mo->momx, FRICTION*1.1);
        mo->momy = FixedMul (mo->momy, FRICTION*1.1);
        return;
       }
}
// end spinning friction Tails 02-28-2000
// Ice on a ledge Tails 11-29-2000

 if (mo->subsector->sector->ffloors)
   {
	 		ffloor_t* rover;
	         for(rover = mo->subsector->sector->ffloors; rover; rover = rover->next)
			 {
	   if(mo->z == *rover->topheight && !mo->momz && *rover->special == 256)
	   {
		mo->momx = FixedMul (mo->momx, FRICTION*1.1);
        mo->momy = FixedMul (mo->momy, FRICTION*1.1);
		   return;
		}
   }
 }
     if (mo->z > mo->floorz && mo->type != MT_CRAWLACOMMANDER)
        return;         // no friction when airborne

    if (mo->flags & MF_CORPSE)
    {
        // do not stop sliding
        //  if halfway off a step with some momentum
        if (mo->momx > FRACUNIT/4
            || mo->momx < -FRACUNIT/4
            || mo->momy > FRACUNIT/4
            || mo->momy < -FRACUNIT/4)
        {
            if (mo->floorz != mo->subsector->sector->floorheight)
                return;
        }
    }
    P_XYFriction (mo, oldx, oldy, true);
}

//
// P_ZMovement
//
void P_ZMovement (mobj_t* mo)
{
    fixed_t     dist;
    fixed_t     delta;

    // check for smooth step up
#ifdef CLIENTPREDICTION2
    if (mo->player && mo->z < mo->floorz && mo->type!=MT_PLAYER)
#else
    if (mo->player && mo->z < mo->floorz && mo->type!=MT_SPIRIT)
#endif
    {
        mo->player->viewheight -= mo->floorz - mo->z;

        mo->player->deltaviewheight
            = ((cv_viewheight.value<<FRACBITS) - mo->player->viewheight)>>3;
    }

	// Snowflake Tails 12-02-2001
	if(mo->type == MT_SNOWFLAKE)
	{
		if(mo->z + mo->momz <= mo->floorz || mo->z < mo->waterz)
			P_RemoveMobj(mo);
		else
			mo->momz = -2*FRACUNIT;
	}

    // adjust height
    mo->z += mo->momz;

	// Ignore still rings Tails 09-02-2001
	if(mo->type == MT_MISC2 && !(mo->momx || mo->momy || mo->momz))
		return;

	// Have player fall through floor? 10-05-2001 Tails
	if(mo->player && mo->player->playerstate == PST_DEAD)
		goto playergravity;

    if ( mo->flags & MF_FLOAT
         && mo->target && mo->health && !(mo->type == MT_DETON || mo->type == MT_JETTBOMBER || mo->type == MT_JETTGUNNER || mo->type == MT_CRAWLACOMMANDER)) // Tails 07-21-2001
    {
        // float down towards target if too close
        if ( !(mo->flags & MF_SKULLFLY)
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

        // Note (id):
        //  somebody left this after the setting momz to 0,
        //  kinda useless there.
        if (mo->flags & MF_SKULLFLY)
        {
            // the skull slammed into something
            mo->momz = -mo->momz;
        }

// start spin Tails
/*        if (mo->player)
        {
         if(!(mo->player->rmomx || mo->player->rmomy) && mo->player->mfspinning && mo->health)
         {
          mo->player->mfspinning = 0;
       if(mo->player->powers[pw_super])
          P_SetMobjState (mo, S_PLAY_SPC1); // Super Sonic Stuff Tails 04-18-2000
       else
          P_SetMobjState (mo, S_PLAY);
         }
        }*/
// end spin Tails

// Mine explodes upon ground contact Tails 06-13-2000
if((mo->type==MT_MINE) && (mo->z <= mo->floorz) && !(mo->state == &states[S_MINE_BOOM1]
   || mo->state == &states[S_MINE_BOOM2] || mo->state == &states[S_MINE_BOOM3]
   || mo->state == &states[S_MINE_BOOM4] || mo->state == &states[S_DISS]))
{
  P_ExplodeMissile(mo);
}

        if (mo->momz < 0) // falling
        {
            if (mo->player && (mo->momz < -8*FRACUNIT))
            {
                // Squat down.
                // Decrease viewheight for a moment
                // after hitting the ground (hard),
                // and utter appropriate sound.
                mo->player->deltaviewheight = mo->momz>>3;
 //               S_StartSound (mo, sfx_spring); Don't say OOF!! Tails 11-05-99
            }

            // set it once and not continuously
// Tails
			if(tmfloorthing)
			{
				// Bouncing boxes Tails 09-28-2001
				if(tmfloorthing->z > tmfloorthing->floorz)
				{
					switch(mo->type)
					{
						case MT_GARGOYLE: // Deep Sea Gargoyle
						case MT_MISC50: // Blue shield box
						case MT_MISC48: // Yellow shield box
						case MT_MISC31: // Green shield box
						case MT_BKTV: // Black shield box
						case MT_MISC74: // Super Sneaker box
						case MT_PRUP: // 1-Up box
						case MT_MISC10: // 10-Ring box
						case MT_MISC11: // 25-Ring box
						case MT_INV: // Invincibility box
							mo->momz = 4*FRACUNIT;
							break;
						default:
							break;
					}
				}
			switch(tmfloorthing->type)
			{
				case MT_GARGOYLE: // Deep Sea Gargoyle
				case MT_MISC50: // Blue shield box
				case MT_MISC48: // Yellow shield box
				case MT_MISC31: // Green shield box
				case MT_BKTV: // Black shield box
				case MT_MISC74: // Super Sneaker box
				case MT_PRUP: // 1-Up box
				case MT_MISC10: // 10-Ring box
				case MT_MISC11: // 25-Ring box
				case MT_INV: // Invincibility box
				if(mo->player)
				{
					if(!(mo->player->mfjumped))
					tmfloorthing = 0;
				}
					break;
				default:
					break;
			}
			}
            if ((mo->z <= mo->floorz) && !(tmfloorthing)) // Tails 9-15-99 Spin Attack
              {
              mo->eflags |= MF_JUSTHITFLOOR; // Tails 9-15-99 Spin Attack
			if(mo->player)
			{
			mo->player->scoreadd = 0; // Tails 11-03-2000
              if(mo->player->mfjumped == 1) // Tails 9-15-99 Spin Attack
			  {
              mo->player->mfjumped = 0; // Tails 9-15-99 Spin Attack
			  }
			  mo->player->gliding = 0;
			  mo->player->glidetime = 0;
			  mo->player->climbing = 0;
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

            //SOM: Flingrings bounce
            if(mo->type == MT_FLINGRING)
			{
              mo->momz = -mo->momz * 0.85;
			}
            else if (!(tmfloorthing) || (tmfloorthing->type == MT_GARGOYLE || tmfloorthing->type == MT_PLAYER))
              mo->momz = 0;
//		if(!(tmfloorthing))
//            mo->momz = 0;
        }

	if(mo->type == MT_STEAM) // Tails 05-29-2001
		return; // Tails 05-29-2001

        mo->z = mo->floorz;


        if ( (mo->flags & MF_MISSILE)
             && !(mo->flags & MF_NOCLIP) )
        {
            P_ExplodeMissile (mo);
            return;
        }
    }
    else if (! (mo->flags & MF_NOGRAVITY) )             // Gravity here!
    {
        fixed_t     gravityadd;
        
        //Fab: NOT SURE WHETHER IT IS USEFUL, just put it here too
        //     TO BE SURE there is no problem for the release..
        //     (this is done in P_Mobjthinker below normally)
        mo->eflags &= ~MF_JUSTHITFLOOR;
/*
        if (mo->momz == 0)
            mo->momz = -cv_gravity.value*2;      // push down
        else
            mo->momz -= cv_gravity.value;        // accelerate fall
*/
        gravityadd = -cv_gravity.value;

		if(mo->eflags & MF_UNDERWATER) // Tails
			gravityadd = -cv_gravity.value/3; // Tails

        // if waist under water, slow down the fall
/*        if ( mo->eflags & MF_UNDERWATER) {
            if ( mo->eflags & MF_SWIMMING )
                gravityadd = 0;     // gameplay: no gravity while swimming
            else
                gravityadd >>= 1; // proper gravity in water Tails 04-04-2000
        }
        else*/ if (mo->momz==0)
            // mobj at stop, no floor, so feel the push of gravity!
            gravityadd <<= 1;

playergravity:
		if(mo->player)
		{
			if ((mo->player->charability==1) && ((mo->player->powers[pw_tailsfly]) || (mo->player->mo->state == &states[S_PLAY_SPC1]) || (mo->player->mo->state == &states[S_PLAY_SPC2]) || (mo->player->mo->state == &states[S_PLAY_SPC3]) || (mo->player->mo->state == &states[S_PLAY_SPC4])))
				gravityadd = -cv_gravity.value/3; // less gravity while flying
			if(mo->player->gliding)
				gravityadd = -cv_gravity.value/3; // less gravity while gliding
			if(mo->player->climbing)
				gravityadd = 0;
			if(cv_nights.value)
				gravityadd = -cv_gravity.value/10; // Tails 07-02-2001
			if(mo->player->playerstate == PST_DEAD) // Added crash check Tails 11-16-2001)
			{
				gravityadd = -cv_gravity.value;
				mo->momz += gravityadd;
				return;
			}
		}

			mo->momz += gravityadd;
    }

    if (mo->z + mo->height > mo->ceilingz)
    {
        mo->z = mo->ceilingz - mo->height;

        //added:22-02-98: player avatar hits his head on the ceiling, ouch!
/*        if (mo->player && (demoversion>=112)  
            && !(mo->player->cheats & CF_FLYAROUND) && mo->momz>8*FRACUNIT )
            S_StartSound (mo, sfx_gasp);
*/ // Don't say ouch Tails
        // hit the ceiling
        if (mo->momz > 0)
			mo->momz = 0;

        if (mo->flags & MF_SKULLFLY)
        {       // the skull slammed into something
            mo->momz = -mo->momz;
        }

        if ( (mo->flags & MF_MISSILE)
             && !(mo->flags & MF_NOCLIP) )
        {
            //SoM: 4/3/2000: Don't explode on the sky!
            if(demoversion >= 129 && mo->subsector->sector->ceilingpic == skyflatnum &&
               mo->subsector->sector->ceilingheight == mo->ceilingz)
            {
              P_RemoveMobj(mo);
              return;
            }

            P_ExplodeMissile (mo);
            return;
        }
    }

    // z friction in water
/*    if (demoversion>=128 && 
        ((mo->eflags & MF_TOUCHWATER) || (mo->eflags & MF_UNDERWATER)) && 
        !(mo->flags & (MF_MISSILE | MF_SKULLFLY)))
    {
         mo->momz = FixedMul (mo->momz, FRICTION*1.02); // Tails 9-24-99
    }
*/
}



//
// P_NightmareRespawn
//
void
P_NightmareRespawn (mobj_t* mobj)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
    subsector_t*        ss;
    mobj_t*             mo;
    mapthing_t*         mthing;

    x = mobj->spawnpoint->x << FRACBITS;
    y = mobj->spawnpoint->y << FRACBITS;

    // somthing is occupying it's position?
    if (!P_CheckPosition (mobj, x, y) )
        return; // no respwan

    // spawn a teleport fog at old spot
    // because of removal of the body?
    mo = P_SpawnMobj (mobj->x,
                      mobj->y,
                      mobj->subsector->sector->floorheight , MT_TFOG);
    // initiate teleport sound
    S_StartSound (mo, sfx_telept);

    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector (x,y);

    mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_TFOG);

    S_StartSound (mo, sfx_telept);

    // spawn the new monster
    mthing = mobj->spawnpoint;

    // spawn it
    if (mobj->info->flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;

    // inherit attributes from deceased one
    mo = P_SpawnMobj (x,y,z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;
    mo->angle = ANG45 * (mthing->angle/45);

    if (mthing->options & MTF_AMBUSH)
        mo->flags |= MF_AMBUSH;

    mo->reactiontime = 18;

    // remove the old monster,
    P_RemoveMobj (mobj);
}


consvar_t cv_respawnmonsters = {"respawnmonsters","0",CV_NETVAR,CV_OnOff};
consvar_t cv_respawnmonsterstime = {"respawnmonsterstime","12",CV_NETVAR,CV_Unsigned};


//
// P_MobjCheckWater : check for water, set stuff in mobj_t struct for
//                    movement code later, this is called either by P_MobjThinker() or P_PlayerThink()
void P_MobjCheckWater (mobj_t* mobj)
{
    sector_t* sector;
    fixed_t   z;
    int       oldeflags;

    if( demoversion<128 || mobj->type==MT_SPLASH) // splash don't do splash
        return;
    //
    // see if we are in water, and set some flags for later
    //
    sector = mobj->subsector->sector;
    oldeflags = mobj->eflags;

    //SoM: 3/28/2000: Only use 270 water type of water. Some boom levels get messed up.
    if ((sector->heightsec > -1 && sector->altheightsec == 1) ||
        (levelflats[sector->floorpic].iswater && sector->heightsec == -1))
    {
        if (sector->heightsec > -1)  //water hack
            z = (sectors[sector->heightsec].floorheight);
        else
            z = sector->floorheight + (FRACUNIT/4); // water texture

        if (z && mobj->z+(mobj->height>>1) <= z) // Added crash check Tails 11-16-2001
        { // Tails 03-06-2000
            mobj->eflags |= MF_UNDERWATER;
			if(mobj->player)
			{
         if(!((mobj->player->powers[pw_super]) || (mobj->player->powers[pw_invulnerability])))
            mobj->player->powers[pw_yellowshield] = false;
        if (mobj->player->powers[pw_underwater] <= 0 && !(mobj->player->powers[pw_greenshield])) // Tails 03-06-2000
            {// Tails 03-06-2000
            mobj->player->powers[pw_underwater] = 30*TICRATE + 1; // Tails 03-06-2000
            }// Tails 03-06-2000
			}
		}
        else
         {
            mobj->eflags &= ~MF_UNDERWATER;
          } // Tails 03-06-2000 (I guess I'm just comment-happy today!)

    }
    else if(sector->ffloors)
    {
      ffloor_t*  rover;

      mobj->eflags &= ~(MF_UNDERWATER|MF_TOUCHWATER);

      for(rover = sector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SWIMMABLE) || rover->flags & FF_SOLID)
          continue;
        if(*rover->topheight < mobj->z || *rover->bottomheight > (mobj->z + (mobj->height / 2)))
          continue;

        if(mobj->z + mobj->height > *rover->topheight)
            mobj->eflags |= MF_TOUCHWATER;
        else
            mobj->eflags &= ~MF_TOUCHWATER;

        if(mobj->z + mobj->height < *rover->topheight)
		{ // Tails
            mobj->eflags |= MF_UNDERWATER;
			if(mobj->player)
			{
         if(!((mobj->player->powers[pw_super]) || (mobj->player->powers[pw_invulnerability])))
            mobj->player->powers[pw_yellowshield] = false;
        if (mobj->player->powers[pw_underwater] <= 0 && !(mobj->player->powers[pw_greenshield])) // Tails 03-06-2000
            {// Tails 03-06-2000
            mobj->player->powers[pw_underwater] = 30*TICRATE + 1; // Tails 03-06-2000
            }// Tails 03-06-2000
			}
		} // Tails
        else
            mobj->eflags &= ~MF_UNDERWATER;
/*
        if(  !(oldeflags & (MF_TOUCHWATER|MF_UNDERWATER))
           && ((mobj->eflags & MF_TOUCHWATER) ||
               (mobj->eflags & MF_UNDERWATER)    )
           && mobj->type != MT_BLOOD)
            P_SpawnSplash (mobj, *rover->topheight); */ // Tails 12-05-2001
      }
      return;
    }
    else
        mobj->eflags &= ~(MF_UNDERWATER|MF_TOUCHWATER);
/*
    // blood doesnt make noise when it falls in water
    if(  !(oldeflags & (MF_TOUCHWATER|MF_UNDERWATER)) 
       && ((mobj->eflags & MF_TOUCHWATER) || 
           (mobj->eflags & MF_UNDERWATER)    )         
      && mobj->type == MT_PLAYER) // Tails 04-04-2000
        P_SpawnSplash (mobj, z); //SoM: 3/17/2000
*/
// Return of WaterZ! Tails 10-31-2000
if(mobj->subsector->sector->heightsec != -1 && mobj->subsector->sector->altheightsec == 1)
  mobj->waterz = sectors[mobj->subsector->sector->heightsec].floorheight;
else
mobj->waterz = mobj->floorz - 10000*FRACUNIT;

}

//
// P_MobjThinker
//
void P_MobjThinker (mobj_t* mobj)
{
    boolean   checkedpos = false;  //added:22-02-98:

    // check mobj against possible water content, before movement code
    P_MobjCheckWater (mobj);

// Start Level end sign stuff Tails 01-14-2001

if(mobj->type == MT_SIGN)
{
//for (i=0 ; i< MAXPLAYERS ; i++)
//{

	if(plyr->exiting)
	{
		if (mobj->state == &states[S_SIGN49])
		{
			P_SetMobjState (mobj, S_SIGN1);
			S_StartSound(mobj, sfx_lvpass);
		}
	}
//}
}

// End Level end sign stuff Tails 01-14-2001

// Fans spawn bubbles underwater Tails 02-28-2001
if(mobj->type == MT_MISC34 && mobj->z + mobj->height < mobj->waterz)
{
	if(!(P_Random() % 16))
	{
		P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_SMALLBUBBLE);
	}
	if(!(P_Random() % 96))
	{
		P_SpawnMobj (mobj->x, mobj->y, mobj->z + (mobj->height / 1.25), MT_MEDIUMBUBBLE);
	}
}

    if(mobj->player)
	{
		if(mobj->eflags & MF_JUSTHITFLOOR && mobj->z==mobj->floorz && mobj->health)
    {
  if(false/*mobj->player->powers[pw_super]*/)
  {
	  if(mobj->player->rmomx || mobj->player->rmomy)
     P_SetMobjState (mobj, S_PLAY_ABL1);  // Super Sonic Stuff Tails 04-18-2000
	  else
     P_SetMobjState (mobj, S_PLAY_SPC1);  // Super Sonic Stuff Tails 04-18-2000
  }
  else
  {
	  if(mobj->player->cmomx || mobj->player->cmomy)
	  {
	  if(mobj->player->speed > 18 && !mobj->player->running)
	 P_SetMobjState (mobj, S_PLAY_SPD1);
	  else if ((mobj->player->rmomx > STOPSPEED || mobj->player->rmomy > STOPSPEED) && (mobj->player->cmomx || mobj->player->cmomy) && !mobj->player->walking)
     P_SetMobjState (mobj, S_PLAY_RUN1);
	  else if ((mobj->momx > STOPSPEED || mobj->momy > STOPSPEED) && !mobj->player->walking)
     P_SetMobjState (mobj, S_PLAY_RUN1);
	  else if ((mobj->player->rmomx < FRACUNIT || mobj->player->rmomy < FRACUNIT) && (mobj->player->cmomx || mobj->player->cmomy) && !(mobj->player->walking || mobj->player->running))
     P_SetMobjState (mobj, S_PLAY);
	  }
	else
	{
	  if(mobj->player->speed > 18 && !mobj->player->running)
	 P_SetMobjState (mobj, S_PLAY_SPD1);
	  else if ((mobj->momx || mobj->momy) && !mobj->player->walking)
     P_SetMobjState (mobj, S_PLAY_RUN1);
	  else if (!(mobj->momx && mobj->momy) && !(mobj->player->walking || mobj->player->running))
     P_SetMobjState (mobj, S_PLAY);
	}
  }

      if(mobj->player->mfjumped == 1)
      {
       mobj->player->mfjumped = 0;
      }
	  if(mobj->player->mfspinning == 1)
	  {
		  mobj->player->mfspinning = 0;
	  }
	  mobj->player->gliding = 0;
	  mobj->player->glidetime = 0;
	  mobj->player->climbing = 0;
			}
		}
	
    //SOM: Check fuse
    if(mobj->fuse) {
      mobj->fuse--;
      if(!mobj->fuse) {

		  subsector_t* ss;
		  fixed_t             x;
		  fixed_t             y;
		  fixed_t             z;
		  mobj_t*			flagmo;
		if(mobj->type == MT_BLUEFLAG)
		{
			x = mobj->spawnpoint->x << FRACBITS;
			y = mobj->spawnpoint->y << FRACBITS;
			ss = R_PointInSubsector(x, y);
			z = ss->sector->floorheight;
			flagmo = P_SpawnMobj(x, y, z, MT_BLUEFLAG);
			flagmo->spawnpoint = mobj->spawnpoint;
		}
		else if(mobj->type == MT_REDFLAG)
		{
			x = mobj->spawnpoint->x << FRACBITS;
			y = mobj->spawnpoint->y << FRACBITS;
			ss = R_PointInSubsector(x, y);
			z = ss->sector->floorheight;
			flagmo = P_SpawnMobj(x, y, z, MT_REDFLAG);
			flagmo->spawnpoint = mobj->spawnpoint;
		}
		switch(mobj->type)
		{
				case MT_MISC50: // Blue shield box
				case MT_MISC48: // Yellow shield box
				case MT_MISC31: // Green shield box
				case MT_BKTV: // Black shield box
				case MT_MISC74: // Super Sneaker box
				case MT_PRUP: // 1-Up box
				case MT_MISC10: // 10-Ring box
				case MT_MISC11: // 25-Ring box
				case MT_INV: // Invincibility box
					P_SetMobjState(mobj, S_DISS); // make sure they dissapear tails
					break;
				default:
					if(mobj->info->deathstate)
						P_ExplodeMissile(mobj);
					else
						P_SetMobjState(mobj, S_DISS); // make sure they dissapear tails
					break;
		}
        }
      }

    //
    // momentum movement
    //
#ifdef CLIENTPREDICTION2
    if((mobj->type==MT_PLAYER) && (mobj->player) && 
        (mobj->player->cmd.angleturn&(TICCMD_XY|TICCMD_RECEIVED)==(TICCMD_XY|TICCMD_RECEIVED)) && 
        (mobj->player->playerstate == PST_LIVE))
    {
        if( mobj->x!=mobj->player->cmd.x || mobj->y!=mobj->player->cmd.y )
        {
            mobj->eflags |= MF_NOZCHECKING;
            // cross special lines and pick up things
            if(!P_TryMove (mobj, mobj->player->cmd.x, mobj->player->cmd.y, true))
            {
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
        P_XYFriction (mobj);

    }
    else
#endif
    if ( mobj->momx ||
         mobj->momy ||
        (mobj->flags&MF_SKULLFLY) )
    {
        P_XYMovement (mobj);
        checkedpos = true;

        // FIXME: decent NOP/NULL/Nil function pointer please.
        if ((mobj->thinker.function.acv == (actionf_v) (-1)))
            return;             // mobj was removed
    }

    //added:28-02-98: always do the gravity bit now, that's simpler
    //                BUT CheckPosition only if wasn't do before.
    if ( !( (mobj->eflags & MF_ONGROUND) &&
            (mobj->z == mobj->floorz) &&
            !mobj->momz
          ) )
    {
        // if didnt check things Z while XYMovement, do the necessary now
        if (!checkedpos && (demoversion>=112))
        {
            // FIXME : should check only with things, not lines
            P_CheckPosition (mobj, mobj->x, mobj->y);

            /* ============ BIG DIRTY MESS : FIXME FAB!!! =============== */
            mobj->floorz = tmfloorz;
            mobj->ceilingz = tmceilingz;
            if (tmfloorthing)
                mobj->eflags &= ~MF_ONGROUND;  //not on real floor
            else
                mobj->eflags |= MF_ONGROUND;
            /* ============ BIG DIRTY MESS : FIXME FAB!!! =============== */

            // now mobj->floorz should be the current sector's z floor
            // or a valid thing's top z
        }

        P_ZMovement (mobj);

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
        if (!cv_respawnmonsters.value)
            return;

        // check for nightmare respawn
        if (! (mobj->flags & MF_COUNTKILL) )
            return;

        mobj->movecount++;

        if (mobj->movecount < cv_respawnmonsterstime.value*TICRATE)
            return;

        if ( leveltime&31 )
            return;

        if (P_Random () > 4)
            return;

        P_NightmareRespawn (mobj);
    }

if(mobj->type == MT_EGGMOBILE && mobj->health < 3 && leveltime & 1 && mobj->health > 0)
	P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_SMOK);

if(mobj->type == MT_EGGMOBILE && mobj->flags & MF_SKULLFLY)
	P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_GRTHOK);

// Some black shield code Tails 04-08-2000
if (mobj->type==MT_BFG)
    P_SetMobjState (mobj, S_BFGLAND3);

if (mobj->state == &states[S_BFGLAND3])
    P_SetMobjState (mobj, S_DISS);

// start bubble dissipate Tails
  if((mobj->type==MT_SMALLBUBBLE || mobj->type==MT_MEDIUMBUBBLE || mobj->type==MT_EXTRALARGEBUBBLE) && (mobj->z >= mobj->waterz || mobj->z + mobj->height >= mobj->ceilingz))
   {
     P_SetMobjState (mobj, S_DISS);
   }
// end bubble dissipate Tails

// start make sure player shows dead Tails 03-15-2000
if(mobj->player)
{
   if(mobj->health <= 0)
   {
     P_SetMobjState (mobj, S_PLAY_DIE3);
   }
}
// end make sure player shows dead Tails 03-15-2000

// Keep Skim at water surface Tails 06-13-2000
if((mobj->type==MT_SKIM) && ((mobj->z > mobj->waterz) || (mobj->z < mobj->waterz)))
mobj->z = mobj->waterz;

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
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags = info->flags;

    mobj->health = info->spawnhealth;

    if (gameskill != sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    // added 4-9-98: dont get out of synch
    if (mobj->type == MT_SPIRIT || mobj->type == MT_CHASECAM)
        mobj->lastlook = 0;
    else
        if( demoversion<129 )
            mobj->lastlook = P_Random () % MAXPLAYERS;
        else
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

		/*        if (mobj->flags & MF_NOBLOOD)
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
		if((mobj->type == MT_MISC2 && mobj->flags & MF_AMBUSH) || mobj->type == MT_DETON || mobj->type == MT_JETTBOMBER || mobj->type == MT_JETTGUNNER) // Special flag for rings Tails 06-03-2001
			mobj->z = mobj->floorz + 32*FRACUNIT;
		else
            mobj->z = mobj->floorz;

    }
    else if (z == ONCEILINGZ)
        mobj->z = mobj->ceilingz - mobj->height;
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


//
// P_RemoveMobj
//
mapthing_t     *itemrespawnque[ITEMQUESIZE];
int             itemrespawntime[ITEMQUESIZE];
int             iquehead;
int             iquetail;

void P_RemoveMobj (mobj_t* mobj)
{
	int random; // Tails 08-09-2001

		switch(mobj->type)
		{
				case MT_MISC50: // Blue shield box
				case MT_MISC48: // Yellow shield box
				case MT_MISC31: // Green shield box
				case MT_BKTV: // Black shield box
				case MT_MISC74: // Super Sneaker box
				case MT_PRUP: // 1-Up box
				case MT_MISC10: // 10-Ring box
				case MT_MISC11: // 25-Ring box
				case MT_INV: // Invincibility box
					random = P_Random() / 32;
					if(random == 0)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC50);
					else if(random == 1)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC48);
					else if(random == 2)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC31);
					else if(random == 3)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_BKTV);
					else if(random == 4)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC74);
					else if(random == 5)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC10);
					else if(random == 6)
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_MISC11);
					else
						P_SpawnMobj(mobj->x, mobj->y, mobj->z, MT_INV);
					break;
				case MT_MISC2:
					itemrespawnque[iquehead] = mobj->spawnpoint;
					itemrespawntime[iquehead] = leveltime;
					iquehead = (iquehead+1)&(ITEMQUESIZE-1);
					// lose one off the end?
					if (iquehead == iquetail)
						iquetail = (iquetail+1)&(ITEMQUESIZE-1);
					break;
				default:
					break;
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


consvar_t cv_itemrespawntime={"respawnitemtime","30",CV_NETVAR,CV_Unsigned};
consvar_t cv_itemrespawn    ={"respawnitem"    , "0",CV_NETVAR,CV_OnOff};
consvar_t cv_flagtime={"flagtime","30",CV_NETVAR,CV_Unsigned}; // Tails 08-03-2001

//
// P_RespawnSpecials
//
void P_RespawnSpecials (void)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    subsector_t*        ss;
    mobj_t*             mo;
    mapthing_t*         mthing;

    int                 i;

    // only respawn items in deathmatch
    if (!cv_itemrespawn.value || !netgame)
        return; //

    // nothing left to respawn?
    if (iquehead == iquetail)
        return;

    // the first item in the queue is the first to respawn
    // wait at least 30 seconds
    if (leveltime - itemrespawntime[iquetail] < cv_itemrespawntime.value*TICRATE)
        return;

    mthing = itemrespawnque[iquetail];

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    // spawn a teleport fog at the new spot
    ss = R_PointInSubsector (x,y);
    mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_IFOG);
    S_StartSound (mo, sfx_itmbk);

    // find which type to spawn
    for (i=0 ; i< NUMMOBJTYPES ; i++)
    {
        if (mthing->type == mobjinfo[i].doomednum)
            break;
    }

    // spawn it
    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
	else if(mthing->type == 2014 && mthing->options & MTF_AMBUSH) // Tails 08-05-2001
		z = ss->sector->floorheight + 32*FRACUNIT; // Tails 08-05-2001
    else
        z = ONFLOORZ;

    mo = P_SpawnMobj (x,y,z, i);
    mo->spawnpoint = mthing;
    mo->angle = ANG45 * (mthing->angle/45);

    // pull it from the que
    iquetail = (iquetail+1)&(ITEMQUESIZE-1);
}

// used when we are going from deathmatch 2 to deathmatch 1
void P_RespawnWeapons(void)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    subsector_t*        ss;
    mobj_t*             mo;
    mapthing_t*         mthing;

    int                 i,j,freeslot;

    freeslot=iquetail;
    for(j=iquetail;j!=iquehead;j=(j+1)&(ITEMQUESIZE-1))
    {
        mthing = itemrespawnque[j];

        i=0;
        switch(mthing->type) {
            case 2001 : //mobjinfo[MT_SHOTGUN].doomednum  :
                 i=MT_SHOTGUN;
                 break;
            case 82   : //mobjinfo[MT_SUPERSHOTGUN].doomednum :
                 i=MT_SUPERSHOTGUN;
                 break;
            case 2002 : //mobjinfo[MT_CHAINGUN].doomednum :
                 i=MT_CHAINGUN;
                 break;
            case 2006 : //mobjinfo[MT_BFG9000].doomednum   : // bfg9000
                 i=MT_BFG9000;
                 break;
            case 2004 : //mobjinfo[MT_PLASMAGUNMISC28].doomednum   : // plasma launcher
                 i=MT_PLASMAGUN;
                 break;
            case 2003 : //mobjinfo[MT_ROCKETLAUNCH].doomednum   : // rocket launcher
                 i=MT_ROCKETLAUNCH;
                 break;
            case 2005 : //mobjinfo[MT_SHAINSAW].doomednum   : // shainsaw
                 i=MT_SHAINSAW;
                 break;
            default:
                 if(freeslot!=j)
                 {
                     itemrespawnque[freeslot]=itemrespawnque[j];
                     itemrespawntime[freeslot]=itemrespawntime[j];
                 }

                 freeslot=(freeslot+1)&(ITEMQUESIZE-1);
                 continue;
        }
        // respwan it
        x = mthing->x << FRACBITS;
        y = mthing->y << FRACBITS;

        // spawn a teleport fog at the new spot
        ss = R_PointInSubsector (x,y);
        mo = P_SpawnMobj (x, y, ss->sector->floorheight , MT_IFOG);
        S_StartSound (mo, sfx_itmbk);

        // spawn it
        if (mobjinfo[i].flags & MF_SPAWNCEILING)
            z = ONCEILINGZ;
        else
            z = ONFLOORZ;

        mo = P_SpawnMobj (x,y,z, i);
        mo->spawnpoint = mthing;
        mo->angle = ANG45 * (mthing->angle/45);
        // here don't increment freeslot
    }
    iquehead=freeslot;
}

extern byte weapontobutton[NUMWEAPONS];

//
// P_SpawnPlayer
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//
// BP: spawn it at a playerspawn mapthing
void P_SpawnPlayer (mapthing_t* mthing)
{
    player_t*           p;
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    mobj_t*             mobj;

    int                 i=mthing->type-1;

    // not playing?
    if (!playeringame[i])
        return;

#ifdef PARANOIA
    if(i<0 && i>=MAXPLAYERS)
        I_Error("P_SpawnPlayer : playernum not in bound (%d)",i);
#endif

    p = &players[i];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn (mthing->type-1);

    x           = mthing->x << FRACBITS;
    y           = mthing->y << FRACBITS;
    z           = ONFLOORZ;
    mobj        = P_SpawnMobj (x,y,z, MT_PLAYER);

#ifdef CLIENTPREDICTION
    //added 1-6-98 : for movement prediction
    p->spirit = P_SpawnMobj (x,y,z, MT_SPIRIT);
#endif

    // set color translations for player sprites
    // added 6-2-98 : change color : now use skincolor (befor is mthing->type-1)
	// Some new stuff here Tails 06-10-2001

		mobj->flags |= (p->skincolor)<<MF_TRANSSHIFT;

    //
    // set 'spritedef' override in mobj for player skins.. (see ProjectSprite)
    // (usefulness : when body mobj is detached from player (who respawns),
    //  the dead body mobj retain the skin through the 'spritedef' override).
    mobj->skin = &skins[p->skin];

    mobj->angle = ANG45 * (mthing->angle/45);
    if (p==&players[consoleplayer])
        localangle = mobj->angle;
    else
    if (p==&players[secondarydisplayplayer])
        localangle2 = mobj->angle;
    mobj->player = p;
    mobj->health = p->health;

    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = cv_viewheight.value<<FRACBITS;
    // added 2-12-98
    p->viewz = p->mo->z + p->viewheight;

    // setup gun psprite
    P_SetupPsprites (p);

    // give all cards in death match mode
    if (cv_deathmatch.value)
        p->cards = it_allkeys;

    if (mthing->type-1 == consoleplayer)
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
    if( p==&players[consoleplayer] )
        mobj->eflags |= MF_INVISIBLE;   // don't show self

    p->spirit->skin    = mobj->skin;
    p->spirit->angle   = mobj->angle;
    p->spirit->player  = mobj->player;
    p->spirit->health  = mobj->health;
    p->spirit->movedir = weapontobutton[p->readyweapon];
#endif
    SV_SpawnPlayer(mthing->type-1,mobj);

    if(camera.chase && displayplayer==mthing->type-1)
       P_ResetCamera(p);
}


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
	int					r; // Vertical Rings Tails 08-05-2001

    if(!mthing->type)
      return; //SoM: 4/7/2000: Ignore type-0 things as NOPs

    // count deathmatch start positions
    if (mthing->type == 11)
    {
        if (deathmatch_p < &deathmatchstarts[MAX_DM_STARTS])
        {
            memcpy (deathmatch_p, mthing, sizeof(*mthing));
            deathmatch_p->type=0; // put it valide
            deathmatch_p++;
        }
        return;
    }

	if (mthing->type == 87) // CTF Startz! Tails 08-04-2001
    {
        if (redctfstarts_p < &redctfstarts[MAXPLAYERS])
        {
            memcpy (redctfstarts_p, mthing, sizeof(*mthing));
            redctfstarts_p->type=0; // put it valide
            redctfstarts_p++;
        }
        return;
    }

	if (mthing->type == 89) // CTF Startz! Tails 08-04-2001
    {
        if (bluectfstarts_p < &bluectfstarts[MAXPLAYERS])
        {
            memcpy (bluectfstarts_p, mthing, sizeof(*mthing));
            bluectfstarts_p->type=0; // put it valide
            bluectfstarts_p++;
        }
        return;
    }

    // check for players specially
    // added 9-2-98 type 5 -> 8 player[x] starts for cooperative
    //              support ctfdoom cooperative playerstart
    //SoM: 4/7/2000: Fix crashing bug.
    if ((mthing->type > 0 && mthing->type <=4) ||
        (mthing->type<=4028 && mthing->type>=4001) )
    {
        if(mthing->type>4000)
             mthing->type=mthing->type-4001+5;

        // save spots for respawning in network games
        playerstarts[mthing->type-1] = *mthing;
        if ((cv_deathmatch.value==0 || cv_gametype.value==0) && demoversion<128) // Tails 03-13-2001
            P_SpawnPlayer (mthing);

        return;
    }

    // check for apropriate skill level
    if (!multiplayer && (mthing->options & 16))
        return;

    //SoM: 4/7/2000: Implement "not deathmatch" thing flag
    if (netgame && cv_deathmatch.value && (mthing->options & 32) )
      return;

    //SoM: 4/7/2000: Implement "not cooperative" thing flag
    if (netgame && !cv_deathmatch.value && (mthing->options & 64) )
      return;

    if (gameskill == sk_baby)
        bit = 1;
    else if (gameskill == sk_nightmare)
        bit = 4;
    else
        bit = 1<<(gameskill-1);

    if (!(mthing->options & bit) )
        return;

    // find which type to spawn
    for (i=0 ; i< NUMMOBJTYPES ; i++)
        if (mthing->type == mobjinfo[i].doomednum)
            break;

    if (i==NUMMOBJTYPES && (!i == 84 || !i == 44)) // Tails 08-05-2001
    {
        CONS_Printf ("\2P_SpawnMapThing: Unknown type %i at (%i, %i)\n",
                      mthing->type,
                      mthing->x, mthing->y);
        return;
    }

    // don't spawn keycards and players in deathmatch
    if (cv_deathmatch.value && mobjinfo[i].flags & MF_NOTDMATCH)
        return;

    // don't spawn any monsters if -nomonsters
    if (nomonsters
        && (i == MT_BLUECRAWLA || i == MT_EGGMOBILE || i == MT_REDCRAWLA || i == MT_GFZFISH)) // Tails 04-01-2001
//             || (mobjinfo[i].flags & MF_COUNTKILL)) )
    {
        return;
    }

	if ((i == 84 || i == 44 || i == MT_MISC2 || i == MT_MISC10 || i == MT_MISC11 || i == MT_MISC50 || i == MT_MISC48 || i == MT_MISC31 || i == MT_BKTV)
		&& gameskill == sk_nightmare
		&& !(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
		|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
		|| gamemap == SSSTAGE7)) // Don't have rings in Very Hard mode Tails 03-26-2001
		return;

	if((i == MT_BLUEFLAG || i == MT_REDFLAG) && !cv_gametype.value == 4)
		return; // Don't spawn flags if you aren't in CTF Mode! Tails 09-03-2001

    // spawn it
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    ss = R_PointInSubsector (x,y);

    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else if (i == MT_MISC2 && mthing->options & MTF_AMBUSH) // Special flag for rings Tails 06-03-2001
		z = ss->sector->floorheight + 32*FRACUNIT;
	else if (i == MT_DETON || i == MT_JETTBOMBER || i == MT_JETTGUNNER || i == MT_EGGMOBILE)
		z = ss->sector->floorheight + 32*FRACUNIT;
	else
        z = ONFLOORZ;

	if(mthing->type == 84) // Vertical Rings - Stack of 5 Tails 08-05-2001
	{
		for(r=1; r<6; r++)
		{
			z = ss->sector->floorheight + 64*FRACUNIT*r;
			mobj = P_SpawnMobj (x,y,z, MT_MISC2);
			mobj->spawnpoint = mthing;

			if (mobj->tics > 0)
		      mobj->tics = 1 + (P_Random () % mobj->tics);

			mobj->angle = ANG45 * (mthing->angle/45);
			if (mthing->options & MTF_AMBUSH)
				mobj->flags |= MF_AMBUSH;
		}
	}
	if(mthing->type == 44) // Vertical Rings - Stack of 5 (suitable for Red Spring) Tails 08-05-2001
	{
		for(r=1; r<6; r++)
		{
			z = ss->sector->floorheight + 128*FRACUNIT*r;
			mobj = P_SpawnMobj (x,y,z, MT_MISC2);
			mobj->spawnpoint = mthing;

			if (mobj->tics > 0)
		      mobj->tics = 1 + (P_Random () % mobj->tics);

			mobj->angle = ANG45 * (mthing->angle/45);
			if (mthing->options & MTF_AMBUSH)
				mobj->flags |= MF_AMBUSH;
		}
	}
	else
	{
    mobj = P_SpawnMobj (x,y,z, i);
    mobj->spawnpoint = mthing;

	if(i == MT_EMMY)
		P_SpawnMobj(x,y,z, MT_TOKEN);
	else if(i == MT_EGGMOBILE && gamemap == 24)
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

    if (mobj->tics > 0)
        mobj->tics = 1 + (P_Random () % mobj->tics);
    if (mobj->flags & MF_COUNTKILL)
        totalkills++;
    if (mobj->flags & MF_COUNTITEM)
        totalitems++;

    mobj->angle = ANG45 * (mthing->angle/45);
    if (mthing->options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;
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
void P_SpawnSplash (mobj_t* mo, boolean flatwater)
                                // flatwater : old water FWATER flat texture
{
    mobj_t*     th;
    fixed_t     z;

    if (demoversion<125)
        return;

    // we are supposed to be in water sector and my current
    // hack uses negative tag as water height
    if (flatwater)
        z = mo->subsector->sector->floorheight + (FRACUNIT/4);
    else
        z = sectors[mo->subsector->sector->heightsec].floorheight; //SoM: 3/17/2000

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


    // get rough idea of speed
    
    thrust = (mo->momx + mo->momy) >> FRACBITS+1;

    if (thrust >= 2 && thrust<=3)
        P_SetMobjState (th,S_SPLASH2);
    else
    if (thrust < 2)
        P_SetMobjState (th,S_SPLASH3);
    
}
*/

// --------------------------------------------------------------------------
// P_SpawnSmoke
// --------------------------------------------------------------------------
// when player gets hurt by lava/slime, spawn at feet
void P_SpawnSmoke ( fixed_t       x,
                    fixed_t       y,
                    fixed_t       z )
{
    mobj_t*     th;

    if (demoversion<125)
        return;

    x = x - ((P_Random()&8) * FRACUNIT) - 4*FRACUNIT;
    y = y - ((P_Random()&8) * FRACUNIT) - 4*FRACUNIT;
    z += (P_Random()&3) * FRACUNIT;


    th = P_SpawnMobj (x,y,z, MT_SMOK);
    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;

    if (th->tics < 1)
        th->tics = 1;
}



// --------------------------------------------------------------------------
// P_SpawnPuff
// --------------------------------------------------------------------------
void P_SpawnPuff ( fixed_t       x,
                   fixed_t       y,
                   fixed_t       z )
{
    mobj_t*     th;

    z += P_Random()<<10;
    z -= P_Random()<<10;

    th = P_SpawnMobj (x,y,z, MT_PUFF);
    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;

    if (th->tics < 1)
        th->tics = 1;

    // don't make punches spark on the wall
    if (attackrange == MELEERANGE)
        P_SetMobjState (th, S_PUFF3);
}



// --------------------------------------------------------------------------
// P_SpawnBlood
// --------------------------------------------------------------------------

static mobj_t*  bloodthing;
// static fixed_t  bloodspawnpointx,bloodspawnpointy; // Tails 11-16-2001

#ifdef WALLSPLATS
boolean PTR_BloodTraverse (intercept_t* in)
{
    line_t*             li;
    divline_t   divl;
    fixed_t     frac;

    fixed_t     z;

    if (in->isaline)
    {
        li = in->d.line;

        z = bloodthing->z + (P_Random()<<(FRACBITS-3));
        z -= P_Random()<<(FRACBITS-3);
        if ( !(li->flags & ML_TWOSIDED) )
            goto hitline;

        P_LineOpening (li);

        // hit lower texture ?
        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            if( openbottom>z )
                goto hitline;
        }

        // hit upper texture ?
        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            if( opentop<z )
                goto hitline;
        }

        // else don't hit
        return true;

hitline:
        P_MakeDivline (li, &divl);
        frac = P_InterceptVector (&divl, &trace);
        R_AddWallSplat (li, P_PointOnLineSide(bloodspawnpointx,bloodspawnpointy,li),"BLUDC0", z, frac, SPLATDRAWMODE_TRANS);
        return false;
    }

    //continue
    return true;
}
#endif

// P_SpawnBloodSplats
// the new SpawnBlood : this one first calls P_SpawnBlood for the usual blood sprites
// then spawns blood splats around on walls
//
void P_SpawnBloodSplats ( fixed_t       x,
                          fixed_t       y,
                          fixed_t       z,
                          int           damage,
                          fixed_t       momx,
                          fixed_t       momy)
{
#ifdef WALLSPLATS
//static int  counter =0;
    fixed_t x2,y2;
    angle_t angle, anglesplat;
    int     distance;
    angle_t anglemul=1;  
    int     numsplats;
    int     i;
#endif
    // spawn the usual falling blood sprites at location
//    P_SpawnBlood (x,y,z,damage);
    //CONS_Printf ("spawned blood counter %d\n", counter++);
    if( demoversion<129)
        return;


#ifdef WALLSPLATS
    // traverse all linedefs and mobjs from the blockmap containing t1,
    // to the blockmap containing the dest. point.
    // Call the function for each mobj/line on the way,
    // starting with the mobj/linedef at the shortest distance...

    if(!momx && !momy)
    {   
        // from inside
        angle=0;
        anglemul=2; 
    }
    else
    {
        // get direction of damage
        x2 = x + momx;
        y2 = y + momy;
        angle = R_PointToAngle2 (x,y,x2,y2);
    }
    distance = damage * 6;
    numsplats = damage / 3+1;
    // BFG is funy without this check
    if( numsplats > 20 )
        numsplats = 20;

    //CONS_Printf ("spawning %d bloodsplats at distance of %d\n", numsplats, distance);
    //CONS_Printf ("damage %d\n", damage);
    bloodspawnpointx = x;
    bloodspawnpointy = y;
    //uses 'bloodthing' set by P_SpawnBlood()
    for (i=0; i<numsplats; i++) {
        // find random angle between 0-180deg centered on damage angle
        anglesplat = angle + (((P_Random() - 128) * FINEANGLES/512*anglemul)<<ANGLETOFINESHIFT);
        x2 = x + distance*finecosine[anglesplat>>ANGLETOFINESHIFT];
        y2 = y + distance*finesine[anglesplat>>ANGLETOFINESHIFT];
        //CONS_Printf ("traversepath cangle %d angle %d fuck %d\n", (angle>>ANGLETOFINESHIFT)*360/FINEANGLES,
        //    (anglesplat>>ANGLETOFINESHIFT)*360/FINEANGLES, (P_Random() - 128));

        P_PathTraverse ( x, y,
                         x2, y2,
                         PT_ADDLINES, 
                         PTR_BloodTraverse );
    }
#endif

#ifdef FLOORSPLATS
    // add a test floor splat
    R_AddFloorSplat (bloodthing->subsector, "STEP2", x, y, bloodthing->floorz, SPLATDRAWMODE_SHADE);
#endif
}


// P_SpawnBlood
// spawn a blood sprite with falling z movement, at location
// the duration and first sprite frame depends on the damage level
// the more damage, the longer is the sprite animation
void P_SpawnBlood ( fixed_t       x,
                    fixed_t       y,
                    fixed_t       z,
                    int           damage )
{
    mobj_t*     th;

    z += P_Random()<<10;
    z -= P_Random()<<10;
    th = P_SpawnMobj (x,y,z, MT_BLOOD);
    if(demoversion>=128)
    {
        th->momx  = P_Random()<<12; //faB:19jan99
        th->momx -= P_Random()<<12; //faB:19jan99
        th->momy  = P_Random()<<12; //faB:19jan99
        th->momy -= P_Random()<<12; //faB:19jan99
    }
    th->momz = FRACUNIT*2;
    th->tics -= P_Random()&3;

    if (th->tics < 1)
        th->tics = 1;

    if (damage <= 12 && damage >= 9)
        P_SetMobjState (th,S_BLOOD2);
    else if (damage < 9)
        P_SetMobjState (th,S_BLOOD3);

    bloodthing = th;
}


//
// P_CheckMissileSpawn
// Moves the missile forward a bit
//  and possibly explodes it right there.
//
void P_CheckMissileSpawn (mobj_t* th)
{
    th->tics -= P_Random()&3;
    if (th->tics < 1)
        th->tics = 1;

    // move a little forward so an angle can
    // be computed if it immediately explodes
    th->x += (th->momx>>1);
    th->y += (th->momy>>1);
    th->z += (th->momz>>1);

    if (!P_TryMove (th, th->x, th->y, false))
        P_ExplodeMissile (th);
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

#ifdef PARANOIA
    if(!source)
        I_Error("P_SpawnMissile : no source");
    if(!dest)
        I_Error("P_SpawnMissile : no dest");
#endif
	if(source->type == MT_JETTGUNNER) // Tails 08-25-2001
    th = P_SpawnMobj (source->x,
                      source->y,
                      source->z - 12*FRACUNIT, type); // Tails 08-25-2001
	else // Tails 08-25-2001
    th = P_SpawnMobj (source->x,
                      source->y,
                      source->z + 4*8*FRACUNIT, type);

    if (th->info->seesound)
        S_StartSound (th, th->info->seesound);

    th->target = source;        // where it came from
    an = R_PointToAngle2 (source->x, source->y, dest->x, dest->y);

// Invis shouldn't matter Tails 01-06-2001
/*
    // fuzzy player
    if (dest->flags & MF_SHADOW)
    {
        an += (P_Random()<<20); // WARNING: don't put this in one line 
        an -= (P_Random()<<20); // else this expretion is ambiguous (evaluation order not diffined)
    }
*/
    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul (th->info->speed, finecosine[an]);
    th->momy = FixedMul (th->info->speed, finesine[an]);

    dist = P_AproxDistance (dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;

    if (dist < 1)
        dist = 1;

    th->momz = (dest->z - source->z) / dist;
    P_CheckMissileSpawn (th);

    return th;
}


//
// P_SpawnPlayerMissile
// Tries to aim at a nearby monster
//
void P_SpawnPlayerMissile ( mobj_t*       source,
                            mobjtype_t    type,
              //added:16-02-98: needed the player here for the aiming
              player_t*     player )
{
    mobj_t*     th;
    angle_t     an;

    fixed_t     x;
    fixed_t     y;
    fixed_t     z;
    fixed_t     slope;

    // angle at which you fire, is player angle
    an = source->angle;

    //added:16-02-98: autoaim is now a toggle
    if (player->autoaim_toggle && cv_allowautoaim.value)
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
                an = source->angle;
                slope = 0;
            }
        }
    }

    //added:18-02-98: if not autoaim, or if the autoaim didnt aim something,
    //                use the mouseaiming
    if (!(player->autoaim_toggle && cv_allowautoaim.value)
                                || (!linetarget && demoversion>111))
    {
        if(demoversion>=128)
            slope = AIMINGTOSLOPE(player->aiming);
        else
            slope = (player->aiming<<FRACBITS)/160;
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
    
    if( demoversion>=128 )
    {   // 1.28 fix, allow full aiming must be much precise
        th->momx = FixedMul(th->momx,finecosine[player->aiming>>ANGLETOFINESHIFT]);
        th->momy = FixedMul(th->momy,finecosine[player->aiming>>ANGLETOFINESHIFT]);
    }
	th->momz = FixedMul( th->info->speed, slope);

    P_CheckMissileSpawn (th);
}
