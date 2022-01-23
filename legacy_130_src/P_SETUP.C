// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_setup.c,v 1.15 2000/05/23 15:22:34 stroggonmeth Exp $
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
// $Log: p_setup.c,v $
// Revision 1.15  2000/05/23 15:22:34  stroggonmeth
// Not much. A graphic bug fixed.
//
// Revision 1.14  2000/05/03 23:51:00  stroggonmeth
// A few, quick, changes.
//
// Revision 1.13  2000/04/19 15:21:02  hurdler
// add SDL midi support
//
// Revision 1.12  2000/04/18 12:55:39  hurdler
// join with Boris' code
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
// Revision 1.8  2000/04/12 16:01:59  hurdler
// ready for T&L code and true static lighting
//
// Revision 1.7  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.6  2000/04/08 11:27:29  hurdler
// fix some boom stuffs
//
// Revision 1.5  2000/04/06 20:40:22  hurdler
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
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Do all the WAD I/O, get map description,
//             set up initial state and misc. LUTs.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "d_main.h"

#include "g_game.h"

#include "p_local.h"
#include "p_setup.h"
#include "p_spec.h"

#include "i_sound.h" //for I_PlayCD()..
#include "r_sky.h"

#include "r_data.h"
#include "r_things.h"
#include "r_sky.h"

#include "s_sound.h"

#include "w_wad.h"
#include "z_zone.h"
#include "r_splats.h"

#ifdef __WIN32__
#include "malloc.h"
#include "math.h"
#endif
#ifdef HWRENDER
#include "i_video.h"            //rendermode
#include "hardware/hw_main.h"
#endif

extern consvar_t cv_chasecam; // declare the cam var! Tails 01-06-2000

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
int             numvertexes;
vertex_t*       vertexes;

int             numsegs;
seg_t*          segs;

int             numsectors;
sector_t*       sectors;

int             numsubsectors;
subsector_t*    subsectors;

int             numnodes;
node_t*         nodes;

int             numlines;
line_t*         lines;

int             numsides;
side_t*         sides;

int             nummapthings;
mapthing_t*     mapthings;

/*
typedef struct mapdata_s {
    int             numvertexes;
    vertex_t*       vertexes;
    int             numsegs;
    seg_t*          segs;
    int             numsectors;
    sector_t*       sectors;
    int             numsubsectors;
    subsector_t*    subsectors;
    int             numnodes;
    node_t*         nodes;
    int             numlines;
    line_t*         lines;
    int             numsides;
    side_t*         sides;
} mapdata_t;
*/


// BLOCKMAP
// Created from axis aligned bounding box
// of the map, a rectangular array of
// blocks of size ...
// Used to speed up collision detection
// by spatial subdivision in 2D.
//
// Blockmap size.
int             bmapwidth;
int             bmapheight;     // size in mapblocks

short*          blockmap;       // int for large maps
// offsets in blockmap are from here
short*          blockmaplump;

// origin of block map
fixed_t         bmaporgx;
fixed_t         bmaporgy;
// for thing chains
mobj_t**        blocklinks;


// REJECT
// For fast sight rejection.
// Speeds up enemy AI by skipping detailed
//  LineOf Sight calculation.
// Without special effect, this could be
//  used as a PVS lookup as well.
//
byte*           rejectmatrix;


// Maintain single and multi player starting spots.
mapthing_t      deathmatchstarts[MAX_DM_STARTS];
mapthing_t*     deathmatch_p;
mapthing_t      playerstarts[MAXPLAYERS];
mapthing_t      bluectfstarts[MAXPLAYERS]; // CTF Tails 08-04-2001
mapthing_t      redctfstarts[MAXPLAYERS]; // CTF Tails 08-04-2001
mapthing_t*     redctfstarts_p; // CTF Tails 08-04-2001
mapthing_t*     bluectfstarts_p; // CTF Tails 08-04-2001

//
// P_LoadVertexes
//
void P_LoadVertexes (int lump)
{
    byte*               data;
    int                 i;
    mapvertex_t*        ml;
    vertex_t*           li;

    // Determine number of lumps:
    //  total lump length / vertex record length.
    numvertexes = W_LumpLength (lump) / sizeof(mapvertex_t);

    // Allocate zone memory for buffer.
    vertexes = Z_Malloc (numvertexes*sizeof(vertex_t),PU_LEVEL,0);

    // Load data into cache.
    data = W_CacheLumpNum (lump,PU_STATIC);

    ml = (mapvertex_t *)data;
    li = vertexes;

    // Copy and convert vertex coordinates,
    // internal representation as fixed.
    for (i=0 ; i<numvertexes ; i++, li++, ml++)
    {
        li->x = SHORT(ml->x)<<FRACBITS;
        li->y = SHORT(ml->y)<<FRACBITS;
    }

    // Free buffer memory.
    Z_Free (data);
}


//
// Computes the line length in frac units, the glide render needs this
//
#define crapmul (1.0f / 65536.0f)

