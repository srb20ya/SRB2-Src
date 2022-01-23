// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_draw.c,v 1.7 2000/04/27 17:48:47 hurdler Exp $
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
//
//
// $Log: hw_draw.c,v $
// Revision 1.7  2000/04/27 17:48:47  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.6  2000/04/24 15:22:47  hurdler
// Support colormap for text
//
// Revision 1.5  2000/04/23 00:30:47  hurdler
// fix a small bug in skin color
//
// Revision 1.4  2000/04/22 21:08:23  hurdler
// I like it better like that
//
// Revision 1.3  2000/04/14 16:34:26  hurdler
// some nice changes for coronas
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      miscellaneous drawing (mainly 2d)
//
//-----------------------------------------------------------------------------


#include "hw_glob.h"
#include "hw_drv.h"

#include "../m_misc.h"      //FIL_WriteFile()
#include "../r_draw.h"      //viewborderlump
#include "../w_wad.h"
#include "../z_zone.h"
#include "../v_video.h"

#ifndef NORMALUNIX // unix does not need this 19991024 by Kin
#include <io.h>
#else
#define O_BINARY 0
#endif // normalunix
#include <fcntl.h>
#include "../i_video.h"  // for rendermode != render_glide

float   gr_patch_scalex;
float   gr_patch_scaley;

static  BOOL    gr_scale_patch;     // HWR_DrawPatch() scaling state

#ifdef WIN32
#pragma pack(1)
#endif
typedef struct {  // sizeof() = 18
  char  id_field_length;
  char  color_map_type;
  char  image_type;
  char  dummy[5];
/*short c_map_origin;
  short c_map_length;
  char  c_map_size;*/
  short x_origin;
  short y_origin;
  short width;
  short height;
  char  image_pix_size;
  char  image_descriptor;
} TGAHeader, *PTGAHeader;
#ifdef WIN32
#pragma pack()
#endif
typedef unsigned char GLRGB[3];
void saveTGA(char *file_name, int width, int height, GLRGB *buffer);

//
// Set current scaling state for HWR_DrawPatch()
//
void HWR_ScalePatch ( BOOL bScalePatch )
{
    gr_scale_patch = bScalePatch;
}

// 
// -----------------+
// HWR_DrawPatch    : Draw a 'tile' graphic
// Notes            : x,y : positions relative to the original Doom resolution
//                  : textes(console+score) + menus + status bar
// -----------------+
void HWR_DrawPatch (GlidePatch_t* gpatch, int x, int y)
{
    FOutVector      v[4];

    // make patch ready in 3Dfx cache
    HWR_GetPatch (gpatch);

//  3--2
//  | /|
//  |/ |
//  0--1
    if ( gr_scale_patch ) {
        v[0].x = v[3].x = (x-gpatch->leftoffset-160.0f)/160.0f;
        v[2].x = v[1].x = (x-gpatch->leftoffset+gpatch->width-160.0f)/160.0f;
        v[0].y = v[1].y = -(y-gpatch->topoffset-100.0f)/100.0f;
        v[2].y = v[3].y = -(y-gpatch->topoffset+gpatch->height-100.0f)/100.0f;
    }
    else {
        v[0].x = v[3].x = ((x-gpatch->leftoffset-vid.width/2.0f)/(vid.width/2.0f));
        v[2].x = v[1].x = ((x-gpatch->leftoffset+gpatch->width-vid.width/2.0f)/(vid.width/2.0f));
        v[0].y = v[1].y = -((y-gpatch->topoffset-vid.height/2.0f)/(vid.height/2.0f));
        v[2].y = v[3].y = -((y-gpatch->topoffset+gpatch->height-vid.height/2.0f)/(vid.height/2.0f));
    }
    v[0].oow = v[1].oow = v[2].oow = v[3].oow = 1.0f;

    v[0].sow = v[3].sow = 0.0f;
    v[2].sow = v[1].sow = gpatch->max_s;
    v[0].tow = v[1].tow = 0.0f;
    v[2].tow = v[3].tow = gpatch->max_t;

    // clip it since it is used for bunny scroll in doom I
    HWD.pfnDrawPolygon( NULL, v, 4, PF_Environment | PF_Clip | PF_NoZClip | PF_NoDepthTest);
}


