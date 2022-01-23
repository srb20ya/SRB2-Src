// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_local.h,v 1.20 2001/08/06 23:57:09 stroggonmeth Exp $
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
// $Log: p_local.h,v $
// Revision 1.20  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.19  2001/07/28 16:18:37  bpereira
// no message
//
// Revision 1.18  2001/07/16 22:35:41  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.17  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.16  2001/03/21 18:24:38  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.15  2001/02/10 12:27:14  bpereira
// no message
//
// Revision 1.14  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.13  2000/11/03 02:37:36  stroggonmeth
// Fix a few warnings when compiling.
//
// Revision 1.12  2000/11/02 19:49:35  bpereira
// no message
//
// Revision 1.11  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.10  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.9  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.8  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.7  2000/04/13 23:47:47  stroggonmeth
// See logs
//
// Revision 1.6  2000/04/11 19:07:24  stroggonmeth
// Finished my logs, fixed a crashing bug.
//
// Revision 1.5  2000/04/08 17:29:24  stroggonmeth
// no message
//
// Revision 1.4  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
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
//      Play functions, animation, global header.
//
//-----------------------------------------------------------------------------


#ifndef __P_LOCAL__
#define __P_LOCAL__

#include "command.h"
#include "d_player.h"
#include "d_think.h"
#include "m_fixed.h"
#include "m_bbox.h"
#include "p_tick.h"
#include "r_defs.h"
#include "p_maputl.h"

#define FLOATSPEED              (FRACUNIT*4)

#define VIEWHEIGHT               41
#define VIEWHEIGHTS             "41"

// default viewheight is changeable at console
extern consvar_t cv_viewheight; // p_mobj.c

// mapblocks are used to check movement
// against lines and things
#define MAPBLOCKUNITS   128
#define MAPBLOCKSIZE    (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT   (FRACBITS+7)
#define MAPBMASK        (MAPBLOCKSIZE-1)
#define MAPBTOFRAC      (MAPBLOCKSHIFT-FRACBITS)


// player radius used only in am_map.c
#define PLAYERRADIUS    (16*FRACUNIT)

// MAXRADIUS is for precalculated sector block boxes
// the spider demon is larger,
// but we do not have any moving sectors nearby
#define MAXRADIUS       (32*FRACUNIT)

#define MAXMOVE         (60*FRACUNIT/NEWTICRATERATIO) // Tails

//added:26-02-98: max Z move up or down without jumping
//      above this, a heigth difference is considered as a 'dropoff'
#define MAXSTEPMOVE     (24*FRACUNIT)

#define USERANGE        (64*FRACUNIT)
#define MELEERANGE      (64*FRACUNIT)
#define MISSILERANGE    (32*64*FRACUNIT)

// follow a player exlusively for 3 seconds
#define BASETHRESHOLD   100

//#define AIMINGTOSLOPE(aiming)   finetangent[(2048+(aiming>>ANGLETOFINESHIFT)) & FINEMASK]
#define AIMINGTOSLOPE(aiming)   finesine[(aiming>>ANGLETOFINESHIFT) & FINEMASK]

//26-07-98: p_mobj.c
extern  consvar_t cv_gravity;

//
// P_TICK
//

// both the head and tail of the thinker list
extern  thinker_t       thinkercap;


void P_InitThinkers (void);
void P_AddThinker (thinker_t* thinker);
void P_RemoveThinker (thinker_t* thinker);


//
// P_PSPR
//
// Empty! Tails


//
// P_USER
//
typedef struct camera_s
{
    boolean     chase;
    angle_t     aiming;
    int         fixedcolormap;

    //SoM: Things used by FS cameras.
    fixed_t     viewheight;
    angle_t     startangle;

	// Camera demobjerization Tails 09-29-2002
    // Info for drawing: position.
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    //More drawing info: to determine current sprite.
    angle_t             angle;  // orientation

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
} camera_t;

extern camera_t camera;
extern camera_t camera2;

extern consvar_t cv_cam_dist; 
extern consvar_t cv_cam_still; // Tails 
extern consvar_t cv_cam_height;
extern consvar_t cv_cam_speed;
extern consvar_t cv_cam_rotate; // Tails 01-30-2002
extern consvar_t cv_cam_rotspeed;

