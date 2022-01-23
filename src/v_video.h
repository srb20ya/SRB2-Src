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
/// \brief Gamma correction LUT

#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomdef.h"
#include "doomtype.h"
#include "r_defs.h"

//
// VIDEO
//

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.

extern byte* screens[5];

extern const byte gammatable[5][256];
extern consvar_t cv_ticrate, cv_usegamma;

// Allocates buffer screens, call before R_Init.
void V_Init(void);

// Set the current RGB palette lookup to use for palettized graphics
void V_SetPalette(int palettenum);

void V_SetPaletteLump(const char *pal);

extern RGBA_t *pLocalPalette;

// Retrieve the ARGB value from a palette color index
#define V_GetColor(color) (pLocalPalette[color&0xFF])

// like V_DrawPatch, + using a colormap.
void V_DrawMappedPatch(int x, int y, int scrn, patch_t* patch, const byte* colormap);

// flags hacked in scrn (not supported by all functions (see src))
#define V_NOSCALESTART       0x00010000  // don't scale x, y, start coords
#define V_SCALESTART         0x00020000  // scale x, y, start coords
#define V_SCALEPATCH         0x00040000  // scale patch
#define V_NOSCALEPATCH       0x00080000  // don't scale patch
#define V_WHITEMAP           0x00100000  // draw white (for v_drawstring)
#define V_SNAPTOTOP          0x00200000  // for centering
#define V_TRANSLUCENT        0x00400000  // TRANSMED applied
#define V_SNAPTOBOTTOM       0x00800000  // for centering
#define V_GREENMAP           0x01000000  // Green colormap
#define V_BLUEMAP            0x02000000  // Blue colormap
#define V_8020TRANS          0x04000000
#define V_9010TRANS          0x08000000
#define V_TOPLEFT            0x10000000
#define V_RETURN8            0x20000000
#define V_SNAPTOLEFT         0x40000000 // for centering
#define V_SNAPTORIGHT        0x80000000 // for centering

// default params: scale patch and scale start
void V_DrawScaledPatch(int x, int y, int scrn, patch_t* patch);

// default params: scale patch and scale start
void V_DrawSmallScaledPatch(int x, int y, int scrn, patch_t* patch, const byte* colormap);

// like V_DrawScaledPatch, plus translucency
void V_DrawTranslucentPatch(int x, int y, int scrn, patch_t* patch);

void V_DrawPatch(int x, int y, int scrn, patch_t* patch);

// Draw a linear block of pixels into the view buffer.
void V_DrawBlock(int x, int y, int scrn, int width, int height, const byte* src);

// fill a box with a single color
void V_DrawFill(int x, int y, int w, int h, int c);
// fill a box with a flat as a pattern
void V_DrawFlatFill(int x, int y, int w, int h, int flatnum);

// fade down the screen buffer before drawing the menu over
void V_DrawFadeScreen(void);

void V_DrawFadeConsBack(int x1, int y1, int x2, int y2);

// draw a single character
void V_DrawCharacter(int x, int y, int c);

void V_DrawLevelTitle(int x, int y, int option, const char* string);

// draw a string using the hu_font
void V_DrawString(int x, int y, int option, const char* string);
void V_DrawCenteredString(int x, int y, int option, const char* string);
void V_DrawRightAlignedString(int x, int y, int option, const char* string);

// Find string width from lt_font chars
int V_LevelNameWidth(const char* string);
int V_LevelNameHeight(const char* string);

void V_DrawCreditString(int x, int y, int option, const char* string);
int V_CreditStringWidth(const char* string);

// Find string width from hu_font chars
int V_StringWidth(const char* string);

void V_DrawPatchFill(patch_t* pat);

void VID_BlitLinearScreen(const byte *srcptr, byte *destptr, int width, int height, int srcrowbytes,
	int destrowbytes);

#endif
