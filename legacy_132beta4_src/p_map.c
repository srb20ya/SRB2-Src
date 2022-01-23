// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_map.c,v 1.26 2001/12/26 17:24:46 hurdler Exp $
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
// $Log: p_map.c,v $
// Revision 1.26  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.25  2001/08/07 00:53:33  hurdler
// lil' change
//
// Revision 1.24  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.23  2001/07/28 16:18:37  bpereira
// no message
//
// Revision 1.22  2001/06/16 08:07:55  bpereira
// no message
//
// Revision 1.21  2001/05/27 13:42:47  bpereira
// no message
//
// Revision 1.20  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.19  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.18  2001/03/30 17:12:50  bpereira
// no message
//
// Revision 1.17  2001/03/19 18:52:01  hurdler
// lil fix
//
// Revision 1.16  2001/03/13 22:14:19  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.15  2001/03/09 21:53:56  metzgermeister
// *** empty log message ***
//
// Revision 1.14  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.13  2000/11/02 19:49:35  bpereira
// no message
//
// Revision 1.12  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.11  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.10  2000/10/01 10:18:17  bpereira
// no message
//
// Revision 1.9  2000/09/28 20:57:16  bpereira
// no message
//
// Revision 1.8  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.7  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.6  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.5  2000/04/15 22:12:57  stroggonmeth
// Minor bug fixes
//
// Revision 1.4  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.3  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Movement, collision handling.
//      Shooting and aiming.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "m_bbox.h"
#include "m_random.h"
#include "p_local.h"
#include "p_inter.h"
#include "r_state.h"
#include "r_main.h"
#include "r_sky.h"
#include "s_sound.h"

#include "r_splats.h"   //faB: testing

#include "z_zone.h" //SoM: 3/15/2000

void P_ResetScore(player_t* player);

fixed_t         tmbbox[4];
mobj_t*         tmthing;
int             tmflags;
fixed_t         tmx;
fixed_t         tmy;

// Tails 08-25-2002
precipmobj_t*   tmprecipthing;
fixed_t preciptmx;
fixed_t preciptmy;
fixed_t preciptmbbox[4];
int     preciptmflags;

void P_InstaThrust (mobj_t* mo, angle_t angle, fixed_t move); // Proto! Tails 11-01-2000
void P_InstaThrustEvenIn2D(mobj_t* mo, angle_t angle, fixed_t move);
void P_Thrust (mobj_t* mo, angle_t angle, fixed_t move); // Proto! Tails 11-01-2000

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
boolean         floatok;

fixed_t         tmfloorz;
fixed_t         tmceilingz;
fixed_t         tmdropoffz;

mobj_t*         tmfloorthing;   // the thing corresponding to tmfloorz
                                // or NULL if tmfloorz is from a sector

//added:28-02-98: used at P_ThingHeightClip() for moving sectors
fixed_t         tmsectorfloorz;
fixed_t         tmsectorceilingz;

// keep track of the line that lowers the ceiling,
// so missiles don't explode against sky hack walls
line_t*         ceilingline;

// set by PIT_CheckLine() for any line that stopped the PIT_CheckLine()
// that is, for any line which is 'solid'
line_t*         blockingline;

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid
int             *spechit;                //SoM: 3/15/2000: Limit removal
int             numspechit;

//SoM: 3/15/2000
msecnode_t*  sector_list = NULL;

// Tails 08-25-2002
mprecipsecnode_t* precipsector_list = NULL;

//SoM: 3/15/2000
static int pe_x; // Pain Elemental position for Lost Soul checks
static int pe_y; // Pain Elemental position for Lost Soul checks
static int ls_x; // Lost Soul position for Lost Soul checks
static int ls_y; // Lost Soul position for Lost Soul checks

camera_t* mapcampointer; // Tails 12-10-2002


//
// TELEPORT MOVE
//

//
// PIT_StompThing
//
static boolean PIT_StompThing (mobj_t* thing)
{
    fixed_t     blockdist;

    //SoM: 3/15/2000: Move self check to start of routine.

    // don't clip against self

    if (thing == tmthing)
        return true;

    if (!(thing->flags & MF_SHOOTABLE) )
        return true;

    blockdist = thing->radius + tmthing->radius;

    if ( abs(thing->x - tmx) >= blockdist || abs(thing->y - tmy) >= blockdist )
        return true;        // didn't hit it

    // monsters don't stomp things except on boss level
    if (!tmthing->player && gamemap != 30)
        return false;

    P_DamageMobj (thing, tmthing, tmthing, 10000);

    return true;
}

//SoM: Not unused. See p_user.c
//SoM: 3/15/2000
// P_GetMoveFactor() returns the value by which the x,y
// movements are multiplied to add to player movement.

int P_GetMoveFactor(mobj_t* mo)
{
  int movefactor = ORIG_FRICTION_FACTOR;

  // If the floor is icy or muddy, it's harder to get moving. This is where
  // the different friction factors are applied to 'trying to move'. In
  // p_mobj.c, the friction factors are applied as you coast and slow down.

  int momentum,friction;

  if (boomsupport && variable_friction &&
      !(mo->flags & (MF_NOGRAVITY | MF_NOCLIP)))
  {
      friction = mo->friction;
      if (friction == ORIG_FRICTION)            // normal floor
          ;
      else if (friction > ORIG_FRICTION)        // ice
      {
          movefactor = mo->movefactor;
          mo->movefactor = ORIG_FRICTION_FACTOR;  // reset
      }
      else                                      // sludge
      {
          
          // phares 3/11/98: you start off slowly, then increase as
          // you get better footing
          
          momentum = (P_AproxDistance(mo->momx,mo->momy));
          movefactor = mo->movefactor;
          if (momentum > MORE_FRICTION_MOMENTUM<<2)
              movefactor <<= 3;
          
          else if (momentum > MORE_FRICTION_MOMENTUM<<1)
              movefactor <<= 2;
          
          else if (momentum > MORE_FRICTION_MOMENTUM)
              movefactor <<= 1;
          
          mo->movefactor = ORIG_FRICTION_FACTOR;  // reset
      }
  }
  return(movefactor);
}

//
// P_TeleportMove
//
boolean
P_TeleportMove
( mobj_t*       thing,
  fixed_t       x,
  fixed_t       y )
{
/*    int                 xl;
    int                 xh;
    int                 yl;
    int                 yh;
    int                 bx;
    int                 by;

    subsector_t*        newsubsec;

    // kill anything occupying the position
    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector (x,y);
    ceilingline = NULL;

    // The base floor/ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;

    validcount++;
    numspechit = 0;

    // stomp on any things contacted
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if (!P_BlockThingsIterator(bx,by,PIT_StompThing))
                return false;
*/
    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition (thing);

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;

    P_SetThingPosition (thing);

	P_CheckPosition(thing, thing->x, thing->y);

    return true;
}


// =========================================================================
//                       MOVEMENT ITERATOR FUNCTIONS
// =========================================================================



static void add_spechit( line_t* ld )
{
    static int spechit_max = 0;

    //SoM: 3/15/2000: Boom limit removal.
    if (numspechit >= spechit_max)
    {
        spechit_max = spechit_max ? spechit_max*2 : 16;
        spechit = (int *)realloc(spechit,sizeof(int)*spechit_max);
    }
    
    spechit[numspechit] = ld - lines;
    numspechit++;
}

void P_ResetPlayer(player_t* player);

void P_DoSpring(mobj_t* spring, mobj_t* object)
{
	switch(spring->type)
	{
		case MT_YELLOWDIAG: // Yellow diagonal spring (pointing up from ground)
		case MT_REDDIAG: // Red diagonal spring (pointing up from ground)
		    spring->flags &= ~MF_SOLID; // De-solidify
		    object->momx = object->momy = object->momz = 0;
		    P_UnsetThingPosition (object);
		    object->x = spring->x;
		    object->y = spring->y;
		    object->z = spring->z + spring->height + 1;
		    P_SetThingPosition (object);
		    object->momz = spring->info->speed;
		    P_InstaThrustEvenIn2D(object, spring->angle, spring->info->damage);
		    P_SetMobjState (spring, spring->info->seestate);
		    spring->flags |= MF_SOLID; // Re-solidify
			if(object->player)
			{
				if(!(object->player->cmd.forwardmove || object->player->cmd.sidemove))
				{
					object->player->mo->angle = spring->angle;

					if (object->player==&players[consoleplayer])
						localangle = spring->angle;
					else if(cv_splitscreen.value && object->player==&players[secondarydisplayplayer])
					    localangle2 = spring->angle;
				}
					
				P_ResetPlayer(object->player);
			    P_SetMobjState (object, S_PLAY_PLG1);
			}
			break;
		case MT_YELLOWDIAGDOWN: // Yellow diagonal spring (upside-down)
		case MT_REDDIAGDOWN: // Yellow diagonal spring (upside-down)
		    spring->flags &= ~MF_SOLID; // De-solidify
		    object->momx = object->momy = object->momz = 0;
		    P_UnsetThingPosition (object);
		    object->x = spring->x;
		    object->y = spring->y;
		    object->z = spring->z - object->height - 1;
		    P_SetThingPosition (object);
		    object->momz = spring->info->speed;
		    P_InstaThrustEvenIn2D(object, spring->angle, spring->info->damage);
		    P_SetMobjState (spring, spring->info->seestate);
		    spring->flags |= MF_SOLID; // Re-solidify
		    if(object->player)
			{
				if(!(object->player->cmd.forwardmove || object->player->cmd.sidemove))
				{
					object->player->mo->angle = spring->angle;
					if (object->player==&players[consoleplayer])
						localangle = spring->angle;
					else if(cv_splitscreen.value && object->player==&players[secondarydisplayplayer])
					   localangle2 = spring->angle;
				}
				P_ResetPlayer(object->player);
			}
		    break;
		 case MT_YELLOWSPRING: // Yellow vertical spring (pointing up)
		 case MT_REDSPRING: // Red vertical spring (pointing up)
		   object->z++;
		   object->momz = spring->info->speed;
		   P_SetMobjState (spring, spring->info->seestate);
		   if(object->player)
		   {
				P_ResetPlayer(object->player);
				P_SetMobjState (object, S_PLAY_PLG1);

				if(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE) // Mimmick SA
				{
					object->momx = object->momy = 0;
					P_UnsetThingPosition(object);
					object->x = spring->x;
					object->y = spring->y;
					P_SetThingPosition(object);
				}
				break;
		   }
		   break;
		 case MT_YELLOWSPRINGDOWN: // Yellow vertical spring (pointing down)
		 case MT_REDSPRINGDOWN: // Red vertical spring (pointing down)
			object->momz = spring->info->speed;
		    P_SetMobjState (spring, spring->info->seestate);
			if(object->player)
			{
				P_ResetPlayer(object->player);
				P_SetMobjState (object, S_PLAY_FALL1);

				if(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE) // Mimmick SA
				{
					object->momx = object->momy = 0;
					P_UnsetThingPosition(object);
					object->x = spring->x;
					object->y = spring->y;
					P_SetThingPosition(object);
				}
			}
		   break;
		 default: // Must be a user-defined spring Tails 12-14-2003
		    spring->flags &= ~MF_SOLID; // De-solidify
		    object->momx = object->momy = object->momz = 0;
		    P_UnsetThingPosition (object);
		    object->x = spring->x;
		    object->y = spring->y;
		    object->z = spring->z;
		    P_SetThingPosition (object);
		    object->momz = spring->info->speed;
		    P_InstaThrustEvenIn2D(object, spring->angle, spring->info->damage);
		    P_SetMobjState (spring, spring->info->seestate);
		    spring->flags |= MF_SOLID; // Re-solidify
			if(object->player)
			{
				if(!(object->player->cmd.forwardmove || object->player->cmd.sidemove))
				{
					object->player->mo->angle = spring->angle;

					if (object->player==&players[consoleplayer])
						localangle = spring->angle;
					else if(cv_splitscreen.value && object->player==&players[secondarydisplayplayer])
					    localangle2 = spring->angle;
				}
					
				P_ResetPlayer(object->player);
			    P_SetMobjState (object, S_PLAY_PLG1);
			}
			break;
	}
}

void P_TransferToAxis(player_t* player, int axisnum);
extern consvar_t cv_tailspickup;

//
// PIT_CheckThing
//
static boolean PIT_CheckThing (mobj_t* thing)
{
    fixed_t             blockdist;
    boolean             solid;
    int                 damage;

    //added:22-02-98:
    fixed_t             topz;
    fixed_t             tmtopz;

    //SoM: 3/15/2000: Moved to front.

    // don't clip against self

	damage = 0;

	if(!tmthing)
		return true;

	if(!thing)
		return true;

    if (thing == tmthing)
        return true;

	if(thing->state == &states[S_DISS])
		return true;

	// Don't collide with your buddies while NiGHTS-flying.
	if(tmthing->player && thing->player
		&& mapheaderinfo[gamemap-1].typeoflevel & TOL_NIGHTS)
	{
		if (tmthing->player->nightsmode)
			return true;

		if (thing->player->nightsmode)
			return true;
	}

	if(tmthing->player && tmthing->player->nightsmode)
	{
		if(thing->type == MT_AXISTRANSFER)
		{
			if(!tmthing->target)
				return true;

			if(tmthing->target->threshold != thing->threshold)
				return true;

			blockdist = thing->radius + tmthing->radius;

			if ( abs(thing->x - tmx) >= blockdist ||
				 abs(thing->y - tmy) >= blockdist )
			{
				// didn't hit it
				return true;
			}

			if (tmthing->player->axishit && tmthing->player->lastaxis == thing)
			{
				tmthing->player->axishit = true;
				return true;
			}

			if(tmthing->target->health <= thing->health)
			{
				// Find the next axis with a ->health
				// +1 from the current axis.
				tmthing->player->lastaxis = thing;
				P_TransferToAxis(tmthing->player, tmthing->target->health == 0 ? thing->health + 1 : tmthing->target->health + 1);
			}
			else if(tmthing->target->health > thing->health)
			{
				// Find the next axis with a ->health
				// -1 from the current axis.
				tmthing->player->lastaxis = thing;
				P_TransferToAxis(tmthing->player, tmthing->target->health - 1);
			}
		}
		else if(thing->type == MT_AXISTRANSFERCONDITION)
		{
			if(!tmthing->target)
				return true;

			if(tmthing->target->threshold != thing->threshold)
				return true;

			blockdist = thing->radius + tmthing->radius;

			if ( abs(thing->x - tmx) >= blockdist ||
				 abs(thing->y - tmy) >= blockdist )
			{
				// didn't hit it
				return true;
			}

			if (tmthing->player->axishit && tmthing->player->lastaxis == thing)
			{
				tmthing->player->axishit = true;
				return true;
			}

			if(tmthing->target->health == thing->health)
			{
				// Find the next axis with a ->health
				// +1 from the current axis.
				tmthing->player->lastaxis = thing;
				P_TransferToAxis(tmthing->player, tmthing->target->health + 1);
			}
			else if(tmthing->target->health == thing->health+1
				&& tmthing->player->flyangle > 90
				&& tmthing->player->flyangle < 270)
			{
				// Find the next axis with a ->health
				// -1 from the current axis.
				tmthing->player->lastaxis = thing;
				P_TransferToAxis(tmthing->player, tmthing->target->health - 1);
			}
		}
		else if(thing->type == MT_AXISTRANSFERCONDITION2)
		{
			if(!tmthing->target)
				return true;

			if(tmthing->target->threshold != thing->threshold)
				return true;

			blockdist = thing->radius + tmthing->radius;

			if ( abs(thing->x - tmx) >= blockdist ||
				 abs(thing->y - tmy) >= blockdist )
			{
				// didn't hit it
				return true;
			}

			if (tmthing->player->axishit && tmthing->player->lastaxis == thing)
			{
				tmthing->player->axishit = true;
				return true;
			}

			if(tmthing->target->health == thing->health)
			{
				// Find the next axis with a ->health
				// +1 from the current axis.
				tmthing->player->lastaxis = thing;
				P_TransferToAxis(tmthing->player, tmthing->target->health + 1);
			}
			else if(tmthing->target->health == thing->health+1
				&& (tmthing->player->flyangle < 90
				|| tmthing->player->flyangle > 270))
			{
				// Find the next axis with a ->health
				// -1 from the current axis.
				tmthing->player->lastaxis = thing;
				P_TransferToAxis(tmthing->player, tmthing->target->health - 1);
			}
		}
		// Transfers you from the last axis to the first one and back.
		else if(thing->type == MT_AXISTRANSFERTOLAST)
		{
			if(!tmthing->target)
				return true;

			if(tmthing->target->threshold != thing->threshold)
				return true;

			blockdist = thing->radius + tmthing->radius;

			if ( abs(thing->x - tmx) >= blockdist ||
				 abs(thing->y - tmy) >= blockdist )
			{
				// didn't hit it
				return true;
			}

			if (tmthing->player->axishit && tmthing->player->lastaxis == thing)
			{
				tmthing->player->axishit = true;
				return true;
			}

			if(tmthing->target->health == thing->health)
			{
				// Find the next axis with a ->health
				// +1 from the current axis.
				tmthing->player->lastaxis = thing;
				P_TransferToAxis(tmthing->player, 1);
			}
			else if(tmthing->target->health == 1)
			{
				// Find the next axis with a ->health
				// -1 from the current axis.
				tmthing->player->lastaxis = thing;
				P_TransferToAxis(tmthing->player, thing->health);
			}
		}
		else if(thing->type == MT_AXISTRANSFERCLOSEST)
		{
			if(!tmthing->target)
				return true;

			if(tmthing->target->threshold != thing->threshold)
				return true;

			blockdist = thing->radius + tmthing->radius;

			if ( abs(thing->x - tmx) >= blockdist ||
				 abs(thing->y - tmy) >= blockdist )
			{
				// didn't hit it
				return true;
			}
			tmthing->player->transfertoclosest = true;
		}
	}
/*
	CONS_Printf("thing is %d\n", thing->type);
	CONS_Printf("tmthing is %d\n", tmthing->type);

	if(!thing || !thing->flags)
		I_Error("This is what I'm looking for!\n%d\n", thing->type);
*/

    if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE) ))
        return true;