float P_SegLength (seg_t* seg)
{
    double      dx,dy;

    // make a vector (start at origin)
    dx = (seg->v2->x - seg->v1->x)*crapmul;
    dy = (seg->v2->y - seg->v1->y)*crapmul;

    return sqrt(dx*dx+dy*dy)*FRACUNIT;
}


//
// P_LoadSegs
//
void P_LoadSegs (int lump)
{
    byte*               data;
    int                 i;
    mapseg_t*           ml;
    seg_t*              li;
    line_t*             ldef;
    int                 linedef;
    int                 side;

    numsegs = W_LumpLength (lump) / sizeof(mapseg_t);
    segs = Z_Malloc (numsegs*sizeof(seg_t),PU_LEVEL,0);
    memset (segs, 0, numsegs*sizeof(seg_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    ml = (mapseg_t *)data;
    li = segs;
    for (i=0 ; i<numsegs ; i++, li++, ml++)
    {
        li->v1 = &vertexes[SHORT(ml->v1)];
        li->v2 = &vertexes[SHORT(ml->v2)];

#ifdef HWRENDER // not win32 only 19990829 by Kin
        // used for the hardware render
        if (rendermode != render_soft)
        {
            li->length = P_SegLength (li);
#ifdef TANDL
            //Hurdler: 04/12/2000: for now, only used in hardware mode
            li->lightmaps = NULL; // list of static lightmap for this seg
#endif
        }
#endif

        li->angle = (SHORT(ml->angle))<<16;
        li->offset = (SHORT(ml->offset))<<16;
        linedef = SHORT(ml->linedef);
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = SHORT(ml->side);
        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;
        if (ldef-> flags & ML_TWOSIDED)
            li->backsector = sides[ldef->sidenum[side^1]].sector;
        else
            li->backsector = 0;
    }

    Z_Free (data);
}


//
// P_LoadSubsectors
//
void P_LoadSubsectors (int lump)
{
    byte*               data;
    int                 i;
    mapsubsector_t*     ms;
    subsector_t*        ss;

    numsubsectors = W_LumpLength (lump) / sizeof(mapsubsector_t);
    subsectors = Z_Malloc (numsubsectors*sizeof(subsector_t),PU_LEVEL,0);
    data = W_CacheLumpNum (lump,PU_STATIC);

    ms = (mapsubsector_t *)data;
    memset (subsectors,0, numsubsectors*sizeof(subsector_t));
    ss = subsectors;

    for (i=0 ; i<numsubsectors ; i++, ss++, ms++)
    {
        ss->numlines = SHORT(ms->numsegs);
        ss->firstline = SHORT(ms->firstseg);
    }

    Z_Free (data);
}



//
// P_LoadSectors
//

//
// levelflats
//
#define MAXLEVELFLATS   256

int                     numlevelflats;
levelflat_t*            levelflats;

//SoM: Other files want this info.
int P_PrecacheLevelFlats()
{
  int flatmemory = 0;
  int i;
  int lump;

  //SoM: 4/18/2000: New flat code to make use of levelflats.
  for(i = 0; i < numlevelflats; i++)
  {
    lump = levelflats[i].lumpnum;
    if(devparm)
      flatmemory += W_LumpLength(lump);
    R_GetFlat (lump);
  }
  return flatmemory;
}

// help function for P_LoadSectors, find a flat in the active wad files,
// allocate an id for it, and set the levelflat (to speedup search)
//
int P_AddLevelFlat (char* flatname, levelflat_t* levelflat)
{
    union {
        char    s[9];
        int     x[2];
    } name8;

    int         i;
    int         v1,v2;
    int         lump;

    strncpy (name8.s,flatname,8);   // make it two ints for fast compares
    name8.s[8] = 0;                 // in case the name was a fill 8 chars
    strupr (name8.s);               // case insensitive
    v1 = name8.x[0];
    v2 = name8.x[1];

    //
    //  first scan through the already found flats
    //
    for (i=0; i<numlevelflats; i++,levelflat++)
    {
        if ( *(int *)levelflat->name == v1
             && *(int *)&levelflat->name[4] == v2)
        {
            break;
        }
    }

    // that flat was already found in the level, return the id
    if (i==numlevelflats)
    {
        // store the name
        *((int*)levelflat->name) = v1;
        *((int*)&levelflat->name[4]) = v2;

        // store the flat lump number
        lump = R_GetFlatNumForName (flatname);
        levelflat->lumpnum = lump;

        //hack test : mark old water textures for splashes
        if (!strncmp(flatname,"FWATER",6))
            levelflat->iswater = true;

        if (devparm)
            CONS_Printf ("flat %#03d: %s\n", numlevelflats, name8.s);

        numlevelflats++;

        if (numlevelflats>=MAXLEVELFLATS)
            I_Error("P_LoadSectors: too much flats in level\n");
    }

    // level flat id
    return i;
}


void P_LoadSectors (int lump)
{
    byte*               data;
    int                 i;
    mapsector_t*        ms;
    sector_t*           ss;

    levelflat_t*        foundflats;

    numsectors = W_LumpLength (lump) / sizeof(mapsector_t);
    sectors = Z_Malloc (numsectors*sizeof(sector_t),PU_LEVEL,0);
    memset (sectors, 0, numsectors*sizeof(sector_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    //Fab:FIXME: allocate for whatever number of flats
    //           512 different flats per level should be plenty
    foundflats = alloca(sizeof(levelflat_t) * MAXLEVELFLATS);
    if (!foundflats)
        I_Error ("P_LoadSectors: no mem\n");
    memset (foundflats, 0, sizeof(levelflat_t) * MAXLEVELFLATS);

    numlevelflats = 0;

    ms = (mapsector_t *)data;
    ss = sectors;
    for (i=0 ; i<numsectors ; i++, ss++, ms++)
    {
        ss->floorheight = SHORT(ms->floorheight)<<FRACBITS;
        ss->ceilingheight = SHORT(ms->ceilingheight)<<FRACBITS;

        //
        //  flats
        //
        ss->floorpic = P_AddLevelFlat (ms->floorpic,foundflats);
        ss->ceilingpic = P_AddLevelFlat (ms->ceilingpic,foundflats);

        ss->lightlevel = SHORT(ms->lightlevel);
        ss->special = SHORT(ms->special);
        ss->tag = SHORT(ms->tag);

        //added:31-03-98: quick hack to test water with DCK
/*        if (ss->tag < 0)
            CONS_Printf("Level uses dck-water-hack\n");*/

        ss->thinglist = NULL;
        ss->touching_thinglist = NULL; //SoM: 4/7/2000

        ss->stairlock = 0;
        ss->nextsec = -1;
        ss->prevsec = -1;

        ss->heightsec = -1; //SoM: 3/17/2000: This causes some real problems
        ss->altheightsec = 0; //SoM: 3/20/2000
        ss->floorlightsec = -1;
        ss->ceilinglightsec = -1;
        ss->ffloors = NULL;
        ss->floor_xoffs = ss->ceiling_xoffs = ss->floor_yoffs = ss->ceiling_yoffs = 0;
        ss->bottommap = ss->midmap = ss->topmap = -1;

		// Keep players out of Castle Eggman! Tails 09-03-2001
		if(ss->tag == 424)
		{
			if(!(players[consoleplayer].emerald1 && players[consoleplayer].emerald2
				&& players[consoleplayer].emerald3 && players[consoleplayer].emerald4 && players[consoleplayer].emerald5
				&& players[consoleplayer].emerald6 && players[consoleplayer].emerald7))
			{
				if(!(gamemap == 22 || gamemap == 23 || gamemap == 24))
					I_Error("Nice try, Spongebob Fancypants!");
				else
					I_Error("Stop cheating and get\nall the emeralds!");
			}
		}
    }

    Z_Free (data);

    // whoa! there is usually no more than 25 different flats used per level!!
    //I_Error ("%d flats found\n", numlevelflats);

    // set the sky flat num
    skyflatnum = P_AddLevelFlat ("F_SKY1",foundflats);

    // copy table for global usage
    levelflats = Z_Malloc (numlevelflats*sizeof(levelflat_t),PU_LEVEL,0);
    memcpy (levelflats, foundflats, numlevelflats*sizeof(levelflat_t));

    // search for animated flats and set up
    P_SetupLevelFlatAnims ();
}


//
// P_LoadNodes
//
void P_LoadNodes (int lump)
{
    byte*       data;
    int         i;
    int         j;
    int         k;
    mapnode_t*  mn;
    node_t*     no;

    numnodes = W_LumpLength (lump) / sizeof(mapnode_t);
    nodes = Z_Malloc (numnodes*sizeof(node_t),PU_LEVEL,0);
    data = W_CacheLumpNum (lump,PU_STATIC);

    mn = (mapnode_t *)data;
    no = nodes;

    for (i=0 ; i<numnodes ; i++, no++, mn++)
    {
        no->x = SHORT(mn->x)<<FRACBITS;
        no->y = SHORT(mn->y)<<FRACBITS;
        no->dx = SHORT(mn->dx)<<FRACBITS;
        no->dy = SHORT(mn->dy)<<FRACBITS;
        for (j=0 ; j<2 ; j++)
        {
            no->children[j] = SHORT(mn->children[j]);
            for (k=0 ; k<4 ; k++)
                no->bbox[j][k] = SHORT(mn->bbox[j][k])<<FRACBITS;
        }
    }

    Z_Free (data);
}


//
// P_LoadThings
//
void P_LoadThings (int lump)
{
    int                 i;
    mapthing_t*         mt;
    boolean             spawn;

    mapthings    = W_CacheLumpNum (lump,PU_LEVEL);
    nummapthings = W_LumpLength (lump) / sizeof(mapthing_t);

    mt = mapthings;
    for (i=0 ; i<nummapthings ; i++, mt++)
    {
        spawn = true;

        // Do not spawn cool, new monsters if !commercial
        if ( gamemode != commercial)
        {
            switch(mt->type)
            {
              case 68:  // Arachnotron
              case 64:  // Archvile
              case 88:  // Boss Brain
              case 89:  // Boss Shooter
              case 69:  // Hell Knight
              case 67:  // Mancubus
              case 71:  // Pain Elemental
              case 65:  // Former Human Commando
              case 66:  // Revenant
              case 84:  // Wolf SS
                spawn = false;
                break;
            }
        }
        if (spawn == false)
            break;

        // Do spawn all other stuff.
        mt->x = SHORT(mt->x);
        mt->y = SHORT(mt->y);
        mt->angle = SHORT(mt->angle);
        mt->type = SHORT(mt->type);
        mt->options = SHORT(mt->options);

        P_SpawnMapThing (mt);
    }
}


//
// P_LoadLineDefs
// Also counts secret lines for intermissions.
//
void P_LoadLineDefs (int lump)
{
    byte*               data;
    int                 i;
    maplinedef_t*       mld;
    line_t*             ld;
    vertex_t*           v1;
    vertex_t*           v2;

    numlines = W_LumpLength (lump) / sizeof(maplinedef_t);
    lines = Z_Malloc (numlines*sizeof(line_t),PU_LEVEL,0);
    memset (lines, 0, numlines*sizeof(line_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    mld = (maplinedef_t *)data;
    ld = lines;
    for (i=0 ; i<numlines ; i++, mld++, ld++)
    {
        ld->flags = SHORT(mld->flags);
        ld->special = SHORT(mld->special);
        ld->tag = SHORT(mld->tag);
        v1 = ld->v1 = &vertexes[SHORT(mld->v1)];
        v2 = ld->v2 = &vertexes[SHORT(mld->v2)];
        ld->dx = v2->x - v1->x;
        ld->dy = v2->y - v1->y;

        if (!ld->dx)
            ld->slopetype = ST_VERTICAL;
        else if (!ld->dy)
            ld->slopetype = ST_HORIZONTAL;
        else
        {
            if (FixedDiv (ld->dy , ld->dx) > 0)
                ld->slopetype = ST_POSITIVE;
            else
                ld->slopetype = ST_NEGATIVE;
        }

        if (v1->x < v2->x)
        {
            ld->bbox[BOXLEFT] = v1->x;
            ld->bbox[BOXRIGHT] = v2->x;
        }
        else
        {
            ld->bbox[BOXLEFT] = v2->x;
            ld->bbox[BOXRIGHT] = v1->x;
        }

        if (v1->y < v2->y)
        {
            ld->bbox[BOXBOTTOM] = v1->y;
            ld->bbox[BOXTOP] = v2->y;
        }
        else
        {
            ld->bbox[BOXBOTTOM] = v2->y;
            ld->bbox[BOXTOP] = v1->y;
        }

        ld->sidenum[0] = SHORT(mld->sidenum[0]);
        ld->sidenum[1] = SHORT(mld->sidenum[1]);

        if (ld->sidenum[0] != -1 && ld->special)
          sides[ld->sidenum[0]].special = ld->special;
    }

    Z_Free (data);
}


void P_LoadLineDefs2()
{
  int i;
  line_t* ld = lines;
  for(i = 0; i < numlines; i++, ld++)
  {
  if (ld->sidenum[0] != -1)
    ld->frontsector = sides[ld->sidenum[0]].sector;
  else
    ld->frontsector = 0;

  if (ld->sidenum[1] != -1)
    ld->backsector = sides[ld->sidenum[1]].sector;
  else
    ld->backsector = 0;
  }
}


//
// P_LoadSideDefs
//
/*void P_LoadSideDefs (int lump)
{
    byte*               data;
    int                 i;
    mapsidedef_t*       msd;
    side_t*             sd;

    numsides = W_LumpLength (lump) / sizeof(mapsidedef_t);
    sides = Z_Malloc (numsides*sizeof(side_t),PU_LEVEL,0);
    memset (sides, 0, numsides*sizeof(side_t));
    data = W_CacheLumpNum (lump,PU_STATIC);

    msd = (mapsidedef_t *)data;
    sd = sides;
    for (i=0 ; i<numsides ; i++, msd++, sd++)
    {
        sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
        sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;
        sd->toptexture = R_TextureNumForName(msd->toptexture);
        sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
        sd->midtexture = R_TextureNumForName(msd->midtexture);

        sd->sector = &sectors[SHORT(msd->sector)];
    }

    Z_Free (data);
}*/

void P_LoadSideDefs (int lump)
{
  numsides = W_LumpLength(lump) / sizeof(mapsidedef_t);
  sides = Z_Malloc(numsides*sizeof(side_t),PU_LEVEL,0);
  memset(sides, 0, numsides*sizeof(side_t));
}

// SoM: 3/22/2000: Delay loading texture names until after loaded linedefs.

//Hurdler: 04/04/2000: proto added
int R_ColormapNumForName(char *name);

void P_LoadSideDefs2(int lump)
{
  byte *data = W_CacheLumpNum(lump,PU_STATIC);
  int  i;
  int  num;
  int  mapnum;

  for (i=0; i<numsides; i++)
    {
      register mapsidedef_t *msd = (mapsidedef_t *) data + i;
      register side_t *sd = sides + i;
      register sector_t *sec;

      sd->textureoffset = SHORT(msd->textureoffset)<<FRACBITS;
      sd->rowoffset = SHORT(msd->rowoffset)<<FRACBITS;

      // refined to allow colormaps to work as wall
      // textures if invalid as colormaps but valid as textures.

      sd->sector = sec = &sectors[SHORT(msd->sector)];
      switch (sd->special)
        {
        case 242:                       // variable colormap via 242 linedef
        case 270:                       //SoM: 3/22/2000: New water type.
        case 272:                       //SoM: 4/4/2000: Just colormap transfer
  #ifdef HWRENDER
          if(rendermode == render_soft)
          {
  #endif
            num = R_CheckTextureNumForName(msd->toptexture);
            if(num == -1)
            {
              sec->topmap = mapnum = R_ColormapNumForName(msd->toptexture);
              sd->toptexture = 0;
              if(devparm)
                CONS_Printf("topmapnum: %i, name: %.8s\n", mapnum, msd->toptexture);
            }
            else
              sd->toptexture = num;

            num = R_CheckTextureNumForName(msd->midtexture);
            if(num == -1)
            {
              sec->midmap = mapnum = R_ColormapNumForName(msd->midtexture);
              sd->midtexture = 0;
              if(devparm)
                CONS_Printf("midmapnum: %i, name: %.8s\n", mapnum, msd->midtexture);
            }
            else
              sd->midtexture = num;

            num = R_CheckTextureNumForName(msd->bottomtexture);
            if(num == -1)
            {
              sec->bottommap = mapnum = R_ColormapNumForName(msd->bottomtexture);
              sd->bottomtexture = 0;
              if(devparm)
                CONS_Printf("bottommapnum: %i, name: %.8s\n", mapnum, msd->bottomtexture);
            }
            else
              sd->bottomtexture = num;
            break;
  #ifdef HWRENDER
          }
          else
          {
            if((num = R_CheckTextureNumForName(msd->toptexture)) == -1)
              sd->toptexture = 0;
            else
              sd->toptexture = num;

            if((num = R_CheckTextureNumForName(msd->midtexture)) == -1)
              sd->midtexture = 0;
            else
              sd->midtexture = num;

            if((num = R_CheckTextureNumForName(msd->bottomtexture)) == -1)
              sd->bottomtexture = 0;
            else
              sd->bottomtexture = num;
          }
  #endif

        case 260:
          num = R_CheckTextureNumForName(msd->midtexture);
          if(num == -1)
            sd->midtexture = 1;
          else
            sd->midtexture = num;

          num = R_CheckTextureNumForName(msd->toptexture);
          if(num == -1)
            sd->toptexture = 1;
          else
            sd->toptexture = num;

          num = R_CheckTextureNumForName(msd->bottomtexture);
          if(num == -1)
            sd->bottomtexture = 1;
          else
            sd->bottomtexture = num;
          break;

/*        case 260: // killough 4/11/98: apply translucency to 2s normal texture
          sd->midtexture = strncasecmp("TRANMAP", msd->midtexture, 8) ?
            (sd->special = W_CheckNumForName(msd->midtexture)) < 0 ||
            W_LumpLength(sd->special) != 65536 ?
            sd->special=0, R_TextureNumForName(msd->midtexture) :
              (sd->special++, 0) : (sd->special=0);
          sd->toptexture = R_TextureNumForName(msd->toptexture);
          sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
          break;*/ //This code is replaced.. I need to fix this though

        default:                        // normal cases
          sd->midtexture = R_TextureNumForName(msd->midtexture);
          sd->toptexture = R_TextureNumForName(msd->toptexture);
          sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
          break;
        }
    }
  Z_Free (data);
}




//
// P_LoadBlockMap
//
void P_LoadBlockMap (int lump)
{
    int         i;
    int         count;

    blockmaplump = W_CacheLumpNum (lump,PU_LEVEL);
    blockmap = blockmaplump+4;
    count = W_LumpLength (lump)/2;

    for (i=0 ; i<count ; i++)
        blockmaplump[i] = SHORT(blockmaplump[i]);

    bmaporgx = blockmaplump[0]<<FRACBITS;
    bmaporgy = blockmaplump[1]<<FRACBITS;
    bmapwidth = blockmaplump[2];
    bmapheight = blockmaplump[3];

    // clear out mobj chains
    count = sizeof(*blocklinks)* bmapwidth*bmapheight;
    blocklinks = Z_Malloc (count,PU_LEVEL, 0);
    memset (blocklinks, 0, count);
}



//
// P_GroupLines
// Builds sector line lists and subsector sector numbers.
// Finds block bounding boxes for sectors.
//
void P_GroupLines (void)
{
    line_t**            linebuffer;
    int                 i;
    int                 j;
    int                 total;
    line_t*             li;
    sector_t*           sector;
    subsector_t*        ss;
    seg_t*              seg;
    fixed_t             bbox[4];
    int                 block;

    // look up sector number for each subsector
    ss = subsectors;
    for (i=0 ; i<numsubsectors ; i++, ss++)
    {
        seg = &segs[ss->firstline];
        ss->sector = seg->sidedef->sector;
    }

    // count number of lines in each sector
    li = lines;
    total = 0;
    for (i=0 ; i<numlines ; i++, li++)
    {
        total++;
        li->frontsector->linecount++;

        if (li->backsector && li->backsector != li->frontsector)
        {
            li->backsector->linecount++;
            total++;
        }
    }

    // build line tables for each sector
    linebuffer = Z_Malloc (total*4, PU_LEVEL, 0);
    sector = sectors;
    for (i=0 ; i<numsectors ; i++, sector++)
    {
        M_ClearBox (bbox);
        sector->lines = linebuffer;
        li = lines;
        for (j=0 ; j<numlines ; j++, li++)
        {
            if (li->frontsector == sector || li->backsector == sector)
            {
                *linebuffer++ = li;
                M_AddToBox (bbox, li->v1->x, li->v1->y);
                M_AddToBox (bbox, li->v2->x, li->v2->y);
            }
        }
        if (linebuffer - sector->lines != sector->linecount)
            I_Error ("P_GroupLines: miscounted");

        // set the degenmobj_t to the middle of the bounding box
        sector->soundorg.x = (bbox[BOXRIGHT]+bbox[BOXLEFT])/2;
        sector->soundorg.y = (bbox[BOXTOP]+bbox[BOXBOTTOM])/2;

        // adjust bounding box to map blocks
        block = (bbox[BOXTOP]-bmaporgy+MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block >= bmapheight ? bmapheight-1 : block;
        sector->blockbox[BOXTOP]=block;

        block = (bbox[BOXBOTTOM]-bmaporgy-MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXBOTTOM]=block;

        block = (bbox[BOXRIGHT]-bmaporgx+MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block >= bmapwidth ? bmapwidth-1 : block;
        sector->blockbox[BOXRIGHT]=block;

        block = (bbox[BOXLEFT]-bmaporgx-MAXRADIUS)>>MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXLEFT]=block;
    }

}


//
// Setup sky texture to use for the level, actually moved the code
// from G_DoLoadLevel() which had nothing to do there.
//
// - in future, each level may use a different sky.
//
// The sky texture to be used instead of the F_SKY1 dummy.
void P_SetupLevelSky (void)
{
    char       skytexname[12];

    // DOOM determines the sky texture to be used
    // depending on the current episode, and the game version.

    if ( (gamemode == commercial) )
      // || (gamemode == pack_tnt) he ! is not a mode is a episode !
      //    || ( gamemode == pack_plut )
    {
		if(gamemap != 33)
		{
        sprintf (skytexname,"SKY%d",gamemap);
        skytexture = R_TextureNumForName (skytexname);
		}
		else
			skytexture = R_TextureNumForName ("SKY1"); // Tails
    }
    else
    if ( (gamemode==retail) ||
         (gamemode==registered) )
    {
        if (gameepisode<1 || gameepisode>4)     // useful??
            gameepisode = 1;

        sprintf (skytexname,"SKY%d",gameepisode);
        skytexture = R_TextureNumForName (skytexname);
    }
    else // who knows?
        skytexture = R_TextureNumForName ("SKY1");


    // scale up the old skies, if needed
    R_SetupSkyDraw ();
}


//
// P_SetupLevel
//
// added comment : load the level from a lump file or from a external wad !
extern int numflats;
extern int numtextures;

int        lastloadedmaplumpnum; // for comparative savegame
boolean P_SetupLevel (int           episode,
                      int           map,
                      skill_t       skill,
                      char*         wadname)      // for wad files
{
    int         i;
	int numplayers; // Tails 08-11-2001

    //Initialize sector node list.
    P_Initsecnode();

    totalkills = totalitems = totalsecret = totalrings = wminfo.maxfrags = numplayers = 0; // Tails 08-11-2001
    wminfo.partime = 180;

	for(i=0; i<MAXPLAYERS; i++) // Find number of players Tails 08-11-2001
	{
		if(playeringame[i])
			numplayers++;
	}

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
        players[i].killcount = players[i].secretcount
            = players[i].itemcount = 0;

        players[i].timebonus = players[i].ringbonus = 0; // Reset the bonus counters Tails 03-10-2000

        players[i].health = 1; // Alright, who added this and didn't comment it? Tails 03-10-2000

        players[i].xtralife = 0; // Tails 04-25-2000

		if(cv_gametype.value == 2) // Tails 04-25-2001
		{
			players[i].lives = 3; // Tails 04-25-2001
			players[i].score = 0; // Tails 05-06-2001
		}
		else if (cv_gametype.value == 1)
			players[i].score = 0;

        players[i].seconds = 0; // Tails 06-06-2000
        players[i].minutes = 0; // Tails 06-06-2000
		players[i].exiting = 0; // Tails 12-20-2000
        //players[i].spirit = NULL;

		players[i].mfspinning = 0;
		players[i].mfstartdash = 0;
		players[i].mfjumped = 0;

		players[i].hunt1 = players[i].hunt2 = players[i].hunt3 = NULL;

    }

	if(gamemap == 33)
		CV_SetValue(&cv_chasecam, false);
	else
		CV_SetValue(&cv_chasecam, true);

    // Initial height of PointOfView
    // will be set by player think.

    players[consoleplayer].viewz = 1;

    // Make sure all sounds are stopped before Z_FreeTags.
    S_Start ();


#if 0 // UNUSED
    if (debugfile)
    {
        Z_FreeTags (PU_LEVEL, MAXINT);
        Z_FileDumpHeap (debugfile);
    }
    else
#endif
        Z_FreeTags (PU_LEVEL, PU_PURGELEVEL-1);

#ifdef WALLSPLATS
    // clear the splats from previous level
    R_ClearLevelSplats ();
#endif

    if (camera.chase)
        camera.mo = NULL;

    // UNUSED W_Profile ();
    
    P_InitThinkers ();

    // if working with a devlopment map, reload it
    W_Reload ();

    //
    //  load the map from internal game resource or external wad file
    //
    if (wadname)
    {
        char *lumpname=NULL;

        // go back to title screen if no map is loaded
        if (!P_AddWadFile (wadname,&lumpname) ||
            lumpname==NULL)            // no maps were found
        {
            // end the game
            D_StartTitle ();
            return false;
        }

        // P_AddWadFile() sets lumpname
        lastloadedmaplumpnum = W_GetNumForName(lumpname);
    }
    else
    {
        // internal game map
        lastloadedmaplumpnum = W_GetNumForName (G_BuildMapName(episode,map));
    }


    leveltime = 0;

    // textures are needed first
//    R_LoadTextures ();
//    R_FlushTextureCache();

    //faB: now part of level loading since in future each level may have
    //     its own anim texture sequences, switches etc.
    P_InitSwitchList ();
    P_InitPicAnims ();
    P_SetupLevelSky ();

    // note: most of this ordering is important
    P_LoadBlockMap (lastloadedmaplumpnum+ML_BLOCKMAP);
    P_LoadVertexes (lastloadedmaplumpnum+ML_VERTEXES);
    P_LoadSectors  (lastloadedmaplumpnum+ML_SECTORS);
    P_LoadSideDefs (lastloadedmaplumpnum+ML_SIDEDEFS);

    P_LoadLineDefs (lastloadedmaplumpnum+ML_LINEDEFS);
    P_LoadSideDefs2(lastloadedmaplumpnum+ML_SIDEDEFS);
    P_LoadLineDefs2();
    P_LoadSubsectors (lastloadedmaplumpnum+ML_SSECTORS);
    P_LoadNodes (lastloadedmaplumpnum+ML_NODES);
    P_LoadSegs (lastloadedmaplumpnum+ML_SEGS);
    rejectmatrix = W_CacheLumpNum (lastloadedmaplumpnum+ML_REJECT,PU_LEVEL);
    P_GroupLines ();

#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
        HWR_CreatePlanePolygons (numnodes-1);
#endif

    bodyqueslot = 0;

    deathmatch_p = deathmatchstarts;

    redctfstarts_p = redctfstarts; // CTF Tails 08-04-2001
    bluectfstarts_p = bluectfstarts; // CTF Tails 08-04-2001

    // added 25-4-98 : reset the players starts
    for(i=0;i<MAXPLAYERS;i++)
       playerstarts[i].type=-1;

    for(i=0;i<MAXPLAYERS;i++) // CTF Tails 08-04-2001
       bluectfstarts[i].type=-1; // CTF Tails 08-04-2001

    for(i=0;i<MAXPLAYERS;i++) // CTF Tails 08-04-2001
       redctfstarts[i].type=-1; // CTF Tails 08-04-2001

    P_LoadThings (lastloadedmaplumpnum+ML_THINGS);

    // spawnplayers now (beffor all structure are not inititialized)
    for (i=0 ; i<MAXPLAYERS ; i++)
        if (playeringame[i])
        {
            if (cv_deathmatch.value || cv_gametype.value == 1 || cv_gametype.value == 3)
            {
                players[i].mo = NULL;
                G_DoReborn(i);
            }
            else
                if( demoversion>=128 )
                {
                    players[i].mo = NULL;
                    G_CoopSpawnPlayer (i);
                }
        }

    // clear special respawning que
    iquehead = iquetail = 0;

    // set up world state
    P_SpawnSpecials ();

    // build subsector connect matrix
    //  UNUSED P_ConnectSubsectors ();

    //Fab:19-07-98:start cd music for this level (note: can be remapped)
	if(gamemap == 33)
        I_PlayCD (map + 1, false);  // Tails
		else
	{
    if (gamemode==commercial)
        I_PlayCD (map + 1, true);                // Doom2, 32 maps prepping for CD version Tails 04-05-2000
    else
        I_PlayCD ((episode-1)*9+map, true);  // Doom1, 9maps per episode
	}

    // preload graphics
#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
    {
        HWR_PrepLevelCache (numtextures);
#ifdef TANDL
        HWR_CreateStaticLightmaps (numnodes-1);
#endif
    }
#endif

    if (precache)
        R_PrecacheLevel ();

    //CONS_Printf("%d vertexs %d segs %d subsector\n",numvertexes,numsegs,numsubsectors);
    return true;
}


//
// Add a wadfile to the active wad files,
// replace sounds, musics, patches, textures, sprites and maps
//
boolean P_AddWadFile (char* wadfilename,char **firstmapname)
{
    int         firstmapreplaced;
    wadfile_t*  wadfile;
    char*       name;
    int         i,j,num,wadfilenum;
    lumpinfo_t* lumpinfo;
    int         replaces;
    boolean     texturechange;

    if ((wadfilenum = W_LoadWadFile (wadfilename))==-1)
    {
        CONS_Printf ("couldn't load wad file %s\n", COM_Argv(1));
        return false;
    }
    wadfile = wadfiles[wadfilenum];

    //
    // search for sound replacements
    //
    lumpinfo = wadfile->lumpinfo;
    replaces = 0;
    texturechange=false;
    for (i=0; i<wadfile->numlumps; i++,lumpinfo++)
    {
        name = lumpinfo->name;
        if (name[0]=='D' && name[1]=='S')
        {
            for (j=1 ; j<NUMSFX ; j++)
            {
                if ( S_sfx[j].name &&
                    !S_sfx[j].link &&
                    !strnicmp(S_sfx[j].name,name+2,6) )
                {
                    // the sound will be reloaded when needed,
                    // since sfx->data will be NULL
                    if (devparm)
                        CONS_Printf ("Sound %.8s replaced\n", name);
#ifndef SDL
                    I_FreeSfx (&S_sfx[j]);
#endif              
                    replaces++;
                }
            }
        }
        else
        if( memcmp(name,"TEXTURE1",8)==0    // find texture replesement too
         || memcmp(name,"TEXTURE2",8)==0
         || memcmp(name,"PNAMES",6)==0)
            texturechange=true;
    }
    if (!devparm && replaces)
        CONS_Printf ("%d sounds replaced\n");

    //
    // search for music replacements
    //
    lumpinfo = wadfile->lumpinfo;
    replaces = 0;
    for (i=0; i<wadfile->numlumps; i++,lumpinfo++)
    {
        name = lumpinfo->name;
        if (name[0]=='D' && name[1]=='_')
        {
            if (devparm)
                CONS_Printf ("Music %.8s replaced\n", name);
            replaces++;
        }
    }
    if (!devparm && replaces)
        CONS_Printf ("%d musics replaced\n");

    //
    // search for sprite replacements
    //
    R_AddSpriteDefs (sprnames, numwadfiles-1);

    //
    // search for texturechange replacements
    //
    if( texturechange ) // inited in the sound check
        R_LoadTextures();       // numtexture changes
    else
        R_FlushTextureCache();  // just reload it from file

    //
    // look for skins
    //
    R_AddSkins (wadfilenum);      //faB: wadfile index in wadfiles[]

    //
    // search for maps
    //
    lumpinfo = wadfile->lumpinfo;
    firstmapreplaced = 0;
    for (i=0; i<wadfile->numlumps; i++,lumpinfo++)
    {
        name = lumpinfo->name;
        num = firstmapreplaced;
        if (gamemode==commercial)       // Doom2
        {
            if (name[0]=='M' &&
                name[1]=='A' &&
                name[2]=='P')
            {
                num = (name[3]-'0')*10 + (name[4]-'0');
                CONS_Printf ("Map %d\n", num);
            }
        }
        else
        {
            if (name[0]=='E' &&
                ((unsigned)name[1]-'0')<='9' &&   // a digit
                name[2]=='M' &&
                ((unsigned)name[3]-'0')<='9' &&
                name[4]==0)
            {
                num = ((name[1]-'0')<<16) + (name[3]-'0');
                CONS_Printf ("Episode %d map %d\n", name[1]-'0',
                                                    name[3]-'0');
            }
        }
        if (num && (num<firstmapreplaced || !firstmapreplaced))
        {
            firstmapreplaced = num;
            if(firstmapname) *firstmapname = name;
        }
    }
    if (!firstmapreplaced)
        CONS_Printf ("no maps added\n");

    return true;
}
