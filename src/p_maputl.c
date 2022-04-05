// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
//-----------------------------------------------------------------------------
/// \file
/// \brief Movement/collision utility functions, as used by functions in p_map.c
/// 
///	Blockmap iterator functions, and some PIT_* functions to use for iteration

#include "p_local.h"
#include "r_main.h"
#include "p_maputl.h"

//
// P_AproxDistance
// Gives an estimation of distance (not exact)
//

fixed_t P_AproxDistance(fixed_t dx, fixed_t dy)
{
	dx = abs(dx);
	dy = abs(dy);
	if(dx < dy)
		return dx + dy - (dx>>1);
    return dx + dy - (dy>>1);
}

//
// P_PointOnLineSide
// Returns 0 or 1
//
int P_PointOnLineSide(fixed_t x, fixed_t y, line_t* line)
{
	fixed_t dx, dy, left, right;

	if(!line->dx)
	{
		if(x <= line->v1->x)
			return line->dy > 0;

		return line->dy < 0;
	}
	if(!line->dy)
	{
		if(y <= line->v1->y)
			return line->dx < 0;

		return line->dx > 0;
	}

	dx = (x - line->v1->x);
	dy = (y - line->v1->y);

	left = FixedMul(line->dy>>FRACBITS, dx);
	right = FixedMul(dy, line->dx>>FRACBITS);

	if(right < left)
		return 0; // front side
	return 1; // back side
}

//
// P_BoxOnLineSide
// Considers the line to be infinite
// Returns side 0 or 1, -1 if box crosses the line.
//
int P_BoxOnLineSide(fixed_t* tmbox, line_t* ld)
{
	int p1, p2;

	switch(ld->slopetype)
	{
		case ST_HORIZONTAL:
			p1 = tmbox[BOXTOP] > ld->v1->y;
			p2 = tmbox[BOXBOTTOM] > ld->v1->y;
			if(ld->dx < 0)
			{
				p1 ^= 1;
				p2 ^= 1;
			}
			break;

		case ST_VERTICAL:
			p1 = tmbox[BOXRIGHT] < ld->v1->x;
			p2 = tmbox[BOXLEFT] < ld->v1->x;
			if(ld->dy < 0)
			{
				p1 ^= 1;
				p2 ^= 1;
			}
			break;

		case ST_POSITIVE:
			p1 = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld);
			p2 = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld);
			break;

		case ST_NEGATIVE:
			p1 = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld);
			p2 = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld);
			break;

		default:
			I_Error("P_BoxOnLineSide: unknown slopetype %d\n", ld->slopetype);
			return -1;
	}

	if(p1 == p2)
		return p1;
	return -1;
}

//
// P_PointOnDivlineSide
// Returns 0 or 1.
//
int P_PointOnDivlineSide(fixed_t x, fixed_t y, divline_t* line)
{
	fixed_t dx, dy, left, right;

	if(!line->dx)
	{
		if(x <= line->x)
			return line->dy > 0;

		return line->dy < 0;
	}
	if(!line->dy)
	{
		if(y <= line->y)
			return line->dx < 0;

		return line->dx > 0;
	}

	dx = (x - line->x);
	dy = (y - line->y);

	// try to quickly decide by looking at sign bits
	if((line->dy ^ line->dx ^ dx ^ dy) & 0x80000000)
	{
		if((line->dy ^ dx) & 0x80000000)
			return 1; // left is negative
		return 0;
	}

	left = FixedMul(line->dy>>8, dx>>8);
	right = FixedMul(dy>>8, line->dx>>8);

	if(right < left)
		return 0; // front side
	return 1; // back side
}

//
// P_MakeDivline
//
void P_MakeDivline(line_t* li, divline_t* dl)
{
	dl->x = li->v1->x;
	dl->y = li->v1->y;
	dl->dx = li->dx;
	dl->dy = li->dy;
}

