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
/// \brief Rendering variables, consvars, defines

#ifndef __R_MAIN__
#define __R_MAIN__

#include "d_player.h"
#include "r_data.h"

//
// POV related.
//
extern fixed_t viewcos, viewsin;
extern int viewheight;
extern int centerx, centery;

extern fixed_t centerxfrac, centeryfrac;
extern fixed_t projection, projectiony;

extern int validcount, linecount, loopcount, framecount;

//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.
// Now why not 32 levels here?
#define LIGHTLEVELS 16
#define LIGHTSEGSHIFT 4

#define MAXLIGHTSCALE 48
#define LIGHTSCALESHIFT 12
#define MAXLIGHTZ 128
#define LIGHTZSHIFT 20

extern lighttable_t* scalelight[LIGHTLEVELS][MAXLIGHTSCALE];
extern lighttable_t* scalelightfixed[MAXLIGHTSCALE];
extern lighttable_t* zlight[LIGHTLEVELS][MAXLIGHTZ];

// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.
#define NUMCOLORMAPS 32

// Utility functions.
int R_PointOnSide(fixed_t x, fixed_t y, node_t* node);
int R_PointOnSegSide(fixed_t x, fixed_t y, seg_t* line);
angle_t R_PointToAngle(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t x2, fixed_t y2, fixed_t x1, fixed_t y1);
fixed_t R_PointToDist(fixed_t x, fixed_t y);
fixed_t R_PointToDist2(fixed_t x2, fixed_t y2, fixed_t x1, fixed_t y1);

// ZDoom C++ to Legacy C conversion Tails 04-29-2002
fixed_t R_SecplaneZatPoint(secplane_t* secplane, fixed_t x, fixed_t y);
fixed_t R_SecplaneZatPointDist(secplane_t* secplane, fixed_t x, fixed_t y, fixed_t dist);
void R_SecplaneFlipVert(secplane_t* secplane);
boolean R_ArePlanesSame(secplane_t* original,  secplane_t* other);
boolean R_ArePlanesDifferent(secplane_t* original,  secplane_t* other);
void R_SecplaneChangeHeight(secplane_t* secplane, fixed_t hdiff);
fixed_t R_SecplaneHeightDiff(secplane_t* secplane, fixed_t oldd);
fixed_t R_SecplanePointToDist(secplane_t* secplane, fixed_t x, fixed_t y, fixed_t z);
fixed_t R_SecplanePointToDist2(secplane_t* secplane, fixed_t x, fixed_t y, fixed_t z);

fixed_t R_ScaleFromGlobalAngle(angle_t visangle);
subsector_t* R_PointInSubsector(fixed_t x, fixed_t y);
subsector_t* R_IsPointInSubsector(fixed_t x, fixed_t y);

//
// REFRESH - the actual rendering functions.
//

extern consvar_t cv_splitscreen;
extern consvar_t cv_viewsize;
extern consvar_t cv_grtranslucenthud;
extern consvar_t cv_chasecam, cv_chasecam2, cv_homing, cv_lightdash;
extern consvar_t cv_numsnow, cv_shadow;
extern consvar_t cv_snow, cv_storm, cv_rain, cv_raindensity, cv_precipdist;
extern consvar_t cv_tailspickup;

// Called by startup code.
void R_Init(void);

// just sets setsizeneeded true
extern boolean setsizeneeded;
void R_SetViewSize(void);

// do it (sometimes explicitly called)
void R_ExecuteSetViewSize(void);

void R_SetupFrame(player_t* player);
// Called by G_Drawer.
void R_RenderPlayerView(player_t* player);

// add commands related to engine, at game startup
void R_RegisterEngineStuff(void);
#endif
