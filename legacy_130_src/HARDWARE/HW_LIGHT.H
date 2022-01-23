// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_light.h,v 1.7 2000/08/03 17:57:42 bpereira Exp $
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
//
//
// $Log: hw_light.h,v $
// Revision 1.7  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.6  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.5  2000/04/14 16:34:26  hurdler
// some nice changes for coronas
//
// Revision 1.4  2000/04/12 16:03:51  hurdler
// ready for T&L code and true static lighting
//
// Revision 1.3  2000/03/29 19:39:49  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Dynamic lighting & coronas add on by Hurdler 
//
//-----------------------------------------------------------------------------


#ifndef _HW_LIGHTS_
#define _HW_LIGHTS_

#include "hw_glob.h"
#include "hw_drv.h"
#include "hw_defs.h"

//#define DL_OLD_METHOD     // shouldn't be defined because the previous methode 
                            // is not yet correctly converted to the new driver

// Do transformation in hardware
//#define DO_MIRROR

#define DL_MAX_LIGHT    256  // maximum number of light (extra light are ignored)

typedef struct {
    float x, y, z;
} lvertex3D_t;

void HWR_InitLight( void );
void HWR_DoCoronasLighting(FOutVector *outVerts, gr_vissprite_t *spr);
void HWR_DL_AddLight(gr_vissprite_t *spr, GlidePatch_t *patch);
void HWR_DynamicShadowing(FOutVector *clVerts, int nrClipVerts, lvertex3D_t *lVerts, player_t *p);
void HWR_PlaneLighting(FOutVector *clVerts, int nrClipVerts, lvertex3D_t *lVerts);
#ifdef DL_OLD_METHOD
void HWR_WallLighting(lvertex3D_t *clZVerts);
#else
void HWR_WallLighting(FOutVector *wlVerts, lvertex3D_t *lVerts);
#endif
void HWR_ResetLights(void);
void HWR_SetLights(int viewnumber);

typedef struct {
    int         nb;
    light_t    *p_lspr[DL_MAX_LIGHT];
    lvertex3D_t position[DL_MAX_LIGHT]; // actually maximum DL_MAX_LIGHT lights
    lvertex3D_t positionnottransformed[DL_MAX_LIGHT]; // remove me when TANDL done
    mobj_t     *mo[DL_MAX_LIGHT];
} dynlights_t;

extern  dynlights_t view_dynlights[];
extern  light_t    *t_lspr[NUMSPRITES];

#endif