//
// P_InterceptVector
// Returns the fractional intercept point along the first divline.
// This is only called by the addthings and addlines traversers.
//
fixed_t P_InterceptVector(divline_t* v2, divline_t* v1)
{
	fixed_t frac, num, den;

	den = FixedMul(v1->dy>>8, v2->dx) - FixedMul(v1->dx>>8, v2->dy);

	if(!den)
		return 0;

	num = FixedMul((v1->x - v2->x)>>8,v1->dy) + FixedMul((v2->y - v1->y)>>8, v1->dx);
	frac = FixedDiv(num, den);

    return frac;
}

//
// P_LineOpening
// Sets opentop and openbottom to the window through a two sided line.
// OPTIMIZE: keep this precalculated
//
fixed_t opentop, openbottom, openrange, lowfloor;

// P_CameraLineOpening
// P_LineOpening, but for camera
// Tails 09-29-2002
void P_CameraLineOpening(line_t* linedef)
{
	sector_t* front;
	sector_t* back;
	fixed_t frontfloor, frontceiling, backfloor, backceiling;

	if(linedef->sidenum[1] == -1)
	{
		// single sided line
		openrange = 0;
		return;
	}

	front = linedef->frontsector;
	back = linedef->backsector;

	// Cameras use the heightsec's heights rather then the actual sector heights.
	// If you can see through it, why not move the camera through it too?
	if(front->heightsec >= 0)
	{
		frontfloor = sectors[front->heightsec].floorheight;
		frontceiling = sectors[front->heightsec].ceilingheight;
	}
	else
	{
		frontfloor = front->floorheight;
		frontceiling = front->ceilingheight;
	}
	if(back->heightsec >= 0)
	{
		backfloor = sectors[back->heightsec].floorheight;
		backceiling = sectors[back->heightsec].ceilingheight;
	}
	else
	{
		backfloor = back->floorheight;
		backceiling = back->ceilingheight;
	}

	{
		fixed_t thingbot, thingtop;

		thingbot = camera.z;
		thingtop = thingbot + camera.height;

		if(frontceiling < backceiling)
			opentop = frontceiling;
		else
			opentop = backceiling;

		if(frontfloor > backfloor)
		{
			openbottom = frontfloor;
			lowfloor = backfloor;
		}
		else
		{
			openbottom = backfloor;
			lowfloor = frontfloor;
		}

		// Check for fake floors in the sector.
		if(front->ffloors || back->ffloors)
		{
			ffloor_t* rover;
			fixed_t lowestceiling = opentop;
			fixed_t highestfloor = openbottom;
			fixed_t lowestfloor = lowfloor;
			fixed_t delta1, delta2;

			thingtop = camera.z + camera.height;

			// Check for frontsector's fake floors
			if(front->ffloors)
				for(rover = front->ffloors; rover; rover = rover->next)
				{
					if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_RENDERALL) || !(rover->flags & FF_EXISTS))
						continue;

					delta1 = abs(camera.z - ((*rover->bottomheight + *rover->topheight) / 2));
					delta2 = abs(thingtop - ((*rover->bottomheight + *rover->topheight) / 2));
					if(*rover->bottomheight < lowestceiling && delta1 >= delta2)
						lowestceiling = *rover->bottomheight;

					if(*rover->topheight > highestfloor && delta1 < delta2)
						highestfloor = *rover->topheight;
					else if(*rover->topheight > lowestfloor && delta1 < delta2)
						lowestfloor = *rover->topheight;
				}

			// Check for backsectors fake floors
			if(back->ffloors)
				for(rover = back->ffloors; rover; rover = rover->next)
				{
					if(!(rover->flags & FF_SOLID) || !(rover->flags & FF_RENDERALL) || !(rover->flags & FF_EXISTS))
						continue;

					delta1 = abs(camera.z - ((*rover->bottomheight + *rover->topheight) / 2));
					delta2 = abs(thingtop - ((*rover->bottomheight + *rover->topheight) / 2));
					if(*rover->bottomheight < lowestceiling && delta1 >= delta2)
						lowestceiling = *rover->bottomheight;

					if(*rover->topheight > highestfloor && delta1 < delta2)
						highestfloor = *rover->topheight;
					else if(*rover->topheight > lowestfloor && delta1 < delta2)
						lowestfloor = *rover->topheight;
				}

			if(highestfloor > openbottom)
				openbottom = highestfloor;

			if(lowestceiling < opentop)
				opentop = lowestceiling;

			if(lowestfloor > lowfloor)
				lowfloor = lowestfloor;
		}
		openrange = opentop - openbottom;
		return;
	}
}

