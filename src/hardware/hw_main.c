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
/// \brief hardware renderer, using the standard HardWareRender driver DLL for SRB2

#include <math.h>

#include "hw_glob.h"
#include "hw_light.h"
#include "hw_drv.h"

#include "../doomstat.h"
#include "../i_video.h" // for rendermode == render_glide
#include "../v_video.h"
#include "../p_local.h"
#include "../p_setup.h"
#include "../r_local.h"
#include "../r_bsp.h"
#include "../d_clisrv.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "../r_splats.h"
#include "../g_game.h"
#include "../st_stuff.h"

#define ZCLIP_PLANE 4.0f
#define R_FAKEFLOORS

// ==========================================================================
// the hardware driver object
// ==========================================================================
struct hwdriver_s hwdriver;

// ==========================================================================
//                                                                     PROTOS
// ==========================================================================


static void HWR_AddSprites(sector_t* sec);
static void HWR_ProjectSprite(mobj_t* thing);
static void HWR_ProjectPrecipitationSprite(precipmobj_t* thing);
static void HWR_Add3DWater(int lumpnum, extrasubsector_t* xsub, fixed_t fixedheight,
                           int lightlevel, int alpha);
static void HWR_Render3DWater(void);
static void HWR_RenderTransparentWalls(void);
static void HWR_FoggingOn (void);
static unsigned int atohex(const char* s);

static void CV_filtermode_ONChange(void);
static void CV_FogDensity_ONChange(void);
static void CV_grFov_OnChange(void);
static void CV_grPolygonSmooth_OnChange(void);
// ==========================================================================
//                                          3D ENGINE COMMANDS & CONSOLE VARS
// ==========================================================================

static CV_PossibleValue_t grfov_cons_t[] = {{0, "MIN"}, {179*FRACUNIT, "MAX"}, {0, NULL}};
static CV_PossibleValue_t grfiltermode_cons_t[]= {{HWD_SET_TEXTUREFILTER_POINTSAMPLED, "Nearest"},
   {HWD_SET_TEXTUREFILTER_BILINEAR, "Bilinear"}, {HWD_SET_TEXTUREFILTER_TRILINEAR, "Trilinear"},
   {HWD_SET_TEXTUREFILTER_MIXED1, "Linear_Nearest"},
   {HWD_SET_TEXTUREFILTER_MIXED2, "Nearest_Linear"}, {0, NULL}};

