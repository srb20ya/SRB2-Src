// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_setup.c,v 1.39 2001/08/19 20:41:03 hurdler Exp $
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
// Revision 1.39  2001/08/19 20:41:03  hurdler
// small changes
//
// Revision 1.38  2001/08/13 16:27:44  hurdler
// Added translucency to linedef 300 and colormap to 3d-floors
//
// Revision 1.37  2001/08/12 22:08:40  hurdler
// Add alpha value for 3d water
//
// Revision 1.36  2001/08/12 17:57:15  hurdler
// Beter support of sector coloured lighting in hw mode
//
// Revision 1.35  2001/08/11 15:18:02  hurdler
// Add sector colormap in hw mode (first attempt)
//
// Revision 1.34  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.33  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.32  2001/07/28 16:18:37  bpereira
// no message
//
// Revision 1.31  2001/06/16 08:07:55  bpereira
// no message
//
// Revision 1.30  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.29  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.28  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.27  2001/03/30 17:12:51  bpereira
// no message
//
// Revision 1.26  2001/03/19 21:18:48  metzgermeister
//   * missing textures in HW mode are replaced by default texture
//   * fixed crash bug with P_SpawnMissile(.) returning NULL
//   * deep water trick and other nasty thing work now in HW mode (tested with tnt/map02 eternal/map02)
//   * added cvar gr_correcttricks
//
// Revision 1.25  2001/03/13 22:14:19  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.24  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.23  2000/11/04 16:23:43  bpereira
// no message
//
// Revision 1.22  2000/11/03 03:27:17  stroggonmeth
// Again with the bug fixing...
//
// Revision 1.21  2000/11/02 19:49:36  bpereira
// no message
//
// Revision 1.20  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.19  2000/10/02 18:25:45  bpereira
// no message
//
// Revision 1.18  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.17  2000/08/11 21:37:17  hurdler
// fix win32 compilation problem
//
// Revision 1.16  2000/08/11 19:10:13  metzgermeister
// *** empty log message ***
//
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
#include "byteptr.h"
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
#include "st_stuff.h"
#include "w_wad.h"
#include "z_zone.h"
#include "r_splats.h"
#include "p_info.h"
#include "t_func.h"
#include "t_script.h"

#include "hu_stuff.h"
#include "console.h"
#include "m_misc.h"

#include "m_fixed.h"

#ifdef __WIN32__
#include "malloc.h"
#include "math.h"
#endif
#ifdef HWRENDER
#include "i_video.h"            //rendermode
#include "hardware/hw_main.h"
#include "hardware/hw_light.h"
#endif

#ifdef LINUX
int strupr(char *n);
#endif

extern consvar_t cv_chasecam; // declare the cam var! Tails 01-06-2000
extern consvar_t cv_chasecam2; // Tails 12-16-2002
extern consvar_t cv_homing;
extern consvar_t cv_lightdash;
extern consvar_t cv_racetype; // Graue 11-17-2003
fixed_t R_PointToDist2(fixed_t x2, fixed_t y2, fixed_t x1, fixed_t y1); // Tails 05-01-2002
subsector_t* R_PointInSubsector(fixed_t x, fixed_t y);
void I_FinishUpdate();

//
// MAP related Lookup tables.
// Store VERTEXES, LINEDEFS, SIDEDEFS, etc.
//
boolean         newlevel = false;
boolean         doom1level = false;    // doom 1 level running under doom 2
char            *levelmapname = NULL;

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

int numstarposts; // Graue 11-17-2003
unsigned bitstarposts; // Graue 11-18-2003

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

long*          blockmap;       // int for large maps
// offsets in blockmap are from here
long*          blockmaplump; // Big blockmap Tails

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
mapthing_t      *deathmatchstarts[MAX_DM_STARTS];
int             numdmstarts;
int             numcoopstarts; // Graue 12-23-2003
//mapthing_t**    deathmatch_p;
mapthing_t      *playerstarts[MAXPLAYERS];
mapthing_t      *bluectfstarts[MAXPLAYERS]; // CTF Tails 08-04-2001
mapthing_t      *redctfstarts[MAXPLAYERS]; // CTF Tails 08-04-2001
int				numredctfstarts; // CTF Tails 08-04-2001
int				numbluectfstarts; // CTF Tails 08-04-2001

void P_ClearStarPost(player_t* player, int postnum); // Tails 07-05-2002

//
// P_InitMapHeaders
//
// Reads the data from the map headers for all levels. // Tails 04-08-2003
void P_ClearMapHeaderInfo(void)
{
	int i;

	for(i=1; i<NUMMAPS+1; i++)
	{
		mapheaderinfo[i-1].actnum = 0;
		mapheaderinfo[i-1].nozone = false;
		mapheaderinfo[i-1].forcecharacter = 255;
		mapheaderinfo[i-1].lvlttl[0] = '\0';
		mapheaderinfo[i-1].musicslot = mus_map01m + i - 1;
		mapheaderinfo[i-1].nextlevel = i + 1;
		mapheaderinfo[i-1].typeoflevel = 0;
		mapheaderinfo[i-1].weather = 0;
		mapheaderinfo[i-1].skynum = i;
		mapheaderinfo[i-1].countdown = 0;
		mapheaderinfo[i-1].cutscenenum = 0;
		mapheaderinfo[i-1].interscreen[0] = '#';
		mapheaderinfo[i-1].scriptname[0] = '#';
		mapheaderinfo[i-1].scriptislump = false;
	}
}
void P_ClearSingleMapHeaderInfo(int i)
{
	mapheaderinfo[i-1].actnum = 0;
	mapheaderinfo[i-1].nozone = false;
	mapheaderinfo[i-1].forcecharacter = 255;
	mapheaderinfo[i-1].lvlttl[0] = '\0';
	mapheaderinfo[i-1].musicslot = mus_map01m + i - 1;
	mapheaderinfo[i-1].nextlevel = i + 1;
	mapheaderinfo[i-1].typeoflevel = 0;
	mapheaderinfo[i-1].weather = 0;
	mapheaderinfo[i-1].skynum = i;
	mapheaderinfo[i-1].countdown = 0;
	mapheaderinfo[i-1].cutscenenum = 0;
	mapheaderinfo[i-1].interscreen[0] = '#';
	mapheaderinfo[i-1].scriptname[0] = '#';
	mapheaderinfo[i-1].scriptislump = false;
}

void DEH_LoadDehackedLump(int lump); // Move up declaration Graue 11-15-2003

