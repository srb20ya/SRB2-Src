// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_main.c,v 1.29 2000/08/03 17:57:42 bpereira Exp $
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
// $Log: hw_main.c,v $
// Revision 1.29  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.28  2000/07/01 09:23:50  bpereira
// no message
//
// Revision 1.27  2000/06/08 19:40:34  hurdler
// my changes before splitting (can be reverted in development branch)
//
// Revision 1.26  2000/05/30 18:03:22  kegetys
// Wall, floor and ceiling lighting is now done by changing only the RGB, not the alpha
//
// Revision 1.25  2000/05/10 17:45:35  kegetys
// no message
//
// Revision 1.24  2000/05/05 18:00:05  bpereira
// no message
//
// Revision 1.23  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.22  2000/04/27 23:41:16  hurdler
// better splitscreen support in OpenGL mode
//
// Revision 1.21  2000/04/27 17:48:47  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.20  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.19  2000/04/24 15:46:34  hurdler
// Support colormap for text
//
// Revision 1.18  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.17  2000/04/23 12:50:32  hurdler
// support filter mode in OpenGL
//
// Revision 1.16  2000/04/22 21:08:23  hurdler
// I like it better like that
//
// Revision 1.15  2000/04/22 16:09:14  hurdler
// support skin color in hardware mode
//
// Revision 1.14  2000/04/18 16:07:16  hurdler
// better support of decals
//
// Revision 1.13  2000/04/18 12:52:21  hurdler
// join with Boris' code
//
// Revision 1.11  2000/04/14 16:34:26  hurdler
// some nice changes for coronas
//
// Revision 1.10  2000/04/12 16:03:51  hurdler
// ready for T&L code and true static lighting
//
// Revision 1.9  2000/04/09 01:59:06  hurdler
// removed warnings
//
// Revision 1.8  2000/04/08 11:28:46  hurdler
// added boom water support
//
// Revision 1.7  2000/03/29 19:39:49  bpereira
// no message
//
// Revision 1.6  2000/03/08 17:02:05  hurdler
// fix the joiningame problem under Linux
//
// Revision 1.5  2000/03/07 14:22:48  hurdler
// no message
//
// Revision 1.4  2000/03/06 16:52:06  hurdler
// hack for OpenGL / Open Entry problem
//
// Revision 1.3  2000/03/05 17:10:56  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      hardware renderer, using the standard HardWareRender driver DLL for Doom Legacy
//
//-----------------------------------------------------------------------------


#include "hw_glob.h"
#include "hw_light.h"

#include "../doomstat.h"
#include "../i_video.h"  // added by Hurdler for rendermode == render_glide
#include "../v_video.h"
#include "../p_local.h"
#include "../p_setup.h"
#include "../r_local.h"
#include "../d_clisrv.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "../r_splats.h"


#ifdef LINUX
# define min(x,y) (x>y?y:x)
# define max(x,y) (x>y?x:y)
#endif // linux

#define ZCLIP_PLANE     4.0f

//#define BACK_TO_FRONT   // render with faB's Back To Front
        

// ==========================================================================
// the hardware driver object
// ==========================================================================
struct hwdriver_s hwdriver;

// ==========================================================================
//                                                                     PROTOS
// ==========================================================================


static  void    HWR_AddSprites (sector_t* sec );
static  void    HWR_ProjectSprite (mobj_t* thing );

static unsigned int atohex(char* s);


// ==========================================================================
//                                          3D ENGINE COMMANDS & CONSOLE VARS
// ==========================================================================

CV_PossibleValue_t grcrappymlook_cons_t[]= {{0,"Off"}, {1,"On"},{2,"Full"}, {0,NULL} };
CV_PossibleValue_t grgamma_cons_t[]= {{1,"MIN"}, {255,"MAX"}, {0,NULL} };
CV_PossibleValue_t grfiltermode_cons_t[]= {{HWD_SET_TEXTUREFILTER_POINTSAMPLED,"Nearest"}, 
                                           {HWD_SET_TEXTUREFILTER_BILINEAR,"Bilinear"},
                                           {HWD_SET_TEXTUREFILTER_TRILINEAR,"Trilinear"}, 
                                           {HWD_SET_TEXTUREFILTER_MIXED1,"Linear_Nearest"},
                                           {HWD_SET_TEXTUREFILTER_MIXED2,"Nearest_Linear"},
                                           {0,NULL} };

// BP: change directely the palette to see the change
void CV_Gammaxxx_ONChange(void)
{
    V_SetPalette( W_CacheLumpName( "PLAYPAL", PU_CACHE ) );
}

void CV_filtermode_ONChange(void);
void CV_FogDensity_ONChange(void);
//static void CV_grFogColor_OnChange (void);
static void CV_grFov_OnChange (void);
//static void CV_grMonsterDL_OnChange (void);
static void CV_grPolygonSmooth_OnChange (void);


consvar_t cv_grrounddown       = {"gr_rounddown",       "Off", 0,       CV_OnOff };
consvar_t cv_grcrappymlook     = {"gr_mlook",          "Full", CV_SAVE, grcrappymlook_cons_t };
consvar_t cv_grfov             = {"gr_fov",              "90", CV_SAVE|CV_CALL, CV_Unsigned, CV_grFov_OnChange };
consvar_t cv_grsky             = {"gr_sky",              "On", 0,       CV_OnOff };
consvar_t cv_grfog             = {"gr_fog",              "On", CV_SAVE, CV_OnOff };
consvar_t cv_grfogcolor        = {"gr_fogcolor",     "101010", CV_SAVE, NULL };
consvar_t cv_grfogdensity      = {"gr_fogdensity",      "500", CV_SAVE|CV_CALL|CV_NOINIT, CV_Unsigned, CV_FogDensity_ONChange };
consvar_t cv_grgammared        = {"gr_gammared",        "127", CV_SAVE|CV_CALL, grgamma_cons_t, CV_Gammaxxx_ONChange };
consvar_t cv_grgammagreen      = {"gr_gammagreen",      "127", CV_SAVE|CV_CALL, grgamma_cons_t, CV_Gammaxxx_ONChange };
consvar_t cv_grgammablue       = {"gr_gammablue",       "127", CV_SAVE|CV_CALL, grgamma_cons_t, CV_Gammaxxx_ONChange };
consvar_t cv_grfiltermode      = {"gr_filtermode", "Bilinear", CV_SAVE|CV_CALL, grfiltermode_cons_t, CV_filtermode_ONChange };
consvar_t cv_grzbuffer         = {"gr_zbuffer",          "On", 0,       CV_OnOff };
// console variables in development
consvar_t cv_grpolygonsmooth   = {"gr_polygonsmooth",   "Off", CV_SAVE|CV_CALL, CV_OnOff, CV_grPolygonSmooth_OnChange };
consvar_t cv_grmd2             = {"gr_md2",             "Off", CV_SAVE, CV_OnOff };

// faB : needs fix : walls are incorrectly clipped one column less
consvar_t cv_grclipwalls       = {"gr_clipwalls",       "Off", 0,       CV_OnOff };

//development variables for diverse uses
consvar_t cv_gralpha = {"gr_alpha","160", 0, CV_Unsigned };
consvar_t cv_grbeta  = {"gr_beta", "0",   0, CV_Unsigned };
consvar_t cv_grgamma = {"gr_gamma","0",   0, CV_Unsigned };

void CV_FogDensity_ONChange(void)
{
    HWD.pfnSetSpecialState( HWD_SET_FOG_DENSITY, cv_grfogdensity.value );
}

void CV_filtermode_ONChange(void)
{
    HWD.pfnSetSpecialState( HWD_SET_TEXTUREFILTERMODE, cv_grfiltermode.value);
}


// ==========================================================================
//                                                               VIEW GLOBALS
// ==========================================================================
// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW      ANG90

