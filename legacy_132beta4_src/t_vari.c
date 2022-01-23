// Emacs style mode select -*- C++ -*-
//----------------------------------------------------------------------------
//
// $Id: t_vari.c,v 1.1 2000/11/02 17:57:28 stroggonmeth Exp $
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
// $Log: t_vari.c,v $
// Revision 1.1  2000/11/02 17:57:28  stroggonmeth
// FraggleScript files...
//
//
//--------------------------------------------------------------------------
//
// Variables.
//
// Variable code: create new variables, look up variables, get value,
// set value
//
// variables are stored inside the individual scripts, to allow for
// 'local' and 'global' variables. This way, individual scripts cannot
// access variables in other scripts. However, 'global' variables can
// be made which can be accessed by all scripts. These are stored inside
// a dedicated script_t which exists only to hold all of these global
// variables.
//
// functions are also stored as variables, these are kept in the global
// script so they can be accessed by all scripts. function variables
// cannot be set or changed inside the scripts themselves.
//
//---------------------------------------------------------------------------

/* includes ************************/
