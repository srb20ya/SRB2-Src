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
/// \brief Sky rendering
/// 
///	The SRB2 sky is a texture map like any
///	wall, wrapping around. A 1024 columns equal 360 degrees.
///	The default sky map is 256 columns and repeats 4 times
///	on a 320 screen?

#include "doomdef.h"
#include "r_sky.h"
#include "r_local.h"
#include "w_wad.h"
#include "z_zone.h"

#include "p_maputl.h" // P_PointOnLineSide

//
// sky mapping
//

/**	\brief Needed to store the number of the dummy sky flat.
	Used for rendering, as well as tracking projectiles etc.
*/
fixed_t skyflatnum;

/**	\brief the lump number of the sky texture
*/
fixed_t skytexture;

/**	\brief the horizon line in a 256x128 sky texture
*/
fixed_t skytexturemid;

/**	\brief the scale of the sky
*/
fixed_t skyscale;

/** \brief used for keeping track of the current sky
*/
int levelskynum;
int globallevelskynum;

/**	\brief	The R_SetupSkyDraw function


	Setup sky draw for old or new skies (new skies = freelook 256x240)
 
	Call at loadlevel after skytexture is set
	
	\warning skycolfunc should be set at R_ExecuteSetViewSize()
	I don't bother because we don't use low detail anymore

	\return	void

	
*/
void R_SetupSkyDraw(void)
{
	texpatch_t* patches;
	patch_t wpatch;
	int count, height, i;

	// parse the patches composing sky texture for the tallest one
	// patches are usually RSKY1,RSKY2... and unique

	// note: the TEXTURES lump doesn't have the taller size of SRB2
	//       skies, but the patches it uses will give the right size

	count = textures[skytexture]->patchcount;
	patches = &textures[skytexture]->patches[0];
	for(height = 0, i = 0; i < count; i++, patches++)
    {
		W_ReadLumpHeader(patches->patch, &wpatch, sizeof(patch_t));
		wpatch.height = SHORT(wpatch.height);
		if(wpatch.height > height)
			height = wpatch.height;
	}

	// the horizon line in a 256x128 sky texture
	skytexturemid = (textures[skytexture]->height/2)<<FRACBITS;

	// get the right drawer, it was set by screen.c, depending on the
	// current video mode bytes per pixel (quick fix)
	skycolfunc = skydrawerfunc;

	R_SetSkyScale();
}

/**	\brief	The R_SetSkyScale function
 
	set the correct scale for the sky at setviewsize
 
	\return void
*/
void R_SetSkyScale(void)
{
	skyscale = FixedDiv(FRACUNIT/2, pspriteyscale)<<1;
}