// Initialize the (old and new) map headers
void P_InitMapHeaders(void)
{
	char mapheader[7];
	char temp[64];
	char*       buf;
	int lumpnum;
	int i;
	boolean nonumber = false;
	int moremapnumbers;
	int mapnum;

	for(mapnum = 1; mapnum < NUMMAPS+1; mapnum++)
	{
		moremapnumbers = mapnum-1;

		strncpy(mapheader, G_BuildMapName(mapnum), 5);

		mapheader[5] = 'D'; // New header
		mapheader[6] = '\0'; // Graue 11-15-2003 bugfix

		lumpnum = W_CheckNumForName(mapheader);

		if(!(lumpnum == -1 || W_LumpLength(lumpnum) <= 0))
		{
			DEH_LoadDehackedLump(lumpnum);
			continue; // Done with this map!
		}

		// If there's no new header, check for an old one!

		mapheader[5] = 'N';
		lumpnum = W_CheckNumForName(mapheader);

		if(lumpnum == -1 || W_LumpLength(lumpnum) <= 0)
			continue; // No information. Oh well!

		// These four are not supported in the old header format
		mapheaderinfo[moremapnumbers].countdown = 0;
		mapheaderinfo[moremapnumbers].cutscenenum = 0;
		mapheaderinfo[moremapnumbers].interscreen[0] = '#';
		mapheaderinfo[moremapnumbers].scriptname[0] = '#';

		buf  = W_CacheLumpNum (lumpnum, PU_CACHE);

		strncpy (mapheaderinfo[moremapnumbers].lvlttl, buf, 32);

		for(i=0; i<32; i++)
		{
			if(mapheaderinfo[moremapnumbers].lvlttl[i] == '\n')
				break;
		}

		mapheaderinfo[moremapnumbers].actnum = atoi(&mapheaderinfo[moremapnumbers].lvlttl[i+1]);

		mapheaderinfo[moremapnumbers].lvlttl[i-1] = '\0';

		strcpy(temp, buf);

		i = 0;
		while(temp[i] != '\n')
			i++;

		i++; // Move it to the one after the \n

		while(temp[i] != '\n')
			i++;

		i++; // Move it to the one after the \n

		mapheaderinfo[moremapnumbers].forcecharacter = atoi(&temp[i]);

		while(temp[i] != '\n')
			i++;

		i++; // Move it to the one after the \n

		mapheaderinfo[moremapnumbers].musicslot = atoi(&temp[i]);

		while(temp[i] != '\n')
			i++;

		i++; // Move it to the one after the \n

		mapheaderinfo[moremapnumbers].nextlevel = atoi(&temp[i]);

		while(temp[i] != '\n')
			i++;

		i++; // Move it to the one after the \n

		mapheaderinfo[moremapnumbers].typeoflevel = atoi(&temp[i]);

		while(temp[i] != '\n')
			i++;

		i++; // Move it to the one after the \n

		mapheaderinfo[moremapnumbers].weather = atoi(&temp[i]);

		while(temp[i] != '\n')
			i++;

		i++; // Move it to the one after the \n

		mapheaderinfo[moremapnumbers].skynum = atoi(&temp[i]);

		if(!mapheaderinfo[moremapnumbers].skynum)
			mapheaderinfo[moremapnumbers].skynum = gamemap;
	}
}

//
// P_LoadMapHeader
//
// Reads the data from the map header for the level. // Tails 04-08-2003
void P_LoadMapHeader(int mapnum)
{
	char mapheader[7];
	char temp[64];
	char*       buf;
	int lumpnum;
	int i;
	boolean nonumber = false;
	int moremapnumbers = mapnum-1;

	strncpy(mapheader, G_BuildMapName(mapnum), 5);

	mapheader[5] = 'D'; // New header
	mapheader[6] = '\0'; // Graue 11-08-2003 bugfix

	lumpnum = W_CheckNumForName(mapheader);

	if(!(lumpnum == -1 || W_LumpLength(lumpnum) <= 0))
	{
		P_ClearSingleMapHeaderInfo(mapnum);
		DEH_LoadDehackedLump(lumpnum);
		return;
	}

	// No new map header? See if an old one exists.

	mapheader[5] = 'N'; // Old header

	lumpnum = W_CheckNumForName(mapheader);

	if(lumpnum == -1 || W_LumpLength(lumpnum) <= 0)
	{
		P_ClearSingleMapHeaderInfo(mapnum);
		return; // No information. Oh well!
	}

	P_ClearSingleMapHeaderInfo(mapnum);

	// These four are not supported in the old header format
	mapheaderinfo[moremapnumbers].countdown = 0;
	mapheaderinfo[moremapnumbers].cutscenenum = 0;
	mapheaderinfo[moremapnumbers].interscreen[0] = '#';
	mapheaderinfo[moremapnumbers].scriptname[0] = '#';

    buf  = W_CacheLumpNum (lumpnum, PU_CACHE);

    strncpy (mapheaderinfo[moremapnumbers].lvlttl, buf, 32);

	for(i=0; i<32; i++)
	{
		if(mapheaderinfo[moremapnumbers].lvlttl[i] == '\n')
			break;
	}

	mapheaderinfo[moremapnumbers].actnum = atoi(&mapheaderinfo[moremapnumbers].lvlttl[i+1]);

	mapheaderinfo[moremapnumbers].lvlttl[i-1] = '\0';

	strcpy(temp, buf);

	i = 0;
	while(temp[i] != '\n')
		i++;

	i++; // Move it to the one after the \n

	while(temp[i] != '\n')
		i++;

	i++; // Move it to the one after the \n

	mapheaderinfo[moremapnumbers].forcecharacter = atoi(&temp[i]);

	while(temp[i] != '\n')
		i++;

	i++; // Move it to the one after the \n

	mapheaderinfo[moremapnumbers].musicslot = atoi(&temp[i]);

	while(temp[i] != '\n')
		i++;

	i++; // Move it to the one after the \n

	mapheaderinfo[moremapnumbers].nextlevel = atoi(&temp[i]);

	while(temp[i] != '\n')
		i++;

	i++; // Move it to the one after the \n

	mapheaderinfo[moremapnumbers].typeoflevel = atoi(&temp[i]);

	while(temp[i] != '\n')
		i++;

	i++; // Move it to the one after the \n

	mapheaderinfo[moremapnumbers].weather = atoi(&temp[i]);

	while(temp[i] != '\n')
		i++;

	i++; // Move it to the one after the \n

	mapheaderinfo[moremapnumbers].skynum = atoi(&temp[i]);

	if(!mapheaderinfo[moremapnumbers].skynum)
		mapheaderinfo[moremapnumbers].skynum = gamemap;

	CONS_Printf("%s\n", mapheaderinfo[moremapnumbers].lvlttl);
	if(mapheaderinfo[moremapnumbers].actnum)
		CONS_Printf("Act %d\n", mapheaderinfo[moremapnumbers].actnum);
}


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
            //Hurdler: 04/12/2000: for now, only used in hardware mode
            li->lightmaps = NULL; // list of static lightmap for this seg
        }
#endif

        li->angle = (SHORT(ml->angle))<<16;
        li->offset = (SHORT(ml->offset))<<16;
        linedef = SHORT(ml->linedef);
        ldef = &lines[linedef];
        li->linedef = ldef;
        li->side = side = SHORT(ml->side);
        li->sidedef = &sides[ldef->sidenum[side]];
        li->frontsector = sides[ldef->sidenum[side]].sector;
        if (ldef-> flags & ML_TWOSIDED)
            li->backsector = sides[ldef->sidenum[side^1]].sector;
        else
            li->backsector = 0;

        li->numlights = 0;
        li->rlights = NULL;
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




int P_FlatNumForName(char *flatname)
{
  return P_AddLevelFlat(flatname, levelflats);
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
        levelflat->lumpnum = R_GetFlatNumForName (flatname);

        if (devparm)
            CONS_Printf ("flat %#03d: %s\n", numlevelflats, name8.s);

        numlevelflats++;

        if (numlevelflats>=MAXLEVELFLATS)
            I_Error("P_LoadSectors: too many flats in level\n");
    }

    // level flat id
    return i;
}


