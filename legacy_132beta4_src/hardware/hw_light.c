// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw_light.c,v 1.42 2001/12/28 16:57:45 hurdler Exp $
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
// Revision 1.42  2001/12/28 16:57:45  hurdler
// Add setcorona command to FS
//
// Revision 1.41  2001/12/26 17:24:47  hurdler
// Update Linux version
//
// Revision 1.40  2001/08/27 19:59:35  hurdler
// Fix colormap in heretic + opengl, fixedcolormap and NEWCORONA
//
// Revision 1.39  2001/08/26 15:27:29  bpereira
// added fov for glide and fixed newcoronas code
//
// Revision 1.38  2001/08/11 01:24:30  hurdler
// Fix backface culling problem with floors/ceiling
//
// Revision 1.37  2001/08/09 21:35:23  hurdler
// Add translucent 3D water in hw mode
//
// Revision 1.36  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.35  2001/08/06 14:13:45  hurdler
// Crappy MD2 implementation (still need lots of work)
//
// Revision 1.34  2001/04/28 15:18:46  hurdler
// newcoronas defined again
//
// Revision 1.33  2001/04/17 22:30:40  hurdler
// fix some (strange!) problems
//
// Revision 1.32  2001/04/09 14:17:45  hurdler
// no message
//
// Revision 1.31  2001/02/28 17:50:56  bpereira
// no message
//
// Revision 1.30  2001/02/24 13:35:22  bpereira
// no message
//
// Revision 1.29  2001/01/25 18:56:27  bpereira
// no message
//
// Revision 1.28  2000/11/18 15:51:25  bpereira
// no message
//
// Revision 1.27  2000/11/02 19:49:39  bpereira
// no message
//
// Revision 1.26  2000/10/04 16:21:57  hurdler
// small clean-up
//
// Revision 1.25  2000/09/28 20:57:20  bpereira
// no message
//
// Revision 1.24  2000/09/21 16:45:11  bpereira
// no message
//
// Revision 1.23  2000/08/31 14:30:57  bpereira
// no message
//
// Revision 1.22  2000/08/11 19:11:57  metzgermeister
// *** empty log message ***
//
// Revision 1.21  2000/08/11 12:27:43  hurdler
// latest changes for v1.30
//
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
#include "../r_state.h"
#include "../r_main.h"
#include "../p_local.h"

//=============================================================================
//                                                                      DEFINES
//=============================================================================

#define DL_SQRRADIUS(x)     dynlights->p_lspr[(x)]->dynamic_sqrradius
#define DL_RADIUS(x)        dynlights->p_lspr[(x)]->dynamic_radius
#define LIGHT_POS(i)        dynlights->position[(i)]

#define DL_HIGH_QUALITY
//#define LIGHTMAPFLAGS  (PF_Masked|PF_Clip|PF_NoAlphaTest)  // debug see overdraw
#define LIGHTMAPFLAGS (PF_Modulated|PF_Additive|PF_Clip)

//=============================================================================
//                                                                       GLOBAL
//=============================================================================


void CV_grMonsterDL_OnChange (void);

consvar_t cv_grdynamiclighting = {"gr_dynamiclighting",  "On", CV_SAVE, CV_OnOff };
consvar_t cv_grstaticlighting  = {"gr_staticlighting",   "On", CV_SAVE, CV_OnOff };
consvar_t cv_grcoronas         = {"gr_coronas",          "On", CV_SAVE, CV_OnOff };
consvar_t cv_grcoronasize      = {"gr_coronasize",        "1", CV_SAVE| CV_FLOAT,0 };
consvar_t cv_grmblighting      = {"gr_mblighting",       "On", CV_SAVE|CV_CALL
                                  , CV_OnOff, CV_grMonsterDL_OnChange };

static dynlights_t view_dynlights[2]; // 2 players in splitscreen mode
static dynlights_t *dynlights = &view_dynlights[0];



#define UNDEFINED_SPR   0x0 // actually just for testing
#define CORONA_SPR      0x1 // a light source which only emit a corona
#define DYNLIGHT_SPR    0x2 // a light source which is only used for dynamic lighting
#define LIGHT_SPR       (DYNLIGHT_SPR|CORONA_SPR)
#define ROCKET_SPR      (DYNLIGHT_SPR|CORONA_SPR|0x10)
//#define MONSTER_SPR     4
//#define AMMO_SPR        8
//#define BONUS_SPR      16

