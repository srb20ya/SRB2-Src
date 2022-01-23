// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_cache.c,v 1.23 2000/08/03 17:57:42 bpereira Exp $
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
// $Log: hw_cache.c,v $
// Revision 1.23  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.22  2000/07/13 21:07:47  metzgermeister
// fixed memory leak
//
// Revision 1.21  2000/07/01 09:23:50  bpereira
// no message
//
// Revision 1.20  2000/05/30 18:01:53  kegetys
// Added the chromakey flag to sprites
//
// Revision 1.19  2000/05/09 22:08:53  hurdler
// fix large sky problem
//
// Revision 1.18  2000/05/09 20:57:31  hurdler
// use my own code for colormap (next time, join with Boris own code)
// (necessary due to a small bug in Boris' code (not found) which shows strange effects under linux)
//
// Revision 1.17  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.16  2000/04/27 17:48:47  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.15  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.14  2000/04/24 17:23:26  hurdler
// Better support of colormap
//
// Revision 1.13  2000/04/24 15:46:34  hurdler
// Support colormap for text
//
// Revision 1.12  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.11  2000/04/23 00:30:47  hurdler
// fix a small bug in skin color
//
// Revision 1.10  2000/04/22 21:08:23  hurdler
// I like it better like that
//
// Revision 1.9  2000/04/22 20:16:30  hurdler
// I like it better like that
//
// Revision 1.8  2000/04/22 19:12:50  hurdler
// support skin color in hardware mode
//
// Revision 1.7  2000/04/22 16:09:14  hurdler
// support skin color in hardware mode
//
// Revision 1.6  2000/04/18 12:52:21  hurdler
// join with Boris' code
//
// Revision 1.5  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.4  2000/04/09 17:18:01  hurdler
// modified coronas' code for 16 bits video mode
//
// Revision 1.3  2000/04/06 20:50:23  hurdler
// add Boris' changes for coronas in doom3.wad
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      load and convert graphics to the hardware format
//
//-----------------------------------------------------------------------------


#include "hw_glob.h"
#include "hw_drv.h"

#include "../i_video.h"     //rendermode
#include "../r_data.h"
#include "../w_wad.h"
#include "../z_zone.h"

//   Build the full textures from patches.
static  GrLOD_t     gr_lods[9] = {
    GR_LOD_LOG2_256,
    GR_LOD_LOG2_128,
    GR_LOD_LOG2_64,
    GR_LOD_LOG2_32,
    GR_LOD_LOG2_16,
    GR_LOD_LOG2_8,
    GR_LOD_LOG2_4,
    GR_LOD_LOG2_2,
    GR_LOD_LOG2_1
};

typedef struct {
    GrAspectRatio_t aspect;
    float           max_s;
    float           max_t;
} booring_aspect_t;

static  booring_aspect_t gr_aspects[8] = {
    {GR_ASPECT_LOG2_1x1, 255, 255},
    {GR_ASPECT_LOG2_2x1, 255, 127},
    {GR_ASPECT_LOG2_4x1, 255,  63},
    {GR_ASPECT_LOG2_8x1, 255,  31},

    {GR_ASPECT_LOG2_1x1, 255, 255},
    {GR_ASPECT_LOG2_1x2, 127, 255},
    {GR_ASPECT_LOG2_1x4,  63, 255},
    {GR_ASPECT_LOG2_1x8,  31, 255}
};

//Hurdler: 25/04/2000: used for new colormap code in hardware mode
static byte     *gr_colormap = NULL; // by default it must be NULL ! (because colormap tables are not initialized)

// --------------------------------------------------------------------------
// Values set after a call to HWR_ResizeBlock()
// --------------------------------------------------------------------------
static  int     blocksize;
static  int     blockwidth;
static  int     blockheight;

extern byte transtables[256][256];

