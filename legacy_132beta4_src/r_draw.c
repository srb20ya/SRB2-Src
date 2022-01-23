// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: r_draw.c,v 1.13 2001/08/06 23:57:09 stroggonmeth Exp $
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
// $Log: r_draw.c,v $
// Revision 1.13  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.12  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.11  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.10  2001/02/24 13:35:21  bpereira
// no message
//
// Revision 1.9  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.8  2000/11/09 17:56:20  stroggonmeth
// Hopefully fixed a few bugs and did a few optimizations.
//
// Revision 1.7  2000/11/03 03:48:54  stroggonmeth
// Fix a few warnings when compiling.
//
// Revision 1.6  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.5  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.4  2000/04/07 18:47:09  hurdler
// There is still a problem with the asm code and boom colormap
// At least, with this little modif, it compiles on my Linux box
//
// Revision 1.3  2000/04/06 21:06:19  stroggonmeth
// Optimized extra_colormap code...
// Added #ifdefs for older water code.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      span / column drawer functions, for 8bpp and 16bpp
//
//      All drawing to the view buffer is accomplished in this file.
//      The other refresh files only know about ccordinates,
//      not the architecture of the frame buffer.
//      The frame buffer is a linear one, and we need only the base address.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "doomstat.h"
#include "r_local.h"
#include "st_stuff.h"   //added:24-01-98:need ST_HEIGHT
#include "i_video.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"
#include "console.h" //Som: Until I get buffering finished

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

// ==========================================================================
//                     COMMON DATA FOR 8bpp AND 16bpp
// ==========================================================================

byte*           viewimage;
int             viewwidth;
int             scaledviewwidth;
int             viewheight;
int             viewwindowx;
int             viewwindowy;

                // pointer to the start of each line of the screen,
byte*           ylookup[MAXVIDHEIGHT];

byte*           ylookup1[MAXVIDHEIGHT]; // for view1 (splitscreen)
byte*           ylookup2[MAXVIDHEIGHT]; // for view2 (splitscreen)

                 // x byte offset for columns inside the viewwindow
                // so the first column starts at (SCRWIDTH-VIEWWIDTH)/2
int             columnofs[MAXVIDWIDTH];

#ifdef HORIZONTALDRAW
//Fab 17-06-98: horizontal column drawer optimisation
byte*           yhlookup[MAXVIDWIDTH];
int             hcolumnofs[MAXVIDHEIGHT];
#endif

// =========================================================================
//                      COLUMN DRAWING CODE STUFF
// =========================================================================

lighttable_t*           dc_colormap;
int                     dc_x;
int                     dc_yl;
int                     dc_yh;

//Hurdler: 04/06/2000: asm code still use it
//#ifdef OLDWATER
int                     dc_yw;          //added:24-02-98: WATER!
lighttable_t*           dc_wcolormap;   //added:24-02-98:WATER!
//#endif

fixed_t                 dc_iscale;
fixed_t                 dc_texturemid;

byte*                   dc_source;

// -----------------------
// translucency stuff here
// -----------------------
#define NUMTRANSTABLES  5     // how many translucency tables are used

byte*                   transtables;    // translucency tables

// R_DrawTransColumn uses this
byte*                   dc_transmap;    // one of the translucency tables


// ----------------------
// translation stuff here
// ----------------------

byte*                   translationtables[MAXSKINS]; // Tails
byte*					defaulttranslationtables; // Tails 08-20-2002

// R_DrawTranslatedColumn uses this
byte*                   dc_translation;

struct r_lightlist_s*   dc_lightlist = NULL;
int                     dc_numlights = 0;
int                     dc_maxlights;

int     dc_texheight;

// =========================================================================
//                      SPAN DRAWING CODE STUFF
// =========================================================================

int                     ds_y;
int                     ds_x1;
int                     ds_x2;

lighttable_t*           ds_colormap;

fixed_t                 ds_xfrac;
fixed_t                 ds_yfrac;
fixed_t                 ds_xstep;
fixed_t                 ds_ystep;

byte*                   ds_source;      // start of a 64*64 tile image
byte*                   ds_transmap;    // one of the translucency tables

// Variable flat sizes Tails 05-14-2003
int flatsize;
int flatmask;
int flatsubtract;


// ==========================================================================
//                        OLD DOOM FUZZY EFFECT
// ==========================================================================

