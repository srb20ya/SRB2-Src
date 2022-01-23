// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: I_video.c,v 1.3 2000/08/10 11:07:51 ydario Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log: I_video.c,v $
// Revision 1.3  2000/08/10 11:07:51  ydario
// fix CRLF
//
// Revision 1.2  2000/08/10 09:19:31  ydario
// *** empty log message ***
//
// Revision 1.1  2000/08/09 12:15:09  ydario
// OS/2 specific platform code
//
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

static const char
rcsid[] = "$Id: I_video.c,v 1.3 2000/08/10 11:07:51 ydario Exp $";

#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <signal.h>

#include "i_os2.h"

#include "doomstat.h"
#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
#include "m_argv.h"
#include "d_main.h"

#include "doomdef.h"

boolean         highcolor;
rendermode_t    rendermode=render_soft;

#define MAXWINMODES (6) 
static char vidModeName[MAXWINMODES][32];
static int windowedModes[MAXWINMODES][2] = {
   // first is default mode
   {320, 200},
   {400, 300},
   {512, 384}, 
   {640, 480},
   {800, 600},
   {1024, 768},
};

int   VID_NumModes( void);
int   VID_GetModeForSize( int w, int h);
void  VID_Init( void);
char* VID_GetModeName( int modenum);
int   VID_SetMode( int modenum);


//
// I_StartFrame
//
void I_StartFrame (void)
{
    // er?
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
    // what is this?
}

//
// I_FinishUpdate
//
void I_FinishUpdate (void)
{
    static int	lasttic;
    int		tics;
    int		i;
    // UNUSED static unsigned char *bigscreen=0;

    // draws little dots on the bottom of the screen
   if (devparm) {
      i = I_GetTime();
      tics = i - lasttic;
      lasttic = i;
      if (tics > 20) tics = 20;

      for (i=0 ; i<tics*2 ; i+=2)
            screens[0][ (vid.height-1)*vid.width*vid.bpp + i] = 0xff;
      for ( ; i<20*2 ; i+=2)
            screens[0][ (vid.height-1)*vid.width*vid.bpp + i] = 0x0;
    }

      // Blit the image using DiveBlit
   if (!pmData->fDataInProcess && pmData->tidBlitThread==0)
      DiveBlitImage ( pmData->hDive, pmData->ulImage, DIVE_BUFFER_SCREEN);
}


//
// This is meant to be called only by CONS_Printf() while game startup
//
void I_LoadingScreen ( PSZ msg )
{
    HPS    hps;
    RECTL  rect;

    if ( msg ) {

        hps = WinGetPS( pmData->hwndClient);
        WinQueryWindowRect( pmData->hwndClient, &rect);
        WinFillRect(hps, &rect, CLR_WHITE);
        WinDrawText( hps, strlen( msg), msg, &rect,
                     0, 0, 
                     DT_WORDBREAK | DT_TOP | DT_LEFT | DT_TEXTATTRS);
    }
}

//
// I_ReadScreen
//
void I_ReadScreen (byte* scr)
{
    memcpy (scr, vid.buffer, vid.width*vid.height*vid.bpp);
}

//
// I_SetPalette
//
void I_SetPalette (byte* palette)
{
   int   i, r, g, b;
   long  colors[ 256];
   byte*  usegamma;

   usegamma = gammatable[scr_gamma%5];

      // set the X colormap entries
   for (i=0 ; i<256 ; i++) {
      r = usegamma[*palette++];
      g = usegamma[*palette++];
      b = usegamma[*palette++];
      colors[i] = (r<<16) + (g<<8) + b; //(PC_RESERVED * 16777216) +
   }
      // set dive palette
   DiveSetSourcePalette( pmData->hDive, 0,
                         pmData->ulNumColors,
                         (PBYTE) colors);
}

//
//  Close the screen, restore previous video mode.
//
void I_ShutdownGraphics(void)
{
    printf( "I_ShutdownGraphics\n");

    if( !graphics_started )
        return;

    ShutdownDIVE( pmData);
    pmData->ulToEnd = 1;                  /* set stop flag */

    graphics_started = false;
}

//
//  Initialize video mode, setup dynamic screen size variables,
//  and allocate screens.
//
void I_StartupGraphics(void)
{
   CONS_Printf("I_StartupGraphics\n");

   if( graphics_started )
      return;

   InitDIVE( pmData);
   //VID_Init();
   //setup the videmodes list,
   VID_SetMode(0);

   //added:03-01-98: register exit code for graphics
   I_AddExitFunc(I_ShutdownGraphics);
   graphics_started = true;
   CONS_Printf("I_StartupGraphics: DONE\n");
}

//added:30-01-98: return number of video modes in pvidmodes list
int VID_NumModes(void)
{
    return MAXWINMODES;
}

//added:03-02-98: return a video mode number from the dimensions
int VID_GetModeForSize( int w, int h)
{
    int i;

   CONS_Printf("VID_GetModeForSize: %dx%d\n", w, h);

    for (i=0; i<MAXWINMODES;i++)
        if(windowedModes[i][0]==w && windowedModes[i][1]==h)
            return i;

   CONS_Printf("VID_GetModeForSize: %dx%d not found\n", w, h);

    return 0;
}

//added:30-01-98:return the name of a video mode
char *VID_GetModeName (int modenum)
{
   sprintf( vidModeName[modenum], "%dx%d", 
            windowedModes[modenum][0],
            windowedModes[modenum][1]);
   //CONS_Printf("VID_GetModeName: %s\n", vidModeName[modenum]);
   return vidModeName[modenum];
}

// ========================================================================
// Sets a video mode
// ========================================================================
int VID_SetMode (int modenum)  //, unsigned char *palette)
{
   CONS_Printf("VID_SetMode(%d)\n", modenum);

   if (modenum >= MAXWINMODES) {
       printf("VID_SetMode modenum >= MAXWINMODES\n");
       return -1;
   }
/*
   if (pmData->pbBuffer) { // init code only once
       printf("VID_SetMode already called\n");
       return -1;
   }
*/
   // initialize vidbuffer size for setmode
   vid.width  = windowedModes[modenum][0];
   vid.height = windowedModes[modenum][1];
   //vid.aspect = pcurrentmode->aspect;
   printf("VID_SetMode %dx%d\n", vid.width, vid.height);

   // adjust window size
   pmData->ulWidth = vid.width;
   pmData->ulHeight = vid.height;
   WinPostMsg( pmData->hwndClient, WM_COMMAND, (MPARAM) ID_NEWTEXT, NULL);
   WinPostMsg( pmData->hwndClient, WM_COMMAND, (MPARAM) ID_SNAP, NULL);

   //if (pmData->pbBuffer)
   //    ShutdownDIVE( pmData);
   //pmData->pbBuffer = 0;
   InitDIVEBuffer( pmData);

   vid.buffer = (byte*) pmData->pbBuffer; //;//

   //added:20-01-98: recalc all tables and realloc buffers based on
   //                vid values.
   vid.rowbytes = vid.width;
   vid.bpp      = 1;
   vid.recalc   = 1;
   vid.modenum  = modenum;

   printf("VID_SetMode(%d) DONE\n", modenum);
   return 1;
}
