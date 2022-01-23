// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_plane.h,v 1.5 2000/11/02 17:50:09 stroggonmeth Exp $
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
// $Log: r_plane.h,v $
// Revision 1.5  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.4  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.3  2000/04/04 00:32:48  stroggonmeth
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
//      Refresh, visplane stuff (floor, ceilings).
//
//-----------------------------------------------------------------------------


#ifndef __R_PLANE__
#define __R_PLANE__

#include "screen.h"     //needs MAXVIDWIDTH/MAXVIDHEIGHT
#include "r_data.h"

//
// Now what is a visplane, anyway?
// Simple : kinda floor/ceiling polygon optimised for Doom rendering.
// 4124 bytes!
//
typedef struct visplane_s
{
  struct visplane_s*    next;//SoM: 3/17/2000

  fixed_t               height;
  int                   picnum;
  int                   lightlevel;
  int                   minx;
  int                   maxx;

  //SoM: 4/3/2000: Colormaps per sector!
  lighttable_t*         extra_colormap;

  // leave pads for [minx-1]/[maxx+1]

  //faB: words sucks .. should get rid of that.. but eats memory
  //added:08-02-98: THIS IS UNSIGNED! VERY IMPORTANT!!
  unsigned short         pad1;
  unsigned short         top[MAXVIDWIDTH];
  unsigned short         pad2;
  unsigned short         pad3;
  unsigned short         bottom[MAXVIDWIDTH];
  unsigned short         pad4;
  fixed_t                flatscale[MAXVIDWIDTH];

  fixed_t xoffs, yoffs;  // SoM: 3/6/2000: Srolling flats.
  struct ffloor_s* ffloor;
} visplane_t;


extern visplane_t*    floorplane;
extern visplane_t*    ceilingplane;

#ifdef OLDWATER
extern visplane_t*    waterplane;
#endif

#ifdef R_FAKEFLOORS
extern visplane_t*    ffloorplane;

//extern short          ffloor_f_clip[MAXVIDWIDTH];
//extern short          ffloor_c_clip[MAXVIDWIDTH];
#endif

// Visplane related.
extern  short*          lastopening;

typedef void (*planefunction_t) (int top, int bottom);

extern planefunction_t  floorfunc;
extern planefunction_t  ceilingfunc_t;

extern short            floorclip[MAXVIDWIDTH];
extern short            ceilingclip[MAXVIDWIDTH];
extern short            waterclip[MAXVIDWIDTH];   //added:18-02-98:WATER!
extern fixed_t          flatscale[MAXVIDWIDTH];
extern fixed_t          yslopetab[MAXVIDHEIGHT*4];

extern fixed_t*         yslope;
extern fixed_t          distscale[MAXVIDWIDTH];

void R_InitPlanes (void);
void R_ClearPlanes (player_t *player);

void R_MapPlane
( int           y,
  int           x1,
  int           x2 );

void R_MakeSpans
( int           x,
  int           t1,
  int           b1,
  int           t2,
  int           b2 );

void R_DrawPlanes (void);

visplane_t* R_FindPlane( fixed_t height,
                         int     picnum,
                         int     lightlevel,
                         fixed_t xoff,
                         fixed_t yoff,
                         lighttable_t* planecolormap,
                         ffloor_t* ffloor); //SoM: 3/17/2000

visplane_t* R_CheckPlane
( visplane_t*   pl,
  int           start,
  int           stop );


// SoM: Draws a single visplane. If !handlesource, it won't allocate or
// remove ds_source.
void R_DrawSinglePlane(visplane_t* pl, boolean handlesource);

void R_PlaneBounds(visplane_t* plane, int *hi, int *low);


typedef struct planemgr_s
{
  visplane_t*  plane;
  fixed_t      height;
  boolean      mark;
  fixed_t      f_pos;  // `F' for `Front sector'.
  fixed_t      b_pos;  // `B' for `Back sector'
  fixed_t      f_frac;
  fixed_t      f_step;
  fixed_t      b_frac;
  fixed_t      b_step;
  short        f_clip[MAXVIDWIDTH];
  short        c_clip[MAXVIDWIDTH];

  struct ffloor_s  *ffloor;
} planemgr_t;

extern planemgr_t    ffloor[MAXFFLOORS];
extern int           numffloors;
#endif