//
// Spectre/Invisibility.
//
#define FUZZTABLE     50
#define FUZZOFF       (1)

static  int fuzzoffset[FUZZTABLE] =
{
    FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};

static  int fuzzpos = 0;     // move through the fuzz table


//  fuzzoffsets are dependend of vid width, for optimising purpose
//  this is called by SCR_Recalc() whenever the screen size changes
//
void R_RecalcFuzzOffsets (void)
{
    int i;
    for (i=0;i<FUZZTABLE;i++)
    {
        fuzzoffset[i] = (fuzzoffset[i] < 0) ? -vid.width : vid.width;
    }
}


// =========================================================================
//                   TRANSLATION COLORMAP CODE
// =========================================================================

char *Color_Names[MAXSKINCOLORS]={
   "Default",
   "Grey" ,
   "Peach", // Tails 02-19-2000
   "dark_red"  ,
   "Silver" , // Tails 02-19-2000
   "Orange", // Tails 02-19-2000
   "red"  ,
   "blue" ,
   "dark_blue"       ,
   "Pink"  , // Tails 02-19-2000
   "Beige",
   "Purple", // By request of Matrixx Hedgehog. Tails 04-07-2002
   "green", // REAL green
   "White", // White (also used for fireflower) Tails 09-09-2002
   "Black", // Tails 06-21-2003
   "Yellow", // By insane popular demand, it returns.
};

CV_PossibleValue_t Color_cons_t[]={{0,NULL},{1,NULL},{2,NULL},{3,NULL},
                                   {4,NULL},{5,NULL},{6,NULL},{7,NULL},
{8,NULL},{9,NULL},{10,NULL},{11,NULL},{12, NULL},{13, NULL},{14, NULL},{15, NULL}, {0,NULL}};

