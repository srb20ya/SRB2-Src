// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: win_vid.c,v 1.6 2000/08/10 19:58:05 bpereira Exp $
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
// $Log: win_vid.c,v $
// Revision 1.6  2000/08/10 19:58:05  bpereira
// no message
//
// Revision 1.5  2000/08/10 17:04:22  hurdler
// add ticrate to hardware mode
//
// Revision 1.4  2000/08/10 14:19:56  hurdler
// add waitvbl
//
// Revision 1.3  2000/02/27 00:42:12  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      win32 video driver for Doom Legacy
//
//-----------------------------------------------------------------------------


#include <stdlib.h>
#include <stdarg.h>

#include "../doomdef.h"
#include "../i_system.h"
#include "../m_argv.h"
#include "../v_video.h"
#include "../st_stuff.h"
#include "../i_video.h"
#include "../z_zone.h"
#include "fabdxlib.h"       //wow! I can make use of my win32 test stuff!!

#include "win_main.h"
#include "win_vid.h"

#include "win_dll.h"                //loading the Glide Render DLL
#include "../hardware/hw_drv.h"     //calling Driver Init & Shutdown
#include "../hardware/hw_main.h"    //calling HWR module Init & Shutdown

// -------
// Globals
// -------

// this is the CURRENT rendermode!! very important : used by w_wad, and many other code
rendermode_t    rendermode;

// synchronize page flipping with screen refresh
consvar_t       cv_vidwait = {"vid_wait","1",CV_SAVE,CV_OnOff};

boolean         highcolor;

static  BOOL        bDIBMode;           // means we are using DIB instead of DirectDraw surfaces
static  BITMAPINFO* bmiMain = NULL;
static  HDC         hDCMain = NULL;


// -----------------
// Video modes stuff
// -----------------

#define MAX_EXTRA_MODES         30
static  vmode_t     extra_modes[MAX_EXTRA_MODES] = {{NULL, NULL}};
static  char        names[MAX_EXTRA_MODES][10];

static  int     totalvidmem;

int     numvidmodes;   //total number of DirectDraw display modes
vmode_t *pvidmodes;    //start of videomodes list.
vmode_t *pcurrentmode; // the current active videomode.

//Hudler: 16/10/99: added for OpenGL gamma correction
RGBA_t  gamma_correction = {0x7f7f7f7f};

static int VID_SetWindowedDisplayMode (viddef_t *lvid, vmode_t *pcurrentmode);

// this holds description of the startup video mode,
// the resolution is 320x200, windowed on the desktop
#define NUMSPECIALMODES  1
vmode_t specialmodes[NUMSPECIALMODES] = {
        {
            NULL,
            "320x200W", //faB: W to make sure it's the windowed mode
            320, 200,   //(200.0/320.0)*(320.0/240.0),
            320, 1,     // rowbytes, bytes per pixel
            1, 2,       // windowed (TRUE), numpages
            NULL,
            VID_SetWindowedDisplayMode,
            0          // misc
        }
};


// ------
// Protos
// ------
static  void VID_Command_NumModes_f (void);
static  void VID_Command_ModeInfo_f (void);
static  void VID_Command_ModeList_f (void);
static  void VID_Command_Mode_f     (void);
static  int VID_SetDirectDrawMode (viddef_t *lvid, vmode_t *pcurrentmode);
static  int VID_SetWindowedDisplayMode (viddef_t *lvid, vmode_t *pcurrentmode);
        vmode_t *VID_GetModePtr (int modenum);
        void VID_Init (void);


// -----------------
// I_StartupGraphics
// Initialize video mode, setup dynamic screen size variables,
// and allocate screens.
// -----------------
void I_StartupGraphics(void)
{
    if (graphics_started)
        return;

    // 0 for 256 color, else use highcolor modes
    //highcolor = M_CheckParm ("-highcolor");

    // use 3dfx version
    if (M_CheckParm ("-3dfx"))
        rendermode = render_glide;
    else if (M_CheckParm ("-opengl"))
        rendermode = render_opengl;
    else if (M_CheckParm ("-minigl")) // MiniGL is considered like ...
        rendermode = render_opengl;   // ... OpenGL in the main code
    else if (M_CheckParm ("-d3d"))
        rendermode = render_d3d;
    else
        rendermode = render_soft;

    VID_Init();

    //added:03-01-98: register exit code for graphics
    I_AddExitFunc (I_ShutdownGraphics);
    graphics_started = TRUE;
}


