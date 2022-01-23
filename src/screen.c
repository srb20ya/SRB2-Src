// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
/// \file
/// \brief Handles multiple resolutions, 8bpp/16bpp(highcolor) modes

#include "doomdef.h"
#include "doomstat.h"
#include "screen.h"
#include "console.h"
#include "am_map.h"
#include "i_system.h"
#include "i_video.h"
#include "r_local.h"
#include "r_sky.h"
#include "m_argv.h"
#include "v_video.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "z_zone.h"
#include "d_main.h"
#include "d_clisrv.h"
#include "f_finale.h"


#if defined(USEASM) //&& (!defined(_MSC_VER) || (_MSC_VER <= 1200))
#define RUSEASM //MSC.NET can't patch itself
#endif

// --------------------------------------------
// assembly or c drawer routines for 8bpp/16bpp
// --------------------------------------------
void (*skycolfunc)(void); // new sky column drawer to draw posts >128 high
void (*colfunc)(void); // standard column, up to 128 high posts

void (*basecolfunc)(void);
void (*fuzzcolfunc)(void); // standard fuzzy effect column drawer
void (*transcolfunc)(void); // translucent column drawer
void (*shadecolfunc)(void); // smokie test..
void (*spanfunc)(void); // span drawer, use a 64x64 tile
void (*basespanfunc)(void); // default span func for color mode
void (*transtransfunc)(void); // translucent translated column drawer

// ------------------
// global video state
// ------------------
viddef_t vid;
int setmodeneeded; //video mode change needed if > 0 (the mode number to set + 1)

static CV_PossibleValue_t scr_depth_cons_t[] = {{8, "8 bits"}, {16, "16 bits"}, {24, "24 bits"},
	{32, "32 bits"}, {0, NULL}};

