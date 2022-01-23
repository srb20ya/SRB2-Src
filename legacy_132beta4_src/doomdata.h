// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: doomdata.h,v 1.4 2000/11/02 17:50:06 stroggonmeth Exp $
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
// $Log: doomdata.h,v $
// Revision 1.4  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.3  2000/04/04 00:32:45  stroggonmeth
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
//      all external data is defined here
//      most of the data is loaded into different structures at run time
//      some internal structures shared by many modules are here
//
//-----------------------------------------------------------------------------

#ifndef __DOOMDATA__
#define __DOOMDATA__

// The most basic types we use, portability.
#include "doomtype.h"

// Some global defines, that configure the game.
#include "doomdef.h"



//
// Map level types.
// The following data structures define the persistent format
// used in the lumps of the WAD files.
//

// Lump order in a map WAD: each map needs a couple of lumps
// to provide a complete scene geometry description.
enum
{
  ML_LABEL,             // A separator, name, ExMx or MAPxx
  ML_THINGS,            // Monsters, items..
  ML_LINEDEFS,          // LineDefs, from editing
  ML_SIDEDEFS,          // SideDefs, from editing
  ML_VERTEXES,          // Vertices, edited and BSP splits generated
  ML_SEGS,              // LineSegs, from LineDefs split by BSP
  ML_SSECTORS,          // SubSectors, list of LineSegs
  ML_NODES,             // BSP nodes
  ML_SECTORS,           // Sectors, from editing
  ML_REJECT,            // LUT, sector-sector visibility        
  ML_BLOCKMAP           // LUT, motion clipping, walls/grid element
};


// A single Vertex.
typedef struct
{
  short         x;
  short         y;
} mapvertex_t;


// A SideDef, defining the visual appearance of a wall,
// by setting textures and offsets.
typedef struct
{
  short         textureoffset;
  short         rowoffset;
  char          toptexture[8];
  char          bottomtexture[8];
  char          midtexture[8];
  // Front sector, towards viewer.
  short         sector;
} mapsidedef_t;



// A LineDef, as used for editing, and as input
// to the BSP builder.
typedef struct
{
  short         v1;
  short         v2;
  short         flags;
  short         special;
  short         tag;
  // sidenum[1] will be -1 if one sided
  short         sidenum[2];             
} maplinedef_t;


//
// LineDef attributes.
//

// Solid, is an obstacle.
#define ML_BLOCKING             1

// Blocks monsters only.
#define ML_BLOCKMONSTERS        2

// Backside will not be present at all
//  if not two sided.
#define ML_TWOSIDED             4

// If a texture is pegged, the texture will have
// the end exposed to air held constant at the
// top or bottom of the texture (stairs or pulled
// down things) and will move with a height change
// of one of the neighbor sectors.
// Unpegged textures allways have the first row of
// the texture at the top pixel of the line for both
// top and bottom textures (use next to windows).

// upper texture unpegged
#define ML_DONTPEGTOP           8

// lower texture unpegged
#define ML_DONTPEGBOTTOM        16      

// Don't activate this special on Easy mode. // Tails 01-14-2003
#define ML_NOEASY               32 // ML_SECRET

// Don't let Knuckles climb on this line
#define ML_NOCLIMB           64

// Don't activate this special on Normal mode. // Tails 01-14-2003
#define ML_NONORMAL             128 // ML_DONTDRAW

// Don't activate this special on Hard mode. // Tails 01-14-2003
#define ML_NOHARD               256 // ML_MAPPED

//SoM: 3/29/2000: If flag is set, the player can use through it.
#define ML_PASSUSE              512

//SoM: 4/1/2000: If flag is set, anything can trigger the line.
#define ML_ALLTRIGGER           1024

// New ones to disable lines for characters Tails 06-22-2003
#define ML_NOSONIC              2048
#define ML_NOTAILS              4096
#define ML_NOKNUX               8192



// Sector definition, from editing.
typedef struct
{
  short         floorheight;
  short         ceilingheight;
  char          floorpic[8];
  char          ceilingpic[8];
  short         lightlevel;
  short         special;
  short         tag;
} mapsector_t;

// SubSector, as generated by BSP.
typedef struct
{
  short         numsegs;
  // Index of first one, segs are stored sequentially.
  short         firstseg;       
} mapsubsector_t;


// LineSeg, generated by splitting LineDefs
// using partition lines selected by BSP builder.
typedef struct
{
  short         v1;
  short         v2;
  short         angle;          
  short         linedef;
  short         side;
  short         offset;
} mapseg_t;



// BSP node structure.

// Indicate a leaf.
#define NF_SUBSECTOR    0x8000

typedef struct
{
  // Partition line from (x,y) to x+dx,y+dy)
  short         x;
  short         y;
  short         dx;
  short         dy;

  // Bounding box for each child,
  // clip against view frustum.
  short         bbox[2][4];

  // If NF_SUBSECTOR its a subsector,
  // else it's a node of another subtree.
  unsigned short        children[2];

} mapnode_t;




// Thing definition, position, orientation and type,
// plus skill/visibility flags and attributes.
typedef struct
{
    short               x;
    short               y;
    short               angle;
    short               type;
    unsigned short      options;
	short               z; // Z support for some objects Tails 05-24-2002
	short               oldnum; // 4001-4028 player starts
    struct mobj_s*      mobj;
} mapthing_t;


extern char *Color_Names[MAXSKINCOLORS];

#define NUMMAPS 1035 // Tails 11-30-2003


#endif                  // __DOOMDATA__
