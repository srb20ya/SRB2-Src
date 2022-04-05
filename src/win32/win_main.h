// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: win_main.h,v 1.5 2000/08/03 17:57:42 bpereira Exp $
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
// $Log: win_main.h,v $
// Revision 1.5  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.4  2000/04/27 18:02:35  hurdler
// changed boolean to int (at least it compiles on my computer)
//
// Revision 1.3  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:12  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//-----------------------------------------------------------------------------
/// \file
/// \brief Win32 Sharing

//#define WIN32_LEAN_AND_MEAN
#define RPC_NO_WINDOWS_H
#include <windows.h>
#include <stdio.h>

extern HINSTANCE myInstance;
extern HWND hWndMain;

// debugging CONS_Printf to file
#ifdef LOGMESSAGES
extern HANDLE logstream;
#endif
extern int appActive;

// the MIDI callback is another thread, and Midi volume is delayed here in window proc
VOID I_SetMidiChannelVolume(DWORD dwChannel, DWORD dwVolumePercent);
extern DWORD dwVolumePercent;

VOID I_GetSysMouseEvents(int mouse_state);
extern unsigned int MSHWheelMessage;

extern BOOL nodinput;
extern BOOL win9x;

//faB: midi channel Volume set is delayed by the MIDI stream callback thread, see win_snd.c
#define WM_MSTREAM_UPDATEVOLUME (WM_USER + 101)

// defined in win_sys.c
VOID I_BeginProfile(VOID); //for timing code
DWORD I_EndProfile(VOID);

VOID I_GetLastErrorMsgBox(VOID);
VOID I_LoadingScreen ( LPCSTR msg );

// output formatted string to file using win32 functions (win_dbg.c)
VOID FPrintf(HANDLE fileHandle, LPCSTR lpFmt, ...);
VOID FPutChar(HANDLE fileHandle, const char *c);

void I_RestartSysMouse(void);
void I_DoStartupMouse(void);
void I_SaveMemToFile(unsigned char* pData, unsigned long iLength, const char* sFileName);
