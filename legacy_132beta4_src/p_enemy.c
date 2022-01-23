// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_enemy.c,v 1.14 2001/07/28 16:18:37 bpereira Exp $
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
// $Log: p_enemy.c,v $
// Revision 1.14  2001/07/28 16:18:37  bpereira
// no message
//
// Revision 1.13  2001/05/27 13:42:47  bpereira
// no message
//
// Revision 1.12  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.11  2001/03/30 17:12:50  bpereira
// no message
//
// Revision 1.10  2001/03/19 21:18:48  metzgermeister
//   * missing textures in HW mode are replaced by default texture
//   * fixed crash bug with P_SpawnMissile(.) returning NULL
//   * deep water trick and other nasty thing work now in HW mode (tested with tnt/map02 eternal/map02)
//   * added cvar gr_correcttricks
//
// Revision 1.9  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.8  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.7  2000/10/08 13:30:01  bpereira
// no message
//
// Revision 1.6  2000/10/01 10:18:17  bpereira
// no message
//
// Revision 1.5  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.4  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.3  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Enemy thinking, AI.
//      Action Pointer Functions
//      that are associated with states/frames.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "r_state.h"
#include "s_sound.h"
#include "m_random.h"
#include "r_things.h"

#include "hardware/hw3sound.h"

void P_Thrust(); // Tails
void P_InstaThrust(); // Tails 08-26-2001
fixed_t P_ReturnThrustX();
fixed_t P_ReturnThrustY();
void P_ExplodeMissile(); // Tails 08-26-2001
void I_PlayCD();
void P_HomingAttack(mobj_t* source, mobj_t* enemy);
boolean PIT_RadiusAttack(mobj_t* thing);

player_t* plyr; // Tails

typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS

} dirtype_t;


//
// P_NewChaseDir related LUT.
//
static dirtype_t opposite[] =
{
  DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
  DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

static dirtype_t diags[] =
{
    DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};





void A_Fall (mobj_t *actor);

// No more FastMonster option Tails 12-11-2001

//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//

static mobj_t*         soundtarget;

static void P_RecursiveSound ( sector_t*     sec,
  int           soundblocks )
{
    int         i;
    line_t*     check;
    sector_t*   other;

    // wake up all monsters in this sector
    if (sec->validcount == validcount
        && sec->soundtraversed <= soundblocks+1)
    {
        return;         // already flooded
    }

    sec->validcount = validcount;
    sec->soundtraversed = soundblocks+1;
    sec->soundtarget = soundtarget;

    for (i=0 ;i<sec->linecount ; i++)
    {
        check = sec->lines[i];
        if (! (check->flags & ML_TWOSIDED) )
            continue;

        P_LineOpening (check);

        if (openrange <= 0)
            continue;   // closed door

        if ( sides[ check->sidenum[0] ].sector == sec)
            other = sides[ check->sidenum[1] ].sector;
        else
            other = sides[ check->sidenum[0] ].sector;
/*
        if (check->flags & ML_SOUNDBLOCK)
        {
            if (!soundblocks)
                P_RecursiveSound (other, 1);
        }
        else*/
            P_RecursiveSound (other, soundblocks);
    }
}



//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
void P_NoiseAlert ( mobj_t*       target,
  mobj_t*       emmiter )
{
    soundtarget = target;
    validcount++;
    P_RecursiveSound (emmiter->subsector->sector, 0);
}




//
// P_CheckMeleeRange
//
static boolean P_CheckMeleeRange (mobj_t*      actor)
{
    mobj_t*     pl;
    fixed_t     dist;

    if (!actor->target)
        return false;

    pl = actor->target;
    dist = P_AproxDistance (pl->x-actor->x, pl->y-actor->y);

	switch(actor->type)
	{
		case MT_JETTBOMBER: // Tails 08-18-2001
			if (dist >= (actor->radius+pl->radius)*2)
				return false;
			break;
		case MT_DETON:
			if (dist >= actor->radius+pl->radius)
				return false;
			break;
		default:
			if (dist >= MELEERANGE-20*FRACUNIT+pl->info->radius)
				return false;
			break;
	}

    //added:19-03-98: check height now, so that damn imps cant attack
    //                you if you stand on a higher ledge.
	if(actor->type == MT_JETTBOMBER || actor->type == MT_SKIM)
	{
		if(pl->z+pl->height > actor->z-(48<<FRACBITS)) // Tails 08-18-2001
			return false; // Tails 08-18-2001
	}
	else
	{
		if ( (pl->z > actor->z+actor->height) ||
			(actor->z > pl->z + pl->height)) // Tails 06-13-2000
			return false;

    if (! P_CheckSight (actor, actor->target) && !(actor->type == MT_JETTBOMBER || actor->type == MT_SKIM))
        return false;
	}

    return true;
}

//
// P_CheckMissileRange
//
static boolean P_CheckMissileRange (mobj_t* actor)
{
    fixed_t     dist;

    if (! P_CheckSight (actor, actor->target) )
        return false;
/*
    if ( actor->flags & MF_JUSTHIT )
    {
        // the target just hit the enemy,
        // so fight back!
        actor->flags &= ~MF_JUSTHIT;
        return true;
    }*/

    if (actor->reactiontime)
        return false;   // do not attack yet

    // OPTIMIZE: get this from a global checksight
    dist = P_AproxDistance ( actor->x-actor->target->x,
                             actor->y-actor->target->y) - 64*FRACUNIT;

    if (!actor->info->meleestate)
        dist -= 128*FRACUNIT;   // no melee attack, so fire more

    dist >>= 16;

/*    if (actor->type == MT_VILE)
    {
        if (dist > 14*64)
            return false;       // too far away
    }

    if (actor->type == MT_UNDEAD)
    {
        if (dist < 196)
            return false;       // close for fist attack
        dist >>= 1;
    }


    if (actor->type == MT_CYBORG
        || actor->type == MT_SPIDER
        || actor->type == MT_SKULL
        || actor->type == MT_IMP) // heretic monster
    {
        dist >>= 1;
    }*/ // Tails

    if (actor->type == MT_EGGMOBILE)
//        || actor->type == MT_SPIDER // Tails 09-03-2001
//        || actor->type == MT_SKULL)
    {
        dist >>= 1;
    }

    if (dist > 200)
        dist = 200;

    if (actor->type == MT_EGGMOBILE && dist > 160)
        dist = 160;

    if (P_Random () < dist)
        return false;

    return true;
}

boolean P_WaterInSector(fixed_t x, fixed_t y)
{
	sector_t* sector;

	sector = R_PointInSubsector(x, y)->sector;

	if(sector->ffloors)
	{
		ffloor_t* rover;

		for(rover = sector->ffloors; rover; rover = rover->next)
		{
			if(!(rover->flags & FF_EXISTS)) continue;

			if(rover->flags & FF_SWIMMABLE)
				return true;
		}
	}

	return false;
}

//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
static const fixed_t xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
static const fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

static boolean P_Move (mobj_t* actor)
{
    fixed_t     tryx;
    fixed_t     tryy;

    if (actor->movedir == DI_NODIR)
        return false;

#ifdef PARANOIA
    if ((unsigned)actor->movedir >= 8)
        I_Error ("Weird actor->movedir!");
#endif

    tryx = actor->x + actor->info->speed*xspeed[actor->movedir];
    tryy = actor->y + actor->info->speed*yspeed[actor->movedir];

	if(actor->type == MT_SKIM && P_WaterInSector(tryx, tryy) == true)
		return false;

    if (!P_TryMove (actor, tryx, tryy, false))
    {
        // open any specials
        if (actor->flags & MF_FLOAT && floatok)
        {
            // must adjust height
            if (actor->z < tmfloorz)
                actor->z += FLOATSPEED;
            else
                actor->z -= FLOATSPEED;

            actor->flags |= MF_INFLOAT;
            return true;
        }

        if (!numspechit)
            return false;

        actor->movedir = DI_NODIR;
        return false;
    }
    else
    {
        actor->flags &= ~MF_INFLOAT;
    }


    if (! (actor->flags & MF_FLOAT) )
    {
        actor->z = actor->floorz;
    }
    return true;
}


//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
static boolean P_TryWalk (mobj_t* actor)
{
    if (!P_Move (actor))
    {
        return false;
    }
    actor->movecount = P_Random()&15;
    return true;
}




static void P_NewChaseDir (mobj_t*     actor)
{
    fixed_t     deltax;
    fixed_t     deltay;

    dirtype_t   d[3];

    int         tdir;
    dirtype_t   olddir;

    dirtype_t   turnaround;

    if (!actor->target)
        I_Error ("P_NewChaseDir: called with no target");

    olddir = actor->movedir;
    turnaround=opposite[olddir];

    deltax = actor->target->x - actor->x;
    deltay = actor->target->y - actor->y;

    if (deltax>10*FRACUNIT)
        d[1]= DI_EAST;
    else if (deltax<-10*FRACUNIT)
        d[1]= DI_WEST;
    else
        d[1]= DI_NODIR;

    if (deltay<-10*FRACUNIT)
        d[2]= DI_SOUTH;
    else if (deltay>10*FRACUNIT)
        d[2]= DI_NORTH;
    else
        d[2]= DI_NODIR;

    // try direct route
    if (   d[1] != DI_NODIR
        && d[2] != DI_NODIR)
    {
        actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
        if (actor->movedir != turnaround && P_TryWalk(actor))
            return;
    }

    // try other directions
    if (P_Random() > 200
        ||  abs(deltay)>abs(deltax))
    {
        tdir=d[1];
        d[1]=d[2];
        d[2]=tdir;
    }

    if (d[1]==turnaround)
        d[1]=DI_NODIR;
    if (d[2]==turnaround)
        d[2]=DI_NODIR;

    if (d[1]!=DI_NODIR)
    {
        actor->movedir = d[1];
        if (P_TryWalk(actor))
        {
            // either moved forward or attacked
            return;
        }
    }

    if (d[2]!=DI_NODIR)
    {
        actor->movedir =d[2];

        if (P_TryWalk(actor))
            return;
    }

    // there is no direct path to the player,
    // so pick another direction.
    if (olddir!=DI_NODIR)
    {
        actor->movedir =olddir;

        if (P_TryWalk(actor))
            return;
    }

    // randomly determine direction of search
    if (P_Random()&1)
    {
        for ( tdir=DI_EAST;
              tdir<=DI_SOUTHEAST;
              tdir++ )
        {
            if (tdir!=turnaround)
            {
                actor->movedir =tdir;

                if ( P_TryWalk(actor) )
                    return;
            }
        }
    }
    else
    {
        for ( tdir=DI_SOUTHEAST;
              tdir >= DI_EAST;
              tdir-- )
        {
            if (tdir!=turnaround)
            {
                actor->movedir =tdir;

                if ( P_TryWalk(actor) )
                    return;
            }
        }
    }

    if (turnaround !=  DI_NODIR)
    {
        actor->movedir =turnaround;
        if ( P_TryWalk(actor) )
            return;
    }

    actor->movedir = DI_NODIR;  // can not move
}


extern consvar_t cv_objectplace;

//
// P_LookForPlayers
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
static boolean P_LookForPlayers ( mobj_t*       actor,
                                  boolean       allaround )
{
    int         c;
    int         stop;
    player_t*   player;
    sector_t*   sector;
    angle_t     an;
    fixed_t     dist;

	if(cv_objectplace.value)
		return false;

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

// Start Ring Shield finder Tails 06-08-2000
//
// P_LookForShield
// Returns true if a player is targeted that has a yellow shield.
//
boolean
P_LookForShield
(mobj_t*       actor)
{
    int         c;
    int         stop;
    player_t*   player;
    sector_t*   sector;

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

		if(player->powers[pw_yellowshield]
		   && ((P_AproxDistance(actor->x-player->mo->x, actor->y-player->mo->y) < RING_DIST
		   && abs(player->mo->z-actor->z) < RING_DIST) || RING_DIST==0))
		{
				actor->target = player->mo;
				return true;
		}
		else
			continue;
    }

    return false;
}
// End Ring Shield finder Tails 06-08-2000
//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
void A_Look (mobj_t* actor)
{
    mobj_t*     targ;

    targ = actor->subsector->sector->soundtarget;

    if (targ && (targ->flags & MF_SHOOTABLE) )
    {
        actor->target = targ;

        if ( actor->flags & MF_AMBUSH )
        {
            if (P_CheckSight (actor, actor->target))
                goto seeyou;
        }
        else
            goto seeyou;
    }


    if (!P_LookForPlayers (actor, false) )
        return;

    // go into chase state
  seeyou:
    if (actor->info->seesound)
    {
        int             sound;

        switch (actor->info->seesound)
        {
          default:
            sound = actor->info->seesound;
            break;
        }
/*
        if (actor->type==MT_SPIDER
         || actor->type == MT_CYBORG
         || (actor->flags2&MF2_BOSS))
        {
            // full volume
            S_StartSound (NULL, sound);
        }
        else
            S_StartScreamSound(actor, sound);*/ // Tails
    }

    P_SetMobjState (actor, actor->info->seestate);
}


//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
void A_Chase (mobj_t*   actor)
{
    int         delta;

    if (actor->reactiontime)
        actor->reactiontime--;


    // modify target threshold
    if  (actor->threshold)
    {
        if (!actor->target
              || actor->target->health <= 0)
        {
            actor->threshold = 0;
        }
        else
            actor->threshold--;
    }

    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
        actor->angle &= (7<<29);
        delta = actor->angle - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90/2;
        else if (delta < 0)
            actor->angle += ANG90/2;
    }

    if (!actor->target
        || !(actor->target->flags&MF_SHOOTABLE))
    {
        // look for a new target
        if (P_LookForPlayers(actor,true))
            return;     // got a new target

        P_SetMobjState (actor, actor->info->spawnstate);
        return;
    }

    // do not attack twice in a row
    if (actor->flags2 & MF2_JUSTATTACKED)
    {
        actor->flags2 &= ~MF2_JUSTATTACKED;
        P_NewChaseDir (actor);
        return;
    }

    // check for melee attack
    if (actor->info->meleestate
        && P_CheckMeleeRange (actor))
    {
        if (actor->info->attacksound)
            S_StartAttackSound(actor, actor->info->attacksound);

        P_SetMobjState (actor, actor->info->meleestate);
        return;
    }

    // check for missile attack
    if (actor->info->missilestate)
    {
        if (actor->movecount)
        {
            goto nomissile;
        }

        if (!P_CheckMissileRange (actor))
            goto nomissile;

        P_SetMobjState (actor, actor->info->missilestate);
        actor->flags2 |= MF2_JUSTATTACKED;
        return;
    }

    // ?
  nomissile:
    // possibly choose another target
    if (multiplayer
        && !actor->threshold
        && !P_CheckSight (actor, actor->target) )
    {
        if (P_LookForPlayers(actor,true))
            return;     // got a new target
    }

    // chase towards player
    if (--actor->movecount<0
        || !P_Move (actor))
    {
        P_NewChaseDir (actor);
    }
/*
    // make active sound
    if (actor->info->activesound
        && P_Random () < 3)
    {
        if(actor->type == MT_WIZARD && P_Random() < 128)
            S_StartScreamSound(actor, actor->info->seesound);
        else if(actor->type == MT_SORCERER2)
            S_StartSound(NULL, actor->info->activesound);
        else
            S_StartScreamSound(actor, actor->info->activesound);
    }*/ // Tails
}