// SoM: Do I really need to comment this?
char *P_FlatNameForNum(int num)
{
  if(num < 0 || num > numlevelflats)
    I_Error("P_FlatNameForNum: Invalid flatnum\n");

  return Z_Strdup(va("%.8s", levelflats[num].name), PU_STATIC, 0);
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
        if( strnicmp(ms->floorpic,"FWATER",6)==0 || 
            strnicmp(ms->floorpic,"FLTWAWA1",8)==0 ||
            strnicmp(ms->floorpic,"FLTFLWW1",8)==0 )
            ss->floortype = FLOOR_WATER;
        else
        if( strnicmp(ms->floorpic,"FLTLAVA1",8)==0 ||
            strnicmp(ms->floorpic,"FLATHUH1",8)==0 )
            ss->floortype = FLOOR_LAVA;
        else
        if( strnicmp(ms->floorpic,"FLTSLUD1",8)==0 )
            ss->floortype = FLOOR_SLUDGE;
        else
            ss->floortype = FLOOR_SOLID;

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
		ss->preciplist = NULL;
		ss->touching_preciplist = NULL;

        ss->stairlock = 0;
        ss->nextsec = -1;
        ss->prevsec = -1;

        ss->heightsec = -1; //SoM: 3/17/2000: This causes some real problems
        ss->altheightsec = 0; //SoM: 3/20/2000
        ss->floorlightsec = -1;
        ss->ceilinglightsec = -1;
        ss->ffloors = NULL;
        ss->lightlist = NULL;
        ss->numlights = 0;
        ss->attached = NULL;
        ss->numattached = 0;
        ss->moved = true;
        ss->floor_xoffs = ss->ceiling_xoffs = ss->floor_yoffs = ss->ceiling_yoffs = 0;
        ss->bottommap = ss->midmap = ss->topmap = -1;
		ss->gravity = NULL; // Tails 08-29-2002
        
        // ----- for special tricks with HW renderer -----
        ss->pseudoSector = false;
        ss->virtualFloor = false;
        ss->virtualCeiling = false;
        ss->sectorLines = NULL;
        ss->stackList = NULL;
        ss->lineoutLength = -1.0;
        // ----- end special tricks -----

		// Keep players out of secret levels! Tails 09-03-2001
		if(ss->tag == 424) // Mario
		{
			if(!(grade & 8))
				I_Error("You need to unlock this level first!\n");
		}
		else if(ss->tag == 425) // NiGHTS
		{
			if(!(grade & 16))
				I_Error("You need to unlock this level first!\n");
		}
		else if(ss->tag == 426) // Christmas Hunt
		{
			if(!(grade & 32))
				I_Error("You need to unlock this level first!\n");
		}
		else if(ss->tag == 427) // Adventure
		{
			if(!(grade & 64))
				I_Error("You need to unlock this level first!\n");
		}
		else if(ss->tag == 428) // Sonic Golf Graue 12-13-2003
		{
			if(!(grade & 1024))
				I_Error("You need to unlock this level first!\n");
		}
    }

    Z_Free (data);

    // whoa! there is usually no more than 25 different flats used per level!!
    //CONS_Printf ("%d flats found\n", numlevelflats);

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

// Tails 11-08-2002
void P_SpawnHoopsAndRings(mapthing_t* mthing);

//
// P_LoadThings
//
void P_LoadThings (int lump)
{
    int                 i;
    mapthing_t*         mt;
    char                *data, *datastart;
	mobj_t*       emblemmobj; // Tails 12-08-2002

    data = datastart = W_CacheLumpNum (lump,PU_LEVEL);
    nummapthings     = W_LumpLength (lump) / (5 * sizeof(short));
    mapthings        = Z_Malloc(nummapthings * sizeof(mapthing_t), PU_LEVEL, NULL);

    //SoM: Because I put a new member into the mapthing_t for use with
    //fragglescript, the format has changed and things won't load correctly
    //using the old method.

	tokenbits = 0; // Tails 12-18-2003

    mt = mapthings;
    for (i=0 ; i<nummapthings ; i++, mt++)
    {
        // Do spawn all other stuff.
        // SoM: Do this first so all the mapthing slots are filled!
        mt->x = SHORT(READSHORT(data));
        mt->y = SHORT(READSHORT(data));
        mt->angle = SHORT(READSHORT(data));
        mt->type = SHORT(READSHORT(data));
        mt->options = SHORT(READSHORT(data));
        mt->mobj = NULL; //SoM:

		if((mt->type<=4028 && mt->type>=4001) || mt->type == 11)
			mt->oldnum = mt->type;
		else
			mt->oldnum = 0;

		// Z for objects Tails 05-26-2002
		mt->z = R_PointInSubsector(mt->x << FRACBITS, mt->y << FRACBITS)->sector->floorheight >> FRACBITS;

        P_SpawnMapThing (mt);
    }
    Z_Free(datastart);

	data = datastart = W_CacheLumpNum (lump,PU_LEVEL);

	// Run through the list of mapthings again to spawn hoops and rings Tails 11-08-2002
	mt = mapthings;
    for (i=0 ; i<nummapthings ; i++, mt++)
    {
		short thisx = SHORT(READSHORT(data));
		short thisy = SHORT(READSHORT(data));
		short thisangle = SHORT(READSHORT(data));
		short thistype = SHORT(READSHORT(data));
		short thisoptions = SHORT(READSHORT(data));

		switch(thistype)
		{
			case 57:
			case 37:
			case 2048:
			case 2010:
			case 2014:
			case 84:
			case 44:
			case 76:
			case 77:
			case 47:
			case 2007:
			case 2046:
			case 2047:
				mt->mobj = NULL; //SoM:

				// Z for objects Tails 05-26-2002
				mt->z = R_PointInSubsector(mt->x << FRACBITS, mt->y << FRACBITS)->sector->floorheight >> FRACBITS;

				P_SpawnHoopsAndRings (mt);
				break;
			default:
				break;
		}
    }
	Z_Free(datastart);

	// Now let's spawn those funky emblem things! Tails 12-08-2002
	if(netgame || multiplayer)
		return;

	if(modifiedgame) // No cheating!!
		return;

	for(i=0; i<NUMEMBLEMS-2; i++)
	{
		if(emblemlocations[i].player != players[0].skin)
			continue;

		if(emblemlocations[i].level != gamemap)
			continue;

		emblemmobj = P_SpawnMobj(emblemlocations[i].x<<FRACBITS,emblemlocations[i].y<<FRACBITS,emblemlocations[i].z<<FRACBITS, MT_EMBLEM);
		P_SetMobjStateNF(emblemmobj, emblemmobj->info->spawnstate + i);

		if(gottenemblems & emblemlocations[i].flagnum)
		{
			P_UnsetThingPosition(emblemmobj);
			emblemmobj->flags |= MF_NOCLIP;
			emblemmobj->flags &= ~MF_SPECIAL;
			emblemmobj->flags |= MF_NOBLOCKMAP;
			emblemmobj->frame |= (tr_transmor<<FF_TRANSSHIFT);
			P_SetThingPosition(emblemmobj);
		}
		else
			emblemmobj->frame &= ~FF_TRANSMASK;

		emblemmobj->health = emblemlocations[i].flagnum;
	}

	// Easter eggs!
	if (!eastermode)
		return;

	for(i=0; i<NUMEGGS; i++)
	{
		if(egglocations[i].level != gamemap)
			continue;


		if(foundeggs & egglocations[i].flagnum) // They already found this egg... drat!
			continue;

		emblemmobj = P_SpawnMobj(egglocations[i].x<<FRACBITS,egglocations[i].y<<FRACBITS,egglocations[i].z<<FRACBITS, MT_EASTEREGG);
		emblemmobj->health = egglocations[i].flagnum;
		emblemmobj->flags =  (emblemmobj->flags & ~MF_TRANSLATION)
				 | ((i % 13)<<MF_TRANSSHIFT);
	}
}

