// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: d_ticcmd.h,v 1.2 2000/02/27 00:42:10 hurdler Exp $
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
// $Log: d_ticcmd.h,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      System specific interface stuff.
//
//-----------------------------------------------------------------------------


#ifndef __D_TICCMD__
#define __D_TICCMD__

#include "m_fixed.h"
#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

// The data sampled per tick (single player)
// and transmitted to other peers (multiplayer).
// Mainly movements/button commands per game tick,
// plus a checksum for internal state consistency.

// bits in angleturn
#define TICCMD_RECEIVED 1      
#define TICCMD_XY     2
typedef struct
{
#ifdef CLIENTPREDICTION2
    fixed_t      x;
    fixed_t      y;
#endif
    char         forwardmove;    // *2048 for move
    char         sidemove;       // *2048 for move
    short        angleturn;      // <<16 for angle delta
                                 // SAVED AS A BYTE into demos
    signed short aiming;    //added:16-02-98:mouse aiming, see G_BuildTicCmd
    byte         buttons;
#ifdef CLIENTPREDICTION
    unsigned short   localtic; //added:6-12-98: for client prediction
#endif
} ticcmd_t;


#endif
