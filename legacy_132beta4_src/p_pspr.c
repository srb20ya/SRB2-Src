// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_pspr.c,v 1.10 2001/08/02 19:15:59 bpereira Exp $
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
// $Log: p_pspr.c,v $
// Revision 1.10  2001/08/02 19:15:59  bpereira
// fix player reset in secret level of doom2
//
// Revision 1.9  2001/06/10 21:16:01  bpereira
// no message
//
// Revision 1.8  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.7  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.6  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.5  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.4  2000/10/01 10:18:18  bpereira
// no message
//
// Revision 1.3  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Weapon sprite animation, weapon objects.
//      Action functions for weapons.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "d_event.h"
#include "p_local.h"
#include "p_pspr.h"
#include "p_inter.h"
#include "s_sound.h"
#include "g_game.h"
#include "g_input.h"
#include "r_main.h"
#include "m_random.h"
#include "p_inter.h"

#include "hardware/hw3sound.h"

#define WEAPONBOTTOM            128*FRACUNIT
#define WEAPONTOP               32*FRACUNIT

//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//
fixed_t         bulletslope;

//added:16-02-98: Fab comments: autoaim for the bullet-type weapons
void P_BulletSlope (mobj_t* mo)
{
    angle_t     an;

    //added:18-02-98: if AUTOAIM, try to aim at something
    if(!mo->player->autoaim_toggle || !cv_allowautoaim.value)
        goto notagetfound;

    // see which target is to be aimed at
    an = mo->angle;
    bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);

    if (!linetarget)
    {
        an += 1<<26;
        bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
        if (!linetarget)
        {
            an -= 2<<26;
            bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
        }
        if(!linetarget)
        {
notagetfound:
            bulletslope = AIMINGTOSLOPE(mo->player->aiming);
        }
    }
}


//
// P_GunShot
//
//added:16-02-98: used only for player (pistol,shotgun,chaingun)
//                supershotgun use p_lineattack directely
void P_GunShot ( mobj_t*       mo,
                 boolean       accurate )
{
    angle_t     angle;
    int         damage;

    damage = 5*(P_Random ()%3+1);
    angle = mo->angle;

    if (!accurate)
    {
        angle += (P_Random()<<18); // WARNING: don't put this in one line 
        angle -= (P_Random()<<18); // else this expretion is ambiguous (evaluation order not diffined)
    }

    P_LineAttack (mo, angle, MISSILERANGE, bulletslope, damage);
}

#include "p_hpspr.c" // FIXTHIS: this sucks