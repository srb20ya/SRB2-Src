// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_map.c,v 1.7 2000/04/23 16:19:52 bpereira Exp $
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


fixed_t         tmbbox[4];
mobj_t*         tmthing;
int             tmflags;
fixed_t         tmx;
fixed_t         tmy;

void P_InstaThrust (); // Proto! Tails 11-01-2000
void P_Thrust (); // Proto! Tails 11-01-2000

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
extern mobj_t*  blockthing; // Tails 9-15-99 Spin Attack

// keep track of special lines as they are hit,
// but don't process them until the move is proven valid
int             *spechit;                //SoM: 3/15/2000: Limit removal
static int      spechit_max = 0;
int             numspechit;

//SoM: 3/15/2000
msecnode_t*  sector_list = NULL;

//SoM: 3/15/2000
static int pe_x; // Pain Elemental position for Lost Soul checks
static int pe_y; // Pain Elemental position for Lost Soul checks
static int ls_x; // Lost Soul position for Lost Soul checks
static int ls_y; // Lost Soul position for Lost Soul checks


//
// TELEPORT MOVE
//

//
// PIT_StompThing
//
boolean PIT_StompThing (mobj_t* thing)
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

    if ( !tmthing->player && gamemap != 30)
        return false;

    P_DamageMobj (thing, tmthing, tmthing, 10000);

    return true;
}


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
    int                 xl;
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

#ifdef FAKEFLOORS
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
        if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS)) continue;

        delta1 = thing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > tmfloorz && abs(delta1) < abs(delta2))
          tmfloorz = tmdropoffz = *rover->topheight;
        if(*rover->bottomheight < tmceilingz && abs(delta1) >= abs(delta2))
          tmceilingz = *rover->bottomheight;
      }
    }
#endif

    // tmfloorthing is set when tmfloorz comes from a thing's top
    tmfloorthing = NULL;

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

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition (thing);

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;

    P_SetThingPosition (thing);

    return true;
}


// =========================================================================
//                       MOVEMENT ITERATOR FUNCTIONS
// =========================================================================

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
        return false;           // one sided line

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
    if (ld->special/* && numspechit<MAXSPECIALCROSS*/)
    {
        //SoM: 3/15/2000: Boom limit removal.
        if (numspechit >= spechit_max)
        {
            spechit_max = spechit_max ? spechit_max*2 : 16;
            spechit = (int *)realloc(spechit,sizeof(int)*spechit_max);
        }
        
        spechit[numspechit] = ld - lines;
        numspechit++;

    }

    return true;
}

