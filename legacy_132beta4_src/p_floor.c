// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_floor.c,v 1.12 2001/03/30 17:12:50 bpereira Exp $
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
// $Log: p_floor.c,v $
// Revision 1.12  2001/03/30 17:12:50  bpereira
// no message
//
// Revision 1.11  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.10  2000/11/02 17:50:07  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.9  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.8  2000/09/28 20:57:16  bpereira
// no message
//
// Revision 1.7  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.6  2000/05/23 15:22:34  stroggonmeth
// Not much. A graphic bug fixed.
//
// Revision 1.5  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.4  2000/04/08 17:29:24  stroggonmeth
// no message
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
//      Floor animation: raising stairs.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "doomstat.h"
#include "p_local.h"
#include "r_state.h"
#include "s_sound.h"
#include "z_zone.h"
#include "g_game.h"


// ==========================================================================
//                              FLOORS
// ==========================================================================

//
// Move a plane (floor or ceiling) and check for crushing
//
//SoM: I had to copy the entire function from Boom because it was causing errors.
result_e T_MovePlane
( sector_t*     sector,
  fixed_t       speed,
  fixed_t       dest,
  boolean       crush,
  int           floorOrCeiling,
  int           direction )
{
  boolean       flag;
  fixed_t       lastpos;     
  fixed_t       destheight; //jff 02/04/98 used to keep floors/ceilings
                            // from moving thru each other

  switch(floorOrCeiling)
  {
    case 0:
      // Moving a floor
      switch(direction)
      {
        case -1:
          //SoM: 3/20/2000: Make splash when platform floor hits water
//          if(boomsupport && sector->heightsec != -1 && sector->altheightsec == 1)
//          {
//            if((sector->floorheight - speed) < sectors[sector->heightsec].floorheight
//               && sector->floorheight > sectors[sector->heightsec].floorheight)
//              S_StartSound((mobj_t *)&sector->soundorg, sfx_gloop);
//          }
          // Moving a floor down
          if (sector->floorheight - speed < dest)
          {
            lastpos = sector->floorheight;
            sector->floorheight = dest;
            flag = P_CheckSector(sector,crush);
            if (flag == true && sector->numattached)                   
            {
              sector->floorheight =lastpos;
              P_CheckSector(sector,crush);
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->floorheight;
            sector->floorheight -= speed;
            flag = P_CheckSector(sector,crush);
            if(flag == true && sector->numattached)
            {
              sector->floorheight = lastpos;
              P_CheckSector(sector, crush);
              return crushed;
            }
          }
          break;
                                                
        case 1:
          // Moving a floor up
          // keep floor from moving thru ceilings
          //SoM: 3/20/2000: Make splash when platform floor hits water
/*          if(boomsupport && sector->heightsec != -1 && sector->altheightsec == 1)
          {
            if((sector->floorheight + speed) > sectors[sector->heightsec].floorheight
               && sector->floorheight < sectors[sector->heightsec].floorheight)
              S_StartSound((mobj_t *)&sector->soundorg, sfx_gloop);
          }*/
          destheight = (!boomsupport || dest<sector->ceilingheight)?
                          dest : sector->ceilingheight;
          if (sector->floorheight + speed > destheight)
          {
            lastpos = sector->floorheight;
            sector->floorheight = destheight;
            flag = P_CheckSector(sector,crush);
            if (flag == true)
            {
              sector->floorheight = lastpos;
              P_CheckSector(sector,crush);
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->floorheight;
            sector->floorheight += speed;
            flag = P_CheckSector(sector,crush);
            if (flag == true)
            {
              if (!boomsupport)
              {
                if (crush == true)
                  return crushed;
              }
              sector->floorheight = lastpos;
              P_CheckSector(sector,crush);
              return crushed;
            }
          }
          break;
      }
      break;
                                                                        
    case 1:
      // moving a ceiling
      switch(direction)
      {
        case -1:
/*          if(boomsupport && sector->heightsec != -1 && sector->altheightsec == 1)
          {
            if((sector->ceilingheight - speed) < sectors[sector->heightsec].floorheight
               && sector->ceilingheight > sectors[sector->heightsec].floorheight)
              S_StartSound((mobj_t *)&sector->soundorg, sfx_gloop);
          }*/
          // moving a ceiling down
          // keep ceiling from moving thru floors
          destheight = (!boomsupport || dest>sector->floorheight)?
                          dest : sector->floorheight;
          if (sector->ceilingheight - speed < destheight)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = destheight;
            flag = P_CheckSector(sector,crush);

            if (flag == true)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);
            }
            return pastdest;
          }
          else
          {
            // crushing is possible
            lastpos = sector->ceilingheight;
            sector->ceilingheight -= speed;
            flag = P_CheckSector(sector,crush);

            if (flag == true)
            {
              if (crush == true)
                return crushed;
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);
              return crushed;
            }
          }
          break;
                                                
        case 1:
/*          if(boomsupport && sector->heightsec != -1 && sector->altheightsec == 1)
          {
            if((sector->ceilingheight + speed) > sectors[sector->heightsec].floorheight
               && sector->ceilingheight < sectors[sector->heightsec].floorheight)
              S_StartSound((mobj_t *)&sector->soundorg, sfx_gloop);
          }*/
          // moving a ceiling up
          if (sector->ceilingheight + speed > dest)
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight = dest;
            flag = P_CheckSector(sector,crush);
            if (flag == true && sector->numattached)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);
            }
            return pastdest;
          }
          else
          {
            lastpos = sector->ceilingheight;
            sector->ceilingheight += speed;
            flag = P_CheckSector(sector,crush);
            if (flag == true && sector->numattached)
            {
              sector->ceilingheight = lastpos;
              P_CheckSector(sector,crush);
              return crushed;
            }
          }
          break;
      }
      break;
    }
    return ok;
}


//
// MOVE A FLOOR TO IT'S DESTINATION (UP OR DOWN)
//
void T_MoveFloor(floormove_t* floor)
{
    result_e    res = 0;
	boolean dontupdate = false;

    res = T_MovePlane(floor->sector,
                      floor->speed,
                      floor->floordestheight,
                      floor->crush,0,floor->direction);
/*
    if (!(leveltime % (8*NEWTICRATERATIO)))
        S_StartSound((mobj_t *)&floor->sector->soundorg,
                     ceilmovesound);
*/ // Tails
    if (res == pastdest)
    {
        //floor->sector->specialdata = NULL;
        if (floor->direction == 1)
        {
            switch(floor->type)
            {
              case donutRaise:
                floor->sector->special = floor->newspecial;
                floor->sector->floorpic = floor->texture;
                break;
              case genFloorChgT: //SoM: 3/6/2000: Add support for General types
              case genFloorChg0:
                floor->sector->special = floor->newspecial;
                //SoM: 3/6/2000: this records the old special of the sector
                floor->sector->oldspecial = floor->oldspecial;
                // Don't break.
              case genFloorChg:
			  case instantMoveFloorByFrontSector: // Graue 12-12-2003
                floor->sector->floorpic = floor->texture;
                break;
              default:
                break;
            }
        }
        else if (floor->direction == -1)
        {
            switch(floor->type)
            {
              case lowerAndChange:
                floor->sector->special = floor->newspecial;
                // SoM: 3/6/2000: Store old special type
                floor->sector->oldspecial = floor->oldspecial;
                floor->sector->floorpic = floor->texture;
                break;
              case genFloorChgT:
              case genFloorChg0:
                floor->sector->special = floor->newspecial;
                floor->sector->oldspecial = floor->oldspecial;
                // Don't break
              case genFloorChg:
			  case instantMoveFloorByFrontSector: // Graue 12-12-2003
                floor->sector->floorpic = floor->texture;
                break;
              default:
                break;
            }
        }

        floor->sector->floordata = NULL; // Clear up the thinker so others can use it
        P_RemoveThinker(&floor->thinker);
		floor->sector->floorspeed = 0;
		dontupdate = true;

        // SoM: This code locks out stair steps while generic, retriggerable generic stairs
        // are building.
      
        if (floor->sector->stairlock==-2) // if this sector is stairlocked
        {
          sector_t *sec = floor->sector;
          sec->stairlock=-1;              // thinker done, promote lock to -1

          while (sec->prevsec!=-1 && sectors[sec->prevsec].stairlock!=-2)
            sec = &sectors[sec->prevsec]; // search for a non-done thinker
          if (sec->prevsec==-1)           // if all thinkers previous are done
          {
            sec = floor->sector;          // search forward
            while (sec->nextsec!=-1 && sectors[sec->nextsec].stairlock!=-2) 
              sec = &sectors[sec->nextsec];
            if (sec->nextsec==-1)         // if all thinkers ahead are done too
            {
              while (sec->prevsec!=-1)    // clear all locks
              {
                sec->stairlock = 0;
                sec = &sectors[sec->prevsec];
              }
              sec->stairlock = 0;
            }
          }
        }

//        S_StartSound((mobj_t *)&floor->sector->soundorg, sfx_pstop);
    }
	if(!dontupdate)
		floor->sector->floorspeed = floor->speed*floor->direction;
	else
		floor->sector->floorspeed = 0;
}