void A_SkimChase (mobj_t*   actor)
{
    int         delta;

    if (actor->reactiontime)
        actor->reactiontime--;

    // modify target threshold
    if  (actor->threshold)
    {
        if (!actor->target
              || actor->target->health <= 0)
        {
            actor->threshold = 0;
        }
        else
            actor->threshold--;
    }

    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
        actor->angle &= (7<<29);
        delta = actor->angle - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90/2;
        else if (delta < 0)
            actor->angle += ANG90/2;
    }

    if (!actor->target
        || !(actor->target->flags&MF_SHOOTABLE))
    {
        // look for a new target
        if (P_LookForPlayers(actor,true))
            return;     // got a new target

        P_SetMobjState (actor, actor->info->spawnstate);
        return;
    }

    // do not attack twice in a row
    if (actor->flags2 & MF2_JUSTATTACKED)
    {
        actor->flags &= ~MF2_JUSTATTACKED;
        P_NewChaseDir (actor);
        return;
    }

    // check for melee attack
    if (actor->info->meleestate
        && P_CheckMeleeRange (actor))
    {
        if (actor->info->attacksound)
            S_StartAttackSound(actor, actor->info->attacksound);

        P_SetMobjState (actor, actor->info->meleestate);
        return;
    }

    // check for missile attack
    if (actor->info->missilestate)
    {
        if (actor->movecount)
        {
            goto nomissile;
        }

        if (!P_CheckMissileRange (actor))
            goto nomissile;

        P_SetMobjState (actor, actor->info->missilestate);
        actor->flags2 |= MF2_JUSTATTACKED;
        return;
    }

    // ?
  nomissile:
    // possibly choose another target
    if (multiplayer
        && !actor->threshold
        && !P_CheckSight (actor, actor->target) )
    {
        if (P_LookForPlayers(actor,true))
            return;     // got a new target
    }

    // chase towards player
    if (--actor->movecount<0
        || !P_Move (actor))
    {
        P_NewChaseDir (actor);
    }
}

//
// A_FaceTarget
//
void A_FaceTarget (mobj_t* actor)
{
    if (!actor->target)
        return;

    actor->flags &= ~MF_AMBUSH;

    actor->angle = R_PointToAngle2 (actor->x,
                                    actor->y,
                                    actor->target->x,
                                    actor->target->y);
}

void A_CyberAttack (mobj_t* actor)
{
	fixed_t x,y,z;

    if (!actor->target)
        return;

	A_FaceTarget(actor);

	if(P_Random() & 1)
	{
		x = actor->x + P_ReturnThrustX(actor, actor->angle-ANG90, 43*FRACUNIT);
		y = actor->y + P_ReturnThrustY(actor, actor->angle-ANG90, 43*FRACUNIT);
	}
	else
	{
		x = actor->x + P_ReturnThrustX(actor, actor->angle+ANG90, 43*FRACUNIT);
		y = actor->y + P_ReturnThrustY(actor, actor->angle+ANG90, 43*FRACUNIT);
	}

	z = actor->z + 48*FRACUNIT;

    P_SpawnXYZMissile (actor, actor->target, MT_ROCKET,
						x, y, z);

	if(!(actor->flags & MF_BOSS))
	{
		if(gameskill <= sk_medium)
			actor->reactiontime = actor->info->reactiontime*TICRATE*2;
		else
			actor->reactiontime = actor->info->reactiontime*TICRATE;
	}
}
/*
#define FATSPREAD       (ANG90/8)

void A_FatAttack1 (mobj_t* actor)
{
    mobj_t*     mo;
    int         an;

    A_FaceTarget (actor);
    // Change direction  to ...
    actor->angle += FATSPREAD;
    P_SpawnMissile (actor, actor->target, MT_FATSHOT);

    mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
    if(mo)
    {
        mo->angle += FATSPREAD;
        an = mo->angle >> ANGLETOFINESHIFT;
        mo->momx = FixedMul (mo->info->speed, finecosine[an]);
        mo->momy = FixedMul (mo->info->speed, finesine[an]);
    }
}

void A_FatAttack2 (mobj_t* actor)
{
    mobj_t*     mo;
    int         an;

    A_FaceTarget (actor);
    // Now here choose opposite deviation.
    actor->angle -= FATSPREAD;
    P_SpawnMissile (actor, actor->target, MT_FATSHOT);

    mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
    if(mo)
    {
        mo->angle -= FATSPREAD*2;
        an = mo->angle >> ANGLETOFINESHIFT;
        mo->momx = FixedMul (mo->info->speed, finecosine[an]);
        mo->momy = FixedMul (mo->info->speed, finesine[an]);
    }
}

void A_FatAttack3 (mobj_t*      actor)
{
    mobj_t*     mo;
    int         an;

    A_FaceTarget (actor);

    mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
    if(mo)
    {
        mo->angle -= FATSPREAD/2;
        an = mo->angle >> ANGLETOFINESHIFT;
        mo->momx = FixedMul (mo->info->speed, finecosine[an]);
        mo->momy = FixedMul (mo->info->speed, finesine[an]);
    }
    
    mo = P_SpawnMissile (actor, actor->target, MT_FATSHOT);
    if(mo)
    {
        mo->angle += FATSPREAD/2;
        an = mo->angle >> ANGLETOFINESHIFT;
        mo->momx = FixedMul (mo->info->speed, finecosine[an]);
        mo->momy = FixedMul (mo->info->speed, finesine[an]);
    }
}
*/

