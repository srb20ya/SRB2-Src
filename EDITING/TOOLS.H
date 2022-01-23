// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: tools.h,v 1.2 2000/05/02 11:58:51 hurdler Exp $
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
// $Log: tools.h,v $
// Revision 1.2  2000/05/02 11:58:51  hurdler
// Updated License (GPL)
//
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "lmptools.h"

// LAUNCH DOOM LEGACY !
// pass lmpextrainfo struct for playing back a demo, this will push only the
//  required arguments on the cmdline to playback the demo
void    Launch (CWnd* Parent, lmpextrainfo_t* lmp, BOOL bMultiplayer, BOOL nomap );

char*   FilePath (char* pathname);
char*   FileBase (char* pathname, BOOL extension = true);
void    DefaultExtension (char *path, char *extension);
void    AddSubDir (char* path, char* subdir);
void    BackslashPath (char* path);
char*   MakeFileName (char* path, char* file, char* extension);

// temporary varargs printf
char*	va(char *format, ...);