// SoM: 3/6/2000: Lots'o'copied code here.. Elevators.
//
// T_MoveElevator()
//
// Move an elevator to it's destination (up or down)
// Called once per tick for each moving floor.
//
// Passed an elevator_t structure that contains all pertinent info about the
// move. See P_SPEC.H for fields.
// No return.
//
// SoM: 3/6/2000: The function moves the plane differently based on direction, so if it's 
// traveling really fast, the floor and ceiling won't hit each other and stop the lift.
void T_MoveElevator(elevator_t* elevator)
{
  result_e      res = 0;
  boolean dontupdate = false;

  if (elevator->direction<0)      // moving down
  {

	  if(elevator->type == elevateContinuous)
	  {
		// Slow down when reaching destination Tails 12-06-2000
		if(abs(elevator->sector->floorheight - elevator->floorwasheight) < abs(elevator->sector->floorheight - elevator->floordestheight))
			elevator->speed = abs(elevator->sector->floorheight - elevator->floorwasheight)/25 + FRACUNIT/4;
		else
			elevator->speed = abs(elevator->sector->floorheight - elevator->floordestheight)/25 + FRACUNIT/4;

		if(elevator->origspeed)
			elevator->speed *= elevator->origspeed/(ELEVATORSPEED/2);

		if(elevator->origspeed)
		{
			if(elevator->speed > elevator->origspeed)
				elevator->speed = (elevator->origspeed);
			if(elevator->speed < 1)
				elevator->speed = 1/NEWTICRATERATIO;
		}
		else
		{
			if(elevator->speed > ((3*FRACUNIT)/NEWTICRATERATIO))
				elevator->speed = ((3*FRACUNIT)/NEWTICRATERATIO);
			if(elevator->speed < 1)
				elevator->speed = 1/NEWTICRATERATIO;
		}
	  }

		res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
		(
		  elevator->sector,
		  elevator->speed,
		  elevator->ceilingdestheight,
		  0,
		  1,                          // move floor
		  elevator->direction
		);

		if (res==ok || res==pastdest) // jff 4/7/98 don't move ceil if blocked
		  T_MovePlane
		  (
			elevator->sector,
			elevator->speed,
			elevator->floordestheight,
			0,
			0,                        // move ceiling
			elevator->direction
		  );
	  }
  else // up
  {

	  if(elevator->type == elevateContinuous)
	  {
		// Slow down when reaching destination Tails 12-06-2000
		if(abs(elevator->sector->ceilingheight - elevator->ceilingwasheight) < abs(elevator->sector->ceilingheight - elevator->ceilingdestheight))
			elevator->speed = abs(elevator->sector->ceilingheight - elevator->ceilingwasheight)/25 + FRACUNIT/4;
		else
			elevator->speed = abs(elevator->sector->ceilingheight - elevator->ceilingdestheight)/25 + FRACUNIT/4;

		if(elevator->origspeed)
			elevator->speed *= elevator->origspeed/(ELEVATORSPEED/2);

		if(elevator->origspeed)
		{
			if(elevator->speed > elevator->origspeed)
				elevator->speed = (elevator->origspeed);
			if(elevator->speed < 1)
				elevator->speed = 1/NEWTICRATERATIO;
		}
		else
		{
			if(elevator->speed > ((3*FRACUNIT)/NEWTICRATERATIO))
				elevator->speed = ((3*FRACUNIT)/NEWTICRATERATIO);
			if(elevator->speed < 1)
				elevator->speed = 1/NEWTICRATERATIO;
		}
	  }

    res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
    (
      elevator->sector,
      elevator->speed,
      elevator->floordestheight,
      0,
      0,                          // move ceiling
      elevator->direction
    );
    if (res==ok || res==pastdest) // jff 4/7/98 don't move floor if blocked
      T_MovePlane
      (
        elevator->sector,
        elevator->speed,
        elevator->ceilingdestheight,
        0,
        1,                        // move floor
        elevator->direction
      );
  }
/*
  // make floor move sound
  if (!(leveltime&7))
    S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_stnmov);
*/
  if (res == pastdest)            // if destination height acheived
  {
    elevator->sector->floordata = NULL;     //jff 2/22/98
    elevator->sector->ceilingdata = NULL;   //jff 2/22/98
	if(elevator->type != elevateContinuous)
	{
		P_RemoveThinker(&elevator->thinker);    // remove elevator from actives
		dontupdate = true;
	}
	if(elevator->type == elevateContinuous)
	{
if(elevator->direction > 0)
{
	elevator->high = 1;
	elevator->low = 0;
	elevator->direction = -1;

		  if(elevator->origspeed)
		    elevator->speed = elevator->origspeed;
		  else
		    elevator->speed = ((3*FRACUNIT)/NEWTICRATERATIO);

		  if(elevator->low)
		  {
			  elevator->floordestheight =
				P_FindNextHighestFloor(elevator->sector, elevator->sector->floorheight);
		      elevator->ceilingdestheight =
				elevator->floordestheight + elevator->sector->ceilingheight - elevator->sector->floorheight;
		  }
		  else
		  {
			  elevator->floordestheight =
				P_FindNextLowestFloor(elevator->sector,elevator->sector->floorheight);
			  elevator->ceilingdestheight =
				elevator->floordestheight + elevator->sector->ceilingheight - elevator->sector->floorheight;
		  }
	elevator->floorwasheight = elevator->sector->floorheight;
	elevator->ceilingwasheight = elevator->sector->ceilingheight;
		  T_MoveElevator(elevator);
}
else
{
	elevator->high = 0;
	elevator->low = 1;
	elevator->direction = 1;

		  if(elevator->origspeed)
		    elevator->speed = elevator->origspeed;
		  else
		    elevator->speed = ((3*FRACUNIT)/NEWTICRATERATIO);

		  if(elevator->low)
		  {
			  elevator->floordestheight =
				P_FindNextHighestFloor(elevator->sector, elevator->sector->floorheight);
		      elevator->ceilingdestheight =
				elevator->floordestheight + elevator->sector->ceilingheight - elevator->sector->floorheight;
		  }
		  else
		  {
			  elevator->floordestheight =
				P_FindNextLowestFloor(elevator->sector,elevator->sector->floorheight);
			  elevator->ceilingdestheight =
				elevator->floordestheight + elevator->sector->ceilingheight - elevator->sector->floorheight;
		  }
	elevator->floorwasheight = elevator->sector->floorheight;
	elevator->ceilingwasheight = elevator->sector->ceilingheight;
	T_MoveElevator(elevator);
}
	}
    // make floor stop sound
//    S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_pstop);
  }
  if(!dontupdate)
  {
	  elevator->sector->floorspeed = elevator->speed*elevator->direction;
	  elevator->sector->ceilspeed = 42;
  }
  else
  {
	  elevator->sector->floorspeed = 0;
	  elevator->sector->ceilspeed = 0;
  }
}


//
// P_SectorCheckWater
//
// Like P_MobjCheckWater, but takes a sector instead of a mobj.
fixed_t P_SectorCheckWater (sector_t* analyzesector, sector_t* elevatorsec)
{
	fixed_t   watertop;

	// Default if no water exists.
	watertop = -1;

    //
    // see if we are in water, and set some flags for later
    //
    //SoM: 3/28/2000: Only use 280 water type of water. Some boom levels get messed up.
    if ((analyzesector->heightsec > -1 && analyzesector->altheightsec == 1))
    {
        if (analyzesector->heightsec > -1)  //water hack
            watertop = (sectors[analyzesector->heightsec].floorheight);
    }
    else if(analyzesector->ffloors)
    {
      ffloor_t*  rover;

      for(rover = analyzesector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SWIMMABLE) || rover->flags & FF_SOLID)
          continue;

		// Set the watertop and waterbottom Tails 02-03-2002
		watertop = *rover->topheight;
      }
    }

	if(watertop < analyzesector->floorheight + abs((elevatorsec->ceilingheight - elevatorsec->floorheight)>>1))
		watertop = -42;

	return watertop;
}

//////////////////////////////////////////////////
// T_BounceCheese ////////////////////////////////
//////////////////////////////////////////////////
// Bounces a floating cheese Tails 02-08-2002

