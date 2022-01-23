// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: byteptr.h,v 1.5 2000/10/21 08:43:28 bpereira Exp $
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
// $Log: byteptr.h,v $
// Revision 1.5  2000/10/21 08:43:28  bpereira
// no message
//
// Revision 1.4  2000/04/16 18:38:06  bpereira
// no message
//
//
// DESCRIPTION:
//    Macro to read/write from/to a char*, used for packet cration and such...
//
//-----------------------------------------------------------------------------


#define WRITEBYTE(p,b)      *((byte   *)p)++ = b
#define WRITECHAR(p,b)      *((char   *)p)++ = b
#define WRITESHORT(p,b)     *((short  *)p)++ = b
#define WRITEUSHORT(p,b)    *((USHORT *)p)++ = b
#define WRITELONG(p,b)      *((long   *)p)++ = b
#define WRITEULONG(p,b)     *((ULONG  *)p)++ = b
#define WRITEFIXED(p,b)     *((fixed_t*)p)++ = b
#define WRITEANGLE(p,b)     *((angle_t*)p)++ = b
#define WRITESTRING(p,b)    { int tmp_i=0; do { WRITECHAR(p,b[tmp_i]); } while(b[tmp_i++]); }
#define WRITESTRINGN(p,b,n) { int tmp_i=0; do { WRITECHAR(p,b[tmp_i]); if(!b[tmp_i]) break;tmp_i++; } while(tmp_i<n); }
#define WRITEMEM(p,s,n)     memcpy(p, s, n);p+=n

#define READBYTE(p)         *((byte   *)p)++
#define READCHAR(p)         *((char   *)p)++
#define READSHORT(p)        *((short  *)p)++
#define READUSHORT(p)       *((USHORT *)p)++
#define READLONG(p)         *((long   *)p)++
#define READULONG(p)        *((ULONG  *)p)++
#define READFIXED(p)        *((fixed_t*)p)++
#define READANGLE(p)        *((angle_t*)p)++
#define READSTRING(p,s)     { int tmp_i=0; do { s[tmp_i]=READBYTE(p);  } while(s[tmp_i++]); }
#define SKIPSTRING(p)       while(READBYTE(p))
#define READMEM(p,s,n)      memcpy(s, p, n);p+=n