// ------------------
// I_ShutdownGraphics
// Close the screen, restore previous video mode.
// ------------------
void I_ShutdownGraphics (void)
{
    if (!graphics_started)
        return;

    CONS_Printf ("I_ShutdownGraphics()\n");

    // release windowed startup stuff
    if (hDCMain) {
        ReleaseDC (hWndMain, hDCMain);
        hDCMain = NULL;
    }
    if (bmiMain) {
        GlobalFree (bmiMain);
        bmiMain = NULL;
    }

    if ( rendermode != render_soft )
    {
        // Hurdler: swap des deux lignes comme ça on close
        //          l'environnement OpenGL/glide après avoir 
        //          vidé la cache ce qui est bcp plus propre
        HWR_Shutdown ();      //free stuff from the hardware renderer
        HWD.pfnShutdown ();   //close 3d card display
        Shutdown3DDriver ();  //free the driver DLL
    }

    // free the last video mode screen buffers
    if (vid.buffer) {
        GlobalFree (vid.buffer);
        vid.buffer = NULL;
    }

    if ( rendermode == render_soft )
    {
        //HWD.pfnShutdown ();
        //ShutdownSoftDriver ();
        CloseDirectDraw ();
    }

    graphics_started = FALSE;
}


// ------------
// I_StartFrame
// ------------
void I_StartFrame (void)
{
    //faB: no use
}


// --------------
// I_UpdateNoBlit
// --------------
void I_UpdateNoBlit (void)
{
    // what is this?
}


#define FPSPOINTS  35
#define SCALE      4
#define PUTDOT(xx,yy,cc) screens[0][((yy)*vid.width+(xx))*vid.bpp]=(cc)

static int fpsgraph[FPSPOINTS];


// --------------
// I_FinishUpdate
// --------------
void I_FinishUpdate (void)
{
    static int  lasttic;
    int         tics;
    int         i;
    //RECT        Rect;

    // display a graph of ticrate 
    if (cv_ticrate.value )
    {
        int k,j,l;

        i = I_GetTime();
        tics = i - lasttic;
        lasttic = i;
        if (tics > 20) tics = 20;

        for (i=0;i<FPSPOINTS-1;i++)
            fpsgraph[i]=fpsgraph[i+1];
        fpsgraph[FPSPOINTS-1]=20-tics;

        if( rendermode == render_soft )
        {
            
            // draw dots
            for(j=0;j<=20*SCALE*vid.dupy;j+=2*SCALE*vid.dupy)
            {
                l=(vid.height-1-j)*vid.width*vid.bpp;
                for (i=0;i<FPSPOINTS*SCALE*vid.dupx;i+=2*SCALE*vid.dupx)
                    screens[0][l+i]=0xff;
            }

            // draw the graph
            for (i=0;i<FPSPOINTS;i++)
                for(k=0;k<SCALE*vid.dupx;k++)
                    PUTDOT(i*SCALE*vid.dupx+k, vid.height-1-(fpsgraph[i]*SCALE*vid.dupy),0xff);
        }
        else
        {
            fline_t p;
            for(j=0;j<=20*SCALE*vid.dupy;j+=2*SCALE*vid.dupy)
            {
                l=(vid.height-1-j);
                for (i=0;i<FPSPOINTS*SCALE*vid.dupx;i+=2*SCALE*vid.dupx)
                {
                    p.a.x = i;
                    p.a.y = l;
                    p.b.x = i+1;
                    p.b.y = l;
                    HWR_drawAMline(&p, 0xff);
            }
        }


            for (i=1;i<FPSPOINTS;i++)
            {
                p.a.x = SCALE * (i-1);
                p.a.y = vid.height-1-fpsgraph[i-1]*SCALE*vid.dupy;
                p.b.x = SCALE * i;
                p.b.y = vid.height-1-fpsgraph[i]*SCALE*vid.dupy;
                HWR_drawAMline(&p, 0xff);
            }
        }
    }

    //

    if ( bDIBMode )
    {
        // paranoia
        if ( !hDCMain || !bmiMain || !vid.buffer )
            return;
        // main game loop, still in a window (-win parm)
        SetDIBitsToDevice (hDCMain,
                           0, 0, 320, 200,
                           0, 0, 0, 200,
                           vid.buffer, bmiMain, DIB_RGB_COLORS);
    }
    else
    if (rendermode != render_soft) {
        HWD.pfnFinishUpdate ( cv_vidwait.value );
    }
    else
    {
        // DIRECT DRAW
        // copy virtual screen to real screen
        // 26-12-99 BP: can fail when not active (alt-tab)
        if(LockScreen())
        {
        //faB: TODO: use directX blit here!!? a blit might use hardware with access
        //     to main memory on recent hardware, and software blit of directX may be
        //  optimized for p2 or mmx??
        VID_BlitLinearScreen (vid.buffer, ScreenPtr,
            vid.width*vid.bpp, vid.height,
            vid.width*vid.bpp, ScreenPitch );

        UnlockScreen();

        // swap screens
        ScreenFlip(cv_vidwait.value);
    }
    }
}


