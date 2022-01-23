// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: win_main.c,v 1.5 2000/08/03 17:57:42 bpereira Exp $
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
// $Log: win_main.c,v $
// Revision 1.5  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.4  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.3  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:12  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Win32 Doom LEGACY
// NOTE:
//      To compile WINDOWS Legacy version : define a '__WIN32__' symbol.
//      to do this go to Project/Settings/ menu, click C/C++ tab, in 
//      'Preprocessor definitions:' add '__WIN32__'
//
//-----------------------------------------------------------------------------


#include <stdio.h>

#include "../doomdef.h"
#include "../doomstat.h"  // netgame
#include "resource.h"

#include "../m_argv.h"
#include "../d_main.h"
#include "../i_system.h"

#include "../keys.h"    //hack quick test

#include "../console.h"

#include "fabdxlib.h"
#include "win_main.h"
#include "win_dbg.h"
#include "../I_sound.h"  // midi pause/unpause


HINSTANCE       myInstance=NULL;
HWND            hWndMain=NULL;
HCURSOR         windowCursor=NULL;                      // main window cursor

boolean         appActive = false;                      //app window is active

#ifdef LOGMESSAGES
// this is were I log debug text, cons_printf, I_error ect for window port debugging
HANDLE  logstream;
#endif

// faB: the MIDI callback is another thread, and Midi volume is delayed here in window proc
extern void I_SetMidiChannelVolume( DWORD dwChannel, DWORD dwVolumePercent );
extern DWORD dwVolumePercent;

void I_LoadingScreen ( LPCSTR msg );
long FAR PASCAL  MainWndproc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    event_t ev;         //Doom input event

    switch( message )
    {
    case WM_CREATE:
        break;

    case WM_ACTIVATEAPP:           // Handle task switching
        appActive = wParam;
        // pause music when alt-tab
        if( appActive )
            I_ResumeSong(0);
        else
            I_PauseSong(0);
        InvalidateRect (hWnd, NULL, TRUE);
        break;

    //for MIDI music
    case WM_MSTREAM_UPDATEVOLUME:
        I_SetMidiChannelVolume( wParam, dwVolumePercent );
        break;

    case WM_PAINT:
        if (!appActive && !bAppFullScreen)
            // app becomes inactive (if windowed )
        {
            // Paint "Game Paused" in the middle of the screen
            PAINTSTRUCT ps;
            RECT        rect;
            HDC hdc = BeginPaint (hWnd, &ps);
            GetClientRect (hWnd, &rect);
            DrawText (hdc, "Game Paused", -1, &rect,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            EndPaint (hWnd, &ps);
            return 0;
        }
        break;

    //case WM_RBUTTONDOWN:
    //case WM_LBUTTONDOWN:

    case WM_MOVE:
        if (bAppFullScreen) {
            SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            return 0;
        }
        else {
            windowPosX = (SHORT) LOWORD(lParam);    // horizontal position
            windowPosY = (SHORT) HIWORD(lParam);    // vertical position
            break;
        }
        break;

        // This is where switching windowed/fullscreen is handled. DirectDraw
        // objects must be destroyed, recreated, and artwork reloaded.

    case WM_DISPLAYCHANGE:
    case WM_SIZE:
        break;

    case WM_SETCURSOR:
        if (bAppFullScreen)
            SetCursor(NULL);
        else
            SetCursor(windowCursor);
        return TRUE;

    case WM_KEYDOWN:
        ev.type = ev_keydown;

handleKeyDoom:
        ev.data1 = 0;
        if (wParam == VK_PAUSE)
        // intercept PAUSE key
        {
            ev.data1 = KEY_PAUSE;
        }
        else if (!keyboard_started)
        // post some keys during the game startup
        // (allow escaping from network synchronization, or pressing enter after
        //  an error message in the console)
        {
            switch (wParam) {
            case VK_ESCAPE: ev.data1 = KEY_ESCAPE;  break;
            case VK_RETURN: ev.data1 = KEY_ENTER;   break;
            default: ev.data1 = MapVirtualKey(wParam,2); // convert in to char
            }
        }

        if (ev.data1) {
            D_PostEvent (&ev);
            return 0;
        }
        break;

    case WM_KEYUP:
        ev.type = ev_keyup;
        goto handleKeyDoom;
        break;

    case WM_CLOSE:
        PostQuitMessage(0);         //to quit while in-game
        ev.data1 = KEY_ESCAPE;      //to exit network synchronization
        ev.type = ev_keydown;
        D_PostEvent (&ev);
        return 0;
    case WM_DESTROY:
        //faB: main app loop will exit the loop and proceed with I_Quit()
        PostQuitMessage(0);
        break;

    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


//
// Do that Windows initialization stuff...
//
HWND    OpenMainWindow (HINSTANCE hInstance, int nCmdShow, char* wTitle)
{
    HWND        hWnd;
    WNDCLASS    wc;
    BOOL        rc;

    // Set up and register window class
    wc.style = CS_HREDRAW | CS_VREDRAW /*| CS_DBLCLKS*/;
    wc.lpfnWndProc = MainWndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DLICON1));
    windowCursor = LoadCursor(NULL, IDC_WAIT); //LoadCursor(hInstance, MAKEINTRESOURCE(IDC_DLCURSOR1));
    wc.hCursor = windowCursor;
    wc.hbrBackground = (HBRUSH )GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "LegacyWC";
    rc = RegisterClass(&wc);
    if( !rc )
        return false;

    // Create a window
    // CreateWindowEx - seems to create just the interior, not the borders

    hWnd = CreateWindowEx(WS_EX_TOPMOST,    //ExStyle
        "LegacyWC",                         //Classname
        wTitle,                             //Windowname
        WS_CAPTION|WS_POPUP|WS_SYSMENU,     //dwStyle       //WS_VISIBLE|WS_POPUP for bAppFullScreen
        0,
        0,
        320,  //GetSystemMetrics(SM_CXSCREEN),
        200,  //GetSystemMetrics(SM_CYSCREEN),
        NULL,                               //hWnd Parent
        NULL,                               //hMenu Menu
        hInstance,
        NULL);

    return hWnd;
}