// 
// HWR_DrawMappedPatch(): Like HWR_DrawPatch but with translated color
//
void HWR_DrawMappedPatch (GlidePatch_t* gpatch, int x, int y, byte *colormap)
{
    FOutVector      v[4];

    // make patch ready in 3Dfx cache
    HWR_GetMappedPatch (gpatch, colormap);

    if ( gr_scale_patch ) {
        v[0].x = v[3].x = (x-gpatch->leftoffset-160.0f)/160.0f;
        v[2].x = v[1].x = (x-gpatch->leftoffset+gpatch->width-160.0f)/160.0f;
        v[0].y = v[1].y = -(y-gpatch->topoffset-100.0f)/100.0f;
        v[2].y = v[3].y = -(y-gpatch->topoffset+gpatch->height-100.0f)/100.0f;
    }
    else {
        v[0].x = v[3].x = ((x-gpatch->leftoffset-vid.width/2.0f)/(vid.width/2.0f));
        v[2].x = v[1].x = ((x-gpatch->leftoffset+gpatch->width-vid.width/2.0f)/(vid.width/2.0f));
        v[0].y = v[1].y = -((y-gpatch->topoffset-vid.height/2.0f)/(vid.height/2.0f));
        v[2].y = v[3].y = -((y-gpatch->topoffset+gpatch->height-vid.height/2.0f)/(vid.height/2.0f));
    }
    v[0].oow = v[1].oow = v[2].oow = v[3].oow = 1.0f;

    v[0].sow = v[3].sow = 0.0f;
    v[2].sow = v[1].sow = gpatch->max_s;
    v[0].tow = v[1].tow = 0.0f;
    v[2].tow = v[3].tow = gpatch->max_t;

    // clip it since it is used for bunny scroll in doom I
    HWD.pfnDrawPolygon( NULL, v, 4, PF_Environment | PF_Clip | PF_NoZClip | PF_NoDepthTest);
}


// this one doesn't scale at all
// Hurdler: ca fait quoi cette proc???
// BP: Bein sa draw des patch non scaled (scale = retresisement ou agrendisement)!
// Hurdler: mouahahah... zonder big blague? 
//          Are you sure this function is called by another one?
void HWR_DrawNonScaledPatch (GlidePatch_t* gpatch, int x, int y)
{
    FOutVector      v[4];
    int             x2,y2;

    // make patch ready in 3Dfx cache
    HWR_GetPatch (gpatch);

//  3--2
//  | /|
//  |/ | 
//  0--1
    
    x -= gpatch->leftoffset;
    y -= gpatch->topoffset;
    x2 = x + gpatch->width;
    y2 = y + gpatch->height;

    v[0].x = v[3].x = (float)x;
    v[2].x = v[1].x = (float)x2;
    v[0].y = v[1].y = -(float)y;
    v[2].y = v[3].y = -(float)y2;

    v[0].oow = v[1].oow = v[2].oow = v[3].oow = 1.0f;

    v[0].sow = v[3].sow = 0;//.5f;
    v[2].sow = v[1].sow = gpatch->max_s;//+0.5f;
    v[0].tow = v[1].tow = 0;//.5f;
    v[2].tow = v[3].tow = gpatch->max_t;//+0.5f;

    HWD.pfnDrawPolygon( NULL, v, 4, PF_Translucent );
}

void HWR_DrawPic(int x, int y, int lumpnum)
{
    FOutVector      v[4];
    GlidePatch_t    *patch;

    // make pic ready in 3Dfx cache
    patch = HWR_GetPic( lumpnum );
    
//  3--2
//  | /|
//  |/ |
//  0--1
    
    v[0].x = v[3].x = (x - 160.0f)/160.0f;
    v[2].x = v[1].x = ((x+patch->width) - 160.0f)/160.0f;
    v[0].y = v[1].y = -(y - 100.0f)/100.0f;
    v[2].y = v[3].y = -((y+patch->height) - 100.0f)/100.0f;

    v[0].oow = v[1].oow = v[2].oow = v[3].oow = 1.0f;

    v[0].sow = v[3].sow =  0;
    v[2].sow = v[1].sow =  1.0;
    v[0].tow = v[1].tow =  0;
    v[2].tow = v[3].tow =  1.0;

    //Hurdler: Boris, the same comment as above... but maybe for pics
    // it not a problem since they don't have any transparent pixel
    // if I'm right !? 
    // But then, the question is: why not 0 instead of PF_Masked ?
    // or maybe PF_Environment ??? (like what I said above)
    HWD.pfnDrawPolygon( NULL, v, 4, PF_Masked); //PF_Translucent );
}

// ==========================================================================
//                                                            V_VIDEO.C STUFF
// ==========================================================================


