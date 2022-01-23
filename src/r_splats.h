// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
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
/// \brief Flat sprites & splats effects

#ifndef __R_SPLATS_H__
#define __R_SPLATS_H__

#include "r_defs.h"

#define WALLSPLATS      // comment this out to compile without splat effects
#ifdef USEASM
#define FLOORSPLATS
#endif

#define MAXLEVELSPLATS      1024

// splat flags
#define SPLATDRAWMODE_MASK 0x03 // mask to get drawmode from flags
#define SPLATDRAWMODE_OPAQUE 0x00
#define SPLATDRAWMODE_SHADE 0x01
#define SPLATDRAWMODE_TRANS 0x02

// ==========================================================================
// DEFINITIONS
// ==========================================================================

// WALL SPLATS are patches drawn on top of wall segs
typedef struct wallsplat_s
{
	int patch; // lump id.
	vertex_t v1, v2; // vertices along the linedef
	fixed_t top;
	fixed_t offset; // offset in columns<<FRACBITS from start of linedef to start of splat
	int flags;
	fixed_t* yoffset;
	line_t* line; // the parent line of the splat seg
	struct wallsplat_s* next;
} wallsplat_t;

// FLOOR SPLATS are pic_t (raw horizontally stored) drawn on top of the floor or ceiling
typedef struct floorsplat_s
{
	int pic; // a pic_t lump id
	int flags;
	vertex_t verts[4]; // (x,y) as viewn from above on map
	fixed_t z; // z (height) is constant for all the floorsplats
	subsector_t* subsector; // the parent subsector
	struct floorsplat_s* next;
	struct floorsplat_s* nextvis;
} floorsplat_t;

// p_setup.c
float P_SegLength(seg_t* seg);

// call at P_SetupLevel()
void R_ClearLevelSplats(void);

#ifdef WALLSPLATS
void R_AddWallSplat(line_t* wallline, int sectorside, char* patchname, fixed_t top,
	fixed_t wallfrac, int flags);
#endif
#ifdef FLOORSPLATS
void R_AddFloorSplat(subsector_t* subsec, const char* picname, fixed_t x, fixed_t y, fixed_t z,
	int flags);
#endif

void R_ClearVisibleFloorSplats(void);
void R_AddVisibleFloorSplats(subsector_t* subsec);
void R_DrawVisibleFloorSplats(void);

#endif /*__R_SPLATS_H__*/