// wall, don't use alpha channel (use chroma key for hole)
#define HWR_DrawPatchInCache8(b,bw,bh,bm,tw,th,ox,oy,r)  HWR_DrawPatchInCache (b,bw,bh,bm,tw,th,ox,oy,r,1)
#define HWR_DrawPatchInCache88(b,bw,bh,bm,tw,th,ox,oy,r)  HWR_DrawPatchInCache (b,bw,bh,bm,tw,th,ox,oy,r,2)

// sprite, use alpha and chroma key for hole
static void HWR_DrawPatchInCache (byte* block,
                                  int   blockwidth,
                                  int   blockheight,
                                  int   blockmodulo,
                                  int   texturewidth,
                                  int   textureheight,
                                  int   originx,        //where to draw the patch in the surface block
                                  int   originy,
                                  patch_t* realpatch,
                                  int   bpp/*,
                                  boolean firetranslucent*/)
{
    int                 x,x1,x2;
    int                 col,ncols;
    fixed_t             xfrac, xfracstep;
    fixed_t             yfrac, yfracstep, position, count;
    fixed_t             scale_y;

    byte        *dest;
    byte        *source;
    column_t    *patchcol;

    x1 = originx;
    x2 = x1 + SHORT(realpatch->width);

    if (x1<0)
        x = 0;
    else
        x = x1;

    if (x2 > texturewidth)
        x2 = texturewidth;

    col  = x * blockwidth / texturewidth;
    ncols= ((x2-x) * blockwidth) / texturewidth;

/*
    CONS_Printf("patch %dx%d texture %dx%d block %dx%d\n", SHORT(realpatch->width),
                                                            SHORT(realpatch->height),
                                                            texturewidth,
                                                            textureheight,
                                                            blockwidth,blockheight);
    CONS_Printf("      col %d ncols %d x %d\n", col, ncols, x);
*/

    // source advance
    xfrac = 0;
    if (x1<0)
        xfrac = -x1<<16;

    xfracstep = (texturewidth << 16) / blockwidth;
    yfracstep = (textureheight<< 16) / blockheight;

    for (block += col*bpp; ncols--; block+=bpp, xfrac+=xfracstep)
    {
        patchcol = (column_t *)((byte *)realpatch
                                + LONG(realpatch->columnofs[xfrac>>16]));

        scale_y = (blockheight << 16) / textureheight;

        while (patchcol->topdelta != 0xff)
        {
            source = (byte *)patchcol + 3;
            count  = ((patchcol->length * scale_y) + (FRACUNIT/2)) >> 16;
            position = originy + patchcol->topdelta;

            yfrac = 0;
            //yfracstep = (patchcol->length << 16) / count;
            if (position < 0) {
                yfrac = -position<<16;
                count += (((position * scale_y) + (FRACUNIT/2)) >> 16);
                position = 0;
            }

            position = ((position * scale_y) + (FRACUNIT/2)) >> 16;
            if (position + count >= blockheight )
                count = blockheight - position;

            dest = block + (position*blockmodulo);
            while (count>0)
            {
                byte texel;
                count--;
                // all opaque runs with alpha 0xff
//BP test
#ifdef DONT_WORK
                if( !firetranslucent )
                    *((unsigned short*)dest) = (0xff<<8) | source[yfrac>>16];
                else
                    if( (transtables+((tr_transfx1-1)<<FF_TRANSSHIFT))[source[yfrac>>16]][32] != source[yfrac>>16] )
                        *((unsigned short*)dest) = (0x80<<8) | source[yfrac>>16];
                    else 
                    {}
#endif
                
                //Hurdler: not perfect, but better than holes
                texel = source[yfrac>>16];
                if ( texel ==HWR_PATCHES_CHROMAKEY_COLORINDEX)
                    texel = HWR_CHROMAKEY_EQUIVALENTCOLORINDEX;
                //Hurdler: 25/04/2000: now support colormap in hardware mode
                else if (gr_colormap)
                    texel = gr_colormap[texel];

                // hope compiler will get this switch out of the loops (dreams...)
                switch (bpp) {
                    case 1 : *dest = texel;                                           break;
                    case 2 : *((unsigned short*)dest) = (0xff<<8) | texel;            break;
                    default : I_Error("no drawer defined for this bpp (%d)\n",bpp);   break;
                }
                dest += blockmodulo;
                yfrac += yfracstep;
            }
            patchcol = (column_t *)(  (byte *)patchcol + patchcol->length + 4);
        }
    }
}


// resize the patch to be 3dfx complient
// set : blocksize
//       blockwidth
//       blockheight
//note :  8bit (1 byte per pixel) palettized format
static void HWR_ResizeBlock ( int originalwidth,
                              int originalheight,
                              GrTexInfo*    grInfo )
{
    int     j,k;
    int     max,min;

    // find a power of 2 width/height
    if (cv_grrounddown.value)
    {
        blockwidth = 256;
        while (originalwidth < blockwidth)
            blockwidth >>= 1;
        if (blockwidth<1)
            I_Error ("3D GenerateTexture : too small");

        blockheight = 256;
        while (originalheight < blockheight)
            blockheight >>= 1;
        if (blockheight<1)
            I_Error ("3D GenerateTexture : too small");
    }
    else
    {
        //size up to nearest power of 2
        blockwidth = 1;
        while (blockwidth < originalwidth)
            blockwidth <<= 1;
        // scale down the original graphics to fit in 256
        if (blockwidth>256)
            blockwidth = 256;
            //I_Error ("3D GenerateTexture : too big");

        //size up to nearest power of 2
        blockheight = 1;
        while (blockheight < originalheight)
            blockheight <<= 1;
        // scale down the original graphics to fit in 256
        if (blockheight>256)
            blockheight = 255;
            //I_Error ("3D GenerateTexture : too big");
    }

    // do the boring LOD stuff.. blech!
    if (blockwidth >= blockheight) {
        max = blockwidth;
        min = blockheight;
    }else{
        max = blockheight;
        min = blockwidth;
    }
    
    for (k=256, j=0; k > max; j++)
        k>>=1;
    grInfo->smallLodLog2 = gr_lods[j];
    grInfo->largeLodLog2 = gr_lods[j];
        
    for (k=max, j=0; k>min && j<4; j++)
        k>>=1;
    // aspect ratio too small for 3Dfx (eg: 8x128 is 1x16 : use 1x8)
    if (j==4){
        j=3;
        //CONS_Printf ("HWR_ResizeBlock : bad aspect ratio %dx%d\n", blockwidth,blockheight);
        if (blockwidth<blockheight)
            blockwidth = max>>3;
        else
            blockheight = max>>3;
    }
    if (blockwidth<blockheight)
        j+=4;
    grInfo->aspectRatioLog2 = gr_aspects[j].aspect;

    blocksize = blockwidth * blockheight;
}


//
// Create a composite texture from patches, adapt the texture size to a power of 2
// height and width for the hardware texture cache.
//
static void HWR_GenerateTexture (int texnum, GlideTexture_t* grtex)
{
    byte*               block;
    texture_t*          texture;
    texpatch_t*         patch;
    patch_t*            realpatch;

    int         i;    
    boolean     skyspecial = false; //poor hack for Legacy large skies..

    texture = textures[texnum];

    // hack the Legacy skies.. texture size is 256x128 but patch size is larger..

    if ( texture->name[0] == 'S' &&
         texture->name[1] == 'K' &&
         texture->name[2] == 'Y' &&
         texture->name[4] == 0 )
    {
        skyspecial = true;
        //texture->patchcount = 1; // BP: is not true fixthis !
        // Hurdler: is it better? (at least now it works with tnt sky)
        // BP: je ne pence pas que ce soit correct : 
        //     - une texture (sky y comprit) est constituer de patch (sprite) mit les uns sur les autres
        //     - fab avait mit patchcount a 1 car en general les textures qui sont plus grande sont 
        //       faite de plusieurs patch mait c'est pas toujours vrais 
        //     - en fait le plus gros probleme est la taille des texture limiter a 256
        //     - le problem de tnt c'est que le premier patch n'est pas celui qui va de 0 a 256 
        //       ces sky la font 4x256 et l'ordre n'a pas d'importance
        //     - finalement je pence que on peut virer le patchcount (on garde le truc normal) et 
        //       on laisse faire le code de clipping de texture 
        //     - (....) bon sa marche mais le sky est "strecher" pour qu'il rentre dans une texture 
        //       256 ce n'est pas parfait mais c'est mieux que rien
        //     tu peu virer ce commentaire une fois que tu l'a lu
        //texture->patchcount = texture->width/256;
        //if (texture->patchcount==0)
        //    texture->patchcount = 1;
        grtex->mipmap.flags = TF_WRAPXY; // don't use the chromakey for sky
    }
    else
        grtex->mipmap.flags = TF_CHROMAKEYED | TF_WRAPXY;

    //CONS_Printf ("Resizing texture %.8s from %dx%d", texture->name, texture->width, texture->height);
    HWR_ResizeBlock (texture->width, texture->height, &grtex->mipmap.grInfo);
    grtex->mipmap.width = blockwidth;
    grtex->mipmap.height = blockheight;
    /*
    CONS_Printf ("to %dx%d\n"
                 "         LodLog2(%d) aspectRatio(%d)\n"
                 "         s: %f t: %f\n",
                 blockwidth, blockheight,
                 grtex->mipmap.grInfo.smallLodLog2, grtex->mipmap.grInfo.aspectRatioLog2,
                 max_s,max_t);
                 */

//    CONS_Printf ("R_GenTex MULTI  %.8s size: %d\n",texture->name,blocksize);
    block = Z_Malloc (blocksize,
                      PU_STATIC,
                      &grtex->mipmap.grInfo.data);

    // set the default transparent color as background
    memset( block, HWR_PATCHES_CHROMAKEY_COLORINDEX, blocksize );

    // Composite the columns together.
    patch = texture->patches;

    for (i=0 , patch = texture->patches;
         i<texture->patchcount;
         i++, patch++)
    {
        realpatch = W_CacheLumpNum (patch->patch, PU_CACHE);
        // correct texture size for Legacy's large skies
        if (skyspecial) {
            //CONS_Printf("sky %d, %d\n",texture->width,SHORT(realpatch->width));
            //texture->width = SHORT(realpatch->width);
            texture->height = SHORT(realpatch->height);
        }
        HWR_DrawPatchInCache8( block,
                               blockwidth, blockheight,
                               blockwidth,
                               texture->width, texture->height,
                               patch->originx, patch->originy,
                               realpatch );
    }

    // make it purgable from zone memory
    // use PU_PURGELEVEL so we can Z_FreeTags all at once
    Z_ChangeTag (block, PU_3DFXCACHE);

    // setup the texture info
    grtex->mipmap.grInfo.format = GR_TEXFMT_P_8;

    grtex->scaleX = crapmul / (float)texture->width;
    grtex->scaleY = crapmul / (float)texture->height;
}


// grTex : 3Dfx texture cache info
//         .data : address of converted patch in heap memory
//                 user for Z_Malloc(), becomes NULL if it is purged from the cache
void HWR_Make3DfxPatch (patch_t* patch, GlidePatch_t* grPatch, GlideMipmap_t *grMipmap)
{
    byte*   block;
    int     newwidth,newheight;
    int     i;

    // don't do it twise (like a cache)
    if(grMipmap->width==0)
    {
        // save the original patch header so that the GlidePatch can be casted
        // into a standard patch_t struct and the existing code can get the
        // orginal patch dimensions and offsets.
        grPatch->width = SHORT(patch->width);
        grPatch->height = SHORT(patch->height);
        grPatch->leftoffset = SHORT(patch->leftoffset);
        grPatch->topoffset = SHORT(patch->topoffset);
        
        // allow the v_video drawing code to recognise a 3Dfx 'converted' patch_t
        grPatch->s3Dfx[0] = '3';
        grPatch->s3Dfx[1] = 'D';
        grPatch->s3Dfx[2] = 'f';
        grPatch->s3Dfx[3] = 'x';
        
        // find a power of 2 width/height
        //CONS_Printf ("Resizing _PATCH_ from %dx%d", SHORT(patch->width), SHORT(patch->height));
        
        // find the good 3dfx size (boring spec)
        HWR_ResizeBlock (SHORT(patch->width), SHORT(patch->height), &grMipmap->grInfo);
        grMipmap->width = blockwidth;
        grMipmap->height = blockheight;
    
        // no wrap around
        grMipmap->flags = 0;
        // setup the texture info
        grMipmap->grInfo.format = GR_TEXFMT_AP_88;
    }
    else
    {
        blockwidth = grMipmap->width;
        blockheight = grMipmap->height;
        blocksize = blockwidth * blockheight;
    }

    if( grMipmap->grInfo.data==NULL )
        block = Z_Malloc (blocksize<<1, PU_STATIC, &(grMipmap->grInfo.data));

    // fill background with chromakey, alpha=0
    for( i=0; i<blocksize; i++ )
            *((unsigned short*)block+i) = (0x00 <<8) | HWR_PATCHES_CHROMAKEY_COLORINDEX;

    // if rounddown, rounddown patches as well as textures
    if (cv_grrounddown.value) 
    {
        newwidth = blockwidth;
        newheight = blockheight;
    }
    else
    {
        // no rounddown, do not size up patches, so they don't look 'scaled'
        if (SHORT(patch->width) < blockwidth)
            newwidth = SHORT(patch->width);
        else
            // a rounddown was forced because patch->width was too large
            newwidth = blockwidth;

        if (SHORT(patch->height) < blockheight)
            newheight = SHORT(patch->height);
        else
            newheight = blockheight;
    }

    HWR_DrawPatchInCache88( block,
                            newwidth, newheight,
                            blockwidth*2,
                            SHORT(patch->width), SHORT(patch->height),
                            0, 0,
                            patch );

    grPatch->max_s = (float)newwidth / (float)blockwidth;
    grPatch->max_t = (float)newheight / (float)blockheight;

    // Now that the texture has been built in cache, it is purgable from zone memory.
    Z_ChangeTag (block, PU_3DFXCACHE);
}


// =====================
// WALL TEXTURES CACHING

// =====================

static int  gr_numtextures;
static GlideTexture_t*  gr_textures;       // for ALL Doom textures

// --------------------------------------------------------------------------
// protos
// --------------------------------------------------------------------------
static void    HWR_GenerateTexture (int texnum, GlideTexture_t* grtex);

void HWR_InitTextureCache (void)
{
    gr_numtextures = 0;
    gr_textures = NULL;
}

void HWR_FreeTextureCache (void)
{
    int i,j;
    // free references to the textures
    HWD.pfnClearMipMapCache ();

    // free all skin after each level: must be done after pfnClearMipMapCache!
    for (j=0; j<numwadfiles; j++)
        for (i=0; i<wadfiles[j]->numlumps; i++)
        {
            GlidePatch_t *grpatch = &(wadfiles[j]->cache3Dfx[i]);
            while (grpatch->mipmap.nextcolormap)
            {
                GlideMipmap_t *grmip = grpatch->mipmap.nextcolormap;
                grpatch->mipmap.nextcolormap = grmip->nextcolormap;
                free(grmip);
            }
        }

    // free all 3Dfx-converted graphics cached in the heap
    // our gool is only the textures since user of the texture is the texture cache
    Z_FreeTags (PU_3DFXCACHE, PU_3DFXCACHE);

    // now the heap don't have any 'user' pointing to our 
    // texturecache info, we can free it
    if (gr_textures)
        free (gr_textures);
}

void HWR_PrepLevelCache (int numtextures)
{
    // problem: the mipmap cache management hold a list of mipmaps.. but they are
    //           reallocated on each level..
    //sub-optimal, but 1) just need re-download stuff in 3Dfx cache VERY fast
    //   2) sprite/menu stuff mixed with level textures so can't do anything else

    // we must free it since numtextures changed
    HWR_FreeTextureCache ();

    gr_numtextures = numtextures;
    gr_textures = malloc (sizeof(GlideTexture_t) * numtextures);
    if (!gr_textures)
        I_Error ("3D can't alloc gr_textures");
    memset (gr_textures, 0, sizeof(GlideTexture_t) * numtextures);
}


// --------------------------------------------------------------------------
// Make sure texture is downloaded and set it as the source
// --------------------------------------------------------------------------
GlideTexture_t* HWR_GetTexture (int tex)
{
    GlideTexture_t* grtex;
#ifdef PARANOIA
    if( tex>=gr_numtextures )
        I_Error(" HWR_GetTexture : tex>=numtextures\n");
#endif
    grtex = &gr_textures[tex];

    if ( (!grtex->mipmap.grInfo.data) &&
         (!grtex->mipmap.downloaded) )
        HWR_GenerateTexture (tex, grtex);

    HWD.pfnSetTexture (&grtex->mipmap);
    return grtex;
}


static void HWR_CacheFlat (GlideMipmap_t* grMipmap, int flatlumpnum)
{
    byte *block;
    // setup the texture info
    grMipmap->grInfo.smallLodLog2 = GR_LOD_LOG2_64;
    grMipmap->grInfo.largeLodLog2 = GR_LOD_LOG2_64;
    grMipmap->grInfo.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
    grMipmap->grInfo.format = GR_TEXFMT_P_8;
    grMipmap->flags = TF_WRAPXY;
    grMipmap->width  = 64;
    grMipmap->height = 64;

    // the flat raw data needn't be converted with palettized textures
    block = Z_Malloc (W_LumpLength(flatlumpnum),
                      PU_3DFXCACHE,
                      &grMipmap->grInfo.data);

    W_ReadLump (flatlumpnum,grMipmap->grInfo.data);
}


// Download a Doom 'flat' to the 3Dfx cache and make it ready for use
void HWR_GetFlat (int flatlumpnum)
{
    GlideMipmap_t*      grmip;

    grmip = &(wadfiles[flatlumpnum>>16]->cache3Dfx[flatlumpnum & 0xffff].mipmap);

    if (!grmip->downloaded && 
        !grmip->grInfo.data) 
            HWR_CacheFlat (grmip, flatlumpnum);

    HWD.pfnSetTexture (grmip);
}


// 
// HWR_Load3DfxMappedPatch(): replace the skin color of the sprite in cache
//                          : load it first in doom cache if not already
// 
static void HWR_LoadMappedPatch(GlideMipmap_t *grmip, GlidePatch_t *gpatch)
{
    if( !grmip->downloaded && 
        !grmip->grInfo.data )
    {   
        patch_t *patch = W_CacheLumpNum(gpatch->patchlump, PU_STATIC);

        gr_colormap = grmip->colormap;
        HWR_Make3DfxPatch(patch, gpatch, grmip);
        gr_colormap = NULL; // set default colormap (green) when we have finished
        
        Z_Free(patch);
    }

    HWD.pfnSetTexture(grmip);
}

// -----------------+
// HWR_GetPatch     : Download a patch to the 3Dfx cache and make it ready for use
// -----------------+
void HWR_GetPatch( GlidePatch_t* gpatch )
{
    GlideMipmap_t*      grmip;

    grmip = &gpatch->mipmap;

    // is it in hardware cache
    if ( !grmip->downloaded &&
         !grmip->grInfo.data )
    {
        // load the software patch, PU_STATIC or the Z_Malloc for 3Dfx patch will
        // flush the software patch before the conversion! oh yeah I suffered
        patch_t *ptr = W_CacheLumpNum(gpatch->patchlump, PU_STATIC);
        HWR_Make3DfxPatch ( ptr, gpatch , &gpatch->mipmap);
        
        // this is inefficient.. but the 3Dfx patch in heap is purgeable so it
        // shouldn't fragment memory, and besides the REAL cache here is the 3Dfx memory
        Z_Free (ptr);
    }

    HWD.pfnSetTexture( grmip );
}


