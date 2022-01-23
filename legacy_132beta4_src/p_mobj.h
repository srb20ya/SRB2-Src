// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_mobj.h,v 1.8 2001/11/17 22:12:53 hurdler Exp $
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
// $Log: p_mobj.h,v $
// Revision 1.8  2001/11/17 22:12:53  hurdler
// Ready to work on beta 4 ;)
//
// Revision 1.7  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.6  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.5  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.4  2000/04/30 10:30:10  bpereira
// no message
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
//      Map Objects, MObj, definition and handling.
//
//-----------------------------------------------------------------------------


#ifndef __P_MOBJ__
#define __P_MOBJ__


// Basics.
#include "tables.h"
#include "m_fixed.h"

// We need the thinker_t stuff.
#include "d_think.h"

// We need the WAD data structure for Map things,
// from the THINGS lump.
#include "doomdata.h"

// States are tied to finite states are
//  tied to animation frames.
// Needs precompiled tables/data structures.
#include "info.h"



//
// NOTES: mobj_t
//
// mobj_ts are used to tell the refresh where to draw an image,
// tell the world simulation when objects are contacted,
// and tell the sound driver how to position a sound.
//
// The refresh uses the next and prev links to follow
// lists of things in sectors as they are being drawn.
// The sprite, frame, and angle elements determine which patch_t
// is used to draw the sprite if it is visible.
// The sprite and frame values are allmost allways set
// from state_t structures.
// The statescr.exe utility generates the states.h and states.c
// files that contain the sprite/frame numbers from the
// statescr.txt source file.
// The xyz origin point represents a point at the bottom middle
// of the sprite (between the feet of a biped).
// This is the default origin position for patch_ts grabbed
// with lumpy.exe.
// A walking creature will have its z equal to the floor
// it is standing on.
//
// The sound code uses the x,y, and subsector fields
// to do stereo positioning of any sound effited by the mobj_t.
//
// The play simulation uses the blocklinks, x,y,z, radius, height
// to determine when mobj_ts are touching each other,
// touching lines in the map, or hit by trace lines (gunshots,
// lines of sight, etc).
// The mobj_t->flags element has various bit flags
// used by the simulation.
//
// Every mobj_t is linked into a single sector
// based on its origin coordinates.
// The subsector_t is found with R_PointInSubsector(x,y),
// and the sector_t can be found with subsector->sector.
// The sector links are only used by the rendering code,
// the play simulation does not care about them at all.
//
// Any mobj_t that needs to be acted upon by something else
// in the play world (block movement, be shot, etc) will also
// need to be linked into the blockmap.
// If the thing has the MF_NOBLOCK flag set, it will not use
// the block links. It can still interact with other things,
// but only as the instigator (missiles will run into other
// things, but nothing can run into a missile).
// Each block in the grid is 128*128 units, and knows about
// every line_t that it contains a piece of, and every
// interactable mobj_t that has its origin contained.
//
// A valid mobj_t is a mobj_t that has the proper subsector_t
// filled in for its xy coordinates and is linked into the
// sector from which the subsector was made, or has the
// MF_NOSECTOR flag set (the subsector_t needs to be valid
// even if MF_NOSECTOR is set), and is linked into a blockmap
// block or has the MF_NOBLOCKMAP flag set.
// Links should only be modified by the P_[Un]SetThingPosition()
// functions.
// Do not change the MF_NO? flags while a thing is valid.
//
// Any questions?
//