void T_BounceCheese(elevator_t* elevator)
{
  //int i;

  fixed_t halfheight;
  fixed_t waterheight;
  boolean nobounce = false;

  if(elevator->sector->teamstartsec == 2) // Oops! Crumbler says to remove yourself!
  {
	elevator->sector->teamstartsec = 1;
	P_RemoveThinker(&elevator->thinker);    // remove elevator from actives
  }

  halfheight = abs(elevator->sector->ceilingheight - elevator->sector->floorheight) >> 1;

  waterheight = P_SectorCheckWater(elevator->actionsector, elevator->sector);

  if(waterheight == -42)
  {
	  nobounce = true;
	  elevator->ceilingwasheight =  elevator->actionsector->floorheight + abs(elevator->sector->ceilingheight - elevator->sector->floorheight);
      elevator->floorwasheight = elevator->actionsector->floorheight;
  }
  else
  {
	  elevator->ceilingwasheight =  waterheight + halfheight;
      elevator->floorwasheight = waterheight - halfheight;
  }

		T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
		(
		  elevator->sector,
		  elevator->speed/2,
		  elevator->sector->ceilingheight - 70*FRACUNIT,
		  0,
		  1,                          // move floor
		  -1
		);

		  T_MovePlane
		  (
			elevator->sector,
			elevator->speed/2,
			elevator->sector->floorheight - 70*FRACUNIT,
			0,
			0,                        // move ceiling
			-1
		  );

	if(elevator->sector->ceilingheight < elevator->ceilingwasheight && elevator->low == 0) // Down
	{
		if(nobounce)
		{
			elevator->speed = 0;
			goto done;
		}

		if(abs(elevator->speed) < 6*FRACUNIT)
			elevator->speed -= elevator->speed/3;
		else
			elevator->speed -= elevator->speed/2;

		elevator->low = 1;
		if(abs(elevator->speed) > 6*FRACUNIT)
		{
			elevator->actionsector->soundorg.z = elevator->sector->floorheight;
			S_StartSound((mobj_t *)&elevator->actionsector->soundorg, sfx_splash);
		}
	}
	else if(elevator->sector->ceilingheight > elevator->ceilingwasheight && elevator->low) // Up
	{
		if(abs(elevator->speed) < 6*FRACUNIT)
			elevator->speed -= elevator->speed/3;
		else
			elevator->speed -= elevator->speed/2;

		elevator->low = 0;
		if(abs(elevator->speed) > 6*FRACUNIT)
		{
			elevator->actionsector->soundorg.z = elevator->sector->floorheight;
			S_StartSound((mobj_t *)&elevator->actionsector->soundorg, sfx_splash);
		}
	}

	if(elevator->sector->ceilingheight < elevator->ceilingwasheight) // Down
	{
		elevator->speed -= elevator->distance;
	}
	else if(elevator->sector->ceilingheight > elevator->ceilingwasheight) // Up
	{
		elevator->speed += gravity;
	}

	if(elevator->speed < 2*FRACUNIT && elevator->speed > -2*FRACUNIT
		&& elevator->sector->ceilingheight < elevator->ceilingwasheight + FRACUNIT/4
		&& elevator->sector->ceilingheight > elevator->ceilingwasheight - FRACUNIT/4)
	{
done:
		elevator->sector->floorheight = elevator->floorwasheight;
		elevator->sector->ceilingheight = elevator->ceilingwasheight;
		elevator->sector->teamstartsec = 0;
		P_RemoveThinker(&elevator->thinker);    // remove elevator from actives
	}

	if(elevator->distance > 0)
		elevator->distance--;
}

//////////////////////////////////////////////////
// T_StartCrumble ////////////////////////////////
//////////////////////////////////////////////////
// Crumbling platform Tails 03-11-2002

void T_StartCrumble(elevator_t* elevator)
{

	if(elevator->distance != 0)
	{
		if(elevator->distance > 0)
		{
			elevator->distance--;
			if(elevator->distance == 0)
				elevator->distance = -15*TICRATE;
			else
				return;
		}
		else if (++elevator->distance == 0)
		{
			elevator->direction = 1; // Up!
			return;
		}
		if(elevator->distance < 0 && elevator->distance > -3 && elevator->high == 42)
		{
			elevator->sector->teamstartsec = 2;
		}
	}

	if(elevator->direction == 1 && elevator->type == elevateContinuous) // No return crumbler
	{
		return;
	}

	if(elevator->direction == -1) // Down
	{
		if(elevator->high == 42 && elevator->low == 0)
		{
			elevator->sector->teamstartsec = 0; // Allow floating now.
			elevator->low = 1;
		}
		else if (elevator->high != 42)
		{
			elevator->speed += gravity; // Gain more and more speed

			if(!(elevator->sector->ceilingheight < elevator->floordestheight))
			{
				T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
				(
				  elevator->sector,
				  elevator->speed,
				  elevator->sector->ceilingheight - 70*FRACUNIT,
				  0,
				  1,                          // move floor
				  elevator->direction
				);

				  T_MovePlane
				  (
					elevator->sector,
					elevator->speed,
					elevator->sector->floorheight - 70*FRACUNIT,
					0,
					0,                        // move ceiling
					elevator->direction
				  );
			}
		}
	}
	else // Up (restore to original position)
	{
		if(elevator->high == 42)
			elevator->sector->teamstartsec = 1;
		elevator->sector->floorheight = elevator->floorwasheight;
		elevator->sector->ceilingheight = elevator->ceilingwasheight;
		elevator->sector->floordata = NULL;     //jff 2/22/98
		P_RemoveThinker(&elevator->thinker);
	}

}

//////////////////////////////////////////////////
// T_AirBob //////////////////////////////////////
//////////////////////////////////////////////////
// Like the still platforms in GHZ and EHZ Tails 03-11-2002

void T_AirBob(elevator_t* elevator)
{
	msecnode_t *node;
	boolean roverfound;

	roverfound = false;

	if(elevator->player && elevator->player->mo)
	{
		// Check if the player is still touching the sector.
		for (node = elevator->player->mo->touching_sectorlist; node; node = node->m_snext)
		{
			if(node->m_sector && node->m_sector->ffloors)
			{
				ffloor_t* rover;

				for(rover = node->m_sector->ffloors; rover; rover = rover->next)
				{
					if(rover->flags & FF_AIRBOB)
					{
						if(rover->master->frontsector == elevator->sector) // Found it!
						{
							roverfound = true; // He's still touching it
						}
					}
				}
			}
		}
	}

	if(roverfound == false && elevator->sector->floorheight == elevator->floorwasheight)
	{
		elevator->sector->ceilingdata = NULL;
		P_RemoveThinker(&elevator->thinker);
	}

  if(elevator->player && elevator->player->mo && elevator->player->mo->z == elevator->sector->ceilingheight)
  {
	  elevator->low = 1;
	  elevator->direction = -1; // Down
  }
  else
  {
	  elevator->low = 0;
	  elevator->direction = 1; // Up
  }


  	if(elevator->sector->floorheight <= elevator->distance) // Reached the sag point
	{
		if(elevator->sector->floordata) // Falling at the moment
			return;

		// Restore original position
		elevator->sector->ceilingheight = elevator->distance + elevator->sector->ceilingheight - elevator->sector->floorheight;
		elevator->sector->floorheight = elevator->distance;
		elevator->speed = 0;
	}
	if(elevator->sector->floorheight >= elevator->floorwasheight)
	{
		elevator->sector->floorheight = elevator->floorwasheight;
		elevator->sector->ceilingheight = elevator->ceilingwasheight;
		elevator->sector->ceilingdata = NULL;
		P_RemoveThinker(&elevator->thinker);
	}

	elevator->speed = FRACUNIT/NEWTICRATERATIO; // I'm paranoid, okay?

  if(elevator->low)
  {
		T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
		(
		  elevator->sector,
		  elevator->speed,
		  elevator->sector->ceilingheight - 70*FRACUNIT,
		  0,
		  1,                          // move floor
		  elevator->direction
		);

		  T_MovePlane
		  (
			elevator->sector,
			elevator->speed,
			elevator->sector->floorheight - 70*FRACUNIT,
			0,
			0,                        // move ceiling
			elevator->direction
		  );
  }
  else // push up!
  {
		T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
		(
		  elevator->sector,
		  elevator->speed,
		  elevator->sector->ceilingheight + 70*FRACUNIT,
		  0,
		  1,                          // move floor
		  elevator->direction
		);

		  T_MovePlane
		  (
			elevator->sector,
			elevator->speed,
			elevator->sector->floorheight + 70*FRACUNIT,
			0,
			0,                        // move ceiling
			elevator->direction
		  );
  }
}