//  Creates the translation tables to map the green color ramp to
//  another ramp (gray, brown, red, ...)
//
//  This is precalculated for drawing the player sprites in the player's
//  chosen color
//
void R_LoadSkinTable(void) // Tails 12-21-2003
{
	int i;
	
	for(i=0; i<MAXSKINS; i++)
		translationtables[i] = Z_MallocAlign (256*(MAXSKINCOLORS-1), PU_STATIC, 0, 16);
}
void R_InitTranslationTables (void)
{
    int         i,j;

    //added:11-01-98: load here the transparency lookup tables 'TINTTAB'
    // NOTE: the TINTTAB resource MUST BE aligned on 64k for the asm optimised
    //       (in other words, transtables pointer low word is 0)
    transtables = Z_MallocAlign (NUMTRANSTABLES*0x10000, PU_STATIC, 0, 16);

    // load in translucency tables
    W_ReadLump( W_GetNumForName("TRANSMED"), transtables );
    W_ReadLump( W_GetNumForName("TRANSMOR"), transtables+0x10000 );
    W_ReadLump( W_GetNumForName("TRANSHI"),  transtables+0x20000 );
    W_ReadLump( W_GetNumForName("TRANSFIR"), transtables+0x30000 );
    W_ReadLump( W_GetNumForName("TRANSFX1"), transtables+0x40000 );

	// The old "default" transtable for thok mobjs and such Tails 08-20-2002
	defaulttranslationtables = Z_MallocAlign (256*(MAXSKINCOLORS), PU_STATIC, 0, 16);

    // Translate the colors specified by the skin information.
    for (i=0 ; i<256 ; i++)
    {
        if (i >= 112 && i <= 127)
        {
            // map green ramp to gray, brown, red
            defaulttranslationtables [i      ] = 0x60 + (i&0xf); // Grey
            defaulttranslationtables [i+  256] = 0x30 + (i&0xf); // Peach // Tails 02-19-2000
            defaulttranslationtables [i+2*256] = 0x20 + (i&0xf); // Dark Red
                
            // added 9-2-98
			defaulttranslationtables [i+3*256] = 0x50 + (i&0xf); // silver // tails 02-19-2000
			defaulttranslationtables [i+4*256] = 0xd0 + (i&0xf); // orange // tails 02-19-2000
			defaulttranslationtables [i+5*256] = 0xb0 + (i&0xf); // light red
			defaulttranslationtables [i+6*256] = 0xc0 + (i&0xf); // light blue
                
            if ((i&0xf) <9)
                defaulttranslationtables [i+7*256] = 0xc7 + (i&0xf);   // dark blue
            else
                defaulttranslationtables [i+7*256] = 0xf0-9 + (i&0xf);
                
			defaulttranslationtables [i+8*256] = 0x10 + (i&0xf); // pink // tails 02-19-2000
                
            defaulttranslationtables [i+9*256] = 0x80 + (i&0xf);     // beige

			// purple Tails 04-07-2002
			switch(i&0xf)
			{
			case 0:
			case 1:
			case 2:
				defaulttranslationtables [i+10*256] = 0xfa;
				break;
			case 3:
			case 4:
			case 5:
				defaulttranslationtables [i+10*256] = 0xfb;
				break;
			case 6:
			case 7:
			case 8:
				defaulttranslationtables [i+10*256] = 0xfc;
				break;
			case 9:
			case 10:
			case 11:
				defaulttranslationtables [i+10*256] = 0xfd;
				break;
			default:
				defaulttranslationtables [i+10*256] = 0xfe;
				break;
			}

			// Green
            defaulttranslationtables [i+11*256] = 0x70 + (i&0xf);
			
			// White Tails 09-09-2002
			switch(i&0xf)
			{
			case 0:
			case 1:
			case 2:
				defaulttranslationtables [i+12*256] = 0x04;
				break;
			case 3:
			case 4:
			case 5:
				defaulttranslationtables [i+12*256] = 0x30;
				break;
			case 6:
			case 7:
			case 8:
				defaulttranslationtables [i+12*256] = 0x50;
				break;
			case 9:
			case 10:
			case 11:
				defaulttranslationtables [i+12*256] = 0x31;
				break;
			default:
				defaulttranslationtables [i+12*256] = 0x51;
				break;
			}

			// Black Tails 06-21-2003
			switch(i&0xf)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				defaulttranslationtables [i+13*256] = 0x68 + (i&0xf);
				break;
			case 8:
				defaulttranslationtables [i+13*256] = 0x05;
				break;
			case 9:
				defaulttranslationtables [i+13*256] = 0x06;
				break;
			case 10:
				defaulttranslationtables [i+13*256] = 0x07;
				break;
			case 11:
				defaulttranslationtables [i+13*256] = 0x08;
				break;
			default:
				defaulttranslationtables [i+13*256] = 0x00;
				break;
			}

			// Yellow Tails 12-21-2002
			switch(i&0xf)
			{
				case 0:
				case 1:
					defaulttranslationtables [i+14*256] = 0xa0;   // yellow
					break;
				case 2:
				case 3:
					defaulttranslationtables [i+14*256] = 0xa1;   // yellow
					break;
				case 4:
				case 5:
					defaulttranslationtables [i+14*256] = 0xa2;   // yellow
					break;
				case 6:
				case 7:
					defaulttranslationtables [i+14*256] = 0xa3;   // yellow
					break;
				case 8:
				case 9:
					defaulttranslationtables [i+14*256] = 0xa4;   // yellow
					break;
				case 10:
				case 11:
					defaulttranslationtables [i+14*256] = 0xa5;   // yellow
					break;
				case 12:
				case 13:
					defaulttranslationtables [i+14*256] = 0xa6;   // yellow
					break;
				default:
					defaulttranslationtables [i+14*256] = 0xa7;   // yellow
					break;
			}
        }
        else
        {
            // Keep all other colors as is.
            for (j=0;j<(MAXSKINCOLORS)*256;j+=256)
                defaulttranslationtables [i+j] = i;
        }
    }
}