#ifdef CLIENTPREDICTION2
    // mobj and spirit of a same player cannot colide
    if( thing->player && (thing->player->spirit == tmthing || thing->player->mo == tmthing) )
        return true;
#endif

	if(thing->type == MT_SPARK || tmthing->type == MT_SPARK) // Don't collide with sparks, hehe! Tails 02-02-2002
		return true;

    blockdist = thing->radius + tmthing->radius;

    if ( abs(thing->x - tmx) >= blockdist ||
         abs(thing->y - tmy) >= blockdist )
    {
        // didn't hit it
        return true;
    }

    // check for skulls slamming into things
    if (tmthing->flags2 & MF2_SKULLFLY)
    {
		if(tmthing->type == MT_EGGMOBILE) // Don't make Eggman stop!
		{
			tmthing->momx = -tmthing->momx;
			tmthing->momy = -tmthing->momy;
			tmthing->momz = -tmthing->momz;
		}
		else
		{
			tmthing->flags2 &= ~MF2_SKULLFLY;
			tmthing->momx = tmthing->momy = tmthing->momz = 0;
			return false;           // stop moving
		}
    }

// Snowballs can hit other things Tails 12-12-2001
    if (tmthing->type == MT_SNOWBALL) // Tails 12-12-2001
    {
        // see if it went over / under
        if (tmthing->z > thing->z + thing->height)
            return true;                // overhead
        if (tmthing->z+tmthing->height < thing->z)
            return true;                // underneath

        if (( tmthing->target && (
            tmthing->target->type == thing->type)) )
        {
            // Don't hit same species as originator.
            if (thing == tmthing->target)
                return true;

            if (thing->type != MT_PLAYER)
            {
                // Explode, but do no damage.
                // Let players missile other players.
                return false;
            }
        }

        if (! (thing->flags & MF_SHOOTABLE) )
        {
            // didn't do any damage
            return !(thing->flags & MF_SOLID);
        }

        // damage / explode
        damage = 1;
		P_DamageMobj (thing, tmthing, tmthing->target, damage); // New way Tails 12-10-2000

        // don't traverse any more
			return true;
    }

    // missiles can hit other things
    if (tmthing->flags & MF_MISSILE || tmthing->type == MT_SHELL || tmthing->type == MT_FIREBALL) // Tails 12-12-2001
    {
        // see if it went over / under
        if (tmthing->z > thing->z + thing->height)
            return true;                // overhead
        if (tmthing->z+tmthing->height < thing->z)
            return true;                // underneath

        if (tmthing->type != MT_SHELL && tmthing->target && (
            tmthing->target->type == thing->type) )
        {
            // Don't hit same species as originator.
            if (thing == tmthing->target)
                return true;

            if (thing->type != MT_PLAYER)
            {
                // Explode, but do no damage.
                // Let players missile other players.
                return false;
            }
        }

        if (! (thing->flags & MF_SHOOTABLE) )
        {
            // didn't do any damage
            return !(thing->flags & MF_SOLID);
        }

		if(tmthing->type == MT_SHELL && tmthing->threshold > TICRATE)
			return true;

        // damage / explode
        damage = 1;
		if(tmthing->flags & MF_ENEMY) // An actual ENEMY! (Like the deton, for example)
			P_DamageMobj (thing, tmthing, tmthing, damage); // New way Tails 12-10-2000
		else
			P_DamageMobj (thing, tmthing, tmthing->target, damage); // New way Tails 12-10-2000

        // don't traverse any more
		return false;
    }

    if (tmthing->z + tmthing->height > thing->z
		&& tmthing->z < thing->z + thing->height
		&& thing->flags & MF_PUSHABLE)
	{ // Push thing! Tails 02-02-2002
		if(thing->flags2 & MF2_SLIDEPUSH) // Make it slide Graue 12-31-2003
		{
			if(tmthing->momy > 0 && tmthing->momy > 4*FRACUNIT
				&& tmthing->momy > thing->momy)
				//&& thing->momy + PUSHACCEL < thing->info->speed)
			{
				thing->momy += PUSHACCEL;
				tmthing->momy -= PUSHACCEL;
			}
			else if(tmthing->momy < 0 && tmthing->momy < -4*FRACUNIT
				&& tmthing->momy < thing->momy)
				//&& thing->momy - PUSHACCEL > -(thing->info->speed))
			{
				thing->momy -= PUSHACCEL;
				tmthing->momy += PUSHACCEL;
			}
			if(tmthing->momx > 0 && tmthing->momx > 4*FRACUNIT
				&& tmthing->momx > thing->momx)
				//&& thing->momx + PUSHACCEL < thing->info->speed)
			{
				thing->momx += PUSHACCEL;
				tmthing->momx -= PUSHACCEL;
			}
			else if(tmthing->momx < 0 && tmthing->momx < -4*FRACUNIT
				&& tmthing->momx < thing->momx)
				//&& thing->momx - PUSHACCEL > -(thing->info->speed))
			{
				thing->momx -= PUSHACCEL;
				tmthing->momx += PUSHACCEL;
			}

			//tmthing->momx = thing->momx;
			//tmthing->momy = thing->momy;

			if(thing->momx > thing->info->speed)
				thing->momx = thing->info->speed;
			else if(thing->momx < -(thing->info->speed))
				thing->momx = -(thing->info->speed);
			if(thing->momy > thing->info->speed)
				thing->momy = thing->info->speed;
			else if(thing->momy < -(thing->info->speed))
				thing->momy = -(thing->info->speed);
			//CONS_Printf("THE SPEED IS %d (heh: %d)\n", thing->info->speed, rand());
			//CONS_Printf("Thing: (%d,%d)\n", thing->momx, thing->momy);
			//CONS_Printf("Tmthing: (%d,%d)\n", tmthing->momx, tmthing->momy);
		}
		else
		{
			if(tmthing->momy > 0 && tmthing->momy > 4*FRACUNIT)
				tmthing->momy = 4*FRACUNIT;
			else if(tmthing->momy < 0 && tmthing->momy < -4*FRACUNIT)
				tmthing->momy = -4*FRACUNIT;
			if(tmthing->momx > 0 && tmthing->momx > 4*FRACUNIT)
				tmthing->momx = 4*FRACUNIT;
			else if(tmthing->momx < 0 && tmthing->momx < -4*FRACUNIT)
				tmthing->momx = -4*FRACUNIT;

			thing->momx = tmthing->momx;
			thing->momy = tmthing->momy;
		}

		thing->target = tmthing;
	}

	if(tmthing->type == MT_SPIKEBALL && thing->player)
		P_TouchSpecialThing(tmthing, thing, true);
	else if(thing->type == MT_SPIKEBALL && tmthing->player)
		P_TouchSpecialThing(thing, tmthing, true);

    // check for special pickup
    if (thing->flags & MF_SPECIAL)
    {
        solid = thing->flags&MF_SOLID;
        if (tmthing->player)
        {
            // can remove thing
            P_TouchSpecialThing (thing, tmthing, true);
        }
        return !solid;
    }
    // check again for special pickup
    if(tmthing->flags & MF_SPECIAL)
    {
        solid = tmthing->flags&MF_SOLID;
        if (thing->player)
        {
            // can remove thing
            P_TouchSpecialThing (tmthing, thing, true);
        }
        return !solid;
    }

	// Sprite Spikes! Tails 06-16-2002
	if(tmthing->type == MT_CEILINGSPIKE)
	{
		if(thing->z + thing->height == tmthing->z && thing->momz >= 0)
			P_DamageMobj(thing, tmthing, tmthing, 1); // Ouch!
	}
	else if(thing->type == MT_CEILINGSPIKE)
	{
		if(tmthing->z + tmthing->height == thing->z && tmthing->momz >= 0)
			P_DamageMobj(tmthing, thing, thing, 1);
	}
	else if(tmthing->type == MT_FLOORSPIKE)
	{
		if(thing->z == tmthing->z + tmthing->height + FRACUNIT
			&& thing->momz <= 0)
		{
			tmthing->threshold = 43;
			P_DamageMobj(thing, tmthing, tmthing, 1);
		}
	}
	else if(thing->type == MT_FLOORSPIKE)
	{
		if(tmthing->z == thing->z + thing->height + FRACUNIT
			&& tmthing->momz <= 0)
		{
			thing->threshold = 43;
			P_DamageMobj(tmthing, thing, thing, 1);
		}
	}

	if((tmthing->flags & MF_PUSHABLE) && tmthing->z <= (thing->z + thing->height + FRACUNIT)
		&& (tmthing->z + tmthing->height) >= thing->z)
	{
		if(thing->flags & MF_SPRING)
			P_DoSpring(thing, tmthing);
	}

	if(cv_tailspickup.value && tmthing->player && thing->player)
	{
		if(tmthing->player->carried && tmthing->tracer == thing)
			return true;
		else if(thing->player->carried && thing->tracer == tmthing)
			return true;
		else if(tmthing->player->powers[pw_tailsfly]
				|| (tmthing->player->charability == 1 && (tmthing->state == &states[S_PLAY_SPC1] || tmthing->state == &states[S_PLAY_SPC2] || tmthing->state == &states[S_PLAY_SPC3] || tmthing->state == &states[S_PLAY_SPC4])))
		{
			if(thing->player->nightsmode)
				return true;

			if((tmthing->z <= thing->z + thing->height + FRACUNIT)
				&& tmthing->z > thing->z + thing->height*2/3
				&& thing->momz <= 0)
			{
				thing->player->carried = true;
				P_ResetPlayer(thing->player);
				P_ResetScore(thing->player);
				thing->tracer = tmthing;
				P_UnsetThingPosition(thing);
				thing->x = tmthing->x;
				thing->y = tmthing->y;
				P_SetThingPosition(thing);
			}
			else
				thing->player->carried = false;
		}
		else
			thing->player->carried = false;

		return true;
	}

	if(thing->player)
	{
		// Objects kill you if it falls from above.
		if(tmthing->z + tmthing->momz <= thing->z + thing->height
			&& tmthing->z + tmthing->momz > thing->z
			&& thing->z == thing->floorz)
		{
			if(tmthing->flags & MF_MONITOR && mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE);
			else if((tmthing->flags & MF_MONITOR) || (tmthing->flags & MF_PUSHABLE))
			{
				if(thing != tmthing->target)
					P_DamageMobj(thing, tmthing, tmthing->target, 10000);

				tmthing->momz = -tmthing->momz/2; // Bounce, just for fun!
				// The tmthing->target allows the pusher of the object
				// to get the frag if he topples it on an opponent.
			}
		}

		// Start some Tag Mode stuff Tails 05-08-2001
		if(cv_gametype.value == GT_TAG && tmthing->player && (((thing->z <= tmthing->z + tmthing->height) && (thing->z + thing->height >= tmthing->z)) || (tmthing->z == thing->z + thing->height + FRACUNIT)))
		{
			if(thing->player->tagit < 298*TICRATE && thing->player->tagit > 0 && !(tmthing->player->powers[pw_flashing] || tmthing->player->tagzone || tmthing->player->powers[pw_invulnerability]))
			{
				P_DamageMobj(tmthing, thing, thing, 1); // Don't allow tag-backs
			}
			else if (tmthing->player->tagit < 298*TICRATE && tmthing->player->tagit > 0 && !(thing->player->powers[pw_flashing] || thing->player->tagzone || thing->player->powers[pw_invulnerability]))
			{
				P_DamageMobj(thing, tmthing, tmthing, 1); // Don't allow tag-backs
			}
		}
		// End some Tag Mode stuff Tails 05-08-2001

		if(thing->z >= tmthing->z) // Stuff where da player don't gotta move Tails 05-29-2001
		{
			switch(tmthing->type)
			{
				case MT_FAN: // fan
					if(thing->z <= tmthing->z + (tmthing->health << FRACBITS))
					{
						thing->momz = tmthing->info->speed;
						P_ResetPlayer(thing->player);
						if(!(thing->state == &states[S_PLAY_FALL1] || thing->state == &states[S_PLAY_FALL2]))
							P_SetMobjState (thing, S_PLAY_FALL1);
					}
					break;
				case MT_STEAM: // Steam, duh! Can't you read? Tails 05-28-2001
					if(tmthing->state == &states[S_STEAM1] && thing->z <= tmthing->z + 16*FRACUNIT) // Only when it bursts
					{
						thing->momz = tmthing->info->speed;
						P_ResetPlayer(thing->player);
						if(!(thing->state == &states[S_PLAY_FALL1] || thing->state == &states[S_PLAY_FALL2]))
							P_SetMobjState (thing, S_PLAY_FALL1);
					}
					break;
			default:
				break;
			}
		}
	}

	if(tmthing->player) // Is the moving/interacting object the player?
	{
		if(tmthing->z >= thing->z)
		{
			switch(thing->type)
			{
				case MT_FAN: // fan
					if(tmthing->z <= thing->z + (thing->health << FRACBITS))
					{
					tmthing->momz = thing->info->speed;
					P_ResetPlayer(tmthing->player);
					if(!(tmthing->state == &states[S_PLAY_FALL1] || tmthing->state == &states[S_PLAY_FALL2]))
						P_SetMobjState (tmthing, S_PLAY_FALL1);
					}
					break;
				case MT_STEAM: // Steam, duh! Can't you read? Tails 05-28-2001
					if(thing->state == &states[S_STEAM1] && tmthing->z <= thing->z + 16*FRACUNIT) // Only when it bursts
					{
					tmthing->momz = thing->info->speed;
					P_ResetPlayer(tmthing->player);
					if(!(tmthing->state == &states[S_PLAY_FALL1] || tmthing->state == &states[S_PLAY_FALL2]))
						P_SetMobjState (tmthing, S_PLAY_FALL1);
					}
					break;
				default:
					break;
			}
		}
		if(((thing->z <= tmthing->z + tmthing->height) && (thing->z + thing->height >= tmthing->z)) || (tmthing->z == thing->z + thing->height + FRACUNIT)) // Are you touching the side of it?
		{
			if((tmthing->player->mfjumped == 1) || (tmthing->player->mfspinning == 1) || (tmthing->player->powers[pw_invulnerability]) || (tmthing->player->powers[pw_super])) // Do you possess the ability to subdue the object?
			{
				if(thing->flags & MF_MONITOR)
				{
					if(((tmthing->player->mfjumped == 0) && (tmthing->player->mfspinning == 0)) && ((tmthing->player->powers[pw_super]) || (tmthing->player->powers[pw_invulnerability]))); // Don't bust boxes like Sonic Adventure Tails 11-02-2000
					else
					{
						if(tmthing->momz < 0)
							tmthing->momz = -tmthing->momz;
						P_DamageMobj(thing, tmthing, tmthing, 1);
					}
				}
				if(thing->flags & MF_SPRING)
				{
					P_DoSpring(thing, tmthing);
				}
				if(thing->flags & MF_BOSS)
				{
					if(tmthing->momz < 0)
						tmthing->momz = -tmthing->momz;
					tmthing->momx = -tmthing->momx;
					tmthing->momy = -tmthing->momy;
					P_DamageMobj(thing, tmthing, tmthing, 1);
				}
			}
			else
			{
				if(thing->flags & MF_SPRING)
					P_DoSpring(thing, tmthing);
			}
		}
	}

    //added:24-02-98:compatibility with old demos, it used to return with...
    //added:27-02-98:for version 112+, nonsolid things pass through other things
    if (!(tmthing->flags & MF_SOLID))
        return !(thing->flags & MF_SOLID);

    //added:22-02-98: added z checking at last
    //SoM: 3/10/2000: Treat noclip things as non-solid!
    if ((thing->flags & MF_SOLID) && (tmthing->flags & MF_SOLID) &&
        !(thing->flags & MF_NOCLIP) && !(tmthing->flags & MF_NOCLIP))
    {
        // pass under
        tmtopz = tmthing->z + tmthing->height;

        if ( tmtopz < thing->z)
        {
            if (thing->z < tmceilingz)
                tmceilingz = thing->z;
            return true;
        }

        topz = thing->z + thing->height + FRACUNIT;

        // block only when jumping not high enough,
        // (dont climb max. 24units while already in air)
        // if not in air, let P_TryMove() decide if its not too high
        if (tmthing->player &&
            tmthing->z < topz &&
            tmthing->z > tmthing->floorz)  // block while in air
            return false;


        if (topz > tmfloorz)
        {
            tmfloorz = topz;
            tmfloorthing = thing;       //thing we may stand on
        }

    }

    // not solid not blocked
    return true;
}