// Graue 11-07-2003
// Someday I'll discover a better way to do this. In the meantime, enjoy an extra
// function almost exactly the same as T_AirBob. Hooray for bloat!
void T_AirBobReverse(elevator_t* elevator)
{
	msecnode_t *node;
	boolean roverfound;

	roverfound = false;

	if(elevator->player && elevator->player->mo)
	{
		// Check if the player is still touching the sector.
		for (node = elevator->player->mo->touching_sectorlist; node; node = node->m_snext)
		{
			if(node->m_sector && node->m_sector->ffloors)
			{
				ffloor_t* rover;

				for(rover = node->m_sector->ffloors; rover; rover = rover->next)
				{
					if(rover->flags & FF_AIRBOB)
					{
						if(rover->master->frontsector == elevator->sector) // Found it!
						{
							roverfound = true; // He's still touching it
						}
					}
				}
			}
		}
	}

	if(roverfound == false && elevator->sector->floorheight == elevator->floorwasheight)
	{
		elevator->sector->ceilingdata = NULL;
		P_RemoveThinker(&elevator->thinker);
	}

	if(elevator->player && elevator->player->mo && elevator->player->mo->z == elevator->sector->ceilingheight)
	{
		elevator->low = 0;
		elevator->direction = 1; // Up
	}
	else
	{
		elevator->low = 1;
		elevator->direction = -1; // Down
	}


  	if(elevator->sector->floorheight >= elevator->distance) // Reached the sag point
	{
		if(elevator->sector->floordata) // Rising at the moment
			return;

		// Restore original position
		elevator->sector->ceilingheight = elevator->distance + elevator->sector->ceilingheight - elevator->sector->floorheight;
		elevator->sector->floorheight = elevator->distance;
		elevator->speed = 0;
	}
	if(elevator->sector->floorheight <= elevator->floorwasheight)
	{
		elevator->sector->floorheight = elevator->floorwasheight;
		elevator->sector->ceilingheight = elevator->ceilingwasheight;
		elevator->sector->ceilingdata = NULL;
		P_RemoveThinker(&elevator->thinker);
	}

	elevator->speed = FRACUNIT/NEWTICRATERATIO; // I'm paranoid, okay? - Yes, I too.

  if(elevator->low)
  {
		T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
		(
		  elevator->sector,
		  elevator->speed,
		  elevator->sector->ceilingheight - 70*FRACUNIT,
		  0,
		  1,                          // move floor
		  elevator->direction
		);

		  T_MovePlane
		  (
			elevator->sector,
			elevator->speed,
			elevator->sector->floorheight - 70*FRACUNIT,
			0,
			0,                        // move ceiling
			elevator->direction
		  );
  }
  else // push up!
  {
		T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
		(
		  elevator->sector,
		  elevator->speed,
		  elevator->sector->ceilingheight + 70*FRACUNIT,
		  0,
		  1,                          // move floor
		  elevator->direction
		);

		  T_MovePlane
		  (
			elevator->sector,
			elevator->speed,
			elevator->sector->floorheight + 70*FRACUNIT,
			0,
			0,                        // move ceiling
			elevator->direction
		  );
  }
}

//////////////////////////////////////////////////
// T_MarioBlock //////////////////////////////////
//////////////////////////////////////////////////
// Mario hits a block! Tails 03-11-2002

void T_MarioBlock(elevator_t* elevator)
{
		T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
		(
		  elevator->sector,
		  elevator->speed,
		  elevator->sector->ceilingheight + 70*FRACUNIT * elevator->direction,
		  0,
		  1,                          // move floor
		  elevator->direction
		);

		  T_MovePlane
		  (
			elevator->sector,
			elevator->speed,
			elevator->sector->floorheight + 70*FRACUNIT * elevator->direction,
			0,
			0,                        // move ceiling
			elevator->direction
		  );

		  if(elevator->sector->ceilingheight >= elevator->ceilingwasheight + 32*FRACUNIT) // Go back down now..
			  elevator->direction = -elevator->direction;
		  else if (elevator->sector->ceilingheight <= elevator->ceilingwasheight)
		  {
			  elevator->sector->ceilingheight = elevator->ceilingwasheight;
			  elevator->sector->floorheight = elevator->floorwasheight;
			  P_RemoveThinker(&elevator->thinker);
			  elevator->sector->floordata = NULL;     //jff 2/22/98
			  elevator->sector->ceilingdata = NULL;   //jff 2/22/98
		  }
}

// Tails 09-20-2002
void T_SpikeSector(elevator_t* elevator)
{
    mobj_t   *thing;
    msecnode_t* node;

    node = elevator->sector->touching_thinglist; // things touching this sector
    for ( ; node ; node = node->m_snext)
    {
		thing = node->m_thing;
		if (!thing->player)
			continue;

		if (!thing->momz > 0 && thing->z <= node->m_sector->floorheight)
		{
			mobj_t* killer;
			killer = P_SpawnMobj(thing->x, thing->y, thing->z, MT_DISS);
			killer->threshold = 43; // Special flag that it was spikes which hurt you.

			P_DamageMobj(thing, killer, killer, 1);
			break;
		}
	}
}

// Tails 09-20-2002
void T_FloatSector(elevator_t* elevator)
{
	fixed_t cheeseheight;
	boolean      tofloat;
	boolean  floatanyway; // Ignore the TEAMSTARTSEC setting.

	tofloat = false;
	floatanyway = false;

	cheeseheight = elevator->sector->floorheight + (elevator->sector->ceilingheight - elevator->sector->floorheight)/2;

	if(elevator->actionsector->ffloors)
    {
      ffloor_t*  rover;

      for(rover = elevator->actionsector->ffloors; rover; rover = rover->next)
      {
        if(!(rover->flags & FF_SWIMMABLE) || rover->flags & FF_SOLID)
          continue;

        if(cheeseheight > *rover->topheight)
            tofloat = true;
        else if(cheeseheight < *rover->topheight)
		{
			tofloat = true;
			floatanyway = true;
		}
	  }
    }

	if(tofloat == true && (elevator->sector->teamstartsec == 0 || floatanyway))
		EV_BounceSector(elevator->sector, 1, elevator->actionsector, false);
}

// Tails 09-20-2002
void T_MarioBlockChecker(elevator_t* elevator)
{
    mobj_t   *thing;
    msecnode_t* node;
	line_t* masterline;

	node = NULL;
	thing = NULL;

	masterline = elevator->sourceline;

	node = elevator->sector->touching_thinglist; // things touching this sector

	if(node != NULL)
	{
		thing = node->m_thing;

		if(thing != NULL && (thing->health))
		{
			if((thing->flags & MF_MONITOR) && thing->threshold == 68)
				sides[masterline->sidenum[0]].midtexture = sides[masterline->sidenum[0]].toptexture;
			else
				sides[masterline->sidenum[0]].midtexture = sides[masterline->sidenum[0]].bottomtexture;
		}
		else
			sides[masterline->sidenum[0]].midtexture = sides[masterline->sidenum[0]].toptexture;
	}
	else
		sides[masterline->sidenum[0]].midtexture = sides[masterline->sidenum[0]].toptexture;
}

