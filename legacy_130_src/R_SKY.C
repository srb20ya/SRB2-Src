// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_sky.c,v 1.2 2000/02/27 00:42:10 hurdler Exp $
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
// $Log: r_sky.c,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Sky rendering. The DOOM sky is a texture map like any
//      wall, wrapping around. A 1024 columns equal 360 degrees.
//      The default sky map is 256 columns and repeats 4 times
//      on a 320 screen?
//  
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "r_sky.h"
#include "r_local.h"
#include "w_wad.h"

//
// sky mapping
//
int                     skyflatnum;
int                     skytexture;
int                     skytexturemid;

fixed_t                 skyscale;
int                     skymode=0;  // old (0), new (1) (quick fix!)

//
// R_InitSkyMap called at startup, once.
//
void R_InitSkyMap (void)
{
    // set at P_LoadSectors
    //skyflatnum = R_FlatNumForName ( SKYFLATNAME );

}


//  Setup sky draw for old or new skies (new skies = freelook 256x240)
//
//  Call at loadlevel after skytexture is set
//
//  NOTE: skycolfunc should be set at R_ExecuteSetViewSize ()
//        I dont bother because we don't use low detail no more
//
void R_SetupSkyDraw (void)
{
    texpatch_t*  patches;
    patch_t      wpatch;
    int          count;
    int          height;
    int          i;

    // parse the patches composing sky texture for the tallest one
    // patches are usually RSKY1,RSKY2... and unique

    // note: the TEXTURES lump doesn't have the taller size of Legacy
    //       skies, but the patches it use will give the right size

    count   = textures[skytexture]->patchcount;
    patches = &textures[skytexture]->patches[0];
    for (height=0,i=0;i<count;i++,patches++)
    {
        W_ReadLumpHeader (patches->patch, &wpatch, sizeof(patch_t));
        if (wpatch.height>height)
            height = wpatch.height;
    }

    // DIRTY : should set the routine depending on colormode in screen.c
    if (height>128)
    {
        // horizon line on 256x240 freelook textures of Legacy
        skytexturemid = 200<<FRACBITS;
        skymode = 1;
    }
    else
    {
        // the horizon line in a 256x128 sky texture
        skytexturemid = 100<<FRACBITS;
        skymode = 0;
    }

    // get the right drawer, it was set by screen.c, depending on the
    // current video mode bytes per pixel (quick fix)
    skycolfunc = skydrawerfunc[skymode];

    R_SetSkyScale ();
}


// set the correct scale for the sky at setviewsize
void R_SetSkyScale (void)
{
    //fix this quick mess
    if (skytexturemid>100<<FRACBITS)
    {
        // normal aspect ratio corrected scale
        skyscale = FixedDiv (FRACUNIT, pspriteyscale);
    }
    else
    {
        // double the texture vertically, bleeergh!!
        skyscale = FixedDiv (FRACUNIT, pspriteyscale)>>1;
    }
}