//Hurdler: now we can change those values via FS :)
light_t lspr[NUMLIGHTS] = {
    // type       offset x,   y  coronas color, c_size,light color,l_radius, sqr radius computed at init
   // UNDEFINED: 0  
    { UNDEFINED_SPR,  0.0f,   0.0f,        0x0,  24.0f,        0x0,   0.0f },
    // weapons
    // RINGSPARK_L
    { LIGHT_SPR,      0.0f,   0.0f, 0x0000e0ff,  16.0f, 0x0000e0ff,  32.0f }, // Tails 09-08-2002
    // SUPERSONIC_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0xff00e0ff,  32.0f, 0xff00e0ff, 128.0f }, // Tails 09-08-2002
    // SUPERSPARK_L
    { LIGHT_SPR,      0.0f,   0.0f, 0xe0ffffff,  8.0f, 0xe0ffffff, 64.0f },
    // INVINCIBLE_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x10ffaaaa,  16.0f, 0x10ffaaaa, 128.0f },
    // GREENSHIELD_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x602b7337, 32.0f, 0x602b7337, 128.0f },
    // BLUESHIELD_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x60cb0000, 32.0f, 0x60cb0000, 128.0f },

    // tall lights
    // YELLOWSHIELD_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x601f7baf, 32.0f, 0x601f7baf, 128.0f },

    // REDSHIELD_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x600000cb, 32.0f, 0x600000cb, 128.0f },

    // BLACKSHIELD_L // Black light? lol
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x60010101, 32.0f, 0x60ff00ff, 128.0f },
    // SMALLREDBALL_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x606060f0,   0.0f, 0x302070ff, 32.0f },

    // small lights
    // RINGLIGHT_L
    { DYNLIGHT_SPR,   0.0f,  0.0f, 0x60b0f0f0,    0.0f, 0x30b0f0f0, 100.0f },
    // GREENSMALL_L
    {    LIGHT_SPR,   0.0f,  14.0f, 0x6070ff70,  60.0f, 0x4070ff70, 100.0f },
    // REDSMALL_L
    {    LIGHT_SPR,   0.0f,  14.0f, 0x705070ff,  60.0f, 0x405070ff, 100.0f },
/*
    // other lights
    // TECHLAMP_L
    {    LIGHT_SPR,   0.0f,  33.0f, 0x80ffb0b0,  75.0f, 0x40ffb0b0, 100.0f },
    // TECHLAMP2_L
    {    LIGHT_SPR,   0.0f,  26.0f, 0x80ffb0b0,  60.0f, 0x40ffb0b0, 100.0f },
    // COLUMN_L
    {    LIGHT_SPR,   3.0f,  19.0f, 0x80b0f0f0,  60.0f, 0x40b0f0f0, 100.0f },
    // CANDLE_L
    {    LIGHT_SPR,   0.0f,   6.0f, 0x60b0f0f0,  20.0f, 0x30b0f0f0,  30.0f },
    // CANDLEABRE_L
    {    LIGHT_SPR,   0.0f,  30.0f, 0x60b0f0f0,  60.0f, 0x30b0f0f0, 100.0f },
*/

    // type       offset x,   y  coronas color, c_size,light color,l_radius, sqr radius computed at init

	// GREENSHINE_L
	{    LIGHT_SPR,   0.0f,  0.0f, 0xff00ff00,  64.0f, 0xff00ff00, 256.0f },
	// ORANGESHINE_L
	{    LIGHT_SPR,   0.0f,  0.0f, 0xff0080ff,  64.0f, 0xff0080ff, 256.0f },
	// PINKSHINE_L
	{    LIGHT_SPR,   0.0f,  0.0f, 0xffe080ff,  64.0f, 0xffe080ff, 256.0f },
	// BLUESHINE_L
	{    LIGHT_SPR,   0.0f,  0.0f, 0xffff0000,  64.0f, 0xffff0000, 256.0f },
	// REDSHINE_L
	{    LIGHT_SPR,   0.0f,  0.0f, 0xff0000ff,  64.0f, 0xff0000ff, 256.0f },
	// LBLUESHINE_L
	{    LIGHT_SPR,   0.0f,  0.0f, 0xffff8080,  64.0f, 0xffff8080, 256.0f },
	// GREYSHINE_L
	{    LIGHT_SPR,   0.0f,  0.0f, 0xffe0e0e0,  64.0f, 0xffe0e0e0, 256.0f },

    // monsters
    // REDBALL_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x606060ff,   0.0f, 0x606060ff, 100.0f },
    // GREENBALL_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x6060ff60, 120.0f, 0x6060ff60, 100.0f },
    // BLUEBALL_L
    { DYNLIGHT_SPR,   0.0f,   0.0f, 0x60ff6060, 120.0f, 0x60ff6060, 100.0f },

	// NIGHTSLIGHT_L
    {    LIGHT_SPR,   0.0f,   6.0f, 0x60ffffff,  16.0f, 0x30ffffff,  32.0f },

	// JETLIGHT_L
    { DYNLIGHT_SPR,   0.0f,   6.0f, 0x60ffaaaa,  16.0f, 0x30ffaaaa,  64.0f },

	// GOOPLIGHT_L
    { DYNLIGHT_SPR,   0.0f,   6.0f, 0x60ff00ff,  16.0f, 0x30ff00ff,  32.0f },

	// STREETLIGHT_L
	{ LIGHT_SPR,      0.0f,   0.0f, 0xe0ffffff,  64.0f, 0xe0ffffff, 384.0f },
};