// --------------------------------------------------------------------------
// Fills a box of pixels using a flat texture as a pattern
// --------------------------------------------------------------------------
void HWR_DrawFlatFill (int x, int y, int w, int h, int flatlumpnum)
{
    FOutVector  v[4];

//  3--2
//  | /|
//  |/ |
//  0--1

    v[0].x = v[3].x = (x - 160.0f)/160.0f;
    v[2].x = v[1].x = ((x+w) - 160.0f)/160.0f;
    v[0].y = v[1].y = -(y - 100.0f)/100.0f;
    v[2].y = v[3].y = -((y+h) - 100.0f)/100.0f;

    v[0].oow = v[1].oow = v[2].oow = v[3].oow = 1.0f;

    // flat is 64x64 lod and texture offsets are [0.0, 1.0]
    v[0].sow = v[3].sow = (x & 63)/64.0f;
    v[2].sow = v[1].sow = v[0].sow + w/64.0f;
    v[0].tow = v[1].tow = (y & 63)/64.0f;
    v[2].tow = v[3].tow = v[0].tow + h/64.0f;

    HWR_GetFlat (flatlumpnum);

    //Hurdler: Boris, the same comment as above... but maybe for pics
    // it not a problem since they don't have any transparent pixel
    // if I'm right !?
    // BTW, I see we put 0 for PFs, and If I'm right, that
    // means we take the previous PFs as default
    // how can we be sure they are ok?
    HWD.pfnDrawPolygon( NULL, v, 4, 0); //PF_Translucent );
}


// --------------------------------------------------------------------------
// Fade down the screen so that the menu drawn on top of it looks brighter
// --------------------------------------------------------------------------
//  3--2
//  | /|
//  |/ |
//  0--1
void HWR_FadeScreenMenuBack( unsigned long color, int height )
{
    FOutVector  v[4];
    FSurfaceInfo Surf;

    // setup some neat-o translucency effect
    if (!height) //cool hack 0 height is full height
        height = VIDHEIGHT;

    v[0].x = v[3].x = -1.0f;
    v[2].x = v[1].x =  1.0f;
    v[0].y = v[1].y =  1.0f-((height<<1)/(float)VIDHEIGHT);
    v[2].y = v[3].y =  1.0f;
    v[0].oow = v[1].oow = v[2].oow = v[3].oow = 1.0f;
    
    v[0].sow = v[3].sow = 0.0f;
    v[2].sow = v[1].sow = 1.0f;
    v[0].tow = v[1].tow = 1.0f;
    v[2].tow = v[3].tow = 0.0f;

    Surf.FlatColor.rgba = UINT2RGBA(color);
    Surf.FlatColor.s.alpha = 0xff/2;
    HWD.pfnDrawPolygon( &Surf, v, 4, PF_NoTexture|PF_Modulated|PF_Translucent|PF_NoDepthTest|PF_Debug);
}


// ==========================================================================
//                                                             R_DRAW.C STUFF
// ==========================================================================

