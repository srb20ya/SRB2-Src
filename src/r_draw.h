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
/// \brief Low-level span/column drawer functions

#ifndef __R_DRAW__
#define __R_DRAW__

#include "r_defs.h"

// -------------------------------
// COMMON STUFF FOR 8bpp AND 16bpp
// -------------------------------
extern byte* ylookup[MAXVIDHEIGHT];
extern byte* ylookup1[MAXVIDHEIGHT];
extern byte* ylookup2[MAXVIDHEIGHT];
extern int columnofs[MAXVIDWIDTH];

// -------------------------
// COLUMN DRAWING CODE STUFF
// -------------------------

extern lighttable_t*  dc_colormap;
extern int dc_x, dc_yl, dc_yh;
extern fixed_t dc_iscale, dc_texturemid;
extern boolean dc_hires;

extern byte* dc_source; // first pixel in a column

// translucency stuff here
extern byte* transtables; // translucency tables, should be (*transtables)[5][256][256]
extern byte* dc_transmap;

// translation stuff here

extern byte* translationtables[MAXSKINS];
extern byte* defaulttranslationtables;
extern byte* bosstranslationtables;
extern byte* dc_translation;

extern struct r_lightlist_s* dc_lightlist;
extern int dc_numlights, dc_maxlights;

//Fix TUTIFRUTI
extern int dc_texheight;
extern int whereitsfrom;

// -----------------------
// SPAN DRAWING CODE STUFF
// -----------------------

extern int ds_y, ds_x1, ds_x2;
extern lighttable_t* ds_colormap;
extern fixed_t ds_xfrac, ds_yfrac, ds_xstep, ds_ystep;
extern byte* ds_source; // start of a 64*64 tile image
extern byte* ds_transmap;

// Variable flat sizes
extern ULONG flatsize;
extern ULONG flatmask;
extern ULONG flatsubtract;

/// \brief Top border
#define BRDR_T 0
/// \brief Bottom border
#define BRDR_B 1
/// \brief Left border
#define BRDR_L 2
/// \brief Right border
#define BRDR_R 3
/// \brief Topleft border
#define BRDR_TL 4
/// \brief Topright border
#define BRDR_TR 5
/// \brief Bottomleft border
#define BRDR_BL 6
/// \brief Bottomright border
#define BRDR_BR 7

extern int viewborderlump[8];

// ------------------------------------------------
// r_draw.c COMMON ROUTINES FOR BOTH 8bpp and 16bpp
// ------------------------------------------------

// Initialize color translation tables, for player rendering etc.
void R_InitTranslationTables(void);

void R_LoadSkinTable(void);

// Custom player skin translation
void R_InitSkinTranslationTables(int starttranscolor, int endtranscolor, int skinnum);
void R_InitViewBuffer(int width, int height);
void R_InitViewBorder(void);
void R_VideoErase(unsigned ofs, int count);

// Rendering function.
void R_FillBackScreen(void);

// If the view size is not full screen, draws a border around it.
void R_DrawViewBorder(void);

// -----------------
// 8bpp DRAWING CODE
// -----------------

void R_DrawColumn_8(void);
void R_DrawSkyColumn_8(void);
void R_DrawShadeColumn_8(void);
void R_DrawTranslucentColumn_8(void);

#ifdef USEASM
void ASMCALL R_DrawColumn_8_ASM(void);
void ASMCALL R_DrawSkyColumn_8_ASM(void);
void ASMCALL R_DrawShadeColumn_8_ASM(void);
void ASMCALL R_DrawTranslucentColumn_8_ASM(void);

void ASMCALL R_DrawColumn_8_Pentium(void); // Optimised for Pentium
void ASMCALL R_DrawColumn_8_NOMMX(void);   // DOSDoom original
void ASMCALL R_DrawColumn_8_K6_MMX(void);  // MMX asm version, optimised for K6
#endif

void R_DrawTranslatedColumn_8(void);
void R_DrawTranslatedTranslucentColumn_8(void);
void R_DrawSpan_8(void);
void R_DrawTranslucentSpan_8(void);

void R_DrawFogSpan_8(void);
void R_DrawFogColumn_8(void);
void R_DrawColumnShadowed_8(void);

// ------------------
// 16bpp DRAWING CODE
// ------------------

void R_DrawColumn_16(void);
void R_DrawSkyColumn_16(void);
void R_DrawTranslucentColumn_16(void);
void R_DrawTranslatedColumn_16(void);
void R_DrawSpan_16(void);

// =========================================================================
#endif  // __R_DRAW__