//
// PIT_CheckThing
//
boolean PIT_CheckThing (mobj_t* thing)
{
    fixed_t             blockdist;
    boolean             solid;
    int                 damage;

    //added:22-02-98:
    fixed_t             topz;
    fixed_t             tmtopz;

    //SoM: 3/15/2000: Moved to front.

	// Stop getting stuck in Eggman! Tails 07-21-2001
	if (thing->type == MT_EGGMOBILE && (thing->state->nextstate == S_EGGMOBILE_PAIN || thing->state->nextstate == S_EGGMOBILE_PAIN2 || thing->state->nextstate == S_EGGMOBILE_PAIN3 || thing->state->nextstate == S_EGGMOBILE_PAIN4 || thing->state->nextstate == S_EGGMOBILE_PAIN5 || thing->state->nextstate == S_EGGMOBILE_PAIN6 || 
		thing->state->nextstate == S_EGGMOBILE_PAIN7 || thing->state->nextstate == S_EGGMOBILE_PAIN8 || thing->state->nextstate == S_EGGMOBILE_PAIN9 || thing->state->nextstate == S_EGGMOBILE_PAIN10 || thing->state->nextstate == S_EGGMOBILE_PAIN11 || thing->state->nextstate == S_EGGMOBILE_PAIN12)) 
		return true;
	else if (tmthing->type == MT_EGGMOBILE && (tmthing->state->nextstate == S_EGGMOBILE_PAIN || tmthing->state->nextstate == S_EGGMOBILE_PAIN2 || tmthing->state->nextstate == S_EGGMOBILE_PAIN3 || tmthing->state->nextstate == S_EGGMOBILE_PAIN4 || tmthing->state->nextstate == S_EGGMOBILE_PAIN5 || tmthing->state->nextstate == S_EGGMOBILE_PAIN6 || 
		tmthing->state->nextstate == S_EGGMOBILE_PAIN7 || tmthing->state->nextstate == S_EGGMOBILE_PAIN8 || tmthing->state->nextstate == S_EGGMOBILE_PAIN9 || tmthing->state->nextstate == S_EGGMOBILE_PAIN10 || tmthing->state->nextstate == S_EGGMOBILE_PAIN11 || tmthing->state->nextstate == S_EGGMOBILE_PAIN12)) 
		return true;

    // don't clip against self
    if (thing == tmthing)
        return true;

    if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE) ))
        return true;

	if(thing->type == MT_SPARK || tmthing->type == MT_SPARK)
		return true;

    blockdist = thing->radius + tmthing->radius;

    if ( abs(thing->x - tmx) >= blockdist ||
         abs(thing->y - tmy) >= blockdist )
    {
        // didn't hit it
        return true;
    }

    // check for skulls slamming into things
    if (tmflags & MF_SKULLFLY)
    {
//		if(!(thing->player) || (thing->player && !(thing->player->mfjumped)))
//		{
//		damage = ((P_Random()%8)+1)*tmthing->info->damage;

//        P_DamageMobj (thing, tmthing, tmthing, damage);
//		}

		if(tmthing->type == MT_EGGMOBILE) // Don't make Eggman stop!
		{
			tmthing->momx = -tmthing->momx;
			tmthing->momy = -tmthing->momy;
			tmthing->momz = -tmthing->momz;
		}
		else
		{
        tmthing->flags &= ~MF_SKULLFLY;
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
        damage = ((P_Random()%8)+1)*tmthing->info->damage;
		if(tmthing->type == MT_DETON)
			P_DamageMobj (thing, tmthing, tmthing, damage); // New way Tails 12-10-2000
		else
			P_DamageMobj (thing, tmthing, tmthing->target, damage); // New way Tails 12-10-2000
/*
        if( P_DamageMobj (thing, tmthing, tmthing->target, damage) && (thing->flags & MF_NOBLOOD)==0 && demoversion>=129 )
            P_SpawnBloodSplats (tmthing->x,tmthing->y,tmthing->z, damage, thing->momx, thing->momy);
*/

        // don't traverse any more
			return true;
    }

    // missiles can hit other things
    if (tmthing->flags & MF_MISSILE || tmthing->type == MT_SNOWBALL) // Tails 12-12-2001
    {
        // see if it went over / under
        if (tmthing->z > thing->z + thing->height)
            return true;                // overhead
        if (tmthing->z+tmthing->height < thing->z)
            return true;                // underneath

        if (!(tmthing->type == MT_DETON) &&( tmthing->target && (
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
        damage = ((P_Random()%8)+1)*tmthing->info->damage;
		if(tmthing->type == MT_DETON)
			P_DamageMobj (thing, tmthing, tmthing, damage); // New way Tails 12-10-2000
		else
			P_DamageMobj (thing, tmthing, tmthing->target, damage); // New way Tails 12-10-2000
/*
        if( P_DamageMobj (thing, tmthing, tmthing->target, damage) && (thing->flags & MF_NOBLOOD)==0 && demoversion>=129 )
            P_SpawnBloodSplats (tmthing->x,tmthing->y,tmthing->z, damage, thing->momx, thing->momy);
*/

        // don't traverse any more
			return false;
    }

	if(tmthing->type == MT_SPIKEBALL && thing->player)
		P_TouchSpecialThing(tmthing, thing);
	else if(thing->type == MT_SPIKEBALL && tmthing->player)
		P_TouchSpecialThing(thing, tmthing);

    // check for special pickup
    if (thing->flags & MF_SPECIAL)
    {
        solid = thing->flags&MF_SOLID;
        if (tmflags&MF_PICKUP)
        {
            // can remove thing
            P_TouchSpecialThing (thing, tmthing);
        }
        return !solid;
    }

// start collectibles can hit a still player Tails 08-07-2000
    if (tmthing->flags & MF_SPECIAL)
    {
        solid = tmthing->flags&MF_SOLID;
        if (thing->flags&MF_PICKUP)
        {
            // can remove thing
            P_TouchSpecialThing (tmthing, thing);
        }
        return !solid;
    }
// end collectibles can hit a still player Tails 08-07-2000

// Push Gargoyles Tails 10-03-2001
if (tmthing->player && thing->type == MT_GARGOYLE)
{
	if((thing->z <= tmthing->z + tmthing->height) && (thing->z + thing->height >= tmthing->z)
		&& (tmthing->momx || tmthing->momy))
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

		thing->target = tmthing;
	}
}

if(thing->player)
{
	// Objects kill you if it falls from above.
	if(tmthing->z + tmthing->momz <= thing->z + thing->height && thing->z == thing->floorz)
	{
		switch(tmthing->type)
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
				P_DamageMobj(thing, tmthing, tmthing->target, 10000);
				tmthing->momz = -tmthing->momz/2; // Bounce, just for fun!
				// The tmthing->target allows the pusher of the object
				// to get the frag if he topples it on an opponent.
				break;
			default:
				break;
		}
	}

	if((thing->z <= tmthing->z + tmthing->height) && (thing->z + thing->height >= tmthing->z))
	{
		if(thing->player->mfspinning || thing->player->mfjumped || thing->player->powers[pw_invulnerability] || thing->player->powers[pw_super])
		{
		   switch(tmthing->type)
		   {
				case MT_BLUECRAWLA: // Blue Crawla
				case MT_REDCRAWLA: // Red Crawla
				case MT_EGGMOBILE: // Eggmobile 1
				case MT_JETTBOMBER: // Jetty-Syn Bomber
				case MT_JETTGUNNER: // Jetty-Syn Gunner
					P_DamageMobj(tmthing, thing, thing, 1);
					break;
				default:
					break;
		   }
	   }
		else
		{
		   switch(tmthing->type)
		   {
				case MT_BLUECRAWLA: // Blue Crawla
				case MT_REDCRAWLA: // Red Crawla
				case MT_EGGMOBILE: // Eggmobile 1
				case MT_JETTBOMBER: // Jetty-Syn Bomber
				case MT_JETTGUNNER: // Jetty-Syn Gunner
					P_DamageMobj(thing, tmthing, tmthing, 1);
					break;
				default:
					break;
			}
		}
	}
}

// Start some Tag Mode stuff Tails 05-08-2001
if(thing->player && tmthing->player && (((thing->z <= tmthing->z + tmthing->height) && (thing->z + thing->height >= tmthing->z)) || (tmthing->z == thing->z + thing->height + FRACUNIT)))
{
	if(thing->player->tagit < 298*TICRATE && thing->player->tagit > 0 && !(tmthing->player->powers[pw_invisibility] || tmthing->player->tagzone || tmthing->player->powers[pw_invulnerability]))
	{
		P_DamageMobj(tmthing, thing, thing, 1); // Don't allow tag-backs
	}
	else if (tmthing->player->tagit < 298*TICRATE && tmthing->player->tagit > 0 && !(thing->player->powers[pw_invisibility] || thing->player->tagzone || thing->player->powers[pw_invulnerability]))
	{
		P_DamageMobj(thing, tmthing, tmthing, 1); // Don't allow tag-backs
	}
}
// End some Tag Mode stuff Tails 05-08-2001

// Start snazzy new collision code! Tails 10-30-2000

	if(thing->player && thing->z >= tmthing->z) // Stuff where da player don't gotta move Tails 05-29-2001
	{
		switch(tmthing->type)
			{
				case MT_MISC34: // fan
					if(thing->z <= (thing->subsector->sector->ceilingheight - .25*(thing->subsector->sector->ceilingheight - thing->subsector->sector->floorheight)))
					{
						thing->momz = 5*FRACUNIT;
						thing->player->mfspinning = 0;
						thing->player->mfjumped = 0;
						thing->player->gliding = 0;
						thing->player->glidetime = 0;
						thing->player->climbing = 0;
						if(!(thing->state == &states[S_PLAY_FALL1] || thing->state == &states[S_PLAY_FALL2]))
							P_SetMobjState (thing, S_PLAY_FALL1);
					}
					break;
				case MT_STEAM: // Steam, duh! Can't you read? Tails 05-28-2001
					if(tmthing->state == &states[S_STEAM1] && thing->z <= tmthing->z + 16*FRACUNIT) // Only when it bursts
					{
						thing->momz = 20*FRACUNIT;
						thing->player->mfspinning = 0;
						thing->player->mfjumped = 0;
						thing->player->gliding = 0;
						thing->player->glidetime = 0;
						thing->player->climbing = 0;
						if(!(thing->state == &states[S_PLAY_FALL1] || thing->state == &states[S_PLAY_FALL2]))
							P_SetMobjState (thing, S_PLAY_FALL1);
					}
					break;
			default:
				break;
			}
	}

if(tmthing->player) // Is the moving/interacting object the player?
	{
	if(tmthing->z >= thing->z)
	{
		switch(thing->type)
			{
			case MT_MISC34: // fan
				if(tmthing->z <= (tmthing->subsector->sector->ceilingheight - .25*(tmthing->subsector->sector->ceilingheight - tmthing->subsector->sector->floorheight)))
				{
				tmthing->momz = 5*FRACUNIT;
                tmthing->player->mfspinning = 0;
                tmthing->player->mfjumped = 0;
                tmthing->player->gliding = 0;
                tmthing->player->glidetime = 0;
                tmthing->player->climbing = 0;
				if(!(tmthing->state == &states[S_PLAY_FALL1] || tmthing->state == &states[S_PLAY_FALL2]))
					P_SetMobjState (tmthing, S_PLAY_FALL1);
				}
				break;
			case MT_STEAM: // Steam, duh! Can't you read? Tails 05-28-2001
				if(thing->state == &states[S_STEAM1] && tmthing->z <= thing->z + 16*FRACUNIT) // Only when it bursts
				{
				tmthing->momz = 20*FRACUNIT;
                tmthing->player->mfspinning = 0;
                tmthing->player->mfjumped = 0;
                tmthing->player->gliding = 0;
                tmthing->player->glidetime = 0;
                tmthing->player->climbing = 0;
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
			switch(thing->type)
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
					if(((tmthing->player->mfjumped == 0) && (tmthing->player->mfspinning == 0)) && ((tmthing->player->powers[pw_super]) || (tmthing->player->powers[pw_invulnerability]))) // Don't bust boxes like Sonic Adventure Tails 11-02-2000
						break;
					else
					{
					if(tmthing->momz < 0)
					tmthing->momz = -tmthing->momz;
					P_DamageMobj(thing, tmthing, tmthing, 1);
					}
					break;
				case MT_YELLOWDIAG: // Yellow diagonal spring (pointing up from ground)
                   tmthing->player->mo->momx = tmthing->player->mo->momy = tmthing->player->mo->momz = 0;
				   P_UnsetThingPosition (tmthing);
				   tmthing->player->mo->x = thing->x;
				   tmthing->player->mo->y = thing->y;
				   tmthing->player->mo->z = thing->z + thing->height + 1;
				   P_SetThingPosition (tmthing);
                   tmthing->player->mo->momz = JUMPGRAVITY*3;
                   P_InstaThrust(tmthing, thing->angle, 30*FRACUNIT);
				   if(!(tmthing->player->cmd.forwardmove || tmthing->player->cmd.sidemove))
				   {
						tmthing->player->mo->angle = thing->angle;
				   if (tmthing->player==&players[consoleplayer])
						localangle = thing->angle;
				   }
                    tmthing->player->mfspinning = 0;
                    tmthing->player->mfjumped = 0;
                    tmthing->player->gliding = 0;
                    tmthing->player->glidetime = 0;
                    tmthing->player->climbing = 0;
                   P_SetMobjState (thing, S_YDIAG2);
//                if(tmthing->player->powers[pw_super])
//                   P_SetMobjState (tmthing->player->mo, S_PLAY_SPC1);
//                else
                   P_SetMobjState (tmthing->player->mo, S_PLAY_PLG1);
                   break;
                 case MT_MISC70: // Yellow vertical spring (pointing up)
				   tmthing->player->mo->z++;
                   tmthing->player->mo->momz = JUMPGRAVITY*3;
                    tmthing->player->mfspinning = 0;
                    tmthing->player->mfjumped = 0;
                    tmthing->player->gliding = 0;
                    tmthing->player->glidetime = 0;
                    tmthing->player->climbing = 0;
                   P_SetMobjState (thing, S_HEADSONSTICK2);
//                if(tmthing->player->powers[pw_super])
//                   P_SetMobjState (tmthing->player->mo, S_PLAY_SPC1);
//                else
                   P_SetMobjState (tmthing->player->mo, S_PLAY_PLG1);
                   break;
                 case MT_MISC84: // Red vertical spring (pointing up)
				   tmthing->player->mo->z++;
                   tmthing->player->mo->momz = JUMPGRAVITY*5;
                    tmthing->player->mfspinning = 0;
                    tmthing->player->mfjumped = 0;
                    tmthing->player->gliding = 0;
                    tmthing->player->glidetime = 0;
                    tmthing->player->climbing = 0;
                   P_SetMobjState (thing, S_COLONGIBS2);
//                if(tmthing->player->powers[pw_super])
//                   P_SetMobjState (tmthing->player->mo, S_PLAY_SPC1);
//                else
                   P_SetMobjState (tmthing->player->mo, S_PLAY_PLG1);
                   break;
				 case MT_SPEEDPAD: // Speed Pad
                   tmthing->player->mo->momx = 0;
                   tmthing->player->mo->momy = 0;
				   P_UnsetThingPosition (tmthing);
				   tmthing->player->mo->x = thing->x;
				   tmthing->player->mo->y = thing->y;
				   tmthing->player->mo->z = thing->z;
				   P_SetThingPosition (tmthing);
                   P_InstaThrust(tmthing, thing->angle, 60*FRACUNIT);
				   break;
                 case MT_EGGMOBILE: // Boss 1
					if(tmthing->momz < 0)
					tmthing->momz = -tmthing->momz;
					tmthing->momx = -tmthing->momx;
					tmthing->momy = -tmthing->momy;
                   P_DamageMobj(thing, tmthing, tmthing, 1);
                   break;
				default:
				break;
				}
			}
		else
		{
			switch(thing->type)
			{
                case MT_BLUECRAWLA: // Blue Crawla
                case MT_REDCRAWLA: // Red Crawla
                case MT_SKIM: // Skim
                case MT_GFZFISH: // Jumping Fish
				case MT_EGGMOBILE: // Boss 1
				case MT_JETTBOMBER: // Jetty-Syn Bomber
				case MT_JETTGUNNER: // Jetty-Syn Gunner
					P_DamageMobj(tmthing, thing, thing, 1); // Hurt the player, since they're aren't in an offensive position
					break;
				case MT_YELLOWDIAG: // Yellow diagonal spring (pointing up from ground)
                   tmthing->player->mo->momx = tmthing->player->mo->momy = tmthing->player->mo->momz = 0;
				   P_UnsetThingPosition (tmthing);
				   tmthing->player->mo->x = thing->x;
				   tmthing->player->mo->y = thing->y;
				   tmthing->player->mo->z = thing->z + thing->height + 1;
				   P_SetThingPosition (tmthing);
                   tmthing->player->mo->momz = JUMPGRAVITY*3;
                   P_InstaThrust(tmthing, thing->angle, 30*FRACUNIT);
				   if(!(tmthing->player->cmd.forwardmove || tmthing->player->cmd.sidemove))
				   {
						tmthing->player->mo->angle = thing->angle;
				   if (tmthing->player==&players[consoleplayer])
						localangle = thing->angle;
				   }
                    tmthing->player->mfspinning = 0;
                    tmthing->player->mfjumped = 0;
                    tmthing->player->gliding = 0;
                    tmthing->player->glidetime = 0;
                    tmthing->player->climbing = 0;
                   P_SetMobjState (thing, S_YDIAG2);
//                if(tmthing->player->powers[pw_super])
//                   P_SetMobjState (tmthing->player->mo, S_PLAY_SPC1);
//                else
                   P_SetMobjState (tmthing->player->mo, S_PLAY_PLG1);
                   break;
                 case MT_MISC70: // Yellow vertical spring (pointing up)
				   tmthing->player->mo->z++;
                   tmthing->player->mo->momz = JUMPGRAVITY*3;
                    tmthing->player->mfspinning = 0;
                    tmthing->player->mfjumped = 0;
                    tmthing->player->gliding = 0;
                    tmthing->player->glidetime = 0;
                    tmthing->player->climbing = 0;
                   P_SetMobjState (thing, S_HEADSONSTICK2);
//                if(tmthing->player->powers[pw_super])
//                   P_SetMobjState (tmthing->player->mo, S_PLAY_SPC1);
//                else
                   P_SetMobjState (tmthing->player->mo, S_PLAY_PLG1);
                   break;
                 case MT_MISC84: // Red vertical spring (pointing up)
				   tmthing->player->mo->z++;
                   tmthing->player->mo->momz = JUMPGRAVITY*5;
                    tmthing->player->mfspinning = 0;
                    tmthing->player->mfjumped = 0;
                    tmthing->player->gliding = 0;
                    tmthing->player->glidetime = 0;
                    tmthing->player->climbing = 0;
                   P_SetMobjState (thing, S_COLONGIBS2);
//                if(tmthing->player->powers[pw_super])
//                   P_SetMobjState (tmthing->player->mo, S_PLAY_SPC1);
//                else
                   P_SetMobjState (tmthing->player->mo, S_PLAY_PLG1);
                   break;
				 case MT_SPEEDPAD: // Speed Pad
                   tmthing->player->mo->momx = 0;
                   tmthing->player->mo->momy = 0;
				   P_UnsetThingPosition (tmthing);
				   tmthing->player->mo->x = thing->x;
				   tmthing->player->mo->y = thing->y;
				   tmthing->player->mo->z = thing->z;
				   tmthing->player->mo->angle = ANG45 * (thing->angle/45);
    if (tmthing->player==&players[consoleplayer])
        localangle = thing->angle;
				   P_SetThingPosition (tmthing);
                   P_InstaThrust(tmthing, thing->angle, 60*FRACUNIT);
				   break;
				default:
					break;
			}
		}
		}
	}
	else if (thing->player) // Is the player on the bottom or not moving?
	{
		if(thing->z + thing->height + FRACUNIT == tmthing->z) // Are you touching the bottom of it?
		{
			if((thing->player->mfjumped == 1) || (thing->player->mfspinning == 1) || (thing->player->powers[pw_invulnerability]) || (thing->player->powers[pw_super]) || (thing->player->powers[pw_tailsfly])) // Do you possess the ability to subdue the object?
			{
			switch(tmthing->type)
				{
                case MT_BLUECRAWLA: // Blue Crawla
                case MT_REDCRAWLA: // Red Crawla
                case MT_SKIM: // Skim
                case MT_GFZFISH: // Jumping Fish
				case MT_JETTBOMBER: // Jetty-Syn Bomber
				case MT_JETTGUNNER: // Jetty-Syn Gunner
					P_DamageMobj(tmthing, thing, thing, 1);
				break;
				case MT_EGGMOBILE: // Boss 1
				if(tmthing->momz > 0)
					tmthing->momz = -tmthing->momz;
					tmthing->momx = -tmthing->momx;
					tmthing->momy = -tmthing->momy;
                   P_DamageMobj(tmthing, thing, thing, 1);
                   break;
				default:
				break;
				}
			}
		else
		{
			switch(tmthing->type)
			{
                case MT_BLUECRAWLA: // Blue Crawla
                case MT_REDCRAWLA: // Red Crawla
                case MT_SKIM: // Skim
                case MT_GFZFISH: // Jumping Fish
				case MT_EGGMOBILE: // Boss 1
				case MT_JETTBOMBER: // Jetty-Syn Bomber
				case MT_JETTGUNNER: // Jetty-Syn Gunner
					P_DamageMobj(thing, tmthing, tmthing, 1); // Hurt the player, since they're aren't in an offensive position
					break;
				default:
					break;
			}
		}
		}

	}

    //added:24-02-98:compatibility with old demos, it used to return with...
    //added:27-02-98:for version 112+, nonsolid things pass through other things
    if (demoversion<112 || !(tmthing->flags & MF_SOLID))
        return !(thing->flags & MF_SOLID);

    //added:22-02-98: added z checking at last
    //SoM: 3/10/2000: Treat noclip things as non-solid!
    if (thing->flags & MF_SOLID && tmthing->flags & MF_SOLID)
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
        // if not in air, let P_TryMove() decide if it's not too high
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

#ifdef FAKEFLOORS
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
        if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_EXISTS)) continue;

        delta1 = thing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        delta2 = thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2));
        if(*rover->topheight > tmfloorz && abs(delta1) < abs(delta2))
          tmfloorz = tmdropoffz = *rover->topheight;
        if(*rover->bottomheight < tmceilingz && abs(delta1) >= abs(delta2))
          tmceilingz = *rover->bottomheight;
      }
    }
#endif

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
    int         side;
    int         oldside;
    line_t*     ld;

    floatok = false;

    if (!P_CheckPosition (thing, x, y))
        return false;           // solid wall or thing
#ifdef CLIENTPREDICTION2
    if ( !(thing->flags & MF_NOCLIP) && !(thing->eflags & MF_NOZCHECKING))
#else
    if ( !(thing->flags & MF_NOCLIP) )
#endif
    {
        fixed_t maxstep = MAXSTEPMOVE;
        if (tmceilingz - tmfloorz < thing->height)
            return false;       // doesn't fit

        floatok = true;

        if ( !(thing->flags & MF_TELEPORT) &&
             tmceilingz - thing->z < thing->height)
            return false;       // mobj must lower itself to fit

        // jump out of water
        if((thing->eflags & (MF_UNDERWATER|MF_TOUCHWATER))==(MF_UNDERWATER|MF_TOUCHWATER))
            maxstep=37*FRACUNIT;

        if ( !(thing->flags & MF_TELEPORT) &&
             (tmfloorz - thing->z > maxstep ) )
            return false;       // too big a step up

        if ( !boomsupport || !allowdropoff)
          if ( !(thing->flags&(MF_DROPOFF|MF_FLOAT))
               && tmfloorz - tmdropoffz > MAXSTEPMOVE )
              return false;       // don't stand over a dropoff
    }

    // the move is ok,
    // so link the thing into its new position
    P_UnsetThingPosition (thing);

    //added:28-02-98: gameplay hack : walk over a small wall while jumping
    //                stop jumping it succeeded
    // BP: removed in 1.28 because we can move in air now
    if ( demoplayback>=112 && demoplayback<128 && thing->player &&
         (thing->player->cheats & CF_JUMPOVER) )
    {
        if (tmfloorz > thing->floorz + MAXSTEPMOVE)
            thing->momz >>= 2;
    }

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->x = x;
    thing->y = y;

    //added:28-02-98:
//    if (tmfloorthing)
//        thing->eflags &= ~MF_ONGROUND;  //not on real floor
//    else
        thing->eflags |= MF_ONGROUND;

    P_SetThingPosition (thing);

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
    }
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

    if(thing->type == MT_MISC2 || thing->type == MT_BLUEORB || thing->type == MT_GREENORB || thing->type == MT_BLACKORB || thing->type == MT_YELLOWORB || thing->type == MT_GTHOK || thing->type == MT_GRTHOK || thing->type == MT_PCTHOK || thing->type == MT_DRTHOK || thing->type == MT_STHOK || thing->type == MT_OTHOK || thing->type == MT_RTHOK || thing->type == MT_BTHOK || thing->type == MT_PTHOK || thing->type == MT_DBTHOK || thing->type == MT_BGTHOK) // Tails
      return true; // Ignore these items Tails

	// Have player fall through floor? 10-05-2001 Tails
	if(thing->player && thing->player->playerstate == PST_DEAD)
		return true;

    if (!tmfloorthing && onfloor)
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

        thing->flags &= ~MF_ONGROUND;
    }

    //debug : be sure it falls to the floor
    thing->eflags &= ~MF_ONGROUND;

    //added:28-02-98:
    // test sector bouding top & bottom, not things

    //if (tmsectorceilingz - tmsectorfloorz < thing->height)
    //    return false;

    if (thing->ceilingz - thing->floorz < thing->height
        //imp dans imp map01
        /*&& thing->z >= thing->floorz*/)
    {
        return false;
    }

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

				if(li->flags & ML_SECRET) // Don't cling on this wall Tails 02-02-2002
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
						if(!slidemo->player->climbing)
							slidemo->player->climbing = 5;

						slidemo->player->gliding = 0;
						slidemo->player->glidetime = 0;

						slidemo->angle = R_PointToAngle2(li->v1->x, li->v1->y, li->v2->x, li->v2->y) + ANG90;
						
						if (slidemo->player==&players[consoleplayer])
							localangle = slidemo->angle;
						
						slidemo->player->mfspinning = slidemo->player->mfjumped = 0;
						
						if(slidemo->player->climbing > 1)
							slidemo->momz = slidemo->momx = slidemo->momy = 0;
						
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
						if(!slidemo->player->climbing)
							slidemo->player->climbing = 5;

						slidemo->player->gliding = 0;
						slidemo->player->glidetime = 0;

						slidemo->angle = R_PointToAngle2(li->v1->x, li->v1->y, li->v2->x, li->v2->y) - ANG90;
						
						if (slidemo->player==&players[consoleplayer])
							localangle = slidemo->angle;
						
						slidemo->player->mfspinning = slidemo->player->mfjumped = 0;
						
						if(slidemo->player->climbing > 1)
							slidemo->momz = slidemo->momx = slidemo->momy = 0;
						
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
//if(mo->player && mo->player->climbing) // Tails 04-12-2001
//return; // Tails 04-12-2001
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


//
// P_LineAttack
//
mobj_t*         linetarget;     // who got hit (or NULL)
mobj_t*         shootthing;

// Height if not aiming up or down
// ???: use slope for monsters?
fixed_t         shootz;
fixed_t         startz; //SoM: Believe it or not, this is useful for 3D floors

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

    if (in->isaline)
    {
        li = in->d.line;

        if ( !(li->flags & ML_TWOSIDED) )
            return false;               // stop

        // Crosses a two sided line.
        // A two sided line will restrict
        // the possible target ranges.
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
    sector_t*           sector;
    mobj_t*             th;

    fixed_t             slope;
    fixed_t             dist;
    fixed_t             thingtopslope;
    fixed_t             thingbottomslope;

    fixed_t             floorz;  //SoM: Bullets should hit fake floors!
    fixed_t             ceilingz;

    //added:18-02-98:
    fixed_t        distz;    //dist between hit z on wall       and gun z
    fixed_t        clipz;    //dist between hit z on floor/ceil and gun z
    boolean        hitplane;    //true if we clipped z on floor/ceil plane
    boolean        diffheights; //check for sky hacks with different ceil heights

    int      sectorside;
    int            dir = aimslope > 0 ? 1 : aimslope < 0 ? -1 : 0;

    if (in->isaline)
    {
        //shut up compiler, otherwise it's only used when TWOSIDED
        diffheights = false;

        li = in->d.line;

        if (li->special)
            P_ShootSpecialLine (shootthing, li);

        if ( !(li->flags & ML_TWOSIDED) )
            goto hitline;

        // crosses a two sided line
        //added:16-02-98: Fab comments : sets opentop, openbottom, openrange
        //                lowfloor is the height of the lowest floor
        //                         (be it front or back)
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

#ifdef FAKEFLOORS
        //SoM: Check 3D FLOORS!
        if(li->frontsector->ffloors)
        {
          ffloor_t*  rover = li->frontsector->ffloors;
          fixed_t    highslope, lowslope;

          for(; rover; rover = rover->next)
          {
            if(!(rover->flags & FF_SOLID)) continue;

            highslope = FixedDiv (*rover->topheight - shootz, dist);
            lowslope = FixedDiv (*rover->bottomheight - shootz, dist);
            if((aimslope >= lowslope && aimslope <= highslope))
              goto hitline;
            if(highslope > 0 && lowslope > 0 && aimslope > highslope)
              goto hitline;
          }
        }


        if(li->backsector->ffloors)
        {
          ffloor_t*  rover = li->backsector->ffloors;
          fixed_t    highslope, lowslope;

          for(; rover; rover = rover->next)
          {
            if(!(rover->flags & FF_SOLID)) continue;

            highslope = FixedDiv (*rover->topheight - shootz, dist);
            lowslope = FixedDiv (*rover->bottomheight - shootz, dist);
            if((aimslope >= lowslope && aimslope <= highslope))
              goto hitline;
          }
        }
#endif

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
#ifdef FAKEFLOORS
            if(sector->ffloors)
            {
              ffloor_t* rover;
              for(rover = sector->ffloors; rover; rover = rover->next)
              {
                if(!(rover->flags & FF_SOLID)) continue;

                if(dir == 1 && *rover->bottomheight < ceilingz && *rover->bottomheight > startz)
                  ceilingz = *rover->bottomheight;
                if(dir == -1 && *rover->topheight > floorz && *rover->topheight < startz)
                  floorz = *rover->topheight;
              }
            }
#endif
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
        }
        //SPLAT TEST ----------------------------------------------------------
        #ifdef WALLSPLATS
        if (!hitplane && demoversion>=129)
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

            //added:24-02-98: compatibility with older demos
            if (demoversion<112)
            {
                diffheights = true;
                hitplane = false;
            }

            // it's a sky hack wall
            if  ((!hitplane &&      //added:18-02-98:not for shots on planes
                 li->backsector &&
                 diffheights &&    //added:18-02-98:skip only REAL sky hacks
                                   //   eg: they use different ceil heights.
                 li->backsector->ceilingpic == skyflatnum))
              return false;
        }

        if(sector->ffloors)
        {
          if(dir == 1 && z > ceilingz)
            z = ceilingz;
          if(dir == -1 && z < floorz)
            z = floorz;
        }

        // Spawn bullet puffs.
        P_SpawnPuff (x,y,z);

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


    // hit thing
    // position a bit closer
    frac = in->frac - FixedDiv (10*FRACUNIT,attackrange);

    x = trace.x + FixedMul (trace.dx, frac);
    y = trace.y + FixedMul (trace.dy, frac);
    z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

    if (demoversion<125)
    {
        // Spawn bullet puffs or blood spots,
        // depending on target type.
        if (in->d.thing->flags & MF_NOBLOOD)
            P_SpawnPuff (x,y,z);
/*        else
            P_SpawnBlood (x,y,z, la_damage);*/
    }

    if (la_damage)
        hitplane = P_DamageMobj (th, shootthing, shootthing, la_damage);
    else
        hitplane = false;

    if (demoversion>=125)
    {
        // Spawn bullet puffs or blood spots,
        // depending on target type.
        if (in->d.thing->flags & MF_NOBLOOD)
            P_SpawnPuff (x,y,z);
        else
            if (hitplane) {
//                P_SpawnBloodSplats (x,y,z, la_damage, trace.dx, trace.dy);
                return false;
            }
    }

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

    if(t1->player && demoversion>=128)
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
    shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;

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
    if(demoversion<128 || !t1->player)
    {   
        // BP: changed from * to fixedmul
        x2 = t1->x + FixedMul(distance,finecosine[angle]);
        y2 = t1->y + FixedMul(distance,finesine[angle]);
    }
    else
    {
        fixed_t cosangle=finecosine[t1->player->aiming>>ANGLETOFINESHIFT];

        x2 = t1->x + FixedMul(FixedMul(distance,finecosine[angle]),cosangle);
        y2 = t1->y + FixedMul(FixedMul(distance,finesine[angle]),cosangle); 
    }

    shootz = t1->z + (t1->height>>1) + 8*FRACUNIT;
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

    if (!in->d.line->special)
    {
        P_LineOpening (in->d.line);
        if (openrange <= 0)
        {
//            S_StartSound (usething, sfx_spndsh);

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
    P_UseSpecialLine (usething, in->d.line, side);

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


    // Boss spider and cyborg
    // take no damage from concussion.
		switch(thing->type) // Tails 08-18-2001
		{
			case MT_EGGMOBILE:
//			case MT_SPIDER:
			case MT_SKIM:
			case MT_JETTBOMBER: // Jetty-Syn Bomber
			case MT_MISC50: // Blue shield box
			case MT_MISC48: // Yellow shield box
			case MT_MISC31: // Green shield box
			case MT_BKTV: // Black shield box
			case MT_MISC74: // Super Sneaker box
			case MT_PRUP: // 1-Up box
			case MT_MISC10: // 10-Ring box
			case MT_MISC11: // 25-Ring box
			case MT_INV: // Invincibility box
				return true;
				break;
			default:
				break;
	}

    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);

    dist = dx>dy ? dx : dy;
    dist -= thing->radius;

    //added:22-02-98: now checks also z dist for rockets exploding
    //                above yer head...
    if (demoversion>=112)
    {
        dz = abs(thing->z+(thing->height>>1) - bombspot->z);
        dist = dist > dz ? dist : dz;
    }
    dist >>= FRACBITS;

    if (dist < 0)
        dist = 0;

    if (dist >= bombdamage)
        return true;    // out of range

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
/*        if( P_DamageMobj (thing, bombspot, bombsource, damage) && (thing->flags & MF_NOBLOOD)==0 && demoversion>=129 )
            P_SpawnBloodSplats (thing->x,thing->y,thing->z, damage, momx, momy);*/
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

//    if(thing->subsector->sector != sectorchecked)
//        return true;

    if (P_ThingHeightClip (thing))
    {
        // keep checking
        return true;
    }

    // crunch bodies to giblets
    if (thing->health <= 0)
    {
        P_SetMobjState (thing, S_GIBS);

        thing->flags &= ~MF_SOLID;
        thing->height = 0;
        thing->radius = 0;
        thing->skin = 0;

        //added:22-02-98: lets have a neat 'crunch' sound!
        S_StartSound (thing, sfx_pop);

        // keep checking
        return true;
    }

    // crunch dropped items
    if (thing->flags & MF_DROPPED)
    {
        P_RemoveMobj (thing);

        // keep checking
        return true;
    }

    if (! (thing->flags & MF_SHOOTABLE) )
    {
        // assume it is bloody gibs or something
        return true;
    }

    nofit = true;

    if (crushchange && !(leveltime&3) )
    {
        P_DamageMobj(thing,NULL,NULL,10);

        // spray blood in a random direction
/*        mo = P_SpawnMobj (thing->x,
                          thing->y,
                          thing->z + thing->height/2, MT_BLOOD);

        mo->momx  = P_Random()<<12;
        mo->momx -= P_Random()<<12;
        mo->momy  = P_Random ()<<12;
        mo->momy -= P_Random ()<<12;*/
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

void P_Initsecnode( void )
{
    headsecnode = NULL;
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

// P_PutSecnode() returns a node to the freelist.

void P_PutSecnode(msecnode_t* node)
{
    node->m_snext = headsecnode;
    headsecnode = node;
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

// Delete an entire sector list

void P_DelSeclist(msecnode_t* node)

{
    while (node)
        node = P_DelSecnode(node);
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
