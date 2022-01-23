// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: r_bsp.c,v 1.16 2000/11/21 21:13:17 stroggonmeth Exp $
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
// $Log: r_bsp.c,v $
// Revision 1.16  2000/11/21 21:13:17  stroggonmeth
// Optimised 3D floors and fixed crashing bug in high resolutions.
//
// Revision 1.15  2000/11/09 17:56:20  stroggonmeth
// Hopefully fixed a few bugs and did a few optimizations.
//
// Revision 1.14  2000/11/02 19:49:36  bpereira
// no message
//
// Revision 1.13  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.12  2000/05/23 15:22:34  stroggonmeth
// Not much. A graphic bug fixed.
//
// Revision 1.11  2000/05/03 23:51:01  stroggonmeth
// A few, quick, changes.
//
// Revision 1.10  2000/04/20 21:47:24  stroggonmeth
// no message
//
// Revision 1.9  2000/04/18 17:39:39  stroggonmeth
// Bug fixes and performance tuning.
//
// Revision 1.8  2000/04/15 22:12:58  stroggonmeth
// Minor bug fixes
//
// Revision 1.7  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.6  2000/04/11 19:07:25  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.5  2000/04/06 21:06:19  stroggonmeth
// Optimized extra_colormap code...
// Added #ifdefs for older water code.
//
// Revision 1.4  2000/04/04 19:28:43  stroggonmeth
// Global colormaps working. Added a new linedef type 272.
//
// Revision 1.3  2000/04/04 00:32:47  stroggonmeth
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
//      BSP traversal, handling of LineSegs for rendering.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "r_local.h"
#include "r_state.h"

#include "r_splats.h"
#include "p_local.h"  //SoM: 4/10/2000: camera
#include "z_zone.h"   //SoM: Check R_Prep3DFloors

seg_t*          curline;
side_t*         sidedef;
line_t*         linedef;
sector_t*       frontsector;
sector_t*       backsector;



//faB:  very ugly realloc() of drawsegs at run-time, I upped it to 512
//      instead of 256.. and someone managed to send me a level with
//      896 drawsegs! So too bad here's a limit removal …-la-Boom
//drawseg_t     drawsegs[MAXDRAWSEGS];
drawseg_t*      drawsegs=NULL;
unsigned        maxdrawsegs;
drawseg_t*      ds_p = NULL;
drawseg_t*      firstnewseg = NULL;


//SoM:3/25/2000: indicates doors closed wrt automap bugfix:
int      doorclosed;

//
// R_ClearDrawSegs
//
void R_ClearDrawSegs (void)
{
    ds_p = drawsegs;
}



//
// ClipWallSegment
// Clips the given range of columns
// and includes it in the new clip list.
//
typedef struct
{
    int first;
    int last;

} cliprange_t;


//SoM: 3/28/2000: Fix from boom.
#define MAXSEGS         MAXVIDWIDTH/2+1

// newend is one past the last valid seg
cliprange_t*    newend;
cliprange_t     solidsegs[MAXSEGS];




//
// R_ClipSolidWallSegment
// Does handle solid walls,
//  e.g. single sided LineDefs (middle texture)
//  that entirely block the view.
//
void R_ClipSolidWallSegment( int first,
  int                   last )
{
    cliprange_t*        next;
    cliprange_t*        start;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    while (start->last < first-1)
        start++;

    if (first < start->first)
    {
        if (last < start->first-1)
        {
            // Post is entirely visible (above start),
            //  so insert a new clippost.
            R_StoreWallRange (first, last);
            next = newend;
            newend++;
            //SoM: 3/28/2000: NO MORE CRASHING!
            if(newend - solidsegs > MAXSEGS)
              I_Error("R_ClipSolidWallSegment: Solid Segs overflow!\n");

            while (next != start)
            {
                *next = *(next-1);
                next--;
            }
            next->first = first;
            next->last = last;
            return;
        }

        // There is a fragment above *start.
        R_StoreWallRange (first, start->first - 1);
        // Now adjust the clip size.
        start->first = first;
    }

    // Bottom contained in start?
    if (last <= start->last)
        return;

    next = start;
    while (last >= (next+1)->first-1)
    {
        // There is a fragment between two posts.
        R_StoreWallRange (next->last + 1, (next+1)->first - 1);
        next++;

        if (last <= next->last)
        {
            // Bottom is contained in next.
            // Adjust the clip size.
            start->last = next->last;
            goto crunch;
        }
    }

    // There is a fragment after *next.
    R_StoreWallRange (next->last + 1, last);
    // Adjust the clip size.
    start->last = last;

    // Remove start+1 to next from the clip list,
    // because start now covers their area.
  crunch:
    if (next == start)
    {
        // Post just extended past the bottom of one post.
        return;
    }


    while (next++ != newend)
    {
        // Remove a post.
        *++start = *next;
    }

    newend = start+1;

    //SoM: 3/28/2000: NO MORE CRASHING!
    if(newend - solidsegs > MAXSEGS)
      I_Error("R_ClipSolidWallSegment: Solid Segs overflow!\n");
}