// Experimental groovy write functions! Tails
void P_WriteThings (int lump)
{
    int                 i;
	int            length;
    mapthing_t*         mt;
    char                *data, *datastart;
	byte*           save_p;
	byte*       savebuffer;

    data = datastart = W_CacheLumpNum (lump,PU_LEVEL);
//    nummapthings     = W_LumpLength (lump) / (5 * sizeof(short));
	save_p = savebuffer = (byte *)malloc(nummapthings * sizeof(mapthing_t));

    //SoM: Because I put a new member into the mapthing_t for use with
    //fragglescript, the format has changed and things won't load correctly
    //using the old method.

	if(!save_p)
    {
        CONS_Printf ("No more free memory for thingwriting!\n");
        return;
    }

    mt = mapthings;
    for (i=0 ; i<nummapthings ; i++, mt++)
    {
        WRITESHORT(save_p, mt->x);
		WRITESHORT(save_p, mt->y);
		WRITESHORT(save_p, mt->angle);

		// For 4001-4028 player starts
		if(mt->oldnum != 0)
			WRITESHORT(save_p, mt->oldnum);
		else
			WRITESHORT(save_p, mt->type);

		WRITESHORT(save_p, mt->options);
    }

	Z_Free(datastart);

	length = save_p - savebuffer;

    FIL_WriteFile ("newthings.lmp", savebuffer, length);
    free(savebuffer);
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
int M_CheckParm(char *check);

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

if (M_CheckParm ("-nocolormaps"))
{
      switch (sd->special)
        {
        case 300:
        case 301:
		case 44:
		case 45:
            if(msd->toptexture[0] == '#')
            {
                char *col = msd->toptexture;
                sd->toptexture = sd->bottomtexture = ((col[1]-'0')*100+(col[2]-'0')*10+col[3]-'0')+1;
            }
            else
                sd->toptexture = sd->bottomtexture = 0;
            sd->midtexture = R_TextureNumForName(msd->midtexture);
            break;

        default:                        // normal cases
          sd->midtexture = R_TextureNumForName(msd->midtexture);
          sd->toptexture = R_TextureNumForName(msd->toptexture);
          sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
          break;
	  }
}
else
{
      switch (sd->special)
        {
        case 242:                       // variable colormap via 242 linedef
        case 270:                       //SoM: 3/22/2000: New water type.
		case 14: // Tails
#ifdef HWRENDER
          if(rendermode == render_soft)
          {
#endif
            num = R_CheckTextureNumForName(msd->toptexture);

            if(num == -1)
            {
              sec->topmap = mapnum = R_ColormapNumForName(msd->toptexture);
              sd->toptexture = 0;
            }
            else
              sd->toptexture = num;

            num = R_CheckTextureNumForName(msd->midtexture);
            if(num == -1)
            {
              sec->midmap = mapnum = R_ColormapNumForName(msd->midtexture);
              sd->midtexture = 0;
            }
            else
              sd->midtexture = num;

            num = R_CheckTextureNumForName(msd->bottomtexture);
            if(num == -1)
            {
              sec->bottommap = mapnum = R_ColormapNumForName(msd->bottomtexture);
              sd->bottomtexture = 0;
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

            break;
          }
#endif

        case 272:                       //SoM: 4/4/2000: Just colormap transfer
		case 16: // Tails

// SoM: R_CreateColormap will only create a colormap in software mode...
// Perhaps we should just call it instead of doing the calculations here.
#ifdef HWRENDER
          if(rendermode == render_soft)
          {
#endif
            if(msd->toptexture[0] == '#' || msd->bottomtexture[0] == '#')
            {
              sec->midmap = R_CreateColormap(msd->toptexture, msd->midtexture, msd->bottomtexture);
              sd->toptexture = sd->bottomtexture = 0;
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
			break;

#ifdef HWRENDER
          }
          else
          {
            //Hurdler: for now, full support of toptexture only
            if(msd->toptexture[0] == '#')// || msd->bottomtexture[0] == '#')
            {
                char *col = msd->toptexture;

                sec->midmap = R_CreateColormap(msd->toptexture, msd->midtexture, msd->bottomtexture);
                sd->toptexture = sd->bottomtexture = 0;
                #define HEX2INT(x) (x >= '0' && x <= '9' ? x - '0' : x >= 'a' && x <= 'f' ? x - 'a' + 10 : x >= 'A' && x <= 'F' ? x - 'A' + 10 : 0)
                #define ALPHA2INT(x) (x >= 'a' && x <= 'z' ? x - 'a' : x >= 'A' && x <= 'Z' ? x - 'A' : 0)
                sec->extra_colormap = &extra_colormaps[sec->midmap];
                sec->extra_colormap->rgba = 
                            (HEX2INT(col[1]) << 4) + (HEX2INT(col[2]) << 0) +
                            (HEX2INT(col[3]) << 12) + (HEX2INT(col[4]) << 8) +
                            (HEX2INT(col[5]) << 20) + (HEX2INT(col[6]) << 16) + 
                            (ALPHA2INT(col[7]) << 24);
                #undef ALPHA2INT
                #undef HEX2INT
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
            break;
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

	   //Hurdler: added for alpha value with translucent 3D-floors/water
        case 300:
        case 301:
		case 44:
		case 45:
            if(msd->toptexture[0] == '#')
            {
                char *col = msd->toptexture;
                sd->toptexture = sd->bottomtexture = ((col[1]-'0')*100+(col[2]-'0')*10+col[3]-'0')+1;
            }
            else
                sd->toptexture = sd->bottomtexture = 0;
            sd->midtexture = R_TextureNumForName(msd->midtexture);
            break;

        default:                        // normal cases
          sd->midtexture = R_TextureNumForName(msd->midtexture);
          sd->toptexture = R_TextureNumForName(msd->toptexture);
          sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);
          break;
        }
}
}

  Z_Free (data);
}


// Init slopes Tails 05-01-2002
// [RH] Set slopes for sectors, based on line specials
//
// P_AlignPlane
//
// Aligns the floor or ceiling of a sector to the corresponding plane
// on the other side of the reference line. (By definition, line must be
// two-sided.)
//
// If (which & 1), sets floor.
// If (which & 2), sets ceiling.
//

#define FIXED2FLOAT(f)			((float)(f) / (float)FRACUNIT)
#define FLOAT2FIXED(f)			(fixed_t)((f) * (float)FRACUNIT)

static void P_AlignPlane (sector_t *sec, line_t *line, int which)
{
	sector_t *refsec;
	int bestdist;
	vertex_t *refvert = (*sec->lines)->v1;	// Shut up, GCC
	int i;
	line_t **probe;
//	vec3_t p, v1, v2, cross;
	float v1a, v1b, v1c, v2a, v2b, v2c, cross1, cross2, cross3;
	secplane_t *refplane;
	secplane_t *srcplane;
	fixed_t srcheight, destheight;

	if (line->backsector == NULL)
		return;

	bestdist = 0;
	for (i = sec->linecount*2, probe = sec->lines; i > 0; i--)
	{
		int dist;
		vertex_t *vert;

		// Do calculations with only the upper bits, because the lower ones
		// are all zero, and we would overflow for a lot of distances if we
		// kept them around.

		if (i & 1)
			vert = (*probe++)->v2;
		else
			vert = (*probe)->v1;

		dist = R_PointToDist2(line->v1->x, line->v1->y, vert->x, vert->y) >> FRACBITS;

		if (dist > bestdist)
		{
			bestdist = dist;
			refvert = vert;
		}
	}

	refsec = line->frontsector == sec ? line->backsector : line->frontsector;

	CONS_Printf("Bestdist is %i\n", bestdist);

//	p[0] = FIXED2FLOAT (line->v1->x);
//	p[1] = FIXED2FLOAT (line->v1->y);
	v1a = FIXED2FLOAT (line->dx);
	v1b = FIXED2FLOAT (line->dy);
	v2a = FIXED2FLOAT (refvert->x - line->v1->x);
	v2b = FIXED2FLOAT (refvert->y - line->v1->y);

	refplane = (which == 0) ? &refsec->floorplane : &refsec->ceilingplane;
	srcplane = (which == 0) ? &sec->floorplane : &sec->ceilingplane;
	srcheight = (which == 0) ? sec->floortexz : sec->ceilingtexz;
	destheight = (which == 0) ? refsec->floortexz : refsec->ceilingtexz;

//	p[2] = FIXED2FLOAT (destheight);
	v1c = 0;
	v2c = FIXED2FLOAT (srcheight - destheight);

//	CrossProduct (v1, v2, cross);
if(true)
{
	cross1 = v1b*v2c - v1c*v2b;
	cross2 = v1c*v2a - v1a*v2c;
	cross3 = v1a*v2b - v1b*v2a;
}

//	VectorNormalize (cross);
if(true)
{
	float length, ilength;

	length = cross1*cross1 + cross2*cross2 + cross3*cross3;
	length = (float)sqrt (length);		// FIXME

	if (length)
	{
		ilength = 1/length;
		cross1 *= ilength;
		cross2 *= ilength;
		cross3 *= ilength;
	}
}

	// Fix backward normals
	if ((cross3 < 0 && which == 0) || (cross3 > 0 && which == 1))
	{
		cross1 = -cross1;
		cross2 = -cross2;
		cross3 = -cross3;
	}

	srcplane->a = FLOAT2FIXED (cross1);
	srcplane->b = FLOAT2FIXED (cross2);
	srcplane->c = FLOAT2FIXED (cross3);
	srcplane->ic = FLOAT2FIXED (1.f/cross3);
	srcplane->d = -TMulScale16 (srcplane->a, line->v1->x,
								srcplane->b, line->v1->y,
								srcplane->c, destheight);

	CONS_Printf("a is %f\n", cross1);
	CONS_Printf("b is %f\n", cross2);
	CONS_Printf("c is %f\n", cross3);
	CONS_Printf("ic is %d\n", srcplane->ic);
	CONS_Printf("d is %d\n", srcplane->d);
}

void P_SetSlopes ()
{
	int i, s;

	for (i = 0; i < numlines; i++)
	{
		if (lines[i].special == 340)
		{
			lines[i].special = 0;
//			lines[i].id = lines[i].args[2];
			if (lines[i].backsector != NULL)
			{
				// args[0] is for floor, args[1] is for ceiling
				//
				// As a special case, if args[1] is 0,
				// then args[0], bits 2-3 are for ceiling.
				for (s = 0; s < 2; s++)
				{
//					int bits = lines[i].args[s] & 3;

//					if (s == 1 && bits == 0)
//						bits = (lines[i].args[0] >> 2) & 3;

//					if (bits == 1)			// align front side to back
						P_AlignPlane (lines[i].frontsector, lines + i, s);
//					else if (bits == 2)		// align back side to front
//						P_AlignPlane (lines[i].backsector, lines + i, s);
				}
			}
		}
	}
}

//
// killough 10/98:
//
// Rewritten to use faster algorithm.
//
// New procedure uses Bresenham-like algorithm on the linedefs, adding the
// linedef to each block visited from the beginning to the end of the linedef.
//
// The algorithm's complexity is on the order of nlines*total_linedef_length.
//
// Please note: This section of code is not interchangable with TeamTNT's
// code which attempts to fix the same problem.

static void P_CreateBlockMap(void)
{
  register unsigned int i;
  fixed_t minx = MAXINT, miny = MININT, maxx = MININT, maxy = MININT;

  // First find limits of map

  // Shut up, compiler! SHUT UP!
  for (i=0; i<(unsigned)numvertexes; i++) // No, make the other one unsigned! Graue 12-14-2003
    {
      if (vertexes[i].x >> FRACBITS < minx)
	minx = vertexes[i].x >> FRACBITS;
      else
	if (vertexes[i].x >> FRACBITS > maxx)
	  maxx = vertexes[i].x >> FRACBITS;
      if (vertexes[i].y >> FRACBITS < miny)
	miny = vertexes[i].y >> FRACBITS;
      else
	if (vertexes[i].y >> FRACBITS > maxy)
	  maxy = vertexes[i].y >> FRACBITS;
    }

  // Save blockmap parameters

  bmaporgx = minx << FRACBITS;
  bmaporgy = miny << FRACBITS;
  bmapwidth  = ((maxx-minx) >> MAPBTOFRAC) + 1;
  bmapheight = ((maxy-miny) >> MAPBTOFRAC) + 1;

  // Compute blockmap, which is stored as a 2d array of variable-sized lists.
  //
  // Pseudocode:
  //
  // For each linedef:
  //
  //   Map the starting and ending vertices to blocks.
  //
  //   Starting in the starting vertex's block, do:
  //
  //     Add linedef to current block's list, dynamically resizing it.
  //
  //     If current block is the same as the ending vertex's block, exit loop.
  //
  //     Move to an adjacent block by moving towards the ending block in 
  //     either the x or y direction, to the block which contains the linedef.

  {
    typedef struct { int n, nalloc, *list; } bmap_t;  // blocklist structure
    unsigned tot = bmapwidth * bmapheight;            // size of blockmap
    bmap_t *bmap = calloc(sizeof *bmap, tot);         // array of blocklists

    for (i=0; (signed)i < numlines; i++) // Tails
      {
	// starting coordinates
	int x = (lines[i].v1->x >> FRACBITS) - minx;
	int y = (lines[i].v1->y >> FRACBITS) - miny;
	
	// x-y deltas
	int adx = lines[i].dx >> FRACBITS, dx = adx < 0 ? -1 : 1;
	int ady = lines[i].dy >> FRACBITS, dy = ady < 0 ? -1 : 1; 

	// difference in preferring to move across y (>0) instead of x (<0)
	int diff = !adx ? 1 : !ady ? -1 :
	  (((x >> MAPBTOFRAC) << MAPBTOFRAC) + 
	   (dx > 0 ? MAPBLOCKUNITS-1 : 0) - x) * (ady = abs(ady)) * dx -
	  (((y >> MAPBTOFRAC) << MAPBTOFRAC) + 
	   (dy > 0 ? MAPBLOCKUNITS-1 : 0) - y) * (adx = abs(adx)) * dy;

	// starting block, and pointer to its blocklist structure
	int b = (y >> MAPBTOFRAC)*bmapwidth + (x >> MAPBTOFRAC);

	// ending block
	int bend = (((lines[i].v2->y >> FRACBITS) - miny) >> MAPBTOFRAC) *
	  bmapwidth + (((lines[i].v2->x >> FRACBITS) - minx) >> MAPBTOFRAC);

	// delta for pointer when moving across y
	dy *= bmapwidth;

	// deltas for diff inside the loop
	adx <<= MAPBTOFRAC;
	ady <<= MAPBTOFRAC;

	// Now we simply iterate block-by-block until we reach the end block.
	while ((unsigned) b < tot)    // failsafe -- should ALWAYS be true
	  {
	    // Increase size of allocated list if necessary
	    if (bmap[b].n >= bmap[b].nalloc)
	      bmap[b].list = realloc(bmap[b].list, 
				     (bmap[b].nalloc = bmap[b].nalloc ? 
				      bmap[b].nalloc*2 : 8)*sizeof*bmap->list);

	    // Add linedef to end of list
	    bmap[b].list[bmap[b].n++] = i;

	    // If we have reached the last block, exit
	    if (b == bend)
	      break;

	    // Move in either the x or y direction to the next block
	    if (diff < 0) 
	      diff += ady, b += dx;
	    else
	      diff -= adx, b += dy;
	  }
      }

    // Compute the total size of the blockmap.
    //
    // Compression of empty blocks is performed by reserving two offset words
    // at tot and tot+1.
    //
    // 4 words, unused if this routine is called, are reserved at the start.

    {
      int count = tot+6;  // we need at least 1 word per block, plus reserved's

      for (i = 0; i < tot; i++)
	if (bmap[i].n)
	  count += bmap[i].n + 2; // 1 header word + 1 trailer word + blocklist

      // Allocate blockmap lump with computed count
      blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);
    }									 

    // Now compress the blockmap.
    {
      int ndx = tot += 4;         // Advance index to start of linedef lists
      bmap_t *bp = bmap;          // Start of uncompressed blockmap

      blockmaplump[ndx++] = 0;    // Store an empty blockmap list at start
      blockmaplump[ndx++] = -1;   // (Used for compression)

      for (i = 4; i < tot; i++, bp++)
	if (bp->n)                                      // Non-empty blocklist
	  {
	    blockmaplump[blockmaplump[i] = ndx++] = 0;  // Store index & header
	    do
	      blockmaplump[ndx++] = bp->list[--bp->n];  // Copy linedef list
	    while (bp->n);
	    blockmaplump[ndx++] = -1;                   // Store trailer
	    free(bp->list);                             // Free linedef list
	  }
	else            // Empty blocklist: point to reserved empty blocklist
	  blockmaplump[i] = tot;

      free(bmap);    // Free uncompressed blockmap
    }
  }
}

//
// P_LoadBlockMap
//
void P_LoadBlockMap (int lump)
{
  long count;

  count = W_LumpLength(lump)/2;

    {
      long i;
      short *wadblockmaplump = W_CacheLumpNum (lump, PU_LEVEL);
      blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);

      // killough 3/1/98: Expand wad blockmap into larger internal one,
      // by treating all offsets except -1 as unsigned and zero-extending
      // them. This potentially doubles the size of blockmaps allowed,
      // because Doom originally considered the offsets as always signed.

      blockmaplump[0] = SHORT(wadblockmaplump[0]);
      blockmaplump[1] = SHORT(wadblockmaplump[1]);
      blockmaplump[2] = (long)(SHORT(wadblockmaplump[2])) & 0xffff;
      blockmaplump[3] = (long)(SHORT(wadblockmaplump[3])) & 0xffff;

      for (i=4 ; i<count ; i++)
        {
          short t = SHORT(wadblockmaplump[i]);          // killough 3/1/98
          blockmaplump[i] = t == -1 ? -1l : (long) t & 0xffff;
        }

      Z_Free(wadblockmaplump);

      bmaporgx = blockmaplump[0]<<FRACBITS;
      bmaporgy = blockmaplump[1]<<FRACBITS;
      bmapwidth = blockmaplump[2];
      bmapheight = blockmaplump[3];
    }

  // clear out mobj chains
  count = sizeof(*blocklinks)* bmapwidth*bmapheight;
  blocklinks = Z_Malloc (count,PU_LEVEL, 0);
  memset (blocklinks, 0, count);
  blockmap = blockmaplump+4;

/* Original
		blockmaplump = W_CacheLumpNum (lump,PU_LEVEL);
		blockmap = blockmaplump+4;
		count = W_LumpLength (lump)/2;

		for (i=0 ; i<count ; i++)
			blockmaplump[i] = SHORT(blockmaplump[i]);

		bmaporgx = blockmaplump[0]<<FRACBITS;
		bmaporgy = blockmaplump[1]<<FRACBITS;
		bmapwidth = blockmaplump[2];
		bmapheight = blockmaplump[3];
	}

	// clear out mobj chains
	count = sizeof(*blocklinks)*bmapwidth*bmapheight;
	blocklinks = Z_Malloc (count,PU_LEVEL, 0);
	memset (blocklinks, 0, count);*/
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


// SoM: 6/27: Don't restrict maps to MAPxx/ExMx any more!
char *levellumps[] =
{
  "label",        // ML_LABEL,    A separator, name, ExMx or MAPxx
  "THINGS",       // ML_THINGS,   Monsters, items..
  "LINEDEFS",     // ML_LINEDEFS, LineDefs, from editing
  "SIDEDEFS",     // ML_SIDEDEFS, SideDefs, from editing
  "VERTEXES",     // ML_VERTEXES, Vertices, edited and BSP splits generated
  "SEGS",         // ML_SEGS,     LineSegs, from LineDefs split by BSP
  "SSECTORS",     // ML_SSECTORS, SubSectors, list of LineSegs
  "NODES",        // ML_NODES,    BSP nodes
  "SECTORS",      // ML_SECTORS,  Sectors, from editing
  "REJECT",       // ML_REJECT,   LUT, sector-sector visibility
  "BLOCKMAP"      // ML_BLOCKMAP  LUT, motion clipping, walls/grid element
};


//
// P_CheckLevel
// Checks a lump and returns weather or not it is a level header lump.
boolean P_CheckLevel(int lumpnum)
{
  int  i;
  int  file, lump;
  
  for(i=ML_THINGS; i<=ML_BLOCKMAP; i++)
    {
      file = lumpnum >> 16;
      lump = (lumpnum & 0xffff) + i;
      if(file > numwadfiles || lump > wadfiles[file]->numlumps ||
         strncmp(wadfiles[file]->lumpinfo[lump].name, levellumps[i], 8) )
        return false;
    }
  return true;    // all right
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

    sprintf (skytexname,"SKY%d",mapheaderinfo[gamemap-1].skynum);
    skytexture = R_TextureNumForName (skytexname);

    // scale up the old skies, if needed
    R_SetupSkyDraw ();
}

void P_SpawnPrecipitation(); // Tails 12-09-2002
void P_ResetPlayer(player_t* player);
//
// P_SetupLevel
//
// added comment : load the level from a lump file or from a external wad !
extern int numtextures;
char       *maplumpname;
extern consvar_t cv_runscripts; // Graue 12-13-2003
extern boolean circintrodone; // Graue 12-24-2003

int        lastloadedmaplumpnum; // for comparative savegame
boolean P_SetupLevel (int           episode,
                      int           map,
                      skill_t       skill,
                      char*         wadname)      // for wad files
{
    int         i;

    CON_Drawer ();  // let the user know what we are going to do
    I_FinishUpdate ();              // page flip or blit buffer

    //Initialize sector node list.
    P_Initsecnode();

    totalkills = totalitems = totalsecret = totalrings = wminfo.maxfrags = 0; // Tails 08-11-2001

    wminfo.partime = 180;

	if(cv_runscripts.value /* Graue 12-13-2003 */ && mapheaderinfo[gamemap-1].scriptname[0] != '#')
	{
		if(mapheaderinfo[gamemap-1].scriptislump)
		{
			int     lumpnum;
			char newname[9];

			strncpy(newname, mapheaderinfo[gamemap-1].scriptname, 8);

			newname[8] = '\0';

			lumpnum = W_CheckNumForName(newname);

			if(lumpnum == -1 || W_LumpLength(lumpnum) <= 0) // Bugfix by Graue 11-14-2003
			{
				CONS_Printf("SOC Error: script lump %s not found/not valid.\n", newname);
				goto noscript;
			}

			COM_BufInsertText (W_CacheLumpNum(lumpnum, PU_CACHE));
		}
		else
			COM_BufAddText(va("exec %s\n", mapheaderinfo[gamemap-1].scriptname));
	}
noscript:
	numstarposts = 0; // Graue 11-17-2003
	bitstarposts = 0; // Graue 11-18-2003

    for (i=0 ; i<MAXPLAYERS ; i++)
    {
		if(netgame || multiplayer)
		{
			// In Co-Op, replenish a user's continues and lives if they are depleted.
			if(players[i].lives <= 0)
			{
				switch(gameskill)
				{
					case sk_insane:
						players[i].lives = 1;
						break;
					case sk_nightmare:
					case sk_hard:
					case sk_medium:
						players[i].lives = 3;
						break;
					case sk_easy:
						players[i].lives = 5;
						break;
					case sk_baby:
						players[i].lives = 9;
						break;
					default: // Oops!?
						CONS_Printf("ERROR: GAME SKILL UNDETERMINED!");
						break;
				}
			}

			if(players[i].continues <= 0)
			{
				switch(gameskill)
				{
					case sk_insane:
						players[i].continues = 0;
						break;
					case sk_nightmare:
					case sk_hard:
					case sk_medium:
						players[i].continues = 1;
						break;
					case sk_easy:
						players[i].continues = 2;
						break;
					case sk_baby:
						players[i].continues = 5;
						break;
					default: // Oops!?
						CONS_Printf("ERROR: GAME SKILL UNDETERMINED!");
						break;
				}
			}
		}

        players[i].killcount = players[i].secretcount
            = players[i].itemcount = 0;

        players[i].timebonus = players[i].ringbonus = countdown = countdown2 = 0; // Reset the bonus counters Tails 03-10-2000

		players[i].mare = 0;

        players[i].health = 1; // Alright, who added this and didn't comment it? Tails 03-10-2000

        players[i].xtralife = 0; // Tails 04-25-2000

		players[i].deadtimer = 0; // Tails 06-05-2002

		if(cv_gametype.value == GT_RACE) // Tails 04-25-2001
		{
			if(players[i].lives < 3)
				players[i].lives = 3;
		}
		else if (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_CHAOS)
			players[i].score = 0;

        players[i].seconds = 0; // Tails 06-06-2000
        players[i].minutes = 0; // Tails 06-06-2000
		players[i].exiting = 0; // Tails 12-20-2000

		if(cv_gametype.value == GT_CIRCUIT)
		{
			players[i].laps = 0; // Graue 11-15-2003
			players[i].exiting = 5*CIRCINTROTIME; // Graue 12-24-2003
		}

		P_ResetPlayer(&players[i]);

		hunt1 = hunt2 = hunt3 = NULL;

		players[i].mo = NULL;
#ifdef CLIENTPREDICTION2
        players[i].spirit = NULL;
#endif
    }

	if(mapheaderinfo[gamemap-1].forcecharacter != 255)
	{
		char skincmd[33];
		if(cv_splitscreen.value)
		{
			sprintf(skincmd, "skin2 %s\n", skins[mapheaderinfo[gamemap-1].forcecharacter].name);
			CV_Set(&cv_skin2, skins[mapheaderinfo[gamemap-1].forcecharacter].name);
		}

		sprintf(skincmd, "skin %s\n", skins[mapheaderinfo[gamemap-1].forcecharacter].name);

		COM_BufAddText(skincmd);

		if(!netgame)
		{
			if(cv_splitscreen.value)
				SetPlayerSkinByNum(secondarydisplayplayer, mapheaderinfo[gamemap-1].forcecharacter);

			SetPlayerSkinByNum(consoleplayer, mapheaderinfo[gamemap-1].forcecharacter);
		}
	}

	if(cv_gametype.value != GT_MATCH && cv_gametype.value != GT_TAG && cv_gametype.value != GT_CTF) // Allow 1st person in Match, Tag, and CTF. Tails 02-26-2002
	{
		CV_SetValue(&cv_chasecam, true);
		if(cv_splitscreen.value)
			CV_SetValue(&cv_chasecam2, true);
	}

	// Graue 12-24-2003
	if(cv_gametype.value == GT_CIRCUIT)
		circintrodone = false;
	else
		circintrodone = true;

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_MARIO)
		mariomode = true;
	else
		mariomode = false;

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_2D)
		twodlevel = true;
	else
		twodlevel = false;

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_XMAS)
		xmasmode = true;
	else if(!xmasoverride)
		xmasmode = false;

    // Initial height of PointOfView
    // will be set by player think.

    players[consoleplayer].viewz = 1;

    // Make sure all sounds are stopped before Z_FreeTags.
    S_StopSounds();


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

    HU_ClearTips();