extern consvar_t cv_cam2_dist; 
extern consvar_t cv_cam2_still;
extern consvar_t cv_cam2_height;
extern consvar_t cv_cam2_speed;
extern consvar_t cv_cam2_rotate;
extern consvar_t cv_cam2_rotspeed;

extern fixed_t t_cam_dist;
extern fixed_t t_cam_height;
extern fixed_t t_cam_rotate;
extern fixed_t t_cam2_dist;
extern fixed_t t_cam2_height;
extern fixed_t t_cam2_rotate;


void   P_ResetCamera (player_t* player, camera_t* thiscam);
void   P_PlayerThink (player_t* player);

// client prediction
void   CL_ResetSpiritPosition (mobj_t *mobj);
void   P_MoveSpirit (player_t* p,ticcmd_t *cmd, int realtics);

//
// P_MOBJ
//
#define ONFLOORZ        MININT
#define ONCEILINGZ      MAXINT

// Time interval for item respawning.
// WARNING MUST be a power of 2
#define ITEMQUESIZE     1024

extern mapthing_t     *itemrespawnque[ITEMQUESIZE];
extern tic_t          itemrespawntime[ITEMQUESIZE];
extern int              iquehead;
extern int              iquetail;


void P_RespawnSpecials (void);
//void P_RespawnWeapons(void);

mobj_t*
P_SpawnMobj
( fixed_t       x,
  fixed_t       y,
  fixed_t       z,
  mobjtype_t    type );

void    P_RemoveMobj (mobj_t* th);
void    P_RemoveSavegameMobj (mobj_t* th); // Tails 02-02-2002
boolean P_SetMobjState (mobj_t* mobj, statenum_t state);
void    P_MobjThinker (mobj_t* mobj);
void    P_RailThinker (mobj_t* mobj);
void    P_SceneryThinker (mobj_t* mobj); // Tails 07-24-2002

//spawn splash at surface of water in sector where the mobj resides
// void    P_SpawnSplash (mobj_t* mo, boolean oldwater); // Tails 12-05-2001
//Fab: when fried in in lava/slime, spawn some smoke
void    P_SpawnSmoke (fixed_t x, fixed_t y, fixed_t z);

mobj_t *P_SpawnMissile (mobj_t* source, mobj_t* dest, mobjtype_t type);

// Tails 10-28-2002
mobj_t *P_SpawnXYZMissile (mobj_t* source, mobj_t* dest, mobjtype_t type, fixed_t x, fixed_t y, fixed_t z);

mobj_t *P_SPMAngle ( mobj_t* source, mobjtype_t type, angle_t angle );
#define P_SpawnPlayerMissile(s,t) P_SPMAngle(s,t,s->angle)

//
// P_ENEMY
//

// when pushing a line 
//#define MAXSPECIALCROSS 16

extern  int     *spechit;                //SoM: 3/15/2000: Limit removal
extern  int     numspechit;

void P_NoiseAlert (mobj_t* target, mobj_t* emmiter);

void P_UnsetThingPosition (mobj_t* thing);
void P_SetThingPosition (mobj_t* thing);

//
// P_MAP
//

// If "floatok" true, move would be ok
// if within "tmfloorz - tmceilingz".
extern boolean          floatok;
extern fixed_t          tmfloorz;
extern fixed_t          tmceilingz;
extern fixed_t          tmsectorceilingz;      //added:28-02-98: p_spawnmobj
extern mobj_t*          tmfloorthing;

extern  line_t*         ceilingline;
extern  line_t*         blockingline;
extern  msecnode_t*     sector_list;

// Tails 08-25-2002
extern mprecipsecnode_t* precipsector_list;

boolean P_CheckPosition (mobj_t *thing, fixed_t x, fixed_t y);
boolean P_TryMove (mobj_t* thing, fixed_t x, fixed_t y, boolean allowdropoff);
boolean P_TeleportMove (mobj_t* thing, fixed_t x, fixed_t y);
void    P_SlideMove (mobj_t* mo);
void    P_BounceMove (mobj_t* mo); // Graue 12-31-2003
boolean P_CheckSight (mobj_t* t1, mobj_t* t2);
void    P_UseLines (player_t* player);

boolean P_CheckSector(sector_t *sector, boolean crunch);
boolean P_ChangeSector (sector_t* sector, boolean crunch);

void    P_DelSeclist(msecnode_t *);
// Tails 08-25-2002
void    P_DelPrecipSeclist(mprecipsecnode_t*);