// Allow skins to choose which color is translated! Tails 06-07-2002
void R_InitSkinTranslationTables(int starttranscolor, int endtranscolor, int skinnum)
{
    int         i,j;

    // Translate the colors specified by the skin information.
    for (i=0 ; i<256 ; i++)
    {
        if (i >= starttranscolor && i <= endtranscolor)
        {
            // map green ramp to gray, brown, red
            translationtables[skinnum] [i      ] = 0x60 + (i&0xf); // Grey
            translationtables[skinnum] [i+  256] = 0x30 + (i&0xf); // Peach // Tails 02-19-2000
            translationtables[skinnum] [i+2*256] = 0x20 + (i&0xf); // Dark Red
                
            // added 9-2-98
			translationtables[skinnum] [i+3*256] = 0x50 + (i&0xf); // silver // tails 02-19-2000
			translationtables[skinnum] [i+4*256] = 0xd0 + (i&0xf); // orange // tails 02-19-2000
			translationtables[skinnum] [i+5*256] = 0xb0 + (i&0xf); // light red
			translationtables[skinnum] [i+6*256] = 0xc0 + (i&0xf); // light blue
                
            if ((i&0xf) <9)
                translationtables[skinnum] [i+7*256] = 0xc7 + (i&0xf);   // dark blue
            else
                translationtables[skinnum] [i+7*256] = 0xf0-9 + (i&0xf);
                
			translationtables[skinnum] [i+8*256] = 0x10 + (i&0xf); // pink // tails 02-19-2000
                
            translationtables[skinnum] [i+9*256] = 0x80 + (i&0xf);     // beige

			// purple Tails 04-07-2002
			switch(i&0xf)
			{
			case 0:
			case 1:
			case 2:
				translationtables[skinnum] [i+10*256] = 0xfa;
				break;
			case 3:
			case 4:
			case 5:
				translationtables[skinnum] [i+10*256] = 0xfb;
				break;
			case 6:
			case 7:
			case 8:
				translationtables[skinnum] [i+10*256] = 0xfc;
				break;
			case 9:
			case 10:
			case 11:
				translationtables[skinnum] [i+10*256] = 0xfd;
				break;
			default:
				translationtables[skinnum] [i+10*256] = 0xfe;
				break;
			}

			// Green
            translationtables[skinnum] [i+11*256] = 0x70 + (i&0xf);

			// White Tails 09-09-2002
			switch(i&0xf)
			{
			case 0:
			case 1:
			case 2:
				translationtables[skinnum] [i+12*256] = 0x04;
				break;
			case 3:
			case 4:
			case 5:
				translationtables[skinnum] [i+12*256] = 0x30;
				break;
			case 6:
			case 7:
			case 8:
				translationtables[skinnum] [i+12*256] = 0x50;
				break;
			case 9:
			case 10:
			case 11:
				translationtables[skinnum] [i+12*256] = 0x31;
				break;
			default:
				translationtables[skinnum] [i+12*256] = 0x51;
				break;
			}

			// Black Tails 06-21-2003
			switch(i&0xf)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				translationtables[skinnum] [i+13*256] = 0x68 + (i&0xf);
				break;
			case 8:
				translationtables[skinnum] [i+13*256] = 0x05;
				break;
			case 9:
				translationtables[skinnum] [i+13*256] = 0x06;
				break;
			case 10:
				translationtables[skinnum] [i+13*256] = 0x07;
				break;
			case 11:
				translationtables[skinnum] [i+13*256] = 0x08;
				break;
			default:
				translationtables[skinnum] [i+13*256] = 0x00;
				break;
			}
/*
			// Yellow
			if ((i&0xf) <8)
                translationtables[skinnum] [i+14*256] = 0xe0 + (i&0xf);
            else
                translationtables[skinnum] [i+14*256] = 0xa0-8 + (i&0xf);
*/
			// Yellow Tails 12-21-2002
			switch(i&0xf)
			{
				case 0:
				case 1:
					translationtables[skinnum] [i+14*256] = 0xa0;   // yellow
					break;
				case 2:
				case 3:
					translationtables[skinnum] [i+14*256] = 0xa1;   // yellow
					break;
				case 4:
				case 5:
					translationtables[skinnum] [i+14*256] = 0xa2;   // yellow
					break;
				case 6:
				case 7:
					translationtables[skinnum] [i+14*256] = 0xa3;   // yellow
					break;
				case 8:
				case 9:
					translationtables[skinnum] [i+14*256] = 0xa4;   // yellow
					break;
				case 10:
				case 11:
					translationtables[skinnum] [i+14*256] = 0xa5;   // yellow
					break;
				case 12:
				case 13:
					translationtables[skinnum] [i+14*256] = 0xa6;   // yellow
					break;
				default:
					translationtables[skinnum] [i+14*256] = 0xa7;   // yellow
					break;
			}

        }
        else
        {
            // Keep all other colors as is.
            for (j=0;j<(MAXSKINCOLORS-1)*256;j+=256)
                translationtables[skinnum] [i+j] = i;
        }
    }
}


