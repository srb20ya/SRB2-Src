// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: win_dll.c,v 1.4 2000/08/10 14:19:56 hurdler Exp $
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
// $Log: win_dll.c,v $
// Revision 1.4  2000/08/10 14:19:56  hurdler
// add waitvbl
//
// Revision 1.3  2000/04/16 18:38:07  bpereira
// no message
//
//
// DESCRIPTION:
//      load and initialise the 3D driver DLL
//
//-----------------------------------------------------------------------------


#include "../hardware/hw_drv.h"        // get the standard 3D Driver DLL exports prototypes
#include "win_dll.h"
#include "win_main.h"       // I_GetLastErrorMsgBox()

// m_misc.h
char *va(char *format, ...);


// ==========================================================================
// STANDARD 3D DRIVER DLL FOR DOOM LEGACY
// ==========================================================================

// note : the 3D driver loading should be put somewhere else..

HINSTANCE hwdInstance = NULL;

loadfunc_t hwdFuncTable[] = {
    {"_Init@4",            &hwdriver.pfnInit},
    {"_Shutdown@0",        &hwdriver.pfnShutdown},
    {"_GetModeList@8",     &hwdriver.pfnGetModeList},
    {"_SetPalette@8",      &hwdriver.pfnSetPalette},
    {"_FinishUpdate@4",    &hwdriver.pfnFinishUpdate},
    {"_Draw2DLine@12",     &hwdriver.pfnDraw2DLine},
    {"_DrawPolygon@16",    &hwdriver.pfnDrawPolygon},
    {"_SetBlend@4",        &hwdriver.pfnSetBlend},
    {"_ClearBuffer@12",    &hwdriver.pfnClearBuffer},
    {"_SetTexture@4",      &hwdriver.pfnSetTexture},
    {"_ReadRect@24",       &hwdriver.pfnReadRect},
    {"_ClipRect@20",       &hwdriver.pfnClipRect},
    {"_ClearMipMapCache@0",&hwdriver.pfnClearMipMapCache},
    {"_SetSpecialState@8", &hwdriver.pfnSetSpecialState},
    {"_DrawMD2@12",        &hwdriver.pfnDrawMD2},
    {"_SetTransform@4",    &hwdriver.pfnSetTransform},
    {NULL,NULL}
};

BOOL Init3DDriver (char* dllName)
{
    hwdInstance = LoadDLL (dllName, hwdFuncTable);
    return (hwdInstance != NULL);
}

void Shutdown3DDriver (void)
{
    UnloadDLL (&hwdInstance);
}


// --------------------------------------------------------------------------
// Load a DLL, returns the HINSTANCE handle or NULL
// --------------------------------------------------------------------------
HINSTANCE LoadDLL (char* dllName, loadfunc_t* funcTable)
{
    void*       funcPtr;
    loadfunc_t* loadfunc;
    HINSTANCE   hInstance;

    if ((hInstance = LoadLibrary (dllName)) != NULL)
    {
        // get function pointers for all functions we use
        for (loadfunc = funcTable; loadfunc->fnName!=NULL; loadfunc++)
        {
            funcPtr = GetProcAddress (hInstance, loadfunc->fnName);
            if (!funcPtr) {
                //I_GetLastErrorMsgBox ();
                MessageBox( NULL, va("The '%s' haven't the good specification (function %s missing)\n\n"
                                     "You must use dll from the same zip of this exe\n", dllName, loadfunc->fnName), "Error", MB_OK|MB_ICONINFORMATION );
                return FALSE;
            }
            // store function address
            *((void**)loadfunc->fnPointer) = funcPtr;
        }
    }
    else
    {
        MessageBox( NULL, va("LoadLibrary() FAILED : couldn't load '%s'\r\n", dllName), "Warning", MB_OK|MB_ICONINFORMATION );
        //I_GetLastErrorMsgBox ();
    }

    return hInstance;
}


// --------------------------------------------------------------------------
// Unload the DLL
// --------------------------------------------------------------------------
void UnloadDLL (HINSTANCE* pInstance)
{
    if (FreeLibrary (*pInstance))
        *pInstance = NULL;
    else
        I_GetLastErrorMsgBox ();
}