// SoM: 3/15/2000
// PIT_CrossLine
// Checks to see if a PE->LS trajectory line crosses a blocking
// line. Returns false if it does.
//
// tmbbox holds the bounding box of the trajectory. If that box
// does not touch the bounding box of the line in question,
// then the trajectory is not blocked. If the PE is on one side
// of the line and the LS is on the other side, then the
// trajectory is blocked.
//
// Currently this assumes an infinite line, which is not quite
// correct. A more correct solution would be to check for an
// intersection of the trajectory and the line, but that takes
// longer and probably really isn't worth the effort.
//
static boolean PIT_CrossLine (line_t* ld)
  {
  if (!(ld->flags & ML_TWOSIDED) ||
      (ld->flags & (ML_BLOCKING|ML_BLOCKMONSTERS)))
    if (!(tmbbox[BOXLEFT]   > ld->bbox[BOXRIGHT]  ||
          tmbbox[BOXRIGHT]  < ld->bbox[BOXLEFT]   ||
          tmbbox[BOXTOP]    < ld->bbox[BOXBOTTOM] ||
          tmbbox[BOXBOTTOM] > ld->bbox[BOXTOP]))
      if (P_PointOnLineSide(pe_x,pe_y,ld) != P_PointOnLineSide(ls_x,ls_y,ld))
        return(false);  // line blocks trajectory
  return(true); // line doesn't block trajectory
  }

extern void P_CameraLineOpening(line_t* linedef);

// PIT_CheckCameraLine
// Adjusts tmfloorz and tmceilingz as lines are contacted - FOR CAMERA ONLY
// Tails 09-29-2002
boolean PIT_CheckCameraLine (line_t* ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
        || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
        return true;

    if (P_BoxOnLineSide (tmbbox, ld) != -1)
        return true;

    // A line has been hit

    // The moving thing's destination position will cross
    // the given line.
    // If this should not be allowed, return false.
    // If the line is special, keep track of it
    // to process later if the move is proven ok.
    // NOTE: specials are NOT sorted by order,
    // so two special lines that are only 8 pixels apart
    // could be crossed in either order.

    // 10-12-99 BP: moved this line to out of the if so upper and 
    //              lower texture can be hit by a splat
    blockingline = ld;
    if (!ld->backsector)
    {
      return false;           // one sided line
    }

    // set openrange, opentop, openbottom
    P_CameraLineOpening (ld);

    // adjust floor / ceiling heights
    if (opentop < tmceilingz)
    {
        tmsectorceilingz = tmceilingz = opentop;
        ceilingline = ld;
    }

    if (openbottom > tmfloorz)
        tmsectorfloorz = tmfloorz = openbottom;

    if (lowfloor < tmdropoffz)
        tmdropoffz = lowfloor;

    return true;
}

//
// PIT_CheckLine
// Adjusts tmfloorz and tmceilingz as lines are contacted
//
boolean PIT_CheckLine (line_t* ld)
{
    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT]
        || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT]
        || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM]
        || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP] )
        return true;

    if (P_BoxOnLineSide (tmbbox, ld) != -1)
        return true;

    // A line has been hit

    // The moving thing's destination position will cross
    // the given line.
    // If this should not be allowed, return false.
    // If the line is special, keep track of it
    // to process later if the move is proven ok.
    // NOTE: specials are NOT sorted by order,
    // so two special lines that are only 8 pixels apart
    // could be crossed in either order.

    // 10-12-99 BP: moved this line to out of the if so upper and 
    //              lower texture can be hit by a splat
    blockingline = ld;
    if (!ld->backsector)
    {
      if(tmthing->flags & MF_MISSILE && ld->special)
        add_spechit(ld);

      return false;           // one sided line
    }

    // missil and Camera can cross uncrossable line
    if (!(tmthing->flags & MF_MISSILE) &&
        !(tmthing->type == MT_CHASECAM) )
    {
        if (ld->flags & ML_BLOCKING)
            return false;       // explicitly blocking everything

        if ( !(tmthing->player) &&
             ld->flags & ML_BLOCKMONSTERS )
            return false;       // block monsters only
    }

    // set openrange, opentop, openbottom
    P_LineOpening (ld);

    // adjust floor / ceiling heights
    if (opentop < tmceilingz)
    {
        tmsectorceilingz = tmceilingz = opentop;
        ceilingline = ld;
    }

    if (openbottom > tmfloorz)
        tmsectorfloorz = tmfloorz = openbottom;

    if (lowfloor < tmdropoffz)
        tmdropoffz = lowfloor;

    // if contacted a special line, add it to the list
    if (ld->special)
      add_spechit(ld);

    return true;
}


// =========================================================================
//                         MOVEMENT CLIPPING
// =========================================================================

//
// P_CheckPosition
// This is purely informative, nothing is modified
// (except things picked up).
//
// in:
//  a mobj_t (can be valid or invalid)
//  a position to be checked
//   (doesn't need to be related to the mobj_t->x,y)
//
// during:
//  special things are touched if MF_PICKUP
//  early out on solid lines?
//
// out:
//  newsubsec
//  tmfloorz
//  tmceilingz
//  tmdropoffz
//   the lowest point contacted
//   (monsters won't move to a dropoff)
//  speciallines[]
//  numspeciallines
//

//added:27-02-98:
//
// tmfloorz
//     the nearest floor or thing's top under tmthing
// tmceilingz
//     the nearest ceiling or thing's bottom over tmthing
//
boolean P_CheckPosition ( mobj_t*       thing,
                          fixed_t       x,
                          fixed_t       y )
{
    int                 xl;
    int                 xh;
    int                 yl;
    int                 yh;
    int                 bx;
    int                 by;
    subsector_t*        newsubsec;
/*
	if(true)
	{
	    thinker_t*  th;
		mobj_t*     mo2;
		boolean foundit = false;

		// scan the remaining thinkers
		// to find all emeralds
		for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo2 = (mobj_t *)th;
			if(mo2 == thing)
			{
				foundit = true;
				break;
			}
		}

		if(foundit == false)
		{
			CONS_Printf("Ack! The mobj number is %d!\n", thing->type);
			return true;
		}
	}
*/
    tmthing = thing;
    tmflags = thing->flags;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;

    newsubsec = R_PointInSubsector (x,y);
    ceilingline = blockingline = NULL;

    // The base floor / ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmsectorfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = tmsectorceilingz = newsubsec->sector->ceilingheight;

    //SoM: 3/23/2000: Check list of fake floors and see if
    //tmfloorz/tmceilingz need to be altered.
    if(newsubsec->sector->ffloors)
    {
      ffloor_t*  rover;
      fixed_t    delta1;
      fixed_t    delta2;
      int        thingtop = thing->z + thing->height;

      for(rover = newsubsec->sector->ffloors; rover; rover = rover->next)
      {
        if((!(rover->flags & FF_SOLID) && !(thing->player && !thing->player->nightsmode && thing->player->skin == 1 && !thing->player->mfspinning && thing->player->speed > 28 && thing->ceilingz - *rover->topheight >= thing->info->height && thing->z < *rover->topheight + 30*FRACUNIT && thing->z > *rover->topheight - 30*FRACUNIT)) || !(rover->flags & FF_EXISTS)) continue;

        delta1 = thing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > tmfloorz && abs(delta1) < abs(delta2))
          tmfloorz = tmdropoffz = *rover->topheight;
        if(*rover->bottomheight < tmceilingz && abs(delta1) >= abs(delta2))
		{
			if(!(rover->flags & FF_PLATFORM))
				tmceilingz = *rover->bottomheight;
		}
      }
    }

    // tmfloorthing is set when tmfloorz comes from a thing's top
    tmfloorthing = NULL;

    validcount++;
    numspechit = 0;

    if ( tmflags & MF_NOCLIP )
        return true;

    // Check things first, possibly picking things up.
    // The bounding box is extended by MAXRADIUS
    // because mobj_ts are grouped into mapblocks
    // based on their origin point, and can overlap
    // into adjacent blocks by up to MAXRADIUS units.

	if(thing->player && thing->player->nightsmode)
	{
		thing->player->axishit = false;
	}

    // BP: added MF_NOCLIPTHING :used by camera to don't be blocked by things
    if(!(thing->flags & MF_NOCLIPTHING))
    {
        xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
        xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
        yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
        yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;
        
        for (bx=xl ; bx<=xh ; bx++)
            for (by=yl ; by<=yh ; by++)
                if (!P_BlockThingsIterator(bx,by,PIT_CheckThing))
                    return false;
    }

	if(thing->player && thing->player->nightsmode
		&& thing->player->axishit != true)
	{
		thing->player->axistransferred = false;
		thing->player->transferangle = -1;
	}

    // check lines
    xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if (!P_BlockLinesIterator (bx,by,PIT_CheckLine))
                return false;

    return true;
}

// P_CheckPosition optimized for the MT_HOOPCOLLIDE object. This needs to be as fast as possible!
boolean P_CheckHoopPosition ( mobj_t*       hoopthing,
                          fixed_t       x,
                          fixed_t       y, fixed_t z, fixed_t radius )
{
	int i;
	fixed_t blockdist;

	for(i=0; i<MAXPLAYERS; i++)
	{
		if(!playeringame[i])
			continue;

		if(!players[i].mo)
			continue;


		blockdist = players[i].mo->radius + radius;

		if ( abs(players[i].mo->x - x) >= blockdist ||
			 abs(players[i].mo->y - y) >= blockdist)
		{
			// didn't hit it
			return true;
		}

		if(players[i].mo->z > z+radius || players[i].mo->z+players[i].mo->height < z-radius)
			return true; // Still didn't hit it.

		// check for pickup
		if(hoopthing->flags & MF_SPECIAL)
		{
			// can remove thing
			P_TouchSpecialThing (hoopthing, players[i].mo, true);
			return false;
		}
	}

    return true;
}

// P_CheckCameraPosition
// Like P_CheckPosition, but for the camera.
// Tails 09-29-2002
boolean P_CheckCameraPosition ( fixed_t       x,
                          fixed_t       y, camera_t* thiscam )
{
    int                 xl;
    int                 xh;
    int                 yl;
    int                 yh;
    int                 bx;
    int                 by;
    subsector_t*        newsubsec;

    tmx = x;
    tmy = y;

    tmbbox[BOXTOP] = y + thiscam->radius;
    tmbbox[BOXBOTTOM] = y - thiscam->radius;
    tmbbox[BOXRIGHT] = x + thiscam->radius;
    tmbbox[BOXLEFT] = x - thiscam->radius;

    newsubsec = R_PointInSubsector (x,y);
    ceilingline = blockingline = NULL;

    // The base floor / ceiling is from the subsector
    // that contains the point.
    // Any contacted lines the step closer together
    // will adjust them.
    tmfloorz = tmsectorfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = tmsectorceilingz = newsubsec->sector->ceilingheight;

    //SoM: 3/23/2000: Check list of fake floors and see if
    //tmfloorz/tmceilingz need to be altered.
    if(newsubsec->sector->ffloors)
    {
      ffloor_t*  rover;
      fixed_t    delta1;
      fixed_t    delta2;
      int        thingtop = thiscam->z + thiscam->height;

      for(rover = newsubsec->sector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS)) continue;

        delta1 = thiscam->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > tmfloorz && abs(delta1) < abs(delta2))
          tmfloorz = tmdropoffz = *rover->topheight;
        if(*rover->bottomheight < tmceilingz && abs(delta1) >= abs(delta2))
          tmceilingz = *rover->bottomheight;
      }
    }

    // Check things first, possibly picking things up.
    // The bounding box is extended by MAXRADIUS
    // because mobj_ts are grouped into mapblocks
    // based on their origin point, and can overlap
    // into adjacent blocks by up to MAXRADIUS units.

    // check lines
    xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if (!P_BlockLinesIterator (bx,by,PIT_CheckCameraLine))
                return false;

    return true;
}



//==========================================================================
//
// CheckMissileImpact
//
//==========================================================================

static void CheckMissileImpact(mobj_t *mobj)
{
    if(!numspechit || !(mobj->flags&MF_MISSILE) || !mobj->target)
        return;

    if(!mobj->target->player)
        return;
}

// P_TryCameraMove
// Attempt to move the camera to a new position
// Tails 09-29-2002
boolean P_TryCameraMove (fixed_t       x,
                    fixed_t       y, camera_t* thiscam)
{
    fixed_t     oldx;
    fixed_t     oldy;

    floatok = false;

    if (!P_CheckCameraPosition (x, y, thiscam))
        return false;           // solid wall or thing

    if ( true )
    {
        fixed_t maxstep = MAXSTEPMOVE;

        if (tmceilingz - tmfloorz < thiscam->height)
            return false;       // doesn't fit

        floatok = true;

        if (tmceilingz - thiscam->z < thiscam->height)
            return false;       // mobj must lower itself to fit

        if ( (tmfloorz - thiscam->z > maxstep) )
            return false;       // too big a step up
    }

    // the move is ok,
    // so link the thing into its new position

    oldx = thiscam->x;
    oldy = thiscam->y;
    thiscam->floorz = tmfloorz;
    thiscam->ceilingz = tmceilingz;
    thiscam->x = x;
    thiscam->y = y;

    return true;
}

