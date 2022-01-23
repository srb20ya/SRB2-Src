// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: v_video.h,v 1.3 2000/03/29 20:10:50 hurdler Exp $
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
// $Log: v_video.h,v $
// Revision 1.3  2000/03/29 20:10:50  hurdler
// your fix didn't work under windows, find another solution
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Gamma correction LUT.
//      Functions to draw patches (by post) directly to screen.
//      Functions to blit a block to the screen.
//
//-----------------------------------------------------------------------------


#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomdef.h"
#include "doomtype.h"
#include "r_defs.h"


//
// VIDEO
//

// unused?
//#define CENTERY                 (vid.height/2)

//added:18-02-98:centering offset for the scaled graphics,
//               this is normally temporarily changed by m_menu.c only.
//               The rest of the time it should be zero.
extern  int     scaledofs;

// Screen 0 is the screen updated by I_Update screen.
// Screen 1 is an extra buffer.

extern  byte*   screens[5];

extern  int     dirtybox[4];

extern  byte    gammatable[5][256];
extern  consvar_t cv_ticrate;

// Allocates buffer screens, call before R_Init.
void V_Init (void);

// Set the current RGB palette lookup to use for palettized graphics
void V_SetPalette( byte* palette );

// Retrieve the ARGB value from a palette color index
ULONG V_GetColor( int color );


void
V_CopyRect
( int           srcx,
  int           srcy,
  int           srcscrn,
  int           width,
  int           height,
  int           destx,
  int           desty,
  int           destscrn );

//added:03-02-98:like V_DrawPatch, + using a colormap.
void V_DrawMappedPatch ( int           x,
                         int           y,
                         int           scrn,
                         patch_t*      patch,
                         byte*         colormap );

//added:05-02-98:V_DrawPatch scaled 2,3,4 times size and position.

// flags hacked in scrn
#define V_NOSCALESTART       0x10000   // dont scale x,y, start coords

void V_DrawScaledPatch ( int           x,
                         int           y,
                         int           scrn,    // + flags
                         patch_t*      patch );

void V_DrawScaledPatchFlipped ( int           x,
                                int           y,
                                int           scrn,  // hacked flags in it...
                                patch_t*      patch );

//added:05-02-98:kiktest : this draws a patch using translucency
void V_DrawTransPatch ( int           x,
                        int           y,
                        int           scrn,
                        patch_t*      patch );

//added:08-02-98: like V_DrawPatch, but uses a colormap, see comments in .c
void V_DrawTranslationPatch ( int           x,
                              int           y,
                              int           scrn,
                              patch_t*      patch,
                              byte*         colormap );

//added:16-02-98: like V_DrawScaledPatch, plus translucency
void V_DrawTranslucentPatch ( int           x,
                              int           y,
                              int           scrn,
                              patch_t*      patch );


void V_DrawPatch ( int           x,
                   int           y,
                   int           scrn,
                   patch_t*      patch);


void V_DrawPatchDirect ( int           x,
                         int           y,
                         int           scrn,
                         patch_t*      patch );



// Draw a linear block of pixels into the view buffer.
void V_DrawBlock ( int           x,
                   int           y,
                   int           scrn,
                   int           width,
                   int           height,
                   byte*         src );

// Reads a linear block of pixels into the view buffer.
void V_GetBlock ( int           x,
                  int           y,
                  int           scrn,
                  int           width,
                  int           height,
                  byte*         dest );

// draw a pic_t, SCALED
void V_DrawScalePic ( int           x1,
                      int           y1,
                      int           scrn,
                      int           lumpnum /*pic_t*        pic */);


void V_MarkRect ( int           x,
                  int           y,
                  int           width,
                  int           height );


//added:05-02-98: fill a box with a single color
void V_DrawFill (int x, int y, int w, int h, int c);
//added:06-02-98: fill a box with a flat as a pattern
void V_DrawFlatFill (int x, int y, int w, int h, int flatnum);

//added:10-02-98: fade down the screen buffer before drawing the menu over
void V_DrawFadeScreen (void);

//added:20-03-98: test console
void V_DrawFadeConsBack (int x1, int y1, int x2, int y2);

//added:20-03-98: draw a single character
void V_DrawCharacter (int x, int y, int c);

//added:05-02-98: draw a string using the hu_font
void V_DrawString (int x, int y, char* string);
//added:05-02-98: V_DrawString which remaps text color to whites
void V_DrawStringWhite (int x, int y, char* string);

// Find string width from hu_font chars
int  V_StringWidth (char* string);

//added:12-02-98:
void V_DrawTiltView (byte *viewbuffer);

//added:05-04-98: test persp. correction !!
void V_DrawPerspView (byte *viewbuffer, int aiming);

void VID_BlitLinearScreen (byte *srcptr, byte *destptr, int width,
                           int height, int srcrowbytes, int destrowbytes);

#endif