light_t *t_lspr[NUMSPRITES] = {
    &lspr[NOLIGHT],     // SPR_MISL
    &lspr[NOLIGHT],     // SPR_PLAY
    &lspr[NOLIGHT],     // SPR_POSS
    &lspr[NOLIGHT],     // SPR_SPOS
    &lspr[NOLIGHT],     // SPR_EGGM
    &lspr[NOLIGHT],     // SPR_BON1
    &lspr[NOLIGHT],     // SPR_SRBX
    &lspr[NOLIGHT],     // SPR_GRBX
    &lspr[NOLIGHT],     // SPR_EMMY
    &lspr[NOLIGHT],     // SPR_PINV
    &lspr[NOLIGHT],     // SPR_GRTV
    &lspr[NOLIGHT],     // SPR_SPRY
	&lspr[NOLIGHT],     // SPR_SUDY
    &lspr[NOLIGHT],     // SPR_SHTV
    &lspr[NOLIGHT],     // SPR_FANS
    &lspr[NOLIGHT],     // SPR_BUBL
    &lspr[NOLIGHT],     // SPR_BLTV
    &lspr[NOLIGHT],     // SPR_YLTV
    &lspr[NOLIGHT],     // SPR_FWR1
    &lspr[NOLIGHT],     // SPR_SPRR
	&lspr[NOLIGHT],     // SPR_SUDR
    &lspr[NOLIGHT],     // SPR_SMOK
    &lspr[NOLIGHT],     // SPR_SPLA
    &lspr[NOLIGHT],     // SPR_TNT1
    &lspr[NOLIGHT],     // SPR_BIRD
    &lspr[NOLIGHT],     // SPR_SQRL
    &lspr[BLUESHIELD_L],     // SPR_BORB
    &lspr[YELLOWSHIELD_L],     // SPR_YORB
    &lspr[GREENSHIELD_L],     // SPR_GORB
    &lspr[BLACKSHIELD_L],     // SPR_KORB
    &lspr[RINGSPARK_L],     // SPR_SPRK
    &lspr[INVINCIBLE_L],     // SPR_IVSP
    &lspr[INVINCIBLE_L],     // SPR_IVSQ
    &lspr[NOLIGHT],     // SPR_DISS
    &lspr[NOLIGHT],     // SPR_BUBP
    &lspr[NOLIGHT],     // SPR_BUBO
    &lspr[NOLIGHT],     // SPR_BUBN
    &lspr[NOLIGHT],     // SPR_BUBM
    &lspr[NOLIGHT],     // SPR_CNTA
    &lspr[NOLIGHT],     // SPR_CNTB
    &lspr[NOLIGHT],     // SPR_CNTC
    &lspr[NOLIGHT],     // SPR_CNTD
    &lspr[NOLIGHT],     // SPR_CNTE
    &lspr[NOLIGHT],     // SPR_CNTF
    &lspr[NOLIGHT],     // SPR_POPP
    &lspr[NOLIGHT],     // SPR_PRUP
    &lspr[NOLIGHT],     // SPR_BKTV
    &lspr[NOLIGHT],     // SPR_SCRA
    &lspr[NOLIGHT],     // SPR_SCRB
    &lspr[NOLIGHT],     // SPR_SCRC
    &lspr[NOLIGHT],     // SPR_SCRD
    &lspr[SUPERSPARK_L],     // SPR_SSPK
    &lspr[NOLIGHT],     // SPR_GRAS
    &lspr[NOLIGHT],     // SPR_YSPR
	&lspr[NOLIGHT],     // SPR_RSPR
	&lspr[NOLIGHT],     // SPR_YSUD
	&lspr[NOLIGHT],     // SPR_RSUD
    &lspr[NOLIGHT],     // SPR_SKIM
    &lspr[NOLIGHT],     // SPR_MINE
    &lspr[NOLIGHT],     // SPR_FISH
    &lspr[NOLIGHT],     // SPR_GARG
    &lspr[NOLIGHT],     // SPR_SPLH
    &lspr[NOLIGHT],     // SPR_THOK
    &lspr[NOLIGHT],     // SPR_THZP
    &lspr[NOLIGHT],     // SPR_SIGN
    &lspr[RINGLIGHT_L],     // SPR_RRNG
    &lspr[NOLIGHT],     // SPR_TTAG
    &lspr[NOLIGHT],     // SPR_STEM
    &lspr[REDBALL_L],     // SPR_RFLG
    &lspr[BLUEBALL_L],     // SPR_BFLG
    &lspr[NOLIGHT],     // SPR_GFLG
    &lspr[BLUEBALL_L],     // SPR_TOKE
    &lspr[GREENSHINE_L],     // SPR_CEMG
    &lspr[ORANGESHINE_L],     // SPR_CEMO
    &lspr[PINKSHINE_L],     // SPR_CEMP
    &lspr[BLUESHINE_L],     // SPR_CEMB
    &lspr[REDSHINE_L],     // SPR_CEMR
    &lspr[LBLUESHINE_L],     // SPR_CEML
    &lspr[GREYSHINE_L],     // SPR_CEMY
    &lspr[NOLIGHT],     // SPR_JETB
    &lspr[NOLIGHT],     // SPR_JETG
    &lspr[NOLIGHT],     // SPR_JBUL
    &lspr[NOLIGHT],     // SPR_MOUS
    &lspr[NOLIGHT],     // SPR_DETN
    &lspr[NOLIGHT],     // SPR_XPLD
    &lspr[NOLIGHT],     // SPR_CHAN
    &lspr[NOLIGHT],     // SPR_CAPE
    &lspr[NOLIGHT],     // SPR_SNO1
    &lspr[NOLIGHT],     // SPR_SANT
    &lspr[NOLIGHT],     // SPR_EMER
    &lspr[NOLIGHT],     // SPR_EMES
    &lspr[NOLIGHT],     // SPR_EMET
    &lspr[NOLIGHT],     // SPR_SBLL
    &lspr[NOLIGHT],     // SPR_SPIK
    &lspr[NOLIGHT],     // SPR_CCOM
    &lspr[NOLIGHT],     // SPR_RAIN
    &lspr[NOLIGHT],     // SPR_DSPK
    &lspr[NOLIGHT],     // SPR_USPK
    //Fab:
    &lspr[NOLIGHT],     // SPR_STPT
    &lspr[RINGLIGHT_L],     // SPR_RNGM
    &lspr[RINGLIGHT_L],     // SPR_RNGR
    &lspr[RINGLIGHT_L],     // SPR_RNGS
    &lspr[RINGLIGHT_L],     // SPR_RNGA
    &lspr[RINGLIGHT_L],     // SPR_RNGE
    &lspr[RINGLIGHT_L],     // SPR_TAEH
    &lspr[RINGLIGHT_L],     // SPR_TAER
    &lspr[RINGLIGHT_L],     // SPR_THER
    &lspr[RINGLIGHT_L],     // SPR_TAHR
	&lspr[RINGLIGHT_L],     // SPR_THOM
	&lspr[RINGLIGHT_L],     // SPR_TAUT
	&lspr[RINGLIGHT_L],     // SPR_TEXP
	&lspr[NOLIGHT],     // SPR_BUS1
	&lspr[NOLIGHT],     // SPR_BUS2
	&lspr[NOLIGHT],     // SPR_FWR2
	&lspr[NOLIGHT],     // SPR_FWR3
	&lspr[NOLIGHT],     // SPR_MIXU
	&lspr[NOLIGHT],     // SPR_QUES
	&lspr[NOLIGHT],     // SPR_MTEX
	&lspr[REDBALL_L],     // SPR_FLAM // Tails 09-06-2002
	&lspr[REDBALL_L],   // SPR_LASR

	// Mario-specific stuff Tails 09-10-2002
	&lspr[REDBALL_L],     // SPR_PUMA
	&lspr[NOLIGHT],     // SPR_HAMM
	&lspr[NOLIGHT],     // SPR_KOOP
	&lspr[NOLIGHT],     // SPR_SHLL
	&lspr[NOLIGHT],     // SPR_MAXE
	&lspr[REDBALL_L],     // SPR_BFLM
	&lspr[SMALLREDBALL_L],     // SPR_FBLL
	&lspr[NOLIGHT],     // SPR_FFWR

	&lspr[NOLIGHT],     // SPR_NSPK

	&lspr[SUPERSONIC_L],     // SPR_SUPE // NiGHTS character flying
	&lspr[SUPERSONIC_L],     // SPR_SUPZ // NiGHTS hurt
	&lspr[SUPERSONIC_L],     // SPR_NDRN // NiGHTS drone
	&lspr[SUPERSONIC_L],     // SPR_NDRL // NiGHTS character drilling
	&lspr[NIGHTSLIGHT_L],     // SPR_SEED // Sonic CD flower seed
	&lspr[JETLIGHT_L],     // SPR_JETF // Boss jet fumes
	&lspr[NOLIGHT],     // SPR_HOOP
	&lspr[NOLIGHT],     // SPR_NSCR
	&lspr[NOLIGHT],     // SPR_NWNG
	&lspr[NOLIGHT],     // SPR_EGGN
	&lspr[NOLIGHT],     // SPR_GOOP
	&lspr[SUPERSPARK_L],     // SPR_BPLD
	&lspr[REDBALL_L],     // SPR_ALRM
	&lspr[NOLIGHT],     // SPR_RDTV
	&lspr[REDSHIELD_L],     // SPR_RORB
	&lspr[NOLIGHT],     // SPR_EGGB
	&lspr[SMALLREDBALL_L], // SPR_SFLM
	&lspr[NOLIGHT], // SPR_TNKA
	&lspr[NOLIGHT], // SPR_TNKB
	&lspr[NOLIGHT], // SPR_SPNK
	&lspr[SUPERSPARK_L], // SPR_TFOG
	&lspr[NOLIGHT], // SPR_EEGG
	&lspr[STREETLIGHT_L], // SPR_LITE
	&lspr[NOLIGHT], // SPR_TRET
	&lspr[SMALLREDBALL_L], // SPR_TRLS
	&lspr[NOLIGHT], // SPR_FWR4
	&lspr[NOLIGHT], // SPR_GOOM
	&lspr[NOLIGHT], // SPR_BGOM
	&lspr[NOLIGHT], // SPR_MUS1
	&lspr[NOLIGHT], // SPR_MUS2
	&lspr[NOLIGHT], // SPR_TOAD
	&lspr[NOLIGHT], // SPR_COIN
	&lspr[NOLIGHT], // SPR_CPRK
	&lspr[NOLIGHT], // SPR_XMS1
	&lspr[NOLIGHT], // SPR_XMS2
	&lspr[NOLIGHT], // SPR_XMS3
	&lspr[NOLIGHT], // SPR_CAPS
	&lspr[SUPERSONIC_L], // SPR_SUPT
	&lspr[NOLIGHT], // SPR_ROID
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
	&lspr[NOLIGHT],
};
 
 //=============================================================================
 //                                                                       EXTERN
 //=============================================================================
 
 extern  float   gr_viewludsin;
 extern  float   gr_viewludcos;