/*
    if (camera.chase)
        camera.mo = NULL;
*/
    // UNUSED W_Profile ();
    
    P_InitThinkers ();

    // if working with a devlopment map, reload it
    W_Reload ();

    //
    //  load the map from internal game resource or external wad file
    //
    if (wadname)
    {
        char *firstmap=NULL;

        // go back to title screen if no map is loaded
        if (!P_AddWadFile (wadname,&firstmap) ||
            firstmap==NULL)            // no maps were found
        {
            return false;
        }

        // P_AddWadFile() sets lumpname
        lastloadedmaplumpnum = W_GetNumForName(firstmap);
        maplumpname = firstmap;
    }
    else
    {
        // internal game map
        lastloadedmaplumpnum = W_GetNumForName (maplumpname = G_BuildMapName(map));
    }

    if(levelmapname) Z_Free(levelmapname);
    levelmapname = Z_Strdup(maplumpname, PU_STATIC, 0);

    leveltime = 0;

	if(mapheaderinfo[gamemap-1].countdown)
		countdowntimer = mapheaderinfo[gamemap-1].countdown*TICRATE;

	countdowntimeup = false; // DuuuuuuuuuhhhH!H!H!!

    // textures are needed first
//    R_LoadTextures ();
//    R_FlushTextureCache();

    R_ClearColormaps();
