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
/// \brief System specific interface stuff.

#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

typedef enum
{
	/// Software
	render_soft = 1, 
	/// 3Dfx Glide
	render_glide = 2,
	/// Direct3D
	render_d3d = 3,
	/// OpenGL/MiniGL
	render_opengl = 4, // the same for render_minigl
	/// Dedicated
	render_none = 5  // for dedicated server
} rendermode_t;

/**	\brief currect render mode
*/
extern rendermode_t rendermode;


/**	\brief use highcolor modes if true
*/
extern boolean highcolor;

/**	\brief setup video mode
*/
void I_StartupGraphics(void);

/**	\brief restore old video mode
*/
void I_ShutdownGraphics(void);

/**	\brief	The I_SetPalette function

	\param	palette	Takes full 8 bit values

	\return	void

	
*/
void I_SetPalette(RGBA_t* palette);

/**	\brief return the number of video modes
*/
int VID_NumModes(void);

/**	\brief	The VID_GetModeForSize function

	\param	w	width
	\param	h	height

	\return	vidmode closest to w:h

	
*/
int VID_GetModeForSize(int w, int h);


/**	\brief	The VID_SetMode function
 
	Set the video mode right now,
	the video mode change is delayed until the start of the next refresh
	by setting the setmodeneeded to a value >0
	setup a video mode, this is to be called from the menu


	\param	modenum	video mode to set to

	\return	currect video mode

	
*/
int VID_SetMode(int modenum);

/**	\brief	The VID_GetModeName function

	\param	modenum	video mode number

	\return	name of video mode

	
*/
const char* VID_GetModeName(int modenum);
void VID_PrepareModeList(void); /// note hack for SDL


/**	\brief can video system do fullscreen
*/
extern boolean allow_fullscreen;

/**	\brief Update video system without updating frame
*/
void I_UpdateNoBlit(void);

/**	\brief Update video system with updating frame
*/
void I_FinishUpdate(void);


/**	\brief	Wait for vertical retrace or pause a bit.

	\param	count	max wait

	\return	void

	
*/
void I_WaitVBL(int count);

/**	\brief	The I_ReadScreen function

	\param	scr	buffer to copy screen to

	\return	void

	
*/
void I_ReadScreen(byte* scr);

/**	\brief Start disk icon
*/
void I_BeginRead(void);

/**	\brief Stop disk icon
*/
void I_EndRead(void);

#endif
