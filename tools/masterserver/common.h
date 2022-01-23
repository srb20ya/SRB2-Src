// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: common.h,v 1.3 2001/03/31 10:17:56 ydario Exp $
//
// Copyright (C) 2000 by DooM Legacy Team.
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
// $Log: common.h,v $
// Revision 1.3  2001/03/31 10:17:56  ydario
// OS/2 compilation fixes
//
// Revision 1.2  2000/08/19 11:31:03  hurdler
// Adding password and log file access tru server
//
// Revision 1.1.1.1  2000/08/18 10:32:09  hurdler
// Initial import of the Master Server code into CVS
//
//
// DESCRIPTION:
//
//
//-----------------------------------------------------------------------------

#ifndef _COMMON_H_
#define _COMMON_H_


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#if defined (_WIN32) || defined (_WIN64) || defined (__OS2__)
#define strcasecmp stricmp
#endif

// ================================ DEFINITIONS ===============================

#if defined (_WIN32) || defined (_WIN64)
#define DEFCOL     ""   // codes couleurs ANSI
#define BLACK      ""
#define RED        ""
#define GREEN      ""
#define BROWN      ""
#define BLUE       ""
#define PURPLE     ""
#define CYAN       ""
#define LIGHTGRAY  ""
#define DARKGRAY   ""
#define LIGHTRED   ""
#define LIGHTGREEN ""
#define YELLOW     ""
#define LIGHTBLUE  ""
#define MAGENTA    ""
#define LIGHTCYAN  ""
#define WHITE      ""

#else

#define DEFCOL     "\033[0m"   // codes couleurs ANSI
#define BLACK      "\033[0;30m"
#define RED        "\033[0;31m"
#define GREEN      "\033[0;32m"
#define BROWN      "\033[0;33m"
#define BLUE       "\033[0;34m"
#define PURPLE     "\033[0;35m"
#define CYAN       "\033[0;36m"
#define LIGHTGRAY  "\033[0;37m"
#define DARKGRAY   "\033[1;30m"
#define LIGHTRED   "\033[1;31m"
#define LIGHTGREEN "\033[1;32m"
#define YELLOW     "\033[1;33m"
#define LIGHTBLUE  "\033[1;34m"
#define MAGENTA    "\033[1;35m"
#define LIGHTCYAN  "\033[1;36m"
#define WHITE      "\033[1;37m"

#endif

typedef enum {
    FE_SIGNAL_ERR,
    FE_SELECT_ERR,
    FE_READ_ERR,
    FE_WRITE_ERR,
    NUM_FATAL_ERROR
} fatal_error_t;

// ================================== PROTOS ==================================

void    clearScreen();
void    fatalError(fatal_error_t);
void    dbgPrintf(char *, char *, ...);
void    logPrintf(FILE *, char *, ...);
void    conPrintf(char *, char *, ...);
FILE    *openFile(char *filename);
char    *pCrypt(char *pw, char *salt);

// ================================== EXTERNS =================================

#endif