#ifdef FRAGGLESCRIPT
    P_LoadLevelInfo (lastloadedmaplumpnum);    // load level lump info(level name etc)
#endif

    //SoM: We've loaded the music lump, start the music.
    S_Start();

    //faB: now part of level loading since in future each level may have
    //     its own anim texture sequences, switches etc.
    P_InitSwitchList ();
    P_InitPicAnims ();
//    P_InitLava ();
    P_SetupLevelSky ();

    // SoM: WOO HOO!
    // SoM: DOH!
    //R_InitPortals ();

    // note: most of this ordering is important
    P_LoadBlockMap (lastloadedmaplumpnum+ML_BLOCKMAP);
    P_LoadVertexes (lastloadedmaplumpnum+ML_VERTEXES);
    P_LoadSectors  (lastloadedmaplumpnum+ML_SECTORS);
    P_LoadSideDefs (lastloadedmaplumpnum+ML_SIDEDEFS);

    P_LoadLineDefs (lastloadedmaplumpnum+ML_LINEDEFS);
    P_LoadSideDefs2(lastloadedmaplumpnum+ML_SIDEDEFS);
	R_MakeColormaps(); // Tails 12-09-2002
    P_LoadLineDefs2();
    P_LoadSubsectors (lastloadedmaplumpnum+ML_SSECTORS);
    P_LoadNodes (lastloadedmaplumpnum+ML_NODES);
    P_LoadSegs (lastloadedmaplumpnum+ML_SEGS);
    rejectmatrix = W_CacheLumpNum (lastloadedmaplumpnum+ML_REJECT,PU_LEVEL);
    P_GroupLines ();

	P_SetSlopes(); // Tails 05-01-2002