void P_LineOpening (line_t* linedef)
{
    sector_t*      front;
    sector_t*      back;

    if(linedef->sidenum[1] == -1)
    {
        // single sided line
        openrange = 0;
        return;
    }

    front = linedef->frontsector;
    back = linedef->backsector;
#ifdef PARANOIA
    if(!front)
        I_Error("lindef without front");
    if(!back)
        I_Error("lindef without back");
#endif

    if(tmthing)
    {
      fixed_t        thingbot, thingtop;

      thingbot = tmthing->z;
      thingtop = thingbot + tmthing->height;

      if(front->ceilingheight < back->ceilingheight)
        opentop = front->ceilingheight;
      else
        opentop = back->ceilingheight;

      if(front->floorheight > back->floorheight)
      {
        openbottom = front->floorheight;
        lowfloor = back->floorheight;
      }
      else
      {
        openbottom = back->floorheight;
        lowfloor = front->floorheight;
      }

      //SoM: 3/27/2000: Check for fake floors in the sector.
      if(front->ffloors || back->ffloors)
      {
        ffloor_t*      rover;

        fixed_t    lowestceiling = opentop;
        fixed_t    highestfloor = openbottom;
        fixed_t    lowestfloor = lowfloor;
        fixed_t    delta1;
        fixed_t    delta2;

        if(!tmthing)
          goto no_thing;

        thingtop = tmthing->z + tmthing->height;

        // Check for frontsector's fake floors
        if(front->ffloors)
          for(rover = front->ffloors; rover; rover = rover->next)
          {
            if(!(rover->flags & FF_SOLID)) continue;

            delta1 = abs(tmthing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
            delta2 = abs(thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
            if(*rover->bottomheight < lowestceiling && delta1 >= delta2)
			{
				if(!(rover->flags & FF_PLATFORM))
					lowestceiling = *rover->bottomheight;
			}

            if(*rover->topheight > highestfloor && delta1 < delta2)
              highestfloor = *rover->topheight;
            else if(*rover->topheight > lowestfloor && delta1 < delta2)
              lowestfloor = *rover->topheight;
          }

        // Check for backsectors fake floors
        if(back->ffloors)
          for(rover = back->ffloors; rover; rover = rover->next)
          {
            if(!(rover->flags & FF_SOLID))
              continue;

            delta1 = abs(tmthing->z - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
            delta2 = abs(thingtop - (*rover->bottomheight + ((*rover->topheight - *rover->bottomheight)/2)));
            if(*rover->bottomheight < lowestceiling && delta1 >= delta2)
			{
				if(!(rover->flags & FF_PLATFORM))
					lowestceiling = *rover->bottomheight;
			}

            if(*rover->topheight > highestfloor && delta1 < delta2)
              highestfloor = *rover->topheight;
            else if(*rover->topheight > lowestfloor && delta1 < delta2)
              lowestfloor = *rover->topheight;
          }

        if(highestfloor > openbottom)
          openbottom = highestfloor;

        if(lowestceiling < opentop)
          opentop = lowestceiling;

        if(lowestfloor > lowfloor)
          lowfloor = lowestfloor;
      }
      openrange = opentop - openbottom;
      return;
    }

    if(front->ceilingheight < back->ceilingheight)
        opentop = front->ceilingheight;
    else
        opentop = back->ceilingheight;

    if(front->floorheight > back->floorheight)
    {
        openbottom = front->floorheight;
        lowfloor = back->floorheight;
    }
    else
    {
        openbottom = back->floorheight;
        lowfloor = front->floorheight;
    }

    no_thing:

    openrange = opentop - openbottom;
}


//
// THING POSITION SETTING
//

//
// P_UnsetThingPosition
// Unlinks a thing from block map and sectors.
// On each position change, BLOCKMAP and other
// lookups maintaining lists ot things inside
// these structures need to be updated.
//
void P_UnsetThingPosition (mobj_t* thing)
{
    int         blockx;
    int         blocky;

	if(!thing)
		return;

	if(!thing->subsector || !thing->subsector->sector)
		return;

    if( ! (thing->flags & MF_NOSECTOR) )
    {
        // inert things don't need to be in blockmap?
        // unlink from subsector
        if(thing->snext)
            thing->snext->sprev = thing->sprev;

        if(thing->sprev)
            thing->sprev->snext = thing->snext;
        else
            thing->subsector->sector->thinglist = thing->snext;
//#ifdef PARANOIA
        thing->sprev = NULL;
        thing->snext = NULL;
//#endif
        //SoM: 4/7/2000
        //
        // Save the sector list pointed to by touching_sectorlist.
        // In P_SetThingPosition, we'll keep any nodes that represent
        // sectors the Thing still touches. We'll add new ones then, and
        // delete any nodes for sectors the Thing has vacated. Then we'll
        // put it back into touching_sectorlist. It's done this way to
        // avoid a lot of deleting/creating for nodes, when most of the
        // time you just get back what you deleted anyway.
        //
        // If this Thing is being removed entirely, then the calling
        // routine will clear out the nodes in sector_list.

        sector_list = thing->touching_sectorlist;
        thing->touching_sectorlist = NULL; //to be restored by P_SetThingPosition
    }

    if(!(thing->flags & MF_NOBLOCKMAP))
    {
        // inert things don't need to be in blockmap
        // unlink from block map
        if(thing->bnext)
            thing->bnext->bprev = thing->bprev;

        if(thing->bprev)
            thing->bprev->bnext = thing->bnext;
        else
        {
            blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
            blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;

            if(blockx>=0 && blockx < bmapwidth
                && blocky>=0 && blocky <bmapheight)
            {
                blocklinks[blocky*bmapwidth+blockx] = thing->bnext;
            }
        }
    }
}

void P_UnsetPrecipThingPosition (precipmobj_t* thing)
{
	if(!thing)
		return;

	if(!thing->subsector || !thing->subsector->sector)
		return;

    // inert things don't need to be in blockmap?
    // unlink from subsector
    if(thing->snext)
        thing->snext->sprev = thing->sprev;

    if(thing->sprev)
        thing->sprev->snext = thing->snext;
    else
        thing->subsector->sector->preciplist = thing->snext;

    thing->sprev = NULL;
    thing->snext = NULL;

    precipsector_list = thing->touching_sectorlist;
    thing->touching_sectorlist = NULL; //to be restored by P_SetThingPosition
}

//
// P_SetThingPosition
// Links a thing into both a block and a subsector
// based on it's x y.
// Sets thing->subsector properly
//
void P_SetThingPosition (mobj_t* thing)
{
    subsector_t*        ss;
    sector_t*           sec;
    int                 blockx;
    int                 blocky;
    mobj_t**            link;
    sector_t*           oldsec;

    if(thing->player && thing->z <= thing->floorz && thing->subsector)
        oldsec = thing->subsector->sector;
    else
        oldsec = NULL;

    // link into subsector
    ss = R_PointInSubsector (thing->x,thing->y);
    thing->subsector = ss;

    if( ! (thing->flags & MF_NOSECTOR) )
    {
        // invisible things don't go into the sector links
        sec = ss->sector;
#ifdef PARANOIA
        if( thing->sprev != NULL || thing->snext != NULL )
    {
        CONS_Printf("linking a thing(%d) that is already linked", thing->type);
        P_UnsetThingPosition(thing);
        P_SetThingPosition(thing);
        return;
//            I_Error("linking a think(%d) that is already linked",thing->type);
    }
#endif

        thing->sprev = NULL;
        thing->snext = sec->thinglist;

        if(sec->thinglist)
            sec->thinglist->sprev = thing;

        sec->thinglist = thing;

        //SoM: 4/6/2000
        //
        // If sector_list isn't NULL, it has a collection of sector
        // nodes that were just removed from this Thing.

        // Collect the sectors the object will live in by looking at
        // the existing sector_list and adding new nodes and deleting
        // obsolete ones.

        // When a node is deleted, its sector links (the links starting
        // at sector_t->touching_thinglist) are broken. When a node is
        // added, new sector links are created.

        P_CreateSecNodeList(thing,thing->x,thing->y);
        thing->touching_sectorlist = sector_list; // Attach to Thing's mobj_t
        sector_list = NULL; // clear for next time
    }


    // link into blockmap
    if(!(thing->flags & MF_NOBLOCKMAP))
    {
        // inert things don't need to be in blockmap
        blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
        blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;

        if(blockx>=0
            && blockx < bmapwidth
            && blocky>=0
            && blocky < bmapheight)
        {
            link = &blocklinks[blocky*bmapwidth+blockx];
            thing->bprev = NULL;
            thing->bnext = *link;
            if(*link)
                (*link)->bprev = thing;

            *link = thing;
        }
        else
        {
            // thing is off the map
            thing->bnext = thing->bprev = NULL;
        }
    }

    // Allows you to 'step' on a new linedef exec when the previous sector's floor is the same height.
    if(thing->player && oldsec != NULL && thing->subsector && oldsec != thing->subsector->sector && thing->z <= thing->subsector->sector->floorheight)
         thing->eflags |= MF_JUSTSTEPPEDDOWN;
}

// Special function for precipitation Tails 08-19-2002
void P_SetPrecipitationThingPosition (precipmobj_t* thing)
{
    subsector_t*        ss;
    sector_t*           sec;

    // link into subsector
    ss = R_PointInSubsector (thing->x,thing->y);
    thing->subsector = ss;

    // invisible things don't go into the sector links
    sec = ss->sector;

    thing->sprev = NULL;
    thing->snext = sec->preciplist;

    if(sec->preciplist)
        sec->preciplist->sprev = thing;

    sec->preciplist = thing;

    P_CreatePrecipSecNodeList(thing,thing->x,thing->y);
    thing->touching_sectorlist = precipsector_list; // Attach to Thing's mobj_t
    precipsector_list = NULL; // clear for next time
}


//
// BLOCK MAP ITERATORS
// For each line/thing in the given mapblock,
// call the passed PIT_* function.
// If the function returns false,
// exit with false without checking anything else.
//


//
// P_BlockLinesIterator
// The validcount flags are used to avoid checking lines
// that are marked in multiple mapblocks,
// so increment validcount before the first call
// to P_BlockLinesIterator, then make one or more calls
// to it.
//
boolean P_BlockLinesIterator (int       x,
                              int       y,
                              boolean   (*func)(line_t*) )
{
    int                 offset;
    const long*              list; // Big blockmap Tails
    line_t*             ld;

    if(x<0
        || y<0
        || x>=bmapwidth
        || y>=bmapheight)
    {
        return true;
    }

    offset = y*bmapwidth+x;
    offset = *(blockmap+offset); //	offset = blockmap[y*bmapwidth+x];

    //Hurdler: FIXME: this a temporary "fix" for the bug with phobia...
    //                ... but it's not correct!!!!!
    if(offset < 0)
    {
		static int first = 1;
		if(first)
		{
			CONS_Printf("Warning: this map has reached a limit of the doom engine.\n");
			first = 0;
		}
        return true;
    }

    for ( list = blockmaplump+offset ; *list != -1 ; list++)
    {
        ld = &lines[*list];

        if(ld->validcount == validcount)
            continue;   // line has already been checked

        ld->validcount = validcount;

        if(!func(ld))
            return false;
    }
    return true;        // everything was checked
}


//
// P_BlockThingsIterator
//
boolean P_BlockThingsIterator ( int                   x,
                                int                   y,
                                boolean(*func)(mobj_t*) )
{
    mobj_t*             mobj;
/*	    thinker_t*  th;
		boolean foundit = false;
*/
    if( x<0
         || y<0
         || x>=bmapwidth
         || y>=bmapheight)
    {
        return true;
    }

    //added:15-02-98: check interaction (ligne de tir, ...)
    //                avec les objets dans le blocmap
    for (mobj = blocklinks[y*bmapwidth+x] ;
         mobj ;
         mobj = mobj->bnext)
    {
/*
		// scan the remaining thinkers
		for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
		{
			if(th->function.acp1 != (actionf_p1)P_MobjThinker)
				continue;

			if(((mobj_t *)th) == mobj)
			{
				foundit = true;
				break;
			}
		}

		if(foundit == false)
		{
			CONS_Printf("Ack! The mobj number is %d!\n", mobj->type);
			return true;
		}
*/
        if(!func( mobj ) )
            return false;
    }
    return true;
}



//
// INTERCEPT ROUTINES
//

//SoM: 4/6/2000: Limit removal
intercept_t*    intercepts = NULL;
intercept_t*    intercept_p = NULL;

divline_t trace;
static boolean earlyout;



//SoM: 4/6/2000: Remove limit on intercepts.
static void P_CheckIntercepts(void)
{
  static size_t max_intercepts = 0;
  size_t count = intercept_p - intercepts;

  if(max_intercepts <= count)
  {
    if(!max_intercepts)
      max_intercepts = 128;
    else
      max_intercepts = max_intercepts * 2;

    intercepts = realloc(intercepts, sizeof(intercept_t) * max_intercepts);
    intercept_p = intercepts + count;
  }
}

//
// PIT_AddLineIntercepts.
// Looks for lines in the given block
// that intercept the given trace
// to add to the intercepts list.
//
// A line is crossed if its endpoints
// are on opposite sides of the trace.
// Returns true if earlyout and a solid line hit.
//
static boolean
PIT_AddLineIntercepts (line_t* ld)
{
    int                 s1;
    int                 s2;
    fixed_t             frac;
    divline_t           dl;

    // avoid precision problems with two routines
    if( trace.dx > FRACUNIT*16
         || trace.dy > FRACUNIT*16
         || trace.dx < -FRACUNIT*16
         || trace.dy < -FRACUNIT*16)
    {
        //Hurdler: crash here with phobia when you shoot on the door next the stone bridge
        //stack overflow???
        s1 = P_PointOnDivlineSide (ld->v1->x, ld->v1->y, &trace);
        s2 = P_PointOnDivlineSide (ld->v2->x, ld->v2->y, &trace);
    }
    else
    {
        s1 = P_PointOnLineSide (trace.x, trace.y, ld);
        s2 = P_PointOnLineSide (trace.x+trace.dx, trace.y+trace.dy, ld);
    }

    if(s1 == s2)
        return true;    // line isn't crossed

    // hit the line
    P_MakeDivline (ld, &dl);
    frac = P_InterceptVector (&trace, &dl);

    if(frac < 0)
        return true;    // behind source

    // try to early out the check
    if(earlyout
        && frac < FRACUNIT
        && !ld->backsector)
    {
        return false;   // stop checking
    }

    //SoM: 4/6/2000: Limit removal
    P_CheckIntercepts();

    intercept_p->frac = frac;
    intercept_p->isaline = true;
    intercept_p->d.line = ld;
    intercept_p++;

    return true;        // continue
}



//
// PIT_AddThingIntercepts
//
static boolean PIT_AddThingIntercepts (mobj_t* thing)
{
    fixed_t             x1;
    fixed_t             y1;
    fixed_t             x2;
    fixed_t             y2;

    int                 s1;
    int                 s2;

    boolean             tracepositive;

    divline_t           dl;

    fixed_t             frac;

    tracepositive = (trace.dx ^ trace.dy)>0;

    // check a corner to corner crossection for hit
    if(tracepositive)
    {
        x1 = thing->x - thing->radius;
        y1 = thing->y + thing->radius;

        x2 = thing->x + thing->radius;
        y2 = thing->y - thing->radius;
    }
    else
    {
        x1 = thing->x - thing->radius;
        y1 = thing->y - thing->radius;

        x2 = thing->x + thing->radius;
        y2 = thing->y + thing->radius;
    }

    s1 = P_PointOnDivlineSide (x1, y1, &trace);
    s2 = P_PointOnDivlineSide (x2, y2, &trace);

    if(s1 == s2)
        return true;            // line isn't crossed

    dl.x = x1;
    dl.y = y1;
    dl.dx = x2-x1;
    dl.dy = y2-y1;

    frac = P_InterceptVector (&trace, &dl);

    if(frac < 0)
        return true;            // behind source

    P_CheckIntercepts();

    intercept_p->frac = frac;
    intercept_p->isaline = false;
    intercept_p->d.thing = thing;
    intercept_p++;

    return true;                // keep going
}

//
// P_TraverseIntercepts
// Returns true if the traverser function returns true
// for all lines.
//
static boolean P_TraverseIntercepts ( traverser_t   func,
                               fixed_t       maxfrac )
{
    size_t              count;
    fixed_t             dist;
    intercept_t*        scan;
    intercept_t*        in;

    count = intercept_p - intercepts;

    in = 0;                     // shut up compiler warning

    while (count--)
    {
        dist = MAXINT;
        for (scan = intercepts ; scan<intercept_p ; scan++)
        {
            if(scan->frac < dist)
            {
                dist = scan->frac;
                in = scan;
            }
        }

        if(dist > maxfrac)
            return true;        // checked everything in range

#if 0  // UNUSED
    {
        // don't check these yet, there may be others inserted
        in = scan = intercepts;
        for ( scan = intercepts ; scan<intercept_p ; scan++)
            if(scan->frac > maxfrac)
                *in++ = *scan;
        intercept_p = in;
        return false;
    }
#endif

        // appelle la fonction en commencant par l' intercept_t le plus
        // proche
        if( !func (in) )
            return false;       // don't bother going farther

        in->frac = MAXINT;
    }

    return true;                // everything was traversed
}

//
// P_PathTraverse
// Traces a line from x1,y1 to x2,y2,
// calling the traverser function for each.
// Returns true if the traverser function returns true
// for all lines.
//
boolean P_PathTraverse ( fixed_t       x1,
                         fixed_t       y1,
                         fixed_t       x2,
                         fixed_t       y2,
                         int           flags,
                         traverser_t   trav)
{
    fixed_t     xt1;
    fixed_t     yt1;
    fixed_t     xt2;
    fixed_t     yt2;

    fixed_t     xstep;
    fixed_t     ystep;

    fixed_t     partial;

    fixed_t     xintercept;
    fixed_t     yintercept;

    int         mapx;
    int         mapy;

    int         mapxstep;
    int         mapystep;

    int         count;

    earlyout = flags & PT_EARLYOUT;

    validcount++;
    intercept_p = intercepts;

    if(((x1-bmaporgx)&(MAPBLOCKSIZE-1)) == 0)
        x1 += FRACUNIT; // don't side exactly on a line

    if(((y1-bmaporgy)&(MAPBLOCKSIZE-1)) == 0)
        y1 += FRACUNIT; // don't side exactly on a line

    trace.x = x1;
    trace.y = y1;
    trace.dx = x2 - x1;
    trace.dy = y2 - y1;

    x1 -= bmaporgx;
    y1 -= bmaporgy;
    xt1 = x1>>MAPBLOCKSHIFT;
    yt1 = y1>>MAPBLOCKSHIFT;

    x2 -= bmaporgx;
    y2 -= bmaporgy;
    xt2 = x2>>MAPBLOCKSHIFT;
    yt2 = y2>>MAPBLOCKSHIFT;

    if(xt2 > xt1)
    {
        mapxstep = 1;
        partial = FRACUNIT - ((x1>>MAPBTOFRAC)&FRACMASK);
        ystep = FixedDiv (y2-y1,abs(x2-x1));
    }
    else if(xt2 < xt1)
    {
        mapxstep = -1;
        partial = (x1>>MAPBTOFRAC)&FRACMASK;
        ystep = FixedDiv (y2-y1,abs(x2-x1));
    }
    else
    {
        mapxstep = 0;
        partial = FRACUNIT;
        ystep = 256*FRACUNIT;
    }

    yintercept = (y1>>MAPBTOFRAC) + FixedMul (partial, ystep);


    if(yt2 > yt1)
    {
        mapystep = 1;
        partial = FRACUNIT - ((y1>>MAPBTOFRAC)&FRACMASK);
        xstep = FixedDiv (x2-x1,abs(y2-y1));
    }
    else if(yt2 < yt1)
    {
        mapystep = -1;
        partial = (y1>>MAPBTOFRAC)&FRACMASK;
        xstep = FixedDiv (x2-x1,abs(y2-y1));
    }
    else
    {
        mapystep = 0;
        partial = FRACUNIT;
        xstep = 256*FRACUNIT;
    }
    xintercept = (x1>>MAPBTOFRAC) + FixedMul (partial, xstep);

    // Step through map blocks.
    // Count is present to prevent a round off error
    // from skipping the break.
    mapx = xt1;
    mapy = yt1;

    for (count = 0 ; count < 64 ; count++)
    {
        if(flags & PT_ADDLINES)
        {
            if(!P_BlockLinesIterator (mapx, mapy,PIT_AddLineIntercepts))
                return false;   // early out
        }

        if(flags & PT_ADDTHINGS)
        {
            if(!P_BlockThingsIterator (mapx, mapy,PIT_AddThingIntercepts))
                return false;   // early out
        }

        if(mapx == xt2
            && mapy == yt2)
        {
            break;
        }

        if((yintercept >> FRACBITS) == mapy)
        {
            yintercept += ystep;
            mapx += mapxstep;
        }
        else if((xintercept >> FRACBITS) == mapx)
        {
            xintercept += xstep;
            mapy += mapystep;
        }

    }
    // go through the sorted list
    return P_TraverseIntercepts ( trav, FRACUNIT );
}


// =========================================================================
//                                                        BLOCKMAP ITERATORS
// =========================================================================

// blockmap iterator for all sorts of use
// your routine must return FALSE to exit the loop earlier
// returns FALSE if the loop exited early after a false return
// value from your user function

//abandoned, maybe I'll need it someday..
/*
boolean P_RadiusLinesCheck (  fixed_t    radius,
                              fixed_t    x,
                              fixed_t    y,
                              boolean   (*func)(line_t*))
{
    int   xl, xh, yl, yh;
    int   bx, by;

    tmbbox[BOXTOP] = y + radius;
    tmbbox[BOXBOTTOM] = y - radius;
    tmbbox[BOXRIGHT] = x + radius;
    tmbbox[BOXLEFT] = x - radius;

    // check lines
    xl = (tmbbox[BOXLEFT] - bmaporgx)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy)>>MAPBLOCKSHIFT;

    for (bx=xl ; bx<=xh ; bx++)
        for (by=yl ; by<=yh ; by++)
            if(!P_BlockLinesIterator (bx,by,func))
                return false;
    return true;
}
*/