// ------------------
// HWR_DrawViewBorder 
// Fill the space around the view window with a Doom flat texture, draw the
// beveled edges.
// 'clearlines' is useful to clear the heads up messages, when the view
// window is reduced, it doesn't refresh all the view borders.
// ------------------
extern int st_borderpatchnum;
void HWR_DrawViewBorder (int clearlines)
{
    int         x,y;
    int         top,side;
    int         baseviewwidth,baseviewheight;
    int         basewindowx,basewindowy;
    GlidePatch_t*   patch;

// comment by hurdler
//    if (gr_viewwidth == VIDWIDTH)
//        return;

    if (!clearlines)
        clearlines = BASEVIDHEIGHT; //refresh all

    // calc view size based on original game resolution
    baseviewwidth  = (cv_viewsize.value * BASEVIDWIDTH/10)&~7;
    // Hurdler: ST_HEIGHT/2 plus joli que ST_HEIGHT
    baseviewheight = (cv_viewsize.value * (BASEVIDHEIGHT-ST_HEIGHT/2)/10)&~1;

    // crap code, clean this up, use base resolution
    // Hurdler: ST_HEIGHT/2 plus joli que ST_HEIGHT
    top  = (BASEVIDHEIGHT - ST_HEIGHT/2 - baseviewheight) / 2;
    side = (BASEVIDWIDTH  - baseviewwidth) / 2;

    // top
    HWR_DrawFlatFill (0, 0,
                     BASEVIDWIDTH, (top<clearlines ? top : clearlines),
                     st_borderpatchnum);
    
    // left
    if (top<clearlines)
        HWR_DrawFlatFill (0, top,
                         side, (clearlines-top < baseviewheight ? clearlines-top : baseviewheight),
                         st_borderpatchnum);    
    
    // right
    if (top<clearlines)
        HWR_DrawFlatFill (side + baseviewwidth, top,
                         side, (clearlines-top < baseviewheight ? clearlines-top : baseviewheight),
                         st_borderpatchnum);    

    // bottom
    if (top+baseviewheight<clearlines)
        HWR_DrawFlatFill (0, top+baseviewheight,
                         BASEVIDWIDTH, (clearlines-baseviewheight-top < top ? clearlines-baseviewheight-top : top),
                         st_borderpatchnum);

    //
    // draw the view borders
    //

    basewindowx = (BASEVIDWIDTH - baseviewwidth)>>1;
    if (baseviewwidth==BASEVIDWIDTH)
        basewindowy = 0;
    else
    // Hurdler: ST_HEIGHT/2 plus joli que ST_HEIGHT
        basewindowy = (BASEVIDHEIGHT - ST_HEIGHT/2 - baseviewheight)>>1;

    // top edge
    if (clearlines > basewindowy-8) {
        patch = W_CachePatchNum (viewborderlump[BRDR_T],PU_CACHE);
        for (x=0 ; x<baseviewwidth; x+=8)
            HWR_DrawPatch (patch,basewindowx+x,basewindowy-8);
    }
    
    // bottom edge
    if (clearlines > basewindowy+baseviewheight) {
        patch = W_CachePatchNum (viewborderlump[BRDR_B],PU_CACHE);
        for (x=0 ; x<baseviewwidth ; x+=8)
            HWR_DrawPatch (patch,basewindowx+x,basewindowy+baseviewheight);
    }
    
    // left edge
    if (clearlines > basewindowy) {
        patch = W_CachePatchNum (viewborderlump[BRDR_L],PU_CACHE);
        for (y=0 ; y<baseviewheight && (basewindowy+y < clearlines); y+=8)
            HWR_DrawPatch (patch,basewindowx-8,basewindowy+y);
    }
    
    // right edge
    if (clearlines > basewindowy) {
        patch = W_CachePatchNum (viewborderlump[BRDR_R],PU_CACHE);
        for (y=0 ; y<baseviewheight && (basewindowy+y < clearlines); y+=8)
            HWR_DrawPatch (patch,basewindowx+baseviewwidth,basewindowy+y);
    }

    // Draw beveled corners.
    if (clearlines > basewindowy-8)
        HWR_DrawPatch (W_CachePatchNum (viewborderlump[BRDR_TL],PU_CACHE),
                      basewindowx-8,
                      basewindowy-8);

    if (clearlines > basewindowy-8)
        HWR_DrawPatch (W_CachePatchNum (viewborderlump[BRDR_TR],PU_CACHE),
                      basewindowx+baseviewwidth,
                      basewindowy-8);

    if (clearlines > basewindowy+baseviewheight)
        HWR_DrawPatch (W_CachePatchNum (viewborderlump[BRDR_BL],PU_CACHE),
                      basewindowx-8,
                      basewindowy+baseviewheight);

    if (clearlines > basewindowy+baseviewheight)
        HWR_DrawPatch (W_CachePatchNum (viewborderlump[BRDR_BR],PU_CACHE),
                      basewindowx+baseviewwidth,
                      basewindowy+baseviewheight);
}


// ==========================================================================
//                                                     AM_MAP.C DRAWING STUFF
// ==========================================================================

// Clear the automap part of the screen
void HWR_clearAutomap( void )
{
    FRGBAFloat fColor = { 0,0,0,1 };

    //FIXTHIS faB - optimize by clearing only colors ?
    //HWD.pfnSetBlend ( PF_NoOcclude );

    // minx,miny,maxx,maxy
    HWD.pfnClipRect( 0, 0, VIDWIDTH , VIDHEIGHT - STAT_HEIGHT, 0.9f );
    HWD.pfnClearBuffer( true, true, &fColor );
    HWD.pfnClipRect( 0, 0, VIDWIDTH, VIDHEIGHT, 0.9f );
}