//
// SkullAttack
// Fly at the player like a missile.
//
#define SKULLSPEED              (20*FRACUNIT)

void A_SkullAttack (mobj_t* actor)
{
    mobj_t*             dest;
    angle_t             an;
    int                 dist;

    if (!actor->target)
        return;

    dest = actor->target;
    actor->flags2 |= MF2_SKULLFLY;
    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget (actor);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul (SKULLSPEED, finecosine[an]);
    actor->momy = FixedMul (SKULLSPEED, finesine[an]);
    dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
    dist = dist / SKULLSPEED;

    if (dist < 1)
        dist = 1;
    actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
}

void A_BossZoom (mobj_t* actor)
{
    mobj_t*             dest;
    angle_t             an;
    int                 dist;

    if (!actor->target)
        return;

    dest = actor->target;
    actor->flags2 |= MF2_SKULLFLY;
    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget (actor);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul (actor->info->speed*5*FRACUNIT, finecosine[an]);
    actor->momy = FixedMul (actor->info->speed*5*FRACUNIT, finesine[an]);
    dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
    dist = dist / (actor->info->speed*5*FRACUNIT);

    if (dist < 1)
        dist = 1;
    actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;
}

// Spawn explosions around the boss and do the sound!
// Tails 10-28-2002
void A_BossScream (mobj_t* actor)
{
	fixed_t x,y,z;
	const double deg2rad = 0.017453293;

	actor->movecount += actor->info->speed*16;
	x = actor->x + cos(actor->movecount * deg2rad) * actor->info->radius;
	y = actor->y + sin(actor->movecount * deg2rad) * actor->info->radius;

	if(actor->movecount >= 360)
		actor->movecount -= 360;

	z = actor->z - 8*FRACUNIT + ((P_Random()<<FRACBITS) / 4);
    S_StartSound(P_SpawnMobj(x,y,z, MT_BOSSEXPLODE), actor->info->deathsound);
}

void A_Scream (mobj_t* actor)
{
    int         sound;

    switch (actor->info->deathsound)
    {
      case 0:
        return;

	// Removed random screams Tails 09-28-2001

      default:
		if(actor->tracer && (actor->tracer->type == MT_SHELL || actor->tracer->type == MT_FIREBALL))
			sound = sfx_lose;
		else
			sound = actor->info->deathsound;
        break;
    }
/*
    // Check for bosses.
    if (actor->type==MT_SPIDER
        || actor->type == MT_CYBORG)
    {
        // full volume
        S_StartSound (NULL, sound);
    }
    else*/ // Tails
        S_StartSound(actor, sound);
}

void A_Pain (mobj_t* actor)
{
    if (actor->info->painsound)
        S_StartSound(actor, actor->info->painsound);
}


//
//  A dying thing falls to the ground (monster deaths)
//
void A_Fall (mobj_t *actor)
{
    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;

	actor->flags |= MF_NOCLIP;
	actor->flags |= MF_NOGRAVITY;
	actor->flags |= MF_FLOAT;

    // So change this if corpse objects
    // are meant to be obstacles.
}

extern consvar_t cv_teleporters;
extern consvar_t cv_superring;
extern consvar_t cv_silverring;
extern consvar_t cv_supersneakers;
extern consvar_t cv_invincibility;
extern consvar_t cv_blueshield;
extern consvar_t cv_greenshield;
extern consvar_t cv_yellowshield;
extern consvar_t cv_redshield;
extern consvar_t cv_blackshield;
extern consvar_t cv_1up;
extern consvar_t cv_eggmanbox;

// Fall and Scream and spawn an exploding monitor
// Tails 08-28-2002
void A_MonitorPop (mobj_t *actor)
{
	mobj_t* remains;
	mobjtype_t item;
	int random;
	mobjtype_t newbox;

    // de-solidify
	P_UnsetThingPosition(actor);
    actor->flags &= ~MF_SOLID;
	actor->flags |= MF_NOCLIP;
	actor->flags |= MF_NOBLOCKMAP;
	P_SetThingPosition(actor);

    remains = P_SpawnMobj(actor->x, actor->y, actor->z, actor->info->speed);
	remains->type = actor->type; // Transfer type information
	remains->flags = actor->flags; // Transfer flags
	remains->fuse = actor->fuse; // Transfer respawn timer
	remains->threshold = 68;
	actor->flags2 |= MF2_BOSSNOTRAP; // Dummy flag to mark this as an exploded TV until it respawns
	actor->tracer = remains;

	S_StartSound(remains, actor->info->deathsound);

	switch(actor->type)
	{
		case MT_QUESTIONBOX: // Random!
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
			if(cv_eggmanbox.value)
			{
				oldi = i;
				for(; i < oldi + cv_eggmanbox.value; i++)
				{
					spawnchance[i] = MT_EGGMANBOX;
					numchoices++;
				}
			}
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
				|| cv_superring.value || cv_eggmanbox.value || cv_teleporters.value))
			{
				CONS_Printf("Note: All monitors turned off.\n");
				return;
			}

			newbox = spawnchance[random%numchoices];
			item = mobjinfo[newbox].damage;

			remains->flags &= ~MF_AMBUSH;
			break;
		}
		default:
			item = actor->info->damage;
			break;
	}

	if(item != 0)
		P_SpawnMobj(actor->x, actor->y, actor->z + 13*FRACUNIT, item)->target = actor->target; // Transfer target
	else
		CONS_Printf("Powerup item not defined in 'damage' field for A_MonitorPop\n");

	P_SetMobjState(actor, S_DISS);
}


//
// A_Explode
//
void A_Explode (mobj_t* actor)
{
    P_RadiusAttack ( actor, actor->target, actor->info->damage );
}


static state_t *P_FinalState(statenum_t state)
{
    while(states[state].tics!=-1)
        state=states[state].nextstate;

    return &states[state];
}

void P_DoPlayerExit(player_t* player);
//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
void A_BossDeath (mobj_t* mo)
{
    thinker_t*  th;
    mobj_t*     mo2;
    line_t      junk;
    int         i;

    // make sure there is a player alive for victory
    for (i=0 ; i<MAXPLAYERS ; i++)
        if (playeringame[i] && players[i].health > 0)
            break;

    if (i==MAXPLAYERS)
        return; // no one left alive, so do not end game

    // scan the remaining thinkers to see
    // if all bosses are dead
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;

        mo2 = (mobj_t *)th;
        if (mo2 != mo
            && mo2->type == mo->type
            /*&& mo2->health > 0*/  // the old one (doom original 1.9)
            && mo2->state!=P_FinalState(mo->info->deathstate))
        {
            // other boss not dead
            return;
        }
    }

    // victory!
/*            if (mo->type == MT_FATSO)
            {
                junk.tag = 666;
                EV_DoFloor(&junk,lowerFloorToLowest);
                return;
            }

            if (mo->type == MT_BABY)
            {
                junk.tag = 667;
                EV_DoFloor(&junk,raiseToTexture);
                return;
            }*/
	if (mo->type == MT_EGGMOBILE || mo->type == MT_EGGMOBILE2) // Tails 12-01-99
    {
		if(mo->flags2 & MF2_CHAOSBOSS)
		{
			mo->health = 0;
			P_SetMobjState(mo, S_DISS);
			return;
		}

		if(mo->flags2 & MF2_BOSSNOTRAP)
		{
			for(i=0; i<MAXPLAYERS; i++)
				P_DoPlayerExit(&players[i]);
		}
		else
		{
			// Bring the egg trap up to the surface
			junk.tag = 680;
			EV_DoElevator(&junk, elevateHighest, false);
			junk.tag = 681;
			EV_DoElevator(&junk, elevateUp, false);
			junk.tag = 682;
			EV_DoElevator(&junk, elevateHighest, false);
		}

		// Stop exploding and prepare to run.
		P_SetMobjState(mo, mo->info->xdeathstate);

		mo->target = NULL;

		// Flee! Flee! Find a point to escape to! If none, just shoot upward!
		// scan the thinkers
		// to find the runaway point
		for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo2 = (mobj_t *)th;

			if(mo2->type == MT_BOSSFLYPOINT)
				mo->target = mo2;
		}

		if(mo->target)
		{
			mo->flags |= MF_NOGRAVITY;
			mo->flags |= MF_NOCLIP;
			if(mo->z < mo->floorz + 64*FRACUNIT)
				mo->momz = 2*FRACUNIT;
			mo->angle = R_PointToAngle2(mo->x, mo->y, mo->target->x, mo->target->y);
			mo->flags2 |= MF2_BOSSFLEE;
			mo->momz = FixedMul(FixedDiv(mo->target->z - mo->z, P_AproxDistance(mo->x-mo->target->x,mo->y-mo->target->y)), 2*FRACUNIT);
		}
		else
		{
			mo->flags |= MF_NOGRAVITY;
			mo->flags |= MF_NOCLIP;
			mo->momz = 2*FRACUNIT;
		}

		if(mo->type == MT_EGGMOBILE2)
		{
			mo2 = P_SpawnMobj(mo->x + P_ReturnThrustX(mo, mo->angle-ANG90, 32*FRACUNIT), mo->y + P_ReturnThrustY(mo, mo->angle-ANG90, 24*FRACUNIT), mo->z + mo->height/2 - 8*FRACUNIT, MT_BOSSTANK1); // Right tank
			mo2->angle = mo->angle;
			P_InstaThrust(mo2, mo2->angle-ANG90, 4*FRACUNIT);
			mo2->momz = 4*FRACUNIT;

			mo2 = P_SpawnMobj(mo->x + P_ReturnThrustX(mo, mo->angle+ANG90, 32*FRACUNIT), mo->y + P_ReturnThrustY(mo, mo->angle-ANG90, 24*FRACUNIT), mo->z + mo->height/2 - 8*FRACUNIT, MT_BOSSTANK2); // Left tank
			mo2->angle = mo->angle;
			P_InstaThrust(mo2, mo2->angle+ANG90, 4*FRACUNIT);
			mo2->momz = 4*FRACUNIT;

			P_SpawnMobj(mo->x, mo->y, mo->z + mo->height + 32*FRACUNIT, MT_BOSSSPIGOT)->momz = 4*FRACUNIT;
			return;
		}
    } // Tails 12-01-99
	else if(mariomode && mo->type == MT_KOOPA)
	{
		junk.tag = 650;
		EV_DoCeiling(&junk, raiseToHighest);
		return;
	}
