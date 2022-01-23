// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: st_lib.c,v 1.6 2001/02/24 13:35:21 bpereira Exp $
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
// $Log: st_lib.c,v $
// Revision 1.6  2001/02/24 13:35:21  bpereira
// no message
//
// Revision 1.5  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.4  2000/10/04 16:19:24  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.3  2000/09/28 20:57:18  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      The status bar widget code.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "st_lib.h"
#include "st_stuff.h"
#include "v_video.h"
#include "z_zone.h"

#include "i_video.h"    //rendermode
//#define DEBUG