#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
    {
        // BP: reset light between levels (we draw preview frame lights on current frame)
        HWR_ResetLights();
        // Correct missing sidedefs & deep water trick
        HWR_CorrectSWTricks();
        HWR_CreatePlanePolygons (numnodes-1);
    }
#endif

    numdmstarts = 0;
    numredctfstarts = 0; // Tails
    numbluectfstarts = 0; // Tails
    // added 25-4-98 : reset the players starts
    //SoM: Set pointers to NULL
    for(i=0;i<MAXPLAYERS;i++)
       playerstarts[i] = NULL;

//    P_InitAmbientSound ();
//    P_InitMonsters ();
//    P_OpenWeapons ();
    P_LoadThings (lastloadedmaplumpnum+ML_THINGS);

	// Graue 12-23-2003
	for(numcoopstarts=0;numcoopstarts<MAXPLAYERS;numcoopstarts++)
		if(!playerstarts[numcoopstarts])
			break;

//  P_CloseWeapons ();

    // set up world state
    P_SpawnSpecials ();

	if(!netgame)
		P_SpawnPrecipitation(); // Tails 12-09-2002
// FIXTHIS: precipitation should work in netgames too! Graue 12-22-2003

    //BP: spawnplayers now (beffor all structure are not inititialized)
    for (i=0 ; i<MAXPLAYERS ; i++)
        if (playeringame[i])
        {
            if (cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG || cv_gametype.value == GT_CHAOS)
            {
                players[i].mo = NULL;
                G_DoReborn(i);
            }
			else // cv_gametype.value is GT_COOP, GT_RACE, or GT_CIRCUIT Graue 12-22-2003
            {
                players[i].mo = NULL;

				// Tails 07-04-2002-
				if(players[i].starposttime)
				{
					G_CoopSpawnPlayer(i, true);
					P_ClearStarPost(&players[i], players[i].starpostnum+1);
				}
				else
					G_CoopSpawnPlayer (i, false);
            }
        }

	camera.x = players[displayplayer].mo->x;
	camera.y = players[displayplayer].mo->y;
	camera.z = players[displayplayer].mo->z;
	camera.angle = players[displayplayer].mo->angle;

	CV_Set(&cv_cam_height, cv_cam_height.defaultvalue);
	CV_Set(&cv_cam_dist, cv_cam_dist.defaultvalue);

	if(server)
	{
		CV_SetValue(&cv_homing, false);
		CV_SetValue(&cv_lightdash, false);
	}

	if(mapheaderinfo[gamemap-1].typeoflevel & TOL_ADVENTURE)
	{
		CV_SetValue(&cv_cam_height, 64);
		CV_SetValue(&cv_cam_dist, 192);

		if(server)
		{
			CV_SetValue(&cv_homing, true);
			CV_SetValue(&cv_lightdash, true);
		}

		if(!netgame)
		{
			if(cv_splitscreen.value)
				CV_SetValue(&cv_analog2, true);

			CV_SetValue(&cv_analog, true);
		}
	}

	if(twodlevel)
	{
		CV_SetValue(&cv_cam_dist, 320);
		CV_SetValue(&cv_analog2, false);
		CV_SetValue(&cv_analog, false);
	}

    // clear special respawning que
    iquehead = iquetail = 0;

    // build subsector connect matrix
    //  UNUSED P_ConnectSubsectors ();

    //Fab:19-07-98:start cd music for this level (note: can be remapped)
    I_PlayCD (map + 1, false);  // Tails

    // preload graphics