/*
    if( cv_allowexitlevel.value )
        G_ExitLevel ();*/
}

void A_CustomPower(mobj_t* actor)
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	if(actor->info->reactiontime >= NUMPOWERS)
	{
		CONS_Printf("Power #%d out of range!\n", actor->info->reactiontime);
		return;
	}

	player = actor->target->player;

	player->powers[actor->info->reactiontime] = actor->info->painchance;
    S_StartSound (player->mo, actor->info->seesound);
}

// start shields Tails 12-05-99
void A_BlueShield (mobj_t* actor) // Tails 12-05-99
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	player = actor->target->player;

	if(!(player->powers[pw_blueshield]))
		P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z, MT_BLUEORB)->target = player->mo;

	player->powers[pw_blueshield] = true;
	player->powers[pw_redshield] = false;
	player->powers[pw_blackshield] = false;
	player->powers[pw_greenshield] = false;
	player->powers[pw_yellowshield] = false; // get rid of yellow shield if have it Tails 03-15-2000
	S_StartSound (player->mo, sfx_shield);
}

void A_YellowShield (mobj_t* actor) // Tails 12-05-99
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	player = actor->target->player;

	if(!(player->powers[pw_yellowshield]))
		P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z, MT_YELLOWORB)->target = player->mo;

	player->powers[pw_yellowshield] = true;
    player->powers[pw_blackshield] = false;
    player->powers[pw_greenshield] = false;
	player->powers[pw_redshield] = false;
    player->powers[pw_blueshield] = false; // get rid of blue shield if have it Tails 03-15-2000
    S_StartSound (player->mo, sfx_shield);
}
// end shields Tails 12-05-99

// Start Ringbox codes Tails

void A_RingBox (mobj_t* actor)
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	player = actor->target->player;

    player->health += actor->info->reactiontime;
	player->mo->health = player->health;
	player->totalring += actor->info->reactiontime;
    S_StartSound (player->mo, actor->info->seesound);
}

// End ringbox codes Tails

// start invincibility code Tails

void A_Invincibility (mobj_t* actor)
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	player = actor->target->player;

    player->powers[pw_invulnerability] = invulntics + 1;

	if(player == &players[consoleplayer])
	{
		if(player->powers[pw_super] == false)
		{
			S_StopMusic();
			if(mariomode)
				S_ChangeMusic(mus_minvnc, false);
			else
				S_ChangeMusic(mus_invinc, false);
		}
	}
}

//end invincibility tails

//start super sneakers tails 03-04-2000
void A_SuperSneakers (mobj_t* actor)
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	player = actor->target->player;

    actor->target->player->powers[pw_sneakers] = sneakertics + 1;

	if(player == &players[consoleplayer])
	{
		if(player->powers[pw_super] == false)
		{
			S_StopMusic();
			S_ChangeMusic(mus_shoes, false);
		}
	}
}
//end super sneakers tails 03-04-2000

// start extra life Tails 03-12-2000
void A_ExtraLife (mobj_t* actor)
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	player = actor->target->player;

    player->lives += 1;

	if(mariomode)
		S_StartSound(player->mo, sfx_marioa);
	else
	{
		player->powers[pw_extralife] = extralifetics + 1;

		if(player == &players[consoleplayer])
		{
			S_StopMusic();
			S_ChangeMusic(mus_xtlife, false);
		}
	}
}
// end extra life Tails 03-12-2000

// start black shield Tails 03-12-2000
void A_BlackShield (mobj_t* actor)
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	player = actor->target->player;

	if(!(player->powers[pw_blackshield]))
	    P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z, MT_BLACKORB)->target = player->mo;

	player->powers[pw_blackshield] = true;
    player->powers[pw_greenshield] = false;
	player->powers[pw_redshield] = false;
    player->powers[pw_yellowshield] = false; // get rid of yellow shield if have it Tails 03-15-2000
    player->powers[pw_blueshield] = false; // get rid of yellow shield if have it Tails 03-15-2000
    S_StartSound (player->mo, sfx_shield);
}
// end black shield Tails 03-12-2000

// start green shield Tails 04-08-2000
void A_GreenShield (mobj_t* actor)
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	player = actor->target->player;

	if(!(player->powers[pw_greenshield]))
		P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GREENORB)->target = player->mo;

    player->powers[pw_blackshield] = false;
    player->powers[pw_greenshield] = true;
	player->powers[pw_redshield] = false;
    player->powers[pw_yellowshield] = false; // get rid of yellow shield if have it Tails 03-15-2000
    player->powers[pw_blueshield] = false; // get rid of yellow shield if have it Tails 03-15-2000
    if(player->powers[pw_underwater] > 12*TICRATE + 1)
    {
        player->powers[pw_underwater]= 0;
    }
	else if(player->powers[pw_underwater] <= 12*TICRATE + 1)
    {
		player->powers[pw_underwater]= 0;

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

	if(player->powers[pw_spacetime] > 1)
    {
		player->powers[pw_spacetime]= 0;

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
    S_StartSound (player->mo, sfx_shield);
}
// end green shield Tails 04-08-2000

void A_RedShield (mobj_t* actor)
{
	player_t* player;

	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	player = actor->target->player;

	if(!(player->powers[pw_redshield]))
		P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_REDORB)->target = player->mo;

	player->powers[pw_redshield] = true;
    player->powers[pw_blackshield] = false;
    player->powers[pw_greenshield] = false;
    player->powers[pw_yellowshield] = false; // get rid of yellow shield if have it Tails 03-15-2000
    player->powers[pw_blueshield] = false; // get rid of yellow shield if have it Tails 03-15-2000
    S_StartSound (player->mo, sfx_shield);
}

// start score logo rise Tails 04-16-2000
void A_ScoreRise (mobj_t*  actor)
{
	actor->momz = actor->info->speed; // make logo rise!
}
// end score logo rise Tails 04-16-2000

// start bunny hop tails

void A_BunnyHop (mobj_t*   actor)
{
	if(actor->z <= actor->floorz)
	{
		actor->momz = 6*FRACUNIT; // make bunny hop!
		actor->angle += P_Random()*FINEANGLES;
		P_InstaThrust(actor, actor->angle, 3*FRACUNIT); // Launch the bunny! PHOOM!!
	}
}

// end bunny hop tails

// start random bubble spawn Tails 03-07-2000
void A_BubbleSpawn (mobj_t*   actor)
{

	if(actor->eflags & MF_UNDERWATER)
		actor->flags2 |= MF2_DONTDRAW;
	else
		actor->flags2 &= ~MF2_DONTDRAW; // Don't draw above water

 if (P_Random () > 128)
   {
        P_SpawnMobj (actor->x,actor->y,actor->z + (actor->height / 2), MT_SMALLBUBBLE);
   }
else if (P_Random () < 128 && P_Random () > 96)
   {
        P_SpawnMobj (actor->x,actor->y,actor->z + (actor->height / 2), MT_MEDIUMBUBBLE);
   }
else if (P_Random () > 96 && P_Random () < 160)
   {
        P_SpawnMobj (actor->x,actor->y,actor->z + (actor->height / 2), MT_EXTRALARGEBUBBLE);
   }
}
// end random bubble spawn Tails 03-07-2000

// start bubble floating Tails 03-07-2000
void A_BubbleRise (mobj_t*   actor)
{
	if(actor->type == MT_EXTRALARGEBUBBLE)
		actor->momz = 1.2*FRACUNIT; // make bubbles rise!
	else
	{
		actor->momz += 1024; // make bubbles rise!

		// Move around slightly to make it look like it's bending around the water
		if(P_Random() < 32)
			P_InstaThrust(actor, P_Random()&1 ? actor->angle + ANG90 : actor->angle, P_Random()&1? FRACUNIT/2 : -FRACUNIT/2);
		else if(P_Random() < 32)
			P_InstaThrust(actor, P_Random()&1 ? actor->angle - ANG90 : actor->angle - ANG180, P_Random()&1? FRACUNIT/2 : -FRACUNIT/2);
	}
}
// end bubble floating Tails 03-07-2000

void P_Attract (mobj_t* source, mobj_t* enemy, boolean nightsgrab) // Home in on your target Tails 06-19-2001
{
    fixed_t     dist;
    mobj_t*     dest;
	fixed_t stuff;

	if(!(enemy->health))
		return;

	if(!source->tracer)
		return; // Nothing to home in on!

	if(!enemy->player)
		return;

    // adjust direction
    dest = source->tracer;

    if (!dest || dest->health <= 0)
        return;

    // change angle
	source->angle = R_PointToAngle2(source->x, source->y, enemy->x, enemy->y);

    // change slope
	dist = P_AproxDistance(P_AproxDistance(dest->x - source->x, dest->y - source->y), dest->z - source->z);

    if (dist < 1)
        dist = 1;


	if(nightsgrab)
	{
		stuff = P_AproxDistance(enemy->momx, enemy->momy);
		stuff += 8*FRACUNIT;
	}
	else
		stuff = source->info->speed;

	source->momx = FixedMul(FixedDiv(dest->x - source->x, dist), stuff);
	source->momy = FixedMul(FixedDiv(dest->y - source->y, dist), stuff);
	source->momz = FixedMul(FixedDiv(dest->z - source->z, dist), stuff);

	return; // Tails 06-20-2001
}

