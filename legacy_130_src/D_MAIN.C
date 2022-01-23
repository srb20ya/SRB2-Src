// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_main.c,v 1.23 2000/08/10 14:50:19 ydario Exp $
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
// $Log: d_main.c,v $
// Revision 1.23  2000/08/10 14:50:19  ydario
// OS/2 port
//
// Revision 1.22  2000/05/07 08:27:56  metzgermeister
// no message
//
// Revision 1.21  2000/04/30 10:30:10  bpereira
// no message
//
// Revision 1.20  2000/04/25 19:49:46  metzgermeister
// support for automatic wad search
//
// Revision 1.19  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.18  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.17  2000/04/22 20:27:35  metzgermeister
// support for immediate fullscreen switching
//
// Revision 1.16  2000/04/21 20:04:20  hurdler
// fix a problem with my last SDL merge
//
// Revision 1.15  2000/04/19 15:21:02  hurdler
// add SDL midi support
//
// Revision 1.14  2000/04/18 12:55:39  hurdler
// join with Boris' code
//
// Revision 1.13  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.12  2000/04/07 23:10:15  metzgermeister
// fullscreen support under X in Linux
//
// Revision 1.11  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
//
// Revision 1.10  2000/04/05 15:47:46  stroggonmeth
// Added hack for Dehacked lumps. Transparent sprites are now affected by colormaps.
//
// Revision 1.9  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.8  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.7  2000/03/28 16:18:41  linuxcub
// Added a command to the Linux sound-server which sets a master volume.
// Someone needs to check that this isn't too much of a performance drop
// on slow machines. (Works for me).
//
// Added code to the main parts of doomlegacy which uses this command to
// implement volume control for sound effects.
//
// Added code so the (really cool) cd music works for me. The volume didn't
// work for me (with a Teac 532E drive): It always started at max (31) no-
// matter what the setting in the config-file was. The added code "jiggles"
// the volume-control, and now it works for me :-)
// If this code is unacceptable, perhaps another solution is to periodically
// compare the cd_volume.value with an actual value _read_ from the drive.
// Ie. not trusting that calling the ioctl with the correct value actually
// sets the hardware-volume to the requested value. Right now, the ioctl
// is assumed to work perfectly, and the value in cd_volume.value is
// compared periodically with cdvolume.
//
// Updated the spec file, so an updated RPM can easily be built, with
// a minimum of editing. Where can I upload my pre-built (S)RPMS to ?
//
// Erling Jacobsen, linuxcub@email.dk
//
// Revision 1.6  2000/03/23 22:54:00  metzgermeister
// added support for HOME/.legacy under Linux
//
// Revision 1.5  2000/03/06 17:33:36  hurdler
// compiler warning removed
//
// Revision 1.4  2000/03/05 17:10:56  bpereira
// no message
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
//
// DESCRIPTION:
//      DOOM main program (D_DoomMain) and game loop (D_DoomLoop),
//      plus functions to determine game mode (shareware, registered),
//      parse command line parameters, configure game parameters (turbo),
//      and call the startup functions.
//
//-----------------------------------------------------------------------------

#ifdef LINUX
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifndef __WIN32__
#include <unistd.h>             // for access
#else
#include <direct.h>
#endif
#include <fcntl.h>

#ifdef __OS2__
#include "I_os2.h"
#endif

#include "doomdef.h"

#include "command.h"
#include "console.h"

#include "doomstat.h"

#include "am_map.h"
#include "d_net.h"
#include "d_netcmd.h"
#include "dehacked.h"
#include "dstrings.h"

#include "f_wipe.h"
#include "f_finale.h"

#include "g_game.h"
#include "g_input.h"

#include "hu_stuff.h"

#include "i_sound.h"
#include "i_system.h"
#include "i_video.h"

#include "m_argv.h"
#include "m_menu.h"
#include "m_misc.h"

#include "p_setup.h"
#include "p_fab.h"

#include "r_main.h"
#include "r_local.h"

#include "s_sound.h"
#include "st_stuff.h"

#include "v_video.h"

#include "wi_stuff.h"
#include "w_wad.h"

#include "z_zone.h"
#include "d_main.h"
#include "d_netfil.h"

#include "time.h" // Tails 11-15-2001

#ifdef HWRENDER
#include "hardware/hw_main.h"   // 3D View Rendering
#endif
#ifdef __WIN32__
#include "win32/win_main.h"
#endif

//
//  DEMO LOOP
//
int             demosequence;
int             pagetic;
char            *pagename="TITLEPIC";

//  PROTOS
void D_PageDrawer (char* lumpname);
void D_AdvanceDemo (void);

#ifdef LINUX
void VID_PrepareModeList(void); // FIXME: very dirty; will use a proper include file
#endif

char*           startupwadfiles[MAX_WADFILES];

boolean         devparm;        // started game with -devparm
boolean         nomonsters;     // checkparm of -nomonsters
boolean			xmasmode; // Xmas Mode Tails 12-02-2001
boolean			mariomode; // Mario Mode Tails 12-18-2001

char*				parmskin; // Player skin defined from parms Tails 06-09-2001
char*				ctfteam; // Player Preferred CTF Team defined from parms Tails 07-31-2001
char*				flagtime; // CTF Flag time defined from parms Tails 07-31-2001
char*				parmcolor; // Player color defined from parms Tails 06-09-2001
char*				parmname; // Player name defined from parms Tails 06-09-2001

boolean         singletics = false; // timedemo

boolean         nomusic;    
boolean         nosound;


boolean         advancedemo;


char            wadfile[1024];          // primary wad file
char            mapdir[1024];           // directory of development maps

//
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
//
event_t         events[MAXEVENTS];
int             eventhead;
int             eventtail;


//
// D_PostEvent
// Called by the I/O functions when input is detected
//
void D_PostEvent (const event_t* ev)
{
    events[eventhead] = *ev;
    eventhead = (++eventhead)&(MAXEVENTS-1);
}
// just for lock this function
#ifdef PC_DOS
void D_PostEvent_end(void) {};
#endif


//
// D_ProcessEvents
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents (void)
{
    event_t*    ev;

    //added:12-02-98: doing a W_CheckNumForName() is a bit clumsy here...

    // IF STORE DEMO, DO NOT ACCEPT INPUT
    //if ( ( gamemode == commercial )
    //     && (W_CheckNumForName("map01")<0) )
    //  return;

    for ( ; eventtail != eventhead ; eventtail = (++eventtail)&(MAXEVENTS-1) )
    {
        ev = &events[eventtail];
        // Menu input
        if (M_Responder (ev))
            continue;              // menu ate the event

        // console input
        if (CON_Responder (ev))
            continue;              // ate the event

        G_Responder (ev);
    }
}


//
// D_Display
//  draw current display, possibly wiping it from the previous
//

#ifdef __WIN32__
void I_DoStartupMouse (void);   //win_sys.c
#endif

// wipegamestate can be set to -1 to force a wipe on the next draw
// added comment : there is a wipe eatch change of the gamestate
gamestate_t  wipegamestate = GS_DEMOSCREEN;
CV_PossibleValue_t screenslink_cons_t[]={{0,"None"},{wipe_ColorXForm+1,"Color"},{wipe_Melt+1,"Melt"},{0,NULL}};
consvar_t cv_screenslink    = {"screenlink","Color", CV_SAVE,screenslink_cons_t};

void D_Display (void)
{
    static  boolean             viewactivestate = false;
    static  boolean             menuactivestate = false;
    static  boolean             fullscreen = false;
    static  gamestate_t         oldgamestate = -1;
    static  int                 borderdrawcount;
    int                         nowtime;
    int                         tics;
    int                         wipestart;
    int                         y;
    boolean                     done;
    boolean                     wipe;
    boolean                     redrawsbar;

    if (nodrawers)
        return;                    // for comparative timing / profiling

    redrawsbar = false;

    //added:21-01-98: check for change of screen size (video mode)
    if (setmodeneeded)
    {
        SCR_SetMode();  // change video mode
    }

    if (vid.recalc)
    {
        //added:26-01-98: NOTE! setsizeneeded is set by SCR_Recalc()
        SCR_Recalc();
    }

    // change the view size if needed
    if (setsizeneeded || scr_viewsize!=cv_viewsize.value)
    {
        R_ExecuteSetViewSize ();
        oldgamestate = -1;                      // force background redraw
        borderdrawcount = 3;
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate &&
        rendermode == render_soft)
    {
        wipe = true;
        wipe_StartScreen(0, 0, vid.width, vid.height);
    }
    else
        wipe = false;


    if (gamestate == GS_LEVEL && gametic)
    {
        HU_Erase();
    }

    // do buffered drawing
    switch (gamestate)
    {
      case GS_LEVEL:
        if (!gametic)
            break;
        if (automapactive)
            AM_Drawer ();
        if (wipe || ((viewheight != vid.height) && fullscreen) )
            redrawsbar = true;
        if (vid.recalc) //redraw (& recalc widgets) when vidmode change
            redrawsbar = true;
        if (menuactivestate)      // redraw stbar because menu fades down the
            redrawsbar = true;    // screen

        fullscreen = (viewheight == vid.height);
#ifdef HWRENDER 
        if (rendermode==render_soft)
#endif
            ST_Drawer (fullscreen, redrawsbar );
        break;

      case GS_INTERMISSION:
        WI_Drawer ();
        break;

      case GS_FINALE:
        F_Drawer ();
        break;

      case GS_DEDICATEDSERVER:
      case GS_DEMOSCREEN:
        D_PageDrawer (pagename);
      case GS_WAITINGPLAYERS:
      case GS_NULL:
        break;
    }

    // draw buffered stuff to screen
    // BP: Used only by linux GGI version
    I_UpdateNoBlit ();

    // draw the view directly
    if (gamestate == GS_LEVEL)
    {
        if( !automapactive )
        {
#ifdef HWRENDER 
            if ( rendermode != render_soft )
                HWR_RenderPlayerView (0, &players[displayplayer]);
            else //if (rendermode == render_soft)
#endif
                R_RenderPlayerView (&players[displayplayer]);

            // added 16-6-98: render the second screen
            if( secondarydisplayplayer != consoleplayer && players[secondarydisplayplayer].mo)
            {
#ifdef HWRENDER 
                if ( rendermode != render_soft )
                    HWR_RenderPlayerView (1, &players[secondarydisplayplayer]);
                else 
#endif
                {
                    //faB: Boris hack :P !!
                    viewwindowy = vid.height/2;
                    memcpy(ylookup,ylookup2,viewheight*sizeof(ylookup[0]));

                    R_RenderPlayerView (&players[secondarydisplayplayer]);

                    viewwindowy = 0;
                    memcpy(ylookup,ylookup1,viewheight*sizeof(ylookup[0]));
                }
            }
        }

        // fullscreen with overlay
        if (st_overlay && !automapactive &&
            (playerdeadview || cv_splitscreen.value || !playerdeadview)) //Fab: full clear view when dead
                 // yeah right fab! we want the stats even when he's dead! Tails
        {
            ST_overlayDrawer (0);
            if(cv_splitscreen.value)
            {
                player_t *p;
                extern player_t *plyr;
                p=plyr;
                plyr=&players[secondarydisplayplayer];
                ST_overlayDrawer (1);
                plyr=p;
            }
        }
        HU_Drawer (); // Moved it after the overlayDrawer Tails 05-20-2001
    }

    // change gamma if needed
    if ((scr_gamma!=cv_usegamma.value) ||
        (gamestate != oldgamestate && gamestate != GS_LEVEL) )
    {
        scr_gamma = cv_usegamma.value;
        V_SetPalette (W_CacheLumpName ("PLAYPAL",PU_CACHE));
    }

    // clean up border stuff
    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL && oldgamestate != GS_LEVEL)
    {
        viewactivestate = false;        // view was not active
        R_FillBackScreen ();    // draw the pattern into the back screen
    }

    // see if the border needs to be updated to the screen
    if( gamestate==GS_LEVEL && !automapactive &&
        (scaledviewwidth!=vid.width) )
    {
        // the menu may draw over parts out of the view window,
        // which are refreshed only when needed
        if (menuactive || menuactivestate || !viewactivestate)
            borderdrawcount = 3;

        if (borderdrawcount)
        {
            R_DrawViewBorder ();    // erase old menu stuff
            borderdrawcount--;
        }
    }

    menuactivestate = menuactive;
    viewactivestate = viewactive;
    oldgamestate = wipegamestate = gamestate;

    // draw pause pic
    if (paused && (!menuactive || netgame))
    {
        patch_t* patch;
        if (automapactive)
            y = 4;
        else
            y = viewwindowy+4;
        patch = W_CachePatchName ("M_PAUSE", PU_CACHE);
        V_DrawScaledPatch(viewwindowx+(BASEVIDWIDTH - patch->width)/2,
                          y,0,patch);
    }


    //added:24-01-98:vid size change is now finished if it was on...
    vid.recalc = 0;

#ifdef HWRENDER 
    // added by Hurdler and moved from win_vid.c (before menus)
    // :in hardware mode, statusbar is drawn after playerview
    // :but before drawing the menus so all the menus are visible
    if ((rendermode!=render_soft) && (gamestate==GS_LEVEL))
        ST_Drawer (viewheight==vid.height, 1);
#endif

    //FIXME: draw either console or menu, not the two
    CON_Drawer ();

    // menus go directly to the screen
    M_Drawer ();          // menu is drawn even on top of everything
    NetUpdate ();         // send out any new accumulation

//
// normal update
//
    if (!wipe)
    {
        if( cv_netstat.value )
        {
            char s[20];
            Net_GetNetStat();
            sprintf(s,"get %d b/s",getbps);
            V_DrawStringWhite (BASEVIDWIDTH-V_StringWidth (s),BASEVIDHEIGHT-ST_HEIGHT-40,s);
            sprintf(s,"send %d b/s",sendbps);
            V_DrawStringWhite (BASEVIDWIDTH-V_StringWidth (s),BASEVIDHEIGHT-ST_HEIGHT-30,s);
            sprintf(s,"GameMiss %.2f%%",gamelostpercent);
            V_DrawStringWhite (BASEVIDWIDTH-V_StringWidth (s),BASEVIDHEIGHT-ST_HEIGHT-20,s);
            sprintf(s,"SysMiss %.2f%%",lostpercent);
            V_DrawStringWhite (BASEVIDWIDTH-V_StringWidth (s),BASEVIDHEIGHT-ST_HEIGHT-10,s);
        }

        //added:12-02-98: tilt view when marine dies... just for fun
        if (gamestate == GS_LEVEL &&
            cv_tiltview.value &&
            players[displayplayer].playerstate==PST_DEAD )
        {
            V_DrawTiltView (screens[0]);
        }
#ifdef PERSPCORRECT
        else if (gamestate == GS_LEVEL &&
                 cv_perspcorr.value )
        {
            V_DrawPerspView (screens[0], players[displayplayer].aiming);
        }
#endif
        else
        {
            //I_BeginProfile();
            I_FinishUpdate ();              // page flip or blit buffer
            //CONS_Printf ("last frame update took %d\n", I_EndProfile());
        }
        return;
    }

//
// wipe update
//
    if(!cv_screenslink.value)
        return;

    wipe_EndScreen(0, 0, vid.width, vid.height);

    wipestart = I_GetTime () - 1;
    y=wipestart;
    do
    {
        do
        {
            nowtime = I_GetTime ();
            tics = nowtime - wipestart;
        } while (!tics);
        wipestart = nowtime;
        done = wipe_ScreenWipe (cv_screenslink.value-1
                                , 0, 0, vid.width, vid.height, tics);
        I_StartTic ();
        I_UpdateNoBlit ();
        M_Drawer ();            // menu is drawn even on top of wipes
        I_FinishUpdate ();      // page flip or blit buffer
    } while (!done && I_GetTime()-y<2*TICRATE);
}



// =========================================================================
//   D_DoomLoop
// =========================================================================

static  ULONG     oldentertics;
static  ULONG     lastrendered;
int p; // Tails 06-10-2001
ULONG   rendergametic;
boolean supdate;

void D_DoomLoop (void)
{
    int  entertic,realtics;

    if (demorecording)
        G_BeginRecording ();

    // user settings
    COM_BufAddText ("exec autoexec.cfg\n");

// Skin, name, and color stuff Tails 06-10-2001
p = M_CheckParm ("-skin");
    if (p && p < myargc-1)
    {
		parmskin = myargv[p+1];
		COM_BufAddText("skin \"");
		COM_BufAddText(parmskin);
		COM_BufAddText("\"\n");
    }

p = M_CheckParm ("-color");
    if (p && p < myargc-1)
    {
		parmcolor = myargv[p+1];
		COM_BufAddText("color \"");
		COM_BufAddText(parmcolor);
		COM_BufAddText("\"\n");
    }

p = M_CheckParm ("-flagtime");
    if (p && p < myargc-1)
    {
		ctfteam = myargv[p+1];
		COM_BufAddText("flagtime \"");
		COM_BufAddText(flagtime);
		COM_BufAddText("\"\n");
    }

p = M_CheckParm ("-name");
    if (p && p < myargc-1)
    {
		parmname = myargv[p+1];
		COM_BufAddText("name \"");
		COM_BufAddText(parmname);
		COM_BufAddText("\"\n");
    }
// Skin, name, and color stuff Tails 06-10-2001

    // end of loading screen: CONS_Printf() will no more call FinishUpdate()
    con_startup = false;

#ifdef __WIN32__
    if ( hWndMain!=NULL )
    {
        SetFocus(hWndMain);
        ShowWindow(hWndMain, SW_SHOW);
        UpdateWindow(hWndMain);
    }
#endif
    //faB: make sure the app window has the focus or
    //     DirectInput acquire keyboard won't work
    CONS_Printf ("I_StartupKeyboard...\n");
    I_StartupKeyboard ();

#ifdef __WIN32__
    CONS_Printf ("I_StartupMouse...\n");
    I_DoStartupMouse ();
#endif

    oldentertics = I_GetTime ();

    // make sure to do a d_display to init mode _before_ load a level
    SCR_SetMode();  // change video mode
    SCR_Recalc();

    while (1)
    {
        // get real tics
        entertic = I_GetTime ();
        realtics = entertic - oldentertics;
        oldentertics = entertic;
        
        // frame syncronous IO operations
        // UNUSED for the moment (18/12/98)
        I_StartFrame ();
        
        // process tics (but maybe not if realtic==0)
        TryRunTics (realtics);
#ifdef CLIENTPREDICTION2
        if(singletics || supdate)
#else
        if(singletics || gametic>rendergametic)
#endif
        {
            rendergametic=gametic;
            lastrendered=I_GetTime();
            
            //added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
            S_UpdateSounds ();  // move positional sounds
            // Update display, next frame, with current state.
            D_Display ();
            supdate=false;
        }
        else
            if(lastrendered+2<I_GetTime()) // in case the server hang or netsplit
                D_Display ();
            
        // Win32 exe uses DirectSound..
#if !defined( __WIN32__) && !defined( __OS2__)
        //
        //Other implementations might need to update the sound here.
        //
#ifndef SNDSERV
        // Sound mixing for the buffer is snychronous.
        I_UpdateSound();
#endif
        // Synchronous sound output is explicitly called.
#ifndef SDL
#ifndef SNDINTR
        // Update sound output.
        I_SubmitSound();
#endif
#endif
#endif //__WIN32__
        // check for media change, loop music..
        I_UpdateCD ();
#ifdef __OS2__
        DosSleep( 0);
#endif
    }
}


// =========================================================================
//   D_AdvanceDemo
// =========================================================================

//
// D_PageTicker
// Handles timing for warped projection
//
void D_PageTicker (void)
{
    if (--pagetic < 0)
        D_AdvanceDemo ();
}



//
// D_PageDrawer : draw a patch supposed to fill the screen,
//                fill the borders with a background pattern (a flat)
//                if the patch doesn't fit all the screen.
//
void D_PageDrawer (char* lumpname)
{
    byte*   src;
    byte*   dest;
    int     x;
    int     y;

    // software mode which uses generally lower resolutions doesn't look
    // good when the pic is scaled, so it fills space aorund with a pattern,
    // and the pic is only scaled to integer multiples (x2, x3...)
    if (rendermode==render_soft)
    {
    if( (vid.width>BASEVIDWIDTH) || (vid.height>BASEVIDHEIGHT) )
    {
        src  = scr_borderpatch;
        dest = screens[0];

        for (y=0; y<vid.height; y++)
        {
            for (x=0; x<vid.width/64; x++)
            {
                memcpy(dest, src+((y&63)<<6), 64);
                dest += 64;
            }
            if (vid.width&63)
            {
                memcpy(dest, src+((y&63)<<6), vid.width&63);
                dest   += (vid.width&63);
            }
        }
    }
    }

    V_DrawScaledPatch(0,0, 0, W_CachePatchName(lumpname, PU_CACHE) );

    //added:08-01-98:if you wanna centre the pages it's here.
    //          I think it's not so beautiful to have the pic centered,
    //          so I leave it in the upper-left corner for now...
    //V_DrawPatch (0,0, 0, W_CachePatchName(pagename, PU_CACHE));
}


//
// D_AdvanceDemo
// Called after each demo or intro demosequence finishes
//
void D_AdvanceDemo (void)
{
    advancedemo = true;
}


//
// This cycles through the demo sequences.
// FIXME - version dependend demo numbers?
//
void D_DoAdvanceDemo (void)
{
//extern consvar_t cv_cam_dist; // Tails 06-23-2001
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    usergame = false;               // no save / end game here
    gameaction = ga_nothing;
// Done lots of stuff here Tails
        demosequence = (demosequence+1)%12;

    switch (demosequence)
    {
      case 0:
		G_DeferedPlayDemo ("titledem"); // Title demo Tails 01-06-2000
        break;
	  case 1:
        pagetic = 1;
        gamestate = GS_DEMOSCREEN;
        pagename = "BLACK";
	    break;
      case 2:
		G_DeferedPlayDemo ("demo1");
        break;
      case 3:
        pagetic = 1;
        gamestate = GS_DEMOSCREEN;
        pagename = "BLACK";
        break;
      case 4:
        G_DeferedPlayDemo ("titledem");
        break;
      case 5:
        pagetic = 1;
        gamestate = GS_DEMOSCREEN;
        pagename = "BLACK";
        break;
      case 6:
        G_DeferedPlayDemo ("demo2");
        break;
      case 7:
        pagetic = 1;
        gamestate = GS_DEMOSCREEN;
        pagename = "BLACK";
        break;
      case 8:
        G_DeferedPlayDemo ("titledem");
        break;
      case 9:
        pagetic = 1;
        gamestate = GS_DEMOSCREEN;
        pagename = "BLACK";
        break;
      case 10:
        G_DeferedPlayDemo ("demo3");
        break;
      case 11:
        pagetic = 1;
        gamestate = GS_DEMOSCREEN;
        pagename = "BLACK";
        break;
    }
}

// =========================================================================
//   D_DoomMain
// =========================================================================

//
// D_StartTitle
//
void D_StartTitle (void)
{
    gameaction = ga_nothing;
    playerdeadview = false;
    displayplayer = consoleplayer = statusbarplayer = 0;
    demosequence = -1;
    D_AdvanceDemo ();
    CON_ToggleOff();
}


//
// D_AddFile
//
void D_AddFile (char *file)
{
    int     numwadfiles;
    char    *newfile;

    for (numwadfiles = 0 ; startupwadfiles[numwadfiles] ; numwadfiles++)
        ;

    newfile = malloc (strlen(file)+1);
    strcpy (newfile, file);

    startupwadfiles[numwadfiles] = newfile;
}


#ifdef __WIN32__
#define R_OK    0                       //faB: win32 does not have R_OK in includes..
#else
#define _MAX_PATH   MAX_WADPATH         
#endif


// ==========================================================================
// Identify the Doom version, and IWAD file to use.
// Sets 'gamemode' to determine whether registered/commmercial features are
// available (notable loading PWAD files).
// ==========================================================================

// return gamemode for Doom or Ultimate Doom, use size to detect which one
gamemode_t GetDoomVersion (char* wadfile)
{
    struct stat sbuf;
    // Fab: and if I patch my main wad and the size gets
    // bigger ? uh?
    // BP: main wad MUST not be patched !
    stat (wadfile, &sbuf);
    if (sbuf.st_size<12408292)
        return registered;
    else
        return retail;      // Ultimate
}

void IdentifyVersion (void)
{
    char*       doom1wad;
    char*       doomwad;
    char*       doomuwad;
    char*       doom2wad;
    char*       plutoniawad;
    char*       tntwad;

//    char*       legacywad;

    char        pathtemp[_MAX_PATH];
    char        pathiwad[_MAX_PATH+16];

    int         p,i;

//Fab:25-04-98:unused now
//    char*       doom2fwad;

    char *doomwaddir;

#ifdef LINUX
    // change to the directory where 'doom3.wad' is found
   I_LocateWad();
#endif
   // Disable Doomwaddir support Tails 03-25-2001
//    doomwaddir = getenv("DOOMWADDIR");
//    if (!doomwaddir)
//    {
        // get the current directory (possible problem on NT with "." as current dir)
        if ( getcwd(pathtemp, _MAX_PATH) != NULL )
            doomwaddir = pathtemp;
        else
            doomwaddir = ".";
//    }

    // Commercial.
    doom2wad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(doom2wad, "%s/%s", doomwaddir, text[DOOM2WAD_NUM]);

    // Retail.
    doomuwad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(doomuwad, "%s/%s", doomwaddir, text[DOOMUWAD_NUM]);

    // Registered.
    doomwad = malloc(strlen(doomwaddir)+1+8+1);
    sprintf(doomwad, "%s/%s", doomwaddir, text[DOOMWAD_NUM]);

    // Shareware.
    doom1wad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(doom1wad, "%s/%s", doomwaddir, text[DOOM1WAD_NUM]);

    // and... Doom LEGACY !!! :)
//    legacywad = malloc(strlen(doomwaddir)+1+9+1);
//    sprintf(legacywad, "%s/doom3.wad", doomwaddir);

    // FinalDoom : Plutonia
    plutoniawad = malloc(strlen(doomwaddir)+1+12+1);
    sprintf(plutoniawad, "%s/plutonia.wad", doomwaddir);

    // FinalDoom : Tnt Evilution
    tntwad = malloc(strlen(doomwaddir)+1+7+1);
    sprintf(tntwad, "%s/tnt.wad", doomwaddir);

    /* French stuff.
    doom2fwad = malloc(strlen(doomwaddir)+1+10+1);
    sprintf(doom2fwad, "%s/doom2f.wad", doomwaddir);*/

    // will be overwrite in case of -cdrom or linux home
    sprintf(configfile, "%s/"CONFIGFILENAME, doomwaddir);

    if (M_CheckParm ("-shdev"))
    {
        gamemode = shareware;
        devparm = true;
        D_AddFile (DEVDATA"doom1.wad");
        D_AddFile (DEVMAPS"data_se/texture1.lmp");
        D_AddFile (DEVMAPS"data_se/pnames.lmp");
        strcpy (configfile,DEVDATA CONFIGFILENAME);
    }
    else
    if (M_CheckParm ("-regdev"))
    {
        gamemode = registered;
        devparm = true;
        D_AddFile (DEVDATA"doom.wad");
        D_AddFile (DEVMAPS"data_se/texture1.lmp");
        D_AddFile (DEVMAPS"data_se/texture2.lmp");
        D_AddFile (DEVMAPS"data_se/pnames.lmp");
        strcpy (configfile,DEVDATA CONFIGFILENAME);
        return;
    }
    else
    if (M_CheckParm ("-comdev"))
    {
        gamemode = commercial;
        devparm = true;
        /* I don't bother
        if(plutonia)
            D_AddFile (DEVDATA"plutonia.wad");
        else if(tnt)
            D_AddFile (DEVDATA"tnt.wad");
        else*/
            D_AddFile (DEVDATA"doom2.wad");

        D_AddFile (DEVMAPS"cdata/texture1.lmp");
        D_AddFile (DEVMAPS"cdata/pnames.lmp");
        strcpy (configfile,DEVDATA CONFIGFILENAME);
        return;
    }
    else
    // specify the name of the IWAD file to use, so we can have several IWAD's
    // in the same directory, and/or have legacy.exe only once in a different location
    if ( (p = M_CheckParm ("-iwad")) && p < myargc-1 )
    {
        sprintf (pathiwad, "%s/%s", doomwaddir, myargv[p+1]);
        if( access (pathiwad,R_OK) ) 
            I_Error("%s not found\n",pathiwad);
        D_AddFile (pathiwad);

        // point to start of filename only
        for (i=strlen(pathiwad)-1; i>=0; i--)
            if (pathiwad[i]=='\\' || pathiwad[i]=='/' || pathiwad[i]==':')
                break;
        i++;

        // find gamemode
        if (!stricmp("plutonia.wad",pathiwad+i))
            gamemode = commercial;
        else if (!stricmp("tnt.wad",pathiwad+i))
            gamemode = commercial;
        else if (!stricmp(text[DOOM2WAD_NUM] ,pathiwad+i))
            gamemode = commercial;
        else if (!stricmp(text[DOOMUWAD_NUM] ,pathiwad+i))
            gamemode = retail;
        else if (!stricmp(text[DOOMWAD_NUM] ,pathiwad+i))
            gamemode = GetDoomVersion (pathiwad);
        else if (!stricmp(text[DOOM1WAD_NUM] ,pathiwad+i))
            gamemode = shareware;
        else {
            gamemode = commercial;
        }
    }
    else
    /*
    if ( !access (doom2fwad,R_OK) )
    {
        gamemode = commercial;
        // C'est ridicule!
        // Let's handle languages in config files, okay?
        language = french;
        CONS_Printf("French version\n");
        D_AddFile (doom2fwad);
    }
    else*/
    if ( !access (doom2wad,R_OK) )
    {
        gamemode = commercial;
        D_AddFile (doom2wad);
    }
    else
    if ( !access (doomuwad,R_OK) )
    {
        gamemode = retail;
        D_AddFile (doomuwad);
    }
    else
    if ( !access (doomwad,R_OK) )
    {
        gamemode = GetDoomVersion (doomwad);
        D_AddFile (doomwad);
    }
    else
    if ( !access (doom1wad,R_OK) )
    {
        gamemode = shareware;
        D_AddFile (doom1wad);
    }
    else
    if ( !access (plutoniawad, R_OK ) )
    {
      gamemode = commercial;
      D_AddFile (plutoniawad);
    }
    else
    if ( !access ( tntwad, R_OK ) )
    {
      gamemode = commercial;
      D_AddFile (tntwad);
    }
    else
    {
        I_Error ("SRB2.SRB not found!\n"); // Tails 12-23-2001
        // unused code : I_ERROR never return
        gamemode = indetermined;
    }

//    D_AddFile(legacywad);
	// Add the players Tails 12-24-2001
	D_AddFile("sonic.plr");
	D_AddFile("tails.plr");
	D_AddFile("knux.plr");
}


/* ======================================================================== */
// Just print the nice red titlebar like the original DOOM2 for DOS.
/* ======================================================================== */
#ifdef PC_DOS
void D_Titlebar (char *title1, char *title2)
{
    // DOOM LEGACY banner
    clrscr();
    textattr((BLUE<<4)+WHITE);
    clreol();
    cputs(title1);

    // standard doom/doom2 banner
    textattr((RED<<4)+WHITE);
    clreol();
    gotoxy((80-strlen(title2))/2,2);
    cputs(title2);
    normvideo();
    gotoxy(1,3);

}
#endif


//added:11-01-98:
//
//  Center the title string, then add the date and time of compilation.
//
void D_MakeTitleString( char *s )
{
    char    temp[82];
    char    *t;
    char    *u;
    int     i;

    for(i=0,t=temp;i<82;i++) *t++=' ';

    for(t=temp+(80-strlen(s))/2,u=s;*u!='\0';)
        *t++ = *u++;

    u=__DATE__;
    for(t=temp+1,i=11;i--;)
        *t++=*u++;
    u=__TIME__;
    for(t=temp+71,i=8;i--;)
        *t++=*u++;

    temp[80]='\0';
    strcpy(s,temp);
}

void D_CheckWadVersion()
{
    int wadversion=0;
    int lump;

    // check main iwad using demo1 version 
    lump = W_CheckNumForNameFirst("demo1");
    // well no demo1, this is not a main wad file
    if(lump == -1)
        I_Error("%s is not a Main wad file (IWAD)\n"
                "try with Doom.wad or Doom2.wad\n"
                "\n"
                "Use -nocheckwadversion to remove this check,\n"
                "but this can cause Legacy to hang\n",wadfiles[0]->filename);
    W_ReadLumpHeader (lump,&wadversion,1);
    if( wadversion<109 )
        I_Error("Your %s file is version %d.%d\n"
                "Doom Legacy need version 1.9\n"
                "Upgrade your version to 1.9 using IdSofware patch\n"
                "\n"
                "Use -nocheckwadversion to remove this check,\n"
                "but this can cause Legacy to hang\n",wadfiles[0]->filename,wadversion/100,wadversion%100);
    
    // check version, of doom3.wad using version lump
    lump=W_CheckNumForName("version");
    if(lump==-1)
        wadversion=0; // or less
    else
    {
        char s[128];
        int  l;
        l=W_ReadLumpHeader (lump,&s,128);
        wadversion=0;
        if( l<128 )
        {
            s[l]='\0';
            if( sscanf(s,"Doom Legacy WAD V%d.%d",&l,&wadversion)==2 )
                wadversion+=l*100;
        }
    }
    if(wadversion!=VERSION)
        I_Error("Your Doom3.wad file is wrong version\n"
                "Use the Doom3.wad comming from the same zip file of this exe\n"
                "\n"
                "Use -nocheckwadversion to remove this check,\n"
                "but this can cause Legacy to hang\n",wadfiles[0]->filename,wadversion/100,wadversion%100);
}


//
// D_DoomMain
//
void D_DoomMain (void)
{
    int     p;
    char    file[256];
    char    legacy[82];    //added:18-02-98: legacy title banner
    char    title[82];    //added:11-01-98:moved, doesn't need to be global

    int     startepisode;
    int     startmap;
    boolean autostart;
	time_t t1; // Date-checker Tails 11-15-2001
	struct tm *tptr; // Date-checker Tails 11-15-2001

    // BP: set correct localtime so time comparison work !
    D_NetFileInit();

    //added:18-02-98:keep error messages until the final flush(stderr)
    if (setvbuf(stderr,NULL,_IOFBF,1000))
        CONS_Printf("setvbuf didnt work\n");

    // get parameters from a response file (eg: doom3 @parms.txt)
    M_FindResponseFile ();

    // identify the main IWAD file to use
    IdentifyVersion ();

    setbuf (stdout, NULL);      // non-buffered output
    modifiedgame = false;

    devparm = M_CheckParm ("-debug"); // Tails 06-04-2000

    // added 18-1-98
    // load dehacked file
    //befor any initialitation patch table and text
    p = M_CheckParm ("-dehacked");
    if (!p)
        p = M_CheckParm ("-deh");  //Fab:02-08-98:like Boom & DosDoom
    if(p!=0)
    {
        while (M_IsNextParm())
            DEH_LoadDehackedFile (M_GetNextParm());
    }

    // search for a deh in -file parm....
    if (M_CheckParm ("-file"))
    {
        while (M_IsNextParm())
        {
            char *s=M_GetNextParm();
            if(stricmp(&s[strlen(s)-3],"deh")==0)
                DEH_LoadDehackedFile (s);
        }
    }

    nomonsters = M_CheckParm ("-noenemies");

    //added:11-01-98:removed the repeated spaces in title strings,
    //               because GCC doesn't expand the TABS from my text editor.
    //  Now the string is centered in a larger one just before output,
    //  and the date and time of compilation is added. (see below)
    switch ( gamemode )
    {
      case retail    :strcpy (title,"The Ultimate DOOM Startup");  break;
      case shareware :strcpy (title,"DOOM Shareware Startup");     break;
      case registered:strcpy (title,"DOOM Registered Startup");    break;
      case commercial:strcpy (title,"Sonic Robo Blast 2");      break; // Tails 03-26-2001
/*FIXME
      case pack_plut :strcpy (title,"DOOM 2: Plutonia Experiment");break;
      case pack_tnt  :strcpy (title,"DOOM 2: TNT - Evilution");    break;
*/
      default        :strcpy (title,"Public DOOM");                break;
    }

    //added:11-01-98:center the string, add compilation time and date.
    sprintf(legacy,"Sonic Robo Blast 2"); // Tails 03-26-2001
    D_MakeTitleString(legacy);

#ifdef PC_DOS
    D_Titlebar(legacy,title);
#else
    CONS_Printf ("%s\n%s\n",legacy,title);
#endif

#ifdef __OS2__
      // set PM window title
   snprintf( pmData->title, sizeof( pmData->title), 
             "Doom LEGACY v%i.%i" VERSIONSTRING ": %s",
             VERSION/100, VERSION%100, title);
#endif

    if (devparm)
        CONS_Printf(D_DEVSTR);

    // default savegame
    strcpy(savegamename,text[NORM_SAVEI_NUM]);

    {
        char *userhome,legacyhome[256];
        if(M_CheckParm("-home") && M_IsNextParm())
            userhome = M_GetNextParm();
        else
            userhome = getenv("HOME");
#ifdef LINUX
        if (!userhome)
            I_Error("Please set $HOME to your home directory\n");
#endif  
        if(userhome)
        {
    // use user specific config file
    sprintf(legacyhome, "%s/"DEFAULTDIR, userhome);
    sprintf(configfile, "%s/"CONFIGFILENAME, legacyhome);
            // can't use sprintf since there is %d in savegamename
            strcatbf(savegamename,legacyhome,"/");
            I_mkdir(legacyhome, 0700);
        }
    }                       

    if (M_CheckParm("-cdrom"))
    {
        CONS_Printf(D_CDROM);
        I_mkdir("c:\\doomdata",0);
        strcpy (configfile,"c:/doomdata/"CONFIGFILENAME);
        strcpy (savegamename,text[CDROM_SAVEI_NUM]);
    }

    // add any files specified on the command line with -file wadfile
    // to the wad list
    //
    // convenience hack to allow -wart e m to add a wad file
    // prepend a tilde to the filename so wadfile will be reloadable
    p = M_CheckParm ("-wart");
    if (p)
    {
        myargv[p][4] = 'p';     // big hack, change to -warp

        // Map name handling.
        switch (gamemode )
        {
          case shareware:
          case retail:
          case registered:
            sprintf (file,"~"DEVMAPS"E%cM%c.wad",
                     myargv[p+1][0], myargv[p+2][0]);
            CONS_Printf("Warping to Episode %s, Map %s.\n",
                   myargv[p+1],myargv[p+2]);
            break;

          case commercial:
          default:
            p = atoi (myargv[p+1]);
            if (p<10)
              sprintf (file,"~"DEVMAPS"cdata/map0%i.wad", p);
            else
              sprintf (file,"~"DEVMAPS"cdata/map%i.wad", p);
            break;
        }
        D_AddFile (file);
    }

// Do special stuff around Christmas time and New Years Tails 11-15-2001
	if( (t1 = time( (time_t *) 0 )) != (time_t) -1 )
	{
		tptr = localtime(&t1);
		if(tptr->tm_mon < 1 && tptr->tm_mday == 1) // New Year's Day
		{
			D_AddFile("menuitem.dll");
			mariomode = true;
		}
		else if((tptr->tm_mon < 1 && tptr->tm_mday < 6) || (tptr->tm_mon == 11 && tptr->tm_mday > 24)) // Christmas to Epiphany
		{
			D_AddFile("3drend.dll");
			xmasmode = true;
		}
	}

    if (M_CheckParm ("-xmas"))
	{
		D_AddFile("3drend.dll");
		xmasmode = true;
	}

    if (M_CheckParm ("-mario"))
	{
		D_AddFile("menuitem.dll");
		mariomode = true;
	}

    if (M_CheckParm ("-file"))
    {
        // the parms after p are wadfile/lump names,
        // until end of parms or another - preceded parm
        modifiedgame = true;            // homebrew levels
        while (M_IsNextParm())
        {
            char *s=M_GetNextParm();
            // deh file are loaded before
            if(stricmp(&s[strlen(s)-3],"deh")!=0)
                D_AddFile (s);
        }
    }


    // get skill / episode / map from parms
    gameskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;


    p = M_CheckParm ("-skill");
    if (p && p < myargc-1)
    {
        gameskill = myargv[p+1][0]-'1';
            autostart = true;
    }

    p = M_CheckParm ("-episode");
    if (p && p < myargc-1)
    {
        startepisode = myargv[p+1][0]-'0';
        startmap = 1;
        autostart = true;
    }

    p = M_CheckParm ("-warp");
    if (p && p < myargc-1)
    {
        if (gamemode == commercial)
            startmap = atoi (myargv[p+1]);
        else
        {
            startepisode = myargv[p+1][0]-'0';
            if (p < myargc-2 &&
                myargv[p+2][0]>='0' &&
                myargv[p+2][0]<='9' )
                startmap = myargv[p+2][0]-'0';
            else
                startmap = 1;
        }
        autostart = true;
    }

    //BP: get 2 megs more for textures cache
    if(M_CheckParm ("-opengl") || M_CheckParm ("-3dfx") || M_CheckParm ("-d3d") || M_CheckParm ("-minigl"))
        mb_used+=2;

    //added:11-02-98: TEMPORARY HACK to modify the base memory allocation
    //                later we'll have to do a better memory handling,
    //                with detection of available mem and w95 support.
    p = M_CheckParm ("-mb");
    if (p && p < myargc-1)
    {
        mb_used = atoi (myargv[p+1]);
        CONS_Printf ("%d megabytes requested for Z_Init.\n", mb_used);
    }

    CONS_Printf (text[Z_INIT_NUM]);
    Z_Init ();

    CONS_Printf (text[W_INIT_NUM]);
    // load wad, including the main wad file
    if(!W_InitMultipleFiles (startupwadfiles))
        CONS_Error("A WAD file was not found\n");

    if(!M_CheckParm("-nocheckwadversion"))
        D_CheckWadVersion();

    //added:28-02-98: check for Ultimate doom.
//    if ( (gamemode==registered) && (W_CheckNumForName("E4M1") > 0) )
//        gamemode = retail;

    // Check for -file in shareware
    if (modifiedgame)
    {
        // These are the lumps that will be checked in IWAD,
        // if any one is not present, execution will be aborted.
        char name[23][8]=
        {
            "e2m1","e2m2","e2m3","e2m4","e2m5","e2m6","e2m7","e2m8","e2m9",
            "e3m1","e3m3","e3m3","e3m4","e3m5","e3m6","e3m7","e3m8","e3m9",
            "dphoof","bfgga0","heada1","cybra1","spida1d1"
        };
        int i;

        if ( gamemode == shareware)
            I_Error("\nYou cannot -file with the shareware "
                    "version. Register!");

        // Check for fake IWAD with right name,
        // but w/o all the lumps of the registered version.
        if (gamemode == registered)
            for (i = 0;i < 23; i++)
                if (W_CheckNumForName(name[i])<0)
                    I_Error("\nThis is not the registered version.");
    }

    // If additonal PWAD files are used, print modified banner
    if (modifiedgame)
        CONS_Printf ( text[MODIFIED_NUM] );


    // Check and print which version is executed.
    switch ( gamemode )
    {
      case shareware:
      case indetermined:
        CONS_Printf (text[SHAREWARE_NUM]);
        break;
      case registered:
      case retail:
      case commercial:
        CONS_Printf (text[COMERCIAL_NUM]);
        break;
      default:
        // Ouch.
        break;
    }

    //SoM: 4/4/2000: INIT DEHACKED LUMPS!
    W_LoadDehackedLumps();

   //---------------------------------------------------- READY SCREEN
    //printf("\nI_StartupComm...");

    CONS_Printf("I_StartupTimer...\n");
    I_StartupTimer ();

    // now initted automatically by use_mouse var code
    //CONS_Printf("I_StartupMouse...\n");
    //I_StartupMouse ();

    //CONS_Printf ("I_StartupKeyboard...\n");
    //I_StartupKeyboard (); // FIXME: this is a dummy, we can remove it!

    // now initialised automatically by use_joystick var code
    //CONS_Printf (text[I_INIT_NUM]);
    //I_InitJoystick ();

    CONS_Printf("I_StartupGraphics...\n");
    I_StartupGraphics ();

   //--------------------------------------------------------- CONSOLE
    // setup loading screen
    SCR_Startup ();

    // we need the font of the console
    CONS_Printf (text[HU_INIT_NUM]);
    HU_Init ();

    COM_Init ();
    CON_Init ();

    D_RegisterClientCommands ();
    D_AddDeathmatchCommands ();
    ST_AddCommands ();
    R_RegisterEngineStuff ();
    S_RegisterSoundStuff ();
    CV_RegisterVar (&cv_screenslink);

    //Fab:29-04-98: do some dirty chatmacros strings initialisation
    HU_HackChatmacros ();
  //--------------------------------------------------------- CONFIG.CFG
    M_FirstLoadConfig(); // WARNING : this do a "COM_BufExecute()"

#ifdef LINUX
    VID_PrepareModeList(); // Regenerate Modelist according to cv_fullscreen
#endif

    // set user default mode or mode set at cmdline
    SCR_CheckDefaultMode ();

    wipegamestate = gamestate;
  //------------------------------------------------ COMMAND LINE PARAMS

// check for gametype definition Tails 05-19-2001
	p = M_CheckParm ("-gametype");
	if(p && p < myargc-1)
	{
		COM_BufAddText ("gametype ");
		COM_BufAddText (myargv[p+1]);
		COM_BufAddText ("\n");
	}

    if (M_CheckParm ("-autoctf"))   COM_BufAddText ("autoctf yes\n"); // Tails 08-04-2001

p = M_CheckParm ("-ctfteam"); // Tails 08-04-2001
    if (p && p < myargc-1) // Tails 08-04-2001
    { // Tails 08-04-2001
		ctfteam = myargv[p+1]; // Tails 08-04-2001
		COM_BufAddText("preferredteam \""); // Tails 08-04-2001
		COM_BufAddText(ctfteam); // Tails 08-04-2001
		COM_BufAddText("\"\n"); // Tails 08-04-2001
    } // Tails 08-04-2001

// Joystick fun! Tails 06-13-2001
	p = M_CheckParm ("-joystick");
	if(p && p < myargc-1)
	{
		COM_BufAddText ("use_joystick ");
		COM_BufAddText (myargv[p+1]);
		COM_BufAddText ("\n");
	}
// Maxplayers setting Tails 06-10-2001
	p = M_CheckParm ("-maxplayers");
	if(p && p < myargc-1)
	{
		COM_BufAddText ("sv_maxplayers ");
		COM_BufAddText (myargv[p+1]);
		COM_BufAddText ("\n");
	}

    // Initialize CD-Audio
    if (!M_CheckParm ("-nocd"))        I_InitCD ();
    if (M_CheckParm ("-respawn"))      COM_BufAddText ("respawnmonsters 1\n");
    if (M_CheckParm("-teamplay"))      COM_BufAddText ("teamplay 1\n");
    if (M_CheckParm("-teamskin"))      COM_BufAddText ("teamplay 2\n");
    if (M_CheckParm("-splitscreen"))   CV_SetValue(&cv_splitscreen,1);
    if (M_CheckParm ("-altdeath"))     COM_BufAddText ("deathmatch 2\n");
    else 
    if (M_CheckParm ("-deathmatch"))   COM_BufAddText ("deathmatch 1\n");

    if (M_CheckParm ("-fast"))         COM_BufAddText ("fastmonsters 1\n");

    if (M_CheckParm ("-analog"))         COM_BufAddText ("analog 1\n"); // Tails

    if (M_CheckParm ("-timer"))
    {
        char *s=M_GetNextParm();
        COM_BufAddText(va("timelimit %s\n",s ));
//        CONS_Printf("Levels will end after %s minute(s).\n",s);
    }

    if (M_CheckParm ("-avg"))
    {
        COM_BufAddText("timelimit 20\n");
        CONS_Printf(text[AUSTIN_NUM]);
    }

	// Eliminated Turbo Tails 06-06-2001
/*
    // turbo option, is not meant to be saved in config, still
    // supported at cmd-line for compatibility
    if ( M_CheckParm ("-turbo") && M_IsNextParm())
        COM_BufAddText (va("turbo %s\n",M_GetNextParm()));
*/
    // push all "+" parameter at the command buffer
    M_PushSpecialParameters();

    CONS_Printf (text[M_INIT_NUM]);
    M_Init ();

    CONS_Printf (text[R_INIT_NUM]);
    R_Init ();

    //
    // setting up sound
    //
    CONS_Printf (text[S_SETSOUND_NUM]);
    nosound = M_CheckParm("-nosound");
    nomusic = M_CheckParm("-nomusic"); // WARNING: DOS version initmusic in I_StartupSound
    I_StartupSound ();
    I_InitMusic ();  // setup music buffer for quick mus2mid
    S_Init (cv_soundvolume.value, cv_musicvolume.value);

    CONS_Printf (text[ST_INIT_NUM]);
    ST_Init ();

    // init all NETWORK
    CONS_Printf (text[D_CHECKNET_NUM]);
    D_CheckNetGame ();

    // check for a driver that wants intermission stats
    p = M_CheckParm ("-statcopy");
    if (p && p<myargc-1)
    {
        I_Error("Sorry but statcopy isn't supported at this time\n");
        /*
        // for statistics driver
        extern  void*   statcopy;

        statcopy = (void*)atoi(myargv[p+1]);
        CONS_Printf (text[STATREG_NUM]);
        */
    }

    // start the apropriate game based on parms
    p = M_CheckParm ("-record");
    if (p && p < myargc-1)
    {
        G_RecordDemo (myargv[p+1]);
        autostart = true;
    }

	// Start "TimeTic" option Tails 04-01-2001
	p = M_CheckParm ("-timetic");
	if(p)
	{
		cv_timetic.value = true;
	}


    // demo doesn't need anymore to be added with D_AddFile()
    p = M_CheckParm ("-playdemo");
    if (!p)
        p = M_CheckParm ("-timedemo");
    if (p && M_IsNextParm())
    {
        char tmp[MAX_WADPATH];
        // add .lmp to identify the EXTERNAL demo file
        // it is NOT possible to play an internal demo using -playdemo,
        // rather push a playdemo command.. to do.

        strcpy (tmp,M_GetNextParm());
        // get spaced filename or directory
        while(M_IsNextParm()) { strcat(tmp," ");strcat(tmp,M_GetNextParm()); }
        FIL_DefaultExtension (tmp,".lmp");

        CONS_Printf ("Playing demo %s.\n",tmp);

        if ( (p=M_CheckParm("-playdemo")) )
        {
            singledemo = true;              // quit after one demo
            G_DeferedPlayDemo (tmp);
        }
        else
            G_TimeDemo (tmp);
        gamestate = wipegamestate = GS_NULL;

        return;         
    }

    p = M_CheckParm ("-loadgame");
    if (p && p < myargc-1)
    {
        G_LoadGame (atoi(myargv[p+1]));
    }
    else
    {
        if(dedicated && server)
        {
            pagename = "TITLEPIC";
            gamestate = GS_DEDICATEDSERVER;
        }
        else
            if (autostart || netgame)
            {
                //added:27-02-98: reset the current version number
                G_Downgrade(VERSION);
                gameaction = ga_nothing;
                usergame = true;
                if(server && !M_CheckParm("+map"))
                   COM_BufAddText (va("map \"%s\"\n",G_BuildMapName(startepisode, startmap)));
            }
            else
                D_StartTitle ();                // start up intro loop

    }
}
