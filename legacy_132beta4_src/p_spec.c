// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_spec.c,v 1.31 2001/08/19 20:41:04 hurdler Exp $
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
// $Log: p_spec.c,v $
// Revision 1.31  2001/08/19 20:41:04  hurdler
// small changes
//
// Revision 1.30  2001/08/13 22:53:40  stroggonmeth
// Small commit
//
// Revision 1.29  2001/08/11 15:18:02  hurdler
// Add sector colormap in hw mode (first attempt)
//
// Revision 1.28  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.27  2001/06/16 08:07:55  bpereira
// no message
//
// Revision 1.26  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.25  2001/05/03 18:23:30  crashrl
// corrected wrong comment concerning teamstartsec
//
// Revision 1.24  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.23  2001/03/21 18:24:38  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.22  2001/03/13 22:14:19  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.21  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.20  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.19  2000/11/21 21:13:17  stroggonmeth
// Optimised 3D floors and fixed crashing bug in high resolutions.
//
// Revision 1.18  2000/11/11 13:59:45  bpereira
// no message
//
// Revision 1.17  2000/11/04 16:23:43  bpereira
// no message
//
// Revision 1.16  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.15  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.14  2000/10/07 20:36:13  crashrl
// Added deathmatch team-start-sectors via sector/line-tag and linedef-type 1000-1031
//
// Revision 1.13  2000/05/23 15:22:34  stroggonmeth
// Not much. A graphic bug fixed.
//
// Revision 1.12  2000/05/03 23:51:00  stroggonmeth
// A few, quick, changes.
//
// Revision 1.11  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.10  2000/04/15 22:12:57  stroggonmeth
// Minor bug fixes
//
// Revision 1.9  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.8  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.7  2000/04/08 17:29:25  stroggonmeth
// no message
//
// Revision 1.6  2000/04/08 11:27:29  hurdler
// fix some boom stuffs
//
// Revision 1.5  2000/04/06 20:54:28  hurdler
// Mostly remove warnings under windows
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
//      Implements special effects:
//      Texture animation, height or lighting changes
//       according to adjacent sectors, respective
//       utility functions, etc.
//      Line Tag handling. Line and Sector triggers.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "p_setup.h"    //levelflats for flat animation
#include "r_data.h"
#include "m_random.h"

#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include "dstrings.h" //SoM: 3/10/2000
#include "r_main.h"   //Two extra includes.
#include "t_script.h"
//#include "r_sky.h" // Portals

#include "hardware/hw3sound.h"

//SoM: Enable Boom features?
const int boomsupport = 1;
const int variable_friction = 1;
const int allow_pushers = 1;



//SoM: 3/7/2000
static void P_SpawnScrollers(void);

static void P_SpawnFriction(void);
static void P_SpawnPushers(void);
static void Add_Pusher(int type, int x_mag, int y_mag, mobj_t* source, int affectee); //SoM: 3/9/2000
void P_FindAnimatedFlat (int i);



//
// Animating textures and planes
// There is another anim_t used in wi_stuff, unrelated.
//
typedef struct
{
    boolean     istexture;
    int         picnum;
    int         basepic;
    int         numpics;
    int         speed;
} anim_t;


//
//      source animation definition
//
#pragma pack(1) //Hurdler: 04/04/2000: I think pragma is more portable
typedef struct
{
    char        istexture;      // if false, it is a flat
    char        endname[9];
    char        startname[9];
    int         speed;
} animdef_t; 
#pragma pack()



#define MAXANIMS     32


//SoM: 3/7/2000: New sturcture without limits.
static anim_t*   lastanim;
static anim_t*   anims;
static size_t    maxanims;

//
// P_InitPicAnims
//

// Floor/ceiling animation sequences,
//  defined by first and last frame,
//  i.e. the flat (64x64 tile) name to
//  be used.
// The full animation sequence is given
//  using all the flats between the start
//  and end entry, in the order found in
//  the WAD file.
//
animdef_t               harddefs[] =
{
    // DOOM II flat animations.
    {false,     "NUKAGE3",      "NUKAGE1",      4},
    {false,     "FWATER16",     "FWATER1",      4},
    {false,     "LWATER16",     "LWATER1",      4},
    {false,     "WATER7",     "WATER0",         4},
    {false,     "SWATER4",      "SWATER1",      8},
    {false,     "LAVA4",        "LAVA1",        8},
    {false,     "BLOOD3",       "BLOOD1",       8},

    {false,     "RROCK08",      "RROCK05",      8},
    {false,     "CHEMG16",      "CHEMG01",      4}, // Tails 12-11-2000 THZ Chemical gunk
    {false,     "SLIME08",      "SLIME05",      4}, // Tails
    {false,     "THZBOXF4",     "THZBOXF1",     2}, // Moved up with the flats Graue 12-18-2003

	{false,     "BLUE3",        "BLUE1",        4},
	{false,     "GREY3",        "GREY1",        4},

    // animated textures
	{true,      "GFALL4",       "GFALL1",       2}, // Short waterfall Tails 11-07-2000
	{true,      "CFALL4",       "CFALL1",       2}, // Long waterfall Tails 11-07-2000
	{true,      "TFALL4",       "TFALL1",       2}, // THZ Chemical fall Tails 11-07-2000
	{true,      "THZBOX04",     "THZBOX01",     2},
	{true,      "SFALL4",       "SFALL1",       4}, // Lava fall Graue 12-18-2003
	{true,      "BFALL4",       "BFALL1",       2}, // HPZ waterfall Graue 12-18-2003
	{true,      "GREYW3",       "GREYW1",       4},
	{true,      "BLUEW3",       "BLUEW1",       4},
	{true,      "COMP6",        "COMP4",        4},
	{true,      "RED3",         "RED1",         4},
	{true,      "YEL3",         "YEL1",         4},

	// Begin dummy slots Tails 12-27-2003
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},
	{false,     "DUMMYSLOT",    "DUMMYSLOT",    8},

    {-1}
};


//
//      Animating line specials
//

//
// Init animated textures
// - now called at level loading P_SetupLevel()
//

static animdef_t   *animdefs;

//SoM: 3/7/2000: Use new boom method of reading lump from wad file.
void P_InitPicAnims (void)
{
  //  Init animation
  int         i;

  if(W_CheckNumForName("ANIMATED") != -1)
    animdefs = (animdef_t *)W_CacheLumpName("ANIMATED",PU_STATIC);
  else
    animdefs = harddefs;

  for (i = 0; animdefs[i].istexture != -1; i++, maxanims++);
  anims = (anim_t *)malloc(sizeof(anim_t) * (maxanims + 1));

  lastanim = anims;
  for (i = 0; animdefs[i].istexture != -1; i++)
  {

    if (animdefs[i].istexture)
    {
      // different episode ?
      if (R_CheckTextureNumForName(animdefs[i].startname) == -1)
          continue;

      lastanim->picnum = R_TextureNumForName (animdefs[i].endname);
      lastanim->basepic = R_TextureNumForName (animdefs[i].startname);
    }
    else
    {
      if ((W_CheckNumForName(animdefs[i].startname)) == -1)
          continue;

      lastanim->picnum = R_FlatNumForName (animdefs[i].endname);
      lastanim->basepic = R_FlatNumForName (animdefs[i].startname);
    }


    lastanim->istexture = (boolean)animdefs[i].istexture;
    lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;

    if (lastanim->numpics < 2)
        I_Error ("P_InitPicAnims: bad cycle from %s to %s",
                  animdefs[i].startname,
                  animdefs[i].endname);

    lastanim->speed = LONG(animdefs[i].speed) * NEWTICRATERATIO;
    lastanim++;
  }
  lastanim->istexture = -1;

  if(animdefs != harddefs)
    Z_ChangeTag (animdefs,PU_CACHE);
}

//  Check for flats in levelflats, that are part
//  of a flat anim sequence, if so, set them up for animation
//
//SoM: 3/16/2000: Changed parameter from pointer to "anims" entry number
void P_FindAnimatedFlat (int animnum)
{
    int            i;
    int            startflatnum,endflatnum;
    levelflat_t*   foundflats = levelflats;

    startflatnum = anims[animnum].basepic;
    endflatnum   = anims[animnum].picnum;

    // note: high word of lumpnum is the wad number
    if ( (startflatnum>>16) != (endflatnum>>16) )
       I_Error ("AnimatedFlat start %s not in same wad as end %s\n",
                animdefs[animnum].startname, animdefs[animnum].endname);

    //
    // now search through the levelflats if this anim flat sequence is used
    //
    for (i = 0; i<numlevelflats; i++, foundflats++)
    {
        // is that levelflat from the flat anim sequence ?
        if (foundflats->lumpnum >= startflatnum &&
            foundflats->lumpnum <= endflatnum)
        {
            foundflats->baselumpnum = startflatnum;
            foundflats->animseq = foundflats->lumpnum - startflatnum;
            foundflats->numpics = endflatnum - startflatnum + 1;
            foundflats->speed = anims[animnum].speed;

            if (devparm)
                CONS_Printf("animflat: %#03d name:%.8s animseq:%d numpics:%d speed:%d\n",
                            i, foundflats->name, foundflats->animseq,
                            foundflats->numpics,foundflats->speed);
        }
    }

}


//
//  Called by P_LoadSectors
//
void P_SetupLevelFlatAnims (void)
{
    int    i;

    // the original game flat anim sequences
    for (i=0 ; anims[i].istexture != -1; i++)
    {
        if (!anims[i].istexture)
        {
            P_FindAnimatedFlat (i);
        }
    }
}


//
// UTILITIES
//


//
// getSide()
// Will return a side_t*
//  given the number of the current sector,
//  the line number, and the side (0/1) that you want.
//
side_t*
getSide
( int           currentSector,
  int           line,
  int           side )
{
    return &sides[ (sectors[currentSector].lines[line])->sidenum[side] ];
}


//
// getSector()
// Will return a sector_t*
//  given the number of the current sector,
//  the line number and the side (0/1) that you want.
//
sector_t*
getSector
( int           currentSector,
  int           line,
  int           side )
{
    return sides[ (sectors[currentSector].lines[line])->sidenum[side] ].sector;
}


//
// twoSided()
// Given the sector number and the line number,
//  it will tell you whether the line is two-sided or not.
//
//SoM: 3/7/2000: Use the boom method
int
twoSided
( int   sector,
  int   line )
{
  return boomsupport?
    ((sectors[sector].lines[line])->sidenum[1] != -1)
    :
    ((sectors[sector].lines[line])->flags & ML_TWOSIDED);
}




//
// getNextSector()
// Return sector_t * of sector next to current.
// NULL if not two-sided line
//
//SoM: 3/7/2000: Use boom method.
sector_t*
getNextSector
( line_t*       line,
  sector_t*     sec )
{
  if (!boomsupport)
  {
    if (!(line->flags & ML_TWOSIDED))
      return NULL;
  }

  if (line->frontsector == sec)
  {
    if (!boomsupport || line->backsector!=sec)
      return line->backsector;
    else
      return NULL;
  }
  return line->frontsector;
}




//
// P_FindLowestFloorSurrounding()
// FIND LOWEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t P_FindLowestFloorSurrounding(sector_t* sec)
{
    int                 i;
    line_t*             check;
    sector_t*           other;
    fixed_t             floor = sec->floorheight;

    for (i=0 ;i < sec->linecount ; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check,sec);

        if (!other)
            continue;

        if (other->floorheight < floor)
            floor = other->floorheight;
    }
    return floor;
}




//
// P_FindHighestFloorSurrounding()
// FIND HIGHEST FLOOR HEIGHT IN SURROUNDING SECTORS
//
fixed_t P_FindHighestFloorSurrounding(sector_t *sec)
{
    int                 i;
    line_t*             check;
    sector_t*           other;
    fixed_t             floor = -500*FRACUNIT;
    int                 foundsector = 0;


    for (i=0 ;i < sec->linecount ; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check,sec);

        if (!other)
            continue;

        if (other->floorheight > floor || !foundsector)
            floor = other->floorheight;

        if(!foundsector)
          foundsector = 1;
    }
    return floor;
}



//
// P_FindNextHighestFloor
// FIND NEXT HIGHEST FLOOR IN SURROUNDING SECTORS
// SoM: 3/7/2000: Use Lee Killough's version insted.
// Rewritten by Lee Killough to avoid fixed array and to be faster
//
fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->floorheight > currentheight)
    {
      int height = other->floorheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->floorheight < height &&
            other->floorheight > currentheight)
          height = other->floorheight;
      return height;
    }
  return currentheight;
}


////////////////////////////////////////////////////
// SoM: Start new Boom functions
////////////////////////////////////////////////////

// P_FindNextLowestFloor()
//
// Passed a sector and a floor height, returns the fixed point value
// of the largest floor height in a surrounding sector smaller than
// the floor height passed. If no such height exists the floorheight
// passed is returned.
//
//
fixed_t P_FindNextLowestFloor(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->floorheight < currentheight)
    {
      int height = other->floorheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->floorheight > height &&
            other->floorheight < currentheight)
          height = other->floorheight;
      return height;
    }
  return currentheight;
}


//
// P_FindNextLowestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the largest ceiling height in a surrounding sector smaller than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
//
fixed_t P_FindNextLowestCeiling(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
        other->ceilingheight < currentheight)
    {
      int height = other->ceilingheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->ceilingheight > height &&
            other->ceilingheight < currentheight)
          height = other->ceilingheight;
      return height;
    }
  return currentheight;
}




//
// P_FindNextHighestCeiling()
//
// Passed a sector and a ceiling height, returns the fixed point value
// of the smallest ceiling height in a surrounding sector larger than
// the ceiling height passed. If no such height exists the ceiling height
// passed is returned.
//
//
fixed_t P_FindNextHighestCeiling(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->ceilingheight > currentheight)
    {
      int height = other->ceilingheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->ceilingheight < height &&
            other->ceilingheight > currentheight)
          height = other->ceilingheight;
      return height;
    }
  return currentheight;
}

////////////////////////////
// End New Boom functions
////////////////////////////



//
// FIND LOWEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t
P_FindLowestCeilingSurrounding(sector_t* sec)
{
    int                 i;
    line_t*             check;
    sector_t*           other;
    fixed_t             height = MAXINT;
    int                 foundsector = 0;

    if (boomsupport) height = 32000*FRACUNIT; //SoM: 3/7/2000: Remove ovf
                                              
    for (i=0 ;i < sec->linecount ; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check,sec);

        if (!other)
            continue;

        if (other->ceilingheight < height || !foundsector)
            height = other->ceilingheight;

        if(!foundsector)
          foundsector = 1;
    }
    return height;
}


//
// FIND HIGHEST CEILING IN THE SURROUNDING SECTORS
//
fixed_t P_FindHighestCeilingSurrounding(sector_t* sec)
{
    int         i;
    line_t*     check;
    sector_t*   other;
    fixed_t     height = 0;
    int         foundsector = 0;

    for (i=0 ;i < sec->linecount ; i++)
    {
        check = sec->lines[i];
        other = getNextSector(check,sec);

        if (!other)
            continue;

        if (other->ceilingheight > height || !foundsector)
            height = other->ceilingheight;

        if(!foundsector)
          foundsector = 1;
    }
    return height;
}



//SoM: 3/7/2000: UTILS.....
//
// P_FindShortestTextureAround()
//
// Passed a sector number, returns the shortest lower texture on a
// linedef bounding the sector.
//
//
fixed_t P_FindShortestTextureAround(int secnum)
{
  int minsize = MAXINT;
  side_t*     side;
  int i;
  sector_t *sec = &sectors[secnum];

  if (boomsupport)
    minsize = 32000<<FRACBITS;

  for (i = 0; i < sec->linecount; i++)
  {
    if (twoSided(secnum, i))
    {
      side = getSide(secnum,i,0);
      if (side->bottomtexture > 0)
        if (textureheight[side->bottomtexture] < minsize)
          minsize = textureheight[side->bottomtexture];
      side = getSide(secnum,i,1);
      if (side->bottomtexture > 0)
        if (textureheight[side->bottomtexture] < minsize)
          minsize = textureheight[side->bottomtexture];
    }
  }
  return minsize;
}



//SoM: 3/7/2000: Stuff.... (can you tell I'm getting tired? It's 12:30!)
//
// P_FindShortestUpperAround()
//
// Passed a sector number, returns the shortest upper texture on a
// linedef bounding the sector.
//
//
fixed_t P_FindShortestUpperAround(int secnum)
{
  int minsize = MAXINT;
  side_t*     side;
  int i;
  sector_t *sec = &sectors[secnum];

  if (boomsupport)
    minsize = 32000<<FRACBITS;

  for (i = 0; i < sec->linecount; i++)
  {
    if (twoSided(secnum, i))
    {
      side = getSide(secnum,i,0);
      if (side->toptexture > 0)
        if (textureheight[side->toptexture] < minsize)
          minsize = textureheight[side->toptexture];
      side = getSide(secnum,i,1);
      if (side->toptexture > 0)
        if (textureheight[side->toptexture] < minsize)
          minsize = textureheight[side->toptexture];
    }
  }
  return minsize;
}