//=============================================================================
//                                                                       EXTERN
//=============================================================================

extern  float   gr_viewludsin;
extern  float   gr_viewludcos;


//=============================================================================
//                                                                       PROTOS
//=============================================================================

static void  HWR_SetLight( void );

void HWR_SuperSonicLightToggle(boolean super)
{
	if(super)
		t_lspr[SPR_PLAY] = &lspr[SUPERSONIC_L];
	else
		t_lspr[SPR_PLAY] = &lspr[NOLIGHT];

}

void CV_grMonsterDL_OnChange (void)
{
    if (cv_grmblighting.value)
    {
//        t_lspr[SPR_BAL1] = &lspr[REDBALL_L];
//        t_lspr[SPR_BAL2] = &lspr[REDBALL_L];
//        t_lspr[SPR_MANF] = &lspr[ROCKET2_L];
//        t_lspr[SPR_BAL7] = &lspr[GREENBALL_L];
//        t_lspr[SPR_APLS] = &lspr[GREENBALL_L];
//        t_lspr[SPR_APBX] = &lspr[GREENBALL_L];
//        t_lspr[SPR_SKUL] = &lspr[REDBALL_L]; // Tails
//        t_lspr[SPR_FATB] = &lspr[REDBALL_L];
    }
    else
    {
//        t_lspr[SPR_BAL1] = &lspr[NOLIGHT];
//        t_lspr[SPR_BAL2] = &lspr[NOLIGHT];
//        t_lspr[SPR_MANF] = &lspr[NOLIGHT];
//        t_lspr[SPR_BAL7] = &lspr[NOLIGHT];
//        t_lspr[SPR_APLS] = &lspr[NOLIGHT];
//        t_lspr[SPR_APBX] = &lspr[NOLIGHT];
//        t_lspr[SPR_SKUL] = &lspr[NOLIGHT]; // Tails
//        t_lspr[SPR_FATB] = &lspr[NOLIGHT];
    }
}