//
// R_ClipPassWallSegment
// Clips the given range of columns,
//  but does not includes it in the clip list.
// Does handle windows,
//  e.g. LineDefs with upper and lower texture.
//
void
R_ClipPassWallSegment
( int   first,
  int   last )
{
    cliprange_t*        start;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = solidsegs;
    while (start->last < first-1)
        start++;

    if (first < start->first)
    {
        if (last < start->first-1)
        {
            // Post is entirely visible (above start).
            R_StoreWallRange (first, last);
            return;
        }

        // There is a fragment above *start.
        R_StoreWallRange (first, start->first - 1);
    }

    // Bottom contained in start?
    if (last <= start->last)
        return;

    while (last >= (start+1)->first-1)
    {
        // There is a fragment between two posts.
        R_StoreWallRange (start->last + 1, (start+1)->first - 1);
        start++;

        if (last <= start->last)
            return;
    }

    // There is a fragment after *next.
    R_StoreWallRange (start->last + 1, last);
}



//
// R_ClearClipSegs
//
void R_ClearClipSegs (void)
{
    solidsegs[0].first = -0x7fffffff;
    solidsegs[0].last = -1;
    solidsegs[1].first = viewwidth;
    solidsegs[1].last = 0x7fffffff;
    newend = solidsegs+2;
}


//SoM: 3/25/2000
// This function is used to fix the automap bug which
// showed lines behind closed doors simply because the door had a dropoff.
//
// It assumes that Doom has already ruled out a door being closed because
// of front-back closure (e.g. front floor is taller than back ceiling).

int R_DoorClosed(void)
{
  return

    // if door is closed because back is shut:
    backsector->ceilingheight <= backsector->floorheight

    // preserve a kind of transparent door/lift special effect:
    && (backsector->ceilingheight >= frontsector->ceilingheight ||
     curline->sidedef->toptexture)

    && (backsector->floorheight <= frontsector->floorheight ||
     curline->sidedef->bottomtexture)

    // properly render skies (consider door "open" if both ceilings are sky):
    && (backsector->ceilingpic !=skyflatnum ||
        frontsector->ceilingpic!=skyflatnum);
}

//
// If player's view height is underneath fake floor, lower the
// drawn ceiling to be just under the floor height, and replace
// the drawn floor and ceiling textures, and light level, with
// the control sector's.
//
// Similar for ceiling, only reflected.
//
//

sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec,
                     int *floorlightlevel, int *ceilinglightlevel,
                     boolean back)
{
  int        mapnum = -1; //SoM: 4/4/2000
  mobj_t*    viewmobj = camera.chase ? camera.mo : viewplayer->mo;

  if (floorlightlevel)
    *floorlightlevel = sec->floorlightsec == -1 ?
      sec->lightlevel : sectors[sec->floorlightsec].lightlevel;

  if (ceilinglightlevel)
    *ceilinglightlevel = sec->ceilinglightsec == -1 ?
      sec->lightlevel : sectors[sec->ceilinglightsec].lightlevel;

  //SoM: 4/4/2000: If the sector has a midmap, it's probably from 280 type
  if(sec->midmap != -1 && sec->altheightsec == 2)
    mapnum = sec->midmap;

  if (sec->heightsec != -1 && !sec->altheightsec)
    {
      const sector_t *s = &sectors[sec->heightsec];
      int heightsec = viewmobj->subsector->sector->heightsec;
      int underwater = heightsec!=-1 && viewz<=sectors[heightsec].floorheight;

      // Replace sector being drawn, with a copy to be hacked
      *tempsec = *sec;

      // Replace floor and ceiling height with other sector's heights.
      tempsec->floorheight   = s->floorheight;
      tempsec->ceilingheight = s->ceilingheight;

      mapnum = s->midmap;

      if ((underwater && (tempsec->  floorheight = sec->floorheight,
                          tempsec->ceilingheight = s->floorheight-1,
                          !back)) || viewz <= s->floorheight)
        {                   // head-below-floor hack
          tempsec->floorpic    = s->floorpic;
          tempsec->floor_xoffs = s->floor_xoffs;
          tempsec->floor_yoffs = s->floor_yoffs;


          if (underwater)
            {
            if (s->ceilingpic == skyflatnum)
              {
                tempsec->floorheight   = tempsec->ceilingheight+1;
                tempsec->ceilingpic    = tempsec->floorpic;
                tempsec->ceiling_xoffs = tempsec->floor_xoffs;
                tempsec->ceiling_yoffs = tempsec->floor_yoffs;
              }
            else
              {
                tempsec->ceilingpic    = s->ceilingpic;
                tempsec->ceiling_xoffs = s->ceiling_xoffs;
                tempsec->ceiling_yoffs = s->ceiling_yoffs;
              }
              mapnum = s->bottommap;
            }

          tempsec->lightlevel  = s->lightlevel;

          if (floorlightlevel)
            *floorlightlevel = s->floorlightsec == -1 ? s->lightlevel :
            sectors[s->floorlightsec].lightlevel;

          if (ceilinglightlevel)
            *ceilinglightlevel = s->ceilinglightsec == -1 ? s->lightlevel :
            sectors[s->ceilinglightsec].lightlevel;
        }
      else
        if (heightsec != -1 && viewz >= sectors[heightsec].ceilingheight &&
            sec->ceilingheight > s->ceilingheight)
          {   // Above-ceiling hack
            tempsec->ceilingheight = s->ceilingheight;
            tempsec->floorheight   = s->ceilingheight + 1;

            tempsec->floorpic    = tempsec->ceilingpic    = s->ceilingpic;
            tempsec->floor_xoffs = tempsec->ceiling_xoffs = s->ceiling_xoffs;
            tempsec->floor_yoffs = tempsec->ceiling_yoffs = s->ceiling_yoffs;

            mapnum = s->topmap;

            if (s->floorpic != skyflatnum)
              {
                tempsec->ceilingheight = sec->ceilingheight;
                tempsec->floorpic      = s->floorpic;
                tempsec->floor_xoffs   = s->floor_xoffs;
                tempsec->floor_yoffs   = s->floor_yoffs;
              }

            tempsec->lightlevel  = s->lightlevel;

            if (floorlightlevel)
              *floorlightlevel = s->floorlightsec == -1 ? s->lightlevel :
              sectors[s->floorlightsec].lightlevel;

            if (ceilinglightlevel)
              *ceilinglightlevel = s->ceilinglightsec == -1 ? s->lightlevel :
              sectors[s->ceilinglightsec].lightlevel;
          }
        sec = tempsec;
    }
  else if (sec->heightsec != -1 && sec->altheightsec == 1) //SoM: 3/20/2000
  {
    sector_t*    s = &sectors[sec->heightsec];
    int          heightsec = viewmobj->subsector->sector->heightsec;
    int          underwater = heightsec!=-1 && viewz<=sectors[heightsec].floorheight;

    *tempsec = *sec;

    if(underwater)
    {
      mapnum = s->bottommap;
      if(sec->floorlightsec != -1 && floorlightlevel && ceilinglightlevel)
        *floorlightlevel = *ceilinglightlevel = tempsec->lightlevel = sectors[sec->floorlightsec].lightlevel;
      if(s->floorheight < tempsec->ceilingheight)
      {
        tempsec->ceilingheight = s->floorheight;
        tempsec->ceilingpic = s->floorpic;
        tempsec->ceiling_xoffs = s->floor_xoffs;
        tempsec->ceiling_yoffs = s->floor_yoffs;
      }
    }
    else if(!underwater && heightsec != -1 && viewz >= sectors[heightsec].ceilingheight)
    {
      mapnum = s->topmap;
      if(sec->ceilinglightsec != -1 && floorlightlevel && ceilinglightlevel)
        *floorlightlevel = *ceilinglightlevel = tempsec->lightlevel = sectors[sec->ceilinglightsec].lightlevel;
      if(s->ceilingheight > tempsec->floorheight)
      {
        tempsec->floorheight = s->ceilingheight;
        tempsec->floorpic = s->ceilingpic;
        tempsec->floor_xoffs = s->ceiling_xoffs;
        tempsec->floor_yoffs = s->ceiling_yoffs;
      }
    }
    else
    {
      mapnum = s->midmap;
      //SoM: Use middle normal sector's lightlevels.
      if(s->floorheight > tempsec->floorheight)
      {
        tempsec->floorheight = s->floorheight;
        tempsec->floorpic = s->floorpic;
        tempsec->floor_xoffs = s->floor_xoffs;
        tempsec->floor_yoffs = s->floor_yoffs;
      }
      else
      {
        if(floorlightlevel)
          *floorlightlevel = tempsec->lightlevel;
      }
      if(s->ceilingheight < tempsec->ceilingheight)
      {
        tempsec->ceilingheight = s->ceilingheight;
        tempsec->ceilingpic = s->ceilingpic;
        tempsec->ceiling_xoffs = s->ceiling_xoffs;
        tempsec->ceiling_yoffs = s->ceiling_yoffs;
      }
      else
      {
        if(ceilinglightlevel)
          *ceilinglightlevel = tempsec->lightlevel;
      }
    }
  sec = tempsec;
  }

  if(mapnum >= 0 && mapnum < num_extra_colormaps)
    sec->extra_colormap = extra_colormaps[mapnum];
  else
    sec->extra_colormap = NULL;

  return sec;
}



//
// R_AddLine
// Clips the given segment
// and adds any visible pieces to the line list.
//
void R_AddLine (seg_t*  line)
{
    int                 x1;
    int                 x2;
    angle_t             angle1;
    angle_t             angle2;
    angle_t             span;
    angle_t             tspan;
    static sector_t     tempsec; //SoM: ceiling/water hack

    curline = line;

    // OPTIMIZE: quickly reject orthogonal back sides.
    angle1 = R_PointToAngle (line->v1->x, line->v1->y);
    angle2 = R_PointToAngle (line->v2->x, line->v2->y);

    // Clip to view edges.
    // OPTIMIZE: make constant out of 2*clipangle (FIELDOFVIEW).
    span = angle1 - angle2;

    // Back side? I.e. backface culling?
    if (span >= ANG180)
        return;

    // Global angle needed by segcalc.
    rw_angle1 = angle1;
    angle1 -= viewangle;
    angle2 -= viewangle;

    tspan = angle1 + clipangle;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;

        angle1 = clipangle;
    }
    tspan = clipangle - angle2;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return;
        angle2 = -clipangle;
    }

    // The seg is in the view range,
    // but not necessarily visible.
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    x1 = viewangletox[angle1];
    x2 = viewangletox[angle2];

    // Does not cross a pixel?
    if (x1 == x2)  //SoM: 3/17/2000: Killough said to change the == to >= for... "robustness"?
        return;

    backsector = line->backsector;

    // Single sided line?
    if (!backsector)
        goto clipsolid;

    backsector = R_FakeFlat(backsector, &tempsec, NULL, NULL, true);

    doorclosed = 0; //SoM: 3/25/2000
    
    // Closed door.
    if (backsector->ceilingheight <= frontsector->floorheight
        || backsector->floorheight >= frontsector->ceilingheight)
        goto clipsolid;

    //SoM: 3/25/2000: Check for automap fix. Store in doorclosed for r_segs.c
    if ((doorclosed = R_DoorClosed()))
      goto clipsolid;

    // Window.
    if (backsector->ceilingheight != frontsector->ceilingheight
        || backsector->floorheight != frontsector->floorheight)
        goto clippass;

    // Reject empty lines used for triggers
    //  and special events.
    // Identical floor and ceiling on both sides,
    // identical light levels on both sides,
    // and no middle texture.
    if (backsector->ceilingpic == frontsector->ceilingpic
        && backsector->floorpic == frontsector->floorpic
        && backsector->lightlevel == frontsector->lightlevel
        && curline->sidedef->midtexture == 0

        //SoM: 3/22/2000: Check offsets too!
        && backsector->floor_xoffs == frontsector->floor_xoffs
        && backsector->floor_yoffs == frontsector->floor_yoffs
        && backsector->ceiling_xoffs == frontsector->ceiling_xoffs
        && backsector->ceiling_yoffs == frontsector->ceiling_yoffs

        //SoM: 3/17/2000: consider altered lighting
        && backsector->floorlightsec == frontsector->floorlightsec
        && backsector->ceilinglightsec == frontsector->ceilinglightsec
        //SoM: 4/3/2000: Consider colormaps
        && backsector->extra_colormap == frontsector->extra_colormap
        && !frontsector->ffloors && !backsector->ffloors)
    {
        return;
    }


  clippass:
    R_ClipPassWallSegment (x1, x2-1);
    return;

  clipsolid:
    R_ClipSolidWallSegment (x1, x2-1);
}


//
// R_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
//   | 0 | 1 | 2
// --+---+---+---
// 0 | 0 | 1 | 2
// 1 | 4 | 5 | 6
// 2 | 8 | 9 | A
int     checkcoord[12][4] =
{
    {3,0,2,1},
    {3,0,2,0},
    {3,1,2,0},
    {0},       // UNUSED
    {2,0,2,1},
    {0},       // UNUSED
    {3,1,3,0},
    {0},       // UNUSED
    {2,0,3,1},
    {2,1,3,1},
    {2,1,3,0}
};


boolean R_CheckBBox (fixed_t*   bspcoord)
{
    int                 boxpos;

    fixed_t             x1;
    fixed_t             y1;
    fixed_t             x2;
    fixed_t             y2;

    angle_t             angle1;
    angle_t             angle2;
    angle_t             span;
    angle_t             tspan;

    cliprange_t*        start;

    int                 sx1;
    int                 sx2;

    // Find the corners of the box
    // that define the edges from current viewpoint.
    if (viewx <= bspcoord[BOXLEFT])
        boxpos = 0;
    else if (viewx < bspcoord[BOXRIGHT])
        boxpos = 1;
    else
        boxpos = 2;

    if (viewy >= bspcoord[BOXTOP])
        boxpos |= 0;
    else if (viewy > bspcoord[BOXBOTTOM])
        boxpos |= 1<<2;
    else
        boxpos |= 2<<2;

    if (boxpos == 5)
        return true;

    x1 = bspcoord[checkcoord[boxpos][0]];
    y1 = bspcoord[checkcoord[boxpos][1]];
    x2 = bspcoord[checkcoord[boxpos][2]];
    y2 = bspcoord[checkcoord[boxpos][3]];

    // check clip list for an open space
    angle1 = R_PointToAngle (x1, y1) - viewangle;
    angle2 = R_PointToAngle (x2, y2) - viewangle;

    span = angle1 - angle2;

    // Sitting on a line?
    if (span >= ANG180)
        return true;

    tspan = angle1 + clipangle;

    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;

        angle1 = clipangle;
    }
    tspan = clipangle - angle2;
    if (tspan > 2*clipangle)
    {
        tspan -= 2*clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;

        angle2 = -clipangle;
    }


    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    sx1 = viewangletox[angle1];
    sx2 = viewangletox[angle2];

    // Does not cross a pixel.
    if (sx1 == sx2)
        return false;
    sx2--;

    start = solidsegs;
    while (start->last < sx2)
        start++;

    if (sx1 >= start->first
        && sx2 <= start->last)
    {
        // The clippost contains the new span.
        return false;
    }

    return true;
}



//
// R_Subsector
// Determine floor/ceiling planes.
// Add sprites of things in sector.
// Draw one or more line segments.
//
#ifdef OLDWATER
// this one is set par subsector, and used by r_segs code to mark water planes
fixed_t             waterheight;
#endif

//Fab: hack, until water is finished
fixed_t             dev_waterheight = MININT;
void Command_Water_f (void)
{
    if (COM_Argc()<2)
    {
        CONS_Printf("dev_water [height] : set water level (development test)");

        if (players[consoleplayer].mo)
            dev_waterheight = players[consoleplayer].mo->z + (16<<16) + 1;

        return;
    }

    dev_waterheight = (atoi(COM_Argv(1)) << 16) + 1;
}


void R_Subsector (int num)
{
    int                 count;
    seg_t*              line;
    subsector_t*        sub;
    sector_t            tempsec; //SoM: 3/17/2000: Deep water hack
    int                 floorlightlevel;
    int                 ceilinglightlevel;
    lighttable_t*       floorcolormap;
    lighttable_t*       ceilingcolormap;
    int                 light;

#ifdef RANGECHECK
    if (num>=numsubsectors)
        I_Error ("R_Subsector: ss %i with numss = %i",
                 num,
                 numsubsectors);
#endif

    //faB: subsectors added at run-time
    if (num>=numsubsectors)
        return;

    sscount++;
    sub = &subsectors[num];
    frontsector = sub->sector;
    count = sub->numlines;
    line = &segs[sub->firstline];


    //SoM: 3/17/2000: Deep water/fake ceiling effect.
    frontsector = R_FakeFlat(frontsector, &tempsec, &floorlightlevel,
                             &ceilinglightlevel, false);

    floorcolormap = ceilingcolormap = frontsector->extra_colormap;

    frontsector->numlights = sub->sector->numlights = 0;

#ifdef R_FAKEFLOORS
    // SoM: Check and prep all 3D floors. Set the sector floor/ceiling light
    // levels and colormaps.
    if(frontsector->ffloors)
    {
      R_Prep3DFloors(frontsector);
      sub->sector->lightlist = frontsector->lightlist;
      sub->sector->numlights = frontsector->numlights;
      light = R_GetPlaneLight(frontsector, frontsector->floorheight);
      if(frontsector->floorlightsec == -1)
        floorlightlevel = frontsector->lightlist[light].lightlevel;
      floorcolormap = frontsector->lightlist[light].extra_colormap;
      light = R_GetPlaneLight(frontsector, frontsector->ceilingheight);
      if(frontsector->ceilinglightsec == -1)
        ceilinglightlevel = frontsector->lightlist[light].lightlevel;
      ceilingcolormap = frontsector->lightlist[light].extra_colormap;
    }
#endif

    sub->sector->extra_colormap = frontsector->extra_colormap;

    if (frontsector->floorheight < viewz || (frontsector->heightsec != -1 &&
        sectors[frontsector->heightsec].ceilingpic == skyflatnum))
    {
        floorplane = R_FindPlane (frontsector->floorheight,
                                  frontsector->floorpic,
                                  floorlightlevel,
                                  frontsector->floor_xoffs,
                                  frontsector->floor_yoffs,
                                  floorcolormap,
                                  NULL);
    }
    else
        floorplane = NULL;

    if (frontsector->ceilingheight > viewz
        || frontsector->ceilingpic == skyflatnum ||
        (frontsector->heightsec != -1 &&
         sectors[frontsector->heightsec].floorpic == skyflatnum))
    {
        ceilingplane = R_FindPlane (frontsector->ceilingheight,
                                    frontsector->ceilingpic,
                                    ceilinglightlevel,
                                    frontsector->ceiling_xoffs,
                                    frontsector->ceiling_yoffs,
                                    ceilingcolormap,
                                    NULL);
    }
    else
        ceilingplane = NULL;

#ifdef OLDWATER
    // -------------------- WATER IN DEV. TEST ------------------------
    //dck hack : use abs(tag) for waterheight
    if (frontsector->tag<0)
        waterheight = ((-frontsector->tag) <<16) + (1<<15);
    else
        waterheight = dev_waterheight;

    //
    if (waterheight > frontsector->floorheight &&
        waterheight < frontsector->ceilingheight )
    {
        waterplane = R_FindPlane (waterheight,
                                  1998,
                                  frontsector->lightlevel,
                                  0, 0,
                                  frontsector->extra_colormap,
                                  NULL);
    }
    else
        waterplane = NULL;
#endif
    // -------------------- WATER IN DEV. TEST ------------------------

#ifdef R_FAKEFLOORS
    numffloors = 0;
    ffloor[numffloors].plane = NULL;
    if(frontsector->ffloors)
    {
      ffloor_t*  rover;
      for(rover = frontsector->sorted.s_next; rover != &frontsector->sorted && numffloors < MAXFFLOORS; rover = rover->s_next) {

      if(!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERPLANES))
        continue;

      ffloor[numffloors].plane = NULL;
      if(viewz < *rover->bottomheight && *rover->bottomheight <= frontsector->ceilingheight)
      {
        light = R_GetPlaneLight(frontsector, *rover->bottomheight);
        ffloor[numffloors].plane = R_FindPlane(*rover->bottomheight,
                                  *rover->bottompic,
                                  frontsector->lightlist[light].lightlevel,
                                  *rover->bottomxoffs,
                                  *rover->bottomyoffs,
                                  frontsector->lightlist[light].extra_colormap,
                                  rover);

        ffloor[numffloors].height = *rover->bottomheight;
        ffloor[numffloors].ffloor = rover;
        numffloors++;
      }
      if(viewz > *rover->topheight && *rover->topheight >= frontsector->floorheight )
      {
        light = R_GetPlaneLight(frontsector, *rover->topheight);
        ffloor[numffloors].plane = R_FindPlane(*rover->topheight,
                                  *rover->toppic,
                                  frontsector->lightlist[light].lightlevel,
                                  *rover->topxoffs,
                                  *rover->topyoffs,
                                  frontsector->lightlist[light].extra_colormap,
                                  rover);
        ffloor[numffloors].height = *rover->topheight;
        ffloor[numffloors].ffloor = rover;
        numffloors++;
      }

      }
    }
#endif

#ifdef FLOORSPLATS
    if (sub->splats)
        R_AddVisibleFloorSplats (sub);
#endif

    R_AddSprites (sub->sector, tempsec.lightlevel);

    while (count--)
    {
        R_AddLine (line);
        line++;
    }
}



//
// R_Sort3DFloors
//
// This function sorts the 3D floors of a given sector... Unfortunetly,
// this function will most likely be called twice per sector (look at
// r_segs.c)
void  R_Sort3DFloors(sector_t* sec)
{
  fixed_t     delta, bestdelta;
  ffloor_t*   rover;
  ffloor_t*   best;
  ffloor_t    unsorted;
  int         count, i;

  if(sec->validsort == validcount)
    return;

  sec->validsort = validcount;

  sec->sorted.s_next = sec->sorted.s_prev = &sec->sorted;

  count = 0;
  for(rover = sec->ffloors; rover; rover = rover->next)
  {
    rover->delta = *rover->bottomheight + ((*rover->topheight - *rover->bottomheight) / 2);
    if(rover->flags & FF_EXISTS)
      count++;
    else
      continue;

    rover->s_next = rover->next;
    rover->s_prev = rover->prev;
    if(!rover->s_next)
      (rover->s_next = &unsorted)->s_prev = rover;
    if(!rover->s_prev)
      (rover->s_prev = &unsorted)->s_next = rover;
  }

  for(i = 0; i < count; i++)
  {
    best = NULL;
    bestdelta = -1;
    for(rover = unsorted.s_next; rover != &unsorted; rover = rover->s_next)
    {
      if(!(rover->flags & FF_EXISTS))
        continue;

      delta = abs(rover->delta - viewz);
      if(delta > bestdelta)
      {
        best = rover;
        bestdelta = delta;
      }
    }
    if(!best)
      return;

    best->s_prev->s_next = best->s_next;
    best->s_next->s_prev = best->s_prev;
    best->s_next = &sec->sorted;
    best->s_prev = sec->sorted.s_prev;
    sec->sorted.s_prev->s_next = best;
    sec->sorted.s_prev = best;
  }
}


//
// R_Prep3DFloors
//
// This function creates the lightlists that the given sector uses to light
// floors/ceilings/walls according to the 3D floors.
void R_Prep3DFloors(sector_t*  sector)
{
  ffloor_t*      rover;
  ffloor_t*      best;
  fixed_t        bestheight, maxheight;
  int            count, i, mapnum;
  sector_t*      sec;

  sector->numlights = 0;
  if(sector->lightlist)
    Z_Free(sector->lightlist);

  R_Sort3DFloors(sector);

  count = 0;
  for(rover = sector->ffloors; rover; rover = rover->next)
  {
    if((rover->flags & FF_EXISTS) && (rover->flags & FF_CUTLEVEL))
      count++;
  }

  count++;
  sector->lightlist = Z_Malloc(sizeof(lightlist_t) * count, PU_LEVEL, 0);
  memset(sector->lightlist, 0, sizeof(lightlist_t) * count);
  sector->numlights = count;

  maxheight = MAXINT;
  for(i = 0; i < count; i++)
  {
    bestheight = MAXINT * -1;
    best = NULL;
    for(rover = sector->ffloors; rover; rover = rover->next)
    {
      if(!(rover->flags & FF_EXISTS) || !(rover->flags & FF_CUTLEVEL))
        continue;

      if(*rover->topheight > bestheight && *rover->topheight < maxheight)
      {
        best = rover;
        bestheight = *rover->topheight;
      }
    }
    if(sector->ceilingheight + 1 > bestheight && sector->ceilingheight + 1 < maxheight)
    {
      sector->lightlist[i].height = maxheight = sector->ceilingheight + 1;
      sector->lightlist[i].lightlevel = sector->lightlevel;
      sector->lightlist[i].caster = NULL;
      sector->lightlist[i].extra_colormap = sector->extra_colormap;
      continue;
    }
    if(!best)
      return;

    sector->lightlist[i].height = maxheight = *best->topheight;
    sector->lightlist[i].caster = best;
    sec = &sectors[best->secnum];
    mapnum = sec->midmap;
    if(mapnum >= 0 && mapnum < num_extra_colormaps)
      sec->extra_colormap = extra_colormaps[mapnum];
    else
      sec->extra_colormap = NULL;

    sector->lightlist[i].lightlevel = *best->bottomlightlevel;
    sector->lightlist[i].extra_colormap = sec->extra_colormap;
  }
}



int   R_GetPlaneLight(sector_t* sector, fixed_t  planeheight)
{
  int      i;
  int      lastone = 0;

  for(i = 1; i < sector->numlights; i++)
  {
    if(sector->lightlist[i].height > planeheight)
      lastone = i;
    else if(sector->lightlist[i].caster && sector->lightlist[i].caster->flags & FF_NOSHADE)
      continue;
    else
      return lastone;
  }
  return i - 1;
}



//
// RenderBSPNode
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.
#if 1
void R_RenderBSPNode (int bspnum)
{
    node_t*     bsp;
    int         side;

    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1)
            // BP: never happen : bspnum = int, children = unsigned short
            // except first call if numsubsectors=0 ! who care ?
            R_Subsector (0);
        else
            R_Subsector (bspnum&(~NF_SUBSECTOR));
        return;
    }

    bsp = &nodes[bspnum];

    // Decide which side the view point is on.
    side = R_PointOnSide (viewx, viewy, bsp);

    // Recursively divide front space.
    R_RenderBSPNode (bsp->children[side]);

    // Possibly divide back space.
    if (R_CheckBBox (bsp->bbox[side^1]))
        R_RenderBSPNode (bsp->children[side^1]);
}
#else



//
// RenderBSPNode : DATA RECURSION version, slower :<
//
// Denis.F. 03-April-1998 : I let this here for learning purpose
//                          but this is slower coz PGCC optimises
//                          fairly well the recursive version.
//                          (it was clocked with p5prof)
//
#define MAX_BSPNUM_PUSHED 512

void R_RenderBSPNode (int bspnum)
{
    node_t*     bsp;
    int         side;

    node_t      *bspstack[MAX_BSPNUM_PUSHED];
    node_t      **bspnum_p;

    //int         visited=0;

    bspstack[0] = NULL;
    bspstack[1] = NULL;
    bspnum_p = &bspstack[2];

    // Recursively divide front space.
    for (;;)
    {
        // Recursively divide front space.
        while (!(bspnum & NF_SUBSECTOR))
        {
            bsp = &nodes[bspnum];

            // Decide which side the view point is on.
            side = R_PointOnSide (viewx, viewy, bsp);

            *bspnum_p++ = bsp;
            *bspnum_p++ = (void*) side;
            bspnum = bsp->children[side];
        }

        // Found a subsector
        if (bspnum == -1)
            R_Subsector (0);
        else
            R_Subsector (bspnum&(~NF_SUBSECTOR));

        side = (int) *--bspnum_p;
        if ((bsp = *--bspnum_p) == NULL )
        {
            // we're done
            //CONS_Printf ("Subsectors visited: %d\n", visited);
            return;
        }

        // Possibly divide back space.
        if (R_CheckBBox (bsp->bbox[side^1]))
            // dirty optimisation here!! :) double-pop done because no push!
            bspnum = bsp->children[side^1];
    }

}
#endif