//SoM: 3/7/2000
//
// P_FindModelFloorSector()
//
// Passed a floor height and a sector number, return a pointer to a
// a sector with that floor height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
//
sector_t *P_FindModelFloorSector(fixed_t floordestheight,int secnum)
{
  int i;
  sector_t *sec=NULL;
  int linecount;

  sec = &sectors[secnum];
  linecount = sec->linecount;
  for (i = 0; i < (!boomsupport && sec->linecount<linecount?
                   sec->linecount : linecount); i++)
  {
    if ( twoSided(secnum, i) )
    {
      if (getSide(secnum,i,0)->sector-sectors == secnum)
          sec = getSector(secnum,i,1);
      else
          sec = getSector(secnum,i,0);

      if (sec->floorheight == floordestheight)
        return sec;
    }
  }
  return NULL;
}



//SoM: 3/7/2000: Last one...
//
// P_FindModelCeilingSector()
//
// Passed a ceiling height and a sector number, return a pointer to a
// a sector with that ceiling height across the lowest numbered two sided
// line surrounding the sector.
//
// Note: If no sector at that height bounds the sector passed, return NULL
//
//
sector_t *P_FindModelCeilingSector(fixed_t ceildestheight,int secnum)
{
  int i;
  sector_t *sec=NULL;
  int linecount;

  sec = &sectors[secnum];
  linecount = sec->linecount;
  for (i = 0; i < (!boomsupport && sec->linecount<linecount?
                   sec->linecount : linecount); i++)
  {
    if ( twoSided(secnum, i) )
    {
      if (getSide(secnum,i,0)->sector-sectors == secnum)
          sec = getSector(secnum,i,1);
      else
          sec = getSector(secnum,i,0);

      if (sec->ceilingheight == ceildestheight)
        return sec;
    }
  }
  return NULL;
}



//
// RETURN NEXT SECTOR # THAT LINE TAG REFERS TO
//
//SoM: 3/7/2000: Killough wrote this to improve the process.
int
P_FindSectorFromLineTag
( line_t*       line,
  int           start )
{
  start = start >= 0 ? sectors[start].nexttag :
    sectors[(unsigned) line->tag % (unsigned) numsectors].firsttag;
  while (start >= 0 && sectors[start].tag != line->tag)
    start = sectors[start].nexttag;
  return start;
}



//
// P_FindSectorFromTag
// Used by FraggleScript
int
P_FindSectorFromTag
( int           tag,
  int           start )
{
  start = start >= 0 ? sectors[start].nexttag :
    sectors[(unsigned) tag % (unsigned) numsectors].firsttag;
  while (start >= 0 && sectors[start].tag != tag)
    start = sectors[start].nexttag;
  return start;
}


//SoM: 3/7/2000: More boom specific stuff...
// killough 4/16/98: Same thing, only for linedefs

int P_FindLineFromLineTag(const line_t *line, int start)
{
  start = start >= 0 ? lines[start].nexttag :
    lines[(unsigned) line->tag % (unsigned) numlines].firsttag;
  while (start >= 0 && lines[start].tag != line->tag)
    start = lines[start].nexttag;
  return start;
}

// New utility function. Tails 10-05-2003
int P_FindLineFromTag(int tag)
{
	int start = lines[0].nexttag;
  while (start >= 0 && lines[start].tag != tag)
    start = lines[start].nexttag;
  return start;
}

// New utility function. Tails 07-20-2003
int P_FindSpecialLineFromTag(int special, int tag)
{
	int i;
	for(i=0; i<numlines; i++)
	{
		if(lines[i].tag == tag)
		{
			if(lines[i].special == special)
				return i;
		}
	}
	return -1;
}


//SoM: 3/7/2000: Oh joy!
// Hash the sector tags across the sectors and linedefs.
static void P_InitTagLists(void)
{
  register int i;

  for (i=numsectors; --i>=0; )
    sectors[i].firsttag = -1;
  for (i=numsectors; --i>=0; )
    {
      int j = (unsigned) sectors[i].tag % (unsigned) numsectors;
      sectors[i].nexttag = sectors[j].firsttag;
      sectors[j].firsttag = i;
    }

  for (i=numlines; --i>=0; )
    lines[i].firsttag = -1;
  for (i=numlines; --i>=0; )
    {
      int j = (unsigned) lines[i].tag % (unsigned) numlines;
      lines[i].nexttag = lines[j].firsttag;
      lines[j].firsttag = i;
    }
}




//
// Find minimum light from an adjacent sector
//
int
P_FindMinSurroundingLight
( sector_t*     sector,
  int           max )
{
    int         i;
    int         min;
    line_t*     line;
    sector_t*   check;

    min = max;
    for (i=0 ; i < sector->linecount ; i++)
    {
        line = sector->lines[i];
        check = getNextSector(line,sector);

        if (!check)
            continue;

        if (check->lightlevel < min)
            min = check->lightlevel;
    }
    return min;
}

//
// P_SectorActive()
//
// Passed a linedef special class (floor, ceiling, lighting) and a sector
// returns whether the sector is already busy with a linedef special of the
// same class. If old demo compatibility true, all linedef special classes
// are the same.
//
//
int P_SectorActive(special_e t,sector_t *sec)
{
  if (!boomsupport)
    return sec->floordata || sec->ceilingdata || sec->lightingdata;
  else
    switch (t)
    {
      case floor_special:
        return (int)sec->floordata;
      case ceiling_special:
        return (int)sec->ceilingdata;
      case lighting_special:
        return (int)sec->lightingdata;
    }
  return 1;
}


//SoM: 3/7/2000
//
// P_CheckTag()
//
// Passed a line, returns true if the tag is non-zero or the line special
// allows no tag without harm. If compatibility, all linedef specials are
// allowed to have zero tag.
//
// Note: Only line specials activated by walkover, pushing, or shooting are
//       checked by this routine.
//
// Another note, written by Graue 12-04-2003: This function is no longer
// used for anything at all. It is only called by commented-out code.
// Weird, huh?
int P_CheckTag(line_t *line)
{
  if (!boomsupport)
    return 1;

  if (line->tag)
    return 1;

  switch(line->special)
  {
    case 1:                 // Manual door specials
    /*case 26:*/ // Tails
    case 27:
    case 28:
    case 31:
    case 32:
    /*case 33:*/ // Tails
    case 34:
    case 117:
    case 118:

    case 139:               // Lighting specials
    case 170:
    case 79:
//    case 35:
    case 138:
    case 171:
    case 81:
    case 13:
    case 192:
    case 169:
    case 80:
    case 12:
    case 194:
    case 173:
    case 157:
    case 104:
    case 193:
    case 172:
    case 156:
    case 17:

    case 195:               // Thing teleporters
    case 174:
    case 97:
    case 39:
    case 126:
    case 125:
    case 210:
    case 209:
    case 208:
    case 207:

    case 11:                // Exits
    case 197:
    case 124:
    case 198:

    case 100:                // Scrolling walls // Tails
    case 85:
    // FraggleScript types!
//    case 272:   // WR
    case 273:
    case 274:   // W1
    case 275:
    case 276:   // SR
    case 277:   // S1
    case 278:   // GR
    case 279:   // G1
      return 1;   // zero tag allowed

    default:
      break;
  }
  return 0;       // zero tag not allowed
}



//SoM: 3/7/2000: Is/WasSecret.
//
// P_IsSecret()
//
// Passed a sector, returns if the sector secret type is still active, i.e.
// secret type is set and the secret has not yet been obtained.
//
boolean P_IsSecret(sector_t *sec)
{
  return (sec->special==9 || (sec->special&SECRET_MASK));
}


//
// P_WasSecret()
//
// Passed a sector, returns if the sector secret type is was active, i.e.
// secret type was set and the secret has been obtained already.
//
boolean P_WasSecret(sector_t *sec)
{
  return (sec->oldspecial==9 || (sec->oldspecial&SECRET_MASK));
}


