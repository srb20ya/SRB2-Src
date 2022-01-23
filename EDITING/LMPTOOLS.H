// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: lmptools.h,v 1.2 2000/05/02 11:58:51 hurdler Exp $
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
// $Log: lmptools.h,v $
// Revision 1.2  2000/05/02 11:58:51  hurdler
// Updated License (GPL)
//
//
//
// DESCRIPTION:
//      LMP file management routines
//
//-----------------------------------------------------------------------------

#ifndef _LMPTOOLS_H_
#define _LMPTOOLS_H_

#include <afxwin.h>

// max size in chars of extra components that may be added to an LMP
//#define LMP_MAX_EXE     12      //filename.ext
//#define LMP_MAX_IWAD    12      //filename.ext
//#define LMP_MAX_PWADS   

typedef struct {
    CWnd* cwnd;          //dialog where info is shown
    //
    int   nQuickInfoID;  //quickinfo box or 0
    //
    int   nFileID;       //filename box or 0
    int   nExeID;        //exe name box or 0
    int   nWadFilesID;   //wad files box or 0
    int   nCommentsID;   //comments box or 0
} lmpshowinfo_t;


// lmpextrainfo flags
#define   lmpplaywindowed   1


// this structure is:
// - passed to LMP_WriteExtraInfo(), to save extra info into an LMP
//   fields that are not defined must be NULL
// - returned by LMP_ReadExtraInfo(), to read extra info from an LMP
//   fields that were not found in LMP are set to NULL
typedef struct {
    char*   demofile;    // LMP file name
    char*   exe;
    char*   iwad;
    char*   pwads;
    char*   comments;
    int     flags;
} lmpextrainfo_t;

typedef struct {
    long  chunkID;
    long  chunksize;
} lmpextrachunk_t;

BOOL    LMP_ReadInfo (char* demofile, lmpshowinfo_t* lmpinfo);

void            LMP_FreeExtraInfo (lmpextrainfo_t* extra);
lmpextrainfo_t* LMP_ReadExtraInfo (char* demofile, int handle = -1);
BOOL            LMP_WriteExtraInfo (lmpextrainfo_t* extra, char* demofile);

#endif //_LMPTOOLS_H_