//
// This is meant to be called only by CONS_Printf() while game startup
//
void I_LoadingScreen ( LPCSTR msg )
{
    //PAINTSTRUCT ps;
    RECT        rect;
    //HDC         hdc;

    // paranoia
    if ( !hDCMain || !bmiMain || !vid.buffer )
        return;

    //hdc = BeginPaint (vid.WndParent, &ps);
    GetClientRect (vid.WndParent, &rect);

    SetDIBitsToDevice (hDCMain,
                       0, 0, 320, 200,
                       0, 0, 0, 200,
                       vid.buffer, bmiMain, DIB_RGB_COLORS);

    if ( msg ) {
        if ( rect.bottom - rect.top > 32 )
            rect.top = rect.bottom - 32;        // put msg on bottom of window
        SetBkMode ( hDCMain, TRANSPARENT );
        SetTextColor ( hDCMain, RGB(0xff,0xff,0xff) );
        DrawText (hDCMain, msg, -1, &rect,
                  DT_WORDBREAK | DT_CENTER ); //| DT_SINGLELINE | DT_VCENTER);
    }
    //EndPaint (vid.WndParent, &ps);
}


// ------------
// I_ReadScreen
// ------------
void I_ReadScreen (byte* scr)
{
    // DEBUGGING
    if (rendermode != render_soft)
        I_Error ("I_ReadScreen: called while in non-software mode");
    CopyMemory (scr, vid.buffer, vid.width*vid.height*vid.bpp);
}


// ------------
// I_SetPalette
// ------------
void I_SetPalette (byte* palette)
{
    int    c,i;
    byte*  usegamma;
    PALETTEENTRY    mainpal[256];

    usegamma = gammatable[scr_gamma%5];

    if ( bDIBMode )
    {
        // set palette in RGBQUAD format, NOT THE SAME ORDER as PALETTEENTRY, grmpf!
        RGBQUAD*    pColors;
        pColors = (RGBQUAD*) ((char*)bmiMain + bmiMain->bmiHeader.biSize);
        ZeroMemory (pColors, sizeof(RGBQUAD)*256);
        for (i=0; i<256; i++, pColors++)
        {
            c = usegamma[*palette++];
            pColors->rgbRed = c;
            c = usegamma[*palette++];
            pColors->rgbGreen = c;
            c = usegamma[*palette++];
            pColors->rgbBlue = c;
        }
    }
    else
    {
        // this clears the 'flag' for each color in palette
        ZeroMemory (mainpal, sizeof(mainpal));

        // set palette in PALETTEENTRY format
        for (i=0; i<256; i++)
        {
            c = usegamma[*palette++];
            mainpal[i].peRed = c;
            c = usegamma[*palette++];
            mainpal[i].peGreen = c;
            c = usegamma[*palette++];
            mainpal[i].peBlue = c;
        }

        if (rendermode != render_soft)
        {
            //Hurdler 16/10/99: added for OpenGL gamma correction
            gamma_correction.s.red   = cv_grgammared.value;
            gamma_correction.s.green = cv_grgammagreen.value;
            gamma_correction.s.blue  = cv_grgammablue.value;
            HWD.pfnSetPalette (mainpal, &gamma_correction); // for palettized textures
        }
        else
            SetDDPalette (mainpal);         // set DirectDraw palette
    }
}