void A_AttractChase (mobj_t*   actor)
{
	if(actor->flags2 & MF2_NIGHTSPULL)
		return;

	// spilled rings flicker before disappearing Tails 01-11-2001
	if(leveltime & 1 && actor->type == actor->info->reactiontime && actor->fuse && actor->fuse < 2*TICRATE)
		actor->flags2 |= MF2_DONTDRAW;
	else
		actor->flags2 &= ~MF2_DONTDRAW;

	if(actor->target && actor->target->player
		&& !actor->target->player->powers[pw_yellowshield] && actor->type != actor->info->reactiontime)
	{
		mobj_t* newring;
		newring = P_SpawnMobj(actor->x, actor->y, actor->z, actor->info->reactiontime);
		newring->flags |= MF_COUNTITEM;
		newring->momx = actor->momx;
		newring->momy = actor->momy;
		newring->momz = actor->momz;
		P_SetMobjState(actor, S_DISS);
	}

	P_LookForShield(actor); // Go find 'em, boy! Tails 06-08-2000

	actor->tracer = actor->target;

	if(!actor->target)
	{
		actor->target = NULL;
		actor->tracer = NULL;
		return;
	}

	if(!actor->target->player)
		return;

	if(!actor->target->health)
		return;

	// If a FlingRing gets attracted by a shield, change it into a normal
	// ring, but don't count towards the total.
	if(actor->type == actor->info->reactiontime)
	{
		P_SetMobjState(actor, S_DISS);
		if(actor->flags & MF_COUNTITEM)
			P_SpawnMobj(actor->x, actor->y, actor->z, actor->info->painchance);
		else
			P_SpawnMobj(actor->x, actor->y, actor->z, actor->info->painchance)->flags &= ~MF_COUNTITEM;
	}

	P_Attract(actor, actor->tracer, false);
}

void A_NightsItemChase (mobj_t*   actor)
{
	if(!actor->tracer)
	{
		actor->tracer = NULL;
		actor->flags2 &= ~MF2_NIGHTSPULL;
		return;
	}

	if(!actor->tracer->player)
		return;

	P_Attract(actor, actor->tracer, true);
}

void A_DropMine(mobj_t* actor) // Tells Skim to drop mine on player Tails 06-13-2000
{
	P_SpawnMobj (actor->x,actor->y,actor->z - (12<<FRACBITS), MT_MINE);
}
// start shield positions Tails 06-28-2000 Works in multiplayer, too!
boolean A_GreenLook(mobj_t* actor)
{
	if(!actor)
		return false;

	if(actor->state == &states[S_DISS])
		return false;

	if(!actor->target || (actor->target->health <= 0) || !actor->target->player || (actor->target->player && !actor->target->player->powers[pw_greenshield]))
	{
		P_SetMobjState(actor, S_DISS);
//		P_RemoveMobj(actor);
		return false;
	}
	else if(actor->target->player->powers[pw_greenshield])
	{
		P_UnsetThingPosition (actor);
		actor->x = actor->target->x;
		actor->y = actor->target->y;
		actor->z = actor->target->z - (actor->target->info->height - actor->target->height)/3; // Adjust height for height changes
		P_SetThingPosition (actor);
		if(actor->target->player->powers[pw_super] == true || actor->target->player->powers[pw_invulnerability] > 1)
			P_SetMobjState(actor, S_DISS);
	}
	return true;
}
boolean A_BlueLook(mobj_t* actor)
{
	if(!actor)
		return false;

	if(actor->state == &states[S_DISS])
		return false;

	if(!actor->target || (actor->target->health <= 0) || !actor->target->player || (actor->target->player && !actor->target->player->powers[pw_blueshield]))
	{
		P_SetMobjState(actor, S_DISS);
//		P_RemoveMobj(actor);
		return false;
	}
	else if(actor->target->player->powers[pw_blueshield])
	{
		P_UnsetThingPosition (actor);
		actor->x = actor->target->x;
		actor->y = actor->target->y;
		actor->z = actor->target->z - (actor->target->info->height - actor->target->height)/3; // Adjust height for height changes
		P_SetThingPosition (actor);
		if(actor->target->player->powers[pw_super] == true || actor->target->player->powers[pw_invulnerability] > 1)
			P_SetMobjState(actor, S_DISS);
	}
	return true;
}
boolean A_RedLook(mobj_t* actor)
{
	if(!actor)
		return false;

	if(actor->state == &states[S_DISS])
		return false;

	if(!actor->target || (actor->target->health <= 0) || !actor->target->player || (actor->target->player && !actor->target->player->powers[pw_redshield]))
	{
		P_SetMobjState(actor, S_DISS);
//		P_RemoveMobj(actor);
		return false;
	}
	else if(actor->target->player->powers[pw_redshield])
	{
		P_UnsetThingPosition (actor);
		actor->x = actor->target->x;
		actor->y = actor->target->y;
		actor->z = actor->target->z - (actor->target->info->height - actor->target->height)/3; // Adjust height for height changes
		P_SetThingPosition (actor);
		if(actor->target->player->powers[pw_super] == true || actor->target->player->powers[pw_invulnerability] > 1)
			P_SetMobjState(actor, S_DISS);
	}
	return true;
}
boolean A_YellowLook(mobj_t* actor)
{
	if(!actor)
		return false;

	if(actor->state == &states[S_DISS])
		return false;

	if(!actor->target || (actor->target->health <= 0) || !actor->target->player || (actor->target->player && !actor->target->player->powers[pw_yellowshield]))
	{
		P_SetMobjState(actor, S_DISS);
//		P_RemoveMobj(actor);
		return false;
	}
	else if(actor->target->player->powers[pw_yellowshield])
	{
		P_UnsetThingPosition (actor);
		actor->x = actor->target->x;
		actor->y = actor->target->y;
		actor->z = actor->target->z - (actor->target->info->height - actor->target->height)/3; // Adjust height for height changes
		P_SetThingPosition (actor);
		if(actor->target->player->powers[pw_super] == true || actor->target->player->powers[pw_invulnerability] > 1)
			P_SetMobjState(actor, S_DISS);
	}
	return true;
}
boolean A_BlackLook(mobj_t* actor)
{
	if(!actor)
		return false;

	if(actor->state == &states[S_DISS])
		return false;

	if(!actor->target || (actor->target->health <= 0) || !actor->target->player || (actor->target->player && !actor->target->player->powers[pw_blackshield]))
	{
		P_SetMobjState(actor, S_DISS);
//		P_RemoveMobj(actor);
		return false;
	}
	else if(actor->target->player->powers[pw_blackshield])
	{
		P_UnsetThingPosition (actor);
		actor->x = actor->target->x;
		actor->y = actor->target->y;
		actor->z = actor->target->z - (actor->target->info->height - actor->target->height)/3; // Adjust height for height changes
		P_SetThingPosition (actor);
		if(actor->target->player->powers[pw_super] == true || actor->target->player->powers[pw_invulnerability] > 1)
			P_SetMobjState(actor, S_DISS);
	}
	return true;
}
// end shield positions Tails 06-28-2000

// start GFZ Fish jump Tails 07-03-2000
void A_FishJump(mobj_t* actor)
{
	if((actor->z <= actor->floorz) || (actor->z <= actor->watertop-(64<<FRACBITS)))
	{
		if(actor->z > actor->watertop-(48<<FRACBITS))
		{
			actor->momz = 11<<FRACBITS;
			P_SetMobjState(actor, S_FISH1);
		}
		else
		{
			actor->momz = 15<<FRACBITS;
			P_SetMobjState(actor, S_FISH1);
		}
	}

	//if((actor->state == &states[S_FISH1]) && (actor->momz == 0))
	//P_SetMobjState(actor, S_FISH2);

	if(actor->momz < 0)
	P_SetMobjState(actor, S_FISH3);
}
// end GFZ Fish jump Tails 07-03-2000

// Start level end sign player
void A_SignPlayer(mobj_t* actor)
{
	if (plyr->skin == 1)
		actor->state->nextstate = S_SIGN53;
	else if (plyr->skin == 2)
		actor->state->nextstate = S_SIGN55;
	else
		actor->state->nextstate = S_SIGN54;
}

// Thrown ring thinker/sparkle trail Tails 03-13-2001
void A_ThrownRing(mobj_t* actor)
{
    int         c;
    int         stop;
    player_t*   player;
    sector_t*   sector;

	if(leveltime % 5 == 1)
	{
		if(actor->flags2 & MF2_EXPLOSION)
			P_SpawnMobj(actor->x, actor->y, actor->z, MT_SMOK);
		else if(!(actor->flags2 & MF2_RAILRING))
			P_SpawnMobj(actor->x, actor->y, actor->z, MT_SPARK);
	}

	// spilled rings flicker before disappearing Tails 01-11-2001
	if(leveltime & 1 && actor->fuse > 0 && actor->fuse < 2*TICRATE)
		actor->flags2 |= MF2_DONTDRAW;
	else
		actor->flags2 &= ~MF2_DONTDRAW;

// Updated homing ring special capability
// If you have a yellow shield, all rings thrown
// at you become homing (except rail)!
// Tails 12-14-2003

	// A non-homing ring getting attracted by a
	// magnetic player. If he gets too far away, make
	// sure to stop the attraction!
	if(actor->tracer &&
		actor->tracer->player->powers[pw_yellowshield]
		&& !(actor->flags2 & MF2_HOMING))
	{
		if(P_AproxDistance(P_AproxDistance(actor->tracer->x-actor->x,
							actor->tracer->y-actor->y), actor->tracer->z-actor->z)
							> RING_DIST)
			actor->tracer = NULL;
	}

	if((actor->tracer)
		&& ((actor->flags2 & MF2_HOMING) || actor->target->player->powers[pw_yellowshield])
		&& !(actor->flags2 & MF2_RAILRING)) // Already found someone to follow.
	{
		int temp = actor->threshold;

		actor->threshold = 32000;
		P_HomingAttack(actor, actor->tracer);
		actor->threshold = temp;
		return;
	}

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
            return;

        if (!playeringame[actor->lastlook])
            continue;

        if (c++ == 2)
            return;

        player = &players[actor->lastlook];

        if (player->health <= 0)
            continue;           // dead

		if(player->mo == actor->target)
			continue;

        if (!P_CheckSight (actor, player->mo))
            continue;           // out of sight

		// check distance
		if(actor->flags2 & MF2_RAILRING)
		{
			if(P_AproxDistance(P_AproxDistance(player->mo->x-actor->x,
								player->mo->y-actor->y), player->mo->z-actor->z)
								> RING_DIST/2)
				continue;
		}
		else
		{
			if(P_AproxDistance(P_AproxDistance(player->mo->x-actor->x,
								player->mo->y-actor->y), player->mo->z-actor->z)
								> RING_DIST)
				continue;
		}

		if((actor->flags2 & MF2_HOMING)
			|| player->powers[pw_yellowshield] == true)
			actor->tracer = player->mo;
		return;
    }

    return;
}