// --------------------------------------------------------------------------
// calcul la projection d'un point sur une droite (determinée par deux 
// points) et ensuite calcul la distance (au carré) de ce point au point
// projecté sur cette droite
// --------------------------------------------------------------------------
static float HWR_DistP2D(FOutVector *p1, FOutVector *p2, FVector *p3, FVector *inter)
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

// check if sphere (radius r) centred in p3 touch the bounding box defined by p1, p2
static boolean SphereTouchBBox3D(FOutVector *p1, FOutVector *p2, FVector *p3, float r)
{
    float minx=p1->x,maxx=p2->x,miny=p2->y,maxy=p1->y,minz=p2->z,maxz=p1->z;

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
    if( minz>maxz )
    {
        minz=maxz;
        maxz=p2->z;
    }

    if( minx-r > p3->x ) return false;
    if( maxx+r < p3->x ) return false;
    if( miny-r > p3->y ) return false;
    if( maxy+r < p3->y ) return false;
    if( minz-r > p3->z ) return false;
    if( maxz+r < p3->z ) return false;
    return true;
}

// Hurdler: The old code was removed by me because I don't think it will be used one day.
//          (It's still available on the CVS for educational purpose: Revision 1.8)

// --------------------------------------------------------------------------
// calcul du dynamic lighting sur les murs
// lVerts contient les coords du mur sans le mlook (up/down)
// --------------------------------------------------------------------------
void HWR_WallLighting(FOutVector *wlVerts)
{
    int             i, j;

    // dynlights->nb == 0 if cv_grdynamiclighting.value is not set
    for (j=0; j<dynlights->nb; j++) {
        FVector         inter;
        FSurfaceInfo    Surf;
        float           dist_p2d, d[4], s;

        // check bounding box first
        if( SphereTouchBBox3D(&wlVerts[2], &wlVerts[0], &LIGHT_POS(j), DL_RADIUS(j))==false )
            continue;
        d[0] = wlVerts[2].x - wlVerts[0].x;
        d[1] = wlVerts[2].z - wlVerts[0].z;
        d[2] = LIGHT_POS(j).x - wlVerts[0].x;
        d[3] = LIGHT_POS(j).z - wlVerts[0].z;
        // backface cull
        if( d[2]*d[1] - d[3]*d[0] < 0 )
            continue;
        // check exact distance
        dist_p2d = HWR_DistP2D(&wlVerts[2], &wlVerts[0],  &LIGHT_POS(j), &inter);
        if (dist_p2d >= DL_SQRRADIUS(j))
            continue;

        d[0] = sqrt((wlVerts[0].x-inter.x)*(wlVerts[0].x-inter.x)+(wlVerts[0].z-inter.z)*(wlVerts[0].z-inter.z));
        d[1] = sqrt((wlVerts[2].x-inter.x)*(wlVerts[2].x-inter.x)+(wlVerts[2].z-inter.z)*(wlVerts[2].z-inter.z));
        //dAB = sqrt((wlVerts[0].x-wlVerts[2].x)*(wlVerts[0].x-wlVerts[2].x)+(wlVerts[0].z-wlVerts[2].z)*(wlVerts[0].z-wlVerts[2].z));
        //if ( (d[0] < dAB) && (d[1] < dAB) ) // test if the intersection is on the wall
        //{
        //    d[0] = -d[0]; // if yes, the left distcance must be negative for texcoord
        //}
        // test if the intersection is on the wall
        if ( (wlVerts[0].x<inter.x && wlVerts[2].x>inter.x) ||
             (wlVerts[0].x>inter.x && wlVerts[2].x<inter.x) ||
             (wlVerts[0].z<inter.z && wlVerts[2].z>inter.z) ||
             (wlVerts[0].z>inter.z && wlVerts[2].z<inter.z) )
        {
            d[0] = -d[0]; // if yes, the left distcance must be negative for texcoord
        }
        d[2] = d[1]; d[3] = d[0];
#ifdef DL_HIGH_QUALITY
        s = 0.5 / DL_RADIUS(j);
#else
        s = 0.5 / sqrt(DL_SQRRADIUS(j)-dist_p2d);
#endif
        for (i=0; i<4; i++) {
            wlVerts[i].sow = 0.5 + d[i]*s;
            wlVerts[i].tow = 0.5 + (wlVerts[i].y-LIGHT_POS(j).y)*s*1.2f;
        }

        HWR_SetLight();

        Surf.FlatColor.rgba = LONG(dynlights->p_lspr[j]->dynamic_color);
#ifdef DL_HIGH_QUALITY
        Surf.FlatColor.s.alpha *= (1-dist_p2d/DL_SQRRADIUS(j));
#endif
        if( !dynlights->mo[j]->state )
            return;
        // next state is null so fade out with alpha
        if( (dynlights->mo[j]->state->nextstate == S_NULL) || (dynlights->mo[j]->state->nextstate == S_DISS)) // Included S_DISS Tails 09-09-2002
            Surf.FlatColor.s.alpha *= (float)dynlights->mo[j]->tics/(float)dynlights->mo[j]->state->tics;

        HWD.pfnDrawPolygon ( &Surf, wlVerts, 4, LIGHTMAPFLAGS );

    } // end for (j=0; j<dynlights->nb; j++)
}


