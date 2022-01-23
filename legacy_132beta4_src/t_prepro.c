// Emacs style mode select -*- C++ -*-
//----------------------------------------------------------------------------
//
// $Id: t_prepro.c,v 1.1 2000/11/02 17:57:28 stroggonmeth Exp $
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
// $Log: t_prepro.c,v $
// Revision 1.1  2000/11/02 17:57:28  stroggonmeth
// FraggleScript files...
//
//
//--------------------------------------------------------------------------
//
// Preprocessor.
//
// The preprocessor must be called when the script is first loaded.
// It performs 2 functions:
//      1: blank out comments (which could be misinterpreted)
//      2: makes a list of all the sections held within {} braces
//      3: 'dry' runs the script: goes thru each statement and
//         sets the types of all the section_t's in the script
//      4: Saves locations of all goto() labels
//
// the system of section_t's is pretty horrible really, but it works
// and its probably the only way i can think of of saving scripts
// half-way thru running
//
// By Simon Howard
//
//---------------------------------------------------------------------------