// for fuck'n debuging
void IO_Color( byte color, byte r, byte g, byte b )
{
/*
outportb( 0x03c8 , color );                // registre couleur
outportb( 0x03c9 , (r>>2) & 0x3f );       // R
outportb( 0x03c9 , (g>>2) & 0x3f );       // G
outportb( 0x03c9 , (b>>2) & 0x3f );       // B
    */
}


//
// return number of video modes in pvidmodes list
//
int VID_NumModes(void)
{
    return numvidmodes - NUMSPECIALMODES;   //faB: dont accept the windowed mode 0
}


// return a video mode number from the dimensions
// returns any available video mode if the mode was not found
int VID_GetModeForSize( unsigned int w, unsigned int h)
{
    vmode_t *pv;
    int     modenum;

#if NUMSPECIALMODES > 1
#error "fix this : pv must point the first fullscreen mode in vidmodes list"
#endif
    // skip the 1st special mode so that it finds only fullscreen modes
    pv = pvidmodes->pnext;
    for (modenum=1; pv!=NULL; pv=pv->pnext,modenum++ )
    {
        if( pv->width ==w &&
            pv->height==h )
            return modenum;
    }

    // if not found, return the first mode avaialable,
    // preferably a full screen mode (all modes after the 'specialmodes')
    if (numvidmodes > NUMSPECIALMODES)
        return NUMSPECIALMODES;         // use first full screen mode

    return 0;   // no fullscreen mode, use windowed mode
}


//
// Enumerate DirectDraw modes available
//
static  int     nummodes=0;
static BOOL GetExtraModesCallback (int width, int height, int bpp)
{
    CONS_Printf ("mode %d x %d x %d bpp\n", width, height, bpp);

    // skip all unwanted modes
    if (highcolor && (bpp != 15))
        goto skip;
    if (!highcolor && (bpp != 8))
        goto skip;

    if ((bpp > 16) ||
        (width > MAXVIDWIDTH) ||
        (height > MAXVIDHEIGHT))
    {
        goto skip;
    }

	// skip non-aspect modes Tails 03-25-2001
	if(!(width == 640 || width == 320))
		goto skip;
	if(!(height == 400 || height == 200))
		goto skip;

    // check if we have space for this mode
    if (nummodes>=MAX_EXTRA_MODES)
    {
        CONS_Printf ("mode skipped (too many)\n");
        return FALSE;
    }

        //DEBUG: test without 320x200 standard mode
        //if (width<640 || height<400)
        //    goto skip;

    // store mode info
    extra_modes[nummodes].pnext = &extra_modes[nummodes+1];
    if (width > 999)
    {
        if (height > 999)
        {
            sprintf (&names[nummodes][0], "%4dx%4d", width, height);
            names[nummodes][9] = 0;
        }
        else
        {
            sprintf (&names[nummodes][0], "%4dx%3d", width, height);
            names[nummodes][8] = 0;
        }
    }
    else
    {
        if (height > 999)
        {
            sprintf (&names[nummodes][0], "%3dx%4d", width, height);
            names[nummodes][8] = 0;
        }
        else
        {
            sprintf (&names[nummodes][0], "%3dx%3d", width, height);
            names[nummodes][7] = 0;
        }
    }

    extra_modes[nummodes].name = &names[nummodes][0];
    extra_modes[nummodes].width = width;
    extra_modes[nummodes].height = height;

    // exactly, the current FinishUdpate() gets the rowbytes itself after locking the video buffer
    // so for now we put anything here
    extra_modes[nummodes].rowbytes = width;
    extra_modes[nummodes].windowed = false;
    extra_modes[nummodes].misc = 0;         // unused
    extra_modes[nummodes].pextradata = NULL;
    extra_modes[nummodes].setmode = VID_SetDirectDrawMode;

    extra_modes[nummodes].numpages = 2;     // double-buffer (but this value is not used)

    extra_modes[nummodes].bytesperpixel = (bpp+1)>>3;

    nummodes++;
skip:
    return TRUE;
}


