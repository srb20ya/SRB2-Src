// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_saveg.h,v 1.2 2000/02/27 00:42:10 hurdler Exp $
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
// $Log: p_saveg.h,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Savegame I/O, archiving, persistence.
//
//-----------------------------------------------------------------------------


#ifndef __P_SAVEG__
#define __P_SAVEG__


#ifdef __GNUG__
#pragma interface
#endif


// Persistent storage/archiving.
// These are the load / save game routines.
void P_ArchivePlayers (void);
void P_UnArchivePlayers (void);
void P_ArchiveWorld (void);
void P_UnArchiveWorld (void);
void P_ArchiveThinkers (void);
void P_UnArchiveThinkers (void);
void P_ArchiveSpecials (void);
void P_UnArchiveSpecials (void);

extern byte*            save_p; 


#endif