fixed_t P_FloorzAtPos(fixed_t x, fixed_t y, fixed_t z, fixed_t height);
// This is the Thwomp's 'brain'. It looks around for players nearby, and if so,
// **SMASH**!!! Muahahhaa....
// Tails 05-26-2003
void T_ThwompSector(elevator_t* elevator)
{
	fixed_t thwompx, thwompy;
	thinker_t*  th;
	mobj_t*     mo2;
	player_t*   closestplayer = NULL;

	if(--elevator->distance > 0)
	{
		sides[elevator->sourceline->sidenum[0]].midtexture = sides[elevator->sourceline->sidenum[0]].bottomtexture;
		return;
	}

	thwompx = elevator->actionsector->soundorg.x;
	thwompy = elevator->actionsector->soundorg.y;

	if(elevator->direction > 0) // Moving back up..
	{
		result_e res = 0;

		// Set the texture from the lower one (normal)
		sides[elevator->sourceline->sidenum[0]].midtexture = sides[elevator->sourceline->sidenum[0]].bottomtexture;

		elevator->speed = 2*FRACUNIT/NEWTICRATERATIO;

		res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
		(
		  elevator->sector,
		  elevator->speed,
		  elevator->floorwasheight,
		  0,
		  0,                          // move floor
		  elevator->direction
		);

		if(res==ok || res==pastdest)
 	    T_MovePlane
		  (
			elevator->sector,
			elevator->speed,
			elevator->ceilingwasheight,
			0,
			1,                        // move ceiling
			elevator->direction
		  );

		if (res==pastdest)
			elevator->direction = 0;
	}
	else if(elevator->direction < 0) // Crashing down!
	{
		result_e res = 0;

		// Set the texture from the upper one (angry)
		sides[elevator->sourceline->sidenum[0]].midtexture = sides[elevator->sourceline->sidenum[0]].toptexture;

		elevator->speed = 10*FRACUNIT/NEWTICRATERATIO;

		res = T_MovePlane             //jff 4/7/98 reverse order of ceiling/floor
		(
		  elevator->sector,
		  elevator->speed,
		  P_FloorzAtPos(thwompx, thwompy, elevator->sector->floorheight, elevator->sector->ceilingheight - elevator->sector->floorheight),
		  0,
		  0,                          // move floor
		  elevator->direction
		);

		if(res==ok || res==pastdest)
 	    T_MovePlane
		  (
			elevator->sector,
			elevator->speed,
		    P_FloorzAtPos(thwompx, thwompy, elevator->sector->floorheight, elevator->sector->ceilingheight - (elevator->sector->floorheight+elevator->speed)) + (elevator->sector->ceilingheight - (elevator->sector->floorheight+elevator->speed/2)),
			0,
			1,                        // move ceiling
			elevator->direction
		  );

		if (res==pastdest)
		{
			S_StartSound((mobj_t*)&elevator->actionsector->soundorg, sfx_thwomp);
			elevator->direction = 1;
			elevator->distance = TICRATE;
		}
	}
	else // Not going anywhere, so look for players.
	{
		// scan the thinkers
		// to find players!
		for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
		{
			if (th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			mo2 = (mobj_t *)th;
			if(mo2->type == MT_PLAYER && mo2->health)
			{
				if(closestplayer == NULL)
				{
					closestplayer = mo2->player;
					continue;
				}

				if(closestplayer != NULL && P_AproxDistance(thwompx-mo2->x, thwompy-mo2->y) < P_AproxDistance(thwompx-closestplayer->mo->x, thwompy-closestplayer->mo->y))
					closestplayer = mo2->player;
			}
		}

		if(closestplayer == NULL) // No players in the game?! Uh.. riiiiight....
			return;

		if(closestplayer
			&& P_AproxDistance(thwompx-closestplayer->mo->x, thwompy-closestplayer->mo->y) > 64*FRACUNIT) // Too far away
			return;

		elevator->direction = -1;
	}
}

void T_CameraScanner(elevator_t* elevator)
{
	// Graue 12-13-2003: fixes for multiple camera scanners in one map
	static int lastactionsector = MAXINT;
	static boolean camerascanned, camerascanned2;

	if(elevator->actionsector - sectors < lastactionsector) // Back on the first action sector
		camerascanned = camerascanned2 = false;
	lastactionsector = elevator->actionsector - sectors;

	if(players[displayplayer].mo)
	if(players[displayplayer].mo->subsector->sector == elevator->actionsector)
	{
		if(t_cam_dist == -42)
			t_cam_dist = cv_cam_dist.value;
		if(t_cam_height == -42)
			t_cam_height = cv_cam_height.value;
		if(t_cam_rotate == -42)
			t_cam_rotate = cv_cam_rotate.value;
		CV_SetValue(&cv_cam_height, elevator->sector->floorheight>>FRACBITS);
		CV_SetValue(&cv_cam_dist, elevator->sector->ceilingheight>>FRACBITS);
		CV_SetValue(&cv_cam_rotate, elevator->distance);
		camerascanned = true; // Graue 12-13-2003
	}
	else if(!camerascanned)
	{
		if(t_cam_height != -42 && cv_cam_height.value != t_cam_height)
			CV_Set(&cv_cam_height, va("%f", (float)t_cam_height/FRACUNIT));
		if(t_cam_dist != -42 && cv_cam_dist.value != t_cam_dist)
			CV_Set(&cv_cam_dist, va("%f", (float)t_cam_dist/FRACUNIT));
		if(t_cam_rotate != -42 && cv_cam_rotate.value != t_cam_rotate)
			CV_Set(&cv_cam_rotate, va("%f", (float)t_cam_rotate));

		t_cam_dist = t_cam_height = t_cam_rotate = -42;
	}

	if(cv_splitscreen.value && players[secondarydisplayplayer].mo)
	if(players[secondarydisplayplayer].mo->subsector->sector == elevator->actionsector)
	{
		if(t_cam2_rotate == -42)
			t_cam2_dist = cv_cam2_dist.value;
		if(t_cam2_rotate == -42)
			t_cam2_height = cv_cam2_height.value;
		if(t_cam2_rotate == -42)
			t_cam2_rotate = cv_cam2_rotate.value;
		CV_SetValue(&cv_cam2_height, elevator->sector->floorheight>>FRACBITS);
		CV_SetValue(&cv_cam2_dist, elevator->sector->ceilingheight>>FRACBITS);
		CV_SetValue(&cv_cam2_rotate, elevator->distance);
		camerascanned2 = true; // Graue 12-13-2003
	}
	else if(!camerascanned2)
	{
		if(t_cam2_height != -42 && cv_cam2_height.value != t_cam2_height)
			CV_Set(&cv_cam2_height, va("%f", (float)t_cam2_height/FRACUNIT));
		if(t_cam2_dist != -42 && cv_cam2_dist.value != t_cam2_dist)
			CV_Set(&cv_cam2_dist, va("%f", (float)t_cam2_dist/FRACUNIT));
		if(t_cam2_rotate != -42 && cv_cam2_rotate.value != t_cam2_rotate)
			CV_Set(&cv_cam2_rotate, va("%f", (float)t_cam2_rotate));

		t_cam2_dist = t_cam2_height = t_cam2_rotate = -42;
	}
}

//
// HANDLE FLOOR TYPES
//
int
EV_DoFloor
( line_t*       line,
  floor_e       floortype )
{
    int                 secnum;
    int                 rtn;
    int                 i;
    sector_t*           sec;
    floormove_t*        floor;

    secnum = -1;
    rtn = 0;
    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {
        sec = &sectors[secnum];
        
        // SoM: 3/6/2000: Boom has multiple thinkers per sector.
        // Don't start a second thinker on the same floor
        if (P_SectorActive(floor_special,sec)) //jff 2/23/98
          continue;

        // new floor thinker
        rtn = 1;
        floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker (&floor->thinker);
        sec->floordata = floor; //SoM: 2/5/2000
        floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
        floor->type = floortype;
        floor->crush = false;
		floor->sector = sec; // All of the cases below do this Graue 12-21-2003

        switch(floortype)
        {
          case lowerFloor:
            floor->direction = -1;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindLowestFloorSurrounding(sec); // Was set at "Highest"??? Tails 11-10-2001
            break;

            //jff 02/03/30 support lowering floor by 24 absolute
          case lowerFloor24:
            floor->direction = -1;
            floor->speed = FLOORSPEED;
            floor->floordestheight = sec->floorheight + 24 * FRACUNIT; // Optimised by Graue 12-21-2003
            break;

            //jff 02/03/30 support lowering floor by 32 absolute (fast)
          case lowerFloor32Turbo:
            floor->direction = -1;
            floor->speed = FLOORSPEED*4;
            floor->floordestheight = sec->floorheight + 32 * FRACUNIT; // Optimised by Graue 12-21-2003
            break;

          case lowerFloorToLowest:
            floor->direction = -1;
            floor->speed = FLOORSPEED*2;
            floor->floordestheight = P_FindLowestFloorSurrounding(sec);
            break;

            //jff 02/03/30 support lowering floor to next lowest floor
          case lowerFloorToNearest:
            floor->direction = -1;
            floor->speed = FLOORSPEED;
            floor->floordestheight =
              P_FindNextLowestFloor(sec,sec->floorheight); // Optimised by Graue 12-21-2003
            break;

          case turboLower:
            floor->direction = -1;
            floor->speed = FLOORSPEED * 4;
            floor->floordestheight = P_FindLowestFloorSurrounding(sec); // Was set at "Highest"??? Tails 11-10-2001
            if (floor->floordestheight != sec->floorheight)
                floor->floordestheight += 8*FRACUNIT;
            break;

          case raiseFloorCrush:
            floor->crush = true;
          case raiseFloor:
            floor->direction = 1;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
            if (floor->floordestheight > sec->ceilingheight)
                floor->floordestheight = sec->ceilingheight;
            floor->floordestheight -= (8*FRACUNIT)* (floortype == raiseFloorCrush);
            break;

          case raiseFloorTurbo:
            floor->direction = 1;
            floor->speed = FLOORSPEED*4;
            floor->floordestheight = P_FindNextHighestFloor(sec,sec->floorheight);
            break;

          case raiseFloorToNearest:
            floor->direction = 1;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindNextHighestFloor(sec,sec->floorheight);
            break;

          case raiseFloorToNearestFast:
            floor->direction = 1;
            floor->speed = (4*FRACUNIT)/NEWTICRATERATIO;
            floor->floordestheight = P_FindNextHighestFloor(sec,sec->floorheight);
            break;

          case raiseFloor24:
            floor->direction = 1;
            floor->speed = FLOORSPEED;
            floor->floordestheight = sec->floorheight + 24 * FRACUNIT; // Optimised by Graue 12-21-2003
            break;

          // SoM: 3/6/2000: support straight raise by 32 (fast)
          case raiseFloor32Turbo:
            floor->direction = 1;
            floor->speed = FLOORSPEED*4;
            floor->floordestheight = sec->floorheight + 32 * FRACUNIT; // Optimised by Graue 12-21-2003
            break;

          case raiseFloor512:
            floor->direction = 1;
            floor->speed = FLOORSPEED;
            floor->floordestheight = sec->floorheight + 512 * FRACUNIT; // Optimised by Graue 12-21-2003
            break;

          case raiseFloor24AndChange:
            floor->direction = 1;
            floor->speed = FLOORSPEED;
            floor->floordestheight = sec->floorheight + 24 * FRACUNIT; // Optimised by Graue 12-21-2003
            sec->floorpic = line->frontsector->floorpic;
            sec->special = line->frontsector->special;
            sec->oldspecial = line->frontsector->oldspecial;
            break;

          case raiseToTexture:
          {
              int       minsize = MAXINT;
              side_t*   side;

              if (boomsupport) minsize = 32000<<FRACBITS; //SoM: 3/6/2000: ???
              floor->direction = 1;
              floor->speed = FLOORSPEED;
              for (i = 0; i < sec->linecount; i++)
              {
                if (twoSided (secnum, i) )
                {
                  side = getSide(secnum,i,0);
                  // jff 8/14/98 don't scan texture 0, its not real
                  if (side->bottomtexture > 0 ||
                      (!boomsupport && !side->bottomtexture))
                    if (textureheight[side->bottomtexture] < minsize)
                      minsize = textureheight[side->bottomtexture];
                  side = getSide(secnum,i,1);
                  // jff 8/14/98 don't scan texture 0, its not real
                  if (side->bottomtexture > 0 ||
                      (!boomsupport && !side->bottomtexture))
                    if (textureheight[side->bottomtexture] < minsize)
                      minsize = textureheight[side->bottomtexture];
                }
              }
              if (!boomsupport)
                floor->floordestheight = sec->floorheight + minsize; // Optimised by Graue 12-21-2003
              else
              {
                floor->floordestheight =
                  (sec->floorheight>>FRACBITS) + (minsize>>FRACBITS); // Optimised by Graue 12-21-2003
                if (floor->floordestheight>32000)
                  floor->floordestheight = 32000;        //jff 3/13/98 do not
                floor->floordestheight<<=FRACBITS;       // allow height overflow
              }
            break;
          }
          //SoM: 3/6/2000: Boom changed allot of stuff I guess, and this was one of 'em 
          case lowerAndChange:
            floor->direction = -1;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindLowestFloorSurrounding(sec);
            floor->texture = sec->floorpic;

            // jff 1/24/98 make sure floor->newspecial gets initialized
            // in case no surrounding sector is at floordestheight
            // --> should not affect compatibility <--
            floor->newspecial = sec->special; 
            //jff 3/14/98 transfer both old and new special
            floor->oldspecial = sec->oldspecial;
    
            //jff 5/23/98 use model subroutine to unify fixes and handling
            // BP: heretic have change something here
            sec = P_FindModelFloorSector(floor->floordestheight,sec-sectors);
            if (sec)
            {
              floor->texture = sec->floorpic;
              floor->newspecial = sec->special;
              //jff 3/14/98 transfer both old and new special
              floor->oldspecial = sec->oldspecial;
            }
            break;
		//SOM: Instant lower baby!
          case instantLower:
            floor->direction = -1;
            floor->speed = MAXINT/2;
            floor->floordestheight =
            P_FindLowestFloorSurrounding(sec);
			break;
		// Graue 12-12-2003: Linedef executor excellence
		  case moveFloorByFrontSector:
			floor->speed = P_AproxDistance(line->dx, line->dy)/8;
			floor->floordestheight = line->frontsector->floorheight;

			if(floor->floordestheight >= sec->floorheight) // Move up
				floor->direction = 1;
			else // Move down
				floor->direction = -1;
			break;
		// Graue 12-12-2003: More linedef executor junk
		  case instantMoveFloorByFrontSector:
			floor->speed = MAXINT/2;
			floor->floordestheight = line->frontsector->floorheight;

			if(floor->floordestheight >= sec->floorheight) // Move up
				floor->direction = 1;
			else // Move down
				floor->direction = -1;

			floor->texture = line->frontsector->floorpic;
			break;
		// Graue 12-21-2003
		  case lowerFloorByLine:
			floor->speed = abs(line->dx)/8;
			floor->floordestheight = sec->floorheight - abs(line->dy);
			floor->direction = -1; // Move down
			break;
		  case raiseFloorByLine:
			floor->speed = abs(line->dx)/8;
			floor->floordestheight = sec->floorheight + abs(line->dy);
			floor->direction = 1; // Move up
			break;
          default:
            break;
        }
    }
    return rtn;
}


// SoM: 3/6/2000: Function for chaning just the floor texture and type.
//
// EV_DoChange()
//
// Handle pure change types. These change floor texture and sector type
// by trigger or numeric model without moving the floor.
//
// The linedef causing the change and the type of change is passed
// Returns true if any sector changes
//
//
int EV_DoChange
( line_t*       line,
  change_e      changetype )
{
  int                   secnum;
  int                   rtn;
  sector_t*             sec;
  sector_t*             secm;

  secnum = -1;
  rtn = 0;
  // change all sectors with the same tag as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];
              
    rtn = 1;

    // handle trigger or numeric change type
    switch(changetype)
    {
      case trigChangeOnly:
        sec->floorpic = line->frontsector->floorpic;
        sec->special = line->frontsector->special;
        sec->oldspecial = line->frontsector->oldspecial;
        break;
      case numChangeOnly:
        secm = P_FindModelFloorSector(sec->floorheight,secnum);
        if (secm) // if no model, no change
        {
          sec->floorpic = secm->floorpic;
          sec->special = secm->special;
          sec->oldspecial = secm->oldspecial;
        }
        break;
      default:
        break;
    }
  }
  return rtn;
}




