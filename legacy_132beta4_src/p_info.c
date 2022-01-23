// Emacs style mode select -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: p_info.c,v 1.15 2001/12/26 17:24:46 hurdler Exp $
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
// $Log: p_info.c,v $
// Revision 1.15  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.14  2001/08/14 00:36:26  hurdler
// Small update
//
// Revision 1.13  2001/08/13 22:53:39  stroggonmeth
// Small commit
//
// Revision 1.12  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.11  2001/08/02 19:15:59  bpereira
// fix player reset in secret level of doom2
//
// Revision 1.10  2001/06/30 15:06:01  bpereira
// fixed wrong next level name in intermission
//
// Revision 1.9  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.8  2001/05/07 20:27:16  stroggonmeth
// no message
//
// Revision 1.7  2001/03/21 18:24:38  stroggonmeth
// Misc changes and fixes. Code cleanup
//
// Revision 1.6  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.5  2000/11/11 13:59:45  bpereira
// no message
//
// Revision 1.4  2000/11/09 17:56:20  stroggonmeth
// Hopefully fixed a few bugs and did a few optimizations.
//
// Revision 1.3  2000/11/06 20:52:16  bpereira
// no message
//
// Revision 1.2  2000/11/03 11:48:39  hurdler
// Fix compiling problem under win32 with 3D-Floors and FragglScript (to verify!)
//
// Revision 1.1  2000/11/03 02:00:44  stroggonmeth
// Added p_info.c and p_info.h
//
//
//--------------------------------------------------------------------------
//
// Level info.
//
// Under smmu, level info is stored in the level marker: ie. "mapxx"
// or "exmx" lump. This contains new info such as: the level name, music
// lump to be played, par time etc.
//
// By Simon Howard
//
//-----------------------------------------------------------------------------