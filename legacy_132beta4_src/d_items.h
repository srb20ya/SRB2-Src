// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_items.h,v 1.3 2001/01/25 22:15:41 bpereira Exp $
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
// $Log: d_items.h,v $
// Revision 1.3  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Items: key cards, artifacts, weapon, ammunition.
//
//-----------------------------------------------------------------------------


#ifndef __D_ITEMS__
#define __D_ITEMS__

#include "doomdef.h"

#ifdef __GNUG__
#pragma interface
#endif


// ==================================
// Difficulty/skill settings/filters.
// ==================================

// Skill flags.
#define MTF_EASY                1
#define MTF_NORMAL              2
#define MTF_HARD                4

// Deaf monsters/do not react to sound.
#define MTF_AMBUSH              8

// "Multi" flag with a few special uses Graue 12-31-2003
#define MTF_MULTI              16

// Power up artifacts.
typedef enum
{
    pw_invulnerability,
    pw_sneakers,
    pw_flashing,
	pw_blueshield, // blue shield Tails 03-04-2000
	pw_redshield, // red shield Tails 12-18-2002
    pw_tailsfly, // tails flying Tails 03-05-2000
    pw_underwater, // underwater timer Tails 03-06-2000
	pw_spacetime, // In space, no one can hear you spin! Tails 06-21-2003
    pw_extralife, // Extra Life timer Tails 03-14-2000
    pw_yellowshield, // yellow shield Tails 03-15-2000
    pw_blackshield, // black shield Tails 04-08-2000
    pw_greenshield, // green shield Tails 04-08-2000
    pw_super, // Are you super? Tails 04-08-2000

	// Mario-specific
	pw_fireflower,

	// New DM Weapons Tails 07-11-2002
	pw_homingring,
	pw_railring,
	pw_shieldring,
	pw_automaticring,
	pw_explosionring,

	// NiGHTS powerups Tails 12-15-2003
	pw_superparaloop,
	pw_nightshelper,

    NUMPOWERS

} powertype_t;

#endif