//
// EVENTS
// Events are operations triggered by using, crossing,
// or shooting special lines, or by timed thinkers.
//
/*
//
// P_CrossSpecialLine - TRIGGER
// Called every time a thing origin is about
//  to cross a line with a non 0 special.
//
void
P_CrossSpecialLine
( int           linenum,
  int           side,
  mobj_t*       thing )
{
    line_t*     line;
    line = &lines[linenum];

    P_ActivateCrossedLine(line, side, thing);
}


void
P_ActivateCrossedLine
( line_t*       line,
  int           side,
  mobj_t*       thing)
{
    int         ok;
    int         forceuse; //SoM: 4/26/2000: ALLTRIGGER should allow monsters to use generalized types too!

    forceuse = line->flags & ML_ALLTRIGGER && thing->type != MT_BLOOD;

    //  Triggers that other things can activate
    if (!thing->player)
    {
        // Things that should NOT trigger specials...
        switch(thing->type)
        {
          case MT_ROCKET:
          case MT_PLASMA:
          case MT_TROOPSHOT:
          case MT_HEADSHOT:
          case MT_BRUISERSHOT:
            return;
            break;

          default: break;
        }
    }

    //SoM: 3/7/2000: Check for generalized line types/
    if (boomsupport)
    {
      // pointer to line function is NULL by default, set non-null if
      // line special is walkover generalized linedef type
      int (*linefunc)(line_t *line)=NULL;
  
      // check each range of generalized linedefs
      if ((unsigned)line->special >= GenFloorBase)
      {
        if (!thing->player)
          if (((line->special & FloorChange) || !(line->special & FloorModel)) && !forceuse)
            return;     // FloorModel is "Allow Monsters" if FloorChange is 0
        if (!line->tag)
          return;
        linefunc = EV_DoGenFloor;
      }
      else if ((unsigned)line->special >= GenCeilingBase)
      {
        if (!thing->player)
          if (((line->special & CeilingChange) || !(line->special & CeilingModel)) && !forceuse)
            return;     // CeilingModel is "Allow Monsters" if CeilingChange is 0
        if (!line->tag)
          return;
        linefunc = EV_DoGenCeiling;
      }
      else if ((unsigned)line->special >= GenDoorBase)
      {
        if (!thing->player)
        {
          if (!(line->special & DoorMonster) && !forceuse)
            return;                    // monsters disallowed from this door
//          if (line->flags & ML_SECRET) // they can't open secret doors either
//            return; // Tails
        }
        if (!line->tag)
          return;
        linefunc = EV_DoGenDoor;
      }
      else if ((unsigned)line->special >= GenLockedBase)
      {
        if (!thing->player)
          return;                     // monsters disallowed from unlocking doors
        if (((line->special&TriggerType)==WalkOnce) || ((line->special&TriggerType)==WalkMany))
        {
            return;
        }
        else
          return;
        linefunc = EV_DoGenLockedDoor;
      }
      else if ((unsigned)line->special >= GenLiftBase)
      {
        if (!thing->player)
          if (!(line->special & LiftMonster) && !forceuse)
            return; // monsters disallowed
        if (!line->tag)
          return;
        linefunc = EV_DoGenLift;
      }
      else if ((unsigned)line->special >= GenStairsBase)
      {
        if (!thing->player)
          if (!(line->special & StairMonster) && !forceuse)
            return; // monsters disallowed
        if (!line->tag)
          return;
        linefunc = EV_DoGenStairs;
      }
      else if ((unsigned)line->special >= GenCrusherBase)
      {
        if (!thing->player)
          if (!(line->special & StairMonster) && !forceuse)
            return; // monsters disallowed
        if (!line->tag)
          return;
        linefunc = EV_DoGenCrusher;
      }
  
      if (linefunc) // if it was a valid generalized type
        switch((line->special & TriggerType) >> TriggerTypeShift)
        {
          case WalkOnce:
            if (linefunc(line))
              line->special = 0;    // clear special if a walk once type
            return;
          case WalkMany:
            linefunc(line);
            return;
          default:                  // if not a walk type, do nothing here
            return;
        }
      }


      if(!thing->player)
      {
        ok = 0;
        switch(line->special)
        {
          case 39:      // TELEPORT TRIGGER
          case 97:      // TELEPORT RETRIGGER
          case 125:     // TELEPORT MONSTERONLY TRIGGER
          case 126:     // TELEPORT MONSTERONLY RETRIGGER
          case 4:       // RAISE DOOR
          case 10:      // PLAT DOWN-WAIT-UP-STAY TRIGGER
          case 88:      // PLAT DOWN-WAIT-UP-STAY RETRIGGER
            ok = 1;
            break; 
          // SoM: 3/4/2000: Add boom compatibility for extra monster usable
          // linedef types.
          case 208:     //SoM: Silent thing teleporters
          case 207:
          case 243:     //Silent line to line teleporter
          case 244:     //Same as above but trigger once.
          case 262:     //Same as 243 but reversed
          case 263:     //Same as 244 but reversed
          case 264:     //Monster only, silent, trigger once, reversed
          case 265:     //Same as 264 but repeatable
          case 266:     //Monster only, silent, trigger once
          case 267:     //Same as 266 bot repeatable
          case 268:     //Monster only, silent, trigger once, set pos to thing
          case 269:     //Monster only, silent, repeatable, set pos to thing
            if(boomsupport)
              ok = 1;
            break;
        }
        //SoM: Anything can trigger this line!
        if(line->flags & ML_ALLTRIGGER)
          ok = 1;

        if (!ok)
            return;
    }

    if (!P_CheckTag(line) && boomsupport)
      return;

    // Note: could use some const's here.
    switch (line->special)
    {
        // TRIGGERS.
        // All from here to RETRIGGERS.
      case 2:
        // Open Door
        if(EV_DoDoor(line,dooropen,VDOORSPEED) || !boomsupport)
          line->special = 0;
        break;

      case 3:
        // Close Door
        if(EV_DoDoor(line,doorclose,VDOORSPEED) || !boomsupport)
          line->special = 0;
        break;

      case 4:
        // Raise Door
        if(EV_DoDoor(line,normalDoor,VDOORSPEED) || !boomsupport)
          line->special = 0;
        break;

      case 5:
        // Raise Floor
        if(EV_DoFloor(line,raiseFloor) || !boomsupport)
          line->special = 0;
        break;

      case 6:
        // Fast Ceiling Crush & Raise
        if(EV_DoCeiling(line,fastCrushAndRaise) || !boomsupport)
          line->special = 0;
        break;

      case 8:
        // Build Stairs
        if(EV_BuildStairs(line, build8) || !boomsupport)
          line->special = 0;
        break;

      case 10:
        // PlatDownWaitUp
        if(EV_DoPlat(line,downWaitUpStay,0) || !boomsupport)
          line->special = 0;
        break;

      case 12:
        // Light Turn On - brightest near
        if(EV_LightTurnOn(line,0) || !boomsupport)
          line->special = 0;
        break;

      case 13:
        // Light Turn On 255
        if(EV_LightTurnOn(line,255) || !boomsupport)
          line->special = 0;
        break;

      case 16:
        // Close Door 30
        if(EV_DoDoor(line,close30ThenOpen,VDOORSPEED) || !boomsupport)
          line->special = 0;
        break;

      case 17:
        // Start Light Strobing
        if(EV_StartLightStrobing(line) || !boomsupport)
          line->special = 0;
        break;

      case 19:
        // Lower Floor
        if(EV_DoFloor(line,lowerFloor) || !boomsupport)
          line->special = 0;
        break;

      case 22:
        // Raise floor to nearest height and change texture
        if(EV_DoPlat(line,raiseToNearestAndChange,0) || !boomsupport)
          line->special = 0;
        break;

      case 25:
        // Ceiling Crush and Raise
        if(EV_DoCeiling(line,crushAndRaise) || !boomsupport)
          line->special = 0;
        break; // Tails

      case 30:
        // Raise floor to shortest texture height
        //  on either side of lines.
        if(EV_DoFloor(line,raiseToTexture) || !boomsupport)
          line->special = 0;
        break;

      case 35:
        // Lights Very Dark
        if(EV_LightTurnOn(line,35) || !boomsupport)
          line->special = 0;
        break;

      case 36:
        // Lower Floor (TURBO)
        if(EV_DoFloor(line,turboLower) || !boomsupport)
          line->special = 0;
        break;

      case 37:
        // LowerAndChange
        if(EV_DoFloor(line,lowerAndChange) || !boomsupport)
          line->special = 0;
        break;

      case 38:
        // Lower Floor To Lowest
        if(EV_DoFloor( line, lowerFloorToLowest ) || !boomsupport)
          line->special = 0;
        break;

      case 39:
        // TELEPORT!
        if(EV_Teleport( line, side, thing ) || !boomsupport)
          line->special = 0;
        break;

      case 40:
        // RaiseCeilingLowerFloor
        if(EV_DoCeiling( line, raiseToHighest ) || EV_DoFloor( line, lowerFloorToLowest ) ||
           !boomsupport)
          line->special = 0;
        break;

      case 44:
        // Ceiling Crush
        if(EV_DoCeiling( line, lowerAndCrush ) || !boomsupport)
          line->special = 0;
        break; // Tails

      case 52:
        // EXIT!
        if( cv_allowexitlevel.value )
        {
            G_ExitLevel ();
            line->special = 0;  // heretic have right
        }
        break;

      case 53:
        // Perpetual Platform Raise
        if(EV_DoPlat(line,perpetualRaise,0) || !boomsupport)
          line->special = 0;
        break;

      case 54:
        // Platform Stop
        if(EV_StopPlat(line) || !boomsupport)
          line->special = 0;
        break;

      case 56:
        // Raise Floor Crush
        if(EV_DoFloor(line,raiseFloorCrush) || !boomsupport)
          line->special = 0;
        break;

      case 57:
        // Ceiling Crush Stop
        if(EV_CeilingCrushStop(line) || !boomsupport)
          line->special = 0;
        break;

      case 58:
        // Raise Floor 24
        if(EV_DoFloor(line,raiseFloor24) || !boomsupport)
          line->special = 0;
        break;

      case 59:
        // Raise Floor 24 And Change
        if(EV_DoFloor(line,raiseFloor24AndChange) || !boomsupport)
          line->special = 0;
        break;

      case 104:
        // Turn lights off in sector(tag)
        if(EV_TurnTagLightsOff(line) || !boomsupport)
          line->special = 0;
        break;

      case 108:
        // Blazing Door Raise (faster than TURBO!)
        if(EV_DoDoor (line,blazeRaise,4*VDOORSPEED) || !boomsupport)
          line->special = 0;
        break;

      case 109:
        // Blazing Door Open (faster than TURBO!)
        if(EV_DoDoor (line,blazeOpen,4*VDOORSPEED) || !boomsupport)
          line->special = 0;
        break;

      case 100:
          // Build Stairs Turbo 16
          if(EV_BuildStairs(line,turbo16) || !boomsupport)
            line->special = 0;
        break;

      case 110:
        // Blazing Door Close (faster than TURBO!)
        if(EV_DoDoor (line,blazeClose,4*VDOORSPEED) || !boomsupport)
          line->special = 0;
        break;

      case 119:
        // Raise floor to nearest surr. floor
        if(EV_DoFloor(line,raiseFloorToNearest) || !boomsupport)
          line->special = 0;
        break;

      case 121:
        // Blazing PlatDownWaitUpStay
        if(EV_DoPlat(line,blazeDWUS,0) || !boomsupport)
          line->special = 0;
        break;

      case 124:
        // Secret EXIT
        if( cv_allowexitlevel.value )
            G_SecretExitLevel ();
        break;

      case 125:
        // TELEPORT MonsterONLY
        if (!thing->player)
        {
            if(EV_Teleport( line, side, thing ) || !boomsupport)
              line->special = 0;
        }
        break;

      case 130:
        // Raise Floor Turbo
        if(EV_DoFloor(line,raiseFloorTurbo) || !boomsupport)
          line->special = 0;
        break;

      case 141:
        // Silent Ceiling Crush & Raise
        if(EV_DoCeiling(line,silentCrushAndRaise) || !boomsupport)
          line->special = 0;
        break;

      //SoM: FraggleScript
      case 273: //(1sided)
        if(side) break;

      case 272: //(2sided)
        t_trigger = thing;
        T_RunScript(line->tag);
        break;

      // once-only triggers
      case 275: //(1sided)
        if(side) break;

      case 274: //(2sided)
        t_trigger = thing;
        T_RunScript(line->tag);
        line->special = 0;        // clear trigger
        break;


      // RETRIGGERS.  All from here till end.
      case 72:
        // Ceiling Crush
        EV_DoCeiling( line, lowerAndCrush );
        break;

      case 73:
        // Ceiling Crush and Raise
        EV_DoCeiling(line,crushAndRaise);
        break;

      case 74:
        // Ceiling Crush Stop
        EV_CeilingCrushStop(line);
        break;

      case 75:
        // Close Door
        EV_DoDoor(line,doorclose,VDOORSPEED);
        break;

      case 76:
        // Close Door 30
        EV_DoDoor(line,close30ThenOpen,VDOORSPEED);
        break;

      case 77:
        // Fast Ceiling Crush & Raise
        EV_DoCeiling(line,fastCrushAndRaise);
        break;

      case 79:
        // Lights Very Dark
        EV_LightTurnOn(line,35);
        break;

      case 80:
        // Light Turn On - brightest near
        EV_LightTurnOn(line,0);
        break;

      case 81:
        // Light Turn On 255
        EV_LightTurnOn(line,255);
        break;

      case 82:
        // Lower Floor To Lowest
        EV_DoFloor( line, lowerFloorToLowest );
        break;

      case 83:
        // Lower Floor
        EV_DoFloor(line,lowerFloor);
        break;

      case 84:
        // LowerAndChange
        EV_DoFloor(line,lowerAndChange);
        break;

      case 86:
        // Open Door
        EV_DoDoor(line,dooropen,VDOORSPEED);
        break;

      case 87:
        // Perpetual Platform Raise
        EV_DoPlat(line,perpetualRaise,0);
        break;

      case 88:
        // PlatDownWaitUp
        EV_DoPlat(line,downWaitUpStay,0);
        break;

      case 89:
        // Platform Stop
        EV_StopPlat(line);
        break;

      case 90:
        // Raise Door
        EV_DoDoor(line,normalDoor,VDOORSPEED);
        break;

      case 91:
        // Raise Floor
        EV_DoFloor(line,raiseFloor);
        break;

      case 92:
        // Raise Floor 24
        EV_DoFloor(line,raiseFloor24);
        break;

      case 93:
        // Raise Floor 24 And Change
        EV_DoFloor(line,raiseFloor24AndChange);
        break;

      case 94:
        // Raise Floor Crush
        EV_DoFloor(line,raiseFloorCrush);
        break;

      case 95:
        // Raise floor to nearest height
        // and change texture.
        EV_DoPlat(line,raiseToNearestAndChange,0);
        break;

      case 96:
        // Raise floor to shortest texture height
        // on either side of lines.
        EV_DoFloor(line,raiseToTexture);
        break;

      case 97:
        // TELEPORT!
        EV_Teleport( line, side, thing );
        break;

      case 98:
        // Lower Floor (TURBO)
        EV_DoFloor(line,turboLower);
        break;

      case 105:
            // Blazing Door Raise (faster than TURBO!)
            EV_DoDoor (line,blazeRaise,4*VDOORSPEED);
        break;

      case 106:
            // Blazing Door Open (faster than TURBO!)
            EV_DoDoor (line,blazeOpen,4*VDOORSPEED);
        break;

      case 107:
            // Blazing Door Close (faster than TURBO!)
            EV_DoDoor (line,blazeClose,4*VDOORSPEED);
        break;

      case 120:
        // Blazing PlatDownWaitUpStay.
        EV_DoPlat(line,blazeDWUS,0);
        break;

      case 126:
        // TELEPORT MonsterONLY.
        if (!thing->player)
            EV_Teleport( line, side, thing );
        break;

      case 128:
        // Raise To Nearest Floor
        EV_DoFloor(line,raiseFloorToNearest);
        break;

      case 129:
        // Raise Floor Turbo
        EV_DoFloor(line,raiseFloorTurbo);
        break;

      // SoM:3/4/2000: Extended Boom W* triggers.
      default:
        if(boomsupport) {
          switch(line->special) {
            //SoM: 3/4/2000:Boom Walk once triggers.
            //SoM: 3/4/2000:Yes this is "copied" code! I just cleaned it up. Did you think I was going to retype all this?!
            case 142:
              // Raise Floor 512
              if (EV_DoFloor(line,raiseFloor512))
                line->special = 0;
              break;
  
            case 143:
              // Raise Floor 24 and change
              if (EV_DoPlat(line,raiseAndChange,24))
                line->special = 0;
              break;

            case 144:
              // Raise Floor 32 and change
              if (EV_DoPlat(line,raiseAndChange,32))
                line->special = 0;
              break;

            case 145:
              // Lower Ceiling to Floor
              if (EV_DoCeiling( line, lowerToFloor ))
                line->special = 0;
              break;

            case 146:
              // Lower Pillar, Raise Donut
              if (EV_DoDonut(line))
                line->special = 0;
              break;

            case 199:
              // Lower ceiling to lowest surrounding ceiling
              if (EV_DoCeiling(line,lowerToLowest))
                line->special = 0;
              break;

            case 200:
              // Lower ceiling to highest surrounding floor
              if (EV_DoCeiling(line,lowerToMaxFloor))
                line->special = 0;
              break;

            case 207:
              // W1 silent teleporter (normal kind)
              if (EV_SilentTeleport(line, side, thing))
                line->special = 0;
              break;

            case 153: 
              // Texture/Type Change Only (Trig)
              if (EV_DoChange(line,trigChangeOnly))
                line->special = 0;
              break;
  
            case 239: 
              // Texture/Type Change Only (Numeric)
              if (EV_DoChange(line,numChangeOnly))
                line->special = 0;
              break;
 
            case 219:
              // Lower floor to next lower neighbor
              if (EV_DoFloor(line,lowerFloorToNearest))
                line->special = 0;
              break;

            case 227:
              // Raise elevator next floor
              if (EV_DoElevator(line,elevateUp))
                line->special = 0;
              break;

            case 231:
              // Lower elevator next floor
              if (EV_DoElevator(line,elevateDown))
                line->special = 0;
              break;

            case 235:
              // Elevator to current floor
              if (EV_DoElevator(line,elevateCurrent))
                line->special = 0;
              break;

            case 243: 
              // W1 silent teleporter (linedef-linedef kind)
              if (EV_SilentLineTeleport(line, side, thing, false))
                line->special = 0;
              break;

            case 262: 
              if (EV_SilentLineTeleport(line, side, thing, true))
                line->special = 0;
              break;
 
            case 264: 
              if (!thing->player &&
              EV_SilentLineTeleport(line, side, thing, true))
                line->special = 0;
              break;

            case 266: 
              if (!thing->player &&
                  EV_SilentLineTeleport(line, side, thing, false))
                line->special = 0;
              break;

            case 268: 
              if (!thing->player && EV_SilentTeleport(line, side, thing))
                line->special = 0;
              break;

            // Extended walk many retriggerable
 
            //Boom added lots of linedefs to fill in the gaps in trigger types

            case 147:
              // Raise Floor 512
              EV_DoFloor(line,raiseFloor512);
              break;

            case 148:
              // Raise Floor 24 and Change
              EV_DoPlat(line,raiseAndChange,24);
              break;

            case 149:
              // Raise Floor 32 and Change
              EV_DoPlat(line,raiseAndChange,32);
              break;

            case 150:
              // Start slow silent crusher
              EV_DoCeiling(line,silentCrushAndRaise);
              break;

            case 151:
              // RaiseCeilingLowerFloor
              EV_DoCeiling( line, raiseToHighest );
              EV_DoFloor( line, lowerFloorToLowest );
              break;

            case 152:
              // Lower Ceiling to Floor
              EV_DoCeiling( line, lowerToFloor );
              break;

            case 256:
              // Build stairs, step 8
              EV_BuildStairs(line,build8);
              break;

            case 257:
              // Build stairs, step 16
              EV_BuildStairs(line,turbo16);
              break;

            case 155:
              // Lower Pillar, Raise Donut
              EV_DoDonut(line);
              break;

            case 156:
              // Start lights strobing
              EV_StartLightStrobing(line);
              break;

            case 157:
              // Lights to dimmest near
              EV_TurnTagLightsOff(line);
              break;

            case 201:
              // Lower ceiling to lowest surrounding ceiling
              EV_DoCeiling(line,lowerToLowest);
              break;

            case 202:
              // Lower ceiling to highest surrounding floor
              EV_DoCeiling(line,lowerToMaxFloor);
              break;

            case 208:
              // WR silent teleporter (normal kind)
              EV_SilentTeleport(line, side, thing);
              break;

            case 212:
              // Toggle floor between C and F instantly
              EV_DoPlat(line,toggleUpDn,0);
              break;

            case 154:
              // Texture/Type Change Only (Trigger)
              EV_DoChange(line,trigChangeOnly);
              break;

            case 240: 
              // Texture/Type Change Only (Numeric)
              EV_DoChange(line,numChangeOnly);
              break;

            case 220:
              // Lower floor to next lower neighbor
              EV_DoFloor(line,lowerFloorToNearest);
              break;

            case 228:
              // Raise elevator next floor
              EV_DoElevator(line,elevateUp);
              break;

            case 232:
              // Lower elevator next floor
              EV_DoElevator(line,elevateDown); // Tails
              break;

            case 236:
              // Elevator to current floor
              EV_DoElevator(line,elevateCurrent);
              break;

            case 244: 
              // WR silent teleporter (linedef-linedef kind)
              EV_SilentLineTeleport(line, side, thing, false);
              break;

            case 263: 
              //Silent line-line reversed
              EV_SilentLineTeleport(line, side, thing, true);
              break;

            case 265: 
              //Monster-only silent line-line reversed
              if (!thing->player)
                EV_SilentLineTeleport(line, side, thing, true);
              break;

            case 267: 
              //Monster-only silent line-line
              if (!thing->player)
                EV_SilentLineTeleport(line, side, thing, false);
              break;

            case 269: 
              //Monster-only silent
              if (!thing->player)
                EV_SilentTeleport(line, side, thing);
              break;
              }
            }
    }
}*/

// Crap for CTF Flags Tails 08-02-2001
mapthing_t     *itemrespawnque[ITEMQUESIZE];
//int             itemrespawntime[ITEMQUESIZE];
int             iquehead;
int             iquetail;

void P_DoPlayerExit(player_t* player);
void P_ResetScore(player_t* player);
void P_InstaThrust(mobj_t* mo, angle_t angle, fixed_t move);

//
// P_ProcessLineSpecial
//
// Processes the line specials triggered by a user
// (Kind of like SRB2's version of trip lines)
void P_ProcessLineSpecial(line_t* line, mobj_t* mo)
{
	int secnum = -1;

	switch(line->special) {
		case 101: // Set tagged sector's floor height/pic Graue 11-04-2003
			EV_DoFloor(line, instantMoveFloorByFrontSector); // Do it the easy way Graue 12-12-2003
			break;

		case 102: // Set tagged sector's ceiling height/pic Graue 11-05-2003
			EV_DoCeiling(line, instantMoveCeilingByFrontSector); // Do it the easy way Graue 12-12-2003
			break;

		case 103: // Set tagged sector's light level Graue 11-05-2003
			{
				short newlightlevel;
				int newfloorlightsec, newceilinglightsec;

				//if(!line->backsector)
				//	break;

				newlightlevel = line->frontsector->lightlevel;
				newfloorlightsec = line->frontsector->floorlightsec;
				newceilinglightsec = line->frontsector->ceilinglightsec;

				// act on all sectors with the same tag as the triggering linedef
				while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
				{
					sectors[secnum].lightlevel = newlightlevel;
					sectors[secnum].floorlightsec = newfloorlightsec;
					sectors[secnum].ceilinglightsec = newceilinglightsec;
				}
			}
			break;

		case 104: // Teleport the player Graue 11-06-2003 (or thing Graue 12-31-2003)
			{
				mobj_t* dest;

				if((secnum = P_FindSectorFromLineTag(line, -1)) < 0)
					return;

				if((dest = P_GetTeleportDestThing(secnum)) == NULL)
					return;

				P_Teleport(mo, dest->x, dest->y, dest->angle);
				mo->z = dest->z;
				mo->momx = mo->momy = mo->momz = 1;
				if(mo->player)
					mo->player->bonuscount += 10; // Flash the palette

				// Play the 'bowrwoosh!' sound
				S_StartSound(dest, sfx_mixup);
			}
			break;

		case 105: // Change music Graue 11-16-2003
			{
				fixed_t musicnum;

				musicnum = P_AproxDistance(line->dx, line->dy);

				if(musicnum < NUMMUSIC && musicnum > mus_None)
					S_ChangeMusic(musicnum, !(line->flags & ML_NOCLIMB));
				else
					S_StopMusic();

				// FIXTHIS: goes back to level music when interrupted
				//          (extra life ditty, speed shoes, etc.)
			}
			break;

		case 106: // Move floor, linelen = speed, frontsector floor = dest height Graue 12-12-2003
			EV_DoFloor(line, moveFloorByFrontSector);
			break;

		case 107: // Move ceiling, linelen = speed, frontsector ceiling = dest height Graue 12-12-2003
			EV_DoCeiling(line, moveCeilingByFrontSector);
			break;

		case 108: // Lower floor by line, dx = speed, dy = amount to lower Graue 12-20-2003
			EV_DoFloor(line, lowerFloorByLine);
			break;

		case 109: // Raise floor by line, dx = speed, dy = amount to raise Graue 12-20-2003
			EV_DoFloor(line, raiseFloorByLine);
			break;

		case 110: // Lower ceiling by line, dx = speed, dy = amount to lower Graue 12-20-2003
			EV_DoCeiling(line, lowerCeilingByLine);
			break;

		case 111: // Raise ceiling by line, dx = speed, dy = amount to raise Graue 12-20-2003
			EV_DoCeiling(line, raiseCeilingByLine);
			break;

		case 112: // Change sector's tag (not done yet)
			break;

		default:
			break;
	}
}

// Graue 12-06-2003
void P_ResetPlayer(player_t* player);
void P_PlayerFlagBurst(player_t* player);
void P_CheckFragLimit(player_t* p);