//
// P_TryMove
// Attempt to move to a new position,
// crossing special lines unless MF_TELEPORT is set.
//
boolean P_TryMove ( mobj_t*       thing,
                    fixed_t       x,
                    fixed_t       y,
                    boolean       allowdropoff)
{
    fixed_t     oldx;
    fixed_t     oldy;

    floatok = false;

    if (!P_CheckPosition (thing, x, y))
    {
        CheckMissileImpact(thing);
        return false;           // solid wall or thing
    }
#ifdef CLIENTPREDICTION2
    if ( !(thing->flags & MF_NOCLIP) && !(thing->eflags & MF_NOZCHECKING))
#else
    if ( !(thing->flags & MF_NOCLIP) )
#endif
    {
        fixed_t maxstep = MAXSTEPMOVE;

		// Don't 'step up' while springing. 'Bout time this was fixed. Tails 08-27-2002
		if(thing->player && thing->state == &states[S_PLAY_PLG1])
			maxstep = 0;
		else
			maxstep = MAXSTEPMOVE;

		if(tmceilingz - tmfloorz < thing->height
			|| thing->z > tmceilingz || thing->z < tmfloorz)
		{
			if(thing->type == MT_SHELL)
			{
				int counting;

				counting = 0;
				thing->angle += ANG45;
				P_UnsetThingPosition(thing);
				thing->x -= thing->momx;
				thing->y -= thing->momy;
				P_SetThingPosition(thing);
				P_InstaThrust(thing, thing->angle, thing->info->speed);

				while(P_TryMove (thing, thing->x, thing->y, true) == false && counting < 500)
				{
					thing->angle += ANG45;
					P_InstaThrust(thing, thing->angle, thing->info->speed);
					counting++;
				}
				S_StartSound(thing, sfx_tink);
				return true;
			}
		}

        if (tmceilingz - tmfloorz < thing->height)
        {
            CheckMissileImpact(thing);
            return false;       // doesn't fit
        }

        floatok = true;

        if (tmceilingz - thing->z < thing->height)
        {
            CheckMissileImpact(thing);
            return false;       // mobj must lower itself to fit
        }

		// Ramp test Tails
		if(thing->player && (thing->player->specialsector == 992 || R_PointInSubsector (x,y)->sector->special == 992) && thing->z == thing->floorz && tmfloorz < thing->z && thing->z - tmfloorz <= MAXSTEPMOVE)
			thing->z = tmfloorz;
		/*
		if(thing->player)
		{
				thing->player->slope = R_PointToAngle2(0, thing->z, P_AproxDistance(x - thing->x, y - thing->y), tmfloorz);
		}
*/
        // jump out of water
        if((thing->eflags & (MF_UNDERWATER|MF_TOUCHWATER))==(MF_UNDERWATER|MF_TOUCHWATER))
            maxstep=37*FRACUNIT;

        if ((tmfloorz - thing->z > maxstep ) )
        {
            CheckMissileImpact(thing);
            return false;       // too big a step up
        }

        if((thing->flags&MF_MISSILE) && tmfloorz > thing->z)
		{
			CheckMissileImpact(thing);
		}

        if ( !boomsupport || !allowdropoff)
          if ( !(thing->flags&(MF_FLOAT))
               && !tmfloorthing
               && tmfloorz - tmdropoffz > MAXSTEPMOVE )
              return false;       // don't stand over a dropoff
    }

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition (thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;

    //added:28-02-98:
    if (tmfloorthing)
        thing->eflags &= ~MF_ONGROUND;  //not on real floor
    else
        thing->eflags |= MF_ONGROUND;

    P_SetThingPosition (thing);
/*
    // if any special lines were hit, do the effect
    if ( !(thing->flags&(MF_TELEPORT|MF_NOCLIP)) &&
         (thing->type != MT_CHASECAM) && (thing->type != MT_SPIRIT))
    {
        while (numspechit--)
        {
            // see if the line was crossed
            ld = lines + spechit[numspechit];
            side = P_PointOnLineSide (thing->x, thing->y, ld);
            oldside = P_PointOnLineSide (oldx, oldy, ld);
            if (side != oldside)
            {
                if (ld->special)
                    P_CrossSpecialLine (ld-lines, oldside, thing);
            }
        }
    }*/

    return true;
}

boolean P_SceneryTryMove ( mobj_t*       thing,
							fixed_t       x,
							fixed_t       y)
{
    fixed_t     oldx;
    fixed_t     oldy;

    if (!P_CheckPosition (thing, x, y))
    {
        return false;           // solid wall or thing
    }
    if ( !(thing->flags & MF_NOCLIP) )
    {
        fixed_t maxstep = MAXSTEPMOVE;
        if (tmceilingz - tmfloorz < thing->height)
        {
            return false;       // doesn't fit
        }

        if (tmceilingz - thing->z < thing->height)
        {
            return false;       // mobj must lower itself to fit
        }

        if (tmfloorz - thing->z > maxstep)
        {
            return false;       // too big a step up
        }
    }

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition (thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;

    //added:28-02-98:
    if (tmfloorthing)
        thing->eflags &= ~MF_ONGROUND;  //not on real floor
    else
        thing->eflags |= MF_ONGROUND;

    P_SetThingPosition (thing);

    return true;
}


//
// P_ThingHeightClip
// Takes a valid thing and adjusts the thing->floorz,
// thing->ceilingz, and possibly thing->z.
// This is called for all nearby monsters
// whenever a sector changes height.
// If the thing doesn't fit,
// the z will be set to the lowest value
// and false will be returned.
//
boolean P_ThingHeightClip (mobj_t* thing)
{
    boolean             onfloor;

    onfloor = (thing->z <= thing->floorz);

    P_CheckPosition (thing, thing->x, thing->y);

    // what about stranding a monster partially off an edge?

	thing->floorz = tmfloorz;
	thing->ceilingz = tmceilingz;

	switch(thing->type)
	{
		case MT_RING:
		case MT_COIN:
		case MT_NIGHTSWING:
		case MT_BLUEORB:
		case MT_GREENORB:
		case MT_BLACKORB:
		case MT_YELLOWORB:
		case MT_THOK:
			return true; // Ignore these items Tails
		default:
			break;
	}

	// Have player fall through floor? 10-05-2001 Tails
	if(thing->player && thing->player->playerstate == PST_DEAD)
		return true;

    if (!tmfloorthing && onfloor && !(thing->flags & MF_NOGRAVITY))
    {
        // walking monsters rise and fall with the floor
        thing->z = thing->floorz;
    }
    else
    {
        // don't adjust a floating monster unless forced to
        //added:18-04-98:test onfloor
        if (!onfloor)                    //was tmsectorceilingz
            if (thing->z+thing->height > tmceilingz)
                thing->z = thing->ceilingz - thing->height;

        //thing->eflags &= ~MF_ONGROUND;
    }

    //debug : be sure it falls to the floor
    thing->eflags &= ~MF_ONGROUND;

    //added:28-02-98:
    // test sector bouding top & bottom, not things

    //if (tmsectorceilingz - tmsectorfloorz < thing->height)
    //    return false;

    if (thing->ceilingz - thing->floorz < thing->height
        // BP: i know that this code cause many trouble but this fix alos 
        // lot of problem, mainly this is implementation of the stepping 
        // for mobj (walk on solid corpse without jumping or fake 3d bridge)
        // problem is imp into imp at map01 and monster going at top of others
        && thing->z >= thing->floorz)
        return false;

    return true;
}



//
// SLIDE MOVE
// Allows the player to slide along any angled walls.
//
fixed_t         bestslidefrac;
fixed_t         secondslidefrac;

line_t*         bestslideline;
line_t*         secondslideline;

mobj_t*         slidemo;

fixed_t         tmxmove;
fixed_t         tmymove;

// P_HitCameraSlideLine
// P_HitSlideLine for camera
// Tails 09-29-2002
void P_HitCameraSlideLine (line_t* ld, camera_t* thiscam)
{
    int                 side;

    angle_t             lineangle;
    angle_t             moveangle;
    angle_t             deltaangle;

    fixed_t             movelen;
    fixed_t             newlen;


    if (ld->slopetype == ST_HORIZONTAL)
    {
        tmymove = 0;
        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {
        tmxmove = 0;
        return;
    }

    side = P_PointOnLineSide (thiscam->x, thiscam->y, ld);

    lineangle = R_PointToAngle2 (0,0, ld->dx, ld->dy);

    if (side == 1)
        lineangle += ANG180;

    moveangle = R_PointToAngle2 (0,0, tmxmove, tmymove);
    deltaangle = moveangle-lineangle;

    if (deltaangle > ANG180)
        deltaangle += ANG180;
    //  I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_AproxDistance (tmxmove, tmymove);
    newlen = FixedMul (movelen, finecosine[deltaangle]);

    tmxmove = FixedMul (newlen, finecosine[lineangle]);
    tmymove = FixedMul (newlen, finesine[lineangle]);
}

//
// P_HitSlideLine
// Adjusts the xmove / ymove
// so that the next move will slide along the wall.
//
void P_HitSlideLine (line_t* ld)
{
    int                 side;

    angle_t             lineangle;
    angle_t             moveangle;
    angle_t             deltaangle;

    fixed_t             movelen;
    fixed_t             newlen;


    if (ld->slopetype == ST_HORIZONTAL)
    {
        tmymove = 0;
        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {
        tmxmove = 0;
        return;
    }

    side = P_PointOnLineSide (slidemo->x, slidemo->y, ld);

    lineangle = R_PointToAngle2 (0,0, ld->dx, ld->dy);

    if (side == 1)
        lineangle += ANG180;

    moveangle = R_PointToAngle2 (0,0, tmxmove, tmymove);
    deltaangle = moveangle-lineangle;

    if (deltaangle > ANG180)
        deltaangle += ANG180;
    //  I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_AproxDistance (tmxmove, tmymove);
    newlen = FixedMul (movelen, finecosine[deltaangle]);

    tmxmove = FixedMul (newlen, finecosine[lineangle]);
    tmymove = FixedMul (newlen, finesine[lineangle]);
}

// Graue 12-31-2003
//
// P_HitBounceLine
// Adjusts the xmove / ymove
// so that the next move will bounce off the wall.
//
void P_HitBounceLine (line_t* ld)
{
    int                 side;

    angle_t             lineangle;
    angle_t             moveangle;
    angle_t             deltaangle;

    fixed_t             movelen;
    //fixed_t             newlen;


    if (ld->slopetype == ST_HORIZONTAL)
    {
        tmymove = -tmymove;
		//if(tmxmove < 0)
		//	tmxmove = tmxmove + (abs(tmxmove)>>3);
		//else
		//	tmxmove = tmxmove - (tmxmove>>3);
		//if(tmymove < 0)
		//	tmymove = tmymove + (abs(tmymove)>>3);
		//else
		//	tmymove = tmymove - (tmymove>>3);
        return;
    }

    if (ld->slopetype == ST_VERTICAL)
    {
        tmxmove = -tmxmove;
		//if(tmxmove < 0)
		//	tmxmove = tmxmove + (abs(tmxmove)>>3);
		//else
		//	tmxmove = tmxmove - (tmxmove>>3);
		//if(tmymove < 0)
		//	tmymove = tmymove + (abs(tmymove)>>3);
		//else
		//	tmymove = tmymove - (tmymove>>3);
        return;
    }

    side = P_PointOnLineSide (slidemo->x, slidemo->y, ld);

    lineangle = R_PointToAngle2 (0,0, ld->dx, ld->dy);

	//CONS_Printf("Line angle is once %f (heh: %d)\n",
	//	(float)lineangle*180/ANG180, rand());

    //if (side == 1)
	if(lineangle >= ANG180)
        lineangle -= ANG180;

	//CONS_Printf("Line angle is then %f (heh: %d)\n",
	//	(float)lineangle*180/ANG180, rand());

    moveangle = R_PointToAngle2 (0,0, tmxmove, tmymove);
	//CONS_Printf("Move angle is %f (heh: %d)\n",
	//	(float)moveangle*180/ANG180, rand());
    deltaangle = moveangle+(2*(lineangle-moveangle));
	//CONS_Printf("Delt angle is %f (heh: %d)\n",
	//	(float)deltaangle*180/ANG180, rand());

    //if (deltaangle > ANG180)
    //    deltaangle += ANG180;
    //  I_Error ("SlideLine: ang>ANG180");

    lineangle >>= ANGLETOFINESHIFT;
    deltaangle >>= ANGLETOFINESHIFT;

    movelen = P_AproxDistance (tmxmove, tmymove);
	//newlen = movelen - (movelen>>3); // 7/8 of original velocity
    //newlen = FixedMul (movelen, finecosine[deltaangle]);

    tmxmove = FixedMul (movelen, finecosine[deltaangle]);
    tmymove = FixedMul (movelen, finesine[deltaangle]);

	deltaangle = R_PointToAngle2(0,0,tmxmove,tmymove);
	//CONS_Printf("Tled angle is %f (heh: %d)\n",
	//	(float)deltaangle*180/ANG180, rand());
}

// PTR_SlideCameraTraverse
// PTR_SlideTraverse for the camera
// Tails 09-29-2002
boolean PTR_SlideCameraTraverse (intercept_t* in)
{
    line_t*     li;

#ifdef PARANOIA
    if (!in->isaline)
        I_Error ("PTR_SlideTraverse: not a line?");
#endif

    li = in->d.line;

    if ( ! (li->flags & ML_TWOSIDED) )
    {
        if (P_PointOnLineSide (mapcampointer->x, mapcampointer->y, li))
        {
            // don't hit the back side
            return true;
        }
        goto isblocking;
    }

    // set openrange, opentop, openbottom
    P_CameraLineOpening (li);

    if (openrange < mapcampointer->height)
        goto isblocking;                // doesn't fit

    if (opentop - mapcampointer->z < mapcampointer->height)
        goto isblocking;                // mobj is too high

    if (openbottom - mapcampointer->z > 24*FRACUNIT )
        goto isblocking;                // too big a step up

    // this line doesn't block movement
    return true;

    // the line does block movement,
    // see if it is closer than best so far
  isblocking:
	{
    if (in->frac < bestslidefrac)
    {
		secondslidefrac = bestslidefrac;
		secondslideline = bestslideline;
		bestslidefrac = in->frac;
		bestslideline = li;
    }
	}

    return false;       // stop
}

fixed_t P_ReturnThrustX(mobj_t* mo, angle_t angle, fixed_t move);
fixed_t P_ReturnThrustY(mobj_t* mo, angle_t angle, fixed_t move);
boolean P_IsClimbingValid(player_t* player)
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

				if((*rover->bottomheight > player->mo->z) && ((player->mo->z - player->mo->momz) > *rover->bottomheight))
				{
					floorclimb = true;
					boostup = false;
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
		}
		else if(player->mo->z >= glidesector->sector->ceilingheight)
		{
			floorclimb = true;
		}

		climb = false;

		if(floorclimb == false)
		{
		//	if(boostup)
		//		player->mo->momz += 2*FRACUNIT/NEWTICRATERATIO;
		//	if(thrust)
		//		P_InstaThrust(player->mo, player->mo->angle, 4*FRACUNIT); // Lil' boost up.

			return false;
		}

		if(skyclimber == true)
		{
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}

	return true;
}

//
// PTR_SlideTraverse
//
boolean PTR_SlideTraverse (intercept_t* in)
{
    line_t*     li;

#ifdef PARANOIA
    if (!in->isaline)
        I_Error ("PTR_SlideTraverse: not a line?");
#endif

    li = in->d.line;

    if ( ! (li->flags & ML_TWOSIDED) )
    {
        if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
        {
            // don't hit the back side
            return true;
        }
        goto isblocking;
    }

    // set openrange, opentop, openbottom
    P_LineOpening (li);

    if (openrange < slidemo->height)
        goto isblocking;                // doesn't fit

    if (opentop - slidemo->z < slidemo->height)
        goto isblocking;                // mobj is too high

    if (openbottom - slidemo->z > 24*FRACUNIT )
        goto isblocking;                // too big a step up

    // this line doesn't block movement
    return true;

    // the line does block movement,
    // see if it is closer than best so far
  isblocking:
	{
		if(slidemo->player) // Whole hella climbin' stuff goin' on here, sucka! Tails 04-12-2001
		{
			if(slidemo->player->gliding || slidemo->player->climbing)
			{
				angle_t climbline;
				int linequadrant;
				int playerquadrant;

				if(li->flags & ML_NOCLIMB) // Don't cling on this wall Tails 02-02-2002
					goto noclimb;

				linequadrant = playerquadrant = 0;

				climbline = R_PointToAngle2(li->v1->x, li->v1->y, li->v2->x, li->v2->y);

				if(climbline >= 0 && climbline < ANG90) // 1st Quadrant
				{
					linequadrant = 1;
					//CONS_Printf("The line quadrant is 1\n");
				}
				else if(climbline >= ANG90 && climbline < ANG180) // 2nd Quadrant
				{
					linequadrant = 2;
					//CONS_Printf("The line quadrant is 2\n");
				}
				else if(climbline >= ANG180 && climbline < ANG270) // 3rd Quadrant
				{
					linequadrant = 3;
					//CONS_Printf("The line quadrant is 3\n");
				}
				else // 4th Quadrant
				{
					linequadrant = 4;
					//CONS_Printf("The line quadrant is 4\n");
				}

				if(slidemo->angle >= 0 && slidemo->angle < ANG90) // 1st Quadrant
				{
					playerquadrant = 1;
					//CONS_Printf("The player quadrant is 1\n");
				}
				else if(slidemo->angle >= ANG90 && slidemo->angle < ANG180) // 2nd Quadrant
				{
					playerquadrant = 2;
					//CONS_Printf("The player quadrant is 2\n");
				}
				else if(slidemo->angle >= ANG180 && slidemo->angle < ANG270) // 3rd Quadrant
				{
					playerquadrant = 3;
					//CONS_Printf("The player quadrant is 3\n");
				}
				else // 4th Quadrant
				{
					playerquadrant = 4;
					//CONS_Printf("The player quadrant is 4\n");
				}

				// If on first side of linedef
				if(P_PointOnLineSide(slidemo->x, slidemo->y, li) == 0)
				{
					if ((!slidemo->player->climbing && ((linequadrant == 1) && (playerquadrant == 2))
						|| ((linequadrant == 2) && (playerquadrant == 3))
						|| ((linequadrant == 3) && (playerquadrant == 4))
						|| ((linequadrant == 4) && (playerquadrant == 1)))

						|| (slidemo->player->climbing == 1 &&
						((linequadrant == 1) && (playerquadrant == 2 || playerquadrant == 3 || playerquadrant == 1))
						|| ((linequadrant == 2) && (playerquadrant == 3 || playerquadrant == 2 || playerquadrant == 4))
						|| ((linequadrant == 3) && (playerquadrant == 4 || playerquadrant == 1 || playerquadrant == 3))
						|| ((linequadrant == 4) && (playerquadrant == 1 || playerquadrant == 2 || playerquadrant == 4))))
					{
						int i;
						angle_t oldangle = slidemo->angle;

						slidemo->angle = R_PointToAngle2(li->v1->x, li->v1->y, li->v2->x, li->v2->y) + ANG90;
						
						if (slidemo->player==&players[consoleplayer])
							localangle = slidemo->angle;
						else if(cv_splitscreen.value && slidemo->player==&players[secondarydisplayplayer])
							localangle2 = slidemo->angle;

						if(!(P_IsClimbingValid(slidemo->player)))
						{
							slidemo->angle = oldangle;

							if (slidemo->player==&players[consoleplayer])
								localangle = slidemo->angle;
							else if(cv_splitscreen.value && slidemo->player==&players[secondarydisplayplayer])
								localangle2 = slidemo->angle;

							goto noclimb;
						}

						if(!slidemo->player->climbing)
							slidemo->player->climbing = 5;

						slidemo->player->gliding = 0;
						slidemo->player->glidetime = 0;
						
						slidemo->player->mfspinning = slidemo->player->mfjumped = 0;

						slidemo->player->thokked = false;

						if(slidemo->player->climbing > 1)
							slidemo->momz = slidemo->momx = slidemo->momy = 0;

						slidemo->player->lastsidehit = li->sidenum[0];

						for (i = 0;i < numlines; i++)
							if(&lines[i] == li)
								slidemo->player->lastlinehit = i;

						P_Thrust(slidemo, slidemo->angle, 5*FRACUNIT);
					}
				}
				else // 2nd side of linedef
				{
					if ((linequadrant == 1 && playerquadrant == 4)
						|| (linequadrant == 2 && playerquadrant == 1)
						|| (linequadrant == 3 && playerquadrant == 2)
						|| (linequadrant == 4 && playerquadrant == 3))
					{
//						int i;
						angle_t oldangle = slidemo->angle;

						slidemo->angle = R_PointToAngle2(li->v1->x, li->v1->y, li->v2->x, li->v2->y) - ANG90;
						
						if (slidemo->player==&players[consoleplayer])
							localangle = slidemo->angle;
						else if(cv_splitscreen.value && slidemo->player==&players[secondarydisplayplayer])
							localangle2 = slidemo->angle;

						if(!(P_IsClimbingValid(slidemo->player)))
						{
							slidemo->angle = oldangle;

							if (slidemo->player==&players[consoleplayer])
								localangle = slidemo->angle;
							else if(cv_splitscreen.value && slidemo->player==&players[secondarydisplayplayer])
								localangle2 = slidemo->angle;

							goto noclimb;
						}

						if(!slidemo->player->climbing)
							slidemo->player->climbing = 5;

						slidemo->player->gliding = 0;
						slidemo->player->glidetime = 0;
						
						slidemo->player->mfspinning = slidemo->player->mfjumped = 0;

						slidemo->player->thokked = false;
						
						if(slidemo->player->climbing > 1)
							slidemo->momz = slidemo->momx = slidemo->momy = 0;

						// 2nd sides cannot scroll, so we will not bother with them.
						
						P_Thrust(slidemo, slidemo->angle, 5*FRACUNIT);
					}
				}
			}
		}
noclimb:
    if (in->frac < bestslidefrac)
    {
		if(slidemo->player && !slidemo->player->climbing) // More climbin' Tails 04-12-2001
		{
			secondslidefrac = bestslidefrac;
			secondslideline = bestslideline;
			bestslidefrac = in->frac;
			bestslideline = li;
		}
		else if (!slidemo->player) // More climbin' Tails 04-12-2001
		{
			secondslidefrac = bestslidefrac;
			secondslideline = bestslideline;
			bestslidefrac = in->frac;
			bestslideline = li;
		}
    }
	}

    return false;       // stop
}

// P_SlideCameraMove
// Tries to slide the camera along a wall.
// Tails 09-29-2002
void P_SlideCameraMove (camera_t* thiscam)
{
    fixed_t             leadx;
    fixed_t             leady;
    fixed_t             trailx;
    fixed_t             traily;
    fixed_t             newx;
    fixed_t             newy;
    int                 hitcount;

    hitcount = 0;

  retry:
    if (++hitcount == 3)
        goto stairstep;         // don't loop forever


    // trace along the three leading corners
    if (thiscam->momx > 0)
    {
        leadx = thiscam->x + thiscam->radius;
        trailx = thiscam->x - thiscam->radius;
    }
    else
    {
        leadx = thiscam->x - thiscam->radius;
        trailx = thiscam->x + thiscam->radius;
    }

    if (thiscam->momy > 0)
    {
        leady = thiscam->y + thiscam->radius;
        traily = thiscam->y - thiscam->radius;
    }
    else
    {
        leady = thiscam->y - thiscam->radius;
        traily = thiscam->y + thiscam->radius;
    }

    bestslidefrac = FRACUNIT+1;

	mapcampointer = thiscam;

    P_PathTraverse ( leadx, leady, leadx+thiscam->momx, leady+thiscam->momy,
                     PT_ADDLINES, PTR_SlideCameraTraverse);
    P_PathTraverse ( trailx, leady, trailx+thiscam->momx, leady+thiscam->momy,
                     PT_ADDLINES, PTR_SlideCameraTraverse);
    P_PathTraverse ( leadx, traily, leadx+thiscam->momx, traily+thiscam->momy,
                     PT_ADDLINES, PTR_SlideCameraTraverse);

    // move up to the wall
    if (bestslidefrac == FRACUNIT+1)
    {
        // the move most have hit the middle, so stairstep
      stairstep:
        if (!P_TryCameraMove (thiscam->x, thiscam->y + thiscam->momy, thiscam)) //SoM: 4/10/2000
            P_TryCameraMove (thiscam->x + thiscam->momx, thiscam->y, thiscam);  //Allow things to
        return;                                             //drop off.
    }

    // fudge a bit to make sure it doesn't hit
    bestslidefrac -= 0x800;
    if (bestslidefrac > 0)
    {
        newx = FixedMul (thiscam->momx, bestslidefrac);
        newy = FixedMul (thiscam->momy, bestslidefrac);

        if (!P_TryCameraMove (thiscam->x+newx, thiscam->y+newy, thiscam))
            goto stairstep;
    }

    // Now continue along the wall.
    // First calculate remainder.
    bestslidefrac = FRACUNIT-(bestslidefrac+0x800);

    if (bestslidefrac > FRACUNIT)
        bestslidefrac = FRACUNIT;

    if (bestslidefrac <= 0)
        return;

    tmxmove = FixedMul (thiscam->momx, bestslidefrac);
    tmymove = FixedMul (thiscam->momy, bestslidefrac);

    P_HitCameraSlideLine (bestslideline, thiscam);     // clip the moves

    thiscam->momx = tmxmove;
    thiscam->momy = tmymove;

    if (!P_TryCameraMove (thiscam->x+tmxmove, thiscam->y+tmymove, thiscam))
    {
        goto retry;
    }
}

//
// P_SlideMove
// The momx / momy move is bad, so try to slide
// along a wall.
// Find the first line hit, move flush to it,
// and slide along it
//
// This is a kludgy mess.
//
void P_SlideMove (mobj_t* mo)
{
    fixed_t             leadx;
    fixed_t             leady;
    fixed_t             trailx;
    fixed_t             traily;
    fixed_t             newx;
    fixed_t             newy;
    int                 hitcount;

    slidemo = mo;
    hitcount = 0;

  retry:
    if (++hitcount == 3)
        goto stairstep;         // don't loop forever


    // trace along the three leading corners
    if (mo->momx > 0)
    {
        leadx = mo->x + mo->radius;
        trailx = mo->x - mo->radius;
    }
    else
    {
        leadx = mo->x - mo->radius;
        trailx = mo->x + mo->radius;
    }

    if (mo->momy > 0)
    {
        leady = mo->y + mo->radius;
        traily = mo->y - mo->radius;
    }
    else
    {
        leady = mo->y - mo->radius;
        traily = mo->y + mo->radius;
    }

    bestslidefrac = FRACUNIT+1;

    P_PathTraverse ( leadx, leady, leadx+mo->momx, leady+mo->momy,
                     PT_ADDLINES, PTR_SlideTraverse );
    P_PathTraverse ( trailx, leady, trailx+mo->momx, leady+mo->momy,
                     PT_ADDLINES, PTR_SlideTraverse );
    P_PathTraverse ( leadx, traily, leadx+mo->momx, traily+mo->momy,
                     PT_ADDLINES, PTR_SlideTraverse );

    // move up to the wall
    if (bestslidefrac == FRACUNIT+1)
    {
        // the move most have hit the middle, so stairstep
      stairstep:
        if (!P_TryMove (mo, mo->x, mo->y + mo->momy, true)) //SoM: 4/10/2000
            P_TryMove (mo, mo->x + mo->momx, mo->y, true);  //Allow things to
        return;                                             //drop off.
    }

    // fudge a bit to make sure it doesn't hit
    bestslidefrac -= 0x800;
    if (bestslidefrac > 0)
    {
        newx = FixedMul (mo->momx, bestslidefrac);
        newy = FixedMul (mo->momy, bestslidefrac);

        if (!P_TryMove (mo, mo->x+newx, mo->y+newy, true))
            goto stairstep;
    }

    // Now continue along the wall.
    // First calculate remainder.
    bestslidefrac = FRACUNIT-(bestslidefrac+0x800);

    if (bestslidefrac > FRACUNIT)
        bestslidefrac = FRACUNIT;

    if (bestslidefrac <= 0)
        return;

    tmxmove = FixedMul (mo->momx, bestslidefrac);
    tmymove = FixedMul (mo->momy, bestslidefrac);

    P_HitSlideLine (bestslideline);     // clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;

    if (!P_TryMove (mo, mo->x+tmxmove, mo->y+tmymove, true))
    {
        goto retry;
    }
}

// Graue 12-31-2003
//
// P_BounceMove
// The momx / momy move is bad, so try to bounce
// off a wall.
//
// This is a kludgy mess. (Yes, I too.) (Me also!) (No, P_BounceMove only.)
//
void P_BounceMove (mobj_t* mo)
{
    fixed_t             leadx;
    fixed_t             leady;
    fixed_t             trailx;
    fixed_t             traily;
    fixed_t             newx;
    fixed_t             newy;
    int                 hitcount;
	fixed_t mmomx, mmomy;

    slidemo = mo;
    hitcount = 0;

  retry:
    if (++hitcount == 3)
        goto stairstep;         // don't loop forever

	if(mo->player)
	{
		mmomx = mo->player->rmomx;
		mmomy = mo->player->rmomy;
	}
	else
	{
		mmomx = mo->momx;
		mmomy = mo->momy;
	}

    // trace along the three leading corners
    if (mo->momx > 0)
    {
        leadx = mo->x + mo->radius;
        trailx = mo->x - mo->radius;
    }
    else
    {
        leadx = mo->x - mo->radius;
        trailx = mo->x + mo->radius;
    }

    if (mo->momy > 0)
    {
        leady = mo->y + mo->radius;
        traily = mo->y - mo->radius;
    }
    else
    {
        leady = mo->y - mo->radius;
        traily = mo->y + mo->radius;
    }

    bestslidefrac = FRACUNIT+1;

    P_PathTraverse ( leadx, leady, leadx+mmomx, leady+mmomy,
                     PT_ADDLINES, PTR_SlideTraverse );
    P_PathTraverse ( trailx, leady, trailx+mmomx, leady+mmomy,
                     PT_ADDLINES, PTR_SlideTraverse );
    P_PathTraverse ( leadx, traily, leadx+mmomx, traily+mmomy,
                     PT_ADDLINES, PTR_SlideTraverse );

    // move up to the wall
    if (bestslidefrac == FRACUNIT+1)
    {
        // the move most have hit the middle, so stairstep
      stairstep:
		//CONS_Printf("Stairstepping (heh: %d)\n", rand());
        if (!P_TryMove (mo, mo->x, mo->y + mmomy, true)) //SoM: 4/10/2000
            P_TryMove (mo, mo->x + mmomx, mo->y, true);  //Allow things to
        return;                                             //drop off.
    }

	// fudge a bit to make sure it doesn't hit
	bestslidefrac -= 0x800;
	if (bestslidefrac > 0)
	{
		//CONS_Printf("Fudging a bit (heh: %d)\n", rand());
	    newx = FixedMul (mmomx, bestslidefrac);
	    newy = FixedMul (mmomy, bestslidefrac);

	    if (!P_TryMove (mo, mo->x+newx, mo->y+newy, true))
	        goto stairstep;
	}

	// Now continue along the wall.
	// First calculate remainder.
	bestslidefrac = FRACUNIT-bestslidefrac;

    if (bestslidefrac > FRACUNIT)
	{
		//CONS_Printf("Twinge! %d\n",rand());
        bestslidefrac = FRACUNIT;
	}

    if (bestslidefrac <= 0)
	{
		//CONS_Printf("!@# !@# !@# !@# !@# !@# !@# !@# !@# %d\n",rand());
        return;
	}

	//CONS_Printf("bestslidefrac is %d, FRACUNIT - whatever is %d\n",
	//	bestslidefrac, (FRACUNIT - (FRACUNIT>>2) - (FRACUNIT>>3)));

    tmxmove = FixedMul (mmomx, (FRACUNIT - (FRACUNIT>>2) - (FRACUNIT>>3)));
    tmymove = FixedMul (mmomy, (FRACUNIT - (FRACUNIT>>2) - (FRACUNIT>>3)));

    P_HitBounceLine (bestslideline);     // clip the moves

    mo->momx = tmxmove;
    mo->momy = tmymove;
	
	if(mo->player)
	{
		mo->player->cmomx = tmxmove;
		mo->player->cmomy = tmymove;
	}

    if (!P_TryMove (mo, mo->x+tmxmove, mo->y+tmymove, true))
    {
		//CONS_Printf("RETRYING LIKE THSIHTI!!!!! (heh: %d)\n", rand());
        goto retry;
    }
}

//
// P_LineAttack
//
mobj_t*         linetarget;     // who got hit (or NULL)
mobj_t*         shootthing;

// Height if not aiming up or down
// ???: use slope for monsters?
fixed_t         shootz;
fixed_t         lastz; //SoM: The last z height of the bullet when it crossed a line

int             la_damage;
fixed_t         attackrange;

fixed_t         aimslope;


//
// PTR_AimTraverse
// Sets linetarget and aimslope when a target is aimed at.
//
//added:15-02-98: comment
// Returns true if the thing is not shootable, else continue through..
//
boolean PTR_AimTraverse (intercept_t* in)
{
    line_t*             li;
    mobj_t*             th;
    fixed_t             slope;
    fixed_t             thingtopslope;
    fixed_t             thingbottomslope;
    fixed_t             dist;
    int                 dir;

    if (in->isaline)
    {
        li = in->d.line;

        if ( !(li->flags & ML_TWOSIDED) )
            return false;               // stop

        // Crosses a two sided line.
        // A two sided line will restrict
        // the possible target ranges.
        tmthing = NULL;
        P_LineOpening (li);

        if (openbottom >= opentop)
            return false;               // stop

        dist = FixedMul (attackrange, in->frac);

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            slope = FixedDiv (openbottom - shootz , dist);
            if (slope > bottomslope)
                bottomslope = slope;
        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            slope = FixedDiv (opentop - shootz , dist);
            if (slope < topslope)
                topslope = slope;
        }

        if (topslope <= bottomslope)
            return false;               // stop

        if(li->frontsector->ffloors || li->backsector->ffloors)
        {
          int  frontflag;

          dir = aimslope > 0 ? 1 : aimslope < 0 ? -1 : 0;

          frontflag = P_PointOnLineSide(shootthing->x, shootthing->y, li);

          //SoM: Check 3D FLOORS!
          if(li->frontsector->ffloors)
          {
            ffloor_t*  rover = li->frontsector->ffloors;
            fixed_t    highslope, lowslope;

            for(; rover; rover = rover->next)
            {
              if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS)) continue;

              highslope = FixedDiv (*rover->topheight - shootz, dist);
              lowslope = FixedDiv (*rover->bottomheight - shootz, dist);
              if((aimslope >= lowslope && aimslope <= highslope))
                return false;

              if(lastz > *rover->topheight && dir == -1 && aimslope < highslope)
                frontflag |= 0x2;

              if(lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
                frontflag |= 0x2;
            }
          }

          if(li->backsector->ffloors)
          {
            ffloor_t*  rover = li->backsector->ffloors;
            fixed_t    highslope, lowslope;

            for(; rover; rover = rover->next)
            {
              if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS)) continue;

              highslope = FixedDiv (*rover->topheight - shootz, dist);
              lowslope = FixedDiv (*rover->bottomheight - shootz, dist);
              if((aimslope >= lowslope && aimslope <= highslope))
                return false;

              if(lastz > *rover->topheight && dir == -1 && aimslope < highslope)
                frontflag |= 0x4;

              if(lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
                frontflag |= 0x4;
            }
          }
          if((!(frontflag & 0x1) && frontflag & 0x2) || (frontflag & 0x1 && frontflag & 0x4))
            return false;
        }

        lastz = FixedMul (aimslope, dist) + shootz;

        return true;                    // shot continues
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
        return true;                    // can't shoot self

    if (!(th->flags&MF_SHOOTABLE))
        return true;                    // corpse or something

    // check angles to see if the thing can be aimed at
    dist = FixedMul (attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    //added:15-02-98: bottomslope is negative!
    if (thingtopslope < bottomslope)
        return true;                    // shot over the thing

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > topslope)
        return true;                    // shot under the thing

    // this thing can be hit!
    if (thingtopslope > topslope)
        thingtopslope = topslope;

    if (thingbottomslope < bottomslope)
        thingbottomslope = bottomslope;

    //added:15-02-98: find the slope just in the middle(y) of the thing!
    aimslope = (thingtopslope+thingbottomslope)/2;
    linetarget = th;

    return false;                       // don't go any farther
}


//
// PTR_ShootTraverse
//
//added:18-02-98: added clipping the shots on the floor and ceiling.
//
boolean PTR_ShootTraverse (intercept_t* in)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
    fixed_t             frac;

    line_t*             li;
    sector_t*           sector=NULL;
    mobj_t*             th;

    fixed_t             slope;
    fixed_t             dist;
    fixed_t             thingtopslope;
    fixed_t             thingbottomslope;

    fixed_t             floorz = 0;  //SoM: Bullets should hit fake floors!
    fixed_t             ceilingz = 0;

    //added:18-02-98:
    fixed_t        distz;    //dist between hit z on wall       and gun z
    fixed_t        clipz;    //dist between hit z on floor/ceil and gun z
    boolean        hitplane;    //true if we clipped z on floor/ceil plane
    boolean        diffheights; //check for sky hacks with different ceil heights

    int            sectorside;
    int            dir;

    if(aimslope > 0)
      dir = 1;
    else if(aimslope < 0)
      dir = -1;
    else
      dir = 0;

    if (in->isaline)
    {
        //shut up compiler, otherwise it's only used when TWOSIDED
        diffheights = false;

        li = in->d.line;

        if ( !(li->flags & ML_TWOSIDED) )
            goto hitline;

        // crosses a two sided line
        //added:16-02-98: Fab comments : sets opentop, openbottom, openrange
        //                lowfloor is the height of the lowest floor
        //                         (be it front or back)
        tmthing = NULL;
        P_LineOpening (li);

        dist = FixedMul (attackrange, in->frac);

        // hit lower texture ?
        if (li->frontsector->floorheight != li->backsector->floorheight)
        {
            //added:18-02-98: comments :
            // find the slope aiming on the border between the two floors
            slope = FixedDiv (openbottom - shootz , dist);
            if (slope > aimslope)
                goto hitline;
        }

        // hit upper texture ?
        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {
            //added:18-02-98: remember : diff ceil heights
            diffheights = true;

            slope = FixedDiv (opentop - shootz , dist);
            if (slope < aimslope)
                goto hitline;
        }

        if(li->frontsector->ffloors || li->backsector->ffloors)
        {
          int  frontflag;

          frontflag = P_PointOnLineSide(shootthing->x, shootthing->y, li);

          //SoM: Check 3D FLOORS!
          if(li->frontsector->ffloors)
          {
            ffloor_t*  rover = li->frontsector->ffloors;
            fixed_t    highslope, lowslope;

            for(; rover; rover = rover->next)
            {
              if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS)) continue;

              highslope = FixedDiv (*rover->topheight - shootz, dist);
              lowslope = FixedDiv (*rover->bottomheight - shootz, dist);
              if((aimslope >= lowslope && aimslope <= highslope))
                goto hitline;

              if(lastz > *rover->topheight && dir == -1 && aimslope < highslope)
                frontflag |= 0x2;

              if(lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
                frontflag |= 0x2;
            }
          }

          if(li->backsector->ffloors)
          {
            ffloor_t*  rover = li->backsector->ffloors;
            fixed_t    highslope, lowslope;

            for(; rover; rover = rover->next)
            {
              if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS)) continue;

              highslope = FixedDiv (*rover->topheight - shootz, dist);
              lowslope = FixedDiv (*rover->bottomheight - shootz, dist);
              if((aimslope >= lowslope && aimslope <= highslope))
                goto hitline;

              if(lastz > *rover->topheight && dir == -1 && aimslope < highslope)
                frontflag |= 0x4;

              if(lastz < *rover->bottomheight && dir == 1 && aimslope > lowslope)
                frontflag |= 0x4;
            }
          }
          if((!(frontflag & 0x1) && frontflag & 0x2) || (frontflag & 0x1 && frontflag & 0x4))
            goto hitline;
        }
        lastz = FixedMul (aimslope, dist) + shootz;

        // shot continues
        return true;


        // hit line
      hitline:

        // position a bit closer
        frac = in->frac - FixedDiv (4*FRACUNIT,attackrange);
        dist = FixedMul (frac, attackrange);    //dist to hit on line

        distz = FixedMul (aimslope, dist);      //z add between gun z and hit z
        z = shootz + distz;                     // hit z on wall

        //added:17-02-98: clip shots on floor and ceiling
        //                use a simple triangle stuff a/b = c/d ...
        // BP:13-3-99: fix the side usage
        hitplane = false;
        sectorside=P_PointOnLineSide(shootthing->x,shootthing->y,li);
        if( li->sidenum[sectorside] != -1 ) // can happen in nocliping mode
        {
            sector = sides[li->sidenum[sectorside]].sector;
            floorz = sector->floorheight;
            ceilingz = sector->ceilingheight;
            if(sector->ffloors)
            {
              ffloor_t* rover;
              for(rover = sector->ffloors; rover; rover = rover->next)
              {
                if(!(rover->flags & FF_SOLID)) continue;

                if(dir == 1 && *rover->bottomheight < ceilingz && *rover->bottomheight > lastz)
				{
					if(!(rover->flags & FF_PLATFORM))
						ceilingz = *rover->bottomheight;
				}
                if(dir == -1 && *rover->topheight > floorz && *rover->topheight < lastz)
                  floorz = *rover->topheight;
              }
            }

            if ((z > ceilingz) && distz)
            {
                clipz = ceilingz - shootz;
                frac = FixedDiv( FixedMul(frac,clipz), distz );
                hitplane = true;
            }
            else
                if ((z < floorz) && distz)
                {
                    clipz = shootz - floorz;
                    frac = -FixedDiv( FixedMul(frac,clipz), distz );
                    hitplane = true;
                }
            if(sector->ffloors)
            {
                if(dir == 1 && z > ceilingz)
                    z = ceilingz;
                if(dir == -1 && z < floorz)
                    z = floorz;
            }
        }
        //SPLAT TEST ----------------------------------------------------------
        #ifdef WALLSPLATS
        if (!hitplane)
        {
            divline_t   divl;
            fixed_t     frac;

            P_MakeDivline (li, &divl);
            frac = P_InterceptVector (&divl, &trace);
            R_AddWallSplat (li, sectorside, "A_DMG1", z, frac, SPLATDRAWMODE_SHADE);
        }
        #endif
        // --------------------------------------------------------- SPLAT TEST


        x = trace.x + FixedMul (trace.dx, frac);
        y = trace.y + FixedMul (trace.dy, frac);

        if (li->frontsector->ceilingpic == skyflatnum)
        {
            // don't shoot the sky!
            if (z > li->frontsector->ceilingheight)
                return false;

            // it's a sky hack wall
            if  ((!hitplane &&      //added:18-02-98:not for shots on planes
                 li->backsector &&
                 diffheights &&    //added:18-02-98:skip only REAL sky hacks
                                   //   eg: they use different ceil heights.
                 li->backsector->ceilingpic == skyflatnum))
              return false;
        }

        if(sector && sector->ffloors)
        {
          if(dir == 1 && z + (16 << FRACBITS) > ceilingz)
            z = ceilingz - (16 << FRACBITS);
          if(dir == -1 && z < floorz)
            z = floorz;
        }
        // Spawn bullet puffs.