void    P_CreateSecNodeList(mobj_t*,fixed_t,fixed_t);
int     P_GetMoveFactor(mobj_t* mo);
void    P_Initsecnode( void );

extern mobj_t*  linetarget;     // who got hit (or NULL)

extern fixed_t attackrange;

fixed_t
P_AimLineAttack
( mobj_t*       t1,
  angle_t       angle,
  fixed_t       distance );

void
P_LineAttack
( mobj_t*       t1,
  angle_t       angle,
  fixed_t       distance,
  fixed_t       slope,
  int           damage );

void
P_RadiusAttack
( mobj_t*       spot,
  mobj_t*       source,
  int           damage );



//
// P_SETUP
//
extern byte*            rejectmatrix;   // for fast sight rejection
extern long*           blockmaplump;   // offsets in blockmap are from here
extern long*           blockmap; // Big blockmap Tails
extern int              bmapwidth;
extern int              bmapheight;     // in mapblocks
extern fixed_t          bmaporgx;
extern fixed_t          bmaporgy;       // origin of block map
extern mobj_t**         blocklinks;     // for thing chains


//
// P_INTER
//

void
P_TouchSpecialThing
( mobj_t*       special,
  mobj_t*       toucher,
  boolean   heightcheck );

boolean
P_DamageMobj
( mobj_t*       target,
  mobj_t*       inflictor,
  mobj_t*       source,
  int           damage );

// Tails 09-06-2002
void P_PlayRinglossSound(mobj_t* source);
void P_PlayDeathSound(mobj_t* source);
void P_PlayVictorySound(mobj_t* source);
void P_PlayTauntSound(mobj_t* source);


//
// P_SIGHT
//

// slopes to top and bottom of target
extern fixed_t  topslope;
extern fixed_t  bottomslope;


//
// P_SPEC
//
#include "p_spec.h"



//SoM: 3/6/2000: Added public "boomsupport variable"
extern const int boomsupport;
extern const int variable_friction;
extern const int allow_pushers;


// heretic specific
extern int ceilmovesound;
extern int doorclosesound;

#define TELEFOGHEIGHT  (32*FRACUNIT)
//extern mobjtype_t      PuffType;
#define FOOTCLIPSIZE   (10*FRACUNIT)
#define HITDICE(a) ((1+(P_Random()&7))*a)

#define MAXCHICKENHEALTH 30

#define BLINKTHRESHOLD  (4*32)
#define WPNLEV2TICS     (40*TICRATE)
#define FLIGHTTICS      (60*TICRATE)

//#define CHICKENTICS     (40*TICRATE)
#define FLOATRANDZ      (MAXINT-1)

void P_RepositionMace(mobj_t *mo);
//void P_ActivateBeak(player_t *player);
void P_DSparilTeleport(mobj_t *actor);
//void P_InitMonsters(void);
boolean P_LookForMonsters(mobj_t *actor);
//int P_GetThingFloorType(mobj_t *thing);
mobj_t *P_CheckOnmobj(mobj_t *thing);
void P_AddMaceSpot(mapthing_t *mthing);
boolean P_SightPathTraverse (fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
//void P_HerePlayerInSpecialSector(player_t *player);
boolean P_TestMobjLocation(mobj_t *mobj);
void P_SetPsprite ( player_t*     player,
                    int           position,
                    statenum_t    stnum );
boolean P_Teleport(mobj_t *thing, fixed_t x, fixed_t y, angle_t angle);
boolean P_SeekerMissile(mobj_t *actor, angle_t thresh, angle_t turnMax);
mobj_t *P_SpawnMissileAngle(mobj_t *source, mobjtype_t type,
        angle_t angle, fixed_t momz);
boolean P_SetMobjStateNF(mobj_t *mobj, statenum_t state);
boolean P_CheckMissileSpawn (mobj_t* th);
void P_ThrustMobj(mobj_t *mo, angle_t angle, fixed_t move);
void P_Thrust(mobj_t *mo, angle_t angle, fixed_t move);
void P_ExplodeMissile (mobj_t* mo);
void P_SetMessage(player_t *player, char *message, boolean ultmsg);
//void P_Massacre(void);
//void P_AddBossSpot(fixed_t x, fixed_t y, angle_t angle);

#endif  // __P_LOCAL__