// -------------------+
// HWR_GetMappedPatch : Same as HWR_GetPatch for sprite color 
// -------------------+
void HWR_GetMappedPatch(GlidePatch_t* gpatch, byte *colormap)
{
    GlideMipmap_t   *grmip, *newmip;

    if( (colormap==colormaps) || (colormap==NULL) ) 
    {
        // Load the default (green) color in doom cache (temporary?) AND hardware cache
        HWR_GetPatch(gpatch);
        return;  
    }
    
    // search for the mimmap
    // skip the first (no colormap translated)
    for(grmip = &gpatch->mipmap ; grmip->nextcolormap ;)
    {
        grmip = grmip->nextcolormap;
        if (grmip->colormap==colormap)
        {
            HWR_LoadMappedPatch(grmip, gpatch);
            return;
        }
    }
    // not found, create it !
    // If we are here, the sprite with the current colormap is not already in hardware memory

    //BP: WARNING : don't free it manualy without clearing the cache of harware renderer 
    //              (it have a liste of mipmap)
    //    this malloc is cleared in HWR_FreeTextureCache
    //    (...) unfortunately z_malloc fragment alot the memory :( so malloc is better
    newmip = malloc(sizeof(GlideMipmap_t));
    grmip->nextcolormap = newmip;
    memset(newmip, 0, sizeof(GlideMipmap_t));

    newmip->colormap   = colormap;
    HWR_LoadMappedPatch(newmip, gpatch);
}


// -----------------+
// HWR_GetPic       : Download a Doom pic (raw row encoded with no 'holes')
// Returns          :
// -----------------+
static const int picmode2GR[] = {
    GR_TEXFMT_P_8,                // PALETTE
    0,                            // INTENSITY          (unsuported yet)
    GR_TEXFMT_ALPHA_INTENSITY_88, // INTENSITY_ALPHA
    0,                            // RGB24              (unsuported yet)
    46,                           // RGBA32             (opengl only)
};

GlidePatch_t *HWR_GetPic( int lumpnum )
{
    GlidePatch_t *grpatch;

    grpatch = &(wadfiles[lumpnum>>16]->cache3Dfx[lumpnum & 0xffff]);

    if(    !grpatch->mipmap.downloaded 
        && !grpatch->mipmap.grInfo.data )
    {
        pic_t *pic = W_CacheLumpNum( lumpnum, PU_STATIC );
        int j,k,len;
        len = W_LumpLength(lumpnum)-8; // 8 = pic_t header
            
        // WARNING : in 3dfx this code work ONLY with texture that size is power of 2 ! (FIXME)
        grpatch->mipmap.width  = grpatch->width  = SHORT(pic->width);
        grpatch->mipmap.height = grpatch->height = SHORT(pic->height);
        Z_Malloc (len, PU_3DFXCACHE, &grpatch->mipmap.grInfo.data);
        memcpy(grpatch->mipmap.grInfo.data, pic->data,len); 
        Z_ChangeTag(pic, PU_CACHE);

        for (k=256, j=0; k > grpatch->mipmap.width; j++)
            k>>=1;
        
        grpatch->mipmap.grInfo.smallLodLog2 = gr_lods[j];
        grpatch->mipmap.grInfo.largeLodLog2 = gr_lods[j];
        grpatch->mipmap.grInfo.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;
        grpatch->mipmap.grInfo.format = picmode2GR[pic->mode];
        grpatch->mipmap.flags = 0;
    }
    HWD.pfnSetTexture( &grpatch->mipmap );

    return grpatch;
}
