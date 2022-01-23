// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: tools.cpp,v 1.3 2001/07/28 17:33:56 bpereira Exp $
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
// $Log: tools.cpp,v $
// Revision 1.3  2001/07/28 17:33:56  bpereira
// added heretic and hexen support
//
// Revision 1.2  2000/05/02 11:58:51  hurdler
// Updated License (GPL)
//
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include <process.h>
#include <stdio.h>
#include <errno.h>
//#include <direct.h>

#include "tools.h"
#include "lmptools.h"
#include "config.h"
#include "dxdetect.h"
#include "win_misc.h"

#include "resource.h"
#include "dlgDemos.h"

// flags passed to Launch()
#define FLAG_LAUNCHDEMO     1       // use info passed in lmpextrainfo struct: exe, iwad, pwads..

// we are using a simple text file to pass the arguments, this way we are sure
// we will never exceed the limit of command-line arguments (plus, the code is simpler)
#define TEMPARGSFILE    "doomargs.tmp"  // this file holds the command-line arguments and is
                                        // passed to Doom Legacy like this:
                                        // doom3 @doomargs.tmp


// --------------------------------------------------------------------------
//                                  LAUNCH DOOM LEGACY WITH LAUNCHER SETTINGS
// --------------------------------------------------------------------------
// - lmpextrainfo NON NULL : only pass cmdline options required to playback the demo
// - bMultiplayer TRUE : cfg_mp_page, the last page active in multiplayer sheets,
//                       indicates server or client, other config variables holds
//                       the information we need to push network parameters
void Launch (CWnd* Parent, lmpextrainfo_t* lmp, BOOL bMultiplayer, BOOL bnomap )
{
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    char    sIWADPath[_MAX_PATH];
    char*   sIWAD;
    char*   args[] = { NULL, "@"TEMPARGSFILE, NULL };
    int     iRetVal;
    char    sEXEPath[_MAX_PATH];
    char*   sProgramPath;

    // Free some memory we don't need
    DX_Cleanup ();

    // chdir to the path of the IWAD
    // (Doom Legacy looks for the Iwad in the current directory)

    // if playback demo, go to the IWAD path specified in the demo
    if (lmp && lmp->iwad) {
        sIWAD = CFG_IwadGameDir( lmp->iwad );
        if ( sIWAD )
            lstrcpy (sIWADPath, FilePath( sIWAD ) );
        else {
            ErrorBox ( "The specified IWAD '%s' is not found in the Game directories\r\n"
                       "(note: the IWAD file specified in the demo should have an '.wad' extension)",
                       lmp->iwad );
            return;
        }
    }
    else {
        // go to the IWAD path currently selected in Game Setup dialog
        lstrcpy (sIWADPath, FilePath( CFG_SelectedGameDir() ) );
    }

    //if ( _chdir (path) ) {
    if ( !SetCurrentDirectory (sIWADPath) ) {
        WIN_DisplayError ("Couldn't change current directory to selected game dir\r\n"
                          "(verify the game directories in the 'Setup' panel)");
        return;
    }

    // create the temporary arguments file, overwrite existing
    hFile = CreateFile (TEMPARGSFILE, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL /*file flag writethrough?*/, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        WIN_DisplayError ("Launch: CreateFile FAILED, unable to pass arguments");
        return;
    }

    CFG_PushArgs ( hFile, lmp, bMultiplayer, bnomap );
    // save config so if something go wrong we have not lost it
    CFG_SaveDefaults ();

    if ( !CloseHandle ( hFile ) )
        ErrorBox ("Launch: warning, CloseHandle FAILED");

    //if (AfxMessageBox(TEMPARGSFILE " is written, click yes to go on", MB_YESNO+MB_DEFBUTTON2,0)==IDNO)
    //    return;

    if ( lmp && lmp->exe ) {
        lstrcpy (sEXEPath, FilePath( cfg_programpath ));
        BackslashPath (sEXEPath);
        lstrcat (sEXEPath, lmp->exe);
        sProgramPath = sEXEPath;
    }
    else
        sProgramPath = cfg_programpath;
    
    //
    // run Doom Legacy
    //
    args[0] = sProgramPath;
    iRetVal = _spawnv(_P_WAIT, sProgramPath, args);
    if ( iRetVal == -1 )
    {
        char *errorstr = NULL;
        switch (errno) {
            case ENOENT  : errorstr="File or path is not found"; break;
            case ENOEXEC : errorstr="Specified file is not executable or has invalid executable-file format";break;
            case ENOMEM  : errorstr="Not enough memory is available to execute new process";break;
            case E2BIG   : errorstr="Argument list exceeds 1024 bytes";break;
            case EINVAL  : errorstr="mode argument is invalid";break;
            default      : break;  // legacy return -1
        }
        if( errorstr )
            ErrorBox (va("Cannot execute %s\n\n %s\n\nLook in Game Mode menu and put a correct Doom Legacy program\n", sProgramPath, errorstr ));
    }

    //
    // record demo info if a demo was recorded
    //
    if ( !lmp && iRetVal!=-1 && !bnomap) { // no playback
        if ( (bMultiplayer && cfg_mp_recorddemo) ||
             (!bMultiplayer && cfg_sp_recorddemo) )
        {
            CNewDemoDlg  dlg (MAKEINTRESOURCE(DLG_NEWDEMO), Parent);
        
            // name of the demo file that we recorded to
            lstrcpy (dlg.sSaveFile, sIWADPath );
            BackslashPath ( dlg.sSaveFile );
            lstrcat (dlg.sSaveFile, "tempdemo.lmp");

            lstrcpy (dlg.sSaveExe, FileBase(cfg_programpath) );                 // EXE we used
        
            lstrcpy (dlg.sSaveIWAD, FileBase( CFG_SelectedGameDir() ) );        // IWAD we used
        
            if ( bMultiplayer && cfg_mp_page==1 )  //TODO: how do we know the addfiles for client?
                lstrcpy (dlg.sSavePwads, cfg_sv_addfiles);
            else if ( !bMultiplayer && cfg_sp_useaddfiles )
                lstrcpy (dlg.sSavePwads, cfg_sp_addfiles);
            else
                dlg.sSavePwads[0] = '\0';   //argh, how does the client know which pwads were used?
        
            dlg.DoModal();
        }
    }
    
    //faB: DOS exe : automatically close the dosbox when it's finished

    //execv method doesn't return
    //_execv (cfg_programpath, args);
    //ErrorBox ("Could not execute program");
    //PostQuitMessage (0);
}