//
// Collect info about DirectDraw display modes we use
//
void VID_GetExtraModes (void)
{
    nummodes = 0;
    EnumDirectDrawDisplayModes (GetExtraModesCallback);

    // add the extra modes (non 320x200) at the start of the mode list (if there are any)
    if (nummodes)
    {
        extra_modes[nummodes-1].pnext = NULL;
        pvidmodes = &extra_modes[0];
        numvidmodes += nummodes;
    }
}


// ---------------
// WindowMode_Init
// Add windowed modes to the start of the list,
// mode 0 is used for windowed console startup (works on all computers with no DirectX)
// ---------------
static void WindowMode_Init(void)
{
    specialmodes[NUMSPECIALMODES-1].pnext = pvidmodes;
    pvidmodes = &specialmodes[0];
    numvidmodes += NUMSPECIALMODES;
}


// *************************************************************************************
// VID_Init
// Initialize Video modes subsystem
// *************************************************************************************
void VID_Init (void)
{
    vmode_t*    pv;
    int         iMode;
    BOOL        bWinParm;

    // if '-win' is specified on the command line, do not add DirectDraw modes
    if ( ( bWinParm = M_CheckParm ("-win") ) )
        rendermode  = render_soft;

    COM_AddCommand ("vid_nummodes", VID_Command_NumModes_f);
    COM_AddCommand ("vid_modeinfo", VID_Command_ModeInfo_f);
    COM_AddCommand ("vid_modelist", VID_Command_ModeList_f);
    COM_AddCommand ("vid_mode", VID_Command_Mode_f);

    // only in DirectX/win32 version
    CV_RegisterVar (&cv_vidwait);

    //setup the videmodes list,
    // note that mode 0 must always be VGA mode 0x13
    pvidmodes = NULL;
    pcurrentmode = NULL;
    numvidmodes = 0;

    // store the main window handle in viddef struct
    vid.WndParent = hWndMain;
    vid.buffer = NULL;

    // we startup in windowed mode using DIB bitmap
    // we will use DirectDraw when switching fullScreen and entering main game loop
    bDIBMode = TRUE;
    bAppFullScreen = FALSE;

    // initialize the appropriate display device
    if ( rendermode != render_soft )
    {
        char* drvname;

        switch (rendermode)
        {
            case render_glide: drvname = "r_glide.dll"; break;
            case render_opengl: 
            // Here is the only difference between OpenGL and MiniGL in the main code
                if (M_CheckParm ("-opengl"))
                    drvname = "r_opengl.dll";
                else
                    drvname = "r_minigl.dll";
                break;
            case render_d3d:   drvname = "r_d3d.dll"; break;
            default: I_Error ("Unknown hardware render mode"); break;
        }

        // load the DLL
        if ( Init3DDriver (drvname) )
        {
            // perform initialisations
            HWD.pfnInit (I_Error);
            // get available display modes for the device
            HWD.pfnGetModeList (&pvidmodes, &numvidmodes);
        }
        else
        {
            switch (rendermode) {
                case render_glide:
                    I_Error ("Error initializing Glide\n");
                    break;
                case render_opengl:
                    I_Error ("Error initializing OpenGL\n");
                    break;
                case render_d3d:
                    I_Error ("Error initializing Direct3D\n");
                    break;
            }
            rendermode = render_soft;
        }
    }
    if (rendermode == render_soft && !bWinParm )
    {
        if (!CreateDirectDrawInstance ())
            I_Error ("Error initializing DirectDraw");
        // get available display modes for the device
        VID_GetExtraModes ();
    }

    // the game boots in 320x200 standard VGA, but
    // we need a highcolor mode to run the game in highcolor
    if (highcolor && numvidmodes==0)
        I_Error ("No 15bit highcolor VESA2 video mode found, cannot run in highcolor.\n");

    // add windowed mode at the start of the list, very important!
    WindowMode_Init();

    if (numvidmodes==0)
        I_Error ("No display modes available.\n");

    //DEBUG
    for (iMode=0,pv=pvidmodes; pv; pv=pv->pnext,iMode++)
    {
        CONS_Printf ("%#02d: %dx%dx%dbpp (desc: '%s')\n",iMode,
                     pv->width,pv->height,pv->bytesperpixel,pv->name);
    }

    // set the startup screen in a window
    VID_SetMode (0);
}


