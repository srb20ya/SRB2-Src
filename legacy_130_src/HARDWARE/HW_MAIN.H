// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_main.h,v 1.9 2000/07/01 09:23:50 bpereira Exp $
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
// $Log: hw_main.h,v $
// Revision 1.9  2000/07/01 09:23:50  bpereira
// no message
//
// Revision 1.8  2000/05/09 20:57:31  hurdler
// use my own code for colormap (next time, join with Boris own code)
// (necessary due to a small bug in Boris' code (not found) which shows strange effects under linux)
//
// Revision 1.7  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.6  2000/04/27 17:48:47  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.5  2000/04/24 15:23:13  hurdler
// Support colormap for text
//
// Revision 1.4  2000/04/22 21:08:23  hurdler
// I like it better like that
//
// Revision 1.3  2000/04/12 16:03:51  hurdler
// ready for T&L code and true static lighting
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      3D render mode functions
//
//-----------------------------------------------------------------------------


#ifndef __HWR_MAIN_H__
#define __HWR_MAIN_H__

#include "hw_data.h"

#include "../am_map.h"
#include "../d_player.h"
#include "../r_defs.h"

// Startup & Shutdown the hardware mode renderer
void HWR_Startup (void);
void HWR_Shutdown (void);

void HWR_clearAutomap (void);
void HWR_drawAMline (fline_t* fl, int color);
void HWR_FadeScreenMenuBack (unsigned long color, int height);
void HWR_RenderPlayerView (int viewnumber, player_t* player);
void HWR_DrawViewBorder (int clearlines);
void HWR_DrawFlatFill (int x, int y, int w, int h, int flatlumpnum);
void HWR_Screenshot (void);
void HWR_InitTextureMapping (void);
void HWR_SetViewSize (int blocks);
void HWR_ScalePatch (BOOL bScalePatch);
void HWR_DrawPatch (GlidePatch_t* gpatch, int x, int y);
void HWR_DrawMappedPatch (GlidePatch_t* gpatch, int x, int y, byte *colormap);
void HWR_Make3DfxPatch (patch_t* patch, GlidePatch_t* grPatch, GlideMipmap_t *grMipmap);
void HWR_CreatePlanePolygons (int bspnum);
#ifdef TANDL
void HWR_CreateStaticLightmaps (int bspnum);
#endif
void HWR_PrepLevelCache (int numtextures);
void HWR_DrawFill(int x, int y, int w, int h, int color);
void HWR_DrawPic(int x,int y,int lumpnum);

void HWR_AddCommands (void);

extern consvar_t cv_grcrappymlook;
extern consvar_t cv_grdynamiclighting;
extern consvar_t cv_grstaticlighting;
extern consvar_t cv_grmblighting;
extern consvar_t cv_grcoronas;
extern consvar_t cv_grfov;
extern consvar_t cv_grpolygonsmooth;
extern consvar_t cv_grmd2;
extern consvar_t cv_grfog;
extern consvar_t cv_grfogcolor;
extern consvar_t cv_grfogdensity;
extern consvar_t cv_grcontrast;
extern consvar_t cv_grgammared;
extern consvar_t cv_grgammagreen;
extern consvar_t cv_grgammablue;

#endif