// --------------------------------------------------------------------------
// FilePath : return path from a filename
// --------------------------------------------------------------------------
char*  FilePath (char* sPathName)
{
static char sPath[_MAX_PATH];
    char    sDrive[_MAX_DRIVE];
    char    sDir[_MAX_DIR];
    
    // let system handle the path its own way..
    _fullpath ( sPath, sPathName, _MAX_PATH );
    _splitpath ( sPath, sDrive, sDir, NULL, NULL );
    _makepath ( sPath, sDrive, sDir, NULL, NULL );
  
/*
    int i;
    for (i = strlen (pathname) - 1; i>=0; i--)
    {
        if (pathname[i]==':' || pathname[i]=='/' || pathname[i]=='\\') {
            strncpy (path, pathname, i+1);
            break;
        }
    }
    path[++i] = '\0';
    */

    return sPath;
}


// --------------------------------------------------------------------------
// FileBase : return file name without the path
// --------------------------------------------------------------------------
char*  FileBase (char* pathname, BOOL extension)
{
    static char fname[_MAX_PATH];
    static char ext[_MAX_EXT];
    _splitpath (pathname, NULL, NULL, fname, ext);
    if (extension)
        strcat (fname, ext);

    return fname;

/*
    static  char    path[_MAX_PATH];    

    int i;

    for (i = strlen (pathname) - 1; i>=0; i--)
    {
        if (pathname[i]==':' || pathname[i]=='/' || pathname[i]=='\\') {
            break;
        }
    }

    i++;
    strcpy (path, pathname+i);
    
    // remove extension ?
    if (!extension)
    {
        for (i=0; path[i] && path[i]!='.'; i++)
            ;
        path[i] = '\0';
    }

    return path;
    */
}


// --------------------------------------------------------------------------
// Append a subdirectory to a path, adding '\' if required
// - If subdir is "..", remove the last subdir in the path (do a 'parent')
// --------------------------------------------------------------------------
void AddSubDir (char* path, char* subdir)
{
    int l = strlen (path) - 1;

    if (subdir[0]=='.' && subdir[1]=='.')
    {
        // skip before ending slash
        if (path[l]==':')
            return;
        if (path[l]=='\\' || path[l]=='/')
            l--;
        // go back to last slash, put a zero to truncate the last subdir
        while (l>=0 && path[l]!='\\' && path[l]!='/' && path[l]!=':')
            l--;
        if (l>=0)
            path[l] = '\0';
        return;
    }
    
    if (path[l]!='\\' && path[l]!='/') {
        path[l+1] = '\\';
        l++;
    }

    strcpy (path+l+1, subdir);
}


// --------------------------------------------------------------------------
// Add a pending '\' to the directory path if it is not present
// --------------------------------------------------------------------------
void BackslashPath (char* path)
{
    int l = strlen(path) - 1;
    if (path[l] != '/' && path[l] != '\\') {
        path[l+1] = '\\';
        path[l+2] = '\0';
    }
}


// --------------------------------------------------------------------------
// checks if needed, and add default extension to filename
// --------------------------------------------------------------------------
void DefaultExtension (char *path, char *extension)
{
    char    *src;

  // search for '.' from end to begin, add .EXT only when not found
    src = path + strlen(path) - 1;

    while (*src != '/' && src != path)
    {
	if (*src == '.')
	    return;		    // it has an extension
	src--;
    }

    strcat (path, extension);
}


// --------------------------------------------------------------------------
// Build a complete path+file name with optional extension added
// --------------------------------------------------------------------------
char* MakeFileName (char* path, char* file, char* extension = NULL)
{
static char    filename[_MAX_PATH+1];

    strcpy (filename, path);
    BackslashPath (filename);
    strcat (filename, file);
    if (extension!=NULL)
        DefaultExtension (filename, extension);
    return filename;
}


// ==========================================================================
//			                                MISC STRING FUNCTIONS
// ==========================================================================


//  Temporary varargs printf
//
char*	va(char *format, ...)
{
    va_list	 argptr;
    static char  string[1024];

    va_start (argptr, format);
    vsprintf (string, format,argptr);
    va_end (argptr);

    return string;
}