// --------------------------
// VID_SetWindowedDisplayMode
// Display the startup 320x200 console screen into a window on the desktop,
// switching to fullscreen display only when we will enter the main game loop.
// - we can display error message boxes for startup errors
// - we can set the last used resolution only once, when entering the main game loop
// --------------------------
static int VID_SetWindowedDisplayMode (viddef_t *lvid, vmode_t *pcurrentmode)
{
    int     iScrWidth, iScrHeight;
    int     iWinWidth, iWinHeight;
    int     iWinLeftBorder, iWinTopBorder;
    //RECT    Rect;

#ifdef DEBUG
    CONS_Printf("VID_SetWindowedDisplayMode()\n");
#endif

    lvid->u.numpages = 1;         // not used
    lvid->direct = NULL;        // DOS remains
    lvid->buffer = NULL;

    // allocate screens
    if (!VID_FreeAndAllocVidbuffer (lvid))
        return -1;

    // lvid->buffer should be NULL here!

    if ((bmiMain = (void*)GlobalAlloc (GPTR, sizeof(BITMAPINFO) + (sizeof(RGBQUAD)*256)))==NULL)
        I_Error ("VID_SWDM(): No mem");

    // setup a BITMAPINFO to allow copying our video buffer to the desktop,
    // with color conversion as needed
    bmiMain->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmiMain->bmiHeader.biWidth = lvid->width;
    bmiMain->bmiHeader.biHeight= -(lvid->height);
    bmiMain->bmiHeader.biPlanes = 1;
    bmiMain->bmiHeader.biBitCount = 8;
    bmiMain->bmiHeader.biCompression = BI_RGB;

    // center window on the desktop
    iScrWidth = GetSystemMetrics(SM_CXFULLSCREEN);
    iScrHeight = GetSystemMetrics(SM_CYFULLSCREEN);

    iWinWidth = lvid->width;
    iWinLeftBorder = GetSystemMetrics(SM_CXFIXEDFRAME);
    iWinWidth += (iWinLeftBorder * 2);
    iWinHeight = lvid->height;
    iWinTopBorder = GetSystemMetrics(SM_CYCAPTION);
    iWinHeight += iWinTopBorder;
    iWinHeight += GetSystemMetrics(SM_CYFIXEDFRAME);

    if( devparm )
        MoveWindow (hWndMain, (iScrWidth - iWinWidth)   , (iScrHeight - iWinHeight)   , iWinWidth, iWinHeight, TRUE);
    else
        MoveWindow (hWndMain, (iScrWidth - iWinWidth)>>1, (iScrHeight - iWinHeight)>>1, iWinWidth, iWinHeight, TRUE);

    SetFocus(hWndMain);
    ShowWindow(hWndMain, SW_SHOW);

    if (!(hDCMain = GetDC(hWndMain)))
        I_Error ("VID_SWDM(): GetDC FAILED");
    //SetStretchBltMode (hDCMain, COLORONCOLOR);

    return 1;
}


// ========================================================================
// Returns a vmode_t from the video modes list, given a video mode number.
// ========================================================================
vmode_t *VID_GetModePtr (int modenum)
{
    vmode_t *pv;

    pv = pvidmodes;
    if (!pv)
        I_Error ("VID_error : No video mode found\n");

    while (modenum--)
    {
        pv = pv->pnext;
        if (!pv)
            I_Error ("VID_error : Mode not available\n");
    }
    return pv;
}


//
// return the name of a video mode
//
char* VID_GetModeName (int modenum)
{
    return (VID_GetModePtr(modenum))->name;
}