angle_t                 gr_clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
int                     gr_viewangletox[FINEANGLES/2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
angle_t                 gr_xtoviewangle[MAXVIDWIDTH+1];


// ==========================================================================
//                                                                    GLOBALS
// ==========================================================================

// uncomment to remove the plane rendering
#define DOPLANES
//#define DOWALLS

// BP: test of draw sky by polygone like in sofware with visplaine, unfortunately
// this don't work since we must have z for pixel and z for texture (not like now with z=oow)
#define POLYSKY

// BP: test change fov when looking up/down but bsp projection messup :(
//#define NOCRAPPYMLOOK

// added to Doom's sector lightlevel to make things a bit brighter (sprites/walls/planes)
#define BRIGHTEN_THE_DAMN_LIGHTLEVELS       12


#define drawtextured  true
//static  boolean     drawtextured = false;

// base values set at SetViewSize
float       gr_basecentery;
float       gr_baseviewwindowy;
float       gr_basewindowcentery;

float       gr_viewwidth;           // viewport clipping boundaries (screen coords)
float       gr_viewheight;
float       gr_centerx;
float       gr_centery;
float       gr_viewwindowx;         // top left corner of view window
float       gr_viewwindowy;
float       gr_windowcenterx;       // center of view window, for projection
float       gr_windowcentery;

float       gr_pspritexscale;
float       gr_pspriteyscale;

seg_t*      gr_curline;
side_t*     gr_sidedef;
line_t*     gr_linedef;
sector_t*   gr_frontsector;
sector_t*   gr_backsector;
//FRGBAFloat  gr_cursectorlight;      // colour & intensity of current sector's lighting

int         gr_segtextured;
int         gr_toptexture;
int         gr_bottomtexture;
int         gr_midtexture;

// --------------------------------------------------------------------------
//                                              STUFF FOR THE PROJECTION CODE
// --------------------------------------------------------------------------

#ifdef TANDL
FTransform      transform;
#endif
// duplicates of the main code, set after R_SetupFrame() passed them into sharedstruct,
// copied here for local use
static  fixed_t dup_viewx;
static  fixed_t dup_viewy;
static  fixed_t dup_viewz;
static  angle_t dup_viewangle;

static  float   gr_viewx;
static  float   gr_viewy;
static  float   gr_viewz;
static  float   gr_viewsin;
static  float   gr_viewcos;

//static  float   gr_projectionx;
//static  float   gr_projectiony;


//11/11/99: changed from static by Hurdler
//04/01/00: Maybe not necessary with the new T&L code (need to be checked!)
static float   gr_viewludsin;  //look up down kik test
static float   gr_viewludcos;
//18/02/00: Hurdler: gr_scalefrustum is not used anymore
//static float   gr_scalefrustum;    //scale 90degree y axis frustum to viewheight


// --------------------------------------------------------------------------
// back to front drawing of subsectors
// --------------------------------------------------------------------------


#ifdef BACK_TO_FRONT

#define MAXBTOFPOLYS    5000

#define BTOF_PLANE      0x10000000      // it's a plane (else it's a wall line)
#define BTOF_CEILING    0x20000000      // it's a ceiling plabe (else it's a floor plane)
#define BTOF_SUBSECNUM  0x0FFFFFFF      // mask for subsector number
#define BTOF_WATERHACK  0x40000000      // test

typedef struct {
    int     flags;          //plane | subsector num
    seg_t*  line;           //seg_t for walls
} btofpoly_t;
static  int             numbtofpolys;
static  btofpoly_t*     btofpolys;

#endif


// ==========================================================================
//                                   FLOOR/CEILING GENERATION FROM SUBSECTORS
// ==========================================================================

#ifdef  DOPLANES

//what is the maximum number of verts around a convex floor/ceiling polygon?
static  FOutVector  planeVerts[256];
//Hurdler: added for correct dynamic ligting with mlook
static  lvertex3D_t plVerts[256];

// -----------------+
// HWR_RenderPlane  : Render a floor or ceiling convex polygon
// -----------------+
void HWR_RenderPlane( extrasubsector_t *xsub,
                      fixed_t           fixedheight,
                      FBITFIELD         PolyFlags )
{
    polyvertex_t*   pv;
    float           height; //constant y for all points on the convex flat polygon
    FOutVector      *v3d;
    int             nrPlaneVerts;   //verts original define of convex flat polygon
    int             i;
#ifndef TANDL
    float           tr_x,tr_y,tr_z;
#endif
    float           flatxref,flatyref;

    FSurfaceInfo    Surf;

    // no convex poly were generated for this subsector
    if (!xsub->planepoly)
        return;
    
    height = ((float)fixedheight) * crapmul;

    pv  = xsub->planepoly->pts;
    nrPlaneVerts = xsub->planepoly->numpts;

    if (nrPlaneVerts < 3)   //not even a triangle ?
        return;

    //reference point for flat texture coord for each vertex around the polygon
    flatxref = ((fixed_t)pv->x & (~63)) / 64.0f;
    flatyref = ((fixed_t)pv->y & (~63)) / 64.0f;

    // transform
    v3d = planeVerts;
#ifndef TANDL
    for (i=0; i<nrPlaneVerts; i++,v3d++,pv++)
    {
        // translation
        tr_x     = pv->x - gr_viewx;
        v3d->sow = (pv->x / 64.0f) - flatxref;
        tr_y     = pv->y - gr_viewy;    //distance close/away
        v3d->tow = flatyref - (pv->y / 64.0f );
        tr_z   = height - gr_viewz;

        // rotation around vertical y axis
        v3d->x = (tr_x * gr_viewsin) - (tr_y * gr_viewcos );
        tr_x   = (tr_x * gr_viewcos) + (tr_y * gr_viewsin );

        //Hurdler: added for correct dynamic ligting with mlook
        plVerts[i].x = v3d->x;
        plVerts[i].y = tr_z;
        plVerts[i].z = tr_x;

        // look up/down ----TOTAL SUCKS!!!--- do the 2 rotation in one!!!!!!!!!!!!!!!!!!!!!
        tr_y     = (tr_x * gr_viewludcos) + (tr_z * gr_viewludsin );
        v3d->oow = (tr_x * gr_viewludsin) - (tr_z * gr_viewludcos );

        //scale to enter the frustum clipping, so that 90deg frustum fit to screen height
        v3d->y = tr_y  * ORIGINAL_ASPECT;
    }
#else
    for (i=0; i<nrPlaneVerts; i++,v3d++,pv++)
    {
        v3d->sow = (pv->x / 64.0f) - flatxref;
        v3d->tow = flatyref - (pv->y / 64.0f );
        v3d->x = pv->x;
        v3d->y = height;
        v3d->oow = pv->y;

        plVerts[i].x = v3d->x;
        plVerts[i].y = v3d->y;
        plVerts[i].z = v3d->oow;
    }
#endif

    // only useful for flat coloured triangles
    //Surf.FlatColor = 0xff804020;

    //  use different light tables
    //  for horizontal / vertical / diagonal
    //  note: try to get the same visual feel as the original
    if(fixedcolormap)
    {
        // TODO: better handling of fixedcolormap
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = 0xff;
        Surf.FlatColor.s.alpha = 0xff;
    }
    else
    {
        FUINT   lightnum;

        lightnum = gr_frontsector->lightlevel + (extralight<<4 ) + BRIGHTEN_THE_DAMN_LIGHTLEVELS;
        if (lightnum > 255)
            lightnum = 255;
        
        // BP: alpha = light ??? what the heck ! fab ?
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = Surf.FlatColor.s.alpha = lightnum;
    }

    HWD.pfnDrawPolygon( &Surf, planeVerts, nrPlaneVerts, PolyFlags|PF_Masked|PF_Modulated|PF_Clip );

    //12/11/99: Hurdler: add here code for dynamic lighting on planes
    HWR_PlaneLighting(planeVerts, nrPlaneVerts, plVerts);

    //experimental code: shadow of the player: should be done only on floor
    //HWR_DynamicShadowing(planeVerts, nrPlaneVerts, plVerts, plyr); 
}

#ifdef POLYSKY
// this don't draw anything it only update the z-buffer so there isn't problem with 
// wall/things upper that sky (map12)
static void HWR_RenderSkyPlane (extrasubsector_t *xsub, fixed_t fixedheight)
//                              FBITFIELD         PolyFlags )
{
    polyvertex_t*   pv;
    float           height; //constant y for all points on the convex flat polygon
    FOutVector      *v3d;
    int             nrPlaneVerts;   //verts original define of convex flat polygon
    int             i;
#ifndef TANDL
    float           tr_x,tr_y,tr_z;
#endif
    float           flatxref,flatyref;

    // no convex poly were generated for this subsector
    if (!xsub->planepoly)
        return;

    height = ((float)fixedheight) * crapmul;

    pv  = xsub->planepoly->pts;
    nrPlaneVerts = xsub->planepoly->numpts;

    if (nrPlaneVerts < 3)   //not even a triangle ?
        return;
    
    //HWR_GetTexture (skytexture);

    //reference point for flat texture coord for each vertex around the polygon
    flatxref = ((fixed_t)pv->x & (~63)) / 64.0f;
    flatyref = ((fixed_t)pv->y & (~63)) / 64.0f;
    
    // transform
    v3d = planeVerts;
#ifndef TANDL
    for (i=0; i<nrPlaneVerts; i++,v3d++,pv++)
    {
        // translation
        tr_x   = pv->x - gr_viewx;
        v3d->sow = (pv->x / 64.0f) - flatxref;
        tr_y     = pv->y - gr_viewy;    //distance close/away
        v3d->tow = flatyref - (pv->y / 64.0f );
        tr_z   = height - gr_viewz;

        // rotation around vertical y axis
        v3d->x = (tr_x * gr_viewsin) - (tr_y * gr_viewcos );
        tr_x   = (tr_x * gr_viewcos) + (tr_y * gr_viewsin );

        // look up/down ----TOTAL SUCKS!!!--- do the 2 rotation in one!!!!!!!!!!!!!!!!!!!!!
        tr_y     = (tr_x * gr_viewludcos) + (tr_z * gr_viewludsin );
        v3d->oow = (tr_x * gr_viewludsin) - (tr_z * gr_viewludcos );

        //scale to enter the frustum clipping, so that 90deg frustum fit to screen height
        v3d->y = tr_y  * ORIGINAL_ASPECT;

//        v3d->sow = -1; //gr_windowcenterx + v3d->x*gr_centerx/v3d->oow;
//        v3d->tow =  1; //gr_windowcentery + v3d->y*gr_centery/v3d->oow;
    }
#else
    for (i=0; i<nrPlaneVerts; i++,v3d++,pv++)
    {
        v3d->sow = must be transformed and projected !;
        v3d->tow = must be transformed and projected !;
        v3d->x = pv->x;
        v3d->y = height;
        v3d->oow = pv->y;

        plVerts[i].x = v3d->x;
        plVerts[i].y = v3d->y;
        plVerts[i].z = v3d->oow;
    }
#endif

    HWD.pfnDrawPolygon( NULL, planeVerts, nrPlaneVerts, PF_Invisible|PF_Occlude|PF_Masked|PF_Clip );
}
#endif //polysky

#endif //doplanes

/*
   wallVerts order is :
          3--2
          | /|
          |/ |
          0--1
*/
/*#ifdef WALLSPLATS
void HWR_DrawSegsSplats( FSurfaceInfo * pSurf )
{
    FOutVector    trVerts[4],*wv;
    wallVert3D    wallVerts[4];
    wallVert3D    *pwallVerts;
    wallsplat_t*  splat;
#ifndef TANDL
    float         tr_x,tr_y;
#endif
    GlidePatch_t* gpatch;
    int           i;
    FSurfaceInfo  pSurf2;
    // seg bbox
    fixed_t       segbbox[4];

    M_ClearBox(segbbox);
    M_AddToBox(segbbox,((polyvertex_t *)gr_curline->v1)->x/crapmul,((polyvertex_t *)gr_curline->v1)->y/crapmul);
    M_AddToBox(segbbox,((polyvertex_t *)gr_curline->v2)->x/crapmul,((polyvertex_t *)gr_curline->v2)->y/crapmul);
*/
    // splat are drawn by line but this func is called for eatch segs of a line
    /* BP: DONT WORK BECAUSE Z-buffer !!!!
           FIXME : the splat must be stored by segs !
    if( gr_curline->linedef->splatdrawn == validcount )
        return;
    gr_curline->linedef->splatdrawn = validcount;
    */
/*
    splat = (wallsplat_t*) gr_curline->linedef->splats;
    for ( ; splat ; splat=splat->next)
    {
        //BP: don't draw splat extern to this seg 
        //    this is quick fix best is explain in logboris.txt at 12-4-2000
        if( !M_PointInBox(segbbox,splat->v1.x,splat->v1.y) && !M_PointInBox(segbbox,splat->v2.x,splat->v2.y))
            continue;

        gpatch = W_CachePatchNum (splat->patch, PU_CACHE);
        HWR_GetPatch(gpatch);

        wallVerts[0].x = wallVerts[3].x = splat->v1.x*crapmul;
        wallVerts[0].z = wallVerts[3].z = splat->v1.y*crapmul;
        wallVerts[2].x = wallVerts[1].x = splat->v2.x*crapmul;
        wallVerts[2].z = wallVerts[1].z = splat->v2.y*crapmul;

        i = splat->top;
        if( splat->yoffset )
            i += *splat->yoffset;

        wallVerts[2].y = wallVerts[3].y = i*crapmul+(gpatch->height>>1);
        wallVerts[0].y = wallVerts[1].y = i*crapmul-(gpatch->height>>1);

        wallVerts[3].s = wallVerts[3].t = wallVerts[2].s = wallVerts[0].t = 0.0f;
        wallVerts[1].s = wallVerts[1].t = wallVerts[2].t = wallVerts[0].s = 1.0f;

        // transform
        wv = trVerts;
        pwallVerts = wallVerts;
#ifndef TANDL
        for (i=0; i<4; i++,wv++,pwallVerts++)
        {
            // translation
            tr_x = pwallVerts->x - gr_viewx;
            tr_y = pwallVerts->z - gr_viewy;
            wv->y = pwallVerts->y - gr_viewz;
            
            // rotation around vertical y axis
            wv->oow = (tr_x * gr_viewcos) + (tr_y * gr_viewsin );
            wv->x   = (tr_x * gr_viewsin) - (tr_y * gr_viewcos );
            
            //look up/down ----TOTAL SUCKS!!!--- do the 2 in one!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            tr_y = wv->y;
            tr_x = wv->oow;
            
            wv->y   = (tr_x * gr_viewludcos) + (tr_y * gr_viewludsin );
            wv->oow = (tr_x * gr_viewludsin) - (tr_y * gr_viewludcos );
            
            //scale y before frustum so that frustum can be scaled to screen height
            wv->y = wv->y * ORIGINAL_ASPECT;
            
            wv->sow = pwallVerts->s;
            wv->tow = pwallVerts->t;
        }
#else
        for (i=0; i<4; i++,wv++,pwallVerts++)
        {
            wv->x   = pwallVerts->x;
            wv->oow = pwallVerts->z;
            wv->y   = pwallVerts->y;
            
            wv->sow = pwallVerts->s;
            wv->tow = pwallVerts->t;
        }
#endif        
        memcpy(&pSurf2,pSurf,sizeof(FSurfaceInfo));
        switch (splat->flags & SPLATDRAWMODE_MASK) {
            case SPLATDRAWMODE_OPAQUE :
                pSurf2.FlatColor.s.alpha = 0xff;
                i = PF_Translucent;
                break;
            case SPLATDRAWMODE_TRANS :
                pSurf2.FlatColor.s.alpha = 128;
                i = PF_Translucent;
                break;
            case SPLATDRAWMODE_SHADE :
                pSurf2.FlatColor.s.alpha = 0xff;
                i = PF_Substractive;
                break;
        }

        HWD.pfnDrawPolygon( &pSurf2, trVerts, 4, i|PF_Modulated|PF_Clip|PF_Decal);
    }
}
#endif
*/
// ==========================================================================
//                                        WALL GENERATION FROM SUBSECTOR SEGS
// ==========================================================================


// v1,v2 : the start & end vertices along the original wall segment, that may have been
//         clipped so that only a visible portion of the wall seg is drawn.
// floorheight, ceilingheight : depend on wall upper/lower/middle, comes from the sectors.

// -----------------+
// HWR_ProjectWall  : 
// -----------------+
/*
   wallVerts order is :
          3--2
          | /|
          |/ |
          0--1
*/
void HWR_ProjectWall( wallVert3D   * wallVerts,
                      FSurfaceInfo * pSurf )
{
    FOutVector  trVerts[4];
    int         i;
    FOutVector  *wv;

#ifndef TANDL
    float       tr_x,tr_y;
#endif
    //Hurdler: added for correct dynamic ligting with mlook
    lvertex3D_t wlVerts[4]; // shouldn't be necessary with new T&L code

    // transform 
    wv = trVerts;
#ifndef TANDL
    for (i=0; i<4; i++,wv++, wallVerts++)
    {
        // translation
        tr_x = wallVerts->x - gr_viewx;
        tr_y = wallVerts->z - gr_viewy;
        wv->y = wallVerts->y - gr_viewz;

        // rotation around vertical y axis
        wv->oow = (tr_x * gr_viewcos) + (tr_y * gr_viewsin );
        wv->x   = (tr_x * gr_viewsin) - (tr_y * gr_viewcos );

        //Hurdler: added for correct dynamic ligting with mlook
        wlVerts[i].x = wv->x;
        wlVerts[i].y = wv->y;
        wlVerts[i].z = wv->oow;

        //look up/down ----TOTAL SUCKS!!!--- do the 2 in one!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        tr_y = wv->y;
        tr_x = wv->oow;

        wv->y   = (tr_x * gr_viewludcos) + (tr_y * gr_viewludsin );
        wv->oow = (tr_x * gr_viewludsin) - (tr_y * gr_viewludcos );
        
        //scale y before frustum so that frustum can be scaled to screen height
        wv->y = wv->y * ORIGINAL_ASPECT;

        wv->sow = wallVerts->s;
        wv->tow = wallVerts->t;
    }   
#else
    // it sounds really stupid to do this conversion with the new T&L code
    // we should directly put the right information in the right structure
    // wallVerts3D seems ok, doesn't need FOutVector
    // also remove the light copy
    for (i=0; i<4; i++, wv++, wallVerts++)
    {
        wv->sow = wallVerts->s;
        wv->tow = wallVerts->t;
        wv->x = wallVerts->x;
        wv->y = wallVerts->y;
        wv->oow = wallVerts->z;

        wlVerts[i].x = wv->x;
        wlVerts[i].y = wv->y;
        wlVerts[i].z = wv->oow;
    }
#endif
        
    HWD.pfnDrawPolygon( pSurf, trVerts, 4, PF_Masked|PF_Modulated|PF_Occlude|PF_Clip);
/*
    if (gr_curline->linedef->splats && cv_splats.value)
        HWR_DrawSegsSplats( pSurf );*/

#ifdef TANDL
    //Hurdler: do static lighting using gr_curline->lm
#endif

    HWR_WallLighting(trVerts, wlVerts);
}

// ==========================================================================
//                                                          BSP , CULL, ETC..
// ==========================================================================

// return the frac from the interception of the clipping line
// (in fact a clipping plane that has a constant, so can clip with simple 2d)
// with the wall segment
//
static float HWR_ClipViewSegment (int x, polyvertex_t* v1, polyvertex_t* v2)
{
    float       num;
    float       den;
    float       v1x;
    float       v1y;
    float       v1dx;
    float       v1dy;
    float       v2dx;
    float       v2dy;

    angle_t     clipangle=gr_xtoviewangle[x];

    // a segment of a polygon
    v1x  = v1->x;
    v1y  = v1->y;
    v1dx = (v2->x - v1->x );
    v1dy = (v2->y - v1->y );

    // the clipping line
    clipangle = clipangle + dup_viewangle; //back to normal angle (non-relative)
    v2dx = (float)finecosine[clipangle>>ANGLETOFINESHIFT] * crapmul;
    v2dy = (float)finesine[clipangle>>ANGLETOFINESHIFT] * crapmul;

    den = v2dy*v1dx - v2dx*v1dy;
    if (den == 0)
        return -1;         // parallel

    // calc the frac along the polygon segment,
    //num = (v2x - v1x)*v2dy + (v1y - v2y)*v2dx;
    //num = -v1x * v2dy + v1y * v2dx;
    num = (gr_viewx - v1x)*v2dy + (v1y - gr_viewy)*v2dx;

    return num / den;
}


#ifdef BACK_TO_FRONT
// Store Wall range information for drawing later back to front
static void HWR_StoreWallRange (float startfrac, float endfrac)
{
    if (numbtofpolys >= MAXBTOFPOLYS)
        return;

    btofpolys[numbtofpolys].flags = 0;
    btofpolys[numbtofpolys].line = gr_curline;      // !!!!!!!!!!!!!!!!!!!! watchout

    // TODO:  store startfrac/endfrac..
    //        BP : and effect if any
    
    numbtofpolys++;
}
#endif

//
// HWR_StoreWallRange
// A portion or all of a wall segment will be drawn, from startfrac to endfrac,
//  where 0 is the start of the segment, 1 the end of the segment
// Anything between means the wall segment has been clipped with solidsegs,
//  reducing wall overdraw to a minimum
//
//20/08/99: Changed by Hurdler (taken from faB's code)
#ifdef BACK_TO_FRONT
static void HWR_DrawWallRange (seg_t* gr_curline, float startfrac, float endfrac)
#else
static void HWR_StoreWallRange (int startfrac, int endfrac)
#endif
{
    wallVert3D  wallVerts[4];
    v2d_t       vs, ve;         // start, end vertices of 2d line (view from above)
    
    fixed_t     worldtop;
    fixed_t     worldbottom;
    fixed_t     worldhigh;
    fixed_t     worldlow;

    fixed_t     worldup;        // for 2sided lines
    fixed_t     worlddown;

    GlideTexture_t* grTex;

    float       cliplow,cliphigh;
    
    fixed_t     texturehpeg;
    fixed_t     texturevpeg;        //mid or top
    fixed_t     texturevpegtop;     //top
    fixed_t     texturevpegbottom;  //bottom
    
    FSurfaceInfo Surf;

    if (startfrac>endfrac)
        return;

    gr_sidedef = gr_curline->sidedef;
    gr_linedef = gr_curline->linedef;

#ifdef BACK_TO_FRONT
    gr_frontsector = gr_linedef->frontsector;
    gr_backsector = gr_linedef->backsector;
#endif
//20/08/99: End of changes
        
    // mark the segment as visible for auto map
    gr_linedef->flags |= ML_MAPPED;

    worldtop    = gr_frontsector->ceilingheight;
    worldbottom = gr_frontsector->floorheight;

    gr_midtexture = gr_toptexture = gr_bottomtexture = 0;

    if (!gr_backsector)
    {
        // single sided line
        gr_midtexture = texturetranslation[gr_sidedef->midtexture];

        // PEGGING
        if (gr_linedef->flags & ML_DONTPEGBOTTOM)
            texturevpeg = gr_frontsector->floorheight + textureheight[gr_sidedef->midtexture] - worldtop;
        else
            // top of texture at top
            texturevpeg = 0;

        texturevpeg += gr_sidedef->rowoffset;
    }
    else
    {
        // two sided line

        worldhigh = gr_backsector->ceilingheight;
        worldlow = gr_backsector->floorheight;

        // hack to allow height changes in outdoor areas
        if (gr_frontsector->ceilingpic == skyflatnum &&
            gr_backsector->ceilingpic  == skyflatnum)
        {
            worldtop = worldhigh;
        }

        // possibly mid texture
        gr_midtexture = texturetranslation[gr_sidedef->midtexture];


        // find positioning of mid 2sided texture
        if (gr_midtexture) {
            // 2sided line mid texture uses lowest ceiling, highest floor
            worldup = worldhigh < worldtop ? worldhigh : worldtop;
            worlddown = worldlow > worldbottom ? worldlow : worldbottom;

            if (gr_linedef->flags & ML_DONTPEGBOTTOM) {
                worldup = worlddown + textureheight[gr_midtexture] + gr_sidedef->rowoffset;
                worlddown = worldup - textureheight[gr_midtexture];
                texturevpeg = 0;
                // 2sided textures don't repeat vertically
                /*
                
                if (worldup - worlddown > textureheight[gr_sidedef->midtexture])
                    worldup = worlddown + textureheight[gr_sidedef->midtexture];
                texturevpeg = worlddown + textureheight[gr_sidedef->midtexture] - worldup;*/
            }
            else {
                texturevpeg = 0;
                worldup += gr_sidedef->rowoffset;
                // 2sided textures don't repeat vertically
                //if (worldup - worlddown > textureheight[gr_sidedef->midtexture])
                    worlddown = worldup - textureheight[gr_midtexture];
            }
        }
       
        // check TOP TEXTURE
        if (worldhigh < worldtop)
        {
            // top texture
            gr_toptexture = texturetranslation[gr_sidedef->toptexture];

            // PEGGING
            if (gr_linedef->flags & ML_DONTPEGTOP)
            {
                // top of texture at top
                texturevpegtop = 0;
            }
            else
            {
                texturevpegtop = gr_backsector->ceilingheight + textureheight[gr_sidedef->toptexture] - worldtop;
            }
        }
        
        // check BOTTOM TEXTURE
        if (worldlow > worldbottom)     //only if VISIBLE!!!
        {
            // bottom texture
            gr_bottomtexture = texturetranslation[gr_sidedef->bottomtexture];
            
            // PEGGING
            if (gr_linedef->flags & ML_DONTPEGBOTTOM )
            {
                // bottom of texture at bottom
                // top of texture at top
                texturevpegbottom = worldtop - worldlow;
            }
            else
                // top of texture at top
                texturevpegbottom = 0;
        }

        texturevpegtop    += gr_sidedef->rowoffset;
        texturevpegbottom += gr_sidedef->rowoffset;
    }

    gr_segtextured = gr_midtexture | gr_toptexture | gr_bottomtexture;
        
    if (gr_segtextured) {
        // x offset the texture
        texturehpeg = gr_sidedef->textureoffset + gr_curline->offset;
    }

    vs.x = ((polyvertex_t *)gr_curline->v1)->x;
    vs.y = ((polyvertex_t *)gr_curline->v1)->y;
    ve.x = ((polyvertex_t *)gr_curline->v2)->x;
    ve.y = ((polyvertex_t *)gr_curline->v2)->y;

    //
    // clip the wall segment to solidsegs
    //

/*  BP : removed since there is no more clipwalls !
    // clip start of segment
    if (startfrac > 0){
        if (startfrac>1)
        {
#ifdef PARANOIA
            CONS_Printf ("startfrac %f\n", startfrac );
#endif
            startfrac = 1;
        }
            vs.x = vs.x + (ve.x - vs.x) * startfrac;
            vs.y = vs.y + (ve.y - vs.y) * startfrac;
        }

    // clip end of segment
    if (endfrac < 1){
        if (endfrac<0)
        {
#ifdef PARANOIA
            CONS_Printf ("  endfrac %f\n", endfrac );
#endif
            endfrac=0;
        }
            ve.x = vs.x + (ve.x - vs.x) * endfrac;
            ve.y = vs.y + (ve.y - vs.y) * endfrac;
        }
*/
    // remember vertices ordering
    //  3--2
    //  | /|
    //  |/ |
    //  0--1
    // make a wall polygon (with 2 triangles), using the floor/ceiling heights,
    // and the 2d map coords of start/end vertices 
    wallVerts[0].x = wallVerts[3].x = vs.x;
    wallVerts[0].z = wallVerts[3].z = vs.y;
    wallVerts[2].x = wallVerts[1].x = ve.x;
    wallVerts[2].z = wallVerts[1].z = ve.y;
    wallVerts[0].w = wallVerts[1].w = wallVerts[2].w = wallVerts[3].w = 1.0f;


    //  use different light tables
    //  for horizontal / vertical / diagonal
    //  note: try to get the same visual feel as the original
    if(fixedcolormap)
    {
        // TODO: better handling of fixedcolormap
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = 0xff;
        Surf.FlatColor.s.alpha = 0xff;
    }
    else
    {
        FUINT   lightnum;

        lightnum = gr_frontsector->lightlevel + (extralight<<4 ) + BRIGHTEN_THE_DAMN_LIGHTLEVELS;
        if (lightnum > 255)
            lightnum = 255;
        
            if (((polyvertex_t *)gr_curline->v1)->y == ((polyvertex_t *)gr_curline->v2)->y &&
                lightnum >= (255/LIGHTLEVELS) )
            lightnum -= (255/LIGHTLEVELS );
            else
            if (((polyvertex_t *)gr_curline->v1)->x == ((polyvertex_t *)gr_curline->v2)->x &&
                lightnum < 255 - (255/LIGHTLEVELS) )
            lightnum += (255/LIGHTLEVELS );

        // store Surface->FlatColor to modulate wall texture
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = Surf.FlatColor.s.alpha = lightnum;
    }

    //
    // draw top texture
    //
    if (gr_toptexture)
    {
        if (drawtextured)
        {
            grTex = HWR_GetTexture (gr_toptexture );

            // clip texture s start/end coords with solidsegs
            if (startfrac > 0 && startfrac < 1)
                cliplow = (texturehpeg * grTex->scaleX) + (gr_curline->length * grTex->scaleX * startfrac);
            else
                cliplow = texturehpeg * grTex->scaleX;
        
            if (endfrac < 1 && endfrac>0)
                cliphigh = (texturehpeg * grTex->scaleX) + (gr_curline->length * grTex->scaleX * endfrac);
            else
                cliphigh = (texturehpeg + gr_curline->length) * grTex->scaleX;

            wallVerts[0].s = wallVerts[3].s = cliplow;
            wallVerts[2].s = wallVerts[1].s = cliphigh;

            wallVerts[3].t = wallVerts[2].t = texturevpegtop * grTex->scaleY;
            wallVerts[0].t = wallVerts[1].t = (texturevpegtop + worldtop - worldhigh) * grTex->scaleY;
        }

        // set top/bottom coords
        wallVerts[2].y = wallVerts[3].y = (float)worldtop * crapmul;
        wallVerts[0].y = wallVerts[1].y = (float)worldhigh * crapmul;

        HWR_ProjectWall( wallVerts, &Surf );
    }

    //
    // draw bottom texture
    //
    if (gr_bottomtexture)
    {
        wallVerts[0].x = wallVerts[3].x = vs.x;
        wallVerts[0].z = wallVerts[3].z = vs.y;
        wallVerts[2].x = wallVerts[1].x = ve.x;
        wallVerts[2].z = wallVerts[1].z = ve.y;
        wallVerts[0].w = wallVerts[1].w = wallVerts[2].w = wallVerts[3].w = 1.0f;

        if (drawtextured) {
            grTex = HWR_GetTexture (gr_bottomtexture );

            // clip texture s start/end coords with solidsegs
            if (startfrac > 0 && startfrac < 1)
                cliplow = (texturehpeg * grTex->scaleX) + (gr_curline->length * grTex->scaleX * startfrac);
            else
                cliplow = texturehpeg * grTex->scaleX;
        
            if (endfrac < 1 && endfrac>0)
                cliphigh = (texturehpeg * grTex->scaleX) + (gr_curline->length * grTex->scaleX * endfrac);
            else
                cliphigh = (texturehpeg + gr_curline->length) * grTex->scaleX;

            wallVerts[0].s = wallVerts[3].s = cliplow;
            wallVerts[2].s = wallVerts[1].s = cliphigh;
            
            wallVerts[3].t = wallVerts[2].t = texturevpegbottom * grTex->scaleY;
            wallVerts[0].t = wallVerts[1].t = (texturevpegbottom + worldlow - worldbottom) * grTex->scaleY;
        }
        
        // set top/bottom coords
        wallVerts[2].y = wallVerts[3].y = (float)worldlow * crapmul;
        wallVerts[0].y = wallVerts[1].y = (float)worldbottom * crapmul;

        HWR_ProjectWall( wallVerts, &Surf );
    }

    //
    // draw at last mid texture, ! we modify worldtop and worldbottom!
    // 
    if (gr_midtexture)
    {
        // 2 sided line
        if (gr_backsector) {
            worldtop = worldup;
            worldbottom = worlddown;
        }

        
        //
        // draw middle texture
        //
        if (drawtextured) {
            grTex = HWR_GetTexture (gr_midtexture );

            // clip texture s start/end coords with solidsegs
            if (startfrac > 0 && startfrac < 1)
                cliplow = (texturehpeg * grTex->scaleX) + (gr_curline->length * grTex->scaleX * startfrac);
            else
                cliplow = texturehpeg * grTex->scaleX;
            
            if (endfrac < 1 && endfrac>0)
                cliphigh = (texturehpeg * grTex->scaleX) + (gr_curline->length * grTex->scaleX * endfrac);
            else
                cliphigh = (texturehpeg + gr_curline->length) * grTex->scaleX;

            wallVerts[0].s = wallVerts[3].s = cliplow;
            wallVerts[2].s = wallVerts[1].s = cliphigh;

            wallVerts[3].t = wallVerts[2].t = texturevpeg * grTex->scaleY;
            wallVerts[0].t = wallVerts[1].t = (texturevpeg + worldtop - worldbottom) * grTex->scaleY;
        }
        
        // set top/bottom coords
        wallVerts[2].y = wallVerts[3].y = (float)worldtop * crapmul;
        wallVerts[0].y = wallVerts[1].y = (float)worldbottom * crapmul;
        
        HWR_ProjectWall( wallVerts, &Surf );
    }
}


//
// ClipWallSegment
// Clips the given range of columns
// and includes it in the new clip list.
//
typedef struct
{
    int first;
    int last;
    
} cliprange_t;


#define MAXSEGS         128

// newend is one past the last valid seg
cliprange_t*    newend;
cliprange_t     gr_solidsegs[MAXSEGS];


void printsolidsegs( void)
{
    cliprange_t*        start;
    if(!newend || cv_grbeta.value!=2)
        return;
    for(start=gr_solidsegs;start!=newend;start++)
        CONS_Printf("%d-%d|",start->first,start->last );
    CONS_Printf("\n\n" );
}

//
//
//
static void HWR_ClipSolidWallSegment (int first, int last )
{
    cliprange_t*        next;
    cliprange_t*        start;
    float           lowfrac, highfrac;
    boolean poorhack=false;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = gr_solidsegs;
    while (start->last < first-1)
        start++;
    
    if (first < start->first)
    {
        if (last < start->first-1)
        {
            // Post is entirely visible (above start),
            //  so insert a new clippost.
            HWR_StoreWallRange (first, last );

            next = newend;
            newend++;
            
            while (next != start)
            {
                *next = *(next-1 );
                next--;
            }
            next->first = first;
            next->last = last;
            printsolidsegs( );
            return;
        }
        
        // There is a fragment above *start.
        if (!cv_grclipwalls.value) {
            if (!poorhack) HWR_StoreWallRange (first, last);
            poorhack=true;
        } 
        else
        {
            highfrac = HWR_ClipViewSegment (start->first+1, (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
            HWR_StoreWallRange (0, highfrac );
        }
        // Now adjust the clip size.
        start->first = first;
    }
    
    // Bottom contained in start?
    if (last <= start->last)
    {
        printsolidsegs( );
        return;
    }
    next = start;
    while (last >= (next+1)->first-1)
    {
        // There is a fragment between two posts.
        if (!cv_grclipwalls.value) {
            if (!poorhack) HWR_StoreWallRange (first,last );
            poorhack=true;
        }
        else
        {
            lowfrac  = HWR_ClipViewSegment (next->last-1 , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
            highfrac = HWR_ClipViewSegment ((next+1)->first+1 , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
            HWR_StoreWallRange (lowfrac, highfrac );
        }
        next++;
        
        if (last <= next->last)
        {
            // Bottom is contained in next.
            // Adjust the clip size.
            start->last = next->last;
            goto crunch;
        }
    }
    
    if(first==next->first+1) // 1 line texture
    {
        if (!cv_grclipwalls.value) {
            if (!poorhack) HWR_StoreWallRange (first,last );
            poorhack=true;
        }else
             HWR_StoreWallRange (0, 1 );
    }
    else
    {
    // There is a fragment after *next.
        if (!cv_grclipwalls.value) {
            if (!poorhack) HWR_StoreWallRange (first,last );
            poorhack=true;
        }
        else
        {
            lowfrac  = HWR_ClipViewSegment ( next->last-1 , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
            HWR_StoreWallRange (lowfrac, 1 );
        }
    }

    // Adjust the clip size.
    start->last = last;

    // Remove start+1 to next from the clip list,
    // because start now covers their area.
crunch:
    if (next == start)
    {
        printsolidsegs( );
        // Post just extended past the bottom of one post.
        return;
    }


    while (next++ != newend)
    {
        // Remove a post.
        *++start = *next;
    }

    newend = start;
    printsolidsegs( );
}

//
//  handle LineDefs with upper and lower texture (windows)
//
static void HWR_ClipPassWallSegment (int       first,
                             int        last )
{
    cliprange_t*        start;
    float           lowfrac, highfrac;
     //to allow noclipwalls but still solidseg reject of non-visible walls
    boolean         poorhack=false;  

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = gr_solidsegs;
    while (start->last < first-1)
        start++;
    
    if (first < start->first)
    {
        if (last < start->first-1)
        {
            // Post is entirely visible (above start).
            HWR_StoreWallRange (0, 1 );
            return;
        }
        
        // There is a fragment above *start.
        if (!cv_grclipwalls.value) {
        //20/08/99: Changed by Hurdler (taken from faB's code)
            if (!poorhack) HWR_StoreWallRange (0,1 );
            poorhack=true;
        } 
        else
        {
            highfrac  = HWR_ClipViewSegment ( min(start->first+1,start->last) , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
            HWR_StoreWallRange (0, highfrac );
        }
    }
    
    // Bottom contained in start?
    if (last <= start->last)
        return;
    
    while (last >= (start+1)->first-1)
    {
        // There is a fragment between two posts.
        if (!cv_grclipwalls.value) {
            if (!poorhack) HWR_StoreWallRange (0,1 );
            poorhack=true;
        } 
        else
        {
            lowfrac  = HWR_ClipViewSegment (max(start->last-1,start->first) , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
            highfrac = HWR_ClipViewSegment (min((start+1)->first+1,(start+1)->last) , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
            HWR_StoreWallRange (lowfrac, highfrac );
        }
        start++;
        
        if (last <= start->last)
            return;
    }

    if(first==start->first+1) // 1 line texture
    {
        if (!cv_grclipwalls.value) {
            if (!poorhack) HWR_StoreWallRange (0,1 );
            poorhack=true;
        }else
             HWR_StoreWallRange (0, 1 );
    }
    else
    {
        // There is a fragment after *next.
        if(!cv_grclipwalls.value) {
            if (!poorhack) HWR_StoreWallRange (0,1 );
            poorhack=true;
        }
        else
        {
            lowfrac  = HWR_ClipViewSegment ( max(start->last-1,start->first) , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
            HWR_StoreWallRange (lowfrac, 1 );
        }
    }
}

// --------------------------------------------------------------------------
//  HWR_ClipToSolidSegs check if it is hide by wall (solidsegs)
// --------------------------------------------------------------------------
static boolean HWR_ClipToSolidSegs (int   first,
                                    int   last )
{
    cliprange_t*        start;

    // Find the first range that touches the range
    //  (adjacent pixels are touching).
    start = gr_solidsegs;
    while (start->last < first-1)
        start++;

    if (first < start->first)
        return true;

    // Bottom contained in start?
    if (last <= start->last)
        return false;

    return true;
}


//
// HWR_ClearClipSegs
//
static void HWR_ClearClipSegs (void)
{
    gr_solidsegs[0].first = -0x7fffffff;
    gr_solidsegs[0].last = -1;
    gr_solidsegs[1].first = VIDWIDTH;    //viewwidth;
    gr_solidsegs[1].last = 0x7fffffff;
    newend = gr_solidsegs+2;
}


// -----------------+
// HWR_AddLine      : Clips the given segment and adds any visible pieces to the line list.
// Notes            : gr_cursectorlight is set to the current subsector -> sector -> light value
//                  : ( it may be mixed with the wall's own flat colour in the future ... )
// -----------------+
static void HWR_AddLine( seg_t * line )
{
    int                 x1;
    int                 x2;
    angle_t             angle1;
    angle_t             angle2;
    angle_t             span;
    angle_t             tspan;

    gr_curline = line;

    // OPTIMIZE: quickly reject orthogonal back sides.
    angle1 = R_PointToAngle (((polyvertex_t *)line->v1)->x*FRACUNIT, ((polyvertex_t *)line->v1)->y*FRACUNIT);
    angle2 = R_PointToAngle (((polyvertex_t *)line->v2)->x*FRACUNIT, ((polyvertex_t *)line->v2)->y*FRACUNIT);

    // Clip to view edges.
    span = angle1 - angle2;

    // backface culling : span is < ANG180 if ang1 > ang2 : the seg is facing
    if (span >= ANG180)
        return;

    // Global angle needed by segcalc.
    //rw_angle1 = angle1;
    angle1 -= dup_viewangle;
    angle2 -= dup_viewangle;

    tspan = angle1 + gr_clipangle;
    if (tspan > 2*gr_clipangle)
    {
        tspan -= 2*gr_clipangle;
        
        // Totally off the left edge?
        if (tspan >= span)
            return;
        
        angle1 = gr_clipangle;
    }
    tspan = gr_clipangle - angle2;
    if (tspan > 2*gr_clipangle)
    {
        tspan -= 2*gr_clipangle;
        
        // Totally off the left edge?
        if (tspan >= span)
            return;
        angle2 = -gr_clipangle;
    }

#if 0
    {
    float fx1,fx2,fy1,fy2;
    //BP: test with a better projection than viewangletox[R_PointToAngle(angle)]
    // do not enable this at release 4 mul and 2 div
    fx1=((polyvertex_t *)(line->v1))->x-gr_viewx;
    fy1=((polyvertex_t *)(line->v1))->y-gr_viewy;
    fy2 = (fx1 * gr_viewcos + fy1 * gr_viewsin);
    if(fy2<0)
        // the point is back 
        fx1 = 0;
    else
        fx1 = gr_windowcenterx + (fx1 * gr_viewsin - fy1 * gr_viewcos) * gr_centerx / fy2; 

    fx2=((polyvertex_t *)(line->v2))->x-gr_viewx;
    fy2=((polyvertex_t *)(line->v2))->y-gr_viewy;
    fy1 = (fx2 * gr_viewcos + fy2 * gr_viewsin);
    if(fy1<0)
        // the point is back 
        fx2 = VIDWIDTH;
    else
        fx2 = gr_windowcenterx + (fx2 * gr_viewsin - fy2 * gr_viewcos) * gr_centerx / fy1;

    x1 = fx1+0.5;
    x2 = fx2+0.5;
    }
#else
    // The seg is in the view range,
    // but not necessarily visible.
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;

    x1 = gr_viewangletox[angle1];
    x2 = gr_viewangletox[angle2];
#endif
    // Does not cross a pixel?
//    if (x1 == x2)
/*    {
        // BP: HERE IS THE MAIN PROBLEM !
        //CONS_Printf("tineline\n");
        return;
    }
*/
    gr_backsector = line->backsector;
    
    // Single sided line?
    if (!gr_backsector)
        goto clipsolid;
    
    // Closed door.
    if (gr_backsector->ceilingheight <= gr_frontsector->floorheight ||
        gr_backsector->floorheight >= gr_frontsector->ceilingheight)
        goto clipsolid;
    
    // Window.
    if (gr_backsector->ceilingheight != gr_frontsector->ceilingheight ||
        gr_backsector->floorheight != gr_frontsector->floorheight)
        goto clippass;
    
    // Reject empty lines used for triggers and special events.
    // Identical floor and ceiling on both sides,
    //  identical light levels on both sides,
    //  and no middle texture.
    if (   gr_backsector->ceilingpic == gr_frontsector->ceilingpic
        && gr_backsector->floorpic == gr_frontsector->floorpic
        && gr_backsector->lightlevel == gr_frontsector->lightlevel
        && gr_curline->sidedef->midtexture == 0)
    {
        return;
    }
    
clippass:
    if (x1 == x2)
    {  x2++;x1-=2; }
    HWR_ClipPassWallSegment (x1, x2-1);
    return;
    
clipsolid:
    if (x1 == x2)
        goto clippass;
    HWR_ClipSolidWallSegment (x1, x2-1);
}


// HWR_CheckBBox
// Checks BSP node/subtree bounding box.
// Returns true
//  if some part of the bbox might be visible.
//
// modified to use local variables

extern  int     checkcoord[12][4];  //r_bsp.c

static boolean  HWR_CheckBBox (fixed_t*   bspcoord)
{
    int                 boxpos;

    fixed_t             x1;
    fixed_t             y1;
    fixed_t             x2;
    fixed_t             y2;

    angle_t             angle1;
    angle_t             angle2;
    angle_t             span;
    angle_t             tspan;

    int                 sx1;
    int                 sx2;

    // Find the corners of the box
    // that define the edges from current viewpoint.
    if (dup_viewx <= bspcoord[BOXLEFT])
        boxpos = 0;
    else if (dup_viewx < bspcoord[BOXRIGHT])
        boxpos = 1;
    else
        boxpos = 2;

    if (dup_viewy >= bspcoord[BOXTOP])
        boxpos |= 0;
    else if (dup_viewy > bspcoord[BOXBOTTOM])
        boxpos |= 1<<2;
    else
        boxpos |= 2<<2;

    if (boxpos == 5)
        return true;

    x1 = bspcoord[checkcoord[boxpos][0]];
    y1 = bspcoord[checkcoord[boxpos][1]];
    x2 = bspcoord[checkcoord[boxpos][2]];
    y2 = bspcoord[checkcoord[boxpos][3]];

    // check clip list for an open space
    angle1 = R_PointToAngle (x1, y1) - dup_viewangle;
    angle2 = R_PointToAngle (x2, y2) - dup_viewangle;

    span = angle1 - angle2;

    // Sitting on a line?
    if (span >= ANG180)
        return true;

    tspan = angle1 + gr_clipangle;

    if (tspan > 2*gr_clipangle)
    {
        tspan -= 2*gr_clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;

        angle1 = gr_clipangle;
    }
    tspan = gr_clipangle - angle2;
    if (tspan > 2*gr_clipangle)
    {
        tspan -= 2*gr_clipangle;

        // Totally off the left edge?
        if (tspan >= span)
            return false;

        angle2 = -gr_clipangle;
    }


    // Find the first clippost
    //  that touches the source post
    //  (adjacent pixels are touching).
    angle1 = (angle1+ANG90)>>ANGLETOFINESHIFT;
    angle2 = (angle2+ANG90)>>ANGLETOFINESHIFT;
    sx1 = gr_viewangletox[angle1];
    sx2 = gr_viewangletox[angle2];

    // Does not cross a pixel.
    if (sx1 == sx2)
        return false;

    return HWR_ClipToSolidSegs (sx1,sx2-1);
}

sector_t *R_FakeFlat(sector_t *, sector_t *, int *, int *, boolean);

// -----------------+
// HWR_Subsector    : Determine floor/ceiling planes.
//                  : Add sprites of things in sector.
//                  : Draw one or more line segments.
// Notes            : Sets gr_cursectorlight to the light of the parent sector, to modulate wall textures
// -----------------+
static int doomwaterflat;  //set by R_InitFlats hack
static void HWR_Subsector( int num )
{
    int                     count;
    seg_t*                  line;
    subsector_t*            sub;
    sector_t                tempsec; //SoM: 4/7/2000
    int                     floorlightlevel;
    int                     ceilinglightlevel;
    
#ifndef BACK_TO_FRONT
    fixed_t                 wh;
#endif

//no risk while developing, enough debugging nights!
#ifdef PARANOIA
    if (num>=addsubsector)
        I_Error ("HWR_Subsector: ss %i with numss = %i, addss = %d",
                                num,numsubsectors,addsubsector );
    
    /*if (num>=numsubsectors)
        I_Error ("HWR_Subsector: ss %i with numss = %i",
                         num,
                         numsubsectors );*/
#endif

    if (num < numsubsectors)
    {
        sscount++;
        // subsector
        sub = &subsectors[num];
        // sector
        gr_frontsector = sub->sector;
        // how many linedefs
        count = sub->numlines;
        // first line seg
        line = &segs[sub->firstline];
    }
    else
    {
        // there are no segs but only planes
        sub = &subsectors[0];
        gr_frontsector = sub->sector;
        count = 0;
        line = NULL;
    }
//20/08/99: Changed by Hurdler (taken from faB's code)
#ifdef BACK_TO_FRONT
#ifdef DOPLANES
    // -------------------- WATER IN DEV. TEST ------------------------
    //dck hack : use abs(tag) for waterheight
    if (gr_frontsector->tag<0)
    {
        if (numbtofpolys<MAXBTOFPOLYS) {
            btofpolys[numbtofpolys].flags = BTOF_PLANE|BTOF_WATERHACK|num;
            numbtofpolys++;
        }
    }
    // -------------------- WATER IN DEV. TEST ------------------------
#endif
#else

    //SoM: 4/7/2000: Test to make Boom water work in Hardware mode.
    gr_frontsector = R_FakeFlat(gr_frontsector, &tempsec, &floorlightlevel,
                                &ceilinglightlevel, false);
    //FIXME: Use floorlightlevel and ceilinglightlevel insted of lightlevel.

    // ------------------------------------------------------------------------
    // sector lighting, DISABLED because it's done in HWR_StoreWallRange
    // ------------------------------------------------------------------------
    // TODO : store a RGBA instead of just intensity, allow coloured sector lighting
    //light = (FUBYTE)(sub->sector->lightlevel & 0xFF) / 255.0f;
    //gr_cursectorlight.red   = light;
    //gr_cursectorlight.green = light;
    //gr_cursectorlight.blue  = light;
    //gr_cursectorlight.alpha = light;


    // render floor ?
#ifdef DOPLANES
    // yeah, easy backface cull! :)
    if (gr_frontsector->floorheight < dup_viewz) {
        if(gr_frontsector->floorpic != skyflatnum) {
            HWR_GetFlat ( levelflats[gr_frontsector->floorpic].lumpnum  );
            HWR_RenderPlane( &extrasubsectors[num], gr_frontsector->floorheight, PF_Occlude );
        }
        else
        {
#ifdef POLYSKY
            HWR_RenderSkyPlane (&extrasubsectors[num], gr_frontsector->floorheight);
#endif
            cv_grsky.value = true;
        }
    }

    if (gr_frontsector->ceilingheight > dup_viewz)
    {
        if(gr_frontsector->ceilingpic != skyflatnum) {
            HWR_GetFlat ( levelflats[gr_frontsector->ceilingpic].lumpnum );
            HWR_RenderPlane (&extrasubsectors[num], gr_frontsector->ceilingheight, PF_Occlude );
        }
        else
        {
#ifdef POLYSKY
            HWR_RenderSkyPlane (&extrasubsectors[num], gr_frontsector->ceilingheight);
#endif
            cv_grsky.value = true;
        }
    }
#endif //doplanes
#endif // backtofront
//20/08/99: End of changes

// Hurder ici se passe les choses intéressantes!
// on vient de tracer le sol et le plafond
// on trace à présent d'abord les sprites et ensuite les murs
// hurdler: faux: on ajoute seulement les sprites, le murs sont tracés d'abord
    if (line!=NULL)
    {
        // draw sprites first , coz they are clipped to the solidsegs of
        // subsectors more 'in front'
        HWR_AddSprites (gr_frontsector);
        while (count--)
        {
                HWR_AddLine (line);
                line++;
        }
    }

//20/08/99: Changed by Hurdler (taken from faB's code)
#ifdef DOPLANES
#ifdef BACK_TO_FRONT
    // render floor ?
    // yeah, easy backface cull! :)
    if (gr_frontsector->floorheight < dup_viewz)
    {
        if(gr_frontsector->floorpic != skyflatnum) {
            //frontsector->floorheight, frontsector->floorpic, frontsector->lightlevel);
        
            //push it
            if (numbtofpolys<MAXBTOFPOLYS) {
                btofpolys[numbtofpolys].flags = BTOF_PLANE | num;
                numbtofpolys++;
            }
        }
        else cv_grsky.value = true;
    }

    if (gr_frontsector->ceilingheight > dup_viewz) 
    {
        if(gr_frontsector->ceilingpic != skyflatnum) {
            //frontsector->ceilingheight, frontsector->ceilingpic, frontsector->lightlevel
            //push it
            if (numbtofpolys<MAXBTOFPOLYS) {
                btofpolys[numbtofpolys].flags = BTOF_PLANE|BTOF_CEILING | num;
                numbtofpolys++;
            }
        }
        else cv_grsky.value = true;
    }
#else // !DEFINED(BACK_TO_FRONT)
    // -------------------- WATER IN DEV. TEST ------------------------
    //dck hack : use abs(tag) for waterheight
    if (gr_frontsector->tag<0)
    {
        wh = ((-gr_frontsector->tag) <<16) + (FRACUNIT/2);
        if (wh > gr_frontsector->floorheight &&
            wh < gr_frontsector->ceilingheight )
        {
            HWR_GetFlat ( doomwaterflat );
            HWR_RenderPlane( &extrasubsectors[num], wh, PF_Translucent );
        }
    }
    // -------------------- WATER IN DEV. TEST ------------------------
#endif
#endif
//20/08/99: End of changes
}


//
// Renders all subsectors below a given node,
//  traversing subtree recursively.
// Just call with BSP root.

#ifdef coolhack
//t;b;l;r
static fixed_t hackbbox[4];
//BOXTOP,
//BOXBOTTOM,
//BOXLEFT,
//BOXRIGHT
static boolean HWR_CheckHackBBox (fixed_t*   bb)
{
    if (bb[BOXTOP]<hackbbox[BOXBOTTOM]) //y up
        return false;
    if (bb[BOXBOTTOM]>hackbbox[BOXTOP])
        return false;
    if (bb[BOXLEFT]>hackbbox[BOXRIGHT])
        return false;
    if (bb[BOXRIGHT]<hackbbox[BOXLEFT])
        return false;
    return true;
}
#endif

// BP: big hack for a test in lighning ref:1249753487AB
int *bbox;

static void HWR_RenderBSPNode (int bspnum)
{
    node_t*     bsp;
    int         side;

    // Found a subsector?
    if (bspnum & NF_SUBSECTOR)
    {
        if (bspnum == -1) {
            //*(gr_drawsubsector_p++) = 0;
            HWR_Subsector (0);
        }
        else {
            //*(gr_drawsubsector_p++) = bspnum&(~NF_SUBSECTOR);
            HWR_Subsector (bspnum&(~NF_SUBSECTOR));
        }
        return;
    }

    // not a subsector, a nodes
    bsp = &nodes[bspnum];

    // Decide which side the view point is on.
    side = R_PointOnSide (dup_viewx, dup_viewy, bsp);

    // BP: big hack for a test in lighning ref:1249753487AB
    bbox=bsp->bbox[side];

    // Recursively divide front space.
    HWR_RenderBSPNode (bsp->children[side]);

    // Possibly divide back space.
    if (HWR_CheckBBox (bsp->bbox[side^1]))
    {
        // BP: big hack for a test in lighning ref:1249753487AB
        bbox=bsp->bbox[side^1];
        HWR_RenderBSPNode (bsp->children[side^1]);
    }
}


/*
//
// Clear 'stack' of subsectors to draw
//
static void HWR_ClearDrawSubsectors (void)
{
    gr_drawsubsector_p = gr_drawsubsectors;
}


//
// Draw subsectors pushed on the drawsubsectors 'stack', back to front
//
static void HWR_RenderSubsectors (void)
{
    while (gr_drawsubsector_p > gr_drawsubsectors)
    {
        HWR_RenderBSPNode (
        lastsubsec->nextsubsec = bspnum & (~NF_SUBSECTOR);
    }
}
*/


// ==========================================================================
//                                                              FROM R_MAIN.C
// ==========================================================================

#ifdef NOCRAPPYMLOOK
angle_t fineanglefov = FIELDOFVIEW;
#endif

//BP : exactely the same as R_InitTextureMapping 
void HWR_InitTextureMapping (void)
{
    int                 i;
    int                 x;
    int                 t;
    fixed_t             focallength;
        
    fixed_t             grcenterx;
    fixed_t             grcenterxfrac;
    int                 grviewwidth;
#ifdef NOCRAPPYMLOOK
    angle_t    clipanglefov;
    static angle_t oldclipanglefov=0;

    clipanglefov = fineanglefov + 2*abs((int)aimingangle);
    if( clipanglefov==oldclipanglefov)
        return;
    oldclipanglefov = clipanglefov;
    clipanglefov >>= ANGLETOFINESHIFT;
    if( clipanglefov >= ((angle_t)ANG180 - (angle_t)ANGLE_1)>> ANGLETOFINESHIFT )
        clipanglefov = (ANG180 - ANGLE_1) >> ANGLETOFINESHIFT;
    
    CONS_Printf ("HW_InitTextureMapping() %d %d %d\n",clipanglefov,aimingangle>> ANGLETOFINESHIFT, fineanglefov >>ANGLETOFINESHIFT );
#else
#define clipanglefov (FIELDOFVIEW>>ANGLETOFINESHIFT)
#endif
    grviewwidth = VIDWIDTH;
    grcenterx = grviewwidth/2;
    grcenterxfrac = grcenterx<<FRACBITS;
    
    // Use tangent table to generate viewangletox:
    //  viewangletox will give the next greatest x
    //  after the view angle.
    //
    // Calc focallength
    //  so FIELDOFVIEW angles covers SCREENWIDTH.
    focallength = FixedDiv (grcenterxfrac,
                            finetangent[FINEANGLES/4+clipanglefov/2] );

    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
        if (finetangent[i] > FRACUNIT*2)
            t = -1;
        else if (finetangent[i] < -FRACUNIT*2)
            t = grviewwidth+1;
        else
        {
            t = FixedMul (finetangent[i], focallength);
            t = (grcenterxfrac - t+FRACUNIT-1)>>FRACBITS;

            if (t < -1)
                t = -1;
            else if (t>grviewwidth+1)
                t = grviewwidth+1;
        }
        gr_viewangletox[i] = t;
    }

    // Scan viewangletox[] to generate xtoviewangle[]:
    //  xtoviewangle will give the smallest view angle
    //  that maps to x.
    for (x=0; x <= grviewwidth; x++)
    {
        i = 0;
        while (gr_viewangletox[i]>x)
            i++;
        gr_xtoviewangle[x] = (i<<ANGLETOFINESHIFT)-ANG90;
    }

    // Take out the fencepost cases from viewangletox.
    for (i=0 ; i<FINEANGLES/2 ; i++)
    {
        if (gr_viewangletox[i] == -1)
            gr_viewangletox[i] = 0;
        else if (gr_viewangletox[i] == grviewwidth+1)
            gr_viewangletox[i]  = grviewwidth;
    }

    gr_clipangle = gr_xtoviewangle[0];
}


// ==========================================================================
// gr_things.c
// ==========================================================================

// sprites are drawn after all wall and planes are rendered, so that
// sprite translucency effects apply on the rendered view (instead of the background sky!!)

gr_vissprite_t     gr_vissprites[MAXVISSPRITES];
gr_vissprite_t*    gr_vissprite_p;

// --------------------------------------------------------------------------
// HWR_ClearSprites
// Called at frame start.
// --------------------------------------------------------------------------
static void HWR_ClearSprites (void)
{
    gr_vissprite_p = gr_vissprites;
}


// --------------------------------------------------------------------------
// HWR_NewVisSprite
// --------------------------------------------------------------------------
gr_vissprite_t  gr_overflowsprite;

gr_vissprite_t* HWR_NewVisSprite (void)
{
    if (gr_vissprite_p == &gr_vissprites[MAXVISSPRITES])
        return &gr_overflowsprite;

    gr_vissprite_p++;
    return gr_vissprite_p-1;
}


// -----------------+
// HWR_DrawSprite   : Draw flat sprites 
//                  : (monsters, bonuses, weapons, lights, ...)
// Returns          :
// -----------------+
static void HWR_DrawSprite( gr_vissprite_t* spr )
{
    int                 i;
    float               tr_x;
    float               tr_y;
    FOutVector          wallVerts[4];   
    FOutVector          *wv;
    GlidePatch_t        *gpatch;      //sprite patch converted to 3Dfx
    FSurfaceInfo        Surf;

    // cache sprite graphics
    //12/12/99: Hurdler: 
    //          OK, I don't change anything for MD2 support because I want to be 
    //          sure to do it the right way. So actually, we keep normal sprite
    //          in memory and we add the md2 model if it exists for that sprite
    gpatch = W_CachePatchNum (spr->patch, PU_CACHE ); 

#ifndef TANDL
    // moved before transformation
    HWR_DL_AddLight( spr, gpatch );
#endif

    // create the sprite billboard
    //    
    //  3--2
    //  | /|
    //  |/ |
    //  0--1
    wallVerts[0].x = wallVerts[3].x = spr->x1;
    wallVerts[2].x = wallVerts[1].x = spr->x2;
    wallVerts[2].y = wallVerts[3].y = spr->ty;
    wallVerts[0].y = wallVerts[1].y = spr->ty - gpatch->height;

    // make a wall polygon (with 2 triangles), using the floor/ceiling heights,
    // and the 2d map coords of start/end vertices 
    wallVerts[0].oow = wallVerts[1].oow = wallVerts[2].oow = wallVerts[3].oow = spr->tz;

    // transform
    wv = wallVerts;

    for (i=0; i<4; i++,wv++)
    {
        //look up/down ----TOTAL SUCKS!!!--- do the 2 in one!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        tr_x = wv->oow;
        tr_y = wv->y;
        wv->y   = (tr_x * gr_viewludcos) + (tr_y * gr_viewludsin);
        wv->oow = (tr_x * gr_viewludsin) - (tr_y * gr_viewludcos);
        // ---------------------- mega lame test ----------------------------------

        //scale y before frustum so that frustum can be scaled to screen height
        wv->y = wv->y * ORIGINAL_ASPECT; 
    }

    if (spr->flip) {
        wallVerts[0].sow = wallVerts[3].sow = gpatch->max_s;
        wallVerts[2].sow = wallVerts[1].sow = 0;
    }else{
        wallVerts[0].sow = wallVerts[3].sow = 0;
        wallVerts[2].sow = wallVerts[1].sow = gpatch->max_s;
    }
    wallVerts[3].tow = wallVerts[2].tow = 0;
    wallVerts[0].tow = wallVerts[1].tow = gpatch->max_t;

    // draw a corona if this sprite contain light(s)
    HWR_DoCoronasLighting(wallVerts, spr);

    // cache the patch in the graphics card memory
    //12/12/99: Hurdler: same comment as above (for md2)
    //Hurdler: 25/04/2000: now support colormap in hardware mode
    HWR_GetMappedPatch(gpatch, spr->colormap);

    // sprite (TODO: coloured-) lighting by modulating the RGB components
    Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = spr->sectorlight;
    
#ifdef TANDL
    // BP: no md2 without t&l
    //12/12/99: Hurdler: 
    //          See my previous comment: if md2 model is supported then draw  
    //          the sprite as an md2 model, otherwise, draw it like before
    // actually only for spider
    // Look at HWR_ProjetctSprite for more
    if (cv_grmd2.value && spr->type==SPR_BSPI)
    {
        // dont forget to enabled the depth test because we can't do this like 
        // before: polygons models are not sorted

        // 1. load model+texture if not already loaded
        // 2. draw model with correct position, rotation,...

        static int          first = 1;  // should use Z_Malloc,...
        static md2_model_t  *model;
        char                *ptr;
        char                filename[128];
        int                 i;
        static int          frame = 0;
        FTransform          p;

        if (first) {
            CONS_Printf ("Loading MD2... ");
            if ((model = md2_readModel ("md2/spider.md2")))
            {
                CONS_Printf (" OK\n");
                //md2_printModelInfo (model);
                first = 0;
            }
            else 
            {
                CONS_Printf (" FAILED\n");
                return;
            }
        }
        //Hurdler: Actually load only the first skin (index 0)
        //         Also, it suppose the texture is in a PCX file
        // This should be put in md2_loadTexture()
        strcpy(filename, "md2/");
        for (i=4, ptr=model->skins[0]; (*ptr) != '\0'; i++, ptr++)
        {
            if ((*ptr == '\\') || (*ptr == '/'))
                i = 3;
            else
                filename[i] = *ptr;
        }
        filename[i] = '\0';
        md2_loadTexture( filename );

        p.anglex = 0.0f;
        p.angley = -90.0f;
        //04/01/00: Hurdler: spr->xxx is not the good position for T&L
        //                   we cannot use sprites already transformed!
        //                   (look at r_opengl to know what is done)
        p.x = (spr->x1+spr->x2)/2.0f;
        p.y = (spr->ty - gpatch->height/2.0f);
        p.z = -spr->tz;
        //CONS_Printf("md2 pos: (%f, %f, %f) ;  transform angle: %f\n", p.x, p.y, p.z, t.anglex);
        
        HWD.pfnDrawMD2(model->glCommandBuffer, 
                       &model->frames[((frame++)/3)%model->header.numFrames],
                       &p);
    }
    else
#endif
    {
        int blend;
        if( spr->mobj->frame & FF_TRANSMASK )
        {
            switch((spr->mobj->frame & FF_TRANSMASK)>>FF_TRANSSHIFT)
            {
                case tr_transmed : Surf.FlatColor.s.alpha = 0x80;blend = PF_Translucent; break;
                case tr_transmor : Surf.FlatColor.s.alpha = 0x40;blend = PF_Translucent; break;
                case tr_transhi  : Surf.FlatColor.s.alpha = 0x30;blend = PF_Translucent; break;
                case tr_transfir : Surf.FlatColor.s.alpha = 0x80;blend = PF_Additive;    break;
                case tr_transfx1 : // TODO: convert sprite, using special alpha per pixel
                                   //       problem: don't work with 5551 opengl
                                   Surf.FlatColor.s.alpha = 0xA0;blend = PF_Additive;    break;
            }
        }
        else
        if( spr->mobj->frame & FF_SMOKESHADE )
        {
            Surf.FlatColor.s.alpha = 0x80;blend = PF_Translucent;
        }
        else if (spr->mobj->flags & MF_SHADOW)
        {
            Surf.FlatColor.s.alpha = 0x40;blend = PF_Translucent;
        }
        else
        {
            // BP: i agree that is little better in environement but it don't 
            //     work properly under glide nor with fogcolor to ffffff :(
            Surf.FlatColor.s.alpha = 0xFF;blend = PF_Translucent; //PF_Environment;
        }

        HWD.pfnDrawPolygon( &Surf, wallVerts, 4, PF_Modulated|blend|PF_Clip);
    }
}

// --------------------------------------------------------------------------
// Sort vissprites by distance
// --------------------------------------------------------------------------
static gr_vissprite_t     gr_vsprsortedhead;

static void HWR_SortVisSprites (void)
{
    int                 i;
    int                 count;
    gr_vissprite_t*     ds;
    gr_vissprite_t*     best=NULL;      //shut up compiler
    gr_vissprite_t      unsorted;
    float               bestdist;

    count = gr_vissprite_p - gr_vissprites;

    unsorted.next = unsorted.prev = &unsorted;

    if (!count)
        return;

    for (ds=gr_vissprites ; ds<gr_vissprite_p ; ds++)
    {
        ds->next = ds+1;
        ds->prev = ds-1;
    }

    gr_vissprites[0].prev = &unsorted;
    unsorted.next = &gr_vissprites[0];
    (gr_vissprite_p-1)->next = &unsorted;
    unsorted.prev = gr_vissprite_p-1;

    // pull the vissprites out by scale
    gr_vsprsortedhead.next = gr_vsprsortedhead.prev = &gr_vsprsortedhead;
    for (i=0 ; i<count ; i++)
    {
        bestdist = ZCLIP_PLANE-1;
        for (ds=unsorted.next ; ds!= &unsorted ; ds=ds->next)
        {
            if (ds->tz > bestdist)
            {
                bestdist = ds->tz;
                best = ds;
            }
        }
        best->next->prev = best->prev;
        best->prev->next = best->next;
        best->next = &gr_vsprsortedhead;
        best->prev = gr_vsprsortedhead.prev;
        gr_vsprsortedhead.prev->next = best;
        gr_vsprsortedhead.prev = best;
    }
}


// --------------------------------------------------------------------------
//  Draw all vissprites
// --------------------------------------------------------------------------
static void HWR_DrawSprites (void)
{
    gr_vissprite_t*        spr;

    // vissprites must be sorted, even with a zbuffer,
    // for the translucency effects to be correct
    HWR_SortVisSprites ();

    if (gr_vissprite_p > gr_vissprites)
    {
        // draw all vissprites back to front
        for (spr = gr_vsprsortedhead.next ;
             spr != &gr_vsprsortedhead ;
             spr = spr->next)
        {
            HWR_DrawSprite( spr );
        }
    }
}


// --------------------------------------------------------------------------
// HWR_AddSprites
// During BSP traversal, this adds sprites by sector.
// --------------------------------------------------------------------------
static unsigned char sectorlight;
static void HWR_AddSprites (sector_t* sec)
{
    mobj_t*             thing;

    // BSP is traversed by subsector.
    // A sector might have been split into several
    //  subsectors during BSP building.
    // Thus we check whether its already added.
    if (sec->validcount == validcount)
        return;

    // Well, now it will be done.
    sec->validcount = validcount;

    // sprite lighting
    sectorlight = (unsigned char)sec->lightlevel & 0xff;

    /*
    lightnum = (sec->lightlevel >> LIGHTSEGSHIFT) + extralight + BRIGHTEN_THE_DAMN_LIGHTLEVELS;
    if (lightnum < 0)
        spritelights = scalelight[0];
    else if (lightnum >= LIGHTLEVELS)
        spritelights = scalelight[LIGHTLEVELS-1];
    else
        spritelights = scalelight[lightnum];
        */

    // Handle all things in sector.
    for (thing = sec->thinglist ; thing ; thing = thing->snext)
    {
        HWR_ProjectSprite (thing);
    }
}

// --------------------------------------------------------------------------
// HWR_ProjectSprite
//  Generates a vissprite for a thing if it might be visible.
// --------------------------------------------------------------------------
// BP why not use xtoviexangle/viewangletox like in bsp ?....
static void HWR_ProjectSprite (mobj_t* thing)
{
    gr_vissprite_t*     vis;

    float               tr_x;
    float               tr_y;
    
    float               tx;
    float               ty;
    float               tz;

    float               x1;
    float               x2;

    spritedef_t*        sprdef;
    spriteframe_t*      sprframe;
    int                 lump;
    unsigned            rot;
    boolean             flip;
    angle_t             ang;

    // transform the origin point
    tr_x = ((float)thing->x * crapmul) - gr_viewx;
    tr_y = ((float)thing->y * crapmul) - gr_viewy;

    // rotation around vertical axis
    tz = (tr_x * gr_viewcos) + (tr_y * gr_viewsin );

    // thing is behind view plane?
    if (tz < ZCLIP_PLANE)
        return;

    tx = (tr_x * gr_viewsin) - (tr_y * gr_viewcos );

    // decide which patch to use for sprite relative to player
#ifdef RANGECHECK
    if ((unsigned)thing->sprite >= numsprites)
        I_Error ("HWR_ProjectSprite: invalid sprite number %i ",
                 thing->sprite );
#endif

    //Fab:02-08-98: 'skin' override spritedef currently used for skin
    if (thing->skin)
        sprdef = &((skin_t *)thing->skin)->spritedef;
    else
        sprdef = &sprites[thing->sprite];

#ifdef RANGECHECK
    if ( (thing->frame&FF_FRAMEMASK) >= sprdef->numframes )
        I_Error ("HWR_ProjectSprite: invalid sprite frame %i : %i ",
                 thing->sprite, thing->frame );
#endif
    sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

    if (sprframe->rotate) {
        // choose a different rotation based on player view
        ang = R_PointToAngle (thing->x, thing->y );          // uses viewx,viewy
        rot = (ang-thing->angle+(unsigned)(ANG45/2)*9)>>29;
        //Fab: lumpid is the index for spritewidth,spriteoffset... tables
        lump = sprframe->lumpid[rot];
        flip = (boolean)sprframe->flip[rot];
    } else {
        // use single rotation for all views
        rot = 0;                        //Fab: for vis->patch below
        lump = sprframe->lumpid[0];     //Fab: see note above
        flip = (boolean)sprframe->flip[0];
    }
    

    // calculate edges of the shape
    tx -= ((float)spriteoffset[lump] * crapmul );

    // project x
    x1 = gr_windowcenterx + (tx * gr_centerx / tz );

    // BP: FOV des sprites, c'est ici que sa ce passe
    // left edge off the right side?
#ifdef NOCRAPPYMLOOK
    if (x1 > gr_viewwidth)
#else
    if ((x1 > gr_viewwidth) && (cv_grfov.value<=90) /*&& !cv_grmd2.value*/) 
#endif
        return;

  //faB:tr_x doesnt matter
    // hurdler: it's used in cliptosolidsegs
  tr_x = x1;

    x1 = tx;

    tx += ((float)spritewidth[lump] * crapmul );
    x2 = gr_windowcenterx + (tx * gr_centerx / tz );

    // BP: FOV des sprites, ici aussi
    // right edge off the left side
#ifdef NOCRAPPYMLOOK
    if (x2 < 0)
#else
    if ((x2 < 0) && (cv_grfov.value<=90) /*&& !cv_grmd2.value*/) 
#endif
        return;

    // sprite completely hidden ?
#ifdef NOCRAPPYMLOOK
    if (!HWR_ClipToSolidSegs((int)tr_x,(int)x2))
#else
    if ((!HWR_ClipToSolidSegs((int)tr_x,(int)x2)) && (cv_grfov.value<=90) /*&& !cv_grmd2.value*/)
#endif
        return;

    //
    // store information in a vissprite
    //
    vis = HWR_NewVisSprite ( );
    vis->x1 = x1;
    vis->x2 = tx;
    vis->tz = tz;
    vis->patch = sprframe->lumppat[rot];
    vis->flip = flip;
    //01/11/99: Hurdler: added so coronas works with all wad, at last!
    vis->type = thing->sprite;
    //Hurdler: 25/04/2000: now support colormap in hardware mode
    if (thing->flags & MF_TRANSLATION)
        vis->colormap = (byte *) translationtables - 256 + ( (thing->flags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) );
    else
        vis->colormap = colormaps;

    // set top/bottom coords
    ty = ((float)thing->z * crapmul) - gr_viewz + ((float)spritetopoffset[lump] * crapmul );
    vis->ty = ty;
    
    // for no floating mobj add a little offset in y so he dosen't have feet in floor
    // (z-buffer problem)
    // mouais: sauf que les torches flottent maintenant!
    if((thing->flags & MF_FLOAT) == 0)
        vis->ty += 4.0;
    vis->mobj = thing;


    //CONS_Printf("------------------\nH: sprite  : %d\nH: frame   : %x\nH: type    : %d\nH: sname   : %s\n\n",
    //            thing->sprite, thing->frame, thing->type, sprnames[thing->sprite]);

    if( thing->state->frame & FF_FULLBRIGHT || fixedcolormap )
        // TODO: disable also the fog
        vis->sectorlight = 0xff;
    else if( sectorlight + BRIGHTEN_THE_DAMN_LIGHTLEVELS < 256 )
        vis->sectorlight = sectorlight + BRIGHTEN_THE_DAMN_LIGHTLEVELS;
    else
        vis->sectorlight = 255;
}


#define BASEYCENTER           (BASEVIDHEIGHT/2)
// -----------------+
// HWR_DrawPSprite  : Draw 'player sprites' : weapons, etc.
//                  : fSectorLight ranges 0...1
// -----------------+
void HWR_DrawPSprite( pspdef_t* psp, int lightlevel)
{
    spritedef_t*        sprdef;
    spriteframe_t*      sprframe;
    int                 lump;
    boolean             flip;

    wallVert3D          wallVerts[4];
    FOutVector          projVerts[4];
    int                 i;
    wallVert3D          *wv;
    float               tx;
    float               ty;
    float               x1;
    float               x2;

    GlidePatch_t*       gpatch;      //sprite patch converted to 3Dfx

    FSurfaceInfo        Surf;

    // decide which patch to use
#ifdef RANGECHECK
    if ( (unsigned)psp->state->sprite >= numsprites)
        I_Error ("HWR_ProjectSprite: invalid sprite number %i ",
                 psp->state->sprite );
#endif
    sprdef = &sprites[psp->state->sprite];
#ifdef RANGECHECK
    if ( (psp->state->frame & FF_FRAMEMASK)  >= sprdef->numframes)
        I_Error ("HWR_ProjectSprite: invalid sprite frame %i : %i ",
                 psp->state->sprite, psp->state->frame );
#endif
    sprframe = &sprdef->spriteframes[ psp->state->frame & FF_FRAMEMASK ];

    //Fab:debug
    //if (sprframe==NULL)
    //    I_Error("sprframes NULL for state %d\n", psp->state - states );

    lump = sprframe->lumpid[0];
    flip = (boolean)sprframe->flip[0];

    // calculate edges of the shape

    tx = (float)(psp->sx - ((BASEVIDWIDTH/2)<<FRACBITS)) * crapmul;
    tx -= ((float)spriteoffset[lump] * crapmul );
    x1 = gr_windowcenterx + (tx * gr_pspritexscale );
    
    wallVerts[3].x = wallVerts[0].x = tx;

    tx += ((float)spritewidth[lump] * crapmul );
    x2 = gr_windowcenterx + (tx * gr_pspritexscale) - 1;

    wallVerts[2].x = wallVerts[1].x = tx;


    //  3--2
    //  | /|
    //  |/ |
    //  0--1
    wallVerts[0].z = wallVerts[1].z = wallVerts[2].z = wallVerts[3].z = 1;
    wallVerts[0].w = wallVerts[1].w = wallVerts[2].w = wallVerts[3].w = 1;

    // cache sprite graphics
    gpatch = W_CachePatchNum (sprframe->lumppat[0], PU_CACHE);
    HWR_GetPatch (gpatch);

    // set top/bottom coords
    ty = (float)(psp->sy - spritetopoffset[lump]) * crapmul;
    if(cv_splitscreen.value && (cv_grfov.value==90))
        ty -= 20; //Hurdler: so it's a bit higher
    wallVerts[3].y = wallVerts[2].y = (float)BASEYCENTER - ty;

    ty += gpatch->height;
    wallVerts[0].y = wallVerts[1].y = (float)BASEYCENTER - ty;
    
    if (flip)
    {
        wallVerts[0].s = wallVerts[3].s = gpatch->max_s;
        wallVerts[2].s = wallVerts[1].s = 0.0f;
    }
    else
    {
        wallVerts[0].s = wallVerts[3].s = 0.0f;
        wallVerts[2].s = wallVerts[1].s = gpatch->max_s;
    }
    wallVerts[3].t = wallVerts[2].t = 0.0f;
    wallVerts[0].t = wallVerts[1].t = gpatch->max_t;

    // project clipped vertices
    wv = wallVerts;
    for (i=0; i<4; i++,wv++)
    {
        //Hurdler: sorry, I had to multiply all by 4 for correct splitscreen mode
        projVerts[i].x = wv->x/40.0f;
        projVerts[i].y = wv->y/25.0f;
        projVerts[i].oow = 4.0f;
        projVerts[i].sow = wv->s;
        projVerts[i].tow = wv->t;
    }

    // clip 2d polygon to view window
    //wClipVerts = ClipToView (projVerts, outVerts, 4 );

    // set transparency and light level
    
    if( viewplayer->mo->flags & MF_SHADOW )
    {
        if( viewplayer->powers[pw_invisibility] > 4*TICRATE ||
            viewplayer->powers[pw_invisibility] & 8 )
            Surf.FlatColor.s.alpha = 0xff/3;
        else
            Surf.FlatColor.s.alpha = 2*0xff/3;
    }
    else
        Surf.FlatColor.s.alpha = 0xff;

    if( fixedcolormap )
    {
        // TODO: better handling of fixedcolormap
        // set the modulate RGB from fixedcolormap 'tint'
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = 0xff;
    }
    else if( psp->state->frame & FF_FULLBRIGHT )
    {
        // TODO: remove fog for this sprite !
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = 0xff;
    }
    else
    {
        // default opaque mode using alpha 0 for holes
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = lightlevel;
        
    }
    // invis player doesnt look good with PF_Environment so use PF_Additive instead
    if(viewplayer->powers[pw_invisibility])
        HWD.pfnDrawPolygon( &Surf, projVerts, 4, PF_Modulated|PF_Translucent|PF_NoDepthTest);
    else
        HWD.pfnDrawPolygon( &Surf, projVerts, 4, PF_Modulated|PF_Environment|PF_NoDepthTest);
}


// --------------------------------------------------------------------------
// HWR_DrawPlayerSprites
// --------------------------------------------------------------------------
static void HWR_DrawPlayerSprites( void )
{
    int         i;
    pspdef_t*   psp;
    int         lightlevel;

    // get light level
    lightlevel = viewplayer->mo->subsector->sector->lightlevel + BRIGHTEN_THE_DAMN_LIGHTLEVELS;
    if( lightlevel > 255 )
        lightlevel = 255;

    // add all active psprites
    for (i=0, psp=viewplayer->psprites;
         i<NUMPSPRITES;
         i++,psp++)
    {
        if (psp->state)
            HWR_DrawPSprite( psp, lightlevel );
    }
}


// ==========================================================================
//
// ==========================================================================
void HWR_DrawSkyBackground( player_t* player )
{
    FOutVector      v[4];
    angle_t         angle;
    float f;

//  3--2
//  | /|
//  |/ |
//  0--1
    HWR_GetTexture( skytexture );

    //Hurdler: the sky is the only texture who need 4.0f instead of 1.0
    //         because it's called just after clearing the screen
    //         and thus, the near clipping plane is set to 3.99
    v[0].x = v[3].x = -4.0f;
    v[1].x = v[2].x =  4.0f;
    v[0].y = v[1].y = -4.0f;
    v[2].y = v[3].y =  4.0f;

    v[0].oow = v[1].oow = v[2].oow = v[3].oow = 4.0f;

    angle = (dup_viewangle + gr_xtoviewangle[0])>>ANGLETOSKYSHIFT;
    angle &= 255;

    v[0].sow = v[3].sow = 255.0/320.0+((float)angle)/255.0f;
    v[2].sow = v[1].sow = ((float)angle)/255.0f;

    f=20+200*FIXED_TO_FLOAT(finetangent[(2048-((int)aimingangle>>(ANGLETOFINESHIFT+1))) & FINEMASK]);
    if(f<0) f=0;
    if(f>240-127) f=240-127;
        v[3].tow = v[2].tow = f/127.0f;   
        v[0].tow = v[1].tow = (f+127)/127.0f;    //suppose 256x128 sky...

    HWD.pfnDrawPolygon( NULL, v, 4, 0 );
}


// -----------------+
// HWR_ClearView : clear the viewwindow, with maximum z value
// -----------------+
void HWR_ClearView( void )
{
    //  3--2
    //  | /|
    //  |/ |
    //  0--1

    //FIXTHIS faB - enable depth mask, disable color mask
    
    HWD.pfnClipRect ((int)gr_viewwindowx,
                     (int)gr_viewwindowy,
                     (int)(gr_viewwindowx + gr_viewwidth),
                     (int)(gr_viewwindowy + gr_viewheight),
                     3.99f );
    HWD.pfnClearBuffer( false, true, 0 );
    
    //disable clip window - set to full size
    // rem by Hurdler
    // HWD.pfnClipRect (0,0,VIDWIDTH,VIDHEIGHT );
}

        
// -----------------+
// HWR_SetViewSize  : set projection and scaling values depending on the 
//                  : view window size
// -----------------+
void HWR_SetViewSize( int blocks )
{
    // setup view size

    // clamping viewsize is normally not needed coz it's done in R_ExecuteSetViewSize()
    // BEFORE calling here
    if ( blocks<3 || blocks>12 )
        blocks = 10;
    if ( blocks > 10 ) {
        gr_viewwidth = (float)VIDWIDTH;
        gr_viewheight = (float)VIDHEIGHT;
    } else {
        gr_viewwidth = (float) ((blocks*VIDWIDTH/10) & ~7 );
        gr_viewheight = (float) ((blocks*(VIDHEIGHT-STAT_HEIGHT/2)/10) & ~1 );
    }

    if( cv_splitscreen.value )
         gr_viewheight /= 2;

    gr_centerx = gr_viewwidth / 2;
    gr_basecentery = gr_viewheight / 2; //note: this is (gr_centerx * gr_viewheight / gr_viewwidth)

    gr_viewwindowx = (VIDWIDTH - gr_viewwidth) / 2;
    gr_windowcenterx = (float)(VIDWIDTH / 2 );
    if (gr_viewwidth == VIDWIDTH) {
        gr_baseviewwindowy = 0;
        gr_basewindowcentery = gr_viewheight / 2;               // window top left corner at 0,0
    } else {
        gr_baseviewwindowy = (VIDHEIGHT-STAT_HEIGHT/2-gr_viewheight) / 2;
        gr_basewindowcentery = (float)((VIDHEIGHT-STAT_HEIGHT/2) / 2 );
    }

    gr_pspritexscale = gr_viewwidth / BASEVIDWIDTH; 
    gr_pspriteyscale = ((VIDHEIGHT*gr_pspritexscale*BASEVIDWIDTH)/BASEVIDHEIGHT)/VIDWIDTH;
}


// -----------------+
// HWR_SetTexturedDraw 
//                  : Activate texture color source
// -----------------+
void HWR_SetTexturedDraw( void )
{

    //FIXTHIS faB -- hmm...

    /*
    GrTextureFilterMode_t   grfilter;

    HWD.pfnSetStat( HWD_SET_TEXTURECOMBINE, HWD_TEXTURECOMBINE_NORMAL );

    grfilter = cv_grfiltermode.value ? HWD_SET_TEXTUREFILTER_BILINEAR : HWD_SET_TEXTUREFILTER_POINTSAMPLED;
    HWD.pfnSetStat( HWD_SET_TEXTUREFILTERMODE, grfilter );

    HWD.pfnSetStat( HWD_SET_MIPMAPMODE, HWD_MIPMAP_DISABLE );

    HWD.pfnSetStat( HWD_SET_COLORSOURCE, HWD_COLORSOURCE_CONSTANTALPHA_SCALE_TEXTURE );
    //note: use the alpha constant per-subsector, using the sector lightlevel as alpha constant!
    HWD.pfnSetStat( HWD_SET_ALPHASOURCE, HWD_ALPHASOURCE_CONSTANT );

    HWD_PFN_SET_CONSTANTCOLOR(0xffffffff );
    //HWD.pfnSetStat( HWD_SET_CONSTANTCOLOR, 0xffffffff );
    */
}


#ifdef BACK_TO_FRONT
//
//
//
static void HWR_RenderBackToFront (void)
{
    btofpoly_t* p;
    int         num;
    subsector_t* sub;
    sector_t*   sec;
    int         lastsubsec = -1;
    unsigned char light;
    int             lightnum;
    fixed_t                 wh;

    if (!numbtofpolys)
        return;

#ifdef PARANOIA
    if (numbtofpolys>MAXBTOFPOLYS)
        I_Error ("numbtofpolys error");
#endif
    
    p = btofpolys + numbtofpolys - 1;

    for ( ; p>=btofpolys; p--)
    {
        // Found a wall or a plane ?
        if (!(p->flags & BTOF_PLANE))
        {
            //TODO: get startfrac, endfrac from btofpoly
            HWR_DrawWallRange (p->line, 0, 1);
        }
        else
        {
            num = p->flags & BTOF_SUBSECNUM;

            if (num<numsubsectors)
            {
                // subsector
                sub = &subsectors[num];
                // sector
                sec = sub->sector;

                // per sector states
                if (num != lastsubsec)
                {
                    lastsubsec = num;

                    // sector lighting
                    //light = (unsigned char)sec->lightlevel & 0xff;
                    //HWD.pfnSetStat( HWD_SET_CONSTANTCOLOR, (light<<24) | 0xf00000 );

                    //  use different light tables
                    //  for horizontal / vertical / diagonal
                    //  note: try to get the same visual feel as the original
                    if (!fixedcolormap)
                    {
                        lightnum = sec->lightlevel + (extralight<<4);
                        if (lightnum > 255)
                            lightnum = 255;
                        HWD_PFN_SET_CONSTANTCOLOR(((lightnum&0xff)<<24) | 0xf00000 );
                    }
                }

                // test du back to front
                if (p->flags & BTOF_WATERHACK)
                {
                    wh = ((-sec->tag) <<16) + (FRACUNIT/2 );
                    if (wh > sec->floorheight &&
                        wh < sec->ceilingheight )
                    {
                        HWR_GetFlat ( doomwaterflat );

                        HWD.pfnSetStat( HWD_SET_ALPHABLEND, HWD_ALPHABLEND_TRANSLUCENT );
                        HWD_PFN_SET_CONSTANTCOLOR((0xc0<<24) | 0x000000 );

                        HWD.pfnSetStat( HWD_SET_DEPTHMASK, false );
                        HWR_RenderPlane( &extrasubsectors[num], wh );
                        HWD.pfnSetStat( HWD_SET_DEPTHMASK, true );
                        HWD.pfnSetStat( HWD_SET_ALPHABLEND, HWD_ALPHABLEND_NONE );
                    }
                }
                else
                if (p->flags & BTOF_CEILING) {
                    // ceiling
                    HWR_GetFlat ( levelflats[sec->ceilingpic].lumpnum );
                    HWR_RenderPlane (&extrasubsectors[num], sec->ceilingheight );
                }
                else
                {                      
                    // floor
                    HWR_GetFlat ( levelflats[sec->floorpic].lumpnum );
                    HWR_RenderPlane (&extrasubsectors[num], sec->floorheight );
                }            
            }
        }
    }
    numbtofpolys = 0;
}
#endif

//25/08/99: added by Hurdler for splitscreen correct palette changes and overlay
extern player_t *plyr;
void ST_doPaletteStuff( void );
void ST_overlayDrawer(int playernum );

// ==========================================================================
//
// ==========================================================================
void HWR_RenderPlayerView (int viewnumber, player_t* player)
{
    //static float    distance = BASEVIDWIDTH;

    //31/08/99: added by Hurdler for splitscreen correct palette changes 
    //                             & splitscreen dynamic lighting
    {
        // do we really need to save player (is it not the same)?
        player_t* saved_player = plyr;
        plyr = player;
        ST_doPaletteStuff( );
        plyr = saved_player;
        HWR_SetLights(viewnumber);
    }

    // note: sets viewangle, viewx, viewy, viewz
    R_SetupFrame (player );
    
    // copy view cam position for local use
    dup_viewx = viewx;
    dup_viewy = viewy;
    dup_viewz = viewz;
    dup_viewangle = viewangle;
    
    // set window position
    gr_centery = gr_basecentery;
    gr_viewwindowy = gr_baseviewwindowy;
    gr_windowcentery = gr_basewindowcentery;
    if ( cv_splitscreen.value && viewnumber==1 ) {
        //gr_centery += (VIDHEIGHT/2 );
        gr_viewwindowy += (VIDHEIGHT/2 );
        gr_windowcentery += (VIDHEIGHT/2 );
    }

    // hmm solidsegs probably useless here
    //R_ClearDrawSegs ( );
    // useless
    //R_ClearPlanes (player );
    //HWR_ClearSprites ( );

    // check for new console commands.
    NetUpdate ( );
        
    gr_viewx = ((float)dup_viewx) * crapmul;
    gr_viewy = ((float)dup_viewy) * crapmul;
    gr_viewz = ((float)dup_viewz) * crapmul;
    gr_viewsin = FIXED_TO_FLOAT(viewsin);
    gr_viewcos = FIXED_TO_FLOAT(viewcos);

    gr_viewludsin = FIXED_TO_FLOAT(finecosine[aimingangle>>ANGLETOFINESHIFT]);
    gr_viewludcos = FIXED_TO_FLOAT(- finesine[aimingangle>>ANGLETOFINESHIFT]);

#ifdef TANDL
    //04/01/2000: Hurdler: added for T&L
    //                     It should replace all other gr_viewxxx when finished
    transform.anglex = (float)(aimingangle>>ANGLETOFINESHIFT)*(360.0f/(float)FINEANGLES);
    transform.angley = (float)(viewangle>>ANGLETOFINESHIFT)*(360.0f/(float)FINEANGLES);
    transform.x      = gr_viewx;  // viewx * crapmul
    transform.y      = gr_viewy;  // viewy * crapmul
    transform.z      = gr_viewz;  // viewz * crapmul
#endif

#ifdef NOCRAPPYMLOOK
    // enlage fOV when looking up/down
    HWR_InitTextureMapping ();
#endif

    //------------------------------------------------------------------------
    HWR_ClearView ();


    if( !drawtextured )
    {
        // shaded colour mode 
        //FIXTHIS faB - set global no texture mode ?
    }
    else
    {
        // set texture mapped drawing 
        HWR_SetTexturedDraw( );
    }

    if (cv_grfog.value)
        HWR_FoggingOn ();

//#ifndef POLYSKY
    if (cv_grsky.value)
        HWR_DrawSkyBackground (player);
//#endif
    //Hurdler: it doesn't work in splitscreen mode
    //cv_grsky.value = false;
    cv_grsky.value = cv_splitscreen.value;

    // added by Hurdler for FOV 120
    if (cv_grfov.value != 90)
        HWD.pfnSetSpecialState(HWD_SET_FOV, cv_grfov.value);

#ifdef TANDL
    //14/11/99: Hurdler: we will add lights while processing sprites
    //it doesn't work with all subsectors (if we use AddSprites to do that).
    //TOO bad, that's why I removed this line (until this is fixed).
//    HWR_ResetLights();
#endif

    HWR_ClearSprites ( );

    HWR_ClearClipSegs ( );

    //04/01/2000: Hurdler: added for T&L
    //                     Actually it only works on Walls and Planes
#ifdef TANDL
    HWD.pfnSetTransform(&transform);
#endif

#ifdef BACK_TO_FRONT
    numbtofpolys = 0;
    HWR_RenderBSPNode (numnodes-1);
    HWR_RenderBackToFront ();
#else
    HWR_RenderBSPNode (numnodes-1);
#endif


#ifndef NOCRAPPYMLOOK
    if (cv_grcrappymlook.value && (aimingangle || cv_grfov.value>90))
    {
        dup_viewangle += ANG90;
        HWR_ClearClipSegs ();
        HWR_RenderBSPNode (numnodes-1); //lefT

        dup_viewangle += ANG90;
        if (cv_grcrappymlook.value == 2 && ((int)aimingangle>ANG45 || (int)aimingangle<-ANG45)) {
            HWR_ClearClipSegs ();
            HWR_RenderBSPNode (numnodes-1); //back
        }

        dup_viewangle += ANG90;
        HWR_ClearClipSegs ();
        HWR_RenderBSPNode (numnodes-1); //right
        
        dup_viewangle += ANG90;
    }
#endif

    // Check for new console commands.
    NetUpdate ();

#ifdef TANDL
    //04/01/2000: Hurdler: added for T&L
    //            Draw the sprites like it was done previously without T&L
    //      note: md2 actually use T&L !!!
    HWD.pfnSetTransform(NULL);
#endif

#ifndef TANDL
    //14/11/99: Hurdler: moved here because it doesn't work with 
    // subsector, see other comments;
    HWR_ResetLights();
#endif

    HWR_DrawSprites ();

    // Check for new console commands.
    NetUpdate ();

    // added by Hurdler for FOV 120: restore normal FOV (FOV 90)
    if (cv_grfov.value != 90)
        HWD.pfnSetSpecialState(HWD_SET_FOV, 90);

    // draw the psprites on top of everything
    //  but does not draw on side views
    if (!viewangleoffset && !camera.chase && cv_psprites.value)
        HWR_DrawPlayerSprites ();

    // added by Hurdler for correct splitscreen
    // moved here by hurdler so it works with the new near clipping plane
    HWD.pfnClipRect (0,0,VIDWIDTH,VIDHEIGHT, 0.9f );

    //------------------------------------------------------------------------
    // put it off for menus etc
    if (cv_grfog.value)
        HWD.pfnSetSpecialState( HWD_SET_FOG_MODE, 0 );

        // Check for new console commands.
    //NetUpdate ();

}


// ==========================================================================
//                                                                        FOG
// ==========================================================================


//FIXTHIS faB

static unsigned int atohex(char* s)
{
    int     iCol;
    char*   sCol;
    char    cCol;  
    int i;

    if (lstrlen(s)<6)
        return 0;

    iCol = 0;
    sCol = s;
    for (i=0; i<6; i++, sCol++)
    {
        iCol <<=4;
        cCol = *sCol;
        if (cCol>='0' && cCol<='9')
            iCol |= cCol-'0';
        else
        {
            if (cCol>='F')
                cCol -= ('a'-'A');
            if (cCol>='A' && cCol<='F')
                iCol = iCol | (cCol-'A'+10);
        }
    }
    //CONS_Printf ("col %x\n", iCol);
    return iCol;
}


void HWR_FoggingOn (void)
{
    HWD.pfnSetSpecialState( HWD_SET_FOG_COLOR, atohex(cv_grfogcolor.string) );
    HWD.pfnSetSpecialState( HWD_SET_FOG_DENSITY, cv_grfogdensity.value );
    HWD.pfnSetSpecialState( HWD_SET_FOG_MODE, 1 );
}


// ==========================================================================
//                                                         3D ENGINE COMMANDS
// ==========================================================================


static void CV_grFov_OnChange (void)
{
    if ((cv_grfov.value <= 0) || (cv_grfov.value >= 180))
        CV_SetValue(&cv_grfov, 90);
    // autoset mlook when FOV > 90
    if ((!cv_grcrappymlook.value) && (cv_grfov.value > 90))
        CV_SetValue(&cv_grcrappymlook, 1);
}

static void CV_grPolygonSmooth_OnChange (void)
{
    HWD.pfnSetSpecialState (HWD_SET_POLYGON_SMOOTH, cv_grpolygonsmooth.value);
}
/*   
static void CV_grFogColor_OnChange (void)
{
    //HWD.pfnSetSpecialState (HWD_SET_FOG_COLOR, atohex(cv_grfogcolor.string));
}
*/
static  void    Command_GrStats_f ( void)
{
    //debug
    Z_CheckHeap (9875);

    CONS_Printf ("Patch info headers:            %d kb\n", Z_TagUsage(PU_3DFXPATCHINFO)>>10);
    CONS_Printf ("3D-converted graphics in heap: %d kb\n", Z_TagUsage(PU_3DFXCACHE)>>10);
}


// **************************************************************************
//                                                            3D ENGINE SETUP
// **************************************************************************

// --------------------------------------------------------------------------
// Add hardware engine commands & consvars
// --------------------------------------------------------------------------
//added by Hurdler: console varibale that are saved
void HWR_AddCommands (void)
{
    CV_RegisterVar (&cv_grgammablue);
    CV_RegisterVar (&cv_grgammagreen);
    CV_RegisterVar (&cv_grgammared);
    //CV_RegisterVar (&cv_grcontrast);
    //CV_RegisterVar (&cv_grpolygonsmooth); // moved below
    CV_RegisterVar (&cv_grmd2);
    CV_RegisterVar (&cv_grmblighting);
    CV_RegisterVar (&cv_grstaticlighting);
    CV_RegisterVar (&cv_grdynamiclighting);
    CV_RegisterVar (&cv_grcoronas);
    CV_RegisterVar (&cv_grfov);
    CV_RegisterVar (&cv_grfogdensity);
    CV_RegisterVar (&cv_grfogcolor);
    CV_RegisterVar (&cv_grfog);
    CV_RegisterVar (&cv_grcrappymlook);
}

void HWR_AddEngineCommands (void)
{
    CV_RegisterVar (&cv_grpolygonsmooth);

    // engine state variables
    //CV_RegisterVar (&cv_grsky);
    CV_RegisterVar (&cv_grfiltermode);
    CV_RegisterVar (&cv_grzbuffer);
    //CV_RegisterVar (&cv_grclipwalls);
    CV_RegisterVar (&cv_grrounddown);

    // engine development mode variables
    // - usage may vary from version to version..
    CV_RegisterVar (&cv_gralpha);

    // ATTENTION beta utiliser pour des test, ne pas le declarer en
    // release !
    CV_RegisterVar (&cv_grbeta);
    CV_RegisterVar (&cv_grgamma);

    // engine commands
    COM_AddCommand ("gr_stats", Command_GrStats_f);
}


// --------------------------------------------------------------------------
// Setup the hardware renderer
// --------------------------------------------------------------------------
void HWR_Startup ( void )
{
static BOOL startupdone = FALSE;
        
    CONS_Printf ("HWR_Startup()\n");

    // setup GlidePatch_t scaling
    gr_patch_scalex = 1.0 / (float)VIDWIDTH;
    gr_patch_scaley = 1.0 / (float)VIDHEIGHT;

    // do this once
    if ( startupdone == FALSE )
    {
        HWR_InitPolyPool ();
        // add console cmds & vars
        HWR_AddEngineCommands ();
        HWR_InitTextureCache ();

#ifdef BACK_TO_FRONT
        // allocate back to front table
        btofpolys = malloc(sizeof(btofpoly_t) * MAXBTOFPOLYS);
        if (btofpolys==NULL)
            I_Error ("Out of memory");
#endif
        // for test water translucent surface
        // changed by Hurdler for Doom1 shareware compatibility
        doomwaterflat  = W_CheckNumForName ("FWATER1");
        if (doomwaterflat == -1) // if FWATER1 not found
            doomwaterflat = W_GetNumForName ("WATER0");
    }

    // initially set textured draw for Console startup
    HWR_SetTexturedDraw();
    HWR_InitLight();

    startupdone = TRUE;
}


// --------------------------------------------------------------------------
// Free resources allocated by the hardware renderer
// --------------------------------------------------------------------------
void HWR_Shutdown (void)
{
    CONS_Printf ("HWR_Shutdown()\n");
    
    HWR_FreeExtraSubsectors ();

//20/08/99: added by Hurdler (taken from faB's code)
#ifdef BACK_TO_FRONT
    if (btofpolys)
        free (btofpolys);
#endif

    HWR_FreePolyPool ();
    HWR_FreeTextureCache ();
}