//added:03-02-98: default screen mode, as loaded/saved in config
consvar_t cv_scr_width = {"scr_width", "320", CV_SAVE, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_scr_height = {"scr_height", "200", CV_SAVE, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_scr_depth = {"scr_depth", "8 bits", CV_SAVE, scr_depth_cons_t, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_fullscreen = {"fullscreen", "Yes", CV_SAVE|CV_CALL, CV_YesNo, SCR_ChangeFullscreen, 0, NULL, NULL, 0, 0, NULL};

// =========================================================================
//                           SCREEN VARIABLES
// =========================================================================

int scr_bpp; // current video mode bytes per pixel
byte* scr_borderpatch; // flat used to fill the reduced view borders set at ST_Init()

// =========================================================================

#ifdef RUSEASM
// tell asm code the new rowbytes value.
void ASMCALL ASM_PatchRowBytes(int rowbytes);
void ASMCALL MMX_PatchRowBytes(int rowbytes);
#endif

//  Short and Tall sky drawer, for the current color mode
void (*skydrawerfunc)(void);

static boolean R_486 = false; //R_DrawColumn8_NOMMX
static boolean R_586 = false; //R_DrawColumn8_Pentium
static boolean R_MMX = false; //R_DrawColumn8_K6_MMX

void SCR_SetMode(void)
{
	if(dedicated)
		return;

	if(!setmodeneeded || WipeInAction)
		return; // should never happen and don't change it during a wipe, BAD!

	VID_SetMode(--setmodeneeded);

	V_SetPalette(0);

	//
	//  setup the right draw routines for either 8bpp or 16bpp
	//
	if(vid.bpp == 1)
	{
		spanfunc = basespanfunc = R_DrawSpan_8;
		transcolfunc = R_DrawTranslatedColumn_8;
		transtransfunc = R_DrawTranslatedTranslucentColumn_8;

		colfunc = basecolfunc = R_DrawColumn_8;
		shadecolfunc = R_DrawShadeColumn_8;
		fuzzcolfunc = R_DrawTranslucentColumn_8;
		skydrawerfunc = R_DrawSkyColumn_8; // tall sky
#ifdef RUSEASM
		//colfunc = basecolfunc = R_DrawColumn_8_ASM;
		shadecolfunc = R_DrawShadeColumn_8_ASM;
		fuzzcolfunc = R_DrawTranslucentColumn_8_ASM;
		//skydrawerfunc = R_DrawSkyColumn_8_ASM;
		if(R_486)
		{
			colfunc = basecolfunc = R_DrawColumn_8_NOMMX;
			CONS_Printf("using 486 code\n");
		}
		if(R_586)
		{
			colfunc = basecolfunc = R_DrawColumn_8_Pentium;
			CONS_Printf("upgrading to 586 code\n");
		}
		if(R_MMX)
		{
			colfunc = basecolfunc = R_DrawColumn_8_K6_MMX;
			CONS_Printf("now using cool MMX code\n");
		}
#endif
	}
	else if(vid.bpp > 1)
	{
		CONS_Printf("using highcolor mode\n");
		spanfunc = basespanfunc = R_DrawSpan_16;
		transcolfunc = R_DrawTranslatedColumn_16;
		transtransfunc = R_DrawTranslucentColumn_16; // No 16bit operation for this function

		colfunc = basecolfunc = R_DrawColumn_16;
		shadecolfunc = NULL; // detect error if used somewhere..
		fuzzcolfunc = R_DrawTranslucentColumn_16;
		skydrawerfunc = R_DrawSkyColumn_16;
	}
	else
		I_Error("unknown bytes per pixel mode %d\n", vid.bpp);
#ifndef DC
	if(vid.width % BASEVIDWIDTH || vid.height % BASEVIDHEIGHT)
		CONS_Printf("WARNING: Resolution is not aspect-correct!\n"
			"Use a multiple of %dx%d\n", BASEVIDWIDTH, BASEVIDHEIGHT);
#endif
	// set the apprpriate drawer for the sky (tall or short)
	setmodeneeded = 0;
}

// do some initial settings for the game loading screen
//
void SCR_Startup(void)
{
#if 0
	const CPUInfoFlags *RCpuInfo = I_CPUInfo();
	if(!M_CheckParm("-NOCPUID") && RCpuInfo)
	{
		R_486 = true;
		if(RCpuInfo->RDTSC)
			R_586 = true;
		if(R_586 && RCpuInfo->MMX)
			R_MMX = true;
	}
#endif
	if(M_CheckParm("-486"))
		R_486 = true;
	if(M_CheckParm("-586"))
		R_586 = true;
	if(M_CheckParm("-MMX"))
		R_MMX = true;

	if(dedicated)
	{
		V_Init();
		V_SetPalette(0);
		return;
	}

	vid.modenum = 0;

	vid.fdupx = (float)vid.width/BASEVIDWIDTH;
	vid.fdupy = (float)vid.height/BASEVIDHEIGHT;
	vid.dupx = (int)vid.fdupx;
	vid.dupy = (int)vid.fdupy;

	vid.baseratio = FRACUNIT;

#ifdef RUSEASM
		MMX_PatchRowBytes(vid.rowbytes);
		ASM_PatchRowBytes(vid.rowbytes);
#endif

	V_Init();
	CV_RegisterVar(&cv_ticrate);

	V_SetPalette(0);
}

// Called at new frame, if the video mode has changed
//
void SCR_Recalc(void)
{
	if(dedicated)
		return;

	// bytes per pixel quick access
	scr_bpp = vid.bpp;

	// scale 1,2,3 times in x and y the patches for the menus and overlays...
	// calculated once and for all, used by routines in v_video.c
	vid.dupx = vid.width / BASEVIDWIDTH;
	vid.dupy = vid.height / BASEVIDHEIGHT;
	vid.fdupx = (float)vid.width / BASEVIDWIDTH;
	vid.fdupy = (float)vid.height / BASEVIDHEIGHT;
	vid.baseratio = FixedDiv(vid.height << FRACBITS, BASEVIDHEIGHT << FRACBITS);

	// patch the asm code depending on vid buffer rowbytes
#ifdef RUSEASM
	ASM_PatchRowBytes(vid.rowbytes);
	MMX_PatchRowBytes(vid.rowbytes);
#endif

	// toggle off automap because some screensize-dependent values will
	// be calculated next time the automap is activated.
	if(automapactive)
		AM_Stop();

	// r_plane stuff: visplanes, openings, floorclip, ceilingclip, spanstart,
	//                spanstop, yslope, distscale, cachedheight, cacheddistance,
	//                cachedxstep, cachedystep
	//             -> allocated at the maximum vidsize, static.

	// r_main: xtoviewangle, allocated at the maximum size.
	// r_things: negonearray, screenheightarray allocated max. size.

	// set the screen[x] ptrs on the new vidbuffers
	V_Init();

	// scr_viewsize doesn't change, neither detailLevel, but the pixels
	// per screenblock is different now, since we've changed resolution.
	R_SetViewSize(); //just set setsizeneeded true now ..

	// vid.recalc lasts only for the next refresh...
	con_recalc = true;
	am_recalc = true;
}

// Check for screen cmd-line parms: to force a resolution.
//
// Set the video mode to set at the 1st display loop (setmodeneeded)
//

void SCR_CheckDefaultMode(void)
{
	int scr_forcex, scr_forcey; // resolution asked from the cmd-line

	if(dedicated)
		return;

	// 0 means not set at the cmd-line
    scr_forcex = scr_forcey = 0;

	if(M_CheckParm("-width") && M_IsNextParm())
		scr_forcex = atoi(M_GetNextParm());

	if(M_CheckParm("-height") && M_IsNextParm())
		scr_forcey = atoi(M_GetNextParm());

	if(scr_forcex && scr_forcey)
	{
		CONS_Printf("Using resolution: %d x %d\n", scr_forcex, scr_forcey);
		// returns -1 if not found, thus will be 0 (no mode change) if not found
		setmodeneeded = VID_GetModeForSize(scr_forcex, scr_forcey) + 1;
	}
	else
	{
		CONS_Printf("Default resolution: %d x %d (%d bits)\n", cv_scr_width.value,
			cv_scr_height.value, cv_scr_depth.value);
		// see note above
		setmodeneeded = VID_GetModeForSize(cv_scr_width.value, cv_scr_height.value) + 1;
	}
}

// sets the modenum as the new default video mode to be saved in the config file
void SCR_SetDefaultMode(void)
{
	// remember the default screen size
	CV_SetValue(&cv_scr_width, vid.width);
	CV_SetValue(&cv_scr_height, vid.height);
	CV_SetValue(&cv_scr_depth, vid.bpp*8);
}

// Change fullscreen on/off according to cv_fullscreen
void SCR_ChangeFullscreen(void)
{
#ifdef DIRECTFULLSCREEN
	// allow_fullscreen is set by VID_PrepareModeList
	// it is used to prevent switching to fullscreen during startup
	if(!allow_fullscreen)
		return;

	if(graphics_started)
	{
		VID_PrepareModeList();
		setmodeneeded = VID_GetModeForSize(vid.width, vid.height) + 1;
	}
	return;
#endif
}