//        P_SpawnPuff (x,y,z);

        // don't go any farther
        return false;
    }

    // shoot a thing
    th = in->d.thing;
    if (th == shootthing)
        return true;            // can't shoot self

    if (!(th->flags&MF_SHOOTABLE))
        return true;            // corpse or something

    // check angles to see if the thing can be aimed at
    dist = FixedMul (attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    if (thingtopslope < aimslope)
        return true;            // shot over the thing

    thingbottomslope = FixedDiv (th->z - shootz, dist);

    if (thingbottomslope > aimslope)
        return true;            // shot under the thing

    // SoM: SO THIS IS THE PROBLEM!!!
    // heh.
    // A bullet would travel through a 3D floor until it hit a LINEDEF! Thus
    // it appears that the bullet hits the 3D floor but it actually just hits
    // the line behind it. Thus allowing a bullet to hit things under a 3D
    // floor and still be clipped a 3D floor.
    if(th->subsector->sector->ffloors)
    {
      sector_t* sector = th->subsector->sector;
      ffloor_t* rover;

      for(rover = sector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SOLID))
          continue;

        if(dir == -1 && *rover->topheight < lastz && *rover->topheight > th->z + th->height)
          return true;
        if(dir == 1 && *rover->bottomheight > lastz && *rover->bottomheight < th->z)
          return true;
      }
    }

    // hit thing
    // position a bit closer
    frac = in->frac - FixedDiv (10*FRACUNIT,attackrange);

    x = trace.x + FixedMul (trace.dx, frac);
    y = trace.y + FixedMul (trace.dy, frac);
    z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

    if (la_damage)
        hitplane = P_DamageMobj (th, shootthing, shootthing, la_damage);
    else
        hitplane = false;

    // Spawn bullet puffs or blood spots,
    // depending on target type.
