// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_light.c,v 1.20 2000/08/10 19:58:04 bpereira Exp $
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
// $Log: hw_light.c,v $
// Revision 1.20  2000/08/10 19:58:04  bpereira
// no message
//
// Revision 1.19  2000/08/10 14:16:25  hurdler
// no message
//
// Revision 1.18  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.17  2000/07/01 09:23:50  bpereira
// no message
//
// Revision 1.16  2000/05/09 21:09:18  hurdler
// people prefer coronas on plasma riffles
//
// Revision 1.15  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.14  2000/04/24 15:46:34  hurdler
// Support colormap for text
//
// Revision 1.13  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.12  2000/04/18 12:52:21  hurdler
// join with Boris' code
//
// Revision 1.10  2000/04/14 16:34:26  hurdler
// some nice changes for coronas
//
// Revision 1.9  2000/04/12 16:03:51  hurdler
// ready for T&L code and true static lighting
//
// Revision 1.8  2000/04/11 01:00:59  hurdler
// Better coronas support
//
// Revision 1.7  2000/04/09 17:18:01  hurdler
// modified coronas' code for 16 bits video mode
//
// Revision 1.6  2000/04/06 20:50:23  hurdler
// add Boris' changes for coronas in doom3.wad
//
// Revision 1.5  2000/03/29 19:39:49  bpereira
// no message
//
// Revision 1.4  2000/03/07 03:31:45  hurdler
// fix linux compilation
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
//      Dynamic/Static lighting & coronas add on by Hurdler
//      !!! Under construction !!!
//
//-----------------------------------------------------------------------------


#include "hw_light.h"
#include "../i_video.h"
#include "../z_zone.h"
#include "../m_random.h"
#include "../m_bbox.h"
#include "../w_wad.h"
#ifdef TANDL
#include "../r_state.h"
#include "../r_main.h"
#include "../p_local.h"
#endif

//=============================================================================
//                                                                      DEFINES
//=============================================================================
// à changer !!!
#define MIN(x,y) ( ((x)<(y)) ? (x) : (y) )
#define MAX(x,y) ( ((x)>(y)) ? (x) : (y) )

#define DL_SQRRADIUS(x)     dynlights->p_lspr[(x)]->dynamic_sqrradius
#define DL_RADIUS(x)        dynlights->p_lspr[(x)]->dynamic_radius
#define LIGHT_POS(i)        dynlights->position[(i)]

#define DL_HIGH_QUALITY

//=============================================================================
//                                                                       GLOBAL
//=============================================================================


void CV_grMonsterDL_OnChange (void);

consvar_t cv_grdynamiclighting = {"gr_dynamiclighting",  "On", CV_SAVE, CV_OnOff };
consvar_t cv_grstaticlighting  = {"gr_staticlighting",   "On", CV_SAVE, CV_OnOff };
consvar_t cv_grcoronas         = {"gr_coronas",          "On", CV_SAVE, CV_OnOff };
consvar_t cv_grmblighting      = {"gr_mblighting",      "On", CV_SAVE|CV_CALL
                                  , CV_OnOff, CV_grMonsterDL_OnChange };

static dynlights_t view_dynlights[2]; // 2 players in splitscreen mode
static dynlights_t *dynlights = &view_dynlights[0];

typedef enum {
    NOLIGHT = 0,
    PLASMA_L,
    PLASMAEXP_L,
    ROCKET_L,
    ROCKETEXP_L,
    BFG_L,
    BFGEXP_L,
    BLUETALL_L,
    GREENTALL_L,
    REDTALL_L,
    BLUESMALL_L,
    GREENSMALL_L,
    REDSMALL_L,
    TECHLAMP_L,
    TECHLAMP2_L,
    COLUMN_L,
    REDBALL_L,
    GREENBALL_L,
    NUMLIGHTS
} lightspritenum_t;

#define UNDEFINED_SPR   0x0 // actually just for testing
#define CORONA_SPR      0x1 // a light source which only emit a corona
#define DYNLIGHT_SPR    0x2 // a light source which is only used for dynamic lighting
#define LIGHT_SPR       (DYNLIGHT_SPR|CORONA_SPR)
#define ROCKET_SPR      (DYNLIGHT_SPR|CORONA_SPR|0x10)
//#define MONSTER_SPR     4
//#define AMMO_SPR        8
//#define BONUS_SPR      16

static light_t lspr[NUMLIGHTS] = {
    // type       offset x,y     coronas color, c_size,light color,l_radius, sqr radius computed at init
   // UNDEFINED: 0  
    { UNDEFINED_SPR,  0.0f,   0.0f,        0x0,  24.0f,        0x0,   0.0f },
    // weapons
    // PLASMA_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x60ff7750,  24.0f, 0x20f77760,  80.0f },
    // PLASMAEXP_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x60ff7750,  24.0f, 0x40f77760, 120.0f },
    // ROCKET_L
    {   ROCKET_SPR,   0.0f,   0.0f, 0x606060f0,  20.0f, 0x4020f7f7, 120.0f },
    // ROCKETEXP_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x606060f0,  20.0f, 0x6020f7f7, 200.0f },
    // BFG_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x6077f777, 120.0f, 0x8060f060, 200.0f },
    // BFGEXP_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x6077f777, 120.0f, 0x6060f060, 400.0f },

    // tall lights
    // BLUETALL_L
    {    LIGHT_SPR,   0.0f,  20.0f, 0x80ff7070,  75.0f, 0x20ff5050, 100.0f }, 
    // GREENTALL_L
    {    LIGHT_SPR,   0.0f,  20.0f, 0x5060ff60,  75.0f, 0x2070ff70, 100.0f }, 
    // REDTALL_L
    {    LIGHT_SPR,   0.0f,  20.0f, 0x705070ff,  75.0f, 0x205070ff, 100.0f }, 

    // small lights
    // BLUESMALL_L
    {    LIGHT_SPR,   0.0f,   4.0f, 0x80ff7070,  60.0f, 0x20ff5050, 100.0f },
    // GREENSMALL_L
    {    LIGHT_SPR,   0.0f,   4.0f, 0x6070ff70,  60.0f, 0x2070ff70, 100.0f },
    // REDSMALL_L
    {    LIGHT_SPR,   0.0f,   4.0f, 0x705070ff,  60.0f, 0x205070ff, 100.0f },

    // other lights
    // TECHLAMP_L
    {    LIGHT_SPR,   0.0f,  36.0f, 0x80ffb0b0,  75.0f, 0x20ffb0b0, 100.0f },
    // TECHLAMP2_L
    {    LIGHT_SPR,   0.0f,  36.0f, 0x80ffb0b0,  60.0f, 0x20ffb0b0, 100.0f },
    // COLUMN_L
    {    LIGHT_SPR,   0.0f,  30.0f, 0x80b0f0f0,  60.0f, 0x20b0f0f0, 100.0f },
    // REDBALL_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x606060f0,   0.0f, 0x302070ff, 100.0f },
    // GREENBALL_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x6077f777, 120.0f, 0x3060f060, 100.0f },
};


light_t *t_lspr[NUMSPRITES] = {
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    //Fab:
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
    &lspr[NOLIGHT],     // SPR_TROO
};


//=============================================================================
//                                                                       EXTERN
//=============================================================================

extern  float   gr_viewludsin;
extern  float   gr_viewludcos;


//=============================================================================
//                                                                       PROTOS
//=============================================================================

static void  HWR_SetLight( void );
static float HWR_DistP2D(lvertex3D_t *p1, lvertex3D_t *p2, lvertex3D_t *p3, lvertex3D_t *inter);

void CV_grMonsterDL_OnChange (void)
{
    if (cv_grmblighting.value)
    {
        t_lspr[SPR_BAL1] = &lspr[REDBALL_L];
        t_lspr[SPR_BAL2] = &lspr[REDBALL_L];
        t_lspr[SPR_MANF] = &lspr[ROCKET_L];
        t_lspr[SPR_BAL7] = &lspr[GREENBALL_L];
        t_lspr[SPR_APLS] = &lspr[GREENBALL_L];
        t_lspr[SPR_APBX] = &lspr[GREENBALL_L];
    }
    else
    {
        t_lspr[SPR_BAL1] = &lspr[NOLIGHT];
        t_lspr[SPR_BAL2] = &lspr[NOLIGHT];
        t_lspr[SPR_MANF] = &lspr[NOLIGHT];
        t_lspr[SPR_BAL7] = &lspr[NOLIGHT];
        t_lspr[SPR_APLS] = &lspr[NOLIGHT];
        t_lspr[SPR_APBX] = &lspr[NOLIGHT];
    }
}

// --------------------------------------------------------------------------
// calcul la projection d'un point sur une droite (determinée par deux 
// points) et ensuite calcul la distance (au carré) de ce point au point
// projecté sur cette droite
// --------------------------------------------------------------------------
static float HWR_DistP2D(lvertex3D_t *p1, lvertex3D_t *p2, lvertex3D_t *p3, lvertex3D_t *inter)
{
    if (p1->z == p2->z) {
        inter->x = p3->x;
        inter->z = p1->z;
    } else if (p1->x == p2->x) {
        inter->x = p1->x;
        inter->z = p3->z;
    } else {
        register float local, pente;
        // Wat een mooie formula! Hurdler's math ;-)
        pente = ( p1->z-p2->z ) / ( p1->x-p2->x );
        local = p1->z - p1->x*pente;
        inter->x = (p3->z - local + p3->x/pente) * (pente/(pente*pente+1));
        inter->z = inter->x*pente + local;
    }
    return (p3->x-inter->x)*(p3->x-inter->x) + (p3->z-inter->z)*(p3->z-inter->z);
}

// check if circle (radius r) centred in p3 touch the bounding box of p1, p2
static boolean CircleTouchBBox(lvertex3D_t *p1, lvertex3D_t *p2, lvertex3D_t *p3, float r)
{
    float minx=p1->x,maxx=p2->x,miny=p2->y,maxy=p1->y;

    if( minx>maxx )
    {
        minx=maxx;
        maxx=p1->x;
    }
    if( miny>maxy )
    {
        miny=maxy;
        maxy=p2->y;
    }
    if( minx-r > p3->x ) return false;
    if( maxx+r < p3->x ) return false;
    if( miny-r > p3->y ) return false;
    if( maxy+r < p3->y ) return false;
    return true;
}

// Hurdler: The old code was removed by me because I don't think it will be used one day.
//          (It's still available on the CVS for educational purpose: Revision 1.8)

// --------------------------------------------------------------------------
// calcul du dynamic lighting sur les murs
// lVerts contient les coords du mur sans le mlook (up/down)
// --------------------------------------------------------------------------
void HWR_WallLighting(FOutVector *wlVerts, lvertex3D_t *lVerts)
{
    int             i, j;

    // dynlights->nb == 0 if cv_grdynamiclighting.value is not set
    for (j=0; j<dynlights->nb; j++) {
        lvertex3D_t     inter;
        FSurfaceInfo    Surf;
        float           dist_p2d, d[4], s;

        // check bounding box first
        if( CircleTouchBBox(&lVerts[2], &lVerts[0], &LIGHT_POS(j), DL_RADIUS(j))==false )
            continue;

        // check exact distance
        dist_p2d = HWR_DistP2D(&lVerts[2], &lVerts[0],  &LIGHT_POS(j), &inter);
        if (dist_p2d >= DL_SQRRADIUS(j))
            continue;

        d[0] = sqrt((lVerts[0].x-inter.x)*(lVerts[0].x-inter.x)+(lVerts[0].z-inter.z)*(lVerts[0].z-inter.z));
        if ((lVerts[0].x-inter.x) < 0)
            d[0] = -d[0];
        d[1] = sqrt((lVerts[2].x-inter.x)*(lVerts[2].x-inter.x)+(lVerts[2].z-inter.z)*(lVerts[2].z-inter.z));
        if ((lVerts[2].x-inter.x) < 0)
            d[1] = -d[1];
        d[2] = d[1]; d[3] = d[0];
#ifdef DL_HIGH_QUALITY
        s = 0.5 / DL_RADIUS(j);
#else
        s = 0.5 / sqrt(DL_SQRRADIUS(j)-dist_p2d);
#endif
        for (i=0; i<4; i++) {
            wlVerts[i].sow = 0.5 + d[i]*s;
            wlVerts[i].tow = 0.5 + (lVerts[i].y-LIGHT_POS(j).y)*s*1.2f;
        }

        HWR_SetLight();

        Surf.FlatColor.rgba = dynlights->p_lspr[j]->dynamic_color;
#ifdef DL_HIGH_QUALITY
        Surf.FlatColor.s.alpha *= (1-dist_p2d/DL_SQRRADIUS(j));
#endif
        if( !dynlights->mo[j]->state )
            return;
        // next state is null so fade out with alpha
        if( (dynlights->mo[j]->state->nextstate == S_NULL) )
            Surf.FlatColor.s.alpha *= (float)dynlights->mo[j]->tics/(float)dynlights->mo[j]->state->tics;

        HWD.pfnDrawPolygon ( &Surf, wlVerts, 4, PF_Modulated|PF_Additive|PF_Clip );

    } // end for (j=0; j<dynlights->nb; j++)
}


// BP: big hack for a test in lighting ref:1249753487AB
extern int *bbox;

// --------------------------------------------------------------------------
// calcul du dynamic lighting sur le sol
// clVerts contient les coords du sol avec le mlook (up/down)
// --------------------------------------------------------------------------
void HWR_PlaneLighting(FOutVector *clVerts, int nrClipVerts, lvertex3D_t *lVerts )
{
    int     i, j;
    lvertex3D_t p1,p2;

    p1.y=bbox[BOXTOP]*crapmul;
    p1.x=bbox[BOXLEFT]*crapmul;
    p2.y=bbox[BOXBOTTOM]*crapmul;
    p2.x=bbox[BOXRIGHT]*crapmul;

    for (j=0; j<dynlights->nb; j++) {
        FSurfaceInfo    Surf;
        float           dist_p2d, s;

        // BP: The kickass Optimization: check if light touch bounding box
        if( CircleTouchBBox(&p1, &p2, &dynlights->positionnottransformed[j], DL_RADIUS(j))==false )
            continue;

        dist_p2d = (lVerts[0].y-LIGHT_POS(j).y);
        dist_p2d *= dist_p2d;
        if (dist_p2d >= DL_SQRRADIUS(j))
            continue;

#ifdef DL_HIGH_QUALITY
        s = 0.5f / DL_RADIUS(j);
#else
        s = 0.5f / sqrt(DL_SQRRADIUS(j)-dist_p2d);
#endif
        for (i=0; i<nrClipVerts; i++) {
            clVerts[i].sow = 0.5f + (lVerts[i].x-LIGHT_POS(j).x)*s;
            clVerts[i].tow = 0.5f + (lVerts[i].z-LIGHT_POS(j).z)*s*1.2f;
        }

        HWR_SetLight();

        Surf.FlatColor.rgba = dynlights->p_lspr[j]->dynamic_color;
#ifdef DL_HIGH_QUALITY
        Surf.FlatColor.s.alpha *= (1-dist_p2d/DL_SQRRADIUS(j));
#endif
        if( !dynlights->mo[j]->state )
            return;
        // next state is null so fade out with alpha
        if( (dynlights->mo[j]->state->nextstate == S_NULL) )
            Surf.FlatColor.s.alpha *= (float)dynlights->mo[j]->tics/(float)dynlights->mo[j]->state->tics;

        HWD.pfnDrawPolygon ( &Surf, clVerts, nrClipVerts, PF_Modulated|PF_Additive|PF_Clip );

    } // end for (j=0; j<dynlights->nb; j++)
}


// --------------------------------------------------------------------------
// coronas lighting
// --------------------------------------------------------------------------
void HWR_DoCoronasLighting(FOutVector *outVerts, gr_vissprite_t *spr) 
{
    light_t   *p_lspr;

    //CONS_Printf("sprite (type): %d (%s)\n", spr->type, sprnames[spr->type]);
    p_lspr = t_lspr[spr->type];
    if( p_lspr == &lspr[ROCKET_L] &&
        spr->mobj->state>=&states[S_EXPLODE1] &&spr->mobj->state<=&states[S_EXPLODE3] )
    {
        p_lspr = &lspr[ROCKETEXP_L];
    }

    if ( cv_grcoronas.value && (p_lspr->type & CORONA_SPR) ) { // it's an object which emits light
        FOutVector      light[4];
        FSurfaceInfo    Surf;
        float           cx=0.0f, cy=0.0f, cz=0.0f; // gravity center
        float           size;
        int             i;

        switch (p_lspr->type)
        {
            case LIGHT_SPR: 
                size  = p_lspr->corona_radius  * ((outVerts[0].oow+120.0f)/950.0f); // d'ou vienne ces constante ?
                break;
            case ROCKET_SPR: 
                p_lspr->corona_color = (((M_Random()>>1)&0xff)<<24)|0x0040ff;
                // don't need a break
            case CORONA_SPR: 
                size  = p_lspr->corona_radius  * ((outVerts[0].oow+60.0f)/100.0f); // d'ou vienne ces constante ?
                break;
        }
        if (size > p_lspr->corona_radius) 
            size = p_lspr->corona_radius;
        size *= 2;

        for (i=0; i<4; i++) {
            cx += outVerts[i].x;
            cy += outVerts[i].y;
            cz += outVerts[i].oow;
        }
        cx /= 4.0f;  cy /= 4.0f;  cz /= 4.0f;

        // more realistique corona !
        if( cz>255*8+250 )
            return;
        Surf.FlatColor.rgba = p_lspr->corona_color;
        if(cz>250.0f)
            Surf.FlatColor.s.alpha = 0xff-((int)cz-250)/8;
        else
            Surf.FlatColor.s.alpha = 0xff;

        // Bp; je comprend pas, ou est la rotation haut/bas ?
        //     tu ajoute un offset a y mais si la tu la reguarde de haut 
        //     sa devrais pas marcher ... comprend pas :(
        //     (...) bon je croit que j'ai comprit il est tout pourit le code ?
        //           car comme l'offset est minime sa ce voit pas !
        light[0].x = cx-size; light[0].oow = cz;
        light[0].y = cy-size*1.33f+p_lspr->light_yoffset; 
        light[0].sow = 0.0f;   light[0].tow = 0.0f;

        light[1].x = cx+size; light[1].oow = cz;
        light[1].y = cy-size*1.33f+p_lspr->light_yoffset; 
        light[1].sow = 1.0f;   light[1].tow = 0.0f;

        light[2].x = cx+size; light[2].oow = cz;
        light[2].y = cy+size*1.33f+p_lspr->light_yoffset; 
        light[2].sow = 1.0f;   light[2].tow = 1.0f;

        light[3].x = cx-size; light[3].oow = cz;
        light[3].y = cy+size*1.33f+p_lspr->light_yoffset; 
        light[3].sow = 0.0f;   light[3].tow = 1.0f;

        HWR_GetPic(W_GetNumForName("corona"));  // TODO: use different coronas

        HWD.pfnDrawPolygon ( &Surf, light, 4, PF_Modulated | PF_Additive | PF_Clip | PF_Corona | PF_NoDepthTest);
    }
}

// --------------------------------------------------------------------------
// Remove all the dynamic lights at eatch frame
// --------------------------------------------------------------------------
void HWR_ResetLights(void)
{
    dynlights->nb = 0;
}

// --------------------------------------------------------------------------
// Change view, thus change lights (splitscreen)
// --------------------------------------------------------------------------
void HWR_SetLights(int viewnumber)
{
    dynlights = &view_dynlights[viewnumber];
}

// --------------------------------------------------------------------------
// Add a light for dynamic lighting
// The light position is already transformed execpt for mlook
// --------------------------------------------------------------------------
void HWR_DL_AddLight(gr_vissprite_t *spr, GlidePatch_t *patch)
{
    light_t   *p_lspr;

    //Hurdler: moved here because it's better ;-)
    if (!cv_grdynamiclighting.value)
        return;

    // check if sprite contain dynamic light
    p_lspr = t_lspr[spr->type];
    if ( (p_lspr->type&DYNLIGHT_SPR) 
         && ((p_lspr->type!=LIGHT_SPR) || cv_grstaticlighting.value) 
         && (dynlights->nb < DL_MAX_LIGHT) 
         && spr->mobj->state )
    {
        LIGHT_POS(dynlights->nb).x = (spr->x1+spr->x2) / 2.0f;
        LIGHT_POS(dynlights->nb).y = spr->ty - patch->height/2.0f;
        LIGHT_POS(dynlights->nb).z = spr->tz;

        dynlights->positionnottransformed[dynlights->nb].x = spr->mobj->x*crapmul;
        dynlights->positionnottransformed[dynlights->nb].y = spr->mobj->y*crapmul;
        dynlights->mo[dynlights->nb] = spr->mobj;
        if( spr->mobj->state>=&states[S_EXPLODE1] && 
            spr->mobj->state<=&states[S_EXPLODE3] )
        {
            p_lspr = &lspr[ROCKETEXP_L];
        }

        dynlights->p_lspr[dynlights->nb] = p_lspr;
        
        dynlights->nb++;
    } 
}

static GlidePatch_t lightmappatch;

void HWR_InitLight( void )
{
    int i;

    // precalculate sqr radius
    for(i=0;i<NUMLIGHTS;i++)
        lspr[i].dynamic_sqrradius = lspr[i].dynamic_radius*lspr[i].dynamic_radius;

    lightmappatch.mipmap.downloaded = false;
}

// -----------------+
// HWR_SetLight    : Download a disc shaped alpha map for rendering fake lights
// -----------------+
void HWR_SetLight( void )
{
    int    i, j;

    if (!lightmappatch.mipmap.downloaded && !lightmappatch.mipmap.grInfo.data)
    {

        USHORT *Data = Z_Malloc( 128*128*sizeof(USHORT), PU_3DFXCACHE, &lightmappatch.mipmap.grInfo.data );
                
            for( i=0; i<128; i++ )
            {
                for( j=0; j<128; j++ )
                {
                    int pos = ((i-64)*(i-64))+((j-64)*(j-64));
                    if (pos <= 63*63) 
                        Data[i*128+j] = ((byte)(255-(4*sqrt(pos)))) << 8 | 0xff;
                    else 
                        Data[i*128+j] = 0;
                }
            }
        lightmappatch.mipmap.grInfo.format = GR_TEXFMT_ALPHA_INTENSITY_88;

        lightmappatch.width = 128;
        lightmappatch.height = 128;
        lightmappatch.mipmap.width = 128;
        lightmappatch.mipmap.height = 128;
        lightmappatch.mipmap.grInfo.smallLodLog2 = GR_LOD_LOG2_128;
        lightmappatch.mipmap.grInfo.largeLodLog2 = GR_LOD_LOG2_128;
        lightmappatch.mipmap.grInfo.aspectRatioLog2 = GR_ASPECT_LOG2_1x1;

    }
    HWD.pfnSetTexture( &lightmappatch.mipmap );
}


void HWR_DynamicShadowing(FOutVector *clVerts, int nrClipVerts, lvertex3D_t *lVerts, player_t *p)
{
    int  i;
    FSurfaceInfo    Surf;

    if (!cv_grdynamiclighting.value)
        return;

    for (i=0; i<nrClipVerts; i++) {
        clVerts[i].sow = 0.5f + lVerts[i].x*0.01;
        clVerts[i].tow = 0.5f + lVerts[i].z*0.01*1.2f;
    }
    
    HWR_SetLight();

    Surf.FlatColor.rgba = 0x70707070;

    HWD.pfnDrawPolygon ( &Surf, clVerts, nrClipVerts, PF_Modulated|PF_Additive|PF_Clip );
    //HWD.pfnDrawPolygon ( &Surf, clVerts, nrClipVerts, PF_Modulated|PF_Environment|PF_Clip );
}


//**********************************************************
// Hurdler: new code for faster static lighting and and T&L
//**********************************************************

#ifdef TANDL

// est ce bien necessaire ?
static sector_t *gr_frontsector;
static sector_t *gr_backsector ;
static seg_t    *gr_curline;


/*
static void HWR_StoreWallRange (int startfrac, int endfrac)
{
...(voir hw_main.c)...
}
*/


// p1 et p2 c'est le deux bou du seg en float
void HWR_BuildWallLightmaps(lvertex3D_t *p1, lvertex3D_t *p2, int lighnum, seg_t *line)
{
    lightmap_t *lp;

    // (...) calcul presit de la projection et de la distance

//    if (dist_p2d >= DL_SQRRADIUS(lightnum))
//        return;

    // (...) attention faire le backfase cull histoir de faire mieux que Q3 !

    lp = malloc(sizeof(lightmap_t));
    lp->next = line->lightmaps;
    line->lightmaps = lp;
    
    // (...) encore des bô calcul bien lourd et on stock tout sa dans la lightmap
}

static void HWR_AddLightMapForLine( int lightnum, seg_t *line)
{
    /*
    int                 x1;
    int                 x2;
    angle_t             angle1;
    angle_t             angle2;
    angle_t             span;
    angle_t             tspan;
    */
    lvertex3D_t         p1,p2;
    
    gr_curline = line;
    gr_backsector = line->backsector;
    
    // Reject empty lines used for triggers and special events.
    // Identical floor and ceiling on both sides,
    //  identical light levels on both sides,
    //  and no middle texture.
/*
    if (   gr_backsector->ceilingpic == gr_frontsector->ceilingpic
        && gr_backsector->floorpic == gr_frontsector->floorpic
        && gr_backsector->lightlevel == gr_frontsector->lightlevel
        && gr_curline->sidedef->midtexture == 0)
    {
        return;
    }
*/

    p1.y=gr_curline->v1->y*crapmul;
    p1.x=gr_curline->v1->x*crapmul;
    p2.y=gr_curline->v2->y*crapmul;
    p2.x=gr_curline->v2->x*crapmul;

    // check bbox of the seg
    if( CircleTouchBBox(&p1, &p2, &LIGHT_POS(lightnum), DL_RADIUS(lightnum))==false )    
        return;

    HWR_BuildWallLightmaps(&p1, &p2, lightnum, line);
}



//TODO: see what HWR_AddLine does
static void HWR_CheckSubsector( int num, fixed_t *bbox )
{
    int         count;
    seg_t       *line;
    subsector_t *sub;
    lvertex3D_t p1,p2;
    int         lightnum;

    p1.y=bbox[BOXTOP]*crapmul;
    p1.x=bbox[BOXLEFT]*crapmul;
    p2.y=bbox[BOXBOTTOM]*crapmul;
    p2.x=bbox[BOXRIGHT]*crapmul;


    if (num < numsubsectors)
    {
        sub = &subsectors[num];         // subsector
        for(lightnum=0; lightnum<dynlights->nb; lightnum++)
        {
            if( CircleTouchBBox(&p1, &p2, &LIGHT_POS(lightnum), DL_RADIUS(lightnum))==false )
                continue;

            count = sub->numlines;          // how many linedefs
            line = &segs[sub->firstline];   // first line seg
            while (count--)
            {
                HWR_AddLightMapForLine (lightnum, line);       // compute lightmap
                line++;
            }
        }
    }
}


// --------------------------------------------------------------------------
// Hurdler: this adds lights by mobj.
// --------------------------------------------------------------------------
static void HWR_AddMobjLights(mobj_t *thing)
{
    if ( t_lspr[thing->sprite]->type & CORONA_SPR )
    {
        LIGHT_POS(dynlights->nb).x = (float)thing->x * crapmul;
        LIGHT_POS(dynlights->nb).y = (float)thing->z * crapmul + t_lspr[thing->sprite]->light_yoffset;
        LIGHT_POS(dynlights->nb).z = (float)thing->y * crapmul;
        
        dynlights->p_lspr[dynlights->nb] = t_lspr[thing->sprite];
        
        dynlights->nb++;
        if (dynlights->nb>DL_MAX_LIGHT)
            dynlights->nb = DL_MAX_LIGHT;
    }
}

//Hurdler: The goal of this function is to walk through all the bsp starting
//         on the top. 
//         We need to do that to know all the lights in the map and all the walls
static void HWR_ComputeLightMapsInBSPNode(int bspnum, fixed_t *bbox)
{
    if (bspnum & NF_SUBSECTOR) // Found a subsector?
    {
        if (bspnum == -1)
            HWR_CheckSubsector(0, bbox);  // probably unecessary: see boris' comment in hw_bsp
        else
            HWR_CheckSubsector(bspnum&(~NF_SUBSECTOR), bbox);
        return;
    }
    HWR_ComputeLightMapsInBSPNode(nodes[bspnum].children[0], nodes[bspnum].bbox[0]);
    HWR_ComputeLightMapsInBSPNode(nodes[bspnum].children[1], nodes[bspnum].bbox[1]);
}

static void HWR_SearchLightsInMobjs(void)
{
    thinker_t*          th;
    //mobj_t*             mobj;

    // search in the list of thinkers
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        // a mobj ?
        if (th->function.acp1 == (actionf_p1)P_MobjThinker)
            HWR_AddMobjLights((mobj_t *)th);
    }
}

//
// HWR_CreateStaticLightmaps()
//
void HWR_CreateStaticLightmaps(int bspnum)
{
    CONS_Printf("HWR_CreateStaticLightmaps\n");

    dynlights->nb = 0;

    // First: Searching for lights
    HWR_SearchLightsInMobjs();
    CONS_Printf("%d lights found\n", dynlights->nb);

    // Second: Build all lightmap for walls covered by lights
    validcount++; // to be sure
    HWR_ComputeLightMapsInBSPNode(bspnum, NULL);
}

#endif //TANDL

/*
TODO:

  - Les coronas ne sont pas gérer avec le nouveau systeme, seul le dynamic lighting l'est
  - calculer l'offset des coronas au chargement du level et non faire la moyenne
    au moment de l'afficher
     BP: euh non en fait il faux encoder la position de la light dans le sprite
         car c'est pas focement au mileux de plus il peut en y avoir plusieur (chandelier)
  - changer la comparaison pour l'affichage des coronas (+ un epsilon)
    BP: non non j'ai trouver mieux :) : lord du AddSprite tu rajoute aussi la coronas
        dans la sprite list ! avec un z de epsilon (attention au ZCLIP_PLANE) et donc on 
        l'affiche en dernier histoir qu'il puisse etre cacher par d'autre sprite :)
        Bon fait metre pas mal de code special dans hwr_project sprite mais sa vaux le 
        coup
  - gerer dynamic et static : retenir le nombre de lightstatic et clearer toute les 
        light>lightstatic (les dynamique) et les lightmap correspondant dans les segs
        puit refaire une passe avec le code si dessus mais rien que pour les dynamiques
        (tres petite modification)
  - finalement virer le hack splitscreen, il n'est plus necessaire !
*/