// Start some steam stuff Tails 05-29-2001
void A_SetSolidSteam(mobj_t* actor)
{
	actor->flags &= ~MF_NOCLIP;
	actor->flags |= MF_SOLID;
	if(!(P_Random() % 8))
		S_StartSound(actor, actor->info->deathsound); // Hiss!
	else
		S_StartSound(actor, actor->info->painsound);
	actor->momz++;
}
void A_UnsetSolidSteam(mobj_t* actor)
{
	actor->flags &= ~MF_SOLID;
	actor->flags |= MF_NOCLIP;
}
// End some steam stuff Tails 05-29-2001

// Start Jetty-Syn Thinkers Tails 08-18-2001
void A_JetChase (mobj_t*   actor)
{
	fixed_t thefloor;

	if(actor->flags & MF_AMBUSH)
		return;

	if(actor->z >= actor->waterbottom && actor->watertop > actor->floorz
		&& actor->z > actor->watertop - 256*FRACUNIT)
		thefloor = actor->watertop;
	else
		thefloor = actor->floorz;

	if(actor->reactiontime)
		actor->reactiontime--;

	if(P_Random() % 32 == 1)
	{
		actor->momx = actor->momx/2;
		actor->momy = actor->momy/2;
		actor->momz = actor->momz/2;
	}

	// Bounce if too close to floor or ceiling -
	// ideal for Jetty-Syns above you on 3d floors
	if(actor->momz && ((actor->z - (32<<FRACBITS)) < thefloor) && !((thefloor + 32*FRACUNIT + actor->height) > actor->ceilingz))
			actor->momz = -actor->momz;

    // modify target threshold
    if  (actor->threshold)
    {
        if (!actor->target
            || actor->target->health <= 0)
        {
            actor->threshold = 0;
        }
        else
            actor->threshold--;
    }

    // turn towards movement direction if not there yet
	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

    // check for melee attack
    if (actor->type == MT_JETTBOMBER && (actor->z > (actor->floorz + (32<<FRACBITS)))
		&& P_CheckMeleeRange (actor) && !actor->reactiontime
		&& (actor->target->z >= actor->floorz))
    {
        if (actor->info->attacksound)
            S_StartSound (actor, actor->info->attacksound);

        P_SpawnMobj (actor->x, actor->y, actor->z - (32<<FRACBITS), MT_MINE)->target = actor;
		actor->reactiontime = TICRATE; // one second
    }

    if ((multiplayer || netgame)
        && !actor->threshold
        && !P_CheckSight (actor, actor->target) )
    {
        if (P_LookForPlayers(actor,true))
            return;     // got a new target
    }

	// If the player is over 3072 fracunits away, then look for another player
	// Bah, the P_AproxDistance didn't want to work, so I'm doing it my own way. =)
	// Tails 09-02-2001
	if(P_AproxDistance(P_AproxDistance(actor->target->x - actor->x, actor->target->y - actor->y), actor->target->z - actor->z) > 3072<<FRACBITS)
	{
        if (P_LookForPlayers(actor,true))
            return;     // got a new target
	}

    // chase towards player
if(gameskill <= sk_easy)
	P_Thrust(actor, actor->angle, actor->info->speed/6);
else if (gameskill < sk_hard)
	P_Thrust(actor, actor->angle, actor->info->speed/4);
else
	P_Thrust(actor, actor->angle, actor->info->speed/2);

    // must adjust height
	if(gameskill <= sk_medium)
	{
		if (actor->z < (actor->target->z+actor->target->height+(32<<FRACBITS)))
			actor->momz += FRACUNIT/2;
		else
			actor->momz -= FRACUNIT/2;
	}
	else
	{
		if (actor->z < (actor->target->z+actor->target->height+(64<<FRACBITS)))
			actor->momz += FRACUNIT/2;
		else
			actor->momz -= FRACUNIT/2;
	}
}

void A_JetbThink(mobj_t* actor)
{
	sector_t* nextsector;

	fixed_t thefloor;

	if(actor->z >= actor->waterbottom && actor->watertop > actor->floorz
		&& actor->z > actor->watertop - 256*FRACUNIT)
		thefloor = actor->watertop;
	else
		thefloor = actor->floorz;

	if(actor->target)
		A_JetChase (actor);
	else if(((actor->z - (32<<FRACBITS)) < thefloor) && !((thefloor + (32<<FRACBITS) + actor->height) > actor->ceilingz))
			actor->z = thefloor+(32<<FRACBITS);

	if (!actor->target
        || !(actor->target->flags&MF_SHOOTABLE))
    {
        // look for a new target
        if (P_LookForPlayers(actor,true))
            return;     // got a new target

        P_SetMobjState (actor, actor->info->spawnstate);
        return;
    }

	nextsector = R_PointInSubsector(actor->x + actor->momx, actor->y + actor->momy)->sector;

	// Move downwards or upwards to go through a passageway.
	if(nextsector->ceilingheight < actor->height)
		actor->momz -= 5*FRACUNIT;
	else if(nextsector->floorheight > actor->z)
		actor->momz += 5*FRACUNIT;
}

void A_JetgShoot(mobj_t* actor)
{
	if(!actor->target)
		return;

	if(actor->reactiontime)
		return;

	if(P_AproxDistance(P_AproxDistance(actor->target->x - actor->x, actor->target->y - actor->y), actor->target->z - actor->z) > actor->info->painchance*FRACUNIT)
		return;

	A_FaceTarget(actor);
    P_SpawnMissile (actor, actor->target, actor->info->raisestate);

	if(gameskill <= sk_medium)
		actor->reactiontime = actor->info->reactiontime*TICRATE*2;
	else
		actor->reactiontime = actor->info->reactiontime*TICRATE;
}

void A_JetgThink(mobj_t* actor)
{
	sector_t* nextsector;

	fixed_t thefloor;

	if(actor->z >= actor->waterbottom && actor->watertop > actor->floorz
		&& actor->z > actor->watertop - 256*FRACUNIT)
		thefloor = actor->watertop;
	else
		thefloor = actor->floorz;

	if(actor->target)
	{
		if(P_Random() <= 32 && !actor->reactiontime)
			P_SetMobjState(actor, actor->info->missilestate);
		else
			A_JetChase (actor);
	}
	else if(actor->z - (32<<FRACBITS) < thefloor && !(thefloor + (32<<FRACBITS) + actor->height > actor->ceilingz))
			actor->z = thefloor+(32<<FRACBITS);

	if (!actor->target
        || !(actor->target->flags&MF_SHOOTABLE))
    {
        // look for a new target
        if (P_LookForPlayers(actor,true))
            return;     // got a new target

        P_SetMobjState (actor, actor->info->spawnstate);
        return;
    }

	nextsector = R_PointInSubsector(actor->x + actor->momx, actor->y + actor->momy)->sector;

	// Move downwards or upwards to go through a passageway.
	if(nextsector->ceilingheight < actor->height)
		actor->momz -= 5*FRACUNIT;
	else if(nextsector->floorheight > actor->z)
		actor->momz += 5*FRACUNIT;
}

void A_MouseThink(mobj_t* actor)
{
	if(actor->reactiontime)
		actor->reactiontime--;

	if(actor->z == actor->floorz && !actor->reactiontime)
	{
		if(P_Random() & 1)
			actor->angle += ANG90;
		else
			actor->angle -= ANG90;

		P_InstaThrust(actor, actor->angle, actor->info->speed);
		actor->reactiontime = TICRATE/5;
	}
}

void A_DetonChase (mobj_t*   actor)
{
/*
Distance between two points in 3D space:

dX = X2-X1
dY = Y2-Y1
dZ = Z2-Z1

Distance = Sqrt( dX^2 + dY^2 + dZ^2 )

	actor->momx = abs(actor->x - actor->target->x)/5;
	actor->momy = abs(actor->y - actor->target->y)/5;
	actor->momz = abs(actor->z - actor->target->z)/5;
*/

    // modify target threshold
        if (!actor->target
            || actor->target->health <= 0)
        {
            actor->threshold = 0;
        }

    if (!actor->target
        || !(actor->target->flags&MF_SHOOTABLE))
    {
        // look for a new target
        if (P_LookForPlayers(actor,true))
            return;     // got a new target

        P_SetMobjState (actor, actor->info->spawnstate);
        return;
    }

    // Face movement direction if not doing so
	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

	if (multiplayer
        && !actor->threshold)
    {
        if (P_LookForPlayers(actor,true))
            return;     // got a new target
    }

    // check for melee attack

    if (P_CheckMeleeRange (actor))
	{
		P_ExplodeMissile(actor);
		P_RadiusAttack(actor, actor, 96);
	}

    // chase towards player

//	if(actor->momx < 10*FRACUNIT && actor->momy < 10*FRACUNIT)

	if(P_AproxDistance(P_AproxDistance(actor->target->x-actor->x, actor->target->y-actor->y), actor->target->z-actor->z)
		> (actor->info->painchance << FRACBITS))
	{
		actor->target = NULL; // Too far away
		return;
	}

	if(actor->reactiontime == 0)
	{
		actor->reactiontime = actor->info->reactiontime;

		return;
	}

	if(actor->reactiontime > 1)
	{
		actor->reactiontime--;
		return;
	}

	if(actor->reactiontime > 0)
	{
		actor->reactiontime = -42;

		if(actor->info->seesound)
			S_StartSound(actor, actor->info->seesound);
	}

	if(actor->reactiontime == -42)
	{
		actor->reactiontime = -42;
		actor->tracer = actor->target;
		P_HomingAttack(actor, actor->target);
	}
}

// Fake little Super Sonic cape Tails 11-12-2001
void A_CapeChase(mobj_t* actor)
{
	fixed_t platx;
	fixed_t platy;

	if(actor->state == &states[S_DISS])
		return;
			
	if(!actor->target || (actor->target->health <= 0) || !actor->target->player || (actor->target->player && !actor->target->player->powers[pw_super]))
	{
		P_SetMobjState(actor, S_DISS);
		return;
	}
	else if(actor->target->player->powers[pw_super])
	{
		platx = P_ReturnThrustX(actor->target, actor->target->angle, -FRACUNIT*4);
		platy = P_ReturnThrustY(actor->target, actor->target->angle, -FRACUNIT*4);
		P_UnsetThingPosition (actor);
		actor->x = actor->target->x + platx;
		actor->y = actor->target->y + platy;
		actor->z = actor->target->z + actor->target->height/2;
		actor->angle = actor->target->angle;
		P_SetThingPosition (actor);
	}
}