// ==========================================================================
//               COMMON DRAWER FOR 8 AND 16 BIT COLOR MODES
// ==========================================================================

// in a perfect world, all routines would be compatible for either mode,
// and optimised enough
//
// in reality, the few routines that can work for either mode, are
// put here


// R_InitViewBuffer
// Creates lookup tables for getting the framebuffer address
//  of a pixel to draw.
//
void R_InitViewBuffer ( int   width,
                        int   height )
{
    int         i;
    int         bytesperpixel = vid.bpp;

    if (bytesperpixel<1 || bytesperpixel>4)
        I_Error ("R_InitViewBuffer : wrong bytesperpixel value %d\n",
                 bytesperpixel);

    // Handle resize,
    //  e.g. smaller view windows
    //  with border and/or status bar.
    viewwindowx = (vid.width-width) >> 1;

    // Column offset for those columns of the view window, but
    // relative to the entire screen
    for (i=0 ; i<width ; i++)
        columnofs[i] = (viewwindowx + i) * bytesperpixel;

    // Same with base row offset.
    if (width == vid.width)
        viewwindowy = 0;
    else
        viewwindowy = (vid.height-height) >> 1;

    // Precalculate all row offsets.
    for (i=0 ; i<height ; i++)
    {
        ylookup[i] = ylookup1[i] = vid.buffer + (i+viewwindowy)*vid.width*bytesperpixel;
                     ylookup2[i] = vid.buffer + (i+(vid.height>>1))*vid.width*bytesperpixel; // for splitscreen
    }
        

#ifdef HORIZONTALDRAW
    //Fab 17-06-98
    // create similar lookup tables for horizontal column draw optimisation

    // (the first column is the bottom line)
    for (i=0; i<width; i++)
        yhlookup[i] = screens[2] + ((width-i-1) * bytesperpixel * height);

    for (i=0; i<height; i++)
        hcolumnofs[i] = i * bytesperpixel;
#endif
}


//
// Store the lumpnumber of the viewborder patches.
//
int viewborderlump[8];
void R_InitViewBorder (void)
{
    viewborderlump[BRDR_T]  = W_GetNumForName ("brdr_t");
    viewborderlump[BRDR_B]  = W_GetNumForName ("brdr_b");
    viewborderlump[BRDR_L]  = W_GetNumForName ("brdr_l");
    viewborderlump[BRDR_R]  = W_GetNumForName ("brdr_r");
    viewborderlump[BRDR_TL] = W_GetNumForName ("brdr_tl");
    viewborderlump[BRDR_BL] = W_GetNumForName ("brdr_bl");
    viewborderlump[BRDR_TR] = W_GetNumForName ("brdr_tr");
    viewborderlump[BRDR_BR] = W_GetNumForName ("brdr_br");
}