// ========================================================================
// Sets a video mode
// ========================================================================
int VID_SetMode (int modenum)  //, unsigned char *palette)
{
    int     stat;
    vmode_t *pnewmode, *poldmode;

    CONS_Printf("VID_SetMode(%d)\n",modenum);

    //faB: if mode 0 (windowed) we must not be fullscreen already,
    //     if other mode, check it is not mode 0 and existing
    if ((modenum != 0) || (bAppFullScreen))
    {
        if ((modenum > numvidmodes) || (modenum < NUMSPECIALMODES))
        {
            if (pcurrentmode == NULL)
                modenum = 0;    // revert to the default base vid mode
            else
            {
                //nomodecheck = TRUE;
                I_Error ("Unknown video mode: %d\n", modenum);
                //nomodecheck = FALSE;
                return 0;
            }
        }
    }

    pnewmode = VID_GetModePtr (modenum);

    // dont switch to the same display mode
    if (pnewmode == pcurrentmode)
        return 1;

    // initialize the new mode
    poldmode = pcurrentmode;
    pcurrentmode = pnewmode;

    // initialize vidbuffer size for setmode
    vid.width  = pcurrentmode->width;
    vid.height = pcurrentmode->height;
    //vid.aspect = pcurrentmode->aspect;                // aspect ratio might be needed later for 3dfx version..
    vid.rowbytes = pcurrentmode->rowbytes;
    vid.bpp      = pcurrentmode->bytesperpixel;
    //hurdler: 15/10/99: added
    if (modenum) { // if not 320x200 windowed mode
        // it's actually a hack
        if (rendermode == render_opengl) {
            // don't accept depth < 16 for OpenGL mode (too much ugly)
            if (cv_scr_depth.value<16)
                CV_SetValue (&cv_scr_depth,  16);
            vid.bpp = cv_scr_depth.value/8;
            vid.u.windowed = !cv_fullscreen.value;
            pcurrentmode->bytesperpixel = vid.bpp;
            pcurrentmode->windowed = vid.u.windowed;
        }
    }

    stat = (*pcurrentmode->setmode) (&vid, pcurrentmode);

    if (stat < 1)
    {
        if (stat == 0)
        {
            // harware could not setup mode
            //if (!VID_SetMode (vid.modenum))
            //        I_Error ("VID_SetMode: couldn't set video mode (hard failure)");
            I_Error ("Couldn't set video mode %d (%dx%d %d bits)\n", modenum, vid.width, vid.height, (vid.bpp*8));
        }
        else
            if (stat == -1)
            {
                I_Error ("Not enough mem for VID_SetMode\n");

                // not enough memory; just put things back the way they were
                /*pcurrentmode = poldmode;
                vid.width = pcurrentmode->width;
                vid.height = pcurrentmode->height;
                vid.rowbytes = pcurrentmode->rowbytes;
                vid.bpp  = pcurrentmode->bytesperpixel;
                return 0;*/
            }
    }

    vid.modenum = modenum;

    // tell game engine to recalc all tables and realloc buffers based on
    // new vid values
    vid.recalc = 1;

    if ( modenum < NUMSPECIALMODES )
    {
        // we are in startup windowed mode
        bAppFullScreen = FALSE;
        bDIBMode = TRUE;
    }
    else
    {
        // we switch to fullscreen
        bAppFullScreen = TRUE;
        bDIBMode = FALSE;
        if ( rendermode != render_soft ) {
            // purge all patch graphics stored in software format
            //Z_FreeTags ( PU_PURGELEVEL, PU_PURGELEVEL+100 );
            HWR_Startup ();
        }
    }

    return 1;
}


// ========================================================================
// Free the video buffer of the last video mode,
// allocate a new buffer for the video mode to set.
// ========================================================================
BOOL    VID_FreeAndAllocVidbuffer (viddef_t *lvid)
{
    int  vidbuffersize;

    vidbuffersize = (lvid->width * lvid->height * lvid->bpp * NUMSCREENS)
        + (lvid->width * ST_HEIGHT * lvid->bpp);  //status bar

    // free allocated buffer for previous video mode
    if (lvid->buffer)
        GlobalFree (lvid->buffer);

    // allocate & clear the new screen buffer
    if ((lvid->buffer = GlobalAlloc (GPTR, vidbuffersize))==NULL)
        return FALSE;

#ifdef DEBUG
    CONS_Printf("VID_FreeAndAllocVidbuffer done, vidbuffersize: %x\n",vidbuffersize);
#endif
    return TRUE;
}


