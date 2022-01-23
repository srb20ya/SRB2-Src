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
/// \brief 16bpp (HIGHCOLOR) span/column drawer functions
/// 
///	NOTE: no includes because this is included as part of r_draw.c

// ==========================================================================
// COLUMNS
// ==========================================================================

/// \brief kick out the upper bit of each component (we're in 5:5:5)
#define HIMASK1 0x7bde

/**	\brief The R_DrawColumn_16 function
	standard upto 128high posts column drawer
*/
void R_DrawColumn_16(void)
{
	int count;
	short* dest;
	fixed_t frac, fracstep;

	count = dc_yh - dc_yl + 1;

	// Zero length, column does not exceed a pixel.
	if(count <= 0)
		return;

#ifdef RANGECHECK
	if((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawColumn_16: %d to %d at %d", dc_yl, dc_yh, dc_x);
#endif

	// Framebuffer destination address.
	// Use ylookup LUT to avoid multiply with ScreenWidth.
	// Use columnofs LUT for subwindows?
	dest = (short*)(ylookup[dc_yl] + columnofs[dc_x]);

	// Determine scaling, which is the only mapping to be done.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery)*fracstep;

	// Inner loop that does the actual texture mapping, e.g. a DDA-like scaling.
	// This is as fast as it gets.

	do
	{
		// Re-map color indices from wall texture column using a lighting/special effects LUT.
		*dest = hicolormaps[((short*)dc_source)[(frac>>FRACBITS)&127]>>1];

		dest += vid.width;
		frac += fracstep;
	} while(--count);
}

/**	\brief The R_DrawSkyColumn_16 function
	LAME cutnpaste: same as R_DrawColumn_16 but wraps around 256
	instead of 128 for the tall sky textures (256x240)
*/
void R_DrawSkyColumn_16(void)
{
	int count;
	short* dest;
	fixed_t frac, fracstep;

	count = dc_yh - dc_yl + 1;

	// Zero length, column does not exceed a pixel.
	if(count <= 0)
		return;

#ifdef RANGECHECK
	if((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawSkyColumn_16: %d to %d at %d", dc_yl, dc_yh, dc_x);
#endif

	dest = (short*)(ylookup[dc_yl] + columnofs[dc_x]);

	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery)*fracstep;

	do
	{
		*dest = hicolormaps[((short*)dc_source)[(frac>>FRACBITS)&255]>>1];

		dest += vid.width;
		frac += fracstep;
	} while(--count);
}

/**	\brief The R_DrawTranslucentColumn_16 function
		LAME cutnpaste: same as R_DrawColumn_16 but does 
		translucent
*/
void R_DrawTranslucentColumn_16(void)
{
	int count;
	short* dest;
	fixed_t frac, fracstep;

	// check out coords for src*
	if((dc_yl < 0) || (dc_x >= vid.width))
		return;

	count = dc_yh - dc_yl;
	if(count < 0)
		return;

#ifdef RANGECHECK
	if((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawTranslucentColumn_16: %d to %d at %d", dc_yl, dc_yh, dc_x);
#endif

	// FIXME. As above.
	dest = (short*)(ylookup[dc_yl] + columnofs[dc_x]);

	// Looks familiar.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery)*fracstep;

	// Here we do an additional index re-mapping.
	do
	{
		*dest = (short)((short)((color8to16[dc_source[frac>>FRACBITS]]>>1) & 0x39ce)
			+ (short)(((*dest & HIMASK1)) & 0x7fff));

		dest += vid.width;
		frac += fracstep;
	} while(count--);
}

/**	\brief The R_DrawTranslatedColumn_16 function
	?
*/
void R_DrawTranslatedColumn_16(void)
{
	int count;
	short* dest;
	fixed_t frac, fracstep;

	count = dc_yh - dc_yl;
	if(count < 0)
		return;

#ifdef RANGECHECK
	if((unsigned)dc_x >= vid.width || dc_yl < 0 || dc_yh >= vid.height)
		I_Error("R_DrawTranslatedColumn_16: %d to %d at %d", dc_yl, dc_yh, dc_x);
#endif

	dest = (short*)(ylookup[dc_yl] + columnofs[dc_x]);

	// Looks familiar.
	fracstep = dc_iscale;
	frac = dc_texturemid + (dc_yl - centery)*fracstep;

	// Here we do an additional index re-mapping.
	do
	{
		*dest = color8to16[dc_colormap[dc_translation[dc_source[frac>>FRACBITS]]]];
		dest += vid.width;

		frac += fracstep;
	} while(count--);
}

// ==========================================================================
// SPANS
// ==========================================================================

/**	\brief The R_*_16 function
	Draws the actual span.
*/
void R_DrawSpan_16(void)
{
	fixed_t xfrac, yfrac;
	short* dest;
	int count, spot;

#ifdef RANGECHECK
	if(ds_x2 < ds_x1 || ds_x1 < 0 || ds_x2 >= vid.width || (unsigned)ds_y > vid.height)
		I_Error("R_DrawSpan_16: %d to %d at %d", ds_x1, ds_x2, ds_y);
#endif

	xfrac = ds_xfrac;
	yfrac = ds_yfrac;

	dest = (short*)(ylookup[ds_y] + columnofs[ds_x1]);

	// We do not check for zero spans here?
	count = ds_x2 - ds_x1;

	if(count <= 0) // We do now!
		return;

	do
	{
		// Current texture index in u, v.
		spot = ((yfrac>>(16-6))&(63*64)) + ((xfrac>>16)&63);

		// Lookup pixel from flat texture tile, re-index using light/colormap.
		*dest++ = hicolormaps[((short*)ds_source)[spot]>>1];

		// Next step in u, v.
		xfrac += ds_xstep;
		yfrac += ds_ystep;
	} while(count--);
}