//
// R_FillBackScreen
// Fills the back screen with a pattern for variable screen sizes
// Also draws a beveled edge.
//
void R_FillBackScreen (void)
{
    byte*       src;
    byte*       dest;
    int         x;
    int         y;
    patch_t*    patch;
    int         step,boff; 
    
    //faB: quickfix, don't cache lumps in both modes
    if (rendermode!=render_soft)
        return;

     //added:08-01-98:draw pattern around the status bar too (when hires),
    //                so return only when in full-screen without status bar.
    if ((scaledviewwidth == vid.width)&&(viewheight==vid.height))
        return;

    src  = scr_borderpatch;
    dest = screens[1];

    for (y=0 ; y<vid.height ; y++)
    {
        for (x=0 ; x<vid.width/64 ; x++)
        {
            memcpy (dest, src+((y&63)<<6), 64);
            dest += 64;
        }

        if (vid.width&63)
        {
            memcpy (dest, src+((y&63)<<6), vid.width&63);
            dest += (vid.width&63);
        }
    }

    //added:08-01-98:dont draw the borders when viewwidth is full vid.width.
    if (scaledviewwidth == vid.width)
       return;
    
    step = 8;
    boff = 8;

    patch = W_CacheLumpNum (viewborderlump[BRDR_T],PU_CACHE);
    for (x=0 ; x<scaledviewwidth ; x+=step)
        V_DrawPatch (viewwindowx+x,viewwindowy-boff,1,patch);
    patch = W_CacheLumpNum (viewborderlump[BRDR_B],PU_CACHE);
    for (x=0 ; x<scaledviewwidth ; x+=step)
        V_DrawPatch (viewwindowx+x,viewwindowy+viewheight,1,patch);
    patch = W_CacheLumpNum (viewborderlump[BRDR_L],PU_CACHE);
    for (y=0 ; y<viewheight ; y+=step)
        V_DrawPatch (viewwindowx-boff,viewwindowy+y,1,patch);
    patch = W_CacheLumpNum (viewborderlump[BRDR_R],PU_CACHE);
    for (y=0 ; y<viewheight ; y+=step)
        V_DrawPatch (viewwindowx+scaledviewwidth,viewwindowy+y,1,patch);

    // Draw beveled corners.
    V_DrawPatch (viewwindowx-boff,
                 viewwindowy-boff,
                 1,
                 W_CacheLumpNum (viewborderlump[BRDR_TL],PU_CACHE));

    V_DrawPatch (viewwindowx+scaledviewwidth,
                 viewwindowy-boff,
                 1,
                 W_CacheLumpNum (viewborderlump[BRDR_TR],PU_CACHE));

    V_DrawPatch (viewwindowx-boff,
                 viewwindowy+viewheight,
                 1,
                 W_CacheLumpNum (viewborderlump[BRDR_BL],PU_CACHE));

    V_DrawPatch (viewwindowx+scaledviewwidth,
                 viewwindowy+viewheight,
                 1,
                 W_CacheLumpNum (viewborderlump[BRDR_BR],PU_CACHE));
}


//
// Copy a screen buffer.
//
void R_VideoErase (unsigned ofs, int count)
{
    // LFB copy.
    // This might not be a good idea if memcpy
    //  is not optiomal, e.g. byte by byte on
    //  a 32bit CPU, as GNU GCC/Linux libc did
    //  at one point.
    memcpy (screens[0]+ofs, screens[1]+ofs, count);
}


//
// R_DrawViewBorder
// Draws the border around the view
//  for different size windows?
//
void R_DrawViewBorder (void)
{
    int         top;
    int         side;
    int         ofs;

#ifdef HWRENDER // not win32 only 19990829 by Kin
    if (rendermode != render_soft)
    {
        HWR_DrawViewBorder (0);
        return;
    }
#endif


#ifdef DEBUG
    fprintf(stderr,"RDVB: vidwidth %d vidheight %d scaledviewwidth %d viewheight %d\n",
             vid.width,vid.height,scaledviewwidth,viewheight);
#endif

     //added:08-01-98: draw the backtile pattern around the status bar too
    //                 (when statusbar width is shorter than vid.width)
    /*
    if( (vid.width>ST_WIDTH) && (vid.height!=viewheight) )
    {
        ofs  = (vid.height-stbarheight)*vid.width;
        side = (vid.width-ST_WIDTH)>>1;
        R_VideoErase(ofs,side);

        ofs += (vid.width-side);
        for (i=1;i<stbarheight;i++)
        {
            R_VideoErase(ofs,side<<1);  //wraparound right to left border
            ofs += vid.width;
        }
        R_VideoErase(ofs,side);
    }*/

    if (scaledviewwidth == vid.width)
        return;

    top  = (vid.height-viewheight) >>1;
    side = (vid.width-scaledviewwidth) >>1;

    // copy top and one line of left side
    R_VideoErase (0, top*vid.width+side);

    // copy one line of right side and bottom
    ofs = (viewheight+top)*vid.width-side;
    R_VideoErase (ofs, top*vid.width+side);

    // copy sides using wraparound
    ofs = top*vid.width + vid.width-side;
    side <<= 1;

    //added:05-02-98:simpler using our new VID_Blit routine
    VID_BlitLinearScreen(screens[1]+ofs, screens[0]+ofs,
                         side, viewheight-1, vid.width, vid.width);

    // useless, old dirty rectangle stuff
    //V_MarkRect (0,0,vid.width, vid.height-stbarheight);
}


// ==========================================================================
//                   INCLUDE 8bpp DRAWING CODE HERE
// ==========================================================================

#include "r_draw8.c"


// ==========================================================================
//                   INCLUDE 16bpp DRAWING CODE HERE
// ==========================================================================

#include "r_draw16.c"