//
// Misc. mobj flags
//
typedef enum
{
    // Call P_SpecialThing when touched.
    MF_SPECIAL          = 0x0001,
    // Blocks.
    MF_SOLID            = 0x0002,
    // Can be hit.
    MF_SHOOTABLE        = 0x0004,
    // Don't use the sector links (invisible but touchable).
    MF_NOSECTOR         = 0x0008,
    // Don't use the blocklinks (inert but displayable)
    MF_NOBLOCKMAP       = 0x0010,

    // Not to be activated by sound, deaf monster.
    MF_AMBUSH           = 0x0020,
    // You can push this object. It can activate switches and things by pushing it on top.
    MF_PUSHABLE          = 0x0040,
    // Object is a boss.
    MF_BOSS     = 0x0080,
    // On level spawning (initial position),
    //  hang from ceiling instead of stand on floor.
    MF_SPAWNCEILING     = 0x0100,
    // Don't apply gravity (every tic),
    //  that is, object will float, keeping current height
    //  or changing it actively.
    MF_NOGRAVITY        = 0x0200,

    // This object is an ambient sound.
    MF_AMBIENT          = 0x0400,
    // Slide this object when it hits a wall. Tails 12-28-2003
    MF_SLIDEME          = 0x0800,
    // Player cheat. ???
    MF_NOCLIP           = 0x1000,
    // This object does not adhere to regular flag/z properties for object placing.
    MF_SPECIALFLAGS            = 0x2000,
    // Allow moves to any height, no gravity.
    // For active floaters, e.g. cacodemons, pain elementals.
    MF_FLOAT            = 0x4000,
    // Monitor powerup icon. These rise a bit.
    MF_MONITORICON      = 0x8000,
    // Don't hit same species, explode on block.
    // Player missiles as well as fireballs of various kinds.
    MF_MISSILE          = 0x10000,
    
	// Item is a spring.
    MF_SPRING          = 0x20000,

	// bounce off walls and things Graue 12-31-2003
	MF_BOUNCE           = 0x40000,

    // Object uses a high-resolution sprite
	// Tails 01-23-2003
    MF_HIRES          = 0x80000,

    // This object is an item box! Tails 01-08-2002
    MF_MONITOR           = 0x100000,
    // Floating to a height for a move, ???
    //  don't auto float to target's height.
    MF_INFLOAT          = 0x200000,

    // On kill, count this enemy object
    //  towards intermission kill total.
    // Happy gathering.
    MF_COUNTKILL        = 0x400000,

    // On picking up, count this item object
    //  towards intermission item total.
    MF_COUNTITEM        = 0x800000,

	// This mobj is an enemy! Tails 12-27-2003
	MF_ENEMY            = 0x1000000,

    // Use the scenery thinker instead if it is just scenery.
	// Tails 07-24-2002
    MF_SCENERY        = 0x2000000,

    // Player sprites in multiplayer modes are modified
    //  using an internal color lookup table for re-indexing.
    // If 0x4 0x8 or 0xc,
    //  use a translation table for player colormaps
    MF_TRANSLATION      = 0x3C000000,    // 0xc000000, original 4color

    // Hmm ???.
    MF_TRANSSHIFT       = 26,

    // for chase camera, don't be blocked by things (partial clipping)
    MF_NOCLIPTHING      = 0x40000000,

    MF_FIRE           = 0x80000000 // This is a fire object (doesn't harm if you have red shield).

} mobjflag_t;

//Tails

typedef enum {
    MF2_LOGRAV         =     0x00000001,      // alternate gravity setting
    MF2_WINDTHRUST     =     0x00000002,      // gets pushed around by the wind
                                              // specials
    MF2_DONTRESPAWN    =     0x00000004,      // Don't respawn this object!
    MF2_RINGORHOOP     =     0x00000008,      // Ring or Hoop object
    MF2_FRET            =    0x00000010,      // Flashing from a previous hit
    MF2_NOTHINK        =     0x00000020,      // Don't run this mobj's thinker
    MF2_DEBRIS         =     0x00000040,      // Splash ring from explosion ring
    MF2_NOTELEPORT     =     0x00000080,      // does not teleport
    MF2_NIGHTSPULL     =     0x00000100,      // Attracted from a paraloop
    MF2_JUSTATTACKED   =     0x00000200,      // can be pushed by other moving
                                              // mobjs
    MF2_FIRING          =     0x00000400,     // turret fire
    MF2_ONMOBJ         =     0x00000800,      // mobj is resting on top of another
                                              // mobj
    // Special handling: skull in flight.
    // Neither a cacodemon nor a missile.
    MF2_SKULLFLY       =     0x00001000,

    MF2_CHAOSBOSS      =     0x00002000,      // Boss has been spawned in chaos mode
    MF2_BOSSNOTRAP     =     0x00004000,      // No Egg Trap after boss Tails 12-05-2002
    MF2_BOSSFLEE       =     0x00008000,      // Boss is fleeing! Tails 12-05-2002
    MF2_AUTOMATIC      =     0x00010000,      // Thrown ring has automatic fire
    MF2_RAILRING       =     0x00020000,      // Thrown ring has rail capabilities     
    MF2_HOMING         =     0x00040000,      // Thrown ring has homing capabilities
	MF2_EXPLOSION      =     0x00080000,      // Thrown ring has explosive properties
    MF2_DONTDRAW       =     0X00100000,      // don't generate a vissprite
	MF2_SLIDEPUSH      =     0x00200000,      // special type of MF_PUSHABLE Graue 12-31-2003
    // DOOM2: Use fuzzy draw (shadow demons or spectres),
    //  temporary player invisibility powerup.
    // LEGACY: no more for translucency, but still makes targeting harder
    MF2_SHADOW         =     0x00400000,
	MF2_STANDONME      =     0x00800000, // While not pushable, stand on me anyway.
} mobjflag2_t;

//
//  New mobj extra flags
//
//added:28-02-98:
typedef enum
{
    // The mobj stands on solid floor (not on another mobj or in air)
    MF_ONGROUND          = 1,
    // The mobj just hit the floor while falling, this is cleared on next frame
    // (instant damage in lava/slime sectors to prevent jump cheat..)
    MF_JUSTHITFLOOR      = 2,
    // The mobj stands in a sector with water, and touches the surface
    // this bit is set once and for all at the start of mobjthinker
    MF_TOUCHWATER        = 4,
    // The mobj stands in a sector with water, and his waist is BELOW the water surface
    // (for player, allows swimming up/down)
    MF_UNDERWATER        = 8,
    // Set by P_MovePlayer() to disable gravity add in P_MobjThinker() ( for gameplay )
    MF_SWIMMING          = 16,
    // used for client prediction code, player can't be blocked in z by walls
    // it is set temporarely when player follow the spirit
    MF_NOZCHECKING       = 32,
} mobjeflag_t;