// -----------------+
// HWR_drawAMline   : draw a line of the automap (the clipping is already done in automap code)
// Arg              : color is a RGB 888 value
// -----------------+
void HWR_drawAMline( fline_t* fl, int color )
{
    F2DCoord  v1, v2;
    RGBA_t    color_rgba;

    color_rgba.rgba = UINT2RGBA(V_GetColor( color ) << 8);
    color_rgba.s.alpha = 0xff;

    v1.x = ((float)fl->a.x-(vid.width/2.0f))*(2.0f/vid.width);
    v1.y = ((float)fl->a.y-(vid.height/2.0f))*(2.0f/vid.height);

    v2.x = ((float)fl->b.x-(vid.width/2.0f))*(2.0f/vid.width);
    v2.y = ((float)fl->b.y-(vid.height/2.0f))*(2.0f/vid.height);

    HWD.pfnDraw2DLine( &v1, &v2, color_rgba );
}


// -----------------+
// HWR_DrawFill     : draw flat coloured rectangle, with no texture
// -----------------+
void HWR_DrawFill( int x, int y, int w, int h, int color )
{
    FOutVector  v[4];
    FSurfaceInfo Surf;
    
//  3--2
//  | /|
//  |/ |
//  0--1
    v[0].x = v[3].x = (x - 160.0f)/160.0f;
    v[2].x = v[1].x = ((x+w) - 160.0f)/160.0f;
    v[0].y = v[1].y = -(y - 100.0f)/100.0f;
    v[2].y = v[3].y = -((y+h) - 100.0f)/100.0f;

    //Hurdler: do we still use this argb color? if not, we should remove it
    v[0].argb = v[1].argb = v[2].argb = v[3].argb = 0xff00ff00; //;
    v[0].oow = v[1].oow = v[2].oow = v[3].oow = 1.0f;

    v[0].sow = v[3].sow = 0.0f;
    v[2].sow = v[1].sow = 1.0f;
    v[0].tow = v[1].tow = 0.0f;
    v[2].tow = v[3].tow = 1.0f;

    Surf.FlatColor.rgba  = UINT2RGBA(V_GetColor( color ) << 8);
    Surf.FlatColor.s.alpha = 0xff;

    HWD.pfnDrawPolygon( &Surf, v, 4, PF_Modulated|PF_NoTexture );
}


// --------------------------------------------------------------------------
// screen shot
// --------------------------------------------------------------------------
void HWR_Screenshot (void)
{
    int         i;
    char        lbmname[12];

    byte*   bufw;
    unsigned short* bufr;
    byte*   dest;
    unsigned short   rgb565;

    bufr = malloc(VIDWIDTH*VIDHEIGHT*2);
    if (!bufr)
        I_Error ("no bufr");
    bufw = malloc(VIDWIDTH*VIDHEIGHT*3);
    if (!bufw)
        I_Error ("no bufw");

    //returns 16bit 565 RGB
    HWD.pfnReadRect (0, 0, VIDWIDTH, VIDHEIGHT, VIDWIDTH*2, bufr);

    for (dest = bufw,i=0; i<VIDWIDTH*VIDHEIGHT; i++)
    {
        rgb565 = bufr[i];
        *(dest++) = (rgb565 & 31) <<3;
        *(dest++) = ((rgb565 >> 5) & 63) <<2;
        *(dest++) = ((rgb565 >> 11) & 31) <<3;
    }
    
    // find a file name to save it to
    strcpy(lbmname,"DOOM000.tga");

    for (i=0 ; i<=999 ; i++)
    {
            lbmname[4] = i/100 + '0';
            lbmname[5] = i/10  + '0';
            lbmname[6] = i%10  + '0';
            if (access(lbmname,0) == -1)
                break;  // file doesn't exist
    }
    if (i<100)
    {
        // save the file
        saveTGA(lbmname, VIDWIDTH, VIDHEIGHT, (GLRGB *)bufw);
    }

    free(bufw);
    free(bufr);
}


// --------------------------------------------------------------------------
// save screenshots with TGA format
// --------------------------------------------------------------------------
void saveTGA(char *file_name, int width, int height, GLRGB *buffer)
{
    int fd;
    long size;
    TGAHeader tga_hdr;

    fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
    if (fd < 0)
        return;

    memset(&tga_hdr, 0, sizeof(tga_hdr));
    tga_hdr.width = width;
    tga_hdr.height = height;
    tga_hdr.image_pix_size = 24;
    tga_hdr.image_type = 2;
    tga_hdr.image_descriptor = 32;
    size = (long)width * (long)height * 3L;

    write(fd, &tga_hdr, sizeof(TGAHeader));
    write(fd, buffer, size);
    close(fd);
}