//
// BUILD A STAIRCASE!
//

// SoM: 3/6/2000: Use the Boom version of this function.
int EV_BuildStairs
( line_t*       line,
  stair_e       type )
{
  int                   secnum;
  int                   osecnum;
  int                   height;
  int                   i;
  int                   newsecnum;
  int                   texture;
  int                   ok;
  int                   rtn;
    
  sector_t*             sec;
  sector_t*             tsec;

  floormove_t*  floor;
    
  fixed_t               stairsize;
  fixed_t               speed;

  secnum = -1;
  rtn = 0;

  // start a stair at each sector tagged the same as the linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];
              
    // don't start a stair if the first step's floor is already moving
    if (P_SectorActive(floor_special,sec))
      continue;
      
    // create new floor thinker for first step
    rtn = 1;
    floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
    P_AddThinker (&floor->thinker);
    sec->floordata = floor;
    floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
    floor->direction = 1;
    floor->sector = sec;
    floor->type = buildStair;   //jff 3/31/98 do not leave uninited

    // set up the speed and stepsize according to the stairs type
    switch(type)
    {
      case build8:
        speed = FLOORSPEED/4;
        stairsize = 8*FRACUNIT;
        if (boomsupport)
          floor->crush = false; //jff 2/27/98 fix uninitialized crush field
        break;
      case turbo16:
        speed = FLOORSPEED*4;
        stairsize = 16*FRACUNIT;
        if (boomsupport)
          floor->crush = true;  //jff 2/27/98 fix uninitialized crush field
        break;
      // used by heretic
      default:
        speed = FLOORSPEED;
        stairsize = type;
        if (boomsupport)
          floor->crush = true;  //jff 2/27/98 fix uninitialized crush field
        break;
    }
    floor->speed = speed;
    height = sec->floorheight + stairsize;
    floor->floordestheight = height;
              
    texture = sec->floorpic;
    osecnum = secnum;           //jff 3/4/98 preserve loop index
      
    // Find next sector to raise
    //   1. Find 2-sided line with same sector side[0] (lowest numbered)
    //   2. Other side is the next sector to raise
    //   3. Unless already moving, or different texture, then stop building
    do
    {
      ok = 0;
      for (i = 0;i < sec->linecount;i++)
      {
        if ( !((sec->lines[i])->flags & ML_TWOSIDED) )
          continue;
                                  
        tsec = (sec->lines[i])->frontsector;
        newsecnum = tsec-sectors;
          
        if (secnum != newsecnum)
          continue;

        tsec = (sec->lines[i])->backsector;
        if (!tsec) continue;     //jff 5/7/98 if no backside, continue
        newsecnum = tsec - sectors;

        // if sector's floor is different texture, look for another
        if (tsec->floorpic != texture)
          continue;

        if (!boomsupport) // jff 6/19/98 prevent double stepsize
          height += stairsize; // jff 6/28/98 change demo compatibility

        // if sector's floor already moving, look for another
        if (P_SectorActive(floor_special,tsec)) //jff 2/22/98
          continue;
                                  
        if (boomsupport) // jff 6/19/98 increase height AFTER continue
          height += stairsize; // jff 6/28/98 change demo compatibility

        sec = tsec;
        secnum = newsecnum;

        // create and initialize a thinker for the next step
        floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
        P_AddThinker (&floor->thinker);

        sec->floordata = floor; //jff 2/22/98
        floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
        floor->direction = 1;
        floor->sector = sec;
        floor->speed = speed;
        floor->floordestheight = height;
        floor->type = buildStair; //jff 3/31/98 do not leave uninited
        //jff 2/27/98 fix uninitialized crush field
        if (boomsupport)
          floor->crush = type==build8? false : true;
        ok = 1;
        break;
      }
    } while(ok);      // continue until no next step is found
    secnum = osecnum; //jff 3/4/98 restore loop index
  }
  return rtn;
}