#if MAXSKINCOLORS > 16

MAXSKINCOLOR have changed
Change MF_TRANSLATION to take effect of the change

#endif

// Map Object definition.
typedef struct mobj_s
{
    // List: thinker links.
    thinker_t           thinker;

    // Info for drawing: position.
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    // More list: links in sector (if needed)
    struct mobj_s*      snext;
    struct mobj_s*      sprev;

    //More drawing info: to determine current sprite.
    angle_t             angle;  // orientation
    spritenum_t         sprite; // used to find patch_t and flip value
    int                 frame;  // frame number, plus bits see p_pspr.h

    //Fab:02-08-98
    void*               skin;      // this one overrides 'sprite' when
                                   // non NULL (currently hack for player
                                   // bodies so they 'remember' the skin)
                                   //
                                   // secondary used when player die and
                                   // play the die sound problem is he is
                                   // already respawn and the corps play
                                   // the sound !!! (he yeah it happens :\)

    // Interaction info, by BLOCKMAP.
    // Links in blocks (if needed).
    struct mobj_s*      bnext;
    struct mobj_s*      bprev;

    struct subsector_s* subsector;

    // The closest interval over all contacted Sectors (or Things).
    fixed_t             floorz;
    fixed_t             ceilingz;

    // For movement checking.
    fixed_t             radius;
    fixed_t             height;

    // Momentums, used to update position.
    fixed_t             momx;
    fixed_t             momy;
    fixed_t             momz;

    // If == validcount, already checked.
    //int                 validcount;

    mobjtype_t          type;
    mobjinfo_t*         info;   // &mobjinfo[mobj->type]

    int                 tics;   // state tic counter
    state_t*            state;
    int                 flags;
    int                 eflags; //added:28-02-98: extra flags see above
    int                 flags2; // heretic stuff
    int                 health;

    // Movement direction, movement generation (zig-zagging).
    int                 movedir;        // 0-7
    int                 movecount;      // when 0, select a new dir

    // Thing being chased/attacked (or NULL),
    // also the originator for missiles.
    struct mobj_s*      target;

    // Reaction time: if non 0, don't attack yet.
    // Used by player to freeze a bit after teleporting.
    int                 reactiontime;

    // If >0, the target will be chased
    // no matter what (even if shot)
    int                 threshold;

    // Additional info record for player avatars only.
    // Only valid if type == MT_PLAYER
    struct player_s*    player;

    // Player number last looked for.
    int                 lastlook;

    // For nightmare and itemrespawn respawn.
    mapthing_t          *spawnpoint;

    // Thing being chased/attacked for tracers.
    struct mobj_s*      tracer;

    //SoM: Friction.
    int friction;
    int movefactor;

    // a linked list of sectors where this object appears
    struct msecnode_s* touching_sectorlist;

    // WARNING : new fields are not automatically added to save game 
    //SOM: Added fuse
    int                 fuse;
    int			watertop; // Water height Tails
    int			waterbottom; // Water height Tails

	int mobjnum; // A unique number for this mobj. Used for restoring pointers on save games.
} mobj_t;

// More crazy crap Tails 08-25-2002
typedef struct precipmobj_s
{
    // List: thinker links.
    thinker_t           thinker;

    state_t*            state;

    // More list: links in sector (if needed)
    struct precipmobj_s*      snext;
    struct precipmobj_s*      sprev;

	// a linked list of sectors where this object appears
    struct mprecipsecnode_s* touching_sectorlist;

    struct subsector_s* subsector;

    //More drawing info: to determine current sprite.
    spritenum_t         sprite; // used to find patch_t and flip value

    // Info for drawing: position.
	fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    // The closest interval over all contacted Sectors (or Things).
    fixed_t             floorz;

	// Precipitation has a fixed radius of 2*FRACUNIT
	// and a fixed height of 4*FRACUNIT

    // Momentums, used to update position.
    fixed_t             momz;

    int                 frame;  // frame number, plus bits see p_pspr.h
    int                 tics;   // state tic counter
    int                 flags;
} precipmobj_t;

// check mobj against water content, before movement code
void P_MobjCheckWater (mobj_t* mobj);
boolean P_MobjCheckOldPosWater (mobj_t* mobj); // Tails 10-01-2002

void P_SpawnMapThing (mapthing_t*    mthing);
void P_SpawnPlayer (mapthing_t* mthing, int playernum); // Added playernum arg Graue 12-23-2003
void P_SpawnStarpostPlayer (mobj_t* mobj, int playernum);

#endif