extern consvar_t cv_fraglimit; // Graue 12-23-2003

//
// P_ProcessSpecialSector
// Function that actually applies the sector special to the player.
void P_ProcessSpecialSector(player_t* player, sector_t* sector, boolean roverspecial)
{
	mobj_t*		mo; // Tails 08-02-2001
	line_t    junk; // Tails 02-06-2002
	int i = 0;

	// Conveyor stuff Graue 12-26-2003
	if(sector->special == 512 || sector->special == 984 || sector->special == 985)
		player->onconveyor = sector->special;
	// Removed else, now unnecessary, and would've caused bugs anyway Graue 01-01-2004

	switch(sector->special)
	{
		case 6: // Space countdown
			if(!player->powers[pw_greenshield] && !player->powers[pw_spacetime])
			{
				player->powers[pw_spacetime] = spacetimetics + 1;
				S_ChangeMusic(mus_drown, false); // Tails 03-14-2000
			}
			return;
		case 10: // Instant kill
			P_DamageMobj(player->mo, NULL, NULL, 10000);
			return;
		case 982: // Exit (for FOF exits; others are handled in p_user.c somewhere)
			if(roverspecial)
			{
				// Graue 12-13-2003: oh, if it were only that simple...
				// this is mostly a copy/paste from p_user.c handling of exits
				extern consvar_t cv_numlaps;
				extern int numstarposts;
				int lineindex;
				boolean skipexit = false;

				if(cv_gametype.value == GT_CIRCUIT) // Graue 11-15-2003
				{
					if(player->starpostnum == numstarposts) // Must have touched all the starposts
					{
						extern consvar_t cv_lapcounteridiocy; // Graue 12-25-2003

						player->laps++;
						if(cv_lapcounteridiocy.value)
						{
							if(player->laps == cv_numlaps.value)
								CONS_Printf("%s finished the final lap\n",player_names[player-players]);
							else
								CONS_Printf("%s started lap %d\n",player_names[player-players],player->laps+1);
						}
						else
							CONS_Printf("%s finished lap %d\n",player_names[player-players],player->laps);

						// Reset starposts (checkpoints) info
						player->starpostangle = player->starposttime = player->starpostnum = player->starpostbit = 0;
						player->starpostx = player->starposty = player->starpostz = 0;
					}

					if(player->laps < cv_numlaps.value)
						skipexit = true;
				}

				if(!skipexit) {
					P_DoPlayerExit(player);
					lineindex = P_FindSpecialLineFromTag(71, sector->tag); // Graue 12-20-2003

					if(lineindex != -1) // Custom exit!
					{
						// Special goodies with the PASSUSE flag depending on emeralds collected Graue 12-31-2003
						if(lines[lineindex].flags & ML_PASSUSE)
						{
							if(emeralds & (EMERALD1 | EMERALD2 | EMERALD3 | EMERALD4 | EMERALD5 | EMERALD6 | EMERALD7))
							{
								if(emeralds & EMERALD8) // The secret eighth emerald. Use linedef length.
									nextmapoverride = P_AproxDistance(lines[lineindex].dx, lines[lineindex].dy);
								else // The first seven, not bad. Use front sector's ceiling.
									nextmapoverride = lines[lineindex].frontsector->ceilingheight;
							}
							else // Don't have all eight emeralds, or even the first seven? Use front sector's floor.
								nextmapoverride = lines[lineindex].frontsector->floorheight;
						}
						else
							nextmapoverride = P_AproxDistance(lines[lineindex].v1->x - lines[lineindex].v2->x, lines[lineindex].v1->y - lines[lineindex].v2->y);

						nextmapoverride >>= FRACBITS; // Graue 12-31-2003

						if(lines[lineindex].flags & ML_NOCLIMB)
							skipstats = true;
					}
				}
			}
			return;
		case 974: // Linedef executor that doesn't require touching floor Graue 12-20-2003
		case 975: // Linedef executor
			{
				int masterline, specialtype = 96;

				// Support multiple trigger types (96-98) Graue 12-19-2003
				while((masterline = P_FindSpecialLineFromTag(specialtype, sector->tag)) < 0)
				{
					specialtype++;
					if(specialtype > 98) // After 98, there aren't any more activators
						return;
				}

				// Special type 97 only works once when you hit floor Graue 12-20-2003
				if(specialtype == 97 && !(player->mo->eflags & MF_JUSTHITFLOOR))
					return;

				{
					int linecnt;
					sector_t* ctlsector; // Graue 11-04-2003

					ctlsector = lines[masterline].frontsector;
					linecnt = ctlsector->linecount;
					for(i=0; i<linecnt; i++)
					{
						if(ctlsector->lines[i]->special != 0
							&& (ctlsector->lines[i]->special < 96 || ctlsector->lines[i]->special > 98)
							&& ctlsector->lines[i]->tag)
							P_ProcessLineSpecial(ctlsector->lines[i], player->mo);
					}
				}

				// Special type 98 only works once Graue 12-20-2003
				if(specialtype == 98)
					lines[masterline].special = 0; // Clear it out
			}		
			return;
		case 976: // Speed pad w/o spin
			//if(player->mo->z > player->mo->floorz)
			//	return;

			if(player->powers[pw_flashing] != 0 && player->powers[pw_flashing] < TICRATE/2)
				return;

			i = P_FindSpecialLineFromTag(65, sector->tag);

			if(i != -1)
			{
				angle_t lineangle;

				lineangle = R_PointToAngle2(lines[i].v2->x, lines[i].v2->y, lines[i].v1->x, lines[i].v1->y);

				player->mo->angle = lineangle;
				
				if(player == &players[consoleplayer])
					localangle = player->mo->angle;
				else if(player == &players[secondarydisplayplayer])
					localangle2 = player->mo->angle;

				P_UnsetThingPosition(player->mo);
				player->mo->x = sector->soundorg.x;
				player->mo->y = sector->soundorg.y;
				P_SetThingPosition(player->mo);

				P_InstaThrust(player->mo, player->mo->angle, MAXMOVE);
				player->powers[pw_flashing] = TICRATE/3;
				S_StartSound(player->mo, sfx_spdpad);
			}
			return;
		case 977: // Speed pad w/ spin
			//if(player->mo->z > player->mo->floorz)
			//	return;

			if(player->powers[pw_flashing] != 0 && player->powers[pw_flashing] < TICRATE/2)
				return;

			i = P_FindSpecialLineFromTag(65, sector->tag);

			if(i != -1)
			{
				angle_t lineangle;

				lineangle = R_PointToAngle2(lines[i].v2->x, lines[i].v2->y, lines[i].v1->x, lines[i].v1->y);

				player->mo->angle = lineangle;
				
				if(player == &players[consoleplayer])
					localangle = player->mo->angle;
				else if(player == &players[secondarydisplayplayer])
					localangle2 = player->mo->angle;

				P_InstaThrust(player->mo, player->mo->angle, MAXMOVE);

				P_ResetScore(player);
				player->mfspinning = 1;

				P_UnsetThingPosition(player->mo);
				player->mo->x = sector->soundorg.x;
				player->mo->y = sector->soundorg.y;
				P_SetThingPosition(player->mo);

				P_SetMobjState (player->mo, S_PLAY_ATK1);
				player->powers[pw_flashing] = TICRATE/3;
				S_StartSound(player->mo, sfx_spdpad);
			}
			return;
		case 978: // Lose one ring per second
			if(leveltime % 15 == 0 && player->mo->health > 1)
			{
				player->mo->health--;
				player->health--;
				S_StartSound(player->mo, sfx_itemup);
			}
			return;
		case 979: // Make player spin
			// Graue 12-08-2003: do we really need to check z with floorz here? possibly FIXTHIS
			if(!player->mfspinning && player->mo->z == player->mo->floorz)
			{
				P_ResetScore(player);
				player->mfspinning = 1;
				P_SetMobjState (player->mo, S_PLAY_ATK1);
				S_StartSound (player->mo, sfx_spin);
			}
			else if(player->mfspinning && player->speed <= 5)
				P_InstaThrust(player->mo, player->mo->angle, 8*FRACUNIT);
			return;
		case 980: // Does nothing?
		case 985: // Conveyor belt
		case 987: // No Tag Zone
		case 990: // Special Stage Time/Rings
		case 991: // Custom Gravity
		case 992: // Ramp Sector
		case 994: // Red Team's Goal Graue 12-31-2003
		case 995: // Blue Team's Goal Graue 12-31-2003
			return;
		case 993: // Starpost Activator Graue 12-08-2003
			{
				mobj_t* post;

				if((post = P_GetStarpostThing(sector - sectors)) == NULL)
					return;

				P_TouchSpecialThing(post, player->mo, false);
			}
		case 988: // Red Team's Base
			if(player->mo->z == player->mo->floorz)
			{
				if(player->ctfteam == 1)
				{
					if(player->gotflag & MF_BLUEFLAG)
					{
						int i;
						extern consvar_t cv_ctf_scoring; // Graue 12-13-2003

						// Make sure the red team still has their own
						// flag at their base so they can score.
						// Tails 08-29-2002
						for(i=0; i<MAXPLAYERS; i++)
						{
							if(playeringame[i] && &players[i] != player)
							{
								if(players[i].gotflag & MF_REDFLAG) // Gasp! Someone else has it!
									return; // Sorry Joe, can't score until you've recovered your own flag.
							}
						}

						// Graue 12-13-2003
						if(cv_ctf_scoring.value == 1)
							redscore += 1;
						else
							redscore += 1000;

						if(cv_fraglimit.value) // Graue 12-23-2003
							P_CheckFragLimit(player);

						mo = P_SpawnMobj(player->mo->x,
										player->mo->y,
										player->mo->z,
										MT_BLUEFLAG);
						player->gotflag &= ~MF_BLUEFLAG;
						mo->flags &= ~MF_SPECIAL;
						mo->fuse = TICRATE;
						mo->spawnpoint = bflagpoint;
						CONS_Printf("%s captured the blue flag.\n", player_names[player-players]);
					}
					if(player->gotflag & MF_REDFLAG) // Returning a flag to your base
					{
						mo = P_SpawnMobj(player->mo->x,
										player->mo->y,
										player->mo->z,
										MT_REDFLAG);
						mo->flags &= ~MF_SPECIAL;
						player->gotflag &= ~MF_REDFLAG;
						mo->fuse = TICRATE;
						mo->spawnpoint = rflagpoint;
					}
				}
				return;
			}
			return;
		case 989: // Blue Team's Base
			if(player->mo->z == player->mo->floorz)
			{
				if(player->ctfteam == 2)
				{
					if(player->gotflag & MF_REDFLAG)
					{
						int i;
						extern consvar_t cv_ctf_scoring; // Graue 12-13-2003

						// Make sure the red team still has their own
						// flag at their base so they can score.
						// Tails 08-29-2002
						for(i=0; i<MAXPLAYERS; i++)
						{
							if(playeringame[i] && &players[i] != player)
							{
								if(players[i].gotflag & MF_BLUEFLAG) // Gasp! Someone else has it!
									return; // Sorry Joe, can't score until you've recovered your own flag.
							}
						}

						// Graue 12-13-2003
						if(cv_ctf_scoring.value == 1)
							bluescore += 1;
						else
							bluescore += 1000;

						if(cv_fraglimit.value) // Graue 12-23-2003
							P_CheckFragLimit(player);

						mo = P_SpawnMobj(player->mo->x,
										player->mo->y,
										player->mo->z,
										MT_REDFLAG);
						player->gotflag &= ~MF_REDFLAG;
						mo->flags &= ~MF_SPECIAL;
						mo->fuse = TICRATE;
						mo->spawnpoint = rflagpoint;
						CONS_Printf("%s captured the red flag.\n", player_names[player-players]);
					}
					if(player->gotflag & MF_BLUEFLAG) // Returning a flag to your base
					{
						mo = P_SpawnMobj(player->mo->x,
										player->mo->y,
										player->mo->z,
										MT_BLUEFLAG);
						mo->flags &= ~MF_SPECIAL;
						player->gotflag &= ~MF_BLUEFLAG;
						mo->fuse = TICRATE;
						mo->spawnpoint = bflagpoint;
					}
				}
				return;
			}
			return;
		case 983:
		case 984: // Slime! Tails 04-15-2001
			if(player->powers[pw_underwater] && !player->powers[pw_greenshield])
				P_DamageMobj(player->mo, NULL, NULL, 1);
			return;
		case 18: // Electrical damage Graue 11-04-2003
			if(!player->powers[pw_yellowshield])
				P_DamageMobj(player->mo, NULL, NULL, 1);
			return;
		case 981: // Water rise stuff Tails 02-06-2002
			junk.tag = 744;
			EV_DoCeiling(&junk, raiseToHighest);
			return;
		case 666: // Egg trap capsule
		{
			thinker_t* th;
			mobj_t* mo2;

			if(sector->ceilingdata || sector->floordata)
				return;
			junk.tag = 680;
			EV_DoElevator(&junk, elevateDown, false);
			junk.tag = 681;
			EV_DoFloor(&junk, raiseFloorToNearestFast);
			junk.tag = 682;
			EV_DoCeiling(&junk, lowerToLowestFast);
			sector->special = 0;

			// Find the center of the Eggtrap and release all the pretty animals!
			// The chimps are my friends.. heeheeheheehehee..... - LouisJM
			for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
			{
				if (th->function.acp1 != (actionf_p1)P_MobjThinker)
					continue;

				mo2 = (mobj_t *)th;
				if (mo2->type == MT_EGGTRAP)
				{
					mo2->fuse = TICRATE/7;
					break;
				}
			}
			return;
		}
	}

    if(sector->special < 32)
    {
      // Has hitten ground.
      switch (sector->special)
      {
        case 5: // instant death
          // HELLSLIME DAMAGE
			P_DamageMobj (player->mo, NULL, NULL, 10000); // Tails
          break;

        case 7: // lava and the like
          // NUKAGE DAMAGE
		  if(!player->powers[pw_redshield])
			P_DamageMobj (player->mo, NULL, NULL, 1);
          break;

        case 16: // instant death (again)
          // SUPER HELLSLIME DAMAGE
                P_DamageMobj (player->mo, NULL, NULL, 10000);
				break;
        case 4:
		  // Spikes
          //P_DamageMobj (player->mo, NULL, NULL, 1);
          break;

        case 9:
          // Special Stage Hurt Sector Graue 12-06-2003
			mo = player->mo;
			if(player->powers[pw_redshield] || player->powers[pw_yellowshield] || player->powers[pw_blueshield] || player->powers[pw_greenshield] || player->powers[pw_blackshield])
			{
				player->powers[pw_redshield] = false;
				player->powers[pw_blueshield] = false;
				player->powers[pw_greenshield] = false;
				player->powers[pw_yellowshield] = false;

				if(player->powers[pw_blackshield]) // Give them what's coming to them!
				{
					player->blackow = 1; // BAM!
					player->powers[pw_blackshield] = false;
					player->jumpdown = true;
				}
				player->mo->z++;

				if(player->mo->eflags & MF_UNDERWATER)
					player->mo->momz = 4.04269230769230769230769230769231*FRACUNIT;
				else
					player->mo->momz = 6.9*FRACUNIT;

				P_InstaThrust (player->mo, -player->mo->angle, 4*FRACUNIT);

				P_SetMobjState(player->mo, player->mo->info->painstate);
				player->powers[pw_flashing] = flashingtics; // Tails

				player->powers[pw_fireflower] = false;
				player->mo->flags =  (player->mo->flags & ~MF_TRANSLATION)
	                     | ((player->skincolor)<<MF_TRANSSHIFT);

				P_ResetPlayer(player);

				S_StartSound(player->mo, sfx_shldls); // Ba-Dum! Shield loss.	

				if(cv_gametype.value == GT_CTF && (player->gotflag & MF_REDFLAG || player->gotflag & MF_BLUEFLAG))
					P_PlayerFlagBurst(player);
			}
			if(mo->health > 1)
			{
				if(player->powers[pw_flashing])
					return;
				player->powers[pw_flashing] = flashingtics;
				P_PlayRinglossSound(mo);
				if(mo->health > 10)
					mo->health -= 10;
				else
					mo->health = 1;
				player->health = mo->health;
				mo->z++;
				if(mo->eflags & MF_UNDERWATER)
					mo->momz = 4.04269230769230769230769230769231*FRACUNIT;
				else
					mo->momz = 6.9*FRACUNIT;
				P_InstaThrust (mo, -mo->angle, 4*FRACUNIT);
				P_ResetPlayer(player);
				P_SetMobjState(mo, S_PLAY_PAIN);
			}
			// P_DamageMobj (player->mo, NULL, NULL, 10);
		  break;

        case 11: // Straight damage.
			P_DamageMobj(player->mo, NULL, NULL, 1);          
          break;

		case 14: // Bouncy sector
			break;

        default:
          //SoM: 3/8/2000: Just ignore.
          //CONS_Printf ("P_PlayerInSpecialSector: unknown special %i",
          //             sector->special);
          break;
      };
   }
   else //SoM: Extended sector types for secrets and damage
   {
     switch ((sector->special&DAMAGE_MASK)>>DAMAGE_SHIFT)
     {
       case 0: // no damage
         break;
       case 1: // 2/5 damage per 31 ticks
         P_DamageMobj (player->mo, NULL, NULL, 5);
         break;
       case 2: // 5/10 damage per 31 ticks
         P_DamageMobj (player->mo, NULL, NULL, 10);
         break;
       case 3: // 10/20 damage per 31 ticks
         if (P_Random()<5)  // take damage even with suit
         {
             P_DamageMobj (player->mo, NULL, NULL, 20);
         }
         break;
     }
	 if (sector->special&SECRET_MASK)
     {
       player->secretcount++;
       sector->special &= ~SECRET_MASK;

       if (players-player == displayplayer)
          CONS_Printf ("\2You found a secret area!\n");

       if (sector->special<32)
         sector->special=0;
     }
   }
}