// Spike ball rotation! Tails 11-15-2001
void A_RotateSpikeBall(mobj_t* actor)
{
	int radius;
	const double deg2rad = 0.017453293;

	if(actor->type == MT_SPECIALSPIKEBALL)
		return;

	if(!actor->target) // This should NEVER happen.
	{
		CONS_Printf("Error: Spikeball has no target\n");
		P_SetMobjState(actor, S_DISS);
		return;
	}

	radius = 12*actor->info->speed;

	actor->angle += actor->info->speed/FRACUNIT;
	P_UnsetThingPosition(actor);
	actor->x = actor->target->x + cos(actor->angle * deg2rad) * radius;
	actor->y = actor->target->y + sin(actor->angle * deg2rad) * radius;
	actor->z = actor->target->z + actor->target->height/2;
	P_SetThingPosition(actor);

	if(actor->angle > 65535 << FRACBITS)
		actor->angle -= 65535 << FRACBITS;
}

// Snow ball for Snow Buster Tails 12-12-2001
void A_SnowBall(mobj_t* actor)
{
	P_InstaThrust(actor, actor->angle, actor->info->speed);
	if(!actor->fuse)
		actor->fuse = 10*TICRATE;
}

void A_CrawlaCommanderThink(mobj_t* actor)
{
	fixed_t dist;
	sector_t* nextsector;
	fixed_t thefloor;

	if(actor->z >= actor->waterbottom && actor->watertop > actor->floorz
		&& actor->z > actor->watertop - 256*FRACUNIT)
		thefloor = actor->watertop;
	else
		thefloor = actor->floorz;

	if(actor->fuse & 1)
		actor->flags2 |= MF2_DONTDRAW;
	else
		actor->flags2 &= ~MF2_DONTDRAW;

	if(actor->reactiontime > 0)
		actor->reactiontime--;

	if(actor->fuse < 2)
	{
		actor->fuse = 0;
		actor->flags2 &= ~MF2_FRET;
	}

	// Hover mode
	if(actor->health > 1 || actor->fuse)
	{
	    if(actor->z < thefloor + (16*FRACUNIT))
			actor->momz += FRACUNIT;
		else if(actor->z < thefloor + (32*FRACUNIT))
			actor->momz += FRACUNIT/2;
		else
			actor->momz += 16;
	}

	if (!actor->target
        || !(actor->target->flags&MF_SHOOTABLE))
    {

        // look for a new target
        if (P_LookForPlayers(actor,true))
            return;     // got a new target

		if(actor->state != &states[actor->info->spawnstate])
	        P_SetMobjState (actor, actor->info->spawnstate);
        return;
    }

	dist = P_AproxDistance(actor->x - actor->target->x, actor->y - actor->target->y);

	if(actor->target->player && actor->health > 1)
	{
		if(dist < 128*FRACUNIT
			&& (actor->target->player->mfjumped || actor->target->player->mfspinning))
		{
			// Auugh! He's trying to kill you! Strafe! STRAAAAFFEEE!!
			if(actor->target->momx || actor->target->momy)
			{
				P_InstaThrust(actor, -actor->angle, 20*FRACUNIT);
			}
			return;
		}
	}
/*
	if(actor->health < 2 && P_Random() < 2)
	{
		P_SpawnMissile (actor, actor->target, MT_JETTBULLET);
	}
*/
	// Face the player
	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

	if(actor->threshold && dist > 256*FRACUNIT)
		actor->momx = actor->momy = 0;

	if(actor->reactiontime && actor->reactiontime <= 2*TICRATE && dist > actor->target->radius - FRACUNIT)
	{
		actor->threshold = 0;

		// Roam around, somewhat in the player's direction.
		actor->angle += (P_Random()<<10);
		actor->angle -= (P_Random()<<10);

		if(actor->health > 1)
			P_InstaThrust(actor, actor->angle, 10*FRACUNIT);
	}
	else if (!actor->reactiontime)
	{
		if(actor->health > 1) // Hover Mode
		{
			if(dist < 512*FRACUNIT)
			{
				actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
				P_InstaThrust(actor, actor->angle, 60*FRACUNIT);
				actor->threshold = 1;
			}
		}
		actor->reactiontime = 2*TICRATE + P_Random()/2;
	}

	if(actor->health == 1)
		P_Thrust(actor, actor->angle, 1);

	// Pogo Mode
	if(!actor->fuse && actor->health == 1 && actor->z <= actor->floorz)
	{
		if(dist < 256*FRACUNIT)
		{
			actor->momz = 15*FRACUNIT;
			actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
			P_InstaThrust(actor, actor->angle, 2*FRACUNIT);
			// pogo on player
		}
		else
		{
			actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y) + (P_Random() & 1 ? -P_Random() : +P_Random());
			P_InstaThrust(actor, actor->angle, 10*FRACUNIT);
			actor->momz = 15*FRACUNIT; // Bounce up in air
		}
	}

//	if(actor->momx || actor->momy)
		nextsector = R_PointInSubsector(actor->x + actor->momx, actor->y + actor->momy)->sector;
//	else
//		nextsector = R_PointInSubsector(actor->x + P_ReturnThrustX(actor, actor->angle, 5*FRACUNIT), actor->y + P_ReturnThrustX(actor, actor->angle, 5*FRACUNIT))->sector;

	// Move downwards or upwards to go through a passageway.
	if(nextsector->floorheight > actor->z && nextsector->floorheight - actor->z < 128*FRACUNIT)
		actor->momz += (nextsector->floorheight - actor->z)/4;
}

// Spurt out rings in many directions Tails 07-12-2002
void A_RingExplode(mobj_t* actor)
{
	int       i;
	mobj_t*   mo;

	for(i = 0; i<32; i++)
	{
		mo = P_SpawnMobj(actor->x,
			             actor->y,
				         actor->z,
					     MT_REDRING);

		if(!mo) // Safety check
			return;

		mo->target = actor->target; // Transfer target so player gets the points

		// Make rings spill out around the player in 16 directions like SA, but spill like Sonic 2.
		// Technically a non-SA way of spilling rings. They just so happen to be a little similar.
		// Tails 05-11-2001
		if(i>15)
		{
			mo->momx = sin(i*22.5) * 60 * FRACUNIT;
			mo->momy = cos(i*22.5) * 60 * FRACUNIT;
			mo->momz = 60*FRACUNIT;
		}
		else
		{
			mo->momx = sin(i*22.5) * 60 * FRACUNIT;
			mo->momy = cos(i*22.5) * 60 * FRACUNIT;
		}
		mo->flags2 |= MF2_DEBRIS;

		mo->fuse = TICRATE/(OLDTICRATE/5);
	}
	
	mo = P_SpawnMobj(actor->x,
		             actor->y,
			         actor->z,
				     MT_REDRING);

	if(!mo) // Safety check
		return;

	mo->target = actor->target;
	mo->momz = 60*FRACUNIT;
	mo->flags2 |= MF2_DEBRIS;
	mo->fuse = TICRATE/(OLDTICRATE/5);

	mo = P_SpawnMobj(actor->x,
		             actor->y,
			         actor->z,
				     MT_REDRING);

	if(!mo) // Safety check
		return;

	mo->target = actor->target;
	mo->momz = -60*FRACUNIT;
	mo->flags2 |= MF2_DEBRIS;
	mo->fuse = TICRATE/(OLDTICRATE/5);

	return;
}

// Mix up all the player positions
void A_MixUp(mobj_t* actor)
{
	if(netgame || multiplayer) // Netgames ONLY.
	{
		int i, numplayers, random;

		numplayers = 0;

		// Count the number of players in the game
		// and grab their xyz coords
		for(i=0;i<MAXPLAYERS;i++)
		{
			if(playeringame[i])
			{
				if(players[i].mo && players[i].playerstate == PST_LIVE && !players[i].exiting)
				{
					numplayers++;
				}
			}
		}

		if(numplayers <= 1) // Not enough players to mix up.
			return;
		else if(numplayers == 2) // Special case -- simple swap
		{
			fixed_t x,y,z;
			angle_t angle;
			int one, two;
			one = -1;
			for(i=0; i<MAXPLAYERS; i++)
			{
				if(playeringame[i])
				{
					if(players[i].mo && players[i].playerstate == PST_LIVE && !players[i].exiting)
					{
						if(one == -1)
							one = i;
						else
						{
							two = i;
							break;
						}
					}
				}
			}
			x = players[one].mo->x;
			y = players[one].mo->y;
			z = players[one].mo->z;
			angle = players[one].mo->angle;

			P_Teleport(players[one].mo, players[two].mo->x, players[two].mo->y, players[two].mo->angle);
			players[one].mo->z = players[two].mo->z;

			P_Teleport(players[two].mo, x, y, angle);
			players[two].mo->z = z;

			players[one].mo->momx = players[one].mo->momy = players[one].mo->momz
				= players[two].mo->momx = players[two].mo->momy = players[two].mo->momz = 1;
			players[one].bonuscount += 10; // Flash the palette.
			players[two].bonuscount += 10; // Flash the palette.
		}
		else
		{
			fixed_t position[MAXPLAYERS][3];
			angle_t anglepos[MAXPLAYERS];
			int pindex[MAXPLAYERS];
			boolean picked[MAXPLAYERS];
			int counter;
			counter = 0;

			for(i=0; i<MAXPLAYERS; i++)
			{
				position[i][0] = -1;
				position[i][1] = -1;
				position[i][2] = -1;
				anglepos[i] = -1;
				pindex[i] = -1;
				picked[i] = false;
			}

			for(i=0; i<MAXPLAYERS; i++)
			{
				if(playeringame[i] && players[i].playerstate == PST_LIVE
					&& players[i].mo && !players[i].exiting)
				{
					position[counter][0] = players[i].mo->x;
					position[counter][1] = players[i].mo->y;
					position[counter][2] = players[i].mo->z;
					pindex[counter] = i;
					anglepos[counter] = players[i].mo->angle;
					players[i].mo->momx = players[i].mo->momy = players[i].mo->momz = 1;
					counter++;
				}
			}

			counter = 0;

			// Mix them up!
			while (true)
			{
				if(counter > 255) // fail-safe to avoid endless loop
					break;
				random = P_Random();
				if(random % numplayers != 0) // Make sure it's not a useless mix
					break;
				counter++;
			}

			// Scramble!
			random = random % numplayers; // I love modular arithmetic, don't you?
			counter = random;

			for(i=0; i<MAXPLAYERS; i++)
			{
				if(playeringame[i] && players[i].playerstate == PST_LIVE
					&& players[i].mo && !players[i].exiting)
				{
					random = 0;
					while((pindex[counter] == i || picked[counter] == true)
						&& random < 255) // Failsafe
					{
						counter = P_Random() % numplayers;
						random++;
					}

					P_Teleport(players[i].mo, position[counter][0], position[counter][1], anglepos[counter]);
					players[i].mo->z = position[counter][2];

					players[i].mo->momx = players[i].mo->momy = players[i].mo->momz = 1;
					players[i].bonuscount += 10; // Flash the palette.
					picked[counter] = true;
				}
			}
		}
		// Play the 'bowrwoosh!' sound
		S_StartSound(0, sfx_mixup);
	}
}