// BP: big hack for a test in lighting ref:1249753487AB
extern int *bbox;
extern FTransform atransform;
// --------------------------------------------------------------------------
// calcul du dynamic lighting sur le sol
// clVerts contient les coords du sol avec le mlook (up/down)
// --------------------------------------------------------------------------
void HWR_PlaneLighting(FOutVector *clVerts, int nrClipVerts)
{
    int     i, j;
    FOutVector p1,p2;

    p1.z=bbox[BOXTOP]*crapmul;
    p1.x=bbox[BOXLEFT]*crapmul;
    p2.z=bbox[BOXBOTTOM]*crapmul;
    p2.x=bbox[BOXRIGHT]*crapmul;
    p2.y=clVerts[0].y;
    p1.y=clVerts[0].y;

    for (j=0; j<dynlights->nb; j++) {
        FSurfaceInfo    Surf;
        float           dist_p2d, s;

        // BP: The kickass Optimization: check if light touch bounding box
        if( SphereTouchBBox3D(&p1, &p2, &dynlights->position[j], DL_RADIUS(j))==false )
            continue;
        // backface cull
        //Hurdler: doesn't work with new TANDL code
        if( (clVerts[0].y > atransform.z)       // true mean it is a ceiling false is a floor
             ^ (LIGHT_POS(j).y < clVerts[0].y) ) // true mean light is down plane false light is up plane
             continue;
        dist_p2d = (clVerts[0].y-LIGHT_POS(j).y);
        dist_p2d *= dist_p2d;
        // done in SphereTouchBBox3D
        //if (dist_p2d >= DL_SQRRADIUS(j))
        //    continue;
        
#ifdef DL_HIGH_QUALITY
        s = 0.5f / DL_RADIUS(j);
#else
        s = 0.5f / sqrt(DL_SQRRADIUS(j)-dist_p2d);
#endif
        for (i=0; i<nrClipVerts; i++) {
            clVerts[i].sow = 0.5f + (clVerts[i].x-LIGHT_POS(j).x)*s;
            clVerts[i].tow = 0.5f + (clVerts[i].z-LIGHT_POS(j).z)*s*1.2f;
        }

        HWR_SetLight();

        Surf.FlatColor.rgba = LONG(dynlights->p_lspr[j]->dynamic_color);
#ifdef DL_HIGH_QUALITY
        Surf.FlatColor.s.alpha *= (1-dist_p2d/DL_SQRRADIUS(j));
#endif
        if( !dynlights->mo[j]->state )
            return;
        // next state is null so fade out with alpha
        if( (dynlights->mo[j]->state->nextstate == S_NULL) )
            Surf.FlatColor.s.alpha *= (float)dynlights->mo[j]->tics/(float)dynlights->mo[j]->state->tics;

        HWD.pfnDrawPolygon ( &Surf, clVerts, nrClipVerts, LIGHTMAPFLAGS );

    } // end for (j=0; j<dynlights->nb; j++)
}