//    if (in->d.thing->flags & MF_NOBLOOD)
//        P_SpawnPuff (x,y,z);

    // don't go any farther
    return false;

}


//
// P_AimLineAttack
//
fixed_t P_AimLineAttack ( mobj_t*       t1,
                          angle_t       angle,
                          fixed_t       distance )
{
    fixed_t     x2;
    fixed_t     y2;

#ifdef PARANOIA
    if(!t1)
       I_Error("P_aimlineattack: mobj == NULL !!!");
#endif

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;

    if(t1->player)
    {
        fixed_t cosineaiming=finecosine[t1->player->aiming>>ANGLETOFINESHIFT];
        int aiming=((int)t1->player->aiming)>>ANGLETOFINESHIFT;
        x2 = t1->x + FixedMul(FixedMul(distance,finecosine[angle]),cosineaiming);
        y2 = t1->y + FixedMul(FixedMul(distance,finesine[angle]),cosineaiming); 

        topslope    =  100*FRACUNIT/160+finetangent[(2048+aiming) & FINEMASK];
        bottomslope = -100*FRACUNIT/160+finetangent[(2048+aiming) & FINEMASK];
    }
    else
    {
        x2 = t1->x + (distance>>FRACBITS)*finecosine[angle];
        y2 = t1->y + (distance>>FRACBITS)*finesine[angle];

        //added:15-02-98: Fab comments...
        // Doom's base engine says that at a distance of 160,
        // the 2d graphics on the plane x,y correspond 1/1 with plane units
        topslope = 100*FRACUNIT/160;
        bottomslope = -100*FRACUNIT/160;
    }
    shootz = lastz = t1->z + (t1->height>>1) + 8*FRACUNIT;

    // can't shoot outside view angles


    attackrange = distance;
    linetarget = NULL;

    //added:15-02-98: comments
    // traverse all linedefs and mobjs from the blockmap containing t1,
    // to the blockmap containing the dest. point.
    // Call the function for each mobj/line on the way,
    // starting with the mobj/linedef at the shortest distance...
    P_PathTraverse ( t1->x, t1->y,
                     x2, y2,
                     PT_ADDLINES|PT_ADDTHINGS,
                     PTR_AimTraverse );

    //added:15-02-98: linetarget is only for mobjs, not for linedefs
    if (linetarget)
        return aimslope;

    return 0;
}