BOOL tlErrorMessage( char *err)
{
    /* make the cursor visible */
    SetCursor(LoadCursor( NULL, IDC_ARROW ));

    //
    // warn user if there is one
    //
    printf("Error %s..\n", err);
    fflush(stdout);

    MessageBox( hWndMain, err, "ERROR", MB_OK );
    return FALSE;
}


// ------------------
// Command line stuff
// ------------------
#define         MAXCMDLINEARGS          64
static  char*   myWargv[MAXCMDLINEARGS+1];
static  char    myCmdline[512];

static void     GetArgcArgv (LPCSTR cmdline)
{
    char*   token;
    int     i, len;
    char    cSep;
    BOOL    bCvar = FALSE, prevCvar = FALSE;

    // split arguments of command line into argv
    strncpy (myCmdline, cmdline, 511);      // in case window's cmdline is in protected memory..for strtok
    len = lstrlen (myCmdline);

    myWargv[0] = "dummy.exe";
    myargc = 1;
    i = 0;
    cSep = ' ';
    while( myargc < MAXCMDLINEARGS )
    {
        // get token
        while ( myCmdline[i] == cSep )
            i++;
        if ( i >= len )
            break;
        token = myCmdline + i;
        if ( myCmdline[i] == '"' ) {
            cSep = '"';
            i++;
            if ( !prevCvar )    //cvar leave the "" in
                token++;
        }
        else
            cSep = ' ';

        //cvar
        if ( myCmdline[i] == '+' && cSep == ' ' )   //a + begins a cvarname, but not after quotes
            bCvar = TRUE;
        else
            bCvar = FALSE;

        while ( myCmdline[i] &&
                myCmdline[i] != cSep )
            i++;

        if ( myCmdline[i] == '"' ) {
             cSep = ' ';
             if ( prevCvar )
                 i++;       // get ending " quote in arg
        }

        prevCvar = bCvar;

        if ( myCmdline + i > token )
        {
            myWargv[myargc++] = token;
        }

        if ( !myCmdline[i] || i >= len )
            break;

        myCmdline[i++] = '\0';
    }
    myWargv[myargc] = NULL;

    // m_argv.c uses myargv[], we used myWargv because we fill the arguments ourselves
    // and myargv is just a pointer, so we set it to point myWargv
    myargv = myWargv;
}


// -----------------------------------------------------------------------------
// HandledWinMain : called by exception handler
// -----------------------------------------------------------------------------
int WINAPI HandledWinMain(HINSTANCE hInstance,
                          HINSTANCE hPrevInstance,
                          LPSTR     lpCmdLine,
                          int       nCmdShow)
{
    int             i;

#ifdef LOGMESSAGES
    // DEBUG!!! - set logstream to NULL to disable debug log
    logstream = INVALID_HANDLE_VALUE;

    logstream = CreateFile ("log.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, NULL);  //file flag writethrough?
    if (logstream == INVALID_HANDLE_VALUE)
        return 0;
#endif

    // fill myargc,myargv for m_argv.c retrieval of cmdline arguments
    CONS_Printf ("GetArgcArgv() ...\n");
    GetArgcArgv(lpCmdLine);

    CONS_Printf ("lpCmdLine is '%s'\n", lpCmdLine);
    CONS_Printf ("Myargc: %d\n", myargc);
    for (i=0;i<myargc;i++)
        CONS_Printf("myargv[%d] : '%s'\n", i, myargv[i]);


    // store for later use, will we need it ?
    myInstance = hInstance;

    // open a dummy window, both 3dfx Glide and DirectX need one.
    if ( (hWndMain = OpenMainWindow(hInstance,nCmdShow,
		                va("Sonic Robo Blast 2 v0.6"))) == NULL )
//                va("Sonic Robo Blast 2 v%i.%i"VERSIONSTRING,VERSION/100,VERSION%100))) == NULL )
    {
        tlErrorMessage("Couldn't open window");
        return FALSE;
    }

    // currently starts DirectInput 
    CONS_Printf ("I_StartupSystem() ...\n");
    I_StartupSystem();

    // startup Doom Legacy
    CONS_Printf ("D_DoomMain() ...\n");
    D_DoomMain ();
    CONS_Printf ("Entering main app loop...\n");
    // never return
    D_DoomLoop();

    // back to Windoze
    return 0;
}


// -----------------------------------------------------------------------------
// Exception handler calls WinMain for catching exceptions
// -----------------------------------------------------------------------------
int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR        lpCmdLine,
                    int          nCmdShow)
{
    int Result = -1;
    __try
    {
        Result = HandledWinMain (hInstance, hPrevInstance, lpCmdLine, nCmdShow);
    }

    __except ( RecordExceptionInfo( GetExceptionInformation(), "main thread", lpCmdLine) )
    {
        //Do nothing here.
    }

    return Result;
}