//SoM: 3/6/2000: boom donut function
//
// EV_DoDonut()
//
// Handle donut function: lower pillar, raise surrounding pool, both to height,
// texture and type of the sector surrounding the pool.
//
// Passed the linedef that triggered the donut
// Returns whether a thinker was created
//
int EV_DoDonut(line_t*  line)
{
  sector_t* s1;
  sector_t* s2;
  sector_t* s3;
  int       secnum;
  int       rtn;
  int       i;
  floormove_t* floor;

  secnum = -1;
  rtn = 0;
  // do function on all sectors with same tag as linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    s1 = &sectors[secnum];                // s1 is pillar's sector
              
    // do not start the donut if the pillar is already moving
    if (P_SectorActive(floor_special,s1)) //jff 2/22/98
      continue;
                      
    s2 = getNextSector(s1->lines[0],s1);  // s2 is pool's sector
    if (!s2) continue;                    // note lowest numbered line around
                                          // pillar must be two-sided 

    // do not start the donut if the pool is already moving
    if (boomsupport && P_SectorActive(floor_special,s2)) 
      continue;                           //jff 5/7/98
                      
    // find a two sided line around the pool whose other side isn't the pillar
    for (i = 0;i < s2->linecount;i++)
    {
      //jff 3/29/98 use true two-sidedness, not the flag
      // killough 4/5/98: changed demo_compatibility to compatibility
      if (!boomsupport)
      {
        if ((!s2->lines[i]->flags & ML_TWOSIDED) ||
            (s2->lines[i]->backsector == s1))
          continue;
      }
      else if (!s2->lines[i]->backsector || s2->lines[i]->backsector == s1)
        continue;

      rtn = 1; //jff 1/26/98 no donut action - no switch change on return

      s3 = s2->lines[i]->backsector;      // s3 is model sector for changes
        
      //  Spawn rising slime
      floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
      P_AddThinker (&floor->thinker);
      s2->floordata = floor; //jff 2/22/98
      floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
      floor->type = donutRaise;
      floor->crush = false;
      floor->direction = 1;
      floor->sector = s2;
      floor->speed = FLOORSPEED / 2;
      floor->texture = s3->floorpic;
      floor->newspecial = 0;
      floor->floordestheight = s3->floorheight;
        
      //  Spawn lowering donut-hole pillar
      floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
      P_AddThinker (&floor->thinker);
      s1->floordata = floor; //jff 2/22/98
      floor->thinker.function.acp1 = (actionf_p1) T_MoveFloor;
      floor->type = lowerFloor;
      floor->crush = false;
      floor->direction = -1;
      floor->sector = s1;
      floor->speed = FLOORSPEED / 2;
      floor->floordestheight = s3->floorheight;
      break;
    }
  }
  return rtn;
}


// SoM: Boom elevator support.
//
// EV_DoElevator
//
// Handle elevator linedef types
//
// Passed the linedef that triggered the elevator and the elevator action
//
// jff 2/22/98 new type to move floor and ceiling in parallel
//
int EV_DoElevator
( line_t*       line,
  elevator_e    elevtype,
  boolean customspeed)
{
  int                   secnum;
  int                   rtn;
  sector_t*             sec;
  elevator_t*           elevator;

  secnum = -1;
  rtn = 0;
  // act on all sectors with the same tag as the triggering linedef
  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];
              
    // If either floor or ceiling is already activated, skip it
    if (sec->floordata || sec->ceilingdata) //jff 2/22/98
      continue;
      
    // create and initialize new elevator thinker
    rtn = 1;
    elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
    P_AddThinker (&elevator->thinker);
    sec->floordata = elevator; //jff 2/22/98
    sec->ceilingdata = elevator; //jff 2/22/98
    elevator->thinker.function.acp1 = (actionf_p1) T_MoveElevator;
    elevator->type = elevtype;

    // set up the fields according to the type of elevator action
    switch(elevtype)
    {
        // elevator down to next floor
      case elevateDown:
        elevator->direction = -1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED/2; // half speed Tails
        elevator->floordestheight =
          P_FindNextLowestFloor(sec,sec->floorheight);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

        // elevator up to next floor
      case elevateUp:
        elevator->direction = 1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED/4; // quarter speed Tails
        elevator->floordestheight =
          P_FindNextHighestFloor(sec,sec->floorheight);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

      // elevator up to highest floor
      case elevateHighest:
        elevator->direction = 1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED/4; // quarter speed Tails
        elevator->floordestheight =
          P_FindHighestFloorSurrounding(sec);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

        // elevator to floor height of activating switch's front sector
      case elevateCurrent:
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED;
        elevator->floordestheight = line->frontsector->floorheight;
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        elevator->direction =
          elevator->floordestheight>sec->floorheight?  1 : -1;
        break;
// Start continuous elevator Tails
	  case elevateContinuous:
		  if(customspeed)
		  {
			  elevator->origspeed = P_AproxDistance(line->dx, line->dy)/4;
			  elevator->origspeed /= NEWTICRATERATIO;
			  elevator->speed = elevator->origspeed;
		  }
		  else
		  {
			  elevator->speed = ELEVATORSPEED/2;
			  elevator->origspeed = 0;
		  }

		  elevator->sector = sec;
		  elevator->low = 1; // FIXTHIS! Graue 12-20-2003
		  if(elevator->low)
		  {
			  elevator->direction = 1;
			  elevator->floordestheight =
				P_FindNextHighestFloor(sec, sec->floorheight);
		      elevator->ceilingdestheight =
				elevator->floordestheight + sec->ceilingheight - sec->floorheight;
		  }
		  else
		  {
			  elevator->direction = -1;
			  elevator->floordestheight =
				P_FindNextLowestFloor(sec,sec->floorheight);
			  elevator->ceilingdestheight =
				elevator->floordestheight + sec->ceilingheight - sec->floorheight;
		  }
	elevator->floorwasheight = elevator->sector->floorheight;
	elevator->ceilingwasheight = elevator->sector->ceilingheight;
	    break;
// End continuous elevator Tails

      case bridgeFall:
        elevator->direction = -1;
        elevator->sector = sec;
        elevator->speed = ELEVATORSPEED*4; // half speed Tails
        elevator->floordestheight =
          P_FindNextLowestFloor(sec,sec->floorheight);
        elevator->ceilingdestheight =
          elevator->floordestheight + sec->ceilingheight - sec->floorheight;
        break;

      default:
        break;
    }
  }
  return rtn;
}

int EV_CrumbleChain
(sector_t* sec)
{
	ffloor_t*  rover;
	int i;

	if(sec->ffloors)
	{
		for(rover = sec->ffloors; rover; rover = rover->next)
		{
			if(!(rover->flags & FF_BUSTUP))
			  continue;

			rover->flags &= ~FF_SOLID;

			S_StartSound(&sec->soundorg, sfx_crumbl);

			for(i= -1; (i = P_FindSectorFromTag(rover->master->tag, i)) >= 0;)
			{
			  sector_t *sector = &sectors[i];
			  P_SpawnMobj(sector->soundorg.x, sector->soundorg.y, *rover->bottomheight + (*rover->topheight - *rover->bottomheight)/2, MT_ROCKCRUMBLE);
			}

			// Throw the floor into the ground
			rover->master->frontsector->floorheight = sec->floorheight - 128*FRACUNIT;
			rover->master->frontsector->ceilingheight = sec->floorheight - 64*FRACUNIT;

			rover->flags |= FF_SOLID;
			rover->flags &= ~FF_BUSTUP;
			rover->master->frontsector->teamstartsec = 1;
		}
	}
/*
	// Now that we've crumbled all of the blocks in that sector, move onto the next one!
	for(i=0; i<sec->linecount; i++)
	{
		CONS_Printf("Crumble Chaining the frontsector.\n");
		EV_CrumbleChain(lines[i].frontsector);

		if(lines[i].sidenum[1] == -1)
			continue;

		CONS_Printf("Crumble Chaining the backsector.\n");
		EV_CrumbleChain(lines[i].backsector);
	}
*/
	return 1;
}