//
// P_LineAttack
// If damage == 0, it is just a test trace
// that will leave linetarget set.
//
//added:16-02-98: Fab comments...
//                t1       est l'attaquant (player ou monstre)
//                angle    est l'angle de tir sur le plan x,y (orientation)
//                distance est la port‚e maximale de la balle
//                slope    est la pente vers la destination (up/down)
//                damage   est les degats infliges par la balle
void P_LineAttack ( mobj_t*       t1,
                    angle_t       angle,
                    fixed_t       distance,
                    fixed_t       slope,
                    int           damage )
{
    fixed_t     x2;
    fixed_t     y2;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    la_damage = damage;

    // player autoaimed attack, 
    if(!t1->player)
    {   
        x2 = t1->x + (distance>>FRACBITS)*finecosine[angle]; 
        y2 = t1->y + (distance>>FRACBITS)*finesine[angle];   
    }
    else
    {
        fixed_t cosangle=finecosine[t1->player->aiming>>ANGLETOFINESHIFT];

        x2 = t1->x + FixedMul(FixedMul(distance,finecosine[angle]),cosangle);
        y2 = t1->y + FixedMul(FixedMul(distance,finesine[angle]),cosangle); 
    }

    shootz = lastz = t1->z + (t1->height>>1) + 8*FRACUNIT;

    attackrange = distance;
    aimslope = slope;

    tmthing = shootthing;

    P_PathTraverse ( t1->x, t1->y,
                     x2, y2,
                     PT_ADDLINES|PT_ADDTHINGS,
                     PTR_ShootTraverse );
}

//
// USE LINES
//
mobj_t*         usething;

boolean PTR_UseTraverse (intercept_t* in)
{
    int         side;

    tmthing = NULL;
    if (!in->d.line->special)
    {
        P_LineOpening (in->d.line);
        if (openrange <= 0)
        {
// Don't make sound! Tails 03-11-2002
//            S_StartSound (usething, sfx_pop);

            // can't use through a wall
            return false;
        }
        // not a special line, but keep checking
        return true ;
    }

    side = 0;
    if (P_PointOnLineSide (usething->x, usething->y, in->d.line) == 1)
        side = 1;

    //  return false;           // don't use back side
//    P_UseSpecialLine (usething, in->d.line, side);

    // can't use for than one special line in a row
    // SoM: USE MORE THAN ONE!
    if(boomsupport && (in->d.line->flags&ML_PASSUSE))
      return true;
    else
      return false;
}


//
// P_UseLines
// Looks for special lines in front of the player to activate.
//
void P_UseLines (player_t*      player)
{
    int         angle;
    fixed_t     x1;
    fixed_t     y1;
    fixed_t     x2;
    fixed_t     y2;

    usething = player->mo;

    angle = player->mo->angle >> ANGLETOFINESHIFT;

    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE>>FRACBITS)*finecosine[angle];
    y2 = y1 + (USERANGE>>FRACBITS)*finesine[angle];

    P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse );
}


//
// RADIUS ATTACK
//
mobj_t*         bombsource;
mobj_t*         bombspot;
int             bombdamage;


//
// PIT_RadiusAttack
// "bombsource" is the creature
// that caused the explosion at "bombspot".
//
boolean PIT_RadiusAttack (mobj_t* thing)
{
    fixed_t     dx;
    fixed_t     dy;
    fixed_t     dz;
    fixed_t     dist;

    if (!(thing->flags & MF_SHOOTABLE) )
        return true;

	if(thing->flags & MF_BOSS)
		return true;

    // Boss spider and cyborg
    // take no damage from concussion.
		switch(thing->type) // Tails 08-18-2001
		{
			case MT_SKIM:
			case MT_JETTBOMBER: // Jetty-Syn Bomber
				return true;
				break;
			default:
				if(thing->flags & MF_MONITOR)
					return true;
				break;
		}

    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);

    dist = dx>dy ? dx : dy;
    dist -= thing->radius;

    //added:22-02-98: now checks also z dist for rockets exploding
    //                above yer head...
    dz = abs(thing->z+(thing->height>>1) - bombspot->z);
    dist = dist > dz ? dist : dz;

    dist >>= FRACBITS;

    if (dist < 0)
        dist = 0;

    if (dist >= bombdamage)
        return true;    // out of range

    if (thing->floorz > bombspot->z && bombspot->ceilingz < thing->z)
        return true;

    if (thing->ceilingz < bombspot->z && bombspot->floorz > thing->z)
        return true;

    if ( P_CheckSight (thing, bombspot) )
    {
        int  damage=bombdamage - dist;
        int  momx=0,momy=0;
        if( dist )
        {
            momx = (thing->x - bombspot->x)/dist;
            momy = (thing->y - bombspot->y)/dist;
        }
        // must be in direct path
		P_DamageMobj (thing, bombspot, bombsource, damage); // Tails 01-11-2001
//        if( P_DamageMobj (thing, bombspot, bombsource, damage) && (thing->flags & MF_NOBLOOD)==0)
//            P_SpawnBloodSplats (thing->x,thing->y,thing->z, damage, momx, momy);
    }

    return true;
}


//
// P_RadiusAttack
// Source is the creature that caused the explosion at spot.
//
void P_RadiusAttack ( mobj_t*       spot,
                      mobj_t*       source,
                      int           damage )
{
    int         x;
    int         y;

    int         xl;
    int         xh;
    int         yl;
    int         yh;

    fixed_t     dist;

    dist = (damage+MAXRADIUS)<<FRACBITS;
    yh = (spot->y + dist - bmaporgy)>>MAPBLOCKSHIFT;
    yl = (spot->y - dist - bmaporgy)>>MAPBLOCKSHIFT;
    xh = (spot->x + dist - bmaporgx)>>MAPBLOCKSHIFT;
    xl = (spot->x - dist - bmaporgx)>>MAPBLOCKSHIFT;
    bombspot = spot;

        bombsource = source;
    bombdamage = damage;

    for (y=yl ; y<=yh ; y++)
        for (x=xl ; x<=xh ; x++)
            P_BlockThingsIterator (x, y, PIT_RadiusAttack );
}



//
// SECTOR HEIGHT CHANGING
// After modifying a sectors floor or ceiling height,
// call this routine to adjust the positions
// of all things that touch the sector.
//
// If anything doesn't fit anymore, true will be returned.
// If crunch is true, they will take damage
//  as they are being crushed.
// If Crunch is false, you should set the sector height back
//  the way it was and call P_ChangeSector again
//  to undo the changes.
//
boolean         crushchange;
boolean         nofit;
sector_t        *sectorchecked;

//
// PIT_ChangeSector
//
boolean PIT_ChangeSector (mobj_t*       thing)
{
//    mobj_t*     mo;

    if (P_ThingHeightClip (thing))
    {
        // keep checking
        return true;
    }
/*
    // crunch bodies to giblets
    if (thing->flags & MF_CORPSE)
    {
        P_SetMobjState (thing, S_GIBS);
        thing->flags &= ~MF_SOLID;
            //added:22-02-98: lets have a neat 'crunch' sound!
//            S_StartSound (thing, sfx_slop);
        thing->skin = 0;

        thing->height = 0;
        thing->radius = 0;
        thing->skin = 0;

        // keep checking
        return true;
    }*/
/*
    // crunch dropped items
    if (thing->flags & MF_DROPPED)
    {
        P_RemoveMobj (thing);

        // keep checking
        return true;
    }
*/
    if (! (thing->flags & MF_SHOOTABLE) )
    {
        // assume it is bloody gibs or something
        return true;
    }

    nofit = true;

	// Testing 3dfloor crunches.. Tails 06-10-2002
	if(thing->z + thing->height > thing->ceilingz && thing->z <= thing->ceilingz)
	{
		boolean isthisit = false;

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
			if(*rover->bottomheight <= thing->ceilingz && abs(delta1) >= abs(delta2))
			{
				thinker_t* think;
				elevator_t* crumbler;

				for(think = thinkercap.next; think != &thinkercap; think = think->next)
				{
					if(think->function.acp1 != (actionf_p1)T_StartCrumble)
						continue;

					crumbler = (elevator_t *)think;

					if(!(crumbler->type == elevateBounce || crumbler->type == elevateContinuous))
						continue;

					if(crumbler->actionsector != thing->subsector->sector)
						continue;

					if(crumbler->sector != rover->master->frontsector)
						continue;

					if(!crumbler->player)
						continue;


					// Hey, gotta check all the pointers.. what can I say?
					if(!crumbler->player->mo)
						continue;

					P_DamageMobj(thing, crumbler->player->mo, crumbler->player->mo, 10000);
					return true;
				}
			}
		  }
		}
		
		if(isthisit == false)
		{
			// Instant-death
			P_DamageMobj(thing, NULL, NULL, 10000);
		}
	}

    if (crushchange && !(leveltime % (4*NEWTICRATERATIO)) )
    {
        P_DamageMobj(thing,NULL,NULL,10);
/*
        if((!(leveltime % (16*NEWTICRATERATIO)) && !(thing->flags&MF_NOBLOOD)) )
        {
            // spray blood in a random direction
            mo = P_SpawnMobj (thing->x,
                              thing->y,
                              thing->z + thing->height/2, MT_BLOOD);
            
            mo->momx  = P_SignedRandom()<<12;
            mo->momy  = P_SignedRandom()<<12;
        }*/
    }

    // keep checking (crush other things)
    return true;
}



//
// P_ChangeSector
//
boolean P_ChangeSector ( sector_t*     sector,
                         boolean       crunch )
{
    int         x;
    int         y;

    nofit = false;
    crushchange = crunch;
    sectorchecked = sector;

    // re-check heights for all things near the moving sector
    for (x=sector->blockbox[BOXLEFT] ; x<= sector->blockbox[BOXRIGHT] ; x++)
        for (y=sector->blockbox[BOXBOTTOM];y<= sector->blockbox[BOXTOP] ; y++)
            P_BlockThingsIterator (x, y, PIT_ChangeSector);


    return nofit;
}


//SoM: 3/15/2000: New function. Much faster.
boolean P_CheckSector(sector_t* sector, boolean crunch)
{
  msecnode_t      *n;

  if (!boomsupport) // use the old routine for old demos though
    return P_ChangeSector(sector,crunch);

  nofit = false;
  crushchange = crunch;


  // killough 4/4/98: scan list front-to-back until empty or exhausted,
  // restarting from beginning after each thing is processed. Avoids
  // crashes, and is sure to examine all things in the sector, and only
  // the things which are in the sector, until a steady-state is reached.
  // Things can arbitrarily be inserted and removed and it won't mess up.
  //
  // killough 4/7/98: simplified to avoid using complicated counter

  if(sector->numattached)
  {
    int            i;
    sector_t*      sec;
    for(i = 0; i < sector->numattached; i ++)
    {
      sec = &sectors[sector->attached[i]];
      for (n=sec->touching_thinglist; n; n=n->m_snext)
        n->visited = false;

      sec->moved = true;

      do {
      for (n=sec->touching_thinglist; n; n=n->m_snext)
        if (!n->visited)
          {
          n->visited  = true;
          if (!(n->m_thing->flags & MF_NOBLOCKMAP))
            PIT_ChangeSector(n->m_thing);
          break;
          }
      } while (n);
    }
  }
  // Mark all things invalid
  sector->moved = true;

  for (n=sector->touching_thinglist; n; n=n->m_snext)
      n->visited = false;
  
  do {
      for (n=sector->touching_thinglist; n; n=n->m_snext)  // go through list
          if (!n->visited)               // unprocessed thing found
          {
              n->visited  = true;          // mark thing as processed
              if (!(n->m_thing->flags & MF_NOBLOCKMAP)) //jff 4/7/98 don't do these
                  PIT_ChangeSector(n->m_thing);    // process it
              break;                 // exit and start over
          }
  } while (n);  // repeat from scratch until all things left are marked valid
  
  return nofit;
}




/*
  SoM: 3/15/2000
  Lots of new Boom functions that work faster and add functionality.
*/

static msecnode_t* headsecnode = NULL;

static mprecipsecnode_t* headprecipsecnode = NULL;

void P_Initsecnode( void )
{
    headsecnode = NULL;
	headprecipsecnode = NULL;
}

// P_GetSecnode() retrieves a node from the freelist. The calling routine
// should make sure it sets all fields properly.

msecnode_t* P_GetSecnode()
{
  msecnode_t* node;

  if (headsecnode)
    {
    node = headsecnode;
    headsecnode = headsecnode->m_snext;
    }
  else
    node = Z_Malloc (sizeof(*node), PU_LEVEL, NULL);
  return(node);
}

mprecipsecnode_t* P_GetPrecipSecnode()
{
  mprecipsecnode_t* node;

  if (headprecipsecnode)
    {
    node = headprecipsecnode;
    headprecipsecnode = headprecipsecnode->m_snext;
    }
  else
    node = Z_Malloc (sizeof(*node), PU_LEVEL, NULL);
  return(node);
}

// P_PutSecnode() returns a node to the freelist.

void P_PutSecnode(msecnode_t* node)
{
    node->m_snext = headsecnode;
    headsecnode = node;
}

// Tails 08-25-2002
void P_PutPrecipSecnode(mprecipsecnode_t* node)
{
    node->m_snext = headprecipsecnode;
    headprecipsecnode = node;
}

// P_AddSecnode() searches the current list to see if this sector is
// already there. If not, it adds a sector node at the head of the list of
// sectors this object appears in. This is called when creating a list of
// nodes that will get linked in later. Returns a pointer to the new node.

msecnode_t* P_AddSecnode(sector_t* s, mobj_t* thing, msecnode_t* nextnode)
{
  msecnode_t* node;

  node = nextnode;
  while (node)
    {
    if (node->m_sector == s)   // Already have a node for this sector?
      {
      node->m_thing = thing; // Yes. Setting m_thing says 'keep it'.
      return(nextnode);
      }
    node = node->m_tnext;
    }

  // Couldn't find an existing node for this sector. Add one at the head
  // of the list.

  node = P_GetSecnode();

  //mark new nodes unvisited.
  node->visited = 0;

  node->m_sector = s;       // sector
  node->m_thing  = thing;     // mobj
  node->m_tprev  = NULL;    // prev node on Thing thread
  node->m_tnext  = nextnode;  // next node on Thing thread
  if (nextnode)
    nextnode->m_tprev = node; // set back link on Thing

  // Add new node at head of sector thread starting at s->touching_thinglist

  node->m_sprev  = NULL;    // prev node on sector thread
  node->m_snext  = s->touching_thinglist; // next node on sector thread
  if (s->touching_thinglist)
    node->m_snext->m_sprev = node;
  s->touching_thinglist = node;
  return(node);
}

// More crazy crap Tails 08-25-2002
mprecipsecnode_t* P_AddPrecipSecnode(sector_t* s, precipmobj_t* thing, mprecipsecnode_t* nextnode)
{
  mprecipsecnode_t* node;

  node = nextnode;
  while (node)
    {
    if (node->m_sector == s)   // Already have a node for this sector?
      {
      node->m_thing = thing; // Yes. Setting m_thing says 'keep it'.
      return(nextnode);
      }
    node = node->m_tnext;
    }

  // Couldn't find an existing node for this sector. Add one at the head
  // of the list.

  node = P_GetPrecipSecnode();

  //mark new nodes unvisited.
  node->visited = 0;

  node->m_sector = s;       // sector
  node->m_thing  = thing;     // mobj
  node->m_tprev  = NULL;    // prev node on Thing thread
  node->m_tnext  = nextnode;  // next node on Thing thread
  if (nextnode)
    nextnode->m_tprev = node; // set back link on Thing

  // Add new node at head of sector thread starting at s->touching_thinglist

  node->m_sprev  = NULL;    // prev node on sector thread
  node->m_snext  = s->touching_preciplist; // next node on sector thread
  if (s->touching_preciplist)
    node->m_snext->m_sprev = node;
  s->touching_preciplist = node;
  return(node);
}


// P_DelSecnode() deletes a sector node from the list of
// sectors this object appears in. Returns a pointer to the next node
// on the linked list, or NULL.

msecnode_t* P_DelSecnode(msecnode_t* node)
{
  msecnode_t* tp;  // prev node on thing thread
  msecnode_t* tn;  // next node on thing thread
  msecnode_t* sp;  // prev node on sector thread
  msecnode_t* sn;  // next node on sector thread

  if (node)
    {

    // Unlink from the Thing thread. The Thing thread begins at
    // sector_list and not from mobj_t->touching_sectorlist.

    tp = node->m_tprev;
    tn = node->m_tnext;
    if (tp)
      tp->m_tnext = tn;
    if (tn)
      tn->m_tprev = tp;

    // Unlink from the sector thread. This thread begins at
    // sector_t->touching_thinglist.

    sp = node->m_sprev;
    sn = node->m_snext;
    if (sp)
      sp->m_snext = sn;
    else
      node->m_sector->touching_thinglist = sn;
    if (sn)
      sn->m_sprev = sp;

    // Return this node to the freelist

    P_PutSecnode(node);
    return(tn);
    }
  return(NULL);
}

// Tails 08-25-2002
mprecipsecnode_t* P_DelPrecipSecnode(mprecipsecnode_t* node)
{
  mprecipsecnode_t* tp;  // prev node on thing thread
  mprecipsecnode_t* tn;  // next node on thing thread
  mprecipsecnode_t* sp;  // prev node on sector thread
  mprecipsecnode_t* sn;  // next node on sector thread

  if (node)
    {

    // Unlink from the Thing thread. The Thing thread begins at
    // sector_list and not from mobj_t->touching_sectorlist.

    tp = node->m_tprev;
    tn = node->m_tnext;
    if (tp)
      tp->m_tnext = tn;
    if (tn)
      tn->m_tprev = tp;

    // Unlink from the sector thread. This thread begins at
    // sector_t->touching_thinglist.

    sp = node->m_sprev;
    sn = node->m_snext;
    if (sp)
      sp->m_snext = sn;
    else
      node->m_sector->touching_preciplist = sn;
    if (sn)
      sn->m_sprev = sp;

    // Return this node to the freelist

    P_PutPrecipSecnode(node);
    return(tn);
    }
  return(NULL);
}

// Delete an entire sector list

void P_DelSeclist(msecnode_t* node)

{
    while (node)
        node = P_DelSecnode(node);
}

// Tails 08-25-2002
void P_DelPrecipSeclist(mprecipsecnode_t* node)
{
    while (node)
        node = P_DelPrecipSecnode(node);
}


// PIT_GetSectors
// Locates all the sectors the object is in by looking at the lines that
// cross through it. You have already decided that the object is allowed
// at this location, so don't bother with checking impassable or
// blocking lines.

boolean PIT_GetSectors(line_t* ld)
{
  if (tmbbox[BOXRIGHT]  <= ld->bbox[BOXLEFT]   ||
      tmbbox[BOXLEFT]   >= ld->bbox[BOXRIGHT]  ||
      tmbbox[BOXTOP]    <= ld->bbox[BOXBOTTOM] ||
      tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
    return true;

  if (P_BoxOnLineSide(tmbbox, ld) != -1)
    return true;

  // This line crosses through the object.

  // Collect the sector(s) from the line and add to the
  // sector_list you're examining. If the Thing ends up being
  // allowed to move to this position, then the sector_list
  // will be attached to the Thing's mobj_t at touching_sectorlist.

  sector_list = P_AddSecnode(ld->frontsector,tmthing,sector_list);

  // Don't assume all lines are 2-sided, since some Things
  // like MT_TFOG are allowed regardless of whether their radius takes
  // them beyond an impassable linedef.

  // Use sidedefs instead of 2s flag to determine two-sidedness.

  if (ld->backsector)
    sector_list = P_AddSecnode(ld->backsector, tmthing, sector_list);

  return true;
}

// Tails 08-25-2002
boolean PIT_GetPrecipSectors(line_t* ld)
{
  if (tmbbox[BOXRIGHT]  <= ld->bbox[BOXLEFT]   ||
      tmbbox[BOXLEFT]   >= ld->bbox[BOXRIGHT]  ||
      tmbbox[BOXTOP]    <= ld->bbox[BOXBOTTOM] ||
      tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
    return true;

  if (P_BoxOnLineSide(tmbbox, ld) != -1)
    return true;

  // This line crosses through the object.

  // Collect the sector(s) from the line and add to the
  // sector_list you're examining. If the Thing ends up being
  // allowed to move to this position, then the sector_list
  // will be attached to the Thing's mobj_t at touching_sectorlist.

  precipsector_list = P_AddPrecipSecnode(ld->frontsector,tmprecipthing,precipsector_list);

  // Don't assume all lines are 2-sided, since some Things
  // like MT_TFOG are allowed regardless of whether their radius takes
  // them beyond an impassable linedef.

  // Use sidedefs instead of 2s flag to determine two-sidedness.

  if (ld->backsector)
    precipsector_list = P_AddPrecipSecnode(ld->backsector, tmprecipthing, precipsector_list);

  return true;
}


// P_CreateSecNodeList alters/creates the sector_list that shows what sectors
// the object resides in.

void P_CreateSecNodeList(mobj_t* thing,fixed_t x,fixed_t y)
{
  int xl;
  int xh;
  int yl;
  int yh;
  int bx;
  int by;
  msecnode_t* node;

  // First, clear out the existing m_thing fields. As each node is
  // added or verified as needed, m_thing will be set properly. When
  // finished, delete all nodes where m_thing is still NULL. These
  // represent the sectors the Thing has vacated.

  node = sector_list;
  while (node)
    {
    node->m_thing = NULL;
    node = node->m_tnext;
    }

  tmthing = thing;
  tmflags = thing->flags;

  tmx = x;
  tmy = y;

  tmbbox[BOXTOP]  = y + tmthing->radius;
  tmbbox[BOXBOTTOM] = y - tmthing->radius;
  tmbbox[BOXRIGHT]  = x + tmthing->radius;
  tmbbox[BOXLEFT]   = x - tmthing->radius;

  validcount++; // used to make sure we only process a line once

  xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
  xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
  yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
  yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      P_BlockLinesIterator(bx,by,PIT_GetSectors);

  // Add the sector of the (x,y) point to sector_list.

  sector_list = P_AddSecnode(thing->subsector->sector,thing,sector_list);

  // Now delete any nodes that won't be used. These are the ones where
  // m_thing is still NULL.

  node = sector_list;
  while (node)
    {
    if (node->m_thing == NULL)
      {
      if (node == sector_list)
        sector_list = node->m_tnext;
      node = P_DelSecnode(node);
      }
    else
      node = node->m_tnext;
    }
}

// More crazy crap Tails 08-25-2002
void P_CreatePrecipSecNodeList(precipmobj_t* thing,fixed_t x,fixed_t y)
{
  int xl;
  int xh;
  int yl;
  int yh;
  int bx;
  int by;
  mprecipsecnode_t* node;

  // First, clear out the existing m_thing fields. As each node is
  // added or verified as needed, m_thing will be set properly. When
  // finished, delete all nodes where m_thing is still NULL. These
  // represent the sectors the Thing has vacated.

  node = precipsector_list;
  while (node)
    {
    node->m_thing = NULL;
    node = node->m_tnext;
    }

  tmprecipthing = thing;
  preciptmflags = thing->flags;

  preciptmx = x;
  preciptmy = y;

  // Precipitation has a fixed radius of 2*FRACUNIT Tails 08-28-2002
  preciptmbbox[BOXTOP]  = y + 2*FRACUNIT;
  preciptmbbox[BOXBOTTOM] = y - 2*FRACUNIT;
  preciptmbbox[BOXRIGHT]  = x + 2*FRACUNIT;
  preciptmbbox[BOXLEFT]   = x - 2*FRACUNIT;

  validcount++; // used to make sure we only process a line once

  xl = (preciptmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
  xh = (preciptmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
  yl = (preciptmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
  yh = (preciptmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

  for (bx=xl ; bx<=xh ; bx++)
    for (by=yl ; by<=yh ; by++)
      P_BlockLinesIterator(bx,by,PIT_GetPrecipSectors);

  // Add the sector of the (x,y) point to sector_list.

  precipsector_list = P_AddPrecipSecnode(thing->subsector->sector,thing,precipsector_list);

  // Now delete any nodes that won't be used. These are the ones where
  // m_thing is still NULL.

  node = precipsector_list;
  while (node)
    {
    if (node->m_thing == NULL)
      {
      if (node == precipsector_list)
        precipsector_list = node->m_tnext;
      node = P_DelPrecipSecnode(node);
      }
    else
      node = node->m_tnext;
    }
}

// heretic code

//---------------------------------------------------------------------------
//
// PIT_CheckOnmobjZ
//
//---------------------------------------------------------------------------
mobj_t *onmobj; //generic global onmobj...used for landing on pods/players

static boolean PIT_CheckOnmobjZ(mobj_t *thing)
{
//    fixed_t blockdist;

	return false; // Ignore this function for now Tails 02-01-2002
/*
    if(!(thing->flags&(MF_SOLID|MF_SPECIAL|MF_SHOOTABLE)))
    { // Can't hit thing
        return(true);
    }
    blockdist = thing->radius+tmthing->radius;
    if(abs(thing->x-tmx) >= blockdist || abs(thing->y-tmy) >= blockdist)
    { // Didn't hit thing
        return(true);
    }
    if(thing == tmthing)
    { // Don't clip against self
        return(true);
    }
    if(tmthing->z > thing->z+thing->height)
    {
        return(true);
    }
    else if(tmthing->z+tmthing->height < thing->z)
    { // under thing
        return(true);
    }
    if(thing->flags&MF_SOLID)
    {
        onmobj = thing;
    }
    return(!(thing->flags&MF_SOLID));*/
}

// P_FloorzAtPos
// Returns the floorz of the XYZ position
// Tails 05-26-2003
fixed_t P_FloorzAtPos(fixed_t x, fixed_t y, fixed_t z, fixed_t height)
{
	sector_t* sec;
	fixed_t floorz = -1;

	sec = R_PointInSubsector(x, y)->sector;
#ifdef FIXROVERBUGS
// Intercept the stupid 'fall through 3dfloors' bug Tails 03-17-2002
    if(sec->ffloors)
    {
      ffloor_t*  rover;
      fixed_t    delta1;
      fixed_t    delta2;
      int        thingtop = z + height;

      for(rover = sec->ffloors; rover; rover = rover->next)
      {
		if(!(rover->flags & FF_EXISTS))
			continue;

        if((!(rover->flags & FF_SOLID || rover->flags & FF_QUICKSAND) || (rover->flags & FF_SWIMMABLE)))
			continue;

		if(rover->flags & FF_QUICKSAND)
		{
			if(z < *rover->topheight && *rover->bottomheight < thingtop)
			{
				floorz = z;
				continue;
			}
		}

        delta1 = z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > floorz && abs(delta1) < abs(delta2))
			floorz = *rover->topheight;
      }
    }
#endif

	return floorz;
}

//=============================================================================
//
// P_FakeZMovement
//
//              Fake the zmovement so that we can check if a move is legal
//=============================================================================

static void P_FakeZMovement(mobj_t *mo)
{
        int dist;
        int delta;

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

//
// adjust height
//
        mo->z += mo->momz;
        if(mo->flags&MF_FLOAT && mo->target && mo->type != MT_EGGMOBILE
											&& mo->type != MT_EGGMOBILE2
											&& mo->type != MT_RING && mo->type != MT_COIN) // Tails
        {       // float down towards target if too close
                if(!(mo->flags2&MF2_SKULLFLY) && !(mo->flags&MF_INFLOAT))
                {
                        dist = P_AproxDistance(mo->x-mo->target->x, mo->y-mo->target->y);
                        delta =( mo->target->z+(mo->height>>1))-mo->z;
                        if (delta < 0 && dist < -(delta*3))
                                mo->z -= FLOATSPEED;
                        else if (delta > 0 && dist < (delta*3))
                                mo->z += FLOATSPEED;
                }
        }/*
        if(mo->player && mo->flags2&MF2_FLY && !(mo->z <= mo->floorz)
                && leveltime&2)
        {
                mo->z += finesine[(FINEANGLES/20*leveltime>>2)&FINEMASK];
        }
*/
//
// clip movement
//
        if(mo->z <= mo->floorz)
        { // Hit the floor
                mo->z = mo->floorz;
                if(mo->momz < 0)
                {
                        mo->momz = 0;
                }
                if(mo->flags2&MF2_SKULLFLY)
                { // The skull slammed into something
                        mo->momz = -mo->momz;
                }
/*                if(mo->info->crashstate && (mo->flags&MF_CORPSE))
                {
                        return;
                }*/
        }
        else if(mo->flags2&MF2_LOGRAV)
        {
                if(mo->momz == 0)
                        mo->momz = -(gravity>>3)*2;
                else
                        mo->momz -= gravity>>3;
        }
        else if (! (mo->flags & MF_NOGRAVITY) )
        {
                if (mo->momz == 0)
                        mo->momz = -gravity*2;
                else
                        mo->momz -= gravity;
        }

        if (mo->z + mo->height > mo->ceilingz)
        {       // hit the ceiling
                if (mo->momz > 0)
                        mo->momz = 0;
                mo->z = mo->ceilingz - mo->height;
                if (mo->flags2 & MF2_SKULLFLY)
                {       // the skull slammed into something
                        mo->momz = -mo->momz;
                }
        }
}

//=============================================================================
//
// P_CheckOnmobj(mobj_t *thing)
//
//              Checks if the new Z position is legal
//=============================================================================

mobj_t *P_CheckOnmobj(mobj_t *thing)
{
    int                     xl,xh,yl,yh,bx,by;
    subsector_t             *newsubsec;
    fixed_t x;
    fixed_t y;
    mobj_t oldmo;
    
    x = thing->x;
    y = thing->y;
    tmthing = thing;
    tmflags = thing->flags;
    oldmo = *thing; // save the old mobj before the fake zmovement
    P_FakeZMovement(tmthing);
    
    tmx = x;
    tmy = y;
    
    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;
    
    newsubsec = R_PointInSubsector (x,y);
    ceilingline = NULL;
    
    //
    // the base floor / ceiling is from the subsector that contains the
    // point.  Any contacted lines the step closer together will adjust them
    //
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;
    
    validcount++;
    numspechit = 0;
    
    if ( tmflags & MF_NOCLIP )
        return NULL;
    
    //
    // check things first, possibly picking things up
    // the bounding box is extended by MAXRADIUS because mobj_ts are grouped
    // into mapblocks based on their origin point, and can overlap into adjacent
    // blocks by up to MAXRADIUS units
    //
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;
    
    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if (!P_BlockThingsIterator(bx,by,PIT_CheckOnmobjZ))
            {
                *tmthing = oldmo;
                return onmobj;
            }

    *tmthing = oldmo;
    return NULL;
}

//----------------------------------------------------------------------------
//
// FUNC P_TestMobjLocation
//
// Returns true if the mobj is not blocked by anything at its current
// location, otherwise returns false.
//
//----------------------------------------------------------------------------
// DONT USE THIS ON A PLAYER MOBJ!! Tails 12-28-2003
boolean P_TestMobjLocation(mobj_t *mobj)
{
        int flags;

        flags = mobj->flags;
//        mobj->flags &= ~MF_PICKUP;
        if(P_CheckPosition(mobj, mobj->x, mobj->y))
        { // XY is ok, now check Z
                mobj->flags = flags;
                if((mobj->z < mobj->floorz)
                        || (mobj->z+mobj->height > mobj->ceilingz))
                { // Bad Z
                        return(false);
                }
                return(true);
        }
        mobj->flags = flags;
        return(false);
}
