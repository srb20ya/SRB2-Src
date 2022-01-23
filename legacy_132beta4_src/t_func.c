// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------------
//
// $Id: t_func.c,v 1.13 2002/01/05 00:58:10 hurdler Exp $
//
// Copyright(C) 2000 Simon Howard
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// $Log: t_func.c,v $
// Revision 1.13  2002/01/05 00:58:10  hurdler
// fix compiling problem when not using hwrender
//
// Revision 1.12  2001/12/31 14:44:50  hurdler
// Last fix for beta 4
//
// Revision 1.11  2001/12/31 13:47:46  hurdler
// Add setcorona FS command and prepare the code for beta 4
//
// Revision 1.10  2001/12/28 16:57:30  hurdler
// Add setcorona command to FS
//
// Revision 1.9  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.8  2001/08/14 00:36:26  hurdler
// Small update
//
// Revision 1.7  2001/08/06 23:57:10  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.6  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.5  2001/03/21 18:24:56  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.4  2001/03/13 22:14:20  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.3  2000/11/09 17:56:20  stroggonmeth
// Hopefully fixed a few bugs and did a few optimizations.
//
// Revision 1.2  2000/11/04 16:23:44  bpereira
// no message
//
// Revision 1.1  2000/11/02 17:57:28  stroggonmeth
// FraggleScript files...
//
//
//--------------------------------------------------------------------------
//
// Functions
//
// functions are stored as variables(see variable.c), the
// value being a pointer to a 'handler' function for the
// function. Arguments are stored in an argc/argv-style list
//
// this module contains all the handler functions for the
// basic FraggleScript Functions.
//
// By Simon Howard
//
//---------------------------------------------------------------------------
