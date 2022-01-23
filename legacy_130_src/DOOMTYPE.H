// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: doomtype.h,v 1.3 2000/08/10 14:53:57 ydario Exp $
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
// $Log: doomtype.h,v $
// Revision 1.3  2000/08/10 14:53:57  ydario
// OS/2 port
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      doom games standard types
//      Simple basic typedefs, isolated here to make it easier
//      separating modules.
//    
//-----------------------------------------------------------------------------


#ifndef __DOOMTYPE__
#define __DOOMTYPE__

#ifdef __WIN32__
#include <windows.h>
#endif
#ifndef _OS2EMX_H
typedef unsigned long ULONG;
typedef unsigned short USHORT;
#endif // _OS2EMX_H

#ifdef __WIN32__
#define INT64  __int64
#else
#define INT64  long long
#endif

#if defined( __MSC__) || defined( __OS2__)
    // Microsoft VisualC++
    #define strncasecmp             strnicmp
    #define strcasecmp              stricmp
    #define inline                  __inline
#else
    #ifdef __WATCOMC__
        #include <dos.h>
        #include <sys\types.h>
        #include <direct.h>
        #include <malloc.h>
        #define strncasecmp             strnicmp
        #define strcasecmp              strcmpi
    #endif
#endif
// added for Linux 19990220 by Kin
#ifdef LINUX
#define stricmp(x,y) strcasecmp(x,y)
#define strnicmp(x,y,n) strncasecmp(x,y,n)
#define lstrlen(x) strlen(x)
#endif


#ifndef __BYTEBOOL__
    #define __BYTEBOOL__

    // Fixed to use builtin bool type with C++.
    //#ifdef __cplusplus
    //    typedef bool boolean;
    //#else

        typedef unsigned char byte;

        //faB: clean that up !!
        #ifdef __WIN32__
            #define false   FALSE           // use windows types
            #define true    TRUE
            #define boolean BOOL
        #else
            typedef enum {false, true} boolean;
        #endif
    //#endif // __cplusplus
#endif // __BYTEBOOL__


// Predefined with some OS.
#ifndef __WIN32__
#include <values.h>
#endif

#ifndef MAXCHAR
#define MAXCHAR   ((char)0x7f)
#endif
#ifndef MAXSHORT
#define MAXSHORT  ((short)0x7fff)
#endif
#ifndef MAXINT
#define MAXINT    ((int)0x7fffffff)
#endif
#ifndef MAXLONG
#define MAXLONG   ((long)0x7fffffff)
#endif

#ifndef MINCHAR
#define MINCHAR   ((char)0x80)
#endif
#ifndef MINSHORT
#define MINSHORT  ((short)0x8000)
#endif
#ifndef MININT
#define MININT    ((int)0x80000000)
#endif
#ifndef MINLONG
#define MINLONG   ((long)0x80000000)
#endif


#endif  //__DOOMTYPE__