// Used for bobbing platforms on the water Tails 02-08-2002
int EV_BounceSector
( sector_t*  sec,
  fixed_t    momz, sector_t* blocksector, boolean player )
{
  elevator_t*           elevator;

    // create and initialize new elevator thinker

  if(sec->teamstartsec == 1) // One at a time, ma'am.
	  return 0;

    elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
	P_AddThinker (&elevator->thinker);
    elevator->thinker.function.acp1 = (actionf_p1) T_BounceCheese;
    elevator->type = elevateBounce;

    // set up the fields according to the type of elevator action
	elevator->sector = sec;
	elevator->speed = abs(momz)/2;
	elevator->actionsector = blocksector;
	elevator->distance = FRACUNIT;
	elevator->low = 1;

	elevator->sector->teamstartsec = 1; // Currently bobbing.

	return 1;
}

// Some other 3dfloor special things Tails 03-11-2002 (Search p_mobj.c for description)
int EV_StartCrumble
( sector_t*  sec, sector_t* roversector, fixed_t roverheight, boolean floating, player_t* player)
{
  elevator_t*           elevator;

    // If floor is already activated, skip it
    if (sec->floordata) //jff 2/22/98
      return 0;
   
    // create and initialize new elevator thinker

	elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
	P_AddThinker (&elevator->thinker);
	sec->floordata = elevator; //jff 2/22/98
    elevator->thinker.function.acp1 = (actionf_p1) T_StartCrumble;
    elevator->type = elevateBounce;

    // set up the fields according to the type of elevator action
	elevator->sector = sec;
	elevator->speed = 0;
	elevator->direction = -1; // Down
	elevator->floorwasheight = elevator->sector->floorheight;
	elevator->ceilingwasheight = elevator->sector->ceilingheight;
	elevator->floordestheight = roversector->floorheight;
	elevator->actionsector = roversector;
	elevator->distance = TICRATE; // Used for delay time
	elevator->low = 0;
	elevator->player = player;

	if(floating)
	{
		elevator->high = 42;
		elevator->sector->teamstartsec = 1;
	}

	P_SpawnMobj(roversector->soundorg.x, roversector->soundorg.y, roverheight, MT_CRUMBLEOBJ);

	return 1;
}

int EV_StartNoReturnCrumble
( sector_t*  sec, sector_t* roversector, fixed_t roverheight, boolean floating, player_t* player)
{
  elevator_t*           elevator;

    // If floor is already activated, skip it
    if (sec->floordata) //jff 2/22/98
      return 0;
   
    // create and initialize new elevator thinker

	elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
	P_AddThinker (&elevator->thinker);
	sec->floordata = elevator; //jff 2/22/98
    elevator->thinker.function.acp1 = (actionf_p1) T_StartCrumble;
    elevator->type = elevateContinuous;

    // set up the fields according to the type of elevator action
	elevator->sector = sec;
	elevator->speed = 0;
	elevator->direction = -1; // Down
	elevator->floorwasheight = elevator->sector->floorheight;
	elevator->ceilingwasheight = elevator->sector->ceilingheight;
	elevator->floordestheight = roversector->floorheight;
	elevator->actionsector = roversector;
	elevator->distance = TICRATE; // Used for delay time
	elevator->low = 0;
	elevator->player = player;

	if(floating)
	{
		elevator->high = 42;
		elevator->sector->teamstartsec = 1;
	}

	P_SpawnMobj(roversector->soundorg.x, roversector->soundorg.y, roverheight, MT_CRUMBLEOBJ);

	return 1;
}

int EV_AirBob
( sector_t*  sec, player_t* player, int amount, boolean reverse)
{
  elevator_t*           elevator;

    // If ceiling is already activated, skip it
    if (sec->ceilingdata) //jff 2/22/98
      return 0;
      
    // create and initialize new elevator thinker

	elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
	P_AddThinker (&elevator->thinker);
	sec->ceilingdata = elevator; //jff 2/22/98

	// Graue 11-07-2003
	if(reverse)
		elevator->thinker.function.acp1 = (actionf_p1) T_AirBobReverse;
	else
		elevator->thinker.function.acp1 = (actionf_p1) T_AirBob;

    elevator->type = elevateBounce;

	elevator->player = player; // Pass the reference!

    // set up the fields according to the type of elevator action
	elevator->sector = sec;
	elevator->speed = FRACUNIT/NEWTICRATERATIO;
	elevator->direction = -1; // Down
	elevator->floorwasheight = P_FindHighestFloorSurrounding(elevator->sector);
	elevator->ceilingwasheight = P_FindHighestCeilingSurrounding(elevator->sector);
	// use amount instead of just 16 all the time Graue 11-04-2003
	elevator->distance = elevator->floorwasheight - amount*FRACUNIT
		* (reverse ? -1 : 1); // How far to sag down
	elevator->low = 1;
	elevator->high = 0;

	return 1;
}

int EV_MarioBlock
( sector_t*  sec, sector_t* roversector, fixed_t topheight, struct line_s* masterline, mobj_t* puncher )
{
    elevator_t*           elevator;
    mobj_t   *thing;
    msecnode_t* node;
	fixed_t oldx,oldy,oldz;

    if (sec->floordata || sec->ceilingdata) //jff 2/22/98
      return 0;

	node = NULL;
	thing = NULL;

	node = sec->touching_thinglist; // things touching this sector

	if(node != NULL)
	{
		thing = node->m_thing;

		if(thing != NULL)
		{
			if((thing->flags & MF_MONITOR) && thing->threshold == 68)
			{
				// "Thunk!" sound
				S_StartSound(puncher, sfx_tink); // Puncher is "close enough".
				return 1;
			}
			// create and initialize new elevator thinker

			elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
			P_AddThinker (&elevator->thinker);
			sec->floordata = elevator; //jff 2/22/98
			sec->ceilingdata = elevator; //jff 2/22/98
			elevator->thinker.function.acp1 = (actionf_p1) T_MarioBlock;
			elevator->type = elevateBounce;

			// set up the fields according to the type of elevator action
			elevator->sector = sec;
			elevator->speed = (4*FRACUNIT)/NEWTICRATERATIO;
			elevator->direction = 1; // Up
			elevator->floorwasheight = elevator->sector->floorheight;
			elevator->ceilingwasheight = elevator->sector->ceilingheight;
			elevator->distance = FRACUNIT;
			elevator->low = 1;
			
			if((thing->flags & MF_MONITOR))
			{
				oldx = thing->x;
				oldy = thing->y;
				oldz = thing->z;
			}

			P_UnsetThingPosition(thing);
			thing->x = roversector->soundorg.x;
			thing->y = roversector->soundorg.y;
			thing->z = topheight;
			thing->momz = 6*FRACUNIT;
			P_SetThingPosition(thing);
			if(thing->flags & MF_SHOOTABLE)
				P_DamageMobj(thing, puncher, puncher, 1);
			else if(thing->type == MT_RING || thing->type == MT_COIN)
			{
				thing->momz = 3*FRACUNIT;
				P_TouchSpecialThing(thing, puncher, false);
				// "Thunk!" sound
				S_StartSound(puncher, sfx_tink); // Puncher is "close enough"
			}
			else
			{
				// "Powerup rise" sound
				S_StartSound(puncher, sfx_cgot); // Puncher is "close enough"
			}

			if((thing->flags & MF_MONITOR))
			{
				P_UnsetThingPosition(thing->tracer);
				thing->tracer->x = oldx;
				thing->tracer->y = oldy;
				thing->tracer->z = oldz;
				thing->tracer->momx = 1;
				thing->tracer->momy = 1;
				P_SetThingPosition(thing->tracer);
			}
		}
/*
		node = NULL;
		node = sec->touching_thinglist;

		if(node == NULL)
		{
			sides[masterline->sidenum[0]].midtexture = sides[masterline->sidenum[0]].toptexture;
		}*/
	}
	else
	{
		// "Thunk!" sound
		S_StartSound(puncher, sfx_tink); // Puncher is "close enough".
	}

	return 1;
}