static int coronalumpnum;
#ifndef NEWCORONAS
// --------------------------------------------------------------------------
// coronas lighting
// --------------------------------------------------------------------------
void HWR_DoCoronasLighting(FOutVector *outVerts, gr_vissprite_t *spr) 
{
    light_t   *p_lspr;

    //CONS_Printf("sprite (type): %d (%s)\n", spr->type, sprnames[spr->type]);
    p_lspr = t_lspr[spr->mobj->sprite];
    if( (spr->mobj->state>=&states[S_EXPLODE1] && spr->mobj->state<=&states[S_EXPLODE3])
     || (spr->mobj->state>=&states[S_FATSHOTX1] && spr->mobj->state<=&states[S_FATSHOTX3]))
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
                size  = p_lspr->corona_radius  * ((outVerts[0].z+120.0f)/950.0f); // d'ou vienne ces constante ?
                break;
            case ROCKET_SPR: 
                p_lspr->corona_color = (((M_Random()>>1)&0xff)<<24)|0x0040ff;
                // don't need a break
            case CORONA_SPR: 
                size  = p_lspr->corona_radius  * ((outVerts[0].z+60.0f)/100.0f); // d'ou vienne ces constante ?
                break;
            default:
                I_Error("HWR_DoCoronasLighting: unknow light type %d",p_lspr->type);
                return;
        }
        if (size > p_lspr->corona_radius) 
            size = p_lspr->corona_radius;
        size *= FIXED_TO_FLOAT(cv_grcoronasize.value<<1);

        // compute position doing average
        for (i=0; i<4; i++) {
            cx += outVerts[i].x;
            cy += outVerts[i].y;
            cz += outVerts[i].z;
        }
        cx /= 4.0f;  cy /= 4.0f;  cz /= 4.0f;

        // more realistique corona !
        if( cz>=255*8+250 )
            return;
        Surf.FlatColor.rgba = p_lspr->corona_color;
        if( cz>250.0f )
            Surf.FlatColor.s.alpha = 0xff-((int)cz-250)/8;
        else
            Surf.FlatColor.s.alpha = 0xff;

        // do not be hide by sprite of the light itself !
        cz = cz - 2.0f; 

        // Bp; je comprend pas, ou est la rotation haut/bas ?
        //     tu ajoute un offset a y mais si la tu la reguarde de haut 
        //     sa devrais pas marcher ... comprend pas :(
        //     (...) bon je croit que j'ai comprit il est tout pourit le code ?
        //           car comme l'offset est minime sa ce voit pas !
        light[0].x = cx-size;  light[0].z = cz;
        light[0].y = cy-size*1.33f+p_lspr->light_yoffset; 
        light[0].sow = 0.0f;   light[0].tow = 0.0f;

        light[1].x = cx+size;  light[1].z = cz;
        light[1].y = cy-size*1.33f+p_lspr->light_yoffset; 
        light[1].sow = 1.0f;   light[1].tow = 0.0f;

        light[2].x = cx+size;  light[2].z = cz;
        light[2].y = cy+size*1.33f+p_lspr->light_yoffset; 
        light[2].sow = 1.0f;   light[2].tow = 1.0f;

        light[3].x = cx-size;  light[3].z = cz;
        light[3].y = cy+size*1.33f+p_lspr->light_yoffset; 
        light[3].sow = 0.0f;   light[3].tow = 1.0f;

        HWR_GetPic(coronalumpnum);  // TODO: use different coronas

        HWD.pfnDrawPolygon ( &Surf, light, 4, PF_Modulated | PF_Additive | PF_Clip | PF_Corona | PF_NoDepthTest);
    }
}
#endif

#ifdef NEWCORONAS
void transform(float *cx, float *cy, float *cz);
// use the lightlist of the frame to draw the coronas at the top of everythink
void HWR_DrawCoronas( void )
{
    int       j;

    if( !cv_grcoronas.value || dynlights->nb<=0)
        return;
    
    HWR_GetPic(coronalumpnum);  // TODO: use different coronas
    for( j=0;j<dynlights->nb;j++ )
    {
        FOutVector      light[4];
        FSurfaceInfo    Surf;
        float           cx=LIGHT_POS(j).x;
        float           cy=LIGHT_POS(j).y;
        float           cz=LIGHT_POS(j).z; // gravity center
        float           size;
        light_t         *p_lspr = dynlights->p_lspr[j];
        
        // it's an object which emits light
        if ( !(p_lspr->type & CORONA_SPR) )
            continue;

        transform(&cx,&cy,&cz);

        // more realistique corona !
        if( cz>=255*8+250 )
            continue;
        Surf.FlatColor.rgba = p_lspr->corona_color;
        if( cz>250.0f )
            Surf.FlatColor.s.alpha = 0xff-((int)cz-250)/8;
        else
            Surf.FlatColor.s.alpha = 0xff;

        switch (p_lspr->type)
        {
            case LIGHT_SPR:
                size  = p_lspr->corona_radius  * ((cz+120.0f)/950.0f); // d'ou vienne ces constante ?
                break;
            case ROCKET_SPR:
                Surf.FlatColor.s.alpha = (M_Random()>>1)&0xff;
                // don't need a break
            case CORONA_SPR:
                size  = p_lspr->corona_radius  * ((cz+60.0f)/100.0f); // d'ou vienne ces constante ?
                break;
            default:
                I_Error("HWR_DoCoronasLighting: unknow light type %d",p_lspr->type);
                continue;
        }
        if (size > p_lspr->corona_radius)
            size = p_lspr->corona_radius;
        size *= FIXED_TO_FLOAT(cv_grcoronasize.value<<1);

        // put light little forward the sprite so there is no 
        // z-buffer problem (coplanar polygons)
        // BP: use PF_Decal do not help :(
        cz = cz - 5.0f; 

        light[0].x = cx-size;  light[0].z = cz;
        light[0].y = cy-size*1.33f;
        light[0].sow = 0.0f;   light[0].tow = 0.0f;

        light[1].x = cx+size;  light[1].z = cz;
        light[1].y = cy-size*1.33f;
        light[1].sow = 1.0f;   light[1].tow = 0.0f;

        light[2].x = cx+size;  light[2].z = cz;
        light[2].y = cy+size*1.33f;
        light[2].sow = 1.0f;   light[2].tow = 1.0f;

        light[3].x = cx-size;  light[3].z = cz;
        light[3].y = cy+size*1.33f;
        light[3].sow = 0.0f;   light[3].tow = 1.0f;

        HWD.pfnDrawPolygon ( &Surf, light, 4, PF_Modulated | PF_Additive | PF_Clip | PF_NoDepthTest | PF_Corona );
    }
}
#endif

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
#ifdef PARANOIA
    if(!spr->mobj)
        I_Error("vissprite without mobj !!!");
#endif
    // check if sprite contain dynamic light
    p_lspr = t_lspr[spr->mobj->sprite];
    if ( (p_lspr->type&DYNLIGHT_SPR) 
         && ((p_lspr->type!=LIGHT_SPR) || cv_grstaticlighting.value) 
         && (dynlights->nb < DL_MAX_LIGHT)

		 && spr->mobj->state )
    {
        LIGHT_POS(dynlights->nb).x = spr->mobj->x*crapmul;
        LIGHT_POS(dynlights->nb).y = spr->mobj->z*crapmul+FIXED_TO_FLOAT(spr->mobj->height>>1)+p_lspr->light_yoffset;
        LIGHT_POS(dynlights->nb).z = spr->mobj->y*crapmul;

        dynlights->mo[dynlights->nb] = spr->mobj;

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
    coronalumpnum = W_GetNumForName("corona");
}

// -----------------+
// HWR_SetLight     : Download a disc shaped alpha map for rendering fake lights
// -----------------+
void HWR_SetLight( void )
{
    int    i, j;

    if (!lightmappatch.mipmap.downloaded && !lightmappatch.mipmap.grInfo.data)
    {

        USHORT *Data = Z_Malloc( 128*128*sizeof(USHORT), PU_HWRCACHE, &lightmappatch.mipmap.grInfo.data );
                
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
        lightmappatch.mipmap.flags = 0; //TF_WRAPXY; // DEBUG: view the overdraw !
    }
    HWD.pfnSetTexture( &lightmappatch.mipmap );
}


void HWR_DynamicShadowing(FOutVector *clVerts, int nrClipVerts, player_t *p)
{
    int  i;
    FSurfaceInfo    Surf;

    if (!cv_grdynamiclighting.value)
        return;

    for (i=0; i<nrClipVerts; i++) {
        clVerts[i].sow = 0.5f + clVerts[i].x*0.01;
        clVerts[i].tow = 0.5f + clVerts[i].z*0.01*1.2f;
    }
    
    HWR_SetLight();

    Surf.FlatColor.rgba = 0x70707070;

    HWD.pfnDrawPolygon ( &Surf, clVerts, nrClipVerts, LIGHTMAPFLAGS );
    //HWD.pfnDrawPolygon ( &Surf, clVerts, nrClipVerts, PF_Modulated|PF_Environment|PF_Clip );
}


//**********************************************************
// Hurdler: new code for faster static lighting and and T&L
//**********************************************************

// est ce bien necessaire ?
//static sector_t *gr_frontsector;
static sector_t *gr_backsector ;
static seg_t    *gr_curline;


/*
static void HWR_StoreWallRange (int startfrac, int endfrac)
{
...(voir hw_main.c)...
}
*/


// p1 et p2 c'est le deux bou du seg en float
void HWR_BuildWallLightmaps(FVector *p1, FVector *p2, int lighnum, seg_t *line)
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
    FVector             p1,p2;
    
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
//    if( CircleTouchBBox(&p1, &p2, &LIGHT_POS(lightnum), DL_RADIUS(lightnum))==false )    
//        return;

    HWR_BuildWallLightmaps(&p1, &p2, lightnum, line);
}



//TODO: see what HWR_AddLine does
static void HWR_CheckSubsector( int num, fixed_t *bbox )
{
    int         count;
    seg_t       *line;
    subsector_t *sub;
    FVector     p1,p2;
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
//            if( CircleTouchBBox(&p1, &p2, &LIGHT_POS(lightnum), DL_RADIUS(lightnum))==false )
//                continue;

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
    return; //Hurdler: TODO!
    CONS_Printf("HWR_CreateStaticLightmaps\n");

    dynlights->nb = 0;

    // First: Searching for lights
    // BP: if i was you, I will make it in create mobj since mobj can be create 
    //     at runtime now with fragle scipt
    HWR_SearchLightsInMobjs();
    CONS_Printf("%d lights found\n", dynlights->nb);

    // Second: Build all lightmap for walls covered by lights
    validcount++; // to be sure
    HWR_ComputeLightMapsInBSPNode(bspnum, NULL);

    dynlights->nb = 0;
}

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