void A_PumaJump(mobj_t* actor)
{
	if((actor->z <= actor->floorz) || (actor->z <= actor->watertop-(64<<FRACBITS)))
	{
		if(actor->z > actor->watertop-(48<<FRACBITS))
		{
			actor->momz = 11<<FRACBITS;
//			P_SetMobjState(actor, S_FISH1);
		}
		else
		{
			actor->momz = 15<<FRACBITS;
//			P_SetMobjState(actor, S_FISH1);
		}
	}

	if(actor->momz < 0
		&& (actor->state != &states[S_PUMA4] || actor->state != &states[S_PUMA5] || actor->state != &states[S_PUMA6]))
		P_SetMobjStateNF(actor, S_PUMA4);
	else if (actor->state != &states[S_PUMA1] || actor->state != &states[S_PUMA2] || actor->state != &states[S_PUMA3])
		P_SetMobjStateNF(actor, S_PUMA1);
}

//
// A_Boss1Chase
// it tries to close as fast as possible
//
void A_Boss1Chase (mobj_t*   actor)
{
    int         delta;

	if(actor->reactiontime)
		actor->reactiontime--;

    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
        actor->angle &= (7<<29);
        delta = actor->angle - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90/2;
        else if (delta < 0)
            actor->angle += ANG90/2;
    }

    // do not attack twice in a row
    if (actor->flags2 & MF2_JUSTATTACKED)
    {
        actor->flags2 &= ~MF2_JUSTATTACKED;
        P_NewChaseDir (actor);
        return;
    }

    if (actor->movecount)
    {
        goto nomissile;
    }

    if (!P_CheckMissileRange (actor))
        goto nomissile;

	if(actor->reactiontime <= 0)
	{
		if(actor->health > actor->info->damage)
		{
			if(P_Random() & 1)
				P_SetMobjState(actor, actor->info->missilestate);
			else
				P_SetMobjState(actor, actor->info->meleestate);
		}
		else
			P_SetMobjState(actor, actor->info->raisestate);

		actor->flags2 |= MF2_JUSTATTACKED;
		actor->reactiontime = 2*TICRATE;
		return;
	}

    // ?
  nomissile:
    // possibly choose another target
    if (multiplayer
        && P_Random() < 2 )
    {
        if (P_LookForPlayers(actor,true))
            return;     // got a new target
    }

    // chase towards player
    if (--actor->movecount<0
        || !P_Move (actor))
    {
        P_NewChaseDir (actor);
    }
}

mobj_t* P_GetClosestAxis(mobj_t* source);
// A_Boss2Chase
//
// Really doesn't 'chase', rather he goes in a circle.
//
// Tails 11-02-2002
void A_Boss2Chase (mobj_t*   actor)
{
	int radius;
	const double deg2rad = 0.017453293;
	boolean reverse = false;

	if(actor->health <= 0)
		return;

	// When reactiontime hits zero, he will go the other way
	if(actor->reactiontime)
		actor->reactiontime--;

	if(actor->reactiontime <= 0)
	{
		reverse = true;
		actor->reactiontime = 2*TICRATE + P_Random();
	}

	actor->target = P_GetClosestAxis(actor);

	if(!actor->target) // This should NEVER happen.
	{
		CONS_Printf("Error: Boss2 has no target!\n");
		A_BossDeath(actor);
		return;
	}

	if(reverse)
		actor->watertop = -actor->watertop;

	radius = 384*FRACUNIT;

	actor->target->angle += actor->watertop >> FRACBITS;
	P_UnsetThingPosition(actor);
	actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x + cos(actor->target->angle * deg2rad) * radius, actor->target->y + sin(actor->target->angle * deg2rad) * radius);
	actor->x = actor->target->x + cos(actor->target->angle * deg2rad) * radius;
	actor->y = actor->target->y + sin(actor->target->angle * deg2rad) * radius;
	actor->z = actor->target->z + 64*FRACUNIT;
	P_SetThingPosition(actor);

	if(actor->angle > ANGLE_MAX)
		actor->angle -= ANGLE_MAX;

	// Spray goo once every second
	if(leveltime % 7 == 1)
	{
		mobj_t* goop;
		// actor->movedir is used to determine the last
		// direction goo was sprayed in. There are 8 possible
		// directions to spray. (45-degree increments)

		actor->movedir++;
		actor->movedir %= 8;

		goop = P_SpawnMobj(actor->x,actor->y,actor->z+actor->height+56*FRACUNIT, actor->info->painchance);
		goop->momx = sin(actor->movedir*45) * 3 * FRACUNIT;
		goop->momy = cos(actor->movedir*45) * 3 * FRACUNIT;
		goop->momz = 4*FRACUNIT;
		goop->fuse = 30*TICRATE+P_Random();
		S_StartSound(actor, actor->info->attacksound);

		if(P_Random() & 1)
		{
			goop->momx *= 2;
			goop->momy *= 2;
		}
		else if(P_Random() > 128)
		{
			goop->momx *= 3;
			goop->momy *= 3;
		}

		actor->flags2 |= MF2_JUSTATTACKED;
	}
}
// Pogo part of Boss 2 AI
// Tails 11-02-2002
void A_Boss2Pogo (mobj_t*   actor)
{
	if(actor->z <= actor->floorz + 8*FRACUNIT && actor->momz <= 0)
	{
		P_SetMobjState(actor, actor->info->raisestate);
		// Pogo Mode
	}
	else if(actor->momz < 0 && actor->reactiontime)
	{
		mobj_t* goop;
		int i;
		// spray in all 8 directions!
		for(i=0; i<8; i++)
		{
			actor->movedir++;
			actor->movedir %= 8;

			goop = P_SpawnMobj(actor->x,actor->y,actor->z+actor->height+56*FRACUNIT, actor->info->painchance);
			goop->momx = sin(actor->movedir*45) * 3 * FRACUNIT;
			goop->momy = cos(actor->movedir*45) * 3 * FRACUNIT;
			goop->momz = 4*FRACUNIT;

			if(cv_gametype.value == GT_CHAOS)
				goop->fuse = 15*TICRATE;
			else
				goop->fuse = 30*TICRATE+P_Random();
		}
		actor->reactiontime = 0;
		S_StartSound(actor, actor->info->attacksound);
		actor->flags2 |= MF2_JUSTATTACKED;
	}
}

// Special function for Boss 2 so you can't just sit and destroy him.
void A_Invinciblerize(mobj_t* actor)
{
	A_Pain(actor);
	actor->reactiontime = 1;
	actor->movecount = TICRATE;
}

void A_DeInvinciblerize(mobj_t* actor)
{
	actor->movecount = actor->state->tics;
}

void A_Boss2PogoSFX(mobj_t* actor)
{
	// Boing!
	if(P_AproxDistance(actor->x-actor->target->x, actor->y-actor->target->y) < 256*FRACUNIT)
	{
		actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);
		P_InstaThrust(actor, actor->angle, actor->info->speed/2);
		// pogo on player
	}
	else
	{
		actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y) + (P_Random() & 1 ? -P_Random() : +P_Random());
		P_InstaThrust(actor, actor->angle, FixedMul(actor->info->speed,2.5*FRACUNIT));
	}
	S_StartSound(actor, actor->info->activesound);
	actor->momz = 12*FRACUNIT; // Bounce up in air
	actor->reactiontime = 1;
}

void A_EggmanBox (mobj_t* actor) // Tails 12-19-2002
{
	if(!actor->target || !actor->target->player)
	{
		CONS_Printf("ERROR: Powerup has no target!\n");
		return;
	}

	P_DamageMobj(actor->target, actor, actor, 1); // Ow!
}

boolean P_SupermanLook4Players ( mobj_t*       actor);

void A_TurretFire(mobj_t* actor)
{
	int count = 0;

	while(P_SupermanLook4Players(actor) && count < MAXPLAYERS)
	{
		if(P_AproxDistance(actor->x - actor->target->x, actor->y - actor->target->y) < 2048*FRACUNIT)
		{
			actor->flags2 |= MF2_FIRING;
			break;
		}

		count++;
	}
}

void A_TurretStop(mobj_t* actor)
{
	actor->flags2 &= ~MF2_FIRING;

	if(actor->target)
		S_StartSound(actor, actor->info->activesound);
}

//#include "p_henemy.c"

void A_SparkFollow(mobj_t* actor)
{
	int radius;
	const double deg2rad = 0.017453293;

	if(actor->state == &states[S_DISS])
		return;
			
	if(!actor->target || (actor->target->health <= 0) || !actor->target->player || (actor->target->player && !actor->target->player->powers[pw_super]))
	{
		P_SetMobjState(actor, S_DISS);
		return;
	}

	radius = actor->info->speed;

	actor->angle += actor->info->damage;
	P_UnsetThingPosition(actor);
	actor->x = actor->target->x + cos(actor->angle * deg2rad) * radius;
	actor->y = actor->target->y + sin(actor->angle * deg2rad) * radius;
	actor->z = actor->target->z + actor->target->height/3 - actor->height;
	P_SetThingPosition(actor);

	if(actor->angle > 65535 << FRACBITS)
		actor->angle -= 65535 << FRACBITS;
}