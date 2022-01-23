// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_defs.h,v 1.6 2000/07/01 09:23:50 bpereira Exp $
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
// $Log: hw_defs.h,v $
// Revision 1.6  2000/07/01 09:23:50  bpereira
// no message
//
// Revision 1.5  2000/05/05 18:00:05  bpereira
// no message
//
// Revision 1.4  2000/04/18 16:07:16  hurdler
// better support of decals
//
// Revision 1.3  2000/04/11 01:00:59  hurdler
// Better coronas support
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      3D hardware renderer API definitions
//
//-----------------------------------------------------------------------------


#ifndef _HWR_DEFS_
#define _HWR_DEFS_


// ==========================================================================
//                                                               SIMPLE TYPES
// ==========================================================================

typedef long            FINT;
typedef unsigned long   FUINT;
typedef unsigned char   FUBYTE;
typedef unsigned long   FBITFIELD;
typedef float           FLOAT;
typedef unsigned char   FBOOLEAN;

// ==========================================================================
//                                                                      MATHS
// ==========================================================================

// Constants
#undef  PI
#define PI         (3.1415926535897932)
#define DEGREE (.01745328f)     // 2*PI/360


// ==========================================================================
//                                                                     COLORS
// ==========================================================================

// byte value for paletted graphics, which represent the transparent color
#define HWR_PATCHES_CHROMAKEY_COLORINDEX   247
#define HWR_CHROMAKEY_EQUIVALENTCOLORINDEX   0

// the chroma key color shows on border sprites, set it to black
#define HWR_PATCHES_CHROMAKEY_COLORVALUE     (0x00000000)    //RGBZ format as in grSstWinOpen()

// RGBA Color components with float type ranging [ 0 ... 1 ]
struct FRGBAFloat
{
    FLOAT   red;
    FLOAT   green;
    FLOAT   blue;
    FLOAT   alpha;
};
typedef struct FRGBAFloat FRGBAFloat;

#define UINT2RGBA(a) ((a&0xff)<<24)|((a&0xff00)<<8)|((a&0xff0000)>>8)|(((FUINT)a&0xff000000)>>24)

union FColorRGBA {
    FUINT rgba;
    struct {
    FUBYTE  red;
    FUBYTE  green;
    FUBYTE  blue;
    FUBYTE  alpha;
    } s;
};
typedef union FColorRGBA RGBA_t;

struct FColorARGB
{
    FUBYTE  alpha;
    FUBYTE  red;
    FUBYTE  green;
    FUBYTE  blue;
};
typedef struct FColorARGB ARGB_t;
typedef struct FColorARGB FColorARGB;



// ==========================================================================
//                                                                    VECTORS
// ==========================================================================

// Simple 2D coordinate
struct F2DCoord
{
    FLOAT x,y;
};
typedef struct F2DCoord F2DCoord;
typedef struct F2DCoord v2d_t;

// Simple 3D vector
struct FVector
{
        FLOAT x,y,z;
};
typedef struct FVector FVector;

// 3D model vector (coords + texture coords)
struct FModelVector
{
    //FVector     Point;
    FLOAT       x,y,z;
    FLOAT       s,t,w;            // texture coordinates
};
typedef struct FModelVector v3d_t;
typedef struct FModelVector wallVert3D;

//Hurdler: Transform (coords + angles)
struct FTransform
{
    FLOAT       x,y,z;           // position
    FLOAT       anglex,angley;   // aimingangle / viewangle 
};
typedef struct FTransform FTransform;

// Transformed vector, as passed to HWR API
struct FOutVector
{
        //FVector     Point;          // OOW is Z
    FLOAT       x,y;
    FUINT       argb;           // flat-shaded color
    FLOAT       oow;            // oow = z (legacy of old code)
    FLOAT       sow;            // s texture ordinate (s over w)
    FLOAT       tow;            // t texture ordinate (t over w)
};
// BP: what the heck, struct have the same name as the type !
typedef struct FOutVector FOutVector;



// ==========================================================================
//                                                               RENDER MODES
// ==========================================================================

// Flags describing how to render a polygon
// You pass a combination of these flags to DrawPolygon()
enum EPolyFlags
{
        // the first 5 are mutually exclusive

    PF_Masked           = 0x00000021,   // Poly is alpha scaled and 0 alpha pels are discarded (holes in texture)
    PF_Translucent      = 0x00000022,   // Poly is transparent, alpha = level of transparency
    PF_Additive         = 0x00000004,   // Poly is added to the frame buffer
    PF_Environment      = 0x00000028,   // Poly should be drawn environment mapped.
                                        // Hurdler: used for text drawing
    PF_Substractive     = 0x00000030,   // for splat 
    PF_AlphaTest        = 0x00000020,   // hiden param
    PF_Blending         = PF_Environment|PF_Additive|PF_Translucent|PF_Masked|PF_Substractive,

        // other flag bits
    
    PF_Occlude          = 0x00000100,   // Update the depth buffer
    PF_NoDepthTest      = 0x00000200,   // Disable the depth test mode
    PF_Invisible        = 0x00000400,   // Disable write to color buffer
    PF_Decal            = 0x00000800,   // Enable polygon offset
    PF_Modulated        = 0x00001000,   // Modulation ( multiply output with constant ARGB )
                                        // When set, pass the color constant into the FSurfaceInfo -> FlatColor
    PF_NoTexture        = 0x00002000,   // Use the small white texture
    PF_Corona           = 0x00004000,   // Tell the rendrer we are drawing a corona
    PF_Clip             = 0x40000000,   // clip to frustum and nearz plane (glide, automatic in opengl)
    PF_NoZClip          = 0x20000000,   // in conjonction with PF_Clip
    PF_Debug            = 0x80000000    // print debug message in driver :)
};


enum ESurfFlags
{
    SF_DYNLIGHT         = 0x00000001,

};

enum ETextureFlags
{
    TF_WRAPX            = 0x00000001,            // wrap around X
    TF_WRAPY            = 0x00000002,            // wrap around Y
    TF_WRAPXY           = TF_WRAPY | TF_WRAPX,   // very common so use alias it more easy
    TF_CHROMAKEYED      = 0x00000010,
};

#ifdef TODO
struct FTextureInfo
{
    FUINT       Width;              // Pixels
    FUINT       Height;             // Pixels
    FUBYTE     *TextureData;        // Image data
    FUINT       Format;             // FORMAT_RGB, ALPHA ...
    FBITFIELD   Flags;              // -- UNUSED YET -- Flags to tell driver about texture.
    void        DriverExtra;        // (OpenGL texture object nr, ... )
                                    // chromakey enabled,...
    
    struct FTextureInfo *Next;      // Manage list of downloaded textures.
};
#else
typedef struct GlideMipmap_s FTextureInfo;
#endif

// Description of a renderable surface
struct FSurfaceInfo
{
        FUINT       PolyFlags;          // Surface flags -- UNUSED YET --
        RGBA_t   FlatColor;          // Flat-shaded color used with PF_Modulated mode
};
typedef struct FSurfaceInfo FSurfaceInfo;

//Hurdler: added for backward compatibility
enum hwdsetspecialstate {
    HWD_SET_FOG_TABLE = 1,
    HWD_SET_FOG_MODE,
    HWD_SET_FOG_COLOR,
    HWD_SET_FOG_DENSITY,
    HWD_SET_FOV,
    HWD_SET_POLYGON_SMOOTH,
    HWD_SET_PALETTECOLOR,
    HWD_SET_TEXTUREFILTERMODE,
};
typedef enum hwdsetspecialstate hwdspecialstate_t;

enum hwdfiltermode {
    HWD_SET_TEXTUREFILTER_POINTSAMPLED, 
    HWD_SET_TEXTUREFILTER_BILINEAR,
    HWD_SET_TEXTUREFILTER_TRILINEAR,
    HWD_SET_TEXTUREFILTER_MIXED1,
    HWD_SET_TEXTUREFILTER_MIXED2,
};


#endif //_HWR_DEFS_