// ========================================================================
// Set video mode routine for DirectDraw display modes
// Out: 1 ok,
//              0 hardware could not set mode,
//     -1 no mem
// ========================================================================
static int VID_SetDirectDrawMode (viddef_t *lvid, vmode_t *pcurrentmode)
{

#ifdef DEBUG
    CONS_Printf("VID_SetDirectDrawMode...\n");
#endif

    // DD modes do double-buffer page flipping, but the game engine doesn't need this..
    lvid->u.numpages = 2;

//MessageBox (hWndMain, "switch full screen","bla",MB_OK|MB_ICONERROR);

    // release ddraw surfaces etc..
    ReleaseChtuff();

    // clean up any old vid buffer lying around, alloc new if needed
    if (!VID_FreeAndAllocVidbuffer (lvid))
        return -1;                  //no mem

    //added:20-01-98: should clear video mem here

    // note use lvid->bpp instead of 8 ... will thios be needed ? will we support other than 256color
    // in software ?
    if (!InitDirectDrawe(hWndMain, lvid->width, lvid->height, 8, TRUE)) // TRUE currently always full screen
        return 0;               // could not set mode

    // this is NOT used with DirectDraw modes, game engine should never use this directly
    // but rather render to memory bitmap buffer
    lvid->direct = NULL;

    return 1;
}


// ========================================================================
//                     VIDEO MODE CONSOLE COMMANDS
// ========================================================================


//  vid_nummodes
//
static  void VID_Command_NumModes_f (void)
{
    int     nummodes;

    nummodes = VID_NumModes ();
    CONS_Printf ("%d video mode(s) available(s)\n", nummodes);
}


//  vid_modeinfo <modenum>
//
static  void VID_Command_ModeInfo_f (void)
{
    vmode_t     *pv;
    int         modenum;

    if (COM_Argc()!=2)
        modenum = vid.modenum;          // describe the current mode
    else
        modenum = atoi (COM_Argv(1));   //    .. the given mode number

    if (modenum >= VID_NumModes() || modenum<1) //faB: dont accept the windowed mode 0
    {
        CONS_Printf ("No such video mode\n");
        return;
    }

    pv = VID_GetModePtr (modenum);

    CONS_Printf("%s\n", VID_GetModeName (modenum));
    CONS_Printf("width : %d\n"
                "height: %d\n", pv->width, pv->height);
    if (rendermode==render_soft)
        CONS_Printf("bytes per scanline: %d\n"
                    "bytes per pixel: %d\n"
                    "numpages: %d\n",
                    pv->rowbytes,
                    pv->bytesperpixel,
                    pv->numpages);
}


//  vid_modelist
//
static  void VID_Command_ModeList_f (void)
{
    int         i, nummodes;
    char        *pinfo;
    vmode_t     *pv;
    boolean     na;

    na = false;

    nummodes = VID_NumModes ();
    for (i=NUMSPECIALMODES ; i<=nummodes ; i++)
    {
        pv = VID_GetModePtr (i);
        pinfo = VID_GetModeName (i);

        if (pv->bytesperpixel==1)
            CONS_Printf ("%d: %s\n", i, pinfo);
        else
            CONS_Printf ("%d: %s (hicolor)\n", i, pinfo);
    }
}


//  vid_mode <modenum>
//
static  void VID_Command_Mode_f (void)
{
    int         modenum;

    if (COM_Argc()!=2)
    {
        CONS_Printf ("vid_mode <modenum> : set video mode\n");
        return;
    }

    modenum = atoi(COM_Argv(1));

    if (modenum >= VID_NumModes() || modenum<1) //faB: dont accept the windowed mode 0
        CONS_Printf ("No such video mode\n");
    else
        // request vid mode change
        setmodeneeded = modenum+1;
}