// Graue 12-31-2003
//
// P_ThingOnSpecial3DFloor
//
// Checks to see if a thing is standing on or is inside a special 3D floor, and if so, returns
// the special type.
short P_ThingOnSpecial3DFloor(mobj_t* mo)
{
	sector_t* sector;
	ffloor_t* rover;

	sector = mo->subsector->sector;
	if(!sector->ffloors)
		return 0;

	for(rover = sector->ffloors; rover; rover = rover->next)
	{
		if(!rover->master->frontsector->special)
			continue;

		// Check the 3D floor's type...
		if(rover->flags & FF_SOLID)
		{
			// Thing must be on top of the floor to be affected...
			if(mo->z != *rover->topheight)
				continue;
		}
		else
		{
			// Water and intangible FOFs
			if(mo->z > *rover->topheight || (mo->z + mo->height) < *rover->bottomheight)
				continue;
		}

		return rover->master->frontsector->special;
	}

	return 0;
}

//
// P_PlayerOnSpecial3DFloor
// Checks to see if a player is standing on or is inside a 3D floor (water)
// and applies any speicials..
void P_PlayerOnSpecial3DFloor(player_t* player)
{
  sector_t* sector;
  ffloor_t* rover;

  sector = player->mo->subsector->sector;
  if(!sector->ffloors)
    return;

  for(rover = sector->ffloors; rover; rover = rover->next)
  {
    if(!rover->master->frontsector->special)
      continue;

    // Check the 3D floor's type...
    if(rover->flags & FF_SOLID)
    {
      // Player must be on top of the floor to be affected...
      if(player->mo->z != *rover->topheight)
        continue;
    }
    else
    {
      //Water and DEATH FOG!!! heh
      if(player->mo->z > *rover->topheight || (player->mo->z + player->mo->height) < *rover->bottomheight)
        continue;
    }
 
    P_ProcessSpecialSector(player, rover->master->frontsector, true);
  }
}


//
// P_PlayerInSpecialSector
// Called every tic frame
//  that the player origin is in a special sector
//
void P_PlayerInSpecialSector (player_t* player)
{
    sector_t*   sector;
	line_t      junk; // Tails 12-03-99 buttons and more

	// Fix my stupid conveyor bug. Graue 01-01-2004
	if(!player->mo->subsector->sector->special)
		player->onconveyor = 0;

	// SoM: Check 3D floors...
    P_PlayerOnSpecial3DFloor(player);

    sector = player->mo->subsector->sector;

	if(sector->special == 666) // Egg trap capsule -- should only be for 3dFloors!
		return;

    //Fab: keep track of what sector type the player's currently in
    player->specialsector = sector->special;

#ifdef OLDWATER
    //Fab: VERY NASTY hack for water QUICK TEST !!!!!!!!!!!!!!!!!!!!!!!
    if (sector->tag < 0)
    {
        player->specialsector = 888;    // no particular value
        return;
    }
    else
    // old water (flat texture)
    if (levelflats[sector->floorpic].iswater)
    {
        player->specialsector = 887;
        return;
    }
#endif

    if (!player->specialsector)     // nothing special, exit
        return;

	// Prettify the list of specials that activate without floor touch Graue 12-08-2003
	switch(player->specialsector)
	{
		case 6: // Space countdown
		case 10: // Instant kill
		case 974: // Linedef executor
		case 976: // Speed pad without spin
		case 977: // Speed pad with spin
		case 980: // Does nothing?
		case 981: // Water rise stuff
		case 983: // Damage (water)
		case 984: // Damage (water) + current
		case 993: // Starpost activator
			P_ProcessSpecialSector(player, sector, false);
			return; // I also added this return, hope it doesn't break anything!
			        // (pun not intended)
	}

	// Falling, not all the way down yet?
	//SoM: 3/17/2000: Damage if in slimey water!
	if (sector->heightsec != -1 && !(player->specialsector == 712 || player->specialsector == 713))
	{
		if(player->mo->z > sectors[sector->heightsec].floorheight)
			return;
	}
	if (player->mo->z != sector->floorheight) // Only go further if on the ground
		return;

// De-generalize buttons and the like Tails 04-15-2001
	switch(player->specialsector)
	{
// start button 1 Tails 12-03-99
      case 690:
        junk.tag = 700;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 701;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 1 Tails 12-03-99
// start button 2 Tails 12-03-99
      case 691:
        junk.tag = 702;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 703;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 2 Tails 12-03-99
// start button 3 Tails 12-03-99
      case 692:
        junk.tag = 704;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 705;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 3 Tails 12-03-99
// start button 4 Tails 12-03-99
      case 693:
        junk.tag = 706;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 707;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 4 Tails 12-03-99
// start button 5 Tails 12-03-99
      case 694:
        junk.tag = 708;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 709;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 5 Tails 12-03-99
// start button 6 Tails 12-03-99
      case 695:
        junk.tag = 710;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 711;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 6 Tails 12-03-99
// start button 7 Tails 12-03-99
      case 696:
        junk.tag = 712;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 713;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 7 Tails 12-03-99
// start button 8 Tails 12-03-99
      case 697:
        junk.tag = 714;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 715;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 8 Tails 12-03-99
// start button 9 Tails 12-03-99
      case 698:
        junk.tag = 716;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 717;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 9 Tails 12-03-99
// start button 10 Tails 12-03-99
      case 699:
        junk.tag = 718;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 719;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 10 Tails 12-03-99
// start button 11 Tails 12-03-99
      case 700:
        junk.tag = 720;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 721;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 11 Tails 12-03-99
// start button 12 Tails 12-03-99
      case 701:
        junk.tag = 722;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 723;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 12 Tails 12-03-99
// start button 13 Tails 12-03-99
      case 702:
        junk.tag = 724;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 725;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 13 Tails 12-03-99
// start button 14 Tails 12-03-99
      case 703:
        junk.tag = 726;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 727;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 14 Tails 12-03-99
// start button 15 Tails 12-03-99
      case 704:
        junk.tag = 728;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 729;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 15 Tails 12-03-99
// start button 16 Tails 12-03-99
      case 705:
        junk.tag = 730;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 731;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 16 Tails 12-03-99
// start button 17 Tails 12-03-99
      case 706:
        junk.tag = 732;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 733;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 17 Tails 12-03-99
// start button 18 Tails 12-03-99
      case 707:
        junk.tag = 734;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 735;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 18 Tails 12-03-99
// start button 19 Tails 12-03-99
      case 708:
        junk.tag = 736;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 737;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 19 Tails 12-03-99
// start button 20 Tails 12-03-99
      case 709:
        junk.tag = 738;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
        junk.tag = 739;
        EV_DoFloor( &junk, lowerFloorToLowest );
		return;
        break;
// end button 20 Tails 12-03-99
// start button 21 (special used for THZ2) Tails 04-15-01
      case 710:
        junk.tag = 740;
		EV_DoFloor(&junk, instantLower); // Tails 10-31-2000
        junk.tag = 741;
        EV_DoFloor( &junk, lowerFloorToLowest );
        junk.tag = 742;
        EV_DoDoor(&junk,dooropen, VDOORSPEED);
		junk.tag = 745;
		{
		  int                   secnum;
		  sector_t*             sec;
		  fixed_t temp;

		  secnum = -1;
		  // act on all sectors with the same tag as the triggering linedef
		  while ((secnum = P_FindSectorFromLineTag(&junk,secnum)) >= 0)
		  {
			sec = &sectors[secnum];
            
			temp = P_FindNextLowestFloor(sec,sec->floorheight);
			sec->ceilingheight = temp + sec->ceilingheight - sec->floorheight;
			sec->floorheight = temp;
		  }
		}
		return;
        break;
// end button 21 (special used for THZ2) Tails 04-15-01
// start door closer Tails 04-15-2001
      case 711:
        junk.tag = 743;
        EV_DoDoor(&junk,blazeClose, VDOORSPEED*4);
		return;
        break;
// end door closer Tails 04-15-2001

// Start water level changers
      case 713:
        junk.tag = 744;
        EV_DoFloor(&junk, raiseFloorTurbo);
		return;
        break;
      case 720:
        junk.tag = 744;
        EV_DoFloor(&junk, turboLower);
		return;
        break;
// End water level changers
// Start THZ2-specific Slime Lower button Tails 04-19-2001
	  case 986:
		// Button
        junk.tag = 712;
        EV_DoFloor(&junk,lowerFloorToLowest);

		// Water
        junk.tag = 713;
        EV_DoCeiling(&junk,lowerToLowest);

		// Platforms floating on the water *not required?*
        junk.tag = 714;
		EV_DoElevator(&junk,elevateDown, false);

		// The door
        junk.tag = 715;
        EV_DoDoor(&junk,dooropen, VDOORSPEED/12); // Open slowly now...

		// I saw the sign!
        junk.tag = 716;
		EV_DoFloor(&junk,instantLower);
		return;
	  break;
// End THZ2-specific Slime Lower button Tails 04-19-2001
	}

    P_ProcessSpecialSector(player, sector, false);
}

//
// P_UpdateSpecials
// Animate planes, scroll walls, etc.
//

void P_UpdateSpecials (void)
{
    anim_t*     anim;
    int         i;
    int         pic; //SoM: 3/8/2000

    levelflat_t*     foundflats;        // for flat animation

    //  LEVEL TIMER
    if (cv_timelimit.value && (ULONG)(cv_timelimit.value * 60 * TICRATE) < leveltime)
        G_ExitLevel();

    //  ANIMATE TEXTURES
    for (anim = anims ; anim < lastanim ; anim++)
    {
      for (i=anim->basepic ; i<anim->basepic+anim->numpics ; i++)
      {
        pic = anim->basepic + ( (leveltime/anim->speed + i)%anim->numpics );
        if (anim->istexture)
          texturetranslation[i] = pic;
      }
    }

    //  ANIMATE FLATS
    //Fab:FIXME: do not check the non-animate flat.. link the animated ones?
    // note: its faster than the original anywaysince it animates only
    //    flats used in the level, and there's usually very few of them
    foundflats = levelflats;
    for (i = 0; i<numlevelflats; i++,foundflats++)
    {
         if (foundflats->speed) // it is an animated flat
         {
             // update the levelflat lump number
             foundflats->lumpnum = foundflats->baselumpnum +
                                   ( (leveltime/foundflats->speed + foundflats->animseq) % foundflats->numpics);
         }
    }

    //  DO BUTTONS
    for (i = 0; i < MAXBUTTONS; i++)
        if (buttonlist[i].btimer)
        {
            buttonlist[i].btimer--;
            if (!buttonlist[i].btimer)
            {
                switch(buttonlist[i].where)
                {
                  case top:
                    sides[buttonlist[i].line->sidenum[0]].toptexture =
                        buttonlist[i].btexture;
                    break;

                  case middle:
                    sides[buttonlist[i].line->sidenum[0]].midtexture =
                        buttonlist[i].btexture;
                    break;

                  case bottom:
                    sides[buttonlist[i].line->sidenum[0]].bottomtexture =
                        buttonlist[i].btexture;
                    break;
                }
                S_StartSound((mobj_t *)&buttonlist[i].soundorg,sfx_swtchn);
                memset(&buttonlist[i],0,sizeof(button_t));
            }
        }

}


//SoM: 3/8/2000: EV_DoDonut moved to p_floor.c

//SoM: 3/23/2000: Adds a sectors floor and ceiling to a sector's ffloor list
ffloor_t* P_AddFakeFloor(sector_t* sec, sector_t* sec2, line_t* master, int flags);
void P_AddFFloor(sector_t* sec, ffloor_t* ffloor);

// Now returns a pointer to the floor for handy-ness. Tails 01-05-2003
ffloor_t* P_AddFakeFloor(sector_t* sec, sector_t* sec2, line_t* master, int flags)
{
  ffloor_t*      ffloor;

	if(sec2->numattached == 0) 
    { 
		sec2->attached = malloc(sizeof(int)); 
        sec2->attached[0] = sec - sectors; 
        sec2->numattached = 1; 
    } 
    else 
    { 
        int  i; 
    
        for(i = 0; i < sec2->numattached; i++) 
          if(sec2->attached[i] == sec - sectors) 
            return NULL; 
    
        sec2->attached = realloc(sec2->attached, sizeof(int) * (sec2->numattached + 1)); 
        sec2->attached[sec2->numattached] = sec - sectors; 
        sec2->numattached ++; 
     } 

  //Add the floor
  ffloor = Z_Malloc(sizeof(ffloor_t), PU_LEVEL, NULL);
  ffloor->secnum = sec2 - sectors;
  ffloor->target = sec;
  ffloor->bottomheight     = &sec2->floorheight;
  ffloor->bottompic        = &sec2->floorpic;
//  ffloor->bottomlightlevel = &sec2->lightlevel;
  ffloor->bottomxoffs      = &sec2->floor_xoffs;
  ffloor->bottomyoffs      = &sec2->floor_yoffs;

  //Add the ceiling
  ffloor->topheight     = &sec2->ceilingheight;
  ffloor->toppic        = &sec2->ceilingpic;
  ffloor->toplightlevel = &sec2->lightlevel;
  ffloor->topxoffs      = &sec2->ceiling_xoffs;
  ffloor->topyoffs      = &sec2->ceiling_yoffs;

  ffloor->flags = flags;
  ffloor->master = master;

  if(flags & FF_TRANSLUCENT)
  {
    if(sides[master->sidenum[0]].toptexture > 0)
      ffloor->alpha = sides[master->sidenum[0]].toptexture;
    else
      ffloor->alpha = 0x80;
  }

  P_AddFFloor(sec, ffloor);

  return ffloor;
}



void P_AddFFloor(sector_t* sec, ffloor_t* ffloor)
{
  ffloor_t* rover;

  if(!sec->ffloors)
  {
    sec->ffloors = ffloor;
    ffloor->next = 0;
    ffloor->prev = 0;
    return;
  }

  for(rover = sec->ffloors; rover->next; rover = rover->next);

  rover->next = ffloor;
  ffloor->prev = rover;
  ffloor->next = 0;
}

//
// SPECIAL SPAWNING
//

// Tails 09-20-2002
void P_AddSpikeThinker
( sector_t*  sec )
{
  elevator_t*           elevator;

    // create and initialize new elevator thinker
	elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
	P_AddThinker (&elevator->thinker);

    elevator->thinker.function.acp1 = (actionf_p1) T_SpikeSector;
    elevator->type = elevateBounce;

    // set up the fields according to the type of elevator action
	elevator->sector = sec;

	return;
}
void P_AddFloatThinker
( sector_t*  sec, sector_t* actionsector )
{
  elevator_t*           elevator;

    // create and initialize new elevator thinker
	elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
	P_AddThinker (&elevator->thinker);

    elevator->thinker.function.acp1 = (actionf_p1) T_FloatSector;
    elevator->type = elevateBounce;

    // set up the fields according to the type of elevator action
	elevator->sector = sec;
	elevator->actionsector = actionsector;

	return;
}

// Changes a mario block between blank and ? depending if it has any contents
// (Good if stuff respawns inside)
void P_AddBlockThinker
( sector_t*  sec, sector_t* actionsector, line_t* sourceline )
{
	elevator_t*           elevator;

    // create and initialize new elevator thinker
	elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
	P_AddThinker (&elevator->thinker);

    elevator->thinker.function.acp1 = (actionf_p1) T_MarioBlockChecker;
	elevator->sourceline = sourceline;

    // set up the fields according to the type of elevator action
	elevator->sector = sec;
	elevator->actionsector = actionsector;

	return;
}

// Even thwomps need to think!
void P_AddThwompThinker
( sector_t*  sec, sector_t* actionsector, line_t* sourceline )
{
  elevator_t*           elevator;

    // create and initialize new elevator thinker
	elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
	P_AddThinker (&elevator->thinker);

    elevator->thinker.function.acp1 = (actionf_p1) T_ThwompSector;
    elevator->type = elevateBounce;

    // set up the fields according to the type of elevator action
	elevator->sector = sec;
	elevator->actionsector = actionsector;
	elevator->floorwasheight = elevator->sector->floorheight;
	elevator->ceilingwasheight = elevator->sector->ceilingheight;
	elevator->direction = 0;
	elevator->distance = 1;
	elevator->sourceline = sourceline;

	return;
}

void P_AddCameraScanner
(sector_t* sourcesec, sector_t* actionsector, int angle)
{
	elevator_t* elevator; // Why not? LOL

	CONS_Printf("Initialising elevator (LOL!) in P_AddCameraScanner\n");
	CONS_Printf("Sourcesec = %d, Actionsector = %d\n", sourcesec - sectors, actionsector - sectors);
	// create and initialize new elevator thinker
	elevator = Z_Malloc (sizeof(*elevator), PU_LEVSPEC, 0);
	P_AddThinker (&elevator->thinker);

	elevator->thinker.function.acp1 = (actionf_p1) T_CameraScanner;
	elevator->type = elevateBounce;

	// set up the fields according to the type of elevator action
	elevator->sector = sourcesec;
	elevator->actionsector = actionsector;
	elevator->distance = angle;
	return;
}

//
// T_LaserFlash
// Do flashing lights.
//
void T_LaserFlash (laserthink_t* flash)
{
	msecnode_t* node;
	mobj_t* thing;
	sector_t* sourcesec;
	fixed_t zplusheight;

	if(leveltime & 1)
		flash->ffloor->flags |= FF_RENDERALL;
	else
		flash->ffloor->flags &= ~FF_RENDERALL;

	sourcesec = flash->ffloor->master->frontsector; // Less to type!

	// Seek out objects to DESTROY! MUAHAHHAHAHAA!!!*cough*
	for (node = flash->sector->touching_thinglist; node; node = node->m_snext)
	{
		thing = node->m_thing;

		zplusheight = thing->z + thing->height;

		if(thing->flags & MF_SHOOTABLE)
		{
			if((thing->z < sourcesec->ceilingheight && thing->z > sourcesec->floorheight)
				|| (zplusheight < sourcesec->ceilingheight && zplusheight > sourcesec->floorheight)
				|| (thing->z < sourcesec->floorheight && zplusheight > sourcesec->ceilingheight))
				P_DamageMobj(thing, NULL, NULL, 1);
		}
	}
}

// Spawns a one-time lightning flash Tails 08-24-2002
void
EV_AddLaserThinker
(ffloor_t*     ffloor, sector_t* sector)
{
    laserthink_t*       flash;

	if(ffloor == NULL)
		return;

    flash = Z_Malloc ( sizeof(*flash), PU_LEVSPEC, 0);

    P_AddThinker (&flash->thinker);

    flash->thinker.function.acp1 = (actionf_p1) T_LaserFlash;
    flash->ffloor = ffloor;
	flash->sector = sector; // For finding mobjs
}

//
// P_SpawnSpecials
// After the map has been loaded, scan for specials
//  that spawn thinkers
//

extern consvar_t cv_storm; // Tails 09-12-2002
extern consvar_t cv_snow;
extern int R_CheckTextureNumForName(char *name);

void P_SpawnSpecials (void)
{
    sector_t*   sector;
    int         i,j;
	boolean     customGravity; // Tails 05-27-2002
	//boolean     skipline; Not used anymore Graue 12-31-2003

	customGravity = false; // Tails 05-27-2002

    //  Init special SECTORs.
    sector = sectors;
    for (i=0 ; i<numsectors ; i++, sector++)
    {
        if (!sector->special)
            continue;

		if(sector->special == 990) // Time for special stage Tails 08-26-2001
		{
			if(mapheaderinfo[gamemap-1].typeoflevel & TOL_GOLF)
			{
				par = (sector->floorheight >> FRACBITS);
			}
			else
			{
				sstimer = (sector->floorheight >> FRACBITS) * TICRATE + 6;

				totalrings = (sector->ceilingheight >> FRACBITS); // Ring count for special stage Tails 08-26-2001
			}
		}
		else if(sector->special == 991) // Custom gravity! Tails 05-27-2002
		{
			gravity = sector->floorheight/1000;
			customGravity = true;
		}
		else if(sector->special == 4) // Spikes
		{
			P_AddSpikeThinker(sector);
		}

        if (sector->special&SECRET_MASK) //SoM: 3/8/2000: count secret flags
          totalsecret++;

        switch (sector->special)//&31)
        {
          case 1:
            // FLICKERING LIGHTS
            P_SpawnLightFlash (sector);
            break;

          case 2:
            // STROBE FAST
            P_SpawnStrobeFlash(sector,FASTDARK,0);
            break;

          case 3:
            // STROBE SLOW
            P_SpawnStrobeFlash(sector,SLOWDARK,0);
            break;

          case 8:
            // GLOWING LIGHT
            P_SpawnGlowingLight(sector);
            sector->special = 8; // Tails 9-15-99
            break;

          case 12:
            // SYNC STROBE SLOW
            P_SpawnStrobeFlash (sector, SLOWDARK, 1);
            break;

          case 13:
            // SYNC STROBE FAST
            P_SpawnStrobeFlash (sector, FASTDARK, 1);
            break;

          case 17:
            P_SpawnFireFlicker(sector);
            break;
        }
    }

	// If no custom gravity was defined, set the default. Tails 05-27-2002
	if(customGravity == false)
		gravity = FRACUNIT/2;

	if(server && !netgame)
	{
		if(xmasmode || mapheaderinfo[gamemap-1].weather == 2)
		{
			cv_snow.value = 1;
			CV_SetValue(&cv_snow, true);
			cv_storm.value = 0;
			CV_SetValue(&cv_storm, false);
		}
		else if(mapheaderinfo[gamemap-1].weather == 1)
		{
			cv_storm.value = 1;
			CV_SetValue(&cv_storm, true);
			cv_snow.value = 0;
			CV_SetValue(&cv_snow, false);
		}
		else
		{
			cv_storm.value = 0;
			cv_snow.value = 0;
			CV_SetValue(&cv_snow, false);
			CV_SetValue(&cv_storm, false);
		}
	}

    //SoM: 3/8/2000: Boom level init functions
    P_RemoveAllActiveCeilings();
    P_RemoveAllActivePlats();
    for (i = 0;i < MAXBUTTONS;i++)
      memset(&buttonlist[i],0,sizeof(button_t));

    P_InitTagLists();   //Create xref tables for tags
    P_SpawnScrollers(); //Add generalized scrollers
    P_SpawnFriction();  //New friction model using linedefs
    P_SpawnPushers();   //New pusher model using linedefs

	// Look for disable linedefs Graue 12-31-2003
	for (i = 0; i < numlines; i++)
	{
		if(lines[i].special == 73)
		{
			for(j=-1; (j = P_FindLineFromLineTag(&lines[i],j)) >= 0; )
			{
				lines[j].tag = 0;
				lines[j].special = 0;
			}
			lines[i].special = 0;
			lines[i].tag = 0;
		}
	}

    //  Init line EFFECTs
    for (i = 0;i < numlines; i++)
    {

		/* // Check for a disable linedef Tails 12-16-2003
		for(j=0; j < numlines; j++)
		{
			if(lines[j].special == 73
				&& lines[i].tag == lines[j].tag
				&& !mapcleared[(P_AproxDistance(lines[j].dx, lines[j].dy)>>FRACBITS)-1]) // Disable
			{
				skipline = true;
				break;
			}
		}

		if(skipline)
			continue; // continue here, not break Graue 12-20-2003 */

		// NEW! Disable special lines in certain difficulty modes.
		// Tails 01-14-2003
		if((lines[i].flags & ML_NOEASY) && gameskill <= sk_easy)
			continue;
		if((lines[i].flags & ML_NONORMAL) && gameskill == sk_medium)
			continue;
		if((lines[i].flags & ML_NOHARD) && gameskill >= sk_hard)
			continue;

		if(!(netgame || multiplayer))
		{
			if(players[consoleplayer].charability == 0 && (lines[i].flags & ML_NOSONIC))
				continue;
			if(players[consoleplayer].charability == 1 && (lines[i].flags & ML_NOTAILS))
				continue;
			if(players[consoleplayer].charability == 2 && (lines[i].flags & ML_NOKNUX))
				continue;
		}

		lines[i].special &= 255; // Graue 11-04-2003

        switch(lines[i].special)
        {
          int s, sec;
		  int ffloorflags; // Graue 12-24-2003

		  case 232: // Activate floating platform
			EV_DoElevator(&lines[i], elevateContinuous, false); // Tails
			break;

		  case 233: // Floating platform with adjustable speed
			EV_DoElevator(&lines[i], elevateContinuous, true); // Tails
			break;

          case 242: // support for drawn heights coming from different sector
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
              sectors[s].heightsec = sec;
            break;

		  case 14: //SoM: 3/20/2000: support for drawn heights coming from different sector
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            {
              sectors[s].heightsec = sec;
              sectors[s].altheightsec = 1;
            }
            break;


		  case 16: //SoM: 4/4/2000: HACK! Copy colormaps. Just plain colormaps.
            for(s = -1; (s = P_FindSectorFromLineTag(lines + i, s)) >= 0;)
            {
              sectors[s].midmap = lines[i].frontsector->midmap;
              sectors[s].altheightsec = 2;
            }
            break;

		  case 25: // FOF (solid, opaque, shadows)
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL);
            break;

		  case 33: // FOF (solid, opaque, no shadows)
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_NOSHADE|FF_CUTLEVEL);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_NOSHADE|FF_CUTLEVEL);
            break;

		  case 34: // Float/bob platform Tails 03-08-2002
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			{
			  P_AddFloatThinker(&sectors[sec], &sectors[s]);
              P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_FLOATBOB);
			}
            break;

		  case 35: // Crumbling platform that will not return Tails 03-11-2002
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_CRUMBLE|FF_NORETURN);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			//{
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_CRUMBLE|FF_NORETURN);
			//}
            break;

		  case 36: // Crumbling platform Tails 03-11-2002
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_CRUMBLE);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_CRUMBLE);
            break;

		  case 37: // Crumbling platform that will float when it hits water Tails 03-11-2002
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			{
			  sectors[sec].teamstartsec = 1;
			  P_AddFloatThinker(&sectors[sec], &sectors[s]);
              P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_CRUMBLE|FF_FLOATBOB);
			}
            break;

		  case 38: // Air bobbing platform Tails 03-11-2002
		  case 68: // Adjustable air bobbing platform Graue 11-04-2003
		  case 72: // Adjustable air bobbing platform in reverse Graue 11-07-2003, 71->72 Graue 11-15-2003
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_AIRBOB);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_AIRBOB);
            break;

		  case 39: // Air bobbing platform that will crumble and bob on the water when it falls and hits Tails 03-11-2002
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			{
				sectors[sec].teamstartsec = 1;
				P_AddFloatThinker(&sectors[sec], &sectors[s]);
                P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_FLOATBOB|FF_AIRBOB|FF_CRUMBLE);
			}
            break;

		  case 40: // Air bobbing platform that will crumble Tails 03-11-2002
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_AIRBOB|FF_CRUMBLE);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_AIRBOB|FF_CRUMBLE);
            break;

          case 41: // Mario Block! Tails 03-11-2002
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			{
			  P_AddBlockThinker(&sectors[sec], &sectors[s], &lines[i]);
              P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_MARIO);
			}
            break;

		  case 42: // Crumbling platform that will float when it hits water, but not return Tails 03-11-2002
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			{
			  sectors[sec].teamstartsec = 1;
			  P_AddFloatThinker(&sectors[sec], &sectors[s]);
              P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_CRUMBLE|FF_FLOATBOB|FF_NORETURN);
			}
            break;

		  case 43: // Crusher! Tails 09-22-2002
			EV_DoCrush(&lines[i], crushAndRaise); // Tails
			break;

		  case 44: // TL block: FOF (solid, translucent)
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_NOSHADE|FF_TRANSLUCENT|FF_EXTRA|FF_CUTEXTRA);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_NOSHADE|FF_TRANSLUCENT|FF_EXTRA|FF_CUTEXTRA);
            break;

		  case 45: // TL water
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_RENDERALL|FF_TRANSLUCENT|FF_SWIMMABLE|FF_BOTHPLANES|FF_ALLSIDES|FF_CUTEXTRA|FF_EXTRA|/*FF_DOUBLESHADOW|*/FF_CUTSPRITES);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_RENDERALL|FF_TRANSLUCENT|FF_SWIMMABLE|FF_BOTHPLANES|FF_ALLSIDES|FF_CUTEXTRA|FF_EXTRA|/*FF_DOUBLESHADOW|*/FF_CUTSPRITES);
            break;

		  case 46: // Fog
            sec = sides[*lines[i].sidenum].sector-sectors;
            // SoM: Because it's fog, check for an extra colormap and set
            // the fog flag...
            if(sectors[sec].extra_colormap)
              sectors[sec].extra_colormap->fog = 1;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
              P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_RENDERALL|FF_FOG|FF_BOTHPLANES|FF_INVERTPLANES|FF_ALLSIDES|FF_INVERTSIDES|FF_CUTEXTRA|FF_EXTRA|FF_DOUBLESHADOW|FF_CUTSPRITES);
            break;

		  case 47: // Light effect
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_CUTSPRITES);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_CUTSPRITES);
            break;

		  case 48: // Opaque water
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_RENDERALL|FF_SWIMMABLE|FF_BOTHPLANES|FF_ALLSIDES|FF_CUTEXTRA|FF_EXTRA|/*FF_DOUBLESHADOW|*/FF_CUTSPRITES);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_RENDERALL|FF_SWIMMABLE|FF_BOTHPLANES|FF_ALLSIDES|FF_CUTEXTRA|FF_EXTRA|/*FF_DOUBLESHADOW|*/FF_CUTSPRITES);
            break;

		  case 49: // Double light effect
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_CUTSPRITES|FF_DOUBLESHADOW);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_CUTSPRITES|FF_DOUBLESHADOW);
            break;

		  case 50: // Crusher (up and then down)! Tails 09-22-2002
			  EV_DoCrush(&lines[i], fastCrushAndRaise); // Tails
			  break;

		  case 51: // 3D Floor type that doesn't draw sides Tails 12-08-2002

			// If line has no-climb set, give it shadows, otherwise don't Graue 12-24-2003
			ffloorflags = FF_EXISTS|FF_SOLID|FF_RENDERPLANES|FF_CUTLEVEL;
			if(!(lines[i].flags & ML_NOCLIMB))
				ffloorflags |= FF_NOSHADE;

			P_AddFakeFloorsByLine(i, ffloorflags);
            break;

		  case 52: // FOF (intangible, translucent)

			// If line has no-climb set, give it shadows, otherwise don't Graue 12-24-2003
			ffloorflags = FF_EXISTS|FF_RENDERALL|FF_TRANSLUCENT|FF_EXTRA|FF_CUTEXTRA;
			if(!(lines[i].flags & ML_NOCLIMB))
				ffloorflags |= FF_NOSHADE;

            P_AddFakeFloorsByLine(i, ffloorflags);
            break;

		  case 53: // Laser block
            sec = sides[*lines[i].sidenum].sector-sectors;
           
			for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			  EV_AddLaserThinker(P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_RENDERALL|FF_NOSHADE|FF_EXTRA|FF_CUTEXTRA),&sectors[s]);
            break;

		  case 54: // A THWOMP! Tails 05-25-2003
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			{
			  P_AddThwompThinker(&sectors[sec], &sectors[s], &lines[i]);
              P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL);
			}
            break;

		  case 55: // Bust-able block Tails 06-20-2003
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_BUSTUP);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_BUSTUP);
			//  // removed FF_CUTLEVEL Graue 10-27-2003
            break;

		  case 56: // Quicksand! Tails 06-25-2003
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_QUICKSAND|FF_RENDERALL|FF_ALLSIDES|FF_CUTSPRITES);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_QUICKSAND|FF_RENDERALL|FF_ALLSIDES|FF_CUTSPRITES);
            break;

		  case 57: // FOF (solid, invisible)
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_NOSHADE);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_NOSHADE);
            break;

		  case 58: // FOF (intangible, invisible) - for combining specials in a sector
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_NOSHADE);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_NOSHADE);
            break;

		  case 59: // 'Platform' - You can jump up through it

			// If line has no-climb set, don't give it shadows, otherwise do Graue 12-24-2003
			ffloorflags = FF_EXISTS|FF_SOLID|FF_RENDERALL|FF_CUTLEVEL|FF_PLATFORM|FF_CUTSPRITES;
			if(lines[i].flags & ML_NOCLIMB)
				ffloorflags |= FF_NOSHADE;

            P_AddFakeFloorsByLine(i, ffloorflags);
            break;

		  case 60: // Adjustable pulsating light
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			  P_SpawnAdjustableGlowingLight(&sectors[sec], &sectors[s], P_AproxDistance(lines[i].v1->x - lines[i].v2->x, lines[i].v1->y - lines[i].v2->y) >> FRACBITS);
			  break;

		  case 61: // Adjustable flickering light
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			  P_SpawnAdjustableFireFlicker(&sectors[sec], &sectors[s], P_AproxDistance(lines[i].v1->x - lines[i].v2->x, lines[i].v1->y - lines[i].v2->y) >> FRACBITS);
			  break;

		  case 62: // Like opaque water, but not swimmable. (Good for snow effect on FOFs)
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_RENDERALL|FF_BOTHPLANES|FF_ALLSIDES|FF_CUTEXTRA|FF_EXTRA|/*FF_DOUBLESHADOW|*/FF_CUTSPRITES);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_RENDERALL|FF_BOTHPLANES|FF_ALLSIDES|FF_CUTEXTRA|FF_EXTRA|/*FF_DOUBLESHADOW|*/FF_CUTSPRITES);
            break;

		  case 63: // Change camera info
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			  P_AddCameraScanner(&sectors[sec], &sectors[s], R_PointToAngle2(lines[i].v2->x, lines[i].v2->y, lines[i].v1->x, lines[i].v1->y)/ANGLE_1);
			break;

		  case 65: // Speed pad (combines with sector special 975 or 976)
			break;

		  //case 66:

		  case 67: // FOF with no floor/ceiling (good for GFZGRASS effect on FOFs) Graue 10-26-2003
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_RENDERSIDES|FF_NOSHADE|FF_ALLSIDES);
            //sec = sides[*lines[i].sidenum].sector-sectors;
            //for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
            //  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_RENDERSIDES|FF_NOSHADE|FF_ALLSIDES);
            break;

		  case 69: // Solid FOF with no floor/ceiling (quite possibly useless) Graue 11-04-2003
			P_AddFakeFloorsByLine(i, FF_EXISTS|FF_SOLID|FF_RENDERSIDES|FF_NOSHADE|FF_CUTLEVEL);
			//sec = sides[*lines[i].sidenum].sector-sectors;
			//for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
			//  P_AddFakeFloor(&sectors[s], &sectors[sec], lines+i, FF_EXISTS|FF_SOLID|FF_RENDERSIDES|FF_NOSHADE|FF_CUTLEVEL);
			break;

          case 213: // floor lighting independently (e.g. lava)
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
              sectors[s].floorlightsec = sec;
            break;

		  case 5: // ceiling lighting independently
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
              sectors[s].ceilinglightsec = sec;
            break;

		  case 26: // Ring Sector Tails 10-31-2000
			EV_DoFloor(&lines[i], instantLower); // Tails 10-31-2000
			break; // Tails 10-31-2000

		  case 24: // Instant raise for ceilings Tails 06-10-2002
			EV_DoCeiling(&lines[i], instantRaise); // Tails 06-10-2002
			break;

		  case 64: // Definable gravity per sector Tails 06-14-2002
            sec = sides[*lines[i].sidenum].sector-sectors;
            for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
              sectors[s].gravity = &sectors[sec].floorheight; // This allows it to change in realtime!
			break;

		  case 70: // Ideya time Tails 06-03-2003
			  {
				thinker_t*  th;
				mobj_t*     mo2;

				// scan the remaining thinkers
				// to find all emeralds
				for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
				{
					if (th->function.acp1 != (actionf_p1)P_MobjThinker)
						continue;

					mo2 = (mobj_t *)th;

					if(mo2->type == MT_NIGHTSDRONE)
						mo2->health = P_AproxDistance(lines[i].dx,lines[i].dy)>>FRACBITS;
				}
			  }
			break;

		  // Disable tags if level not cleared Tails 12-16-2003
		  case 73: // Disable
			  break;

		  case 71: // Custom exit Graue 12-31-2003
			  break;

		  case 96: // Linedef executor (combines with sector special 975) and commands
		  case 97:
		  case 98: // Updated this list Graue 12-24-2003
		  case 101:
		  case 102:
		  case 103:
		  case 104:
		  case 105:
		  case 106:
		  case 107:
		  case 108:
		  case 109:
		  case 110:
		  case 111:
		  case 112:
			break;

          default:
            //if(lines[i].special>=1000 && lines[i].special<1032)
            //{
//                for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
//                {
//                  sectors[s].teamstartsec = lines[i].special-999; // only 999 so we know when it is set (it's != 0)
//                }
                break;
            //}
        }
    }
}

// P_AddFakeFloorsByLine
// 
// New utility function to simplify a lot of repeated code in P_SpawnSpecials
//
void P_AddFakeFloorsByLine(int line, int ffloorflags)
{
	int s, sec;

	sec = sides[*lines[line].sidenum].sector-sectors;
	for (s = -1; (s = P_FindSectorFromLineTag(lines+line,s)) >= 0;)
		P_AddFakeFloor(&sectors[s], &sectors[sec], lines+line, ffloorflags);
}

/*
  SoM: 3/8/2000: General scrolling functions.
  T_Scroll,
  Add_Scroller,
  Add_WallScroller,
  P_SpawnScrollers
*/
//
// This function, with the help of r_plane.c and r_bsp.c, supports generalized
// scrolling floors and walls, with optional mobj-carrying properties, e.g.
// conveyor belts, rivers, etc. A linedef with a special type affects all
// tagged sectors the same way, by creating scrolling and/or object-carrying
// properties. Multiple linedefs may be used on the same sector and are
// cumulative, although the special case of scrolling a floor and carrying
// things on it, requires only one linedef. The linedef's direction determines
// the scrolling direction, and the linedef's length determines the scrolling
// speed. This was designed so that an edge around the sector could be used to
// control the direction of the sector's scrolling, which is usually what is
// desired.
//
// Process the active scrollers.

void T_Scroll(scroll_t *s)
{
  fixed_t  dx = s->dx, dy = s->dy;

  if (s->control != -1)
    {   // compute scroll amounts based on a sector's height changes
      fixed_t height = sectors[s->control].floorheight +
        sectors[s->control].ceilingheight;
      fixed_t delta = height - s->last_height;
      s->last_height = height;
      dx = FixedMul(dx, delta);
      dy = FixedMul(dy, delta);
    }

  if (s->accel)
    {
      s->vdx = dx += s->vdx;
      s->vdy = dy += s->vdy;
    }

  if (!(dx | dy))                   // no-op if both (x,y) offsets 0
    return;

  switch (s->type)
    {
      side_t *side;
      sector_t *sec;
      fixed_t height, waterheight;
      msecnode_t *node;
      mobj_t *thing;
	  line_t *line;
	  int i, sect; // Graue 11-11-2003

    case sc_side:                   //Scroll wall texture
        side = sides + s->affectee;
        side->textureoffset += dx;
        side->rowoffset += dy;
        break;

    case sc_floor:                  //Scroll floor texture
        sec = sectors + s->affectee;
        sec->floor_xoffs += dx;
        sec->floor_yoffs += dy;
        break;

    case sc_ceiling:               //Scroll ceiling texture
        sec = sectors + s->affectee;
        sec->ceiling_xoffs += dx;
        sec->ceiling_yoffs += dy;
        break;

    case sc_carry:

      sec = sectors + s->affectee;
      height = sec->floorheight;
      waterheight = sec->heightsec != -1 &&
        sectors[sec->heightsec].floorheight > height ?
        sectors[sec->heightsec].floorheight : MININT;

      for (node = sec->touching_thinglist; node; node = node->m_snext)
        if (!((thing = node->m_thing)->flags & MF_NOCLIP) &&
            (!(thing->flags & MF_NOGRAVITY || thing->z > height) ||
             thing->z < waterheight))
          {
            // Move objects only if on floor or underwater,
            // non-floating, and clipped.
            thing->momx += dx;
            thing->momy += dy;
			if(thing->player) // Tails 04-13-2001
			{ // Tails 04-13-2001
				thing->player->cmomx += dx;
				thing->player->cmomy += dy;
				thing->player->cmomx = FixedMul (thing->player->cmomx, 0xe800); // Tails 04-13-2001
				thing->player->cmomy = FixedMul (thing->player->cmomy, 0xe800); // Tails 04-13-2001
			} // Tails 04-13-2001
          }
      break;

    case sc_carry_ceiling:       // to be added later, and later has come! Graue 11-11-2003

      sec = sectors + s->affectee;
	  height = sec->ceilingheight;
	  waterheight = sec->heightsec != -1 &&
	  sectors[sec->heightsec].floorheight > height ?
	  sectors[sec->heightsec].floorheight : MININT;

	  // sec is the control sector, find the real sector(s) to use
	  for(i = 0; i < sec->linecount; i++)
	  {
		boolean is3dblock;

		line = sec->lines[i];
		switch(line->special)
		{
			case 25: // 3D floor: solid, opaque, shadowcasting
			case 33: // 3D floor: solid, opaque, no shadows
			case 44: // 3D floor: solid, translucent
			case 51: // 3D floor: solid, no sides
			case 57: // 3D floor: solid, invisible
			case 69: // 3D floor: solid, sides only
				is3dblock = true;
				//CONS_Printf("It's a 3D block, all right!\n");
				break;
			default: // Ignore specials that are not solid 3D blocks
				is3dblock = false;
				//CONS_Printf("not a 3D block :(\n");
				break;
		}

		if(!is3dblock) continue;

		for (sect = -1; (sect = P_FindSectorFromTag(line->tag,sect)) >= 0;)
		{
			sector_t *sec;
			sec = sectors + sect;

			//CONS_Printf("hello?\n");

			for (node = sec->touching_thinglist; node; node = node->m_snext) {

				thing = node->m_thing;
				//CONS_Printf("Hooray!\n");
				//if(!thing) { CONS_Printf("SHIT\n"); return;}
				//CONS_Printf("Hooray #2\n");
				//CONS_Printf("Hooray #%i\n",thing->flags);
				//if(thing->flags & MF_NOCLIP) CONS_Printf("Thing is not clipped!\n");
				//else CONS_Printf("Thing is clipped!\n");
				//if(thing->flags & MF_NOGRAVITY) CONS_Printf("Thing is a floaty!\n");
				//else CONS_Printf("Thing has gravity and doesn't float!\n");
				//CONS_Printf("thing->z is: %i, ",thing->z);
				//CONS_Printf("height is: %i, ",height);
				//CONS_Printf("waterheight is: %i\n",waterheight);

				if (!(thing->flags & MF_NOCLIP)) // Thing must be clipped
				if (!(thing->flags & MF_NOGRAVITY || thing->z != height) ||
					thing->z < waterheight) // Thing must a) be non-floating and have z == height
											// or b) be underwater
				{
					// Move objects only if on floor or underwater,
					// non-floating, and clipped.
					thing->momx += dx;
					thing->momy += dy;
					if(thing->player) // Tails 04-13-2001
					{ // Tails 04-13-2001
						thing->player->cmomx += dx;
						thing->player->cmomy += dy;
						thing->player->cmomx = FixedMul (thing->player->cmomx, 0xe800); // Tails 04-13-2001
						thing->player->cmomy = FixedMul (thing->player->cmomy, 0xe800); // Tails 04-13-2001
					} // Tails 04-13-2001
				};
			} // end of for loop through touching_thinglist
		} // end of loop through sectors
	  }
      break; // end of sc_carry_ceiling
	} // end of switch
}

//
// Add_Scroller()
//
// Add a generalized scroller to the thinker list.
//
// type: the enumerated type of scrolling: floor, ceiling, floor carrier,
//   wall, floor carrier & scroller
//
// (dx,dy): the direction and speed of the scrolling or its acceleration
//
// control: the sector whose heights control this scroller's effect
//   remotely, or -1 if no control sector
//
// affectee: the index of the affected object (sector or sidedef)
//
// accel: non-zero if this is an accelerative effect
//

static void Add_Scroller(int type, fixed_t dx, fixed_t dy,
                         int control, int affectee, int accel)
{
  scroll_t *s = Z_Malloc(sizeof *s, PU_LEVSPEC, 0);
  s->thinker.function.acp1 = (actionf_p1) T_Scroll;
  s->type = type;
  s->dx = dx;
  s->dy = dy;
  s->accel = accel;
  s->vdx = s->vdy = 0;
  if ((s->control = control) != -1)
    s->last_height =
      sectors[control].floorheight + sectors[control].ceilingheight;
  s->affectee = affectee;
  P_AddThinker(&s->thinker);
}

// Adds wall scroller. Scroll amount is rotated with respect to wall's
// linedef first, so that scrolling towards the wall in a perpendicular
// direction is translated into vertical motion, while scrolling along
// the wall in a parallel direction is translated into horizontal motion.

static void Add_WallScroller(fixed_t dx, fixed_t dy, const line_t *l,
                             int control, int accel)
{
  fixed_t x = abs(l->dx), y = abs(l->dy), d;
  if (y > x)
    d = x, x = y, y = d;
  d = FixedDiv(x, finesine[(tantoangle[FixedDiv(y,x) >> DBITS] + ANG90)
                          >> ANGLETOFINESHIFT]);
  x = -FixedDiv(FixedMul(dy, l->dy) + FixedMul(dx, l->dx), d);
  y = -FixedDiv(FixedMul(dx, l->dy) - FixedMul(dy, l->dx), d);
  Add_Scroller(sc_side, x, y, control, *l->sidenum, accel);
}

// Amount (dx,dy) vector linedef is shifted right to get scroll amount
#define SCROLL_SHIFT 5

// Factor to scale scrolling effect into mobj-carrying properties = 3/32.
// (This is so scrolling floors and objects on them can move at same speed.)
#define CARRYFACTOR ((fixed_t)(FRACUNIT*.09375))

