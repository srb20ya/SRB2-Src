// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: win_dbg.h,v 1.2 2000/02/27 00:42:12 hurdler Exp $
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
// $Log: win_dbg.h,v $
// Revision 1.2  2000/02/27 00:42:12  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//-----------------------------------------------------------------------------
/// \file
/// \brief exception handler

//#define WIN32_LEAN_AND_MEAN
#define RPC_NO_WINDOWS_H
#include <windows.h>

// called in the exception filter of the __try block, writes all useful debugging information
// to a file, using only win32 functions in case the C runtime is in a bad state.
int __cdecl RecordExceptionInfo (PEXCEPTION_POINTERS data/*, LPCSTR Message, LPSTR lpCmdLine*/);

#ifdef __MINGW32__

#include <excpt.h>

#ifndef TRYLEVEL_NONE

#define NO_SEH_MINGW //Alam:?
struct _EXCEPTION_POINTERS *GetExceptionInformation(VOID);

//Alam_GBC: use __try1( seh )
#ifndef __try
#define __try
#endif //__try

#undef NO_SEH_MINGW //Alam: win_dbg's code not working with MINGW
//Alam_GBC: use __except1
#ifndef __except
#define __except(x) if(0)
#endif //__except

#endif // !__TRYLEVEL_NONE

#endif // __MINGW32__