#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
    {
        HWR_PrepLevelCache (numtextures);
        HWR_CreateStaticLightmaps (numnodes-1);
    }
#endif

    if (precache)
        R_PrecacheLevel ();

	nextmapoverride = 0;
	skipstats = false;

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
        CONS_Printf ("couldn't load wad file %s\n", wadfilename);
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

                    I_FreeSfx (&S_sfx[j]);

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
        CONS_Printf ("%d sounds replaced\n", replaces);

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
        CONS_Printf ("%d musics replaced\n", replaces);

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

        if (name[0]=='M' &&
            name[1]=='A' &&
            name[2]=='P') // Ignore the headers Tails 05-06-2003
        {
			num = mapnumber(name[3], name[4]);

			if(name[5]=='N' || name[5]=='D')
				P_LoadMapHeader(num); // Tails 04-09-2003
			else if(name[5]!='\0') // Graue 11-08-2003
				continue;

			CONS_Printf ("%s\n", name);
        }

        if (num && (num<firstmapreplaced || !firstmapreplaced))
        {
            firstmapreplaced = num;
            if(firstmapname) *firstmapname = name;
        }
    }
    if (!firstmapreplaced)
        CONS_Printf ("no maps added\n");

    // reload status bar (warning should have valide player !)
    if( gamestate == GS_LEVEL )
        ST_Start();

    return true;
}