// Initialize the scrollers
static void P_SpawnScrollers(void)
{
  int i;
  line_t *l = lines;

  for (i=0;i<numlines;i++,l++)
    {
      fixed_t dx = l->dx >> SCROLL_SHIFT;  // direction and speed of scrolling
      fixed_t dy = l->dy >> SCROLL_SHIFT;
      int control = -1, accel = 0;         // no control sector or acceleration
      int special = l->special;

      // Types 245-249 are same as 250-254 except that the
      // first side's sector's heights cause scrolling when they change, and
      // this linedef controls the direction and speed of the scrolling. The
      // most complicated linedef since donuts, but powerful :)

      if (special >= 245 && special <= 249)         // displacement scrollers
        {
          special += 250-245;
          control = sides[*l->sidenum].sector - sectors;
        }
      else
        if (special >= 214 && special <= 218)       // accelerative scrollers
          {
            accel = 1;
            special += 250-214;
            control = sides[*l->sidenum].sector - sectors;
          }
	  else if(special == 200 || special == 201) // displacement scrollers Graue 11-09-2003
	  {
		  special += 2;
		  control = sides[*l->sidenum].sector - sectors;
	  }
	  else if(special == 204 || special == 205) // accelerative scrollers Graue 11-09-2003
	  {
		  accel = 1;
		  special -= 2;
		  control = sides[*l->sidenum].sector - sectors;
	  }

      switch (special)
        {
          register int s;

        case 250:   // scroll effect ceiling
		case 202:   // scroll and carry objects on ceiling Graue 11-09-2003
          for (s=-1; (s = P_FindSectorFromLineTag(l,s)) >= 0;)
            Add_Scroller(sc_ceiling, -dx, dy, control, s, accel);
		  if (special != 202)
			break;

		case 203:	// carry objects on ceiling
		  dx = FixedMul(dx,CARRYFACTOR);
		  dy = FixedMul(dy,CARRYFACTOR);
		  for (s=-1; (s = P_FindSectorFromLineTag(l,s)) >= 0;)
			Add_Scroller(sc_carry_ceiling, dx, dy, control, s, accel);
		  break;

        case 251:   // scroll effect floor
        case 253:   // scroll and carry objects on floor
          for (s=-1; (s = P_FindSectorFromLineTag(l,s)) >= 0;)
            Add_Scroller(sc_floor, -dx, dy, control, s, accel);
          if (special != 253)
            break;

        case 252:	// carry objects on floor
          dx = FixedMul(dx,CARRYFACTOR);
          dy = FixedMul(dy,CARRYFACTOR);
          for (s=-1; (s = P_FindSectorFromLineTag(l,s)) >= 0;)
            Add_Scroller(sc_carry, dx, dy, control, s, accel);
          break;

          // scroll wall according to linedef
          // (same direction and speed as scrolling floors)
        case 254:
          for (s=-1; (s = P_FindLineFromLineTag(l,s)) >= 0;)
            if (s != i)
              Add_WallScroller(dx, dy, lines+s, control, accel);
          break;

        case 255:
          s = lines[i].sidenum[0];
          Add_Scroller(sc_side, -sides[s].textureoffset,
                       sides[s].rowoffset, -1, s, accel);
          break;

        case 100: // Tails                  // scroll first side
          Add_Scroller(sc_side,  FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
          break;

        case 99: // heretic right scrolling
              break; // doom use it as bluekeydoor
        case 85:                  // jff 1/30/98 2-way scroll
          Add_Scroller(sc_side, -FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
          break;
        }
    }
}



/*
  SoM: 3/8/2000: Friction functions start.
  Add_Friction,
  T_Friction,
  P_SpawnFriction
*/

// Adds friction thinker.
static void Add_Friction(int friction, int movefactor, int affectee)
{
    friction_t *f = Z_Malloc(sizeof *f, PU_LEVSPEC, 0);

    f->thinker.function.acp1 = (actionf_p1) T_Friction;
    f->friction = friction;
    f->movefactor = movefactor;
    f->affectee = affectee;
    P_AddThinker(&f->thinker);
}



//Function to apply friction to all the things in a sector.
void T_Friction(friction_t *f)
{
    sector_t *sec;
    mobj_t   *thing;
    msecnode_t* node;
	boolean foundfloor = false;

    if (!boomsupport || !variable_friction)
        return;

    sec = sectors + f->affectee;

    // Be sure the special sector type is still turned on. If so, proceed.
    // Else, bail out; the sector type has been changed on us.

    if (!(sec->special & FRICTION_MASK))
	{
		if(sec->ffloors)
		{
		  ffloor_t*  rover;

		  for(rover = sec->ffloors; rover; rover = rover->next)
		  {
			// Do some small extra checks here to possibly save unneeded work.
			if(!(rover->master->frontsector->special & FRICTION_MASK))
			  continue;
			foundfloor = true;
		  }
		}

		if(foundfloor == false) // Not even a 3d floor has the FRICTION_MASK.
			return;
	}

    // Assign the friction value to players on the floor, non-floating,
    // and clipped. Normally the object's friction value is kept at
    // ORIG_FRICTION and this thinker changes it for icy or muddy floors.

    // In Phase II, you can apply friction to Things other than players.

    // When the object is straddling sectors with the same
    // floorheight that have different frictions, use the lowest
    // friction value (muddy has precedence over icy).

    node = sec->touching_thinglist; // things touching this sector
    while (node)
        {
        thing = node->m_thing;
		// Graue 12-31-2003: apparently, all I had to do was comment out part of the next line and
		//                   friction works for all mobj's
		// (or at least MF_PUSHABLEs, which is all I care about anyway)
        if (// thing->player &&
            !(thing->flags & (MF_NOGRAVITY | MF_NOCLIP)) &&
            thing->z == thing->floorz)
            {
			if(foundfloor && thing->z == sec->floorheight); // Skip

            else if ((thing->friction == ORIG_FRICTION) ||     // normal friction?
                (f->friction < thing->friction))
                {
                thing->friction   = f->friction;
                thing->movefactor = f->movefactor;
                }
            }
        node = node->m_snext;
        }
}

//Spawn all friction.
static void P_SpawnFriction(void)
    {
    int i;
    line_t *l = lines;
    register int s;
    int length;     // line length controls magnitude
    int friction;   // friction value to be applied during movement
    int movefactor; // applied to each player move to simulate inertia

    for (i = 0 ; i < numlines ; i++,l++)
        if (l->special == 223)
            {
            length = P_AproxDistance(l->dx,l->dy)>>FRACBITS;
            friction = (0x1EB8*length)/0x80 + 0xD000;

            if(friction > FRACUNIT)
              friction = FRACUNIT;
            if(friction < 0)
              friction = 0;

            // The following check might seem odd. At the time of movement,
            // the move distance is multiplied by 'friction/0x10000', so a
            // higher friction value actually means 'less friction'.

            if (friction > ORIG_FRICTION)       // ice
                movefactor = ((0x10092 - friction)*(0x70))/0x158;
            else
                movefactor = ((friction - 0xDB34)*(0xA))/0x80;

            // killough 8/28/98: prevent odd situations
            if (movefactor < 32)
              movefactor = 32;

            for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                Add_Friction(friction,movefactor,s);
            }
    }


/*
  SoM: 3/8/2000: Push/Pull/Wind/Current functions.
  Add_Pusher,
  PIT_PushThing,
  T_Pusher,
  P_GetPushThing,
  P_SpawnPushers
*/


#define PUSH_FACTOR 7

// Adds a pusher
static void Add_Pusher(int type, int x_mag, int y_mag, mobj_t* source, int affectee)
    {
    pusher_t *p = Z_Malloc(sizeof *p, PU_LEVSPEC, 0);

    p->thinker.function.acp1 = (actionf_p1) T_Pusher;
    p->source = source;
    p->type = type;
    p->x_mag = x_mag>>FRACBITS;
    p->y_mag = y_mag>>FRACBITS;
	// "The right triangle of the square of the length of the hypotenuse is equal to the sum of the squares of the lengths of the other two sides."
	// "Bah! Stupid brains! Don't you know anything besides the Pythagorean Theorem?" - Earthworm Jim
	// Tails 06-14-2002
	if(type == p_downcurrent || type == p_upcurrent || type == p_upwind || type == p_downwind)
		p->magnitude = P_AproxDistance(p->x_mag,p->y_mag)<<(FRACBITS-PUSH_FACTOR);
	else
		p->magnitude = P_AproxDistance(p->x_mag,p->y_mag);
    if (source) // point source exist?
        {
        p->radius = (p->magnitude)<<(FRACBITS+1); // where force goes to zero
        p->x = p->source->x;
        p->y = p->source->y;
		p->z = p->source->z;
        }
    p->affectee = affectee;
    P_AddThinker(&p->thinker);
    }


// PIT_PushThing determines the angle and magnitude of the effect.
// The object's x and y momentum values are changed.

pusher_t* tmpusher; // pusher structure for blockmap searches

boolean PIT_PushThing(mobj_t* thing)
{
	if (thing->player &&
		!(thing->flags & (MF_NOGRAVITY | MF_NOCLIP)))
	{
		//angle_t pushangle;
		int dist;
		int speed;
		int sx,sy,sz; // Add Z Graue 12-12-2003

		sx = tmpusher->x;
		sy = tmpusher->y;
		sz = tmpusher->z;

		// Make sure the Z is in range Graue 12-12-2003
		if(thing->z < sz - tmpusher->radius || thing->z > sz + tmpusher->radius)
			return false;

		// dist = P_AproxDistance(thing->x - sx,thing->y - sy);
		dist = P_AproxDistance(P_AproxDistance(thing->x - sx, thing->y - sy),
		                       thing->z - sz); // Graue 12-12-2003
		speed = (tmpusher->magnitude -
				((dist>>FRACBITS)>>1))<<(FRACBITS-PUSH_FACTOR-1);

		// If speed <= 0, you're outside the effective radius. You also have
		// to be able to see the push/pull source point.

		// Rewrite with bits and pieces of P_HomingAttack Graue 12-12-2003
		if ((speed > 0) && (P_CheckSight(thing,tmpusher->source)))
		{
			fixed_t tmpmomx, tmpmomy, tmpmomz;
			tmpmomx = FixedMul(FixedDiv(sx - thing->x, dist), speed);
			tmpmomy = FixedMul(FixedDiv(sy - thing->y, dist), speed);
			tmpmomz = FixedMul(FixedDiv(sz - thing->z, dist), speed);
			if(tmpusher->source->type == MT_PUSH) // away!
			{
				tmpmomx *= -1;
				tmpmomy *= -1;
				tmpmomz *= -1;
			}

			thing->momx += tmpmomx;
			thing->momy += tmpmomy;
			thing->momz += tmpmomz;

			if(thing->player)
			{
				thing->player->cmomx += tmpmomx;
				thing->player->cmomy += tmpmomy;
				// SHIT! Problem encountered: no cmomz!
				thing->player->cmomx = FixedMul(thing->player->cmomx, 0xe800);
				thing->player->cmomy = FixedMul(thing->player->cmomy, 0xe800);
			}

			/* pushangle = R_PointToAngle2(thing->x,thing->y,sx,sy);
			if (tmpusher->source->type == MT_PUSH)
				pushangle += ANG180;    // away
			pushangle >>= ANGLETOFINESHIFT;
			thing->momx += FixedMul(speed,finecosine[pushangle]);
			thing->momy += FixedMul(speed,finesine[pushangle]);

			if(thing->player) // Tails 04-13-2001
			{ // Tails 04-13-2001
				thing->player->cmomx += FixedMul(speed,finecosine[pushangle]);
				thing->player->cmomy += FixedMul(speed,finesine[pushangle]);
				thing->player->cmomx = FixedMul (thing->player->cmomx, 0xe800); // Tails 04-13-2001
				thing->player->cmomy = FixedMul (thing->player->cmomy, 0xe800); // Tails 04-13-2001
			} // Tails 04-13-2001 */

		}
	}
	return true;
}



// T_Pusher looks for all objects that are inside the radius of
// the effect.

void T_Pusher(pusher_t *p)
    {
    sector_t *sec;
    mobj_t   *thing;
    msecnode_t* node;
    int xspeed = 0,yspeed = 0;
    int xl,xh,yl,yh,bx,by;
    int radius;
    int ht = 0;
	boolean inwater;
	boolean touching;
	fixed_t z;

	inwater = touching = false;

	xspeed = yspeed = 0;

    if (!allow_pushers)
        return;

    sec = sectors + p->affectee;

    // Be sure the special sector type is still turned on. If so, proceed.
    // Else, bail out; the sector type has been changed on us.

    if (!(sec->special & PUSH_MASK))
	{
		boolean foundfloor;
		foundfloor = false;

		if(sec->ffloors)
		{
		  ffloor_t*  rover;

		  for(rover = sec->ffloors; rover; rover = rover->next)
		  {
			// Do some small extra checks here to possibly save unneeded work.
			if(!(rover->master->frontsector->special & PUSH_MASK) || rover->master->frontsector->special == 983)
			  continue;
			foundfloor = true;
		  }
		}

		if(foundfloor == false) // Not even a 3d floor has the PUSH_MASK.
			return;
	}

    // For constant pushers (wind/current) there are 3 situations:
    //
    // 1) Affected Thing is above the floor.
    //
    //    Apply the full force if wind, no force if current.
    //
    // 2) Affected Thing is on the ground.
    //
    //    Apply half force if wind, full force if current.
    //
    // 3) Affected Thing is below the ground (underwater effect).
    //
    //    Apply no force if wind, full force if current.
    //
    // Apply the effect to clipped players only for now.
    //
    // In Phase II, you can apply these effects to Things other than players.

    if (p->type == p_push)
        {

        // Seek out all pushable things within the force radius of this
        // point pusher. Crosses sectors, so use blockmap.

        tmpusher = p; // MT_PUSH/MT_PULL point source
        radius = p->radius; // where force goes to zero
        tmbbox[BOXTOP]    = p->y + radius;
        tmbbox[BOXBOTTOM] = p->y - radius;
        tmbbox[BOXRIGHT]  = p->x + radius;
        tmbbox[BOXLEFT]   = p->x - radius;

        xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
        xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
        yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
        yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;
        for (bx=xl ; bx<=xh ; bx++)
            for (by=yl ; by<=yh ; by++)
                P_BlockThingsIterator(bx,by,PIT_PushThing);
        return;
        }

    // constant pushers p_wind and p_current

    node = sec->touching_thinglist; // things touching this sector
    for ( ; node ; node = node->m_snext)
        {
        thing = node->m_thing;
        if (!thing->player || (thing->flags & (MF_NOGRAVITY | MF_NOCLIP)))
            continue;

		// Find the area that the 'thing' is in
		// Kudos to P_MobjCheckWater().
		// Tails 09-15-2002
		if (sec->heightsec > -1 && sec->altheightsec == 1)
		{
			boolean water;
			water = false;

			if (sec->heightsec > -1)  //water hack
			{
				z = (sectors[sec->heightsec].floorheight);
				water = true;
			}
			else if (sec->floortype == FLOOR_WATER || sec->floortype == FLOOR_LAVA) // Lava support
			{
				z = sec->floorheight + (FRACUNIT/4); // water texture
				water = true;
			}

			if(water == true) // Sector has water
			{
				if (thing->z<=z &&thing->z+thing->height > z)
					touching = true;

				if (thing->z+(thing->height>>1) <= z) // Tails
				{
					inwater = true;
				}
			}
		}
		
		// Not "else"! Check ALL possibilities!
		if((inwater == false || touching == false) // Only if both aren't true
			&& sec->ffloors)
		{
		  ffloor_t*  rover;

		  for(rover = sec->ffloors; rover; rover = rover->next)
		  {
			if(*rover->topheight < thing->z || *rover->bottomheight > (thing->z + (thing->height >> 1)))
			  continue;

			if(!(rover->master->frontsector->special & PUSH_MASK) || rover->master->frontsector->special == 983)
				continue;

			if(thing->z + thing->height > *rover->topheight)
				touching = true;

			if(thing->z + (thing->height >> 1) < *rover->topheight)
			{
				inwater = true;
			}
		  }
		}

		if (thing->z == sec->floorheight && (sec->floortype == FLOOR_WATER || sec->floortype == FLOOR_LAVA)) // Lava support
			touching = true;

		
		if (p->type == p_wind)
            {
                if (!touching && !inwater) // above ground
                    {
                    xspeed = p->x_mag; // full force
                    yspeed = p->y_mag;
                    }
                else if (touching) // on ground
                    {
                    xspeed = (p->x_mag)>>1; // half force
                    yspeed = (p->y_mag)>>1;
                    }
                else if (inwater) // underwater
                    xspeed = yspeed = 0; // no force
                else
					xspeed = yspeed = 0;
            }
		else if (p->type == p_upwind)
            {
                if (!touching && !inwater) // above ground
                    {
                    thing->momz += p->magnitude;
                    }
                else if (touching) // on ground
                    {
                    thing->momz += (p->magnitude)>>1;
                    }
                else if (inwater) // underwater
                    xspeed = yspeed = 0; // no force
                else
					xspeed = yspeed = 0;
            }
		else if (p->type == p_downwind)
            {
                if (!touching && !inwater) // above ground
                    {
                    thing->momz -= p->magnitude;
                    }
                else if (touching) // on ground
                    {
                    thing->momz -= (p->magnitude)>>1;
                    }
                else if (inwater) // underwater
                    xspeed = yspeed = 0; // no force
                else
					xspeed = yspeed = 0;
            }
        else // p_current
        {
			// Added Z currents Tails 06-10-2002
			if(!touching && !inwater) // Not in water at all
					xspeed = yspeed = 0; // no force
			else // underwater / touching water
			{
				if(p->type == p_upcurrent)
					thing->momz += p->magnitude;
				else if(p->type == p_downcurrent)
					thing->momz -= p->magnitude;
				else
				{
					xspeed = p->x_mag; // full force
					yspeed = p->y_mag;
				}
			}
        }

		if(p->type != p_downcurrent && p->type != p_upcurrent
			&& p->type != p_upwind && p->type != p_downwind)
		{
			thing->momx += xspeed<<(FRACBITS-PUSH_FACTOR);
			thing->momy += yspeed<<(FRACBITS-PUSH_FACTOR);
				if(thing->player) // Tails 04-13-2001
				{ // Tails 04-13-2001
					thing->player->cmomx += xspeed<<(FRACBITS-PUSH_FACTOR);
					thing->player->cmomy += yspeed<<(FRACBITS-PUSH_FACTOR);
					thing->player->cmomx = FixedMul (thing->player->cmomx, 0xe800); // Tails 04-13-2001
					thing->player->cmomy = FixedMul (thing->player->cmomy, 0xe800); // Tails 04-13-2001
				} // Tails 04-13-2001
		}
	}
}


// Get pusher object.
mobj_t* P_GetPushThing(int s)
    {
    mobj_t* thing;
    sector_t* sec;

    sec = sectors + s;
    thing = sec->thinglist;
    while (thing)
        {
        switch(thing->type)
            {
          case MT_PUSH:
          case MT_PULL:
            return thing;
          default:
            break;
            }
        thing = thing->snext;
        }
    return NULL;
    }


// Spawn pushers.
static void P_SpawnPushers(void)
    {
    int i;
    line_t *l = lines;
    register int s;
    mobj_t* thing;

    for (i = 0 ; i < numlines ; i++,l++)
        switch(l->special)
            {
          case 224: // wind
            for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                Add_Pusher(p_wind,l->dx,l->dy,NULL,s);
            break;
          case 225: // current
            for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                Add_Pusher(p_current,l->dx,l->dy,NULL,s);
            break;
		  case 227: // current up Tails 06-10-2002
            for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                Add_Pusher(p_upcurrent,l->dx,l->dy,NULL,s);
            break;
		  case 228: // current down Tails 06-10-2002
            for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                Add_Pusher(p_downcurrent,l->dx,l->dy,NULL,s);
            break;
		  case 229: // wind up Tails
			  for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                Add_Pusher(p_upwind,l->dx,l->dy,NULL,s);
			break;
		  case 230: // wind down Tails
			  for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                Add_Pusher(p_downwind,l->dx,l->dy,NULL,s);
			break;
          case 226: // push/pull
            for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                {
                thing = P_GetPushThing(s);
                if (thing) // No MT_P* means no effect
                    Add_Pusher(p_push,l->dx,l->dy,thing,s);
                }
            break;
            }
    }

// Get teleport destination object. Graue 11-06-2003
mobj_t* P_GetTeleportDestThing(int s)
{
    mobj_t* thing;
    sector_t* sec;

    sec = sectors + s;
    thing = sec->thinglist;
    while (thing)
	{
		if(thing->type == MT_TELEPORTMAN)
			return thing;
        thing = thing->snext;
    }
    return NULL;
}

// Get starpost object. Graue 12-08-2003
// FIXTHIS: make a generic "get such-and-such object" function and remove this,
// P_GetPushThing, and P_GetTeleportDestThing
mobj_t* P_GetStarpostThing(int s)
{
	mobj_t* thing;
	sector_t* sec;

	sec = sectors + s;
	thing = sec->thinglist;
	while (thing)
	{
		if(thing->type == MT_STARPOST)
			return thing;
		thing = thing->snext;
	}
	return NULL;
}