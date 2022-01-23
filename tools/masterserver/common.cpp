// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: common.cpp,v 1.4 2000/09/01 23:30:32 hurdler Exp $
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
// $Log: common.cpp,v $
// Revision 1.4  2000/09/01 23:30:32  hurdler
// minor changes (stats, log & timeout)
//
// Revision 1.3  2000/08/19 14:33:05  hurdler
// changes on log file management
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

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include "common.h"

// ================================== GLOBALS =================================

static char *fatal_error_msg[NUM_FATAL_ERROR] = {
    "Error: signal()",
    "Error: select()",
    "Error: read()",
    "Error: write()",
};

// used by xxxPrintf() functions as temporary variable
static char     str[1024];
static va_list  arglist;

// ================================= FUNCTIONS ================================

/*
 * clearScreen():
 */
void clearScreen()
{
    printf("\033[0m \033[2J \033[0;0H");
}


/*
 * fatalError():
 */
void fatalError(fatal_error_t num)
{
    clearScreen();
    printf("\n");
    perror(fatal_error_msg[num]);
    printf("\n");
    exit(-1);
}


/*
 * dbgPrintf():
 */
void dbgPrintf(char *col, char *lpFmt, ...)
{
#if defined (__DEBUG__)
    va_start (arglist, lpFmt);
    vsprintf (str, lpFmt, arglist);
    va_end   (arglist);

    printf("%s%s", col, str);
    fflush(stdout);
#endif
}


/*
 * conPrintf():
 */
void conPrintf(char *col, char *lpFmt, ...)
{
    va_start (arglist, lpFmt);
    vsprintf (str, lpFmt, arglist);
    va_end   (arglist);

    printf("%s%s", col, str);
    fflush(stdout);
}


/*
 * logPrintf():
 */
void logPrintf(FILE *f, char *lpFmt, ...)
{
    va_start (arglist, lpFmt);
    vsprintf (str, lpFmt, arglist);
    va_end   (arglist);
    char     *ct;

    time_t t = time(NULL);
    ct = ctime(&t);
    ct[strlen(ct)-1] = '\0';
    fprintf(f, "%s: %s", ct, str);
    fflush(f);
#if defined (__DEBUG__)
    printf("%s%s", DEFCOL, str);
    fflush(stdout);
#endif
}


/*
 * openFile():
 */
FILE *openFile(char *filename)
{
    return fopen(filename, "a+t");
}