static consvar_t cv_grsky = {"gr_sky", "On", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
// console variables in development
static consvar_t cv_grpolygonsmooth = {"gr_polygonsmooth", "Off", CV_CALL, CV_OnOff,
                                       CV_grPolygonSmooth_OnChange, 0, NULL, NULL, 0, 0, NULL};
// needs fix: walls are incorrectly clipped one column less
static const consvar_t cv_grclipwalls = {"gr_clipwalls", "Off", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

//development variables for diverse uses
static consvar_t cv_gralpha = {"gr_alpha", "160", 0, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};
static consvar_t cv_grbeta = {"gr_beta", "0", 0, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};
/// /todo: unused
static consvar_t cv_grgamma = {"gr_gamma","0", 0, CV_Unsigned, NULL, 0, NULL, NULL, 0, 0, NULL};

float grfovadjust = 0.0f;

consvar_t cv_grrounddown = {"gr_rounddown", "Off", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfov = {"gr_fov", "90", CV_FLOAT|CV_CALL, grfov_cons_t, CV_grFov_OnChange, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfogdensity = {"gr_fogdensity", "25", CV_CALL|CV_NOINIT, CV_Unsigned,
                             CV_FogDensity_ONChange, 0, NULL, NULL, 0, 0, NULL};

// Unfortunately, this can no longer be saved...
consvar_t cv_grfiltermode = {"gr_filtermode", "Bilinear", CV_CALL, grfiltermode_cons_t,
                             CV_filtermode_ONChange, 0, NULL, NULL, 0, 0, NULL};
//static consvar_t cv_grzbuffer = {"gr_zbuffer", "On", 0, CV_OnOff};
consvar_t cv_grcorrecttricks = {"gr_correcttricks", "On", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grsolvetjoin = {"gr_solvetjoin", "On", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

// console variables in development
consvar_t cv_grmd2 = {"gr_md2", "Off", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grtranswall = {"gr_transwall", "On", 0, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

void CV_FogDensity_ONChange(void)
{
	HWD.pfnSetSpecialState(HWD_SET_FOG_DENSITY, cv_grfogdensity.value);
}

void CV_filtermode_ONChange(void)
{
	HWD.pfnSetSpecialState(HWD_SET_TEXTUREFILTERMODE, cv_grfiltermode.value);
}


// ==========================================================================
//                                                               VIEW GLOBALS
// ==========================================================================
// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW ANG90

static angle_t gr_clipangle;

// The viewangletox[viewangle + FINEANGLES/4] lookup
// maps the visible view angles to screen X coordinates,
// flattening the arc to a flat projection plane.
// There will be many angles mapped to the same X.
static int gr_viewangletox[FINEANGLES/2];

// The xtoviewangleangle[] table maps a screen pixel
// to the lowest viewangle that maps back to x ranges
// from clipangle to -clipangle.
static angle_t gr_xtoviewangle[MAXVIDWIDTH+1];

// ==========================================================================
//                                                                    GLOBALS
// ==========================================================================

// uncomment to remove the plane rendering
#define DOPLANES
//#define DOWALLS

// test of drawing sky by polygons like in software with visplane, unfortunately
// this doesn't work since we must have z for pixel and z for texture (not like now with z=oow)
//#define POLYSKY

// test change fov when looking up/down but bsp projection messup :(
//#define NOCRAPPYMLOOK

/// \note crappy
#define drawtextured true

// base values set at SetViewSize
static float gr_basecentery;

float gr_baseviewwindowy, gr_basewindowcentery;
float gr_viewwidth, gr_viewheight; // viewport clipping boundaries (screen coords)
float gr_viewwindowx;

static float gr_centerx, gr_centery;
static float gr_viewwindowy; // top left corner of view window
static float gr_windowcenterx; // center of view window, for projection
static float gr_windowcentery;

static float gr_pspritexscale, gr_pspriteyscale;

static seg_t* gr_curline;
static side_t* gr_sidedef;
static line_t* gr_linedef;
static sector_t* gr_frontsector;
static sector_t* gr_backsector;

// --------------------------------------------------------------------------
//                                              STUFF FOR THE PROJECTION CODE
// --------------------------------------------------------------------------

FTransform atransform;
// duplicates of the main code, set after R_SetupFrame() passed them into sharedstruct,
// copied here for local use
static fixed_t dup_viewx, dup_viewy, dup_viewz;
static angle_t dup_viewangle;

static float gr_viewx, gr_viewy, gr_viewz;
static float gr_viewsin, gr_viewcos;

// Maybe not necessary with the new T&L code (needs to be checked!)
static float gr_viewludsin, gr_viewludcos; // look up down kik test
static float gr_fovlud;

// ==========================================================================
//                                    LIGHT stuffs
// ==========================================================================

static byte lightleveltonumlut[256];

// added to SRB2's sector lightlevel to make things a bit brighter (sprites/walls/planes)
static byte LightLevelToLum(int l)
{
	l = lightleveltonumlut[l];

	if(l > 255)
		l = 255;
	return (byte)l;
}

static inline void InitLumLut(void)
{
	int i, k;
	for(i = 0; i < 256; i++)
	{
		// this polygon is the solution of equ:
		// f(0)=0, f(1)=1 f(.5)=.5, f'(0)=0, f'(1)=0), f'(.5)=K
#define K 2
#define A (-24+16*K)
#define B ( 60-40*K)
#define C (32*K-50)
#define D (-8*K+15)
		float x = (float)i/255;
		float xx, xxx;
		xx = x*x;
		xxx = x*xx;
		k = (int)(255*(A*xx*xxx + B*xx*xx + C*xxx + D*xx));

		lightleveltonumlut[i] = (byte)(min(255, k));
	}
}

// ==========================================================================
//                                   FLOOR/CEILING GENERATION FROM SUBSECTORS
// ==========================================================================

#ifdef DOPLANES

// maximum number of verts around a convex floor/ceiling polygon
// FIXME: gothic2 map02 has a 304 vertex poly!!!!
#define MAXPLANEVERTICES 2048
static FOutVector  planeVerts[MAXPLANEVERTICES];

// -----------------+
// HWR_RenderPlane  : Render a floor or ceiling convex polygon
// -----------------+
static void HWR_RenderPlane(sector_t* sector, extrasubsector_t* xsub, fixed_t fixedheight,
			 FBITFIELD PolyFlags, int lightlevel, int lumpnum, sector_t* FOFsector)
{
	polyvertex_t*   pv;
	float           height; //constant y for all points on the convex flat polygon
	FOutVector      *v3d;
	int             nrPlaneVerts;   //verts original define of convex flat polygon
	int             i;
	float           flatxref,flatyref;
	float flatsize;
	int flatflag;
	int size;

	FSurfaceInfo    Surf;

	// no convex poly were generated for this subsector
	if (!xsub->planepoly)
		return;

	height = FIXED_TO_FLOAT(fixedheight);

	pv  = xsub->planepoly->pts;
	nrPlaneVerts = xsub->planepoly->numpts;

	if (nrPlaneVerts < 3)   //not even a triangle ?
		return;

	if(nrPlaneVerts > MAXPLANEVERTICES) // FIXME: exceeds plVerts size
	{
		CONS_Printf("polygon size of %d exceeds max value of %d vertices\n", nrPlaneVerts, MAXPLANEVERTICES);
		return;
	}

	size = W_LumpLength(lumpnum);

	switch(size)
	{
		case 4194304: // 2048x2048 lump
			flatsize = 2048.0f;
			flatflag = 2047;
			break;
		case 1048576: // 1024x1024 lump
			flatsize = 1024.0f;
			flatflag = 1023;
			break;
		case 262144:// 512x512 lump
			flatsize = 512.0f;
			flatflag = 511;
			break;
		case 65536: // 256x256 lump
			flatsize = 256.0f;
			flatflag = 255;
			break;
		case 16384: // 128x128 lump
			flatsize = 128.0f;
			flatflag = 127;
			break;
		case 1024: // 32x32 lump
			flatsize = 32.0f;
			flatflag = 31;
			break;
		default: // 64x64 lump
			flatsize = 64.0f;
			flatflag = 63;
			break;
	}

	// reference point for flat texture coord for each vertex around the polygon
	flatxref = (float)(((fixed_t)pv->x & (~flatflag)) / flatsize);
	flatyref = (float)(((fixed_t)pv->y & (~flatflag)) / flatsize);

	// transform
	v3d = planeVerts;
	for (i=0; i<nrPlaneVerts; i++,v3d++,pv++)
	{
		// Hurdler: add scrolling texture on floor/ceiling
		float scrollx = 0.0f, scrolly = 0.0f; 

		if(FOFsector != NULL)
		{
			// Negate the following scrollx's and scrolly's Graue 02-01-2004
			if(fixedheight < dup_viewz) // it's a floor
			{
				scrollx = -FIXED_TO_FLOAT(FOFsector->floor_xoffs)/flatsize;
				scrolly = -FIXED_TO_FLOAT(FOFsector->floor_yoffs)/flatsize;
			}
			else // it's a ceiling
			{
				scrollx = -FIXED_TO_FLOAT(FOFsector->ceiling_xoffs)/flatsize;
				scrolly = -FIXED_TO_FLOAT(FOFsector->ceiling_yoffs)/flatsize;
			}
		}
		else if (gr_frontsector)
		{
			if (fixedheight<dup_viewz) // it's a floor
			{
					scrollx = FIXED_TO_FLOAT(gr_frontsector->floor_xoffs)/flatsize;
					scrolly = FIXED_TO_FLOAT(gr_frontsector->floor_yoffs)/flatsize;
			}
			else // it's a floor
			{
					scrollx = FIXED_TO_FLOAT(gr_frontsector->ceiling_xoffs)/flatsize;
					scrolly = FIXED_TO_FLOAT(gr_frontsector->ceiling_yoffs)/flatsize;
			}
		}
		v3d->sow = (float)((pv->x / flatsize) - flatxref + scrollx);
		v3d->tow = (float)(flatyref - (pv->y / flatsize) + scrolly);
		v3d->x = pv->x;
		v3d->y = height;
		v3d->z = pv->y;
		if(sector)
		{
#ifdef SLOPENESS
			int q;
			for(q=0; q<sector->linecount; q++)
			{
				if(v3d->x == sector->lines[q]->v1->x>>FRACBITS)
				{
					if(v3d->z == sector->lines[q]->v1->y>>FRACBITS && sector->special == 16384)
					{
						v3d->y += sector->lines[q]->v1->z>>FRACBITS;
						break;
					}
				}
			}
#endif
		}
	}

	// only useful for flat coloured triangles
	//Surf.FlatColor = 0xff804020;

	// use different light tables
	// for horizontal / vertical / diagonal
	// note: try to get the same visual feel as the original
	Surf.FlatColor.s.red = Surf.FlatColor.s.green = 
	Surf.FlatColor.s.blue = LightLevelToLum(lightlevel); // Don't take from the frontsector

	// colormap test
	if(gr_frontsector)
	{
		sector_t* sector = gr_frontsector;

		if(gr_frontsector->ffloors)
		{
			ffloor_t* caster;

			caster = gr_frontsector->lightlist[R_GetPlaneLight(gr_frontsector, fixedheight, false)].caster;
			sector = caster ? &sectors[caster->secnum] : gr_frontsector;
		}
		if(sector && sector->extra_colormap)
		{
			RGBA_t temp;
			int light, alpha;

			light = LightLevelToLum(lightlevel);
			temp.rgba = sector->extra_colormap->rgba;
			alpha = temp.s.alpha;
			Surf.FlatColor.s.red = (byte)(((26-alpha)*light + alpha*temp.s.red)/26);
			Surf.FlatColor.s.blue = (byte)(((26-alpha)*light + alpha*temp.s.blue)/26);
			Surf.FlatColor.s.green = (byte)(((26-alpha)*light + alpha*temp.s.green)/26);
			Surf.FlatColor.s.alpha = 0xff;
		}
	}

	if(PolyFlags & PF_Translucent)
	{
		Surf.FlatColor.s.alpha = (unsigned char)(PolyFlags>>24);
		HWD.pfnDrawPolygon(&Surf, planeVerts, nrPlaneVerts,
		   PF_Translucent|PF_Modulated|PF_Occlude|PF_Clip);
	}
	else
	{
		Surf.FlatColor.s.alpha = 0xff;
		HWD.pfnDrawPolygon(&Surf, planeVerts, nrPlaneVerts,
				 PolyFlags|PF_Masked|PF_Modulated|PF_Clip);
	}

	// add here code for dynamic lighting on planes
	HWR_PlaneLighting(planeVerts, nrPlaneVerts);
}

#ifdef POLYSKY
// this don't draw anything it only update the z-buffer so there isn't problem with
// wall/things upper that sky (map12)
static void HWR_RenderSkyPlane(extrasubsector_t* xsub, fixed_t fixedheight)
{
	polyvertex_t*   pv;
	float           height; //constant y for all points on the convex flat polygon
	FOutVector      *v3d;
	int             nrPlaneVerts;   //verts original define of convex flat polygon
	int             i;
	float           flatxref,flatyref;

	// no convex poly were generated for this subsector
	if (!xsub->planepoly)
		return;

	height = FIXED_TO_FLOAT(fixedheight);

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
	for (i=0; i<nrPlaneVerts; i++,v3d++,pv++)
	{
		v3d->sow = must be transformed and projected !;
		v3d->tow = must be transformed and projected !;
		v3d->x = pv->x;
		v3d->y = height;
		v3d->z = pv->y;
	}

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
#ifdef WALLSPLATS
static void HWR_DrawSegsSplats( FSurfaceInfo * pSurf )
{
	FOutVector    trVerts[4],*wv;
	wallVert3D    wallVerts[4];
	wallVert3D    *pwallVerts;
	wallsplat_t*  splat;
	GlidePatch_t* gpatch;
	int           i;
	FSurfaceInfo  pSurf2;
	// seg bbox
	fixed_t       segbbox[4];

	M_ClearBox(segbbox);
	M_AddToBox(segbbox,(fixed_t)(((polyvertex_t *)gr_curline->v1)->x*FRACUNIT),(fixed_t)(((polyvertex_t *)gr_curline->v1)->y*FRACUNIT));
	M_AddToBox(segbbox,(fixed_t)(((polyvertex_t *)gr_curline->v2)->x*FRACUNIT),(fixed_t)(((polyvertex_t *)gr_curline->v2)->y*FRACUNIT));

	// splat are drawn by line but this func is called for eatch segs of a line
	/* BP: DONT WORK BECAUSE Z-buffer !!!!
		   FIXME : the splat must be stored by segs !
	if( gr_curline->linedef->splatdrawn == validcount )
		return;
	gr_curline->linedef->splatdrawn = validcount;
	*/

	splat = (wallsplat_t*) gr_curline->linedef->splats;
	for ( ; splat ; splat=splat->next)
	{
		//BP: don't draw splat extern to this seg
		//    this is quick fix best is explain in logboris.txt at 12-4-2000
		if( !M_PointInBox(segbbox,splat->v1.x,splat->v1.y) && !M_PointInBox(segbbox,splat->v2.x,splat->v2.y))
			continue;

		gpatch = W_CachePatchNum (splat->patch, PU_CACHE);
		HWR_GetPatch(gpatch);

		wallVerts[0].x = wallVerts[3].x = FIXED_TO_FLOAT(splat->v1.x);
		wallVerts[0].z = wallVerts[3].z = FIXED_TO_FLOAT(splat->v1.y);
		wallVerts[2].x = wallVerts[1].x = FIXED_TO_FLOAT(splat->v2.x);
		wallVerts[2].z = wallVerts[1].z = FIXED_TO_FLOAT(splat->v2.y);

		i = splat->top;
		if( splat->yoffset )
			i += *splat->yoffset;

		wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(i)+(gpatch->height>>1);
		wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(i)-(gpatch->height>>1);

		wallVerts[3].s = wallVerts[3].t = wallVerts[2].s = wallVerts[0].t = 0.0f;
		wallVerts[1].s = wallVerts[1].t = wallVerts[2].t = wallVerts[0].s = 1.0f;

		// transform
		wv = trVerts;
		pwallVerts = wallVerts;
		for (i=0; i<4; i++,wv++,pwallVerts++)
		{
			wv->x   = pwallVerts->x;
			wv->z = pwallVerts->z;
			wv->y   = pwallVerts->y;

			wv->sow = pwallVerts->s;
			wv->tow = pwallVerts->t;
		}
		memcpy(&pSurf2,pSurf,sizeof(FSurfaceInfo));
		switch (splat->flags & SPLATDRAWMODE_MASK)
		{
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

// ==========================================================================
//                                        WALL GENERATION FROM SUBSECTOR SEGS
// ==========================================================================


int HWR_TranstableToAlpha(int transtablenum, FSurfaceInfo *pSurf)
{
	switch(transtablenum)
	{
		case tr_transmed : pSurf->FlatColor.s.alpha = 0x80;return  PF_Translucent; 
		case tr_transmor : pSurf->FlatColor.s.alpha = 0x40;return  PF_Translucent; 
		case tr_transhi  : pSurf->FlatColor.s.alpha = 0x30;return  PF_Translucent; 
		case tr_transfir : pSurf->FlatColor.s.alpha = 0x80;return  PF_Additive;    
		case tr_transfx1 : pSurf->FlatColor.s.alpha = 0xff;return  PF_Translucent; 
	}
	return PF_Translucent;
}

// v1,v2 : the start & end vertices along the original wall segment, that may have been
//         clipped so that only a visible portion of the wall seg is drawn.
// floorheight, ceilingheight : depend on wall upper/lower/middle, comes from the sectors.

static void HWR_AddTransparentWall(wallVert3D *wallVerts, FSurfaceInfo * pSurf, int texnum, int blend);

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
static void HWR_ProjectWall( wallVert3D   * wallVerts,
                                    FSurfaceInfo * pSurf,
                                    int          blendmode)
{
	FOutVector  trVerts[4];
	int         i;
	FOutVector  *wv;

	// transform
	wv = trVerts;
	// it sounds really stupid to do this conversion with the new T&L code
	// we should directly put the right information in the right structure
	// wallVerts3D seems ok, doesn't need FOutVector
	// also remove the light copy
	for (i=0; i<4; i++, wv++, wallVerts++)
	{
		wv->sow = wallVerts->s;
		wv->tow = wallVerts->t;
		wv->x   = wallVerts->x;
		wv->y   = wallVerts->y;
		wv->z = wallVerts->z;
	}

	HWD.pfnDrawPolygon( pSurf, trVerts, 4, blendmode|PF_Modulated|PF_Occlude|PF_Clip);

	if (gr_curline->linedef->splats && cv_splats.value)
		HWR_DrawSegsSplats( pSurf );

	//Hurdler: TDOD: do static lighting using gr_curline->lm
	HWR_WallLighting(trVerts);

	//Hurdler: for better dynamic light in dark area, we should draw the light first 
	//         and then the wall all that with the right blending func
	//HWD.pfnDrawPolygon( pSurf, trVerts, 4, PF_Additive|PF_Modulated|PF_Occlude|PF_Clip);
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
	v2dx = FIXED_TO_FLOAT(finecosine[clipangle>>ANGLETOFINESHIFT]);
	v2dy = FIXED_TO_FLOAT(finesine[clipangle>>ANGLETOFINESHIFT]);

	den = v2dy*v1dx - v2dx*v1dy;
	if (den == 0)
		return -1;         // parallel

	// calc the frac along the polygon segment,
	//num = (v2x - v1x)*v2dy + (v1y - v2y)*v2dx;
	//num = -v1x * v2dy + v1y * v2dx;
	num = (gr_viewx - v1x)*v2dy + (v1y - gr_viewy)*v2dx;

	return num / den;
}



//
// HWR_SplitWall
// 
static void HWR_SplitWall(sector_t* sector, wallVert3D *wallVerts, int texnum, FSurfaceInfo* Surf, int cutflag)
{
	/* SoM: split up and light walls according to the
	 lightlist. This may also include leaving out parts
	 of the wall that can't be seen */
	GlideTexture_t*  glTex;
	float realtop, realbot, top, bot;
	float pegt, pegb, pegmul;
	float height, bheight = 0;
	int   solid, i;
	lightlist_t*   list = sector->lightlist;
			
	realtop = top = wallVerts[2].y;
	realbot = bot = wallVerts[0].y;
	pegt = wallVerts[2].t;
	pegb = wallVerts[0].t;
	pegmul = (pegb - pegt) / (top - bot);

	for(i = 1; i < sector->numlights; i++)
	{
		if(top < realbot)
			return;

	//Hurdler: fix a crashing bug, but is it correct?
//		if (!list[i].caster)
//			continue;

		if(list[i].caster)
			solid = list[i].caster->flags & cutflag;
		else
			solid = false;

		height = FIXED_TO_FLOAT(list[i].height);
		if(solid)
			bheight = FIXED_TO_FLOAT(*list[i].caster->bottomheight);  

		if(height >= top)
		{
			if(solid && top > bheight)
				top = bheight;
			continue;
		}

		//Found a break;
		bot = height;

		if(bot < realbot)
			bot = realbot;

		{
			FUINT lightnum;
			sector_t* sector;

			lightnum = LightLevelToLum(*list[i-1].lightlevel);
			// store Surface->FlatColor to modulate wall texture
			Surf->FlatColor.s.red = Surf->FlatColor.s.green = Surf->FlatColor.s.blue =
				(unsigned char)lightnum;

			// colormap test
			sector =  list[i-1].caster ? &sectors[list[i-1].caster->secnum] : gr_frontsector;
			if(sector->extra_colormap)
			{
				RGBA_t temp;
				int alpha;

				temp.rgba = sector->extra_colormap->rgba;
				alpha = temp.s.alpha;
				Surf->FlatColor.s.red =
					(unsigned char)(((26 - alpha)*lightnum + alpha*temp.s.red)/26);
				Surf->FlatColor.s.blue =
					(unsigned char)(((26 - alpha)*lightnum + alpha*temp.s.blue)/26);
				Surf->FlatColor.s.green =
					(unsigned char)(((26 - alpha)*lightnum + alpha*temp.s.green)/26);
				Surf->FlatColor.s.alpha = 0xff;
			}
		}

		wallVerts[3].t = wallVerts[2].t = pegt + ((realtop - top) * pegmul);
		wallVerts[0].t = wallVerts[1].t = pegt + ((realtop - bot) * pegmul);

		// set top/bottom coords
		wallVerts[2].y = wallVerts[3].y = top;
		wallVerts[0].y = wallVerts[1].y = bot;

		glTex = HWR_GetTexture(texnum);
		if(glTex->mipmap.flags & TF_TRANSPARENT)
			HWR_AddTransparentWall(wallVerts, Surf, texnum, PF_Environment);
		else
			HWR_ProjectWall(wallVerts, Surf, PF_Masked);

		if(solid)
			top = bheight;
		else
			top = height;
	}

	bot = realbot;
	if(top <= realbot)
		return;

	{
		FUINT   lightnum;
		sector_t    *sector;
	
		lightnum = LightLevelToLum(*list[i-1].lightlevel);
		// store Surface->FlatColor to modulate wall texture
		Surf->FlatColor.s.red = Surf->FlatColor.s.green = Surf->FlatColor.s.blue
			= (unsigned char)lightnum;
	
		sector =  list[i-1].caster ? &sectors[list[i-1].caster->secnum] : gr_frontsector;
		if (sector->extra_colormap)
		{
			RGBA_t  temp;
			int     alpha;
		
			temp.rgba = sector->extra_colormap->rgba;
			alpha = temp.s.alpha;
			Surf->FlatColor.s.red = (unsigned char)(((26 - alpha)*lightnum + alpha*temp.s.red)/26);
			Surf->FlatColor.s.blue = (unsigned char)(((26 - alpha)*lightnum + alpha*temp.s.blue)/26);
			Surf->FlatColor.s.green = (unsigned char)(((26 - alpha)*lightnum + alpha*temp.s.green)/26);
			Surf->FlatColor.s.alpha = 0xff;
		}
	}

	wallVerts[3].t = wallVerts[2].t = pegt + ((realtop - top) * pegmul);
	wallVerts[0].t = wallVerts[1].t = pegt + ((realtop - bot) * pegmul);
			
	// set top/bottom coords
	wallVerts[2].y = wallVerts[3].y = top;
	wallVerts[0].y = wallVerts[1].y = bot;

	glTex = HWR_GetTexture(texnum);
	if (glTex->mipmap.flags & TF_TRANSPARENT)
		HWR_AddTransparentWall( wallVerts, Surf, texnum, PF_Environment );
	else
		HWR_ProjectWall( wallVerts, Surf, PF_Masked );
}


//
// HWR_StoreWallRange
// A portion or all of a wall segment will be drawn, from startfrac to endfrac,
//  where 0 is the start of the segment, 1 the end of the segment
// Anything between means the wall segment has been clipped with solidsegs,
//  reducing wall overdraw to a minimum
//
static void HWR_StoreWallRange (int startfrac, int endfrac)
{
	wallVert3D  wallVerts[4];
	v2d_t       vs, ve;         // start, end vertices of 2d line (view from above)

	fixed_t     worldtop;
	fixed_t     worldbottom;
	fixed_t     worldhigh=0;
	fixed_t     worldlow=0;

	GlideTexture_t* grTex = NULL;
	float       cliplow = 0.0f,cliphigh = 0.0f;
	int         gr_midtexture;
	fixed_t     h, l;          // 3D sides and 2s middle textures

	FUINT   lightnum = 0; // shut up compiler
	FSurfaceInfo Surf;

	if (startfrac>endfrac)
		return;

	gr_sidedef = gr_curline->sidedef;
	gr_linedef = gr_curline->linedef;

	// mark the segment as visible for auto map
//	gr_linedef->flags |= ML_MAPPED;

	worldtop    = gr_frontsector->ceilingheight;
	worldbottom = gr_frontsector->floorheight;

	vs.x = ((polyvertex_t *)gr_curline->v1)->x;
	vs.y = ((polyvertex_t *)gr_curline->v1)->y;
	ve.x = ((polyvertex_t *)gr_curline->v2)->x;
	ve.y = ((polyvertex_t *)gr_curline->v2)->y;

	//
	// clip the wall segment to solidsegs
	//

/*  BP : removed since there is no more clipwalls !
	// clip start of segment
	if (startfrac > 0)
	{
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
	if (endfrac < 1)
	{
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

	if (drawtextured) 
	{
		// x offset the texture
		fixed_t texturehpeg = gr_sidedef->textureoffset + gr_curline->offset;

		// clip texture s start/end coords with solidsegs
		if (startfrac > 0 && startfrac < 1)
			cliplow = texturehpeg + gr_curline->length * startfrac;
		else
			cliplow = (float)texturehpeg;
		
		if (endfrac > 0 && endfrac < 1)
			cliphigh = texturehpeg + gr_curline->length  * endfrac;
		else
			cliphigh = texturehpeg + gr_curline->length;
	}

	//  use different light tables
	//  for horizontal / vertical / diagonal
	//  note: try to get the same visual feel as the original
	Surf.FlatColor.s.alpha = 0xff;

	lightnum = LightLevelToLum(gr_frontsector->lightlevel);

	if(((polyvertex_t*)gr_curline->v1)->y == ((polyvertex_t*)gr_curline->v2)->y
		&& lightnum >= (255/LIGHTLEVELS))
	{
		lightnum -= 255/LIGHTLEVELS;
	}
	else if(((polyvertex_t*)gr_curline->v1)->x == ((polyvertex_t*)gr_curline->v2)->x
		&& lightnum < 255 - (255/LIGHTLEVELS))
	{
		lightnum += 255/LIGHTLEVELS;
	}

	// store Surface->FlatColor to modulate wall texture
	Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue =
		(unsigned char)lightnum;

	if(gr_frontsector)
	{
		sector_t* sector = gr_frontsector;

		// colormap test
		if(sector->ffloors)
		{
			ffloor_t* caster;

			caster = sector->lightlist[R_GetPlaneLight(sector, sector->floorheight, false)].caster;
			sector = caster ? &sectors[caster->secnum] : sector;
		}
		if(sector->extra_colormap)
		{
			RGBA_t temp;
			int alpha;

			temp.rgba = sector->extra_colormap->rgba;
			alpha = temp.s.alpha;
			Surf.FlatColor.s.red =
				(unsigned char)(((26 - alpha)*lightnum + alpha*temp.s.red)/26);
			Surf.FlatColor.s.blue =
				(unsigned char)(((26 - alpha)*lightnum + alpha*temp.s.blue)/26);
			Surf.FlatColor.s.green =
				(unsigned char)(((26 - alpha)*lightnum + alpha*temp.s.green)/26);
			Surf.FlatColor.s.alpha = 0xff;
		}
	}

	if (gr_backsector)
	{
		// two sided line
		worldhigh = gr_backsector->ceilingheight;
		worldlow  = gr_backsector->floorheight;

		// hack to allow height changes in outdoor areas
		if (gr_frontsector->ceilingpic == skyflatnum &&
			gr_backsector->ceilingpic  == skyflatnum)
		{
			worldtop = worldhigh;
		}

		// check TOP TEXTURE
		if (worldhigh < worldtop && texturetranslation[gr_sidedef->toptexture])
		{
			if (drawtextured)
			{
				fixed_t     texturevpegtop;     //top

				grTex = HWR_GetTexture ( texturetranslation[gr_sidedef->toptexture] );
				
				// PEGGING
				if (gr_linedef->flags & ML_DONTPEGTOP)
					texturevpegtop = 0;
				else
					texturevpegtop = worldhigh + textureheight[gr_sidedef->toptexture] - worldtop;
				
				texturevpegtop += gr_sidedef->rowoffset;

				
				wallVerts[3].t = wallVerts[2].t = texturevpegtop * grTex->scaleY;
				wallVerts[0].t = wallVerts[1].t = (texturevpegtop + worldtop - worldhigh) * grTex->scaleY;
				wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
				wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
			}
			
			// set top/bottom coords
			wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(worldtop);
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldhigh);

			if(gr_frontsector->numlights)
				HWR_SplitWall(gr_frontsector, wallVerts, texturetranslation[gr_sidedef->toptexture], &Surf, FF_CUTSOLIDS);
			else if (grTex->mipmap.flags & TF_TRANSPARENT)
				HWR_AddTransparentWall( wallVerts, &Surf, texturetranslation[gr_sidedef->toptexture], PF_Environment );
			else
				HWR_ProjectWall(wallVerts, &Surf, PF_Masked );
		}

		// check BOTTOM TEXTURE
		if (worldlow > worldbottom && texturetranslation[gr_sidedef->bottomtexture])     //only if VISIBLE!!!
		{
			if (drawtextured)
			{
				fixed_t     texturevpegbottom=0;  //bottom

				grTex = HWR_GetTexture ( texturetranslation[gr_sidedef->bottomtexture] );
				
				// PEGGING
				if (gr_linedef->flags & ML_DONTPEGBOTTOM )
					texturevpegbottom = worldtop - worldlow;
				else
					texturevpegbottom = 0;
				
				texturevpegbottom += gr_sidedef->rowoffset;

				
				wallVerts[3].t = wallVerts[2].t = texturevpegbottom * grTex->scaleY;
				wallVerts[0].t = wallVerts[1].t = (texturevpegbottom + worldlow - worldbottom) * grTex->scaleY;
				wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
				wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
			}
			
			// set top/bottom coords
			wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(worldlow);
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldbottom);
			
			if(gr_frontsector->numlights)
				HWR_SplitWall(gr_frontsector, wallVerts, texturetranslation[gr_sidedef->bottomtexture], &Surf, FF_CUTSOLIDS);
			else if (grTex->mipmap.flags & TF_TRANSPARENT)
				HWR_AddTransparentWall( wallVerts, &Surf, texturetranslation[gr_sidedef->bottomtexture], PF_Environment );
			else
				HWR_ProjectWall(wallVerts, &Surf, PF_Masked );
		}
		gr_midtexture = texturetranslation[gr_sidedef->midtexture];
		if (gr_midtexture)
		{
			int blendmode;
			fixed_t  opentop, openbottom, polytop, polybottom;

			// SoM: a little note: This code re-arranging will
			// fix the bug in Nimrod map02. opentop and openbottom
			// record the limits the texture can be displayed in.
			// polytop and polybottom, are the ideal (i.e. unclipped)
			// heights of the polygon, and h & l, are the final (clipped)
			// poly coords. 

			opentop = worldtop < worldhigh ? worldtop : worldhigh;
			openbottom = worldbottom > worldlow ? worldbottom : worldlow;

			if (gr_linedef->flags & ML_DONTPEGBOTTOM)
			{
				polybottom = openbottom + gr_sidedef->rowoffset;
				polytop = polybottom + textureheight[gr_midtexture];
			}
			else
			{
				polytop = opentop + gr_sidedef->rowoffset;
				polybottom = polytop - textureheight[gr_midtexture];
			}
			if ( (gr_frontsector->ceilingheight == gr_backsector->ceilingheight)
				 /*|| (gr_linedef->flags & ML_DONTDRAW)*/ )
				h = polytop;
			else
				h = polytop < opentop ? polytop : opentop;
			  
			if ( (gr_frontsector->floorheight == gr_backsector->floorheight)
			 /*    || (gr_linedef->flags & ML_DONTDRAW)*/ )
				l = polybottom;
			else
				l = polybottom > openbottom ? polybottom : openbottom;

			if (drawtextured) 
			{
				fixed_t     texturevpeg;
				// PEGGING
				if (gr_linedef->flags & ML_DONTPEGBOTTOM) 
					texturevpeg = l + textureheight[gr_sidedef->midtexture] - h + polybottom - l;
				else
					texturevpeg = polytop - h;

				grTex = HWR_GetTexture (gr_midtexture );

				wallVerts[3].t = wallVerts[2].t = texturevpeg * grTex->scaleY;
				wallVerts[0].t = wallVerts[1].t = (h - l + texturevpeg) * grTex->scaleY;
				wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
				wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
			}
			// set top/bottom coords
			wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(h);
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(l);
	
			// set alpha for transparent walls (new boom and legacy linedef types)
			// ooops ! this do not work at all because render order we should render it in backtofront order
			switch ( gr_linedef->special )
			{
				case 289:
				case 284: blendmode = HWR_TranstableToAlpha(tr_transmed, &Surf);break;
				case 285: blendmode = HWR_TranstableToAlpha(tr_transmor, &Surf);break;
				case 286: blendmode = HWR_TranstableToAlpha(tr_transhi , &Surf);break;
				case 287: blendmode = HWR_TranstableToAlpha(tr_transfir, &Surf);break;
				case 288: //FIXME: not work like this must be laoded with firetranslucent to true !
				          blendmode = HWR_TranstableToAlpha(tr_transfx1, &Surf);break;
				case 283: blendmode = PF_Substractive;break;
				default: blendmode = PF_Masked;break;
			}
			if (grTex->mipmap.flags & TF_TRANSPARENT)
				blendmode = PF_Environment;

			if (blendmode != PF_Masked)
				HWR_AddTransparentWall(wallVerts, &Surf, gr_midtexture, blendmode);
			else
				HWR_ProjectWall( wallVerts, &Surf, blendmode);
		}
	}
	else
	{
		// Single sided line... Deal only with the middletexture (if one exists)
		gr_midtexture = texturetranslation[gr_sidedef->midtexture];
		if (gr_midtexture)
		{
			if (drawtextured) 
			{
				fixed_t     texturevpeg;
				// PEGGING
				if (gr_linedef->flags & ML_DONTPEGBOTTOM) 
					texturevpeg = worldbottom + textureheight[gr_sidedef->midtexture] - worldtop + gr_sidedef->rowoffset;
				else
					// top of texture at top
					texturevpeg = gr_sidedef->rowoffset;

				grTex = HWR_GetTexture (gr_midtexture );

				wallVerts[3].t = wallVerts[2].t = texturevpeg * grTex->scaleY;
				wallVerts[0].t = wallVerts[1].t = (texturevpeg + worldtop - worldbottom) * grTex->scaleY;
				wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
				wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
			}
			// set top/bottom coords
			wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(worldtop);
			wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(worldbottom);

			// I don't think that solid walls can use translucent linedef types...
			if(gr_frontsector->numlights)
				HWR_SplitWall(gr_frontsector, wallVerts, gr_midtexture, &Surf, FF_CUTSOLIDS);
			else
			{
				if (grTex->mipmap.flags & TF_TRANSPARENT)
					HWR_AddTransparentWall(wallVerts, &Surf, gr_midtexture, PF_Environment);
				else
					HWR_ProjectWall(wallVerts, &Surf, PF_Masked );
			}
		}
	}


	//Hurdler: 3d-floors test
#ifdef R_FAKEFLOORS
	if(gr_frontsector && gr_backsector && gr_frontsector->tag != gr_backsector->tag && (gr_backsector->ffloors || gr_frontsector->ffloors))
	{
		ffloor_t*  rover;
		fixed_t    highcut = 0, lowcut = 0;

		highcut = gr_frontsector->ceilingheight < gr_backsector->ceilingheight ? gr_frontsector->ceilingheight : gr_backsector->ceilingheight;
		lowcut = gr_frontsector->floorheight > gr_backsector->floorheight ? gr_frontsector->floorheight : gr_backsector->floorheight;

		if(gr_backsector->ffloors)
		{
			for(rover = gr_backsector->ffloors; rover; rover = rover->next)
			{
				if(!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERSIDES) || (rover->flags & FF_INVERTSIDES))
					continue;
				if(*rover->topheight < lowcut || *rover->bottomheight > highcut)
					continue;


				h = *rover->topheight;
				l = *rover->bottomheight;
				if(h > highcut)
					h = highcut;
				if(l < lowcut)
					l = lowcut;
				//Hurdler: HW code starts here
				//FIXME: check if peging is correct
				// set top/bottom coords
				wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(h);
				wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(l);

				if (drawtextured)
				{
					grTex = HWR_GetTexture ( texturetranslation[sides[rover->master->sidenum[0]].midtexture] );
				
					wallVerts[3].t = wallVerts[2].t = (*rover->topheight - h) * grTex->scaleY;
					wallVerts[0].t = wallVerts[1].t = (h - l + (*rover->topheight - h)) * grTex->scaleY;
					wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
					wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
				}
				if (!(rover->flags & FF_FOG))
				{
					int blendmode = PF_Masked;

					if (rover->flags & FF_TRANSLUCENT)
					{
						blendmode = PF_Translucent;
						if (cv_grtranswall.value)
							Surf.FlatColor.s.alpha = (byte)rover->alpha;
					}
					else if (grTex->mipmap.flags & TF_TRANSPARENT)
					{
						blendmode = PF_Environment;
					}

					if(gr_frontsector->numlights)
						HWR_SplitWall(gr_frontsector, wallVerts, texturetranslation[sides[rover->master->sidenum[0]].midtexture], &Surf, rover->flags & FF_EXTRA ? FF_CUTEXTRA : FF_CUTSOLIDS);
					else
					{
						if (blendmode != PF_Masked)
							HWR_AddTransparentWall(wallVerts, &Surf, texturetranslation[sides[rover->master->sidenum[0]].midtexture], blendmode);
						else
							HWR_ProjectWall(wallVerts, &Surf, PF_Masked );
					}
				}
			}
		}
		else if(gr_frontsector->ffloors)
		{
			for(rover = gr_frontsector->ffloors; rover; rover = rover->next)
			{
				if(!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERSIDES) || !(rover->flags & FF_ALLSIDES))
					continue;
				if(*rover->topheight < lowcut || *rover->bottomheight > highcut)
					continue;

				h = *rover->topheight;
				l = *rover->bottomheight;
				if(h > highcut)
					h = highcut;
				if(l < lowcut)
					l = lowcut;
				//Hurdler: HW code starts here
				//FIXME: check if peging is correct
				// set top/bottom coords
				wallVerts[2].y = wallVerts[3].y = FIXED_TO_FLOAT(h);
				wallVerts[0].y = wallVerts[1].y = FIXED_TO_FLOAT(l);

				if (drawtextured)
				{
					grTex = HWR_GetTexture ( texturetranslation[sides[rover->master->sidenum[0]].midtexture] );

					wallVerts[3].t = wallVerts[2].t = (*rover->topheight - h) * grTex->scaleY;
					wallVerts[0].t = wallVerts[1].t = (h - l + (*rover->topheight - h)) * grTex->scaleY;
					wallVerts[0].s = wallVerts[3].s = cliplow * grTex->scaleX;
					wallVerts[2].s = wallVerts[1].s = cliphigh * grTex->scaleX;
				}
				if (!(rover->flags & FF_FOG))
				{
					int blendmode = PF_Masked;

					if (rover->flags & FF_TRANSLUCENT)
					{
						blendmode = PF_Translucent;
						if (cv_grtranswall.value)
							Surf.FlatColor.s.alpha = (byte)rover->alpha;
					}
					else if (grTex->mipmap.flags & TF_TRANSPARENT)
					{
						blendmode = PF_Environment;
					}

					if(gr_backsector->numlights)
						HWR_SplitWall(gr_backsector, wallVerts, texturetranslation[sides[rover->master->sidenum[0]].midtexture], &Surf, rover->flags & FF_EXTRA ? FF_CUTEXTRA : FF_CUTSOLIDS);
					else
					{
						if (blendmode != PF_Masked)
							HWR_AddTransparentWall(wallVerts, &Surf, texturetranslation[sides[rover->master->sidenum[0]].midtexture], blendmode);
						else
							HWR_ProjectWall( wallVerts, &Surf, PF_Masked );
					}
				}
			}
		}
	}
#endif
//Hurdler: end of 3d-floors test
}

//Hurdler: just like in r_bsp.c
#if 0
#define MAXSEGS         MAXVIDWIDTH/2+1
#else
//Alam_GBC: Or not
#define MAXSEGS         128
#endif

// hw_newend is one past the last valid seg
static cliprange_t*    hw_newend;
static cliprange_t     gr_solidsegs[MAXSEGS];


static void printsolidsegs( void)
{
	cliprange_t*        start;
	if(!hw_newend || cv_grbeta.value!=2)
		return;
	for(start=gr_solidsegs;start!=hw_newend;start++)
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
			HWR_StoreWallRange(first, last );

			next = hw_newend;
			hw_newend++;

			while (next != start)
			{
				*next = *(next-1 );
				next--;
			}
			next->first = first;
			next->last = last;
			printsolidsegs();
			return;
		}

		// There is a fragment above *start.
		if (!cv_grclipwalls.value)
		{
			if (!poorhack) HWR_StoreWallRange(first, last);
			poorhack=true;
		}
		else
		{
			highfrac = HWR_ClipViewSegment(start->first+1, (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
			HWR_StoreWallRange (0, (int)highfrac );
		}
		// Now adjust the clip size.
		start->first = first;
	}

	// Bottom contained in start?
	if (last <= start->last)
	{
		printsolidsegs();
		return;
	}
	next = start;
	while (last >= (next+1)->first-1)
	{
		// There is a fragment between two posts.
		if (!cv_grclipwalls.value)
		{
			if (!poorhack) HWR_StoreWallRange(first,last );
			poorhack=true;
		}
		else
		{
			lowfrac  = HWR_ClipViewSegment(next->last-1 , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
			highfrac = HWR_ClipViewSegment((next+1)->first+1 , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
			HWR_StoreWallRange((int)lowfrac, (int)highfrac );
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
		if (!cv_grclipwalls.value)
		{
			if (!poorhack) HWR_StoreWallRange(first,last );
			poorhack=true;
		}
		else
			HWR_StoreWallRange (0, 1 );
	}
	else
	{
	// There is a fragment after *next.
		if (!cv_grclipwalls.value)
		{
			if (!poorhack) HWR_StoreWallRange(first,last );
			poorhack=true;
		}
		else
		{
			lowfrac  = HWR_ClipViewSegment( next->last-1 , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
			HWR_StoreWallRange ((int)lowfrac, 1 );
		}
	}

	// Adjust the clip size.
	start->last = last;

	// Remove start+1 to next from the clip list,
	// because start now covers their area.
crunch:
	if (next == start)
	{
		printsolidsegs();
		// Post just extended past the bottom of one post.
		return;
	}


	while (next++ != hw_newend)
	{
		// Remove a post.
		*++start = *next;
	}

	hw_newend = start;
	printsolidsegs();
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
		if (!cv_grclipwalls.value)
		{	//20/08/99: Changed by Hurdler (taken from faB's code)
			if (!poorhack) HWR_StoreWallRange (0,1 );
			poorhack=true;
		}
		else
		{
			highfrac  = HWR_ClipViewSegment ( min(start->first+1,start->last) , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
			HWR_StoreWallRange (0, (int)highfrac );
		}
	}

	// Bottom contained in start?
	if (last <= start->last)
		return;

	while (last >= (start+1)->first-1)
	{
		// There is a fragment between two posts.
		if (!cv_grclipwalls.value)
		{
			if (!poorhack) HWR_StoreWallRange (0,1 );
			poorhack=true;
		}
		else
		{
			lowfrac  = HWR_ClipViewSegment (max(start->last-1,start->first) , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
			highfrac = HWR_ClipViewSegment (min((start+1)->first+1,(start+1)->last) , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
			HWR_StoreWallRange ((int)lowfrac, (int)highfrac );
		}
		start++;

		if (last <= start->last)
			return;
	}

	if(first==start->first+1) // 1 line texture
	{
		if (!cv_grclipwalls.value)
		{
			if (!poorhack) HWR_StoreWallRange (0,1 );
			poorhack=true;
		}
		else
			 HWR_StoreWallRange (0, 1 );
	}
	else
	{
		// There is a fragment after *next.
		if(!cv_grclipwalls.value)
		{
			if (!poorhack) HWR_StoreWallRange (0,1 );
			poorhack=true;
		}
		else
		{
			lowfrac  = HWR_ClipViewSegment ( max(start->last-1,start->first) , (polyvertex_t *)gr_curline->v1, (polyvertex_t *)gr_curline->v2 );
			HWR_StoreWallRange ((int)lowfrac, 1 );
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
	gr_solidsegs[1].first = vid.width;    //viewwidth;
	gr_solidsegs[1].last = 0x7fffffff;
	hw_newend = gr_solidsegs+2;
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

	// SoM: Backsector needs to be run through R_FakeFlat
	sector_t            tempsec; 

	gr_curline = line;

	// OPTIMIZE: quickly reject orthogonal back sides.
	angle1 = R_PointToAngle ((fixed_t)(((polyvertex_t *)line->v1)->x*FRACUNIT), (fixed_t)(((polyvertex_t *)line->v1)->y*FRACUNIT));
	angle2 = R_PointToAngle ((fixed_t)(((polyvertex_t *)line->v2)->x*FRACUNIT), (fixed_t)(((polyvertex_t *)line->v2)->y*FRACUNIT));

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

		angle2 = (angle_t)-(signed)gr_clipangle;
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
			fx2 = vid.width;
		else
			fx2 = gr_windowcenterx + (fx2 * gr_viewsin - fy2 * gr_viewcos) * gr_centerx / fy1;
	
		x1 = fx1+0.5f;
		x2 = fx2+0.5f;
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

	gr_backsector = R_FakeFlat(gr_backsector, &tempsec, NULL, NULL, true);

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
	    && gr_curline->sidedef->midtexture == 0
	    && !gr_backsector->ffloors && !gr_frontsector->ffloors)
		// SoM: For 3D sides... Boris, would you like to take a 
		// crack at rendering 3D sides? You would need to add the
		// above check and add code to HWR_StoreWallRange...
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

		angle2 = (angle_t)-(signed)gr_clipangle;
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
	int                     locFloorHeight, locCeilingHeight;
	int                     light;
	fixed_t                 wh;

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

	//SoM: 4/7/2000: Test to make Boom water work in Hardware mode.
	gr_frontsector = R_FakeFlat(gr_frontsector, &tempsec, &floorlightlevel,
								&ceilinglightlevel, false);
	//FIXME: Use floorlightlevel and ceilinglightlevel insted of lightlevel.

	// ------------------------------------------------------------------------
	// sector lighting, DISABLED because it's done in HWR_StoreWallRange
	// ------------------------------------------------------------------------
	/// \todo store a RGBA instead of just intensity, allow coloured sector lighting
	//light = (FUBYTE)(sub->sector->lightlevel & 0xFF) / 255.0f;
	//gr_cursectorlight.red   = light;
	//gr_cursectorlight.green = light;
	//gr_cursectorlight.blue  = light;
	//gr_cursectorlight.alpha = light;

// ----- for special tricks with HW renderer -----
	if(gr_frontsector->pseudoSector)
	{
		locFloorHeight   = gr_frontsector->virtualFloorheight;
		locCeilingHeight = gr_frontsector->virtualCeilingheight;
	}
	else if(gr_frontsector->virtualFloor)
	{
		locFloorHeight   = gr_frontsector->virtualFloorheight;
		if(gr_frontsector->virtualCeiling)
			locCeilingHeight = gr_frontsector->virtualCeilingheight;
		else
			locCeilingHeight = gr_frontsector->ceilingheight;
	}
	else if(gr_frontsector->virtualCeiling)
	{
		locCeilingHeight = gr_frontsector->virtualCeilingheight;
		locFloorHeight   = gr_frontsector->floorheight;
	}
	else
	{
		locFloorHeight   = gr_frontsector->floorheight;
		locCeilingHeight = gr_frontsector->ceilingheight;       
	}
// ----- end special tricks -----

	if(gr_frontsector->ffloors)
	{
		if(gr_frontsector->moved)
		{
			gr_frontsector->numlights = sub->sector->numlights = 0;
			R_Prep3DFloors(gr_frontsector);
			sub->sector->lightlist = gr_frontsector->lightlist;
			sub->sector->numlights = gr_frontsector->numlights;
			sub->sector->moved = gr_frontsector->moved = false;
		}

		floorlightlevel = *gr_frontsector->lightlist[R_GetPlaneLight(gr_frontsector, locFloorHeight, false)].lightlevel;
		ceilinglightlevel = *gr_frontsector->lightlist[R_GetPlaneLight(gr_frontsector, locCeilingHeight, false)].lightlevel;
	}

	// render floor ?
#ifdef DOPLANES
	// yeah, easy backface cull! :)
	if (locFloorHeight < dup_viewz)
	{
		if(gr_frontsector->floorpic != skyflatnum)
		{
			if (sub->validcount != validcount)
			{
				HWR_GetFlat ( levelflats[gr_frontsector->floorpic].lumpnum  );
				HWR_RenderPlane( gr_frontsector, &extrasubsectors[num], locFloorHeight, PF_Occlude, floorlightlevel, levelflats[gr_frontsector->floorpic].lumpnum, NULL);
			}
		}
		else
		{
#ifdef POLYSKY
			HWR_RenderSkyPlane (&extrasubsectors[num], locFloorHeight);
#endif
			cv_grsky.value = true;
		}
	}

	if (locCeilingHeight > dup_viewz)
	{
		if(gr_frontsector->ceilingpic != skyflatnum)
		{
			if (sub->validcount != validcount)
			{
				HWR_GetFlat ( levelflats[gr_frontsector->ceilingpic].lumpnum );
				HWR_RenderPlane ( NULL, &extrasubsectors[num], locCeilingHeight, PF_Occlude, ceilinglightlevel, levelflats[gr_frontsector->ceilingpic].lumpnum,NULL);
			}
		}
		else
		{
#ifdef POLYSKY
			HWR_RenderSkyPlane (&extrasubsectors[num], locCeilingHeight);
#endif
			cv_grsky.value = true;
		}
	}

#ifdef R_FAKEFLOORS
	if( gr_frontsector->ffloors )
	{
		/// \todo fix light, xoffs, yoffs, extracolormap ?
		ffloor_t*  rover;

		R_Prep3DFloors(gr_frontsector);
		for(rover = gr_frontsector->ffloors; 
		 rover; 
		 rover = rover->next) 
		{

			if(!(rover->flags & FF_EXISTS) || !(rover->flags & FF_RENDERPLANES))
				continue;
			if (sub->validcount == validcount)
				continue;

			if(*rover->bottomheight <= gr_frontsector->ceilingheight &&
			   *rover->bottomheight >= gr_frontsector->floorheight &&
			   ((dup_viewz < *rover->bottomheight && !(rover->flags & FF_INVERTPLANES)) ||
			   (dup_viewz > *rover->bottomheight && (rover->flags & FF_BOTHPLANES))))
			{
				if (rover->flags & (FF_TRANSLUCENT|FF_FOG)) // SoM: Flags are more efficient
				{
					light = R_GetPlaneLight(gr_frontsector, *rover->bottomheight, dup_viewz < *rover->bottomheight ? true : false);
					HWR_Add3DWater(levelflats[*rover->bottompic].lumpnum,
					               &extrasubsectors[num], 
					               *rover->bottomheight, 
					               *gr_frontsector->lightlist[light].lightlevel,
					               rover->alpha);
				}
				else
				{
					HWR_GetFlat ( levelflats[*rover->bottompic].lumpnum );
					light = R_GetPlaneLight(gr_frontsector, *rover->bottomheight, dup_viewz < *rover->bottomheight ? true : false);
					HWR_RenderPlane ( NULL, &extrasubsectors[num], *rover->bottomheight, PF_Occlude, *gr_frontsector->lightlist[light].lightlevel, levelflats[*rover->bottompic].lumpnum,
					                  rover->master->frontsector);
				}
			}
			if(*rover->topheight >= gr_frontsector->floorheight &&
			 *rover->topheight <= gr_frontsector->ceilingheight &&
			  ((dup_viewz > *rover->topheight && !(rover->flags & FF_INVERTPLANES)) ||
			   (dup_viewz < *rover->topheight && (rover->flags & FF_BOTHPLANES))))
			{
				if (rover->flags & (FF_TRANSLUCENT|FF_FOG))
				{
					light = R_GetPlaneLight(gr_frontsector, *rover->topheight, dup_viewz < *rover->topheight ? true : false);
					HWR_Add3DWater(levelflats[*rover->toppic].lumpnum,
					                          &extrasubsectors[num], 
					                          *rover->topheight, 
					                          *gr_frontsector->lightlist[light].lightlevel,
					                          rover->alpha);
				}
				else
				{
					HWR_GetFlat ( levelflats[*rover->toppic].lumpnum );
					light = R_GetPlaneLight(gr_frontsector, *rover->topheight, dup_viewz < *rover->topheight ? true : false);
					HWR_RenderPlane ( NULL, &extrasubsectors[num], *rover->topheight, PF_Occlude, *gr_frontsector->lightlist[light].lightlevel, levelflats[*rover->toppic].lumpnum,
					                  rover->master->frontsector);
				}
			}
		}
	}
#endif
#endif //doplanes

// Hurder ici se passe les choses int�essantes!
// on vient de tracer le sol et le plafond
// on trace �pr�ent d'abord les sprites et ensuite les murs
// hurdler: faux: on ajoute seulement les sprites, le murs sont trac� d'abord
	if (line)
	{
		// draw sprites first , coz they are clipped to the solidsegs of
		// subsectors more 'in front'
		HWR_AddSprites (gr_frontsector);

		//Hurdler: at this point validcount must be the same, but is not because
		//         gr_frontsector doesn't point anymore to sub->sector due to
		//         the call gr_frontsector = R_FakeFlat(...)
		//         if it's not done, the sprite is drawn more than once,
		//         what looks really bad with translucency or dynamic light,
		//         without talking about the overdraw of course.
		sub->sector->validcount = validcount;/// \todo fix that in a better way

		while (count--)
		{
				HWR_AddLine (line);
				line++;
		}
	}

//20/08/99: Changed by Hurdler (taken from faB's code)
#ifdef DOPLANES
	// -------------------- WATER IN DEV. TEST ------------------------
	//dck hack : use abs(tag) for waterheight
	if (gr_frontsector->tag<0)
	{
		wh = ((-gr_frontsector->tag) <<16) + (FRACUNIT/2);
		if (wh > gr_frontsector->floorheight &&
			wh < gr_frontsector->ceilingheight )
		{
			HWR_GetFlat ( doomwaterflat );
			HWR_RenderPlane(  gr_frontsector, &extrasubsectors[num], wh, PF_Translucent, gr_frontsector->lightlevel, doomwaterflat, NULL);
		}
	}
	// -------------------- WATER IN DEV. TEST ------------------------
#endif
	sub->validcount = validcount;
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
fixed_t *hwbbox;

static void HWR_RenderBSPNode (int bspnum)
{
	node_t*     bsp;
	int         side;

	// Found a subsector?
	if (bspnum & NF_SUBSECTOR)
	{
		if (bspnum == -1)
		{
			//*(gr_drawsubsector_p++) = 0;
			HWR_Subsector (0);
		}
		else
		{
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
	hwbbox=bsp->bbox[side];

	// Recursively divide front space.
	HWR_RenderBSPNode (bsp->children[side]);

	// Possibly divide back space.
	if (HWR_CheckBBox (bsp->bbox[side^1]))
	{
		// BP: big hack for a test in lighning ref:1249753487AB
		hwbbox=bsp->bbox[side^1];
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
static angle_t fineanglefov = FIELDOFVIEW;
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
	grviewwidth = vid.width;
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

static gr_vissprite_t     gr_vissprites[MAXVISSPRITES];
static gr_vissprite_t*    gr_vissprite_p;

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
static gr_vissprite_t  gr_overflowsprite;

static gr_vissprite_t* HWR_NewVisSprite (void)
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
	GlidePatch_t        *gpatch;      //sprite patch converted to hardware
	FSurfaceInfo        Surf;

	if(!spr->mobj)
		return;

	if(!spr->mobj->subsector)
		return;

	// cache sprite graphics
	//12/12/99: Hurdler:
	//          OK, I don't change anything for MD2 support because I want to be
	//          sure to do it the right way. So actually, we keep normal sprite
	//          in memory and we add the md2 model if it exists for that sprite

	// convert srpite differently when fxtranslucent is detected
	if( (spr->mobj->frame & FF_TRANSMASK) == tr_transfx1<<FF_TRANSSHIFT)
	{
		firetranslucent = true;
		gpatch = W_CachePatchNum (spr->patchlumpnum, PU_CACHE );
		firetranslucent = false;
	}
	else
		gpatch = W_CachePatchNum (spr->patchlumpnum, PU_CACHE );    

	HWR_DL_AddLight( spr, gpatch );

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
	wallVerts[0].z = wallVerts[1].z = wallVerts[2].z = wallVerts[3].z = spr->tz;

	// transform
	wv = wallVerts;

	for (i=0; i<4; i++,wv++)
	{
		//look up/down ----TOTAL SUCKS!!!--- do the 2 in one!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		tr_x = wv->z;
		tr_y = wv->y;
		wv->y = (tr_x * gr_viewludcos) + (tr_y * gr_viewludsin);
		wv->z = (tr_x * gr_viewludsin) - (tr_y * gr_viewludcos);
		// ---------------------- mega lame test ----------------------------------

		//scale y before frustum so that frustum can be scaled to screen height
		if(spr->mobj && spr->mobj->flags & MF_HIRES)
		{
			wv->y *= ORIGINAL_ASPECT * gr_fovlud/2;
			wv->x *= gr_fovlud/2;
		}
		else
		{
			wv->y *= ORIGINAL_ASPECT * gr_fovlud;
			wv->x *= gr_fovlud;
		}
	}

	if (spr->flip)
	{
		wallVerts[0].sow = wallVerts[3].sow = gpatch->max_s;
		wallVerts[2].sow = wallVerts[1].sow = 0;
	}else{
		wallVerts[0].sow = wallVerts[3].sow = 0;
		wallVerts[2].sow = wallVerts[1].sow = gpatch->max_s;
	}
	wallVerts[3].tow = wallVerts[2].tow = 0;
	wallVerts[0].tow = wallVerts[1].tow = gpatch->max_t;

	// cache the patch in the graphics card memory
	//12/12/99: Hurdler: same comment as above (for md2)
	//Hurdler: 25/04/2000: now support colormap in hardware mode
	HWR_GetMappedPatch(gpatch, spr->colormap);

	// sprite lighting by modulating the RGB components
	/// \todo coloured
	Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = spr->sectorlight;

	// colormap test
	{
		sector_t* sector = spr->mobj->subsector->sector;

		if(sector->ffloors)
		{
			ffloor_t* caster;

			caster = sector->lightlist[R_GetPlaneLight(sector, spr->mobj->z, false)].caster;
			sector = caster ? &sectors[caster->secnum] : sector;
		}
		if(sector->extra_colormap)
		{
			RGBA_t temp;
			int alpha;

			temp.rgba = sector->extra_colormap->rgba;
			alpha = temp.s.alpha;
			Surf.FlatColor.s.red = (byte)(((26-alpha)*spr->sectorlight + alpha*temp.s.red)/26);
			Surf.FlatColor.s.blue = (byte)(((26-alpha)*spr->sectorlight + alpha*temp.s.blue)/26);
			Surf.FlatColor.s.green = (byte)(((26-alpha)*spr->sectorlight + alpha*temp.s.green)/26);
			Surf.FlatColor.s.alpha = 0xff;
		}
	}

	/// \todo do the test earlier
	if(!cv_grmd2.value || (md2_models[spr->mobj->sprite].scale < 0.0f))
	{
		int blend = 0;
		if(spr->mobj->frame & FF_TRANSMASK)
			blend = HWR_TranstableToAlpha((spr->mobj->frame & FF_TRANSMASK)>>FF_TRANSSHIFT, &Surf);
		else if(spr->mobj->frame & FF_SMOKESHADE)
		{
			Surf.FlatColor.s.alpha = 0x80;
			blend = PF_Translucent;
		}
		else if(spr->mobj->flags2 & MF2_SHADOW)
		{
			Surf.FlatColor.s.alpha = 0x40;
			blend = PF_Translucent;
		}
		else
		{
			// BP: i agree that is little better in environement but it don't
			//     work properly under glide nor with fogcolor to ffffff :(
			// Hurdler: PF_Environement would be cool, but we need to fix
			//          the issue with the fog before
			Surf.FlatColor.s.alpha = 0xFF;
			blend = PF_Translucent|PF_Occlude;
		}

		HWD.pfnDrawPolygon(&Surf, wallVerts, 4, blend|PF_Modulated|PF_Clip);
	}

	// draw a corona if this sprite contain light(s)
#ifndef NEWCORONAS
	HWR_DoCoronasLighting(wallVerts, spr);
#endif
}

// Sprite drawer for precipitation
static void HWR_DrawPrecipitationSprite(gr_vissprite_t* spr)
{
	int                 i;
	int blend=0;
	float               tr_x;
	float               tr_y;
	FOutVector          wallVerts[4];
	FOutVector          *wv;
	GlidePatch_t        *gpatch;      //sprite patch converted to hardware
	FSurfaceInfo        Surf;

	// cache sprite graphics
	gpatch = W_CachePatchNum (spr->patchlumpnum, PU_CACHE );    

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
	wallVerts[0].z = wallVerts[1].z = wallVerts[2].z = wallVerts[3].z = spr->tz;

	// transform
	wv = wallVerts;

	for (i=0; i<4; i++,wv++)
	{
		//look up/down ----TOTAL SUCKS!!!--- do the 2 in one!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		tr_x = wv->z;
		tr_y = wv->y;
		wv->y = (tr_x * gr_viewludcos) + (tr_y * gr_viewludsin);
		wv->z = (tr_x * gr_viewludsin) - (tr_y * gr_viewludcos);
		// ---------------------- mega lame test ----------------------------------

		//scale y before frustum so that frustum can be scaled to screen height
		wv->y *= ORIGINAL_ASPECT * gr_fovlud;
		wv->x *= gr_fovlud;
	}

	if (spr->flip)
	{
		wallVerts[0].sow = wallVerts[3].sow = gpatch->max_s;
		wallVerts[2].sow = wallVerts[1].sow = 0;
	}
	else
	{
		wallVerts[0].sow = wallVerts[3].sow = 0;
		wallVerts[2].sow = wallVerts[1].sow = gpatch->max_s;
	}
	wallVerts[3].tow = wallVerts[2].tow = 0;
	wallVerts[0].tow = wallVerts[1].tow = gpatch->max_t;

	// cache the patch in the graphics card memory
	//12/12/99: Hurdler: same comment as above (for md2)
	//Hurdler: 25/04/2000: now support colormap in hardware mode
	HWR_GetMappedPatch(gpatch, spr->colormap);

	// sprite lighting by modulating the RGB components
	/// \todo coloured
	Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = spr->sectorlight;

	blend = HWR_TranstableToAlpha((spr->mobj->frame & FF_TRANSMASK)>>FF_TRANSSHIFT, &Surf);

	HWD.pfnDrawPolygon( &Surf, wallVerts, 4, blend|PF_Modulated|PF_Clip );
}

// --------------------------------------------------------------------------
// Sort vissprites by distance
// --------------------------------------------------------------------------
static gr_vissprite_t     gr_vsprsortedhead;

static void HWR_SortVisSprites (void)
{
	size_t              i, count;
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
	if (gr_vissprite_p > gr_vissprites)
	{
		gr_vissprite_t* spr;

		// draw all vissprites back to front
		for (spr = gr_vsprsortedhead.next ;
		 spr != &gr_vsprsortedhead ;
		  spr = spr->next)
		{
			if(spr->precip == true)
				HWR_DrawPrecipitationSprite(spr);
			else
				HWR_DrawSprite( spr );
		}
	}
}

// --------------------------------------------------------------------------
//  Draw all MD2
// --------------------------------------------------------------------------
static void HWR_DrawMD2S(void)
{
	if (gr_vissprite_p > gr_vissprites)
	{
		gr_vissprite_t* spr;

		// draw all MD2 back to front
		for (spr = gr_vsprsortedhead.next ;
		 spr != &gr_vsprsortedhead ;
		  spr = spr->next)
		{
			HWR_DrawMD2( spr );
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
	precipmobj_t*       precipthing;
	fixed_t             adx,ady,approx_dist;

	// BSP is traversed by subsector.
	// A sector might have been split into several
	//  subsectors during BSP building.
	// Thus we check whether its already added.
	if (sec->validcount == validcount)
		return;

	// Well, now it will be done.
	sec->validcount = validcount;

	// sprite lighting
	sectorlight = LightLevelToLum(sec->lightlevel & 0xff);

	// NiGHTS stages have a draw distance limit because of the
	// HUGE number of SPRiTES!
	if(maptol & TOL_NIGHTS)
	{
		// Special function for precipitation Tails 08-18-2002
		for (thing = sec->thinglist ; thing ; thing = thing->snext)
		{
			if(!thing)
				continue;

			if((thing->flags2 & MF2_DONTDRAW)==0)
			{
				adx = abs(players[displayplayer].mo->x - thing->x);
				ady = abs(players[displayplayer].mo->y - thing->y);

				// From _GG1_ p.428. Approx. eucledian distance fast.
				approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

				if(approx_dist < NIGHTS_DRAW_DIST)
					HWR_ProjectSprite (thing);
				else if(cv_splitscreen.value && players[secondarydisplayplayer].mo)
				{
					adx = abs(players[secondarydisplayplayer].mo->x - thing->x);
					ady = abs(players[secondarydisplayplayer].mo->y - thing->y);

					// From _GG1_ p.428. Approx. eucledian distance fast.
					approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

					if(approx_dist < NIGHTS_DRAW_DIST)
						HWR_ProjectSprite (thing);
				}
			}
		}
	}
	else
	{
		// Handle all things in sector.
		for (thing = sec->thinglist ; thing ; thing = thing->snext)
		{
			if(!thing)
				continue;

			if((thing->flags2 & MF2_DONTDRAW)==0)
				HWR_ProjectSprite (thing);

			if(!thing->snext)
				break;
		}
	}

	for (precipthing = sec->preciplist ; precipthing ; precipthing = precipthing->snext)
	{
		if(!precipthing)
			continue;

		adx = abs(players[displayplayer].mo->x - precipthing->x);
		ady = abs(players[displayplayer].mo->y - precipthing->y);

		// From _GG1_ p.428. Approx. eucledian distance fast.
		approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

		// Only draw the precipitation oh-so-far from the player.
		if(approx_dist < (cv_precipdist.value << FRACBITS))
			HWR_ProjectPrecipitationSprite (precipthing);
		else if(cv_splitscreen.value && players[secondarydisplayplayer].mo)
		{
			adx = abs(players[secondarydisplayplayer].mo->x - precipthing->x);
			ady = abs(players[secondarydisplayplayer].mo->y - precipthing->y);

			// From _GG1_ p.428. Approx. eucledian distance fast.
			approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);

			if(approx_dist < (cv_precipdist.value << FRACBITS))
				HWR_ProjectPrecipitationSprite (precipthing);
		}
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
	float               tz;

	float               x1;
	float               x2;

	spritedef_t*        sprdef;
	spriteframe_t*      sprframe;
	int                 lump;
	unsigned            rot;
	boolean             flip;
	angle_t             ang;

	if(!thing)
		return;

	// transform the origin point
	tr_x = FIXED_TO_FLOAT(thing->x) - gr_viewx;
	tr_y = FIXED_TO_FLOAT(thing->y) - gr_viewy;

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
		I_Error ("HWR_ProjectSprite: invalid sprite frame %i : %i for %s",
		         thing->sprite, thing->frame, sprnames[thing->sprite] );
#endif
	sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

	if (sprframe->rotate)
	{
		// choose a different rotation based on player view
		ang = R_PointToAngle (thing->x, thing->y );          // uses viewx,viewy
		rot = (ang-thing->angle+(unsigned)(ANG45/2)*9)>>29;
		//Fab: lumpid is the index for spritewidth,spriteoffset... tables
		lump = sprframe->lumpid[rot];
		flip = (boolean)sprframe->flip[rot];
	}
	else
	{
		// use single rotation for all views
		rot = 0;                        //Fab: for vis->patch below
		lump = sprframe->lumpid[0];     //Fab: see note above
		flip = (boolean)sprframe->flip[0];
	}


	// calculate edges of the shape
	tx -= FIXED_TO_FLOAT(spriteoffset[lump]);

	// project x
	x1 = gr_windowcenterx + (tx * gr_centerx / tz );

	// BP: FOV des sprites, c'est ici que sa ce passe
	// left edge off the right side?
#ifdef NOCRAPPYMLOOK
	if (x1 > gr_viewwidth)
#else
	if ((x1 > gr_viewwidth) && !cv_grcrappymlook.value)
	//if ((x1 > gr_viewwidth) && (cv_grfov.value<=90) /*&& !cv_grmd2.value*/)
#endif
		return;

 	//faB:tr_x doesnt matter
	// hurdler: it's used in cliptosolidsegs
	tr_x = x1;

	x1 = tx;

	tx += FIXED_TO_FLOAT(spritewidth[lump]);
	x2 = gr_windowcenterx + (tx * gr_centerx / tz );

	// BP: FOV des sprites, ici aussi
	// right edge off the left side
#ifdef NOCRAPPYMLOOK
	if (x2 < 0)
#else
	if ((x2 < 0) && !cv_grcrappymlook.value)
	//if ((x2 < 0) && (cv_grfov.value<=90) /*&& !cv_grmd2.value*/)
#endif
		return;

	// sprite completely hidden ?
#ifdef NOCRAPPYMLOOK
	if (!HWR_ClipToSolidSegs((int)tr_x,(int)x2))
#else
	if ((!HWR_ClipToSolidSegs((int)tr_x,(int)x2)) && !cv_grcrappymlook.value)
	//if ((!HWR_ClipToSolidSegs((int)tr_x,(int)x2)) && (cv_grfov.value<=90) /*&& !cv_grmd2.value*/)
#endif
		return;

	//
	// store information in a vissprite
	//
	vis = HWR_NewVisSprite ();
	vis->x1 = x1;
	vis->x2 = tx;
	vis->tz = tz;
	vis->patchlumpnum = sprframe->lumppat[rot];
	vis->flip = flip;
	vis->mobj = thing;

	//Hurdler: 25/04/2000: now support colormap in hardware mode
	if (thing->flags & MF_TRANSLATION)
	{
		// New colormap stuff for skins Tails 06-07-2002
		if(vis->mobj->player) // This thing is a player!
		{
			vis->colormap = (byte *) translationtables[vis->mobj->player->skin] - 256 + ( (thing->flags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) );
		}
		else if((vis->mobj->flags & MF_BOSS) && (vis->mobj->flags2 & MF2_FRET) && (leveltime & 1)) // Bosses "flash"
		{
			vis->colormap = (byte *) bosstranslationtables;
		}
		else
		{
			vis->colormap = (byte *) defaulttranslationtables - 256 + ( (thing->flags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) );
		}
	}
	else
		vis->colormap = colormaps;

	// set top/bottom coords
	vis->ty = FIXED_TO_FLOAT(thing->z + spritetopoffset[lump]) - gr_viewz;

	//CONS_Printf("------------------\nH: sprite  : %d\nH: frame   : %x\nH: type    : %d\nH: sname   : %s\n\n",
	//            thing->sprite, thing->frame, thing->type, sprnames[thing->sprite]);

	if(thing->state && (thing->state->frame & FF_FULLBRIGHT) )
		/// \todo disable also the fog
		vis->sectorlight = 0xff;
	else 
		vis->sectorlight = sectorlight;

	vis->precip = false;
}

// Precipitation projector for hardware mode Tails 08-25-2002
static void HWR_ProjectPrecipitationSprite (precipmobj_t* thing)
{
	gr_vissprite_t*     vis;

	float               tr_x;
	float               tr_y;

	float               tx;
	float               tz;

	float               x1;
	float               x2;

	spritedef_t*        sprdef;
	spriteframe_t*      sprframe;
	int                 lump;
	unsigned            rot;
	boolean             flip;

	// transform the origin point
	tr_x = FIXED_TO_FLOAT(thing->x) - gr_viewx;
	tr_y = FIXED_TO_FLOAT(thing->y) - gr_viewy;

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

	sprdef = &sprites[thing->sprite];

#ifdef RANGECHECK
	if ( (thing->frame&FF_FRAMEMASK) >= sprdef->numframes )
		I_Error ("HWR_ProjectSprite: invalid sprite frame %i : %i for %s",
				 thing->sprite, thing->frame, sprnames[thing->sprite] );
#endif
	sprframe = &sprdef->spriteframes[ thing->frame & FF_FRAMEMASK];

	// use single rotation for all views
	rot = 0;                        //Fab: for vis->patch below
	lump = sprframe->lumpid[0];     //Fab: see note above
	flip = (boolean)sprframe->flip[0];

	// calculate edges of the shape
	tx -= FIXED_TO_FLOAT(spriteoffset[lump]);

	// project x
	x1 = gr_windowcenterx + (tx * gr_centerx / tz );

	// BP: FOV des sprites, c'est ici que sa ce passe
	// left edge off the right side?
#ifdef NOCRAPPYMLOOK
	if (x1 > gr_viewwidth)
#else
	if ((x1 > gr_viewwidth) && !cv_grcrappymlook.value)
	//if ((x1 > gr_viewwidth) && (cv_grfov.value<=90))
#endif
		return;

	//faB:tr_x doesnt matter
	// hurdler: it's used in cliptosolidsegs
	tr_x = x1;

	x1 = tx;

	tx += FIXED_TO_FLOAT(spritewidth[lump]);
	x2 = gr_windowcenterx + (tx * gr_centerx / tz );

	// BP: FOV des sprites, ici aussi
	// right edge off the left side
#ifdef NOCRAPPYMLOOK
	if (x2 < 0)
#else
	if ((x2 < 0) && !cv_grcrappymlook.value)
	//if ((x2 < 0) && (cv_grfov.value<=90))
#endif
		return;

	// sprite completely hidden ?
#ifdef NOCRAPPYMLOOK
	if (!HWR_ClipToSolidSegs((int)tr_x,(int)x2))
#else
	if ((!HWR_ClipToSolidSegs((int)tr_x,(int)x2)) && !cv_grcrappymlook.value)
	//if ((!HWR_ClipToSolidSegs((int)tr_x,(int)x2)) && (cv_grfov.value<=90))
#endif
		return;

	//
	// store information in a vissprite
	//
	vis = HWR_NewVisSprite ();
	vis->x1 = x1;
	vis->x2 = tx;
	vis->tz = tz;
	vis->patchlumpnum = sprframe->lumppat[rot];
	vis->flip = flip;
	vis->mobj = (mobj_t*)thing;

	vis->colormap = colormaps;

	// set top/bottom coords
	vis->ty = FIXED_TO_FLOAT(thing->z + spritetopoffset[lump]) - gr_viewz;

	vis->sectorlight = 0xff;
	vis->precip = true;
}

// ==========================================================================
//
// ==========================================================================
static inline void HWR_DrawSkyBackground( player_t* player )
{
	FOutVector      v[4];
	angle_t         angle;
	float f;

//  3--2
//  | /|
//  |/ |
//  0--1

	 player = NULL;
	HWR_GetTexture( skytexture );

	//Hurdler: the sky is the only texture who need 4.0f instead of 1.0
	//         because it's called just after clearing the screen
	//         and thus, the near clipping plane is set to 3.99
	v[0].x = v[3].x = -4.0f;
	v[1].x = v[2].x =  4.0f;
	v[0].y = v[1].y = -4.0f;
	v[2].y = v[3].y =  4.0f;

	v[0].z = v[1].z = v[2].z = v[3].z = 4.0f;

#define WRAPANGLE (ANGLE_MAX/4)
	angle = ((dup_viewangle + gr_xtoviewangle[0])%WRAPANGLE);

	v[0].sow = v[3].sow = (float)(1.0f+((float)angle)/(WRAPANGLE-1));
	v[2].sow = v[1].sow = ((float)angle)/(WRAPANGLE-1);

	f = (float)(40 + (textures[skytexture]->height/2)
	    * FIXED_TO_FLOAT(finetangent[(2048
	     - ((int)aimingangle>>(ANGLETOFINESHIFT + 1))) & FINEMASK]));
	if(f<0) f=0;
	if(f>240-127) f=240-127;

	v[3].tow = v[2].tow = f/(textures[skytexture]->height/2);
	v[0].tow = v[1].tow = (f+127)/(textures[skytexture]->height/2);    //suppose 256x128 sky...

	HWD.pfnDrawPolygon( NULL, v, 4, 0 );
}


// -----------------+
// HWR_ClearView : clear the viewwindow, with maximum z value
// -----------------+
static inline void HWR_ClearView( void )
{
	//  3--2
	//  | /|
	//  |/ |
	//  0--1

	/// \bug faB - enable depth mask, disable color mask

	HWD.pfnGClipRect((int)gr_viewwindowx,
					 (int)gr_viewwindowy,
					 (int)(gr_viewwindowx + gr_viewwidth),
					 (int)(gr_viewwindowy + gr_viewheight),
					 3.99f );
	HWD.pfnClearBuffer( false, true, 0 );

	//disable clip window - set to full size
	// rem by Hurdler
	// HWD.pfnGClipRect (0,0,vid.width,vid.height );
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
	//if ( blocks > 10 || (blocks==10)) // Tails
	if (blocks > 10 || (blocks == 10 && cv_grtranslucenthud.value < 255))
	{
		gr_viewwidth = (float)vid.width;
		gr_viewheight = (float)vid.height;
	}
	else
	{
		gr_viewwidth = (float) ((blocks*vid.width/10) & ~7 );
		gr_viewheight = (float) ((blocks*(vid.height)/10) & ~1 );
	}

	if( cv_splitscreen.value )
		 gr_viewheight /= 2;

	gr_centerx = gr_viewwidth / 2;
	gr_basecentery = gr_viewheight / 2; //note: this is (gr_centerx * gr_viewheight / gr_viewwidth)

	gr_viewwindowx = (vid.width - gr_viewwidth) / 2;
	gr_windowcenterx = (float)(vid.width / 2 );
	if (gr_viewwidth == vid.width) 
	{
		gr_baseviewwindowy = 0;
		gr_basewindowcentery = gr_viewheight / 2;               // window top left corner at 0,0
	}
	else
	{
		gr_baseviewwindowy = (vid.height-gr_viewheight) / 2;
		gr_basewindowcentery = (float)(vid.height / 2 );
	}

	gr_pspritexscale = gr_viewwidth / BASEVIDWIDTH;
	gr_pspriteyscale = ((vid.height*gr_pspritexscale*BASEVIDWIDTH)/BASEVIDHEIGHT)/vid.width;
}

//Hurdler: 3D water sutffs
static int numplanes = 0;
static int numwalls = 0;

// ==========================================================================
//
// ==========================================================================
void HWR_RenderPlayerView(int viewnumber, player_t* player)
{
	{
		// do we really need to save player (is it not the same)?
		player_t* saved_player = stplyr;
		stplyr = player;
		ST_doPaletteStuff();
		stplyr = saved_player;
		HWR_SetLights(viewnumber);
	}

	// note: sets viewangle, viewx, viewy, viewz
	R_SetupFrame(player);

	// copy view cam position for local use
	dup_viewx = viewx;
	dup_viewy = viewy;
	dup_viewz = viewz;
	dup_viewangle = viewangle;

	// set window position
	gr_centery = gr_basecentery;
	gr_viewwindowy = gr_baseviewwindowy;
	gr_windowcentery = gr_basewindowcentery;
	if(cv_splitscreen.value && viewnumber == 1)
	{
		gr_viewwindowy += (vid.height/2 );
		gr_windowcentery += (vid.height/2 );
	}

	// check for new console commands.
	NetUpdate();

	gr_viewx = FIXED_TO_FLOAT(dup_viewx);
	gr_viewy = FIXED_TO_FLOAT(dup_viewy);
	gr_viewz = FIXED_TO_FLOAT(dup_viewz);
	gr_viewsin = FIXED_TO_FLOAT(viewsin);
	gr_viewcos = FIXED_TO_FLOAT(viewcos);

	gr_viewludsin = FIXED_TO_FLOAT(finecosine[aimingangle>>ANGLETOFINESHIFT]);
	gr_viewludcos = FIXED_TO_FLOAT(-finesine[aimingangle>>ANGLETOFINESHIFT]);

	//04/01/2000: Hurdler: added for T&L
	//                     It should replace all other gr_viewxxx when finished
	atransform.anglex = (float)(aimingangle>>ANGLETOFINESHIFT)*(360.0f/(float)FINEANGLES);
	atransform.angley = (float)(viewangle>>ANGLETOFINESHIFT)*(360.0f/(float)FINEANGLES);
	atransform.x      = gr_viewx;  // FIXED_TO_FLOAT(viewx)
	atransform.y      = gr_viewy;  // FIXED_TO_FLOAT(viewy)
	atransform.z      = gr_viewz;  // FIXED_TO_FLOAT(viewz)
	atransform.scalex = 1;
	atransform.scaley = ORIGINAL_ASPECT;
	atransform.scalez = 1;
	atransform.fovxangle = FIXED_TO_FLOAT(cv_grfov.value)+grfovadjust; // Tails
	atransform.fovyangle = FIXED_TO_FLOAT(cv_grfov.value)+grfovadjust; // Tails
	atransform.splitscreen = cv_splitscreen.value;
	gr_fovlud = (float)(1/tan(((cv_grfov.value>>FRACBITS) + grfovadjust)*PI/360));

#ifdef NOCRAPPYMLOOK
	// enlage fOV when looking up/down
	HWR_InitTextureMapping();
#endif

	//------------------------------------------------------------------------
	HWR_ClearView();

	if (cv_grfog.value)
		HWR_FoggingOn();
	
	if (cv_grsky.value)
		HWR_DrawSkyBackground(player);

	//Hurdler: it doesn't work in splitscreen mode
	//Ccv_grsky.value = false;
	cv_grsky.value = cv_splitscreen.value;

	// added by Hurdler for FOV 120
//	if (cv_grfov.value != 90)
//		HWD.pfnSetSpecialState(HWD_SET_FOV, cv_grfov.value);


	//14/11/99: Hurdler: we will add lights while processing sprites
	//it doesn't work with all subsectors (if we use AddSprites to do that).
	//TOO bad, that's why I removed this line (until this is fixed).
//	HWR_ResetLights();

	HWR_ClearSprites();

	HWR_ClearClipSegs();

	//04/01/2000: Hurdler: added for T&L
	//                     Actually it only works on Walls and Planes
	HWD.pfnSetTransform(&atransform);

	validcount++;
	HWR_RenderBSPNode(numnodes-1);

#ifndef NOCRAPPYMLOOK
	if (cv_grcrappymlook.value && (aimingangle || cv_grfov.value+grfovadjust>90*FRACUNIT))
	{
		dup_viewangle += ANG90;
		HWR_ClearClipSegs();
		HWR_RenderBSPNode(numnodes-1); //lefT

		dup_viewangle += ANG90;
		if (cv_grcrappymlook.value == 2 && ((int)aimingangle>ANG45 || (int)aimingangle<-ANG45))
		{
			HWR_ClearClipSegs();
			HWR_RenderBSPNode(numnodes-1); //back
		}

		dup_viewangle += ANG90;
		HWR_ClearClipSegs();
		HWR_RenderBSPNode(numnodes-1); //right

		dup_viewangle += ANG90;
	}
#endif
	if (numwalls && !cv_grtranswall.value) //Hurdler: render transparent walls after everything
	{
		HWR_RenderTransparentWalls();
	}

	// Check for new console commands.
	NetUpdate();

	//14/11/99: Hurdler: moved here because it doesn't work with
	// subsector, see other comments;
	HWR_ResetLights();

	// Draw MD2 and sprites
	HWR_SortVisSprites();
	HWR_DrawMD2S();
	// Draw the sprites like it was done previously without T&L
	HWD.pfnSetTransform(NULL);
	HWR_DrawSprites ();

#ifdef NEWCORONAS
	//Hurdler: they must be drawn before translucent planes, what about gl fog?
	HWR_DrawCoronas();
#endif

	if (numplanes || numwalls) //Hurdler: render 3D water and transparent walls after everything
	{
		HWD.pfnSetTransform(&atransform);
		if (numplanes)
			HWR_Render3DWater();
		if (numwalls && cv_grtranswall.value)
			HWR_RenderTransparentWalls();
		HWD.pfnSetTransform(NULL);
	}

	// Check for new console commands.
	NetUpdate();

	//------------------------------------------------------------------------
	// put it off for menus etc
	if (cv_grfog.value)
		HWD.pfnSetSpecialState( HWD_SET_FOG_MODE, 0 );

	// added by Hurdler for correct splitscreen
	// moved here by hurdler so it works with the new near clipping plane
	HWD.pfnGClipRect (0,0,vid.width,vid.height, 0.9f );
}


// ==========================================================================
//                                                                        FOG
// ==========================================================================


/// \author faB

static unsigned int atohex(const char* s)
{
	int     iCol;
	const char*   sCol;
	char    cCol;
	int i;

	if (strlen(s)<6)
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


static void HWR_FoggingOn (void)
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
	if((netgame || multiplayer) && !cv_debug && cv_grfov.value != 90*FRACUNIT)
		CV_Set(&cv_grfov, cv_grfov.defaultvalue);

	// autoset mlook when FOV > 90
	if ((!cv_grcrappymlook.value) && (cv_grfov.value > 90*FRACUNIT))
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

	CONS_Printf("Patch info headers : %7d kb\n", Z_TagUsage(PU_HWRPATCHINFO)>>10);
	CONS_Printf("3D Texture cache   : %7d kb\n", Z_TagUsage(PU_HWRCACHE)>>10);
	CONS_Printf("Plane polygon      : %7d kb\n", Z_TagUsage(PU_HWRPLANE)>>10);
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
	//CV_RegisterVar (&cv_grpolygonsmooth); // moved below
	CV_RegisterVar (&cv_grmd2);
	CV_RegisterVar (&cv_grtranswall);
	CV_RegisterVar (&cv_grrounddown);
	CV_RegisterVar (&cv_grfov);
	CV_RegisterVar (&cv_grfogdensity);
	CV_RegisterVar (&cv_grfiltermode);
	CV_RegisterVar (&cv_grcorrecttricks);
	CV_RegisterVar (&cv_grsolvetjoin);
}

static inline void HWR_AddEngineCommands (void)
{
	CV_RegisterVar (&cv_grpolygonsmooth);

	// engine state variables
	//CV_RegisterVar (&cv_grsky);
	//CV_RegisterVar (&cv_grzbuffer);
	//CV_RegisterVar (&cv_grclipwalls);
	//CV_RegisterVar (&cv_voodoocompatibility);

	// engine development mode variables
	// - usage may vary from version to version..
	CV_RegisterVar (&cv_gralpha);
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
	static int startupdone = 0;

	// setup GlidePatch_t scaling
	gr_patch_scalex = (float)(1.0f / vid.width);
	gr_patch_scaley = (float)(1.0f / vid.height);

	// initalze light lut translation 
	InitLumLut();

	// do this once
	if (!startupdone)
	{
		CONS_Printf ("HWR_Startup()\n");
		HWR_InitPolyPool ();
		// add console cmds & vars
		HWR_AddEngineCommands ();
		HWR_InitTextureCache ();

		// for test water translucent surface
		doomwaterflat  = W_CheckNumForName ("FWATER1");
		if (doomwaterflat == -1) // if FWATER1 not found (in doom shareware)
			doomwaterflat = W_GetNumForName ("WATER0");

		HWR_InitMD2();
	}

	HWR_InitLight();

	if( rendermode == render_opengl )
		textureformat = patchformat = GR_RGBA;

	startupdone = 1;
}


// --------------------------------------------------------------------------
// Free resources allocated by the hardware renderer
// --------------------------------------------------------------------------
void HWR_Shutdown (void)
{
	CONS_Printf ("HWR_Shutdown()\n");
	HWR_FreeExtraSubsectors ();
	HWR_FreePolyPool ();
	HWR_FreeTextureCache ();
}

void transform(float *cx, float *cy, float *cz)
{
	float tr_x,tr_y;
	// translation
	tr_x = *cx - gr_viewx;
	tr_y = *cz - gr_viewy;
//	*cy = *cy;
	
	// rotation around vertical y axis
	*cx = (tr_x * gr_viewsin) - (tr_y * gr_viewcos );
	tr_x = (tr_x * gr_viewcos) + (tr_y * gr_viewsin );

	//look up/down ----TOTAL SUCKS!!!--- do the 2 in one!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	tr_y = *cy - gr_viewz;
	
	*cy = (tr_x * gr_viewludcos) + (tr_y * gr_viewludsin );
	*cz = (tr_x * gr_viewludsin) - (tr_y * gr_viewludcos );
	
	//scale y before frustum so that frustum can be scaled to screen height
	*cy *= ORIGINAL_ASPECT * gr_fovlud;
	*cx *= gr_fovlud;
}


//Hurdler: 3D Water stuff
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MAX_3DWATER 512
static planeinfo_t *planeinfo = NULL;

static void HWR_Add3DWater( int               lumpnum,
                            extrasubsector_t *xsub,
                            fixed_t           fixedheight,
                            int               lightlevel,
                            int               alpha)
{
	if ( !(numplanes % MAX_3DWATER) )
	{
		planeinfo = (planeinfo_t *) realloc(planeinfo, (numplanes+MAX_3DWATER)*sizeof(planeinfo_t));
	}
	planeinfo[numplanes].fixedheight = fixedheight;
	planeinfo[numplanes].lightlevel = lightlevel;
	planeinfo[numplanes].lumpnum = lumpnum;
	planeinfo[numplanes].xsub = xsub;
	planeinfo[numplanes].alpha = alpha;
	numplanes++;
}

#define DIST_PLANE(i) ABS(planeinfo[(i)].fixedheight-dup_viewz)

//FIXME: this doesn't work yet
static void HWR_QuickSortPlane(int low, int high)
{
	if (low < high)
	{
		int lo = low;
		int hi = high+1;
		int pivot;
		pivot = DIST_PLANE(low);

		for ( ; ; )
		{
			while (DIST_PLANE(++lo) < pivot);
			while (DIST_PLANE(--hi) > pivot);
			if (lo < hi)
			{ //swap (ia, low, hi);
				planeinfo_t temp;
				memcpy(&temp, &planeinfo[low], sizeof(planeinfo_t));
				memcpy(&planeinfo[low], &planeinfo[hi], sizeof(planeinfo_t));
				memcpy(&planeinfo[hi], &temp, sizeof(planeinfo_t));
			}
			else
				break;
		}
		{ //swap (ia, low, hi);
			planeinfo_t temp;
			memcpy(&temp, &planeinfo[low], sizeof(planeinfo_t));
			memcpy(&planeinfo[low], &planeinfo[hi], sizeof(planeinfo_t));
			memcpy(&planeinfo[hi], &temp, sizeof(planeinfo_t));
		}
		HWR_QuickSortPlane(low, hi-1);
		HWR_QuickSortPlane(hi+1, high);
	} 
}


static void HWR_Render3DWater(void)
{
	int i;

	//bubble sort 3D Water for correct alpha blending
	//FIXME: do a quick sort since there can be lots of plane to sort
	{
		int permut = 1;
		while (permut)
		{
			int j;
			for (j=0, permut=0; j<numplanes-1; j++)
			{
				if (ABS(planeinfo[j].fixedheight-dup_viewz) < ABS(planeinfo[j+1].fixedheight-dup_viewz))
				{
					planeinfo_t temp;
					memcpy(&temp, &planeinfo[j+1], sizeof(planeinfo_t));
					memcpy(&planeinfo[j+1], &planeinfo[j], sizeof(planeinfo_t));
					memcpy(&planeinfo[j], &temp, sizeof(planeinfo_t));
					permut = 1;
				}
			}
		}
	}
	//HWR_QuickSortPlane(0, numplanes-1);

	gr_frontsector = NULL; //Hurdler: gr_fronsector is no longer valid
	for (i=0; i<numplanes; i++)
	{
		FBITFIELD PolyFlags = PF_Translucent | (planeinfo[i].alpha<<24);

		HWR_GetFlat ( planeinfo[i].lumpnum );
		HWR_RenderPlane ( NULL, planeinfo[i].xsub, planeinfo[i].fixedheight, PolyFlags, planeinfo[i].lightlevel, planeinfo[i].lumpnum,
			NULL);
	}
	numplanes = 0;
}

//Hurdler: manage transparent texture a little better
#define ABS(x) ((x) < 0 ? -(x) : (x))
#define MAX_TRANSPARENTWALL 256

typedef struct
{ 
	wallVert3D    wallVerts[4];
	FSurfaceInfo  Surf;
	int           texnum;
	int           blend;
} wallinfo_t;

static wallinfo_t *wallinfo = NULL;

static void HWR_AddTransparentWall( wallVert3D *wallVerts, FSurfaceInfo *pSurf, int texnum, int blend )
{
	if ( !(numwalls % MAX_TRANSPARENTWALL) )
	{
		wallinfo = (wallinfo_t *) realloc(wallinfo, (numwalls+MAX_TRANSPARENTWALL)*sizeof(wallinfo_t));
	}
	memcpy(wallinfo[numwalls].wallVerts, wallVerts, sizeof(wallinfo[numwalls].wallVerts));
	memcpy(&wallinfo[numwalls].Surf, pSurf, sizeof(FSurfaceInfo));
	wallinfo[numwalls].texnum = texnum;
	wallinfo[numwalls].blend = blend;
	numwalls++;
}

static void HWR_RenderWall( wallVert3D   *wallVerts, FSurfaceInfo *pSurf, int blend );

static void HWR_RenderTransparentWalls(void)
{
	int i;

	/*{ // sorting is disbale for now, do it!
		int permut = 1;
		while (permut)
		{
			int j;
			for (j=0, permut=0; j<numwalls-1; j++)
			{
				if (ABS(wallinfo[j].fixedheight-dup_viewz) < ABS(wallinfo[j+1].fixedheight-dup_viewz))
				{
					wallinfo_t temp;
					memcpy(&temp, &wallinfo[j+1], sizeof(wallinfo_t));
					memcpy(&wallinfo[j+1], &wallinfo[j], sizeof(wallinfo_t));
					memcpy(&wallinfo[j], &temp, sizeof(wallinfo_t));
					permut = 1;
				}
			}
		}
	}*/

	for (i=0; i<numwalls; i++)
	{
		HWR_GetTexture(wallinfo[i].texnum);
		HWR_RenderWall(wallinfo[i].wallVerts, &wallinfo[i].Surf, wallinfo[i].blend);
	}
	numwalls = 0;
}

static void HWR_RenderWall( wallVert3D   *wallVerts, FSurfaceInfo *pSurf, int blend )
{
	FOutVector  trVerts[4];
	int         i;
	FOutVector  *wv;

	// transform
	wv = trVerts;
	// it sounds really stupid to do this conversion with the new T&L code
	// we should directly put the right information in the right structure
	// wallVerts3D seems ok, doesn't need FOutVector
	// also remove the light copy
	for (i=0; i<4; i++, wv++, wallVerts++)
	{
		wv->sow = wallVerts->s;
		wv->tow = wallVerts->t;
		wv->x   = wallVerts->x;
		wv->y   = wallVerts->y;
		wv->z = wallVerts->z;
	}

	HWD.pfnDrawPolygon( pSurf, trVerts, 4, blend|PF_Modulated|PF_Occlude|PF_Clip);

	if (gr_curline->linedef->splats && cv_splats.value)
		HWR_DrawSegsSplats( pSurf );

	//Hurdler: TDOD: do static lighting using gr_curline->lm
	HWR_WallLighting(trVerts);
}

void HWR_SetPaletteColor(int palcolor)
{
	HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, palcolor);
}

int HWR_GetTextureUsed(void)
{
	return HWD.pfnGetTextureUsed();
}
