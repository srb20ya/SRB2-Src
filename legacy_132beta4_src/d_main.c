// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_main.c,v 1.53 2001/12/31 16:56:39 metzgermeister Exp $
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
// Revision 1.53  2001/12/31 16:56:39  metzgermeister
// see Dec 31 log
// .
//
// Revision 1.52  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.51  2001/08/12 15:21:03  bpereira
// see my log
//
// Revision 1.50  2001/07/16 22:35:40  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.49  2001/05/27 13:42:47  bpereira
// no message
//
// Revision 1.48  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.47  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.46  2001/04/27 13:32:13  bpereira
// no message
//
// Revision 1.45  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.44  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.43  2001/04/02 18:54:32  bpereira
// no message
//
// Revision 1.42  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.41  2001/03/30 17:12:49  bpereira
// no message
//
// Revision 1.40  2001/03/19 18:25:02  hurdler
// Is there a GOOD reason to check for modified game with shareware version?
//
// Revision 1.39  2001/03/03 19:43:09  ydario
// OS/2 code cleanup
//
// Revision 1.38  2001/02/24 13:35:19  bpereira
// no message
//
// Revision 1.37  2001/02/10 12:27:13  bpereira
// no message
//
// Revision 1.36  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.35  2000/11/06 20:52:15  bpereira
// no message
//
// Revision 1.34  2000/11/03 03:27:17  stroggonmeth
// Again with the bug fixing...
//
// Revision 1.33  2000/11/02 19:49:35  bpereira
// no message
//
// Revision 1.32  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.31  2000/10/21 08:43:28  bpereira
// no message
//
// Revision 1.30  2000/10/08 13:29:59  bpereira
// no message
//
// Revision 1.29  2000/10/02 18:25:44  bpereira
// no message
//
// Revision 1.28  2000/10/01 10:18:16  bpereira
// no message
//
// Revision 1.27  2000/09/28 20:57:14  bpereira
// no message
//
// Revision 1.26  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.25  2000/08/29 15:53:47  hurdler
// Remove master server connect timeout on LAN (not connected to Internet)
//
// Revision 1.24  2000/08/21 21:13:00  metzgermeister
// Implementation of I_GetKey() in Linux
//
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
#include "p_info.h"

#include "r_main.h"
#include "r_local.h"

#include "s_sound.h"
#include "st_stuff.h"

#include "t_script.h"

#include "v_video.h"

#include "wi_stuff.h"
#include "w_wad.h"

#include "z_zone.h"
#include "d_main.h"
#include "d_netfil.h"
#include "m_cheat.h"

#include "time.h" // Tails 11-15-2001

#ifdef HWRENDER
#include "hardware/hw_main.h"   // 3D View Rendering
#endif

#include "hardware/hw3sound.h"

//
//  DEMO LOOP
//
int             demosequence;
int             pagetic;
char            *pagename="MAP1PIC"; // Tails

//  PROTOS
void D_PageDrawer (char* lumpname);
void D_AdvanceDemo (void);

#ifdef LINUX
void VID_PrepareModeList(void); // FIXME: very dirty; will use a proper include file
#endif

char*           startupwadfiles[MAX_WADFILES];

extern consvar_t debug; // Tails
boolean         devparm;        // started game with -devparm
//boolean         nomonsters;     // checkparm of -nomonsters
boolean			xmasmode; // Xmas Mode Tails 12-02-2001
boolean         xmasoverride;
boolean			mariomode; // Mario Mode Tails 12-18-2001
boolean         eastermode; // Easter mode Tails 02-06-2003

char*				parmskin; // Player skin defined from parms Tails 06-09-2001
char*				ctfteam; // Player Preferred CTF Team defined from parms Tails 07-31-2001
char*				flagtime; // CTF Flag time defined from parms Tails 07-31-2001
char*				parmcolor; // Player color defined from parms Tails 06-09-2001
char*				parmname; // Player name defined from parms Tails 06-09-2001

boolean         singletics = false; // timedemo

boolean         nomusic;    
boolean         nosound;
boolean         nofmod; // No OGG-based music Tails 11-21-2002


boolean         advancedemo;


char            wadfile[1024];          // primary wad file
char            mapdir[1024];           // directory of development maps

//
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
// referenced from i_system.c for I_GetKey()

event_t         events[MAXEVENTS];
int             eventhead;
int             eventtail;

boolean dedicated;

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

void D_Display (void)
{
    static  boolean             menuactivestate = false;
    static  gamestate_t         oldgamestate = -1;
    static  int                 borderdrawcount;
    tic_t                       nowtime;
    tic_t                       tics;
    tic_t                       wipestart;
    int                         y;
    boolean                     done;
    boolean                     wipe;
    boolean                     redrawsbar;
    boolean                     viewactivestate = false;

    if (nodrawers)
        return;                    // for comparative timing / profiling

    redrawsbar = false;

    //added:21-01-98: check for change of screen size (video mode)
    if (setmodeneeded)
        SCR_SetMode();  // change video mode

    if (vid.recalc)
        //added:26-01-98: NOTE! setsizeneeded is set by SCR_Recalc()
        SCR_Recalc();

    // change the view size if needed
    if( setsizeneeded )
    {
        R_ExecuteSetViewSize ();
        oldgamestate = -1;                      // force background redraw
        borderdrawcount = 3;
        redrawsbar = true;
    }

    // save the current screen if about to wipe
    if (gamestate != wipegamestate)
    {
        wipe = true;

		if(rendermode == render_soft)
			wipe_StartScreen(0, 0, vid.width, vid.height);
    }
    else
        wipe = false;

	// draw buffered stuff to screen
    // BP: Used only by linux GGI version
    I_UpdateNoBlit ();

	// Fade to black first Tails 06-07-2002
	if(rendermode == render_soft)
	{
		if(wipe)
		{
			V_DrawFill(0,0,vid.width,vid.height,0);
			wipe_EndScreen(0, 0, vid.width, vid.height);

			wipestart = I_GetTime () - 1;
			y=wipestart+2*TICRATE; // init a timeout
			do
			{
				do
				{
					nowtime = I_GetTime ();
					tics = nowtime - wipestart;
				} while (!tics);
				wipestart = nowtime;
				done = wipe_ScreenWipe (wipe_ColorXForm
										, 0, 0, vid.width, vid.height, tics);
				I_OsPolling ();
				I_UpdateNoBlit ();
				M_Drawer ();            // menu is drawn even on top of wipes
				I_FinishUpdate ();      // page flip or blit buffer
			} while (!done && I_GetTime()<(unsigned)y);
		}

		wipe_StartScreen(0, 0, vid.width, vid.height);
	}
	else if(wipe) // Delay the hardware modes as well
	{
		tic_t                       nowtime;
		tic_t                       tics;
		tic_t                       wipestart;
		int                         y;

		wipestart = I_GetTime () - 1;
		y=wipestart+32; // init a timeout
		do
		{
			do
			{
				nowtime = I_GetTime ();
				tics = nowtime - wipestart;
			} while (!tics);

			I_OsPolling ();
			I_UpdateNoBlit ();
			M_Drawer ();            // menu is drawn even on top of wipes
			I_FinishUpdate ();      // page flip or blit buffer
		} while (I_GetTime()<(unsigned)y);
	}

    // do buffered drawing
    switch (gamestate)
    {
      case GS_LEVEL:
        if (!gametic)
            break;
        HU_Erase();
        if (automapactive)
            AM_Drawer ();
        if (wipe || menuactivestate
#ifdef HWRENDER
                 || rendermode != render_soft
#endif
                 || vid.recalc)
            redrawsbar = true;
        break;

      case GS_INTERMISSION:
        WI_Drawer ();
		HU_Erase();
		HU_Drawer ();
        break;

      case GS_FINALE:
        F_Drawer ();
        break;

	  case GS_INTRO: // Tails 02-15-2002
	  case GS_INTRO2: // Intro Tails 02-15-2002
		F_IntroDrawer();
		break;

	  case GS_CUTSCENE:
		  F_CutsceneDrawer();
		  break;

	  case GS_DEMOEND: // Tails 09-01-2002
		F_DemoEndDrawer();
		break;

	  case GS_EVALUATION:
		F_GameEvaluationDrawer();
		break;

	  case GS_CREDITS:
		F_CreditDrawer(); // Tails 05-05-2003
		break;

	  case GS_TITLESCREEN:
		F_TitleScreenDrawer(); // Tails 11-30-2002
		break;

      case GS_DEDICATEDSERVER:
      case GS_DEMOSCREEN:
        D_PageDrawer (pagename);
      case GS_WAITINGPLAYERS:
      case GS_NULL:
        break;
    }

	// Transitions for Introduction Tails 02-16-2002
	if(gamestate == GS_INTRO && oldgamestate == GS_INTRO2)
		wipe = true;
	else if(gamestate == GS_INTRO2 && oldgamestate == GS_INTRO)
		wipe = true;

    // clean up border stuff
    // see if the border needs to be initially drawn
    if (gamestate == GS_LEVEL) 
    {
        if( oldgamestate != GS_LEVEL )
        {
            viewactivestate = false;        // view was not active
            R_FillBackScreen ();    // draw the pattern into the back screen
        }

        // see if the border needs to be updated to the screen
        if( !automapactive && (scaledviewwidth!=vid.width) )
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

        // draw the view directly
        if( !automapactive && !dedicated)
        {
            if( players[displayplayer].mo )
            {
#ifdef CLIENTPREDICTION2
                players[displayplayer].mo->flags2 |= MF2_DONTDRAW;
#endif
#ifdef HWRENDER 
                if( rendermode != render_soft )
                    HWR_RenderPlayerView (0, &players[displayplayer]);
                else //if (rendermode == render_soft)
#endif
                    R_RenderPlayerView (&players[displayplayer]);
#ifdef CLIENTPREDICTION2
                players[displayplayer].mo->flags2 &=~MF2_DONTDRAW;
#endif
            }

            // added 16-6-98: render the second screen
            if( secondarydisplayplayer != consoleplayer && players[secondarydisplayplayer].mo)
            {
#ifdef CLIENTPREDICTION2
                players[secondarydisplayplayer].mo->flags2 |= MF2_DONTDRAW;
#endif
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
#ifdef CLIENTPREDICTION2
                players[secondarydisplayplayer].mo->flags2 &=~MF2_DONTDRAW;
#endif
            }
        }

        ST_Drawer (redrawsbar);

        HU_Drawer ();
    }

    // change gamma if needed
    if (gamestate != oldgamestate && gamestate != GS_LEVEL) 
        V_SetPalette (0);

    menuactivestate = menuactive;
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

    //FIXME: draw either console or menu, not the two
    CON_Drawer ();

    M_Drawer ();          // menu is drawn even on top of everything
    NetUpdate ();         // send out any new accumulation

//
// normal update
//
    if (!wipe)
    {
        if( cv_netstat.value )
        {
            char s[50];
            Net_GetNetStat();
            sprintf(s,"get %d b/s",getbps);
            V_DrawString(BASEVIDWIDTH-V_StringWidth (s),BASEVIDHEIGHT-ST_HEIGHT-40, V_WHITEMAP, s);
            sprintf(s,"send %d b/s",sendbps);
            V_DrawString(BASEVIDWIDTH-V_StringWidth (s),BASEVIDHEIGHT-ST_HEIGHT-30, V_WHITEMAP, s);
            sprintf(s,"GameMiss %.2f%%",gamelostpercent);
            V_DrawString(BASEVIDWIDTH-V_StringWidth (s),BASEVIDHEIGHT-ST_HEIGHT-20, V_WHITEMAP, s);
            sprintf(s,"SysMiss %.2f%%",lostpercent);
            V_DrawString(BASEVIDWIDTH-V_StringWidth (s),BASEVIDHEIGHT-ST_HEIGHT-10, V_WHITEMAP, s);
        }

#ifdef TILTVIEW
        //added:12-02-98: tilt view when marine dies... just for fun
        if (gamestate == GS_LEVEL &&
            cv_tiltview.value &&
            players[displayplayer].playerstate==PST_DEAD )
        {
            V_DrawTiltView (screens[0]);
        }
        else
#endif
#ifdef PERSPCORRECT
        if (gamestate == GS_LEVEL &&
                 cv_perspcorr.value )
        {
            V_DrawPerspView (screens[0], players[displayplayer].aiming);
        }
        else
#endif
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

	if(rendermode == render_soft)
	{
		wipe_EndScreen(0, 0, vid.width, vid.height);

		wipestart = I_GetTime () - 1;
		y=wipestart+2*TICRATE; // init a timeout
		do
		{
			do
			{
				nowtime = I_GetTime ();
				tics = nowtime - wipestart;
			} while (!tics);
			wipestart = nowtime;
			done = wipe_ScreenWipe (wipe_ColorXForm
									, 0, 0, vid.width, vid.height, tics);
			I_OsPolling ();
			I_UpdateNoBlit ();
			M_Drawer ();            // menu is drawn even on top of wipes
			I_FinishUpdate ();      // page flip or blit buffer
		} while (!done && I_GetTime()<(unsigned)y);
	}
	else // Delay the hardware modes as well
	{
		tic_t                       nowtime;
		tic_t                       tics;
		tic_t                       wipestart;
		int                         y;

		wipestart = I_GetTime () - 1;
		y=wipestart+32; // init a timeout
		do
		{
			do
			{
				nowtime = I_GetTime ();
				tics = nowtime - wipestart;
			} while (!tics);

			I_OsPolling ();
			I_UpdateNoBlit ();
			M_Drawer ();            // menu is drawn even on top of wipes
			I_FinishUpdate ();      // page flip or blit buffer
		} while (I_GetTime()<(unsigned)y);
	}
}



// =========================================================================
//   D_DoomLoop
// =========================================================================

tic_t   rendergametic,oldentertics;
boolean supdate;
int p; // Tails 06-10-2001

//#define SAVECPU_EXPERIMENTAL

void D_DoomLoop (void)
{
    tic_t oldentertics,entertic,realtics,rendertimeout=-1;

    if (demorecording)
        G_BeginRecording ();

    // user settings
    COM_BufAddText ("exec autoexec.cfg\n");

	if(dedicated)
		COM_BufAddText("dedicated on\n");

	if(M_CheckParm ("-voodoo")) // 256x256 Texture Limiter
		COM_BufAddText("gr_voodoocompatibility on\n");

// Skin, name, and color stuff Tails 06-10-2001
p = M_CheckParm ("-skin");
    if (p && p < myargc-1)
    {
		parmskin = myargv[p+1];
		COM_BufAddText("skin \"");
		COM_BufAddText(parmskin);
		COM_BufAddText("\"\n");

		if(!netgame && modifiedgame)
			SetPlayerSkin(0, parmskin);
    }

p = M_CheckParm ("-color");
    if (p && p < myargc-1)
    {
		parmcolor = myargv[p+1];
		COM_BufAddText("color \"");
		COM_BufAddText(parmcolor);
		COM_BufAddText("\"\n");
    }

p = M_CheckParm ("-skin2");
    if (p && p < myargc-1)
    {
		parmskin = myargv[p+1];
		COM_BufAddText("skin2 \"");
		COM_BufAddText(parmskin);
		COM_BufAddText("\"\n");
    }

p = M_CheckParm ("-color2");
    if (p && p < myargc-1)
    {
		parmcolor = myargv[p+1];
		COM_BufAddText("color2 \"");
		COM_BufAddText(parmcolor);
		COM_BufAddText("\"\n");
    }

p = M_CheckParm ("-flagtime");
    if (p && p < myargc-1)
    {
		flagtime = myargv[p+1];
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

p = M_CheckParm ("-name2");
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
        
#ifdef SAVECPU_EXPERIMENTAL
	if(realtics == 0)
	{
	    usleep(10000);
	    continue;
	}
#endif

        // frame syncronous IO operations
        // UNUSED for the moment (18/12/98)
        I_StartFrame ();

#ifdef HW3SOUND
        HW3S_BeginFrameUpdate();
#endif

        // process tics (but maybe not if realtic==0)
        TryRunTics (realtics);

#ifdef CLIENTPREDICTION2
        if(singletics || supdate)
#else
        if(singletics || gametic>rendergametic)
#endif
        {
            rendergametic=gametic;
            rendertimeout=entertic+TICRATE/17;
            
            // Update display, next frame, with current state.
            D_Display ();
            supdate=false;
        }
        else
            if( rendertimeout < entertic ) // in case the server hang or netsplit
                D_Display ();

        //added:16-01-98:consoleplayer -> displayplayer (hear sounds from viewpoint)
        S_UpdateSounds ();  // move positional sounds
            
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
#ifndef SNDINTR
        // Update sound output.
        I_SubmitSound();
#endif

#endif //__WIN32__
        // check for media change, loop music..
        I_UpdateCD ();

#ifdef HW3SOUND
        HW3S_EndFrameUpdate();
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
    players[consoleplayer].playerstate = PST_LIVE;  // not reborn
    advancedemo = false;
    gameaction = ga_nothing;

    demosequence = 0;//(demosequence+1)%6;

    switch (demosequence)
    {
      case 0:
        pagename = "TITLESKY";
        pagetic = 9999999;
        gamestate = GS_DEMOSCREEN;
        break;
/*      case 1:
        G_DeferedPlayDemo ("titledem");
        pagetic = 9999999;
        break;
      case 2:
        pagetic = 2;
        gamestate = GS_DEMOSCREEN;
        pagename = "BLACK";
        break;
      case 3:
        G_DeferedPlayDemo ("demo1");
        pagetic = 9999999;
        break;
      case 4:
        gamestate = GS_DEMOSCREEN;
        pagetic = 2;
        pagename = "BLACK";
        break;
      case 5:
        G_DeferedPlayDemo ("titledem");
        pagetic = 9999999;
        break;*/
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
    paused = false;
    advancedemo = false;
    F_StartTitleScreen();
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
#elif !defined( __OS2__)
#define _MAX_PATH   MAX_WADPATH
#endif


// ==========================================================================
// Identify the Doom version, and IWAD file to use.
// ==========================================================================

void IdentifyVersion (void)
{
    char*       doom2wad;

    char        pathtemp[_MAX_PATH];
    char        pathiwad[_MAX_PATH+16];
    int         i;

    char *doomwaddir;

#ifdef LINUX
    // change to the directory where 'doom3.wad' is found
   I_LocateWad();
#endif

    // get the current directory (possible problem on NT with "." as current dir)
    if ( getcwd(pathtemp, _MAX_PATH) != NULL )
	    doomwaddir = pathtemp;
    else
        doomwaddir = ".";
        
#ifdef __MACOS__
        // cwd is always "/" when app is dbl-clicked
        if (!stricmp(doomwaddir,"/"))
                doomwaddir = I_GetWadDir();
#endif
    // Commercial.
    doom2wad = malloc(strlen(doomwaddir)+1+9+1);
    sprintf(doom2wad, "%s/%s", doomwaddir, text[DOOM2WAD_NUM]);

    // will be overwrite in case of -cdrom or linux home
    sprintf(configfile, "%s/"CONFIGFILENAME, doomwaddir);

    // specify the name of the IWAD file to use, so we can have several IWAD's
    // in the same directory, and/or have legacy.exe only once in a different location
    if ( M_CheckParm ("-iwad") )
    {
        // BP: big hack for fullpath wad, we shoudl use wadpath instead in d_addfile
        char *s = M_GetNextParm();
        if( s[0] == '/' || s[0] == '\\' || s[1]==':' )
            sprintf (pathiwad, "%s", s);
        else    
            sprintf (pathiwad, "%s/%s", doomwaddir, s );

        if( access (pathiwad,R_OK) ) 
            I_Error("%s not found\n",pathiwad);
        D_AddFile (pathiwad);

        // point to start of filename only
        for (i=strlen(pathiwad)-1; i>=0; i--)
            if (pathiwad[i]=='\\' || pathiwad[i]=='/' || pathiwad[i]==':')
                break;
        i++;
    }
    else
	{
		if ( !access (doom2wad,R_OK) )
		{
			D_AddFile (doom2wad);
		}
		else if ( !access ("srb2.wad",R_OK) )
		{
			D_AddFile ("srb2.wad");
		}
		else
		{
			I_Error ("SRB2.SRB not found! %s\n", doom2wad); // Tails 12-23-2001
		}
	}

	// Add the players Tails 12-24-2001
	D_AddFile("sonic.plr");
	D_AddFile("tails.plr");
	D_AddFile("knux.plr");

	// Add the weapons
	D_AddFile("auto.wpn");
	D_AddFile("bomb.wpn");
	D_AddFile("home.wpn");
	D_AddFile("rail.wpn");
	D_AddFile("infn.wpn");

	// Add... nights?
	D_AddFile("drill.dta");
	D_AddFile("soar.dta");

	D_AddFile("zim.dta"); // ZIIIIM!
	
	// Don't forget the music!
	D_AddFile("music.dta");
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

void P_InitMapHeaders(void);
void P_ClearMapHeaderInfo(void);
//
// D_DoomMain
//
void D_DoomMain (void)
{
    int     p;
//    char    file[256];
    char    legacy[82];    //added:18-02-98: legacy title banner
    char    title[82];    //added:11-01-98:moved, doesn't need to be global

    int     startepisode;
    int     startmap;
    boolean autostart;
 	time_t t1; // Date-checker Tails 11-15-2001
	struct tm *tptr; // Date-checker Tails 11-15-2001

    //added:18-02-98:keep error messages until the final flush(stderr)
    if (setvbuf(stderr,NULL,_IOFBF,1000))
        CONS_Printf("setvbuf didnt work\n");

    // get parameters from a response file (eg: doom3 @parms.txt)
    M_FindResponseFile ();

	// MAINCFG is now taken care of where "OBJCTCFG" is handled
	G_LoadGameSettings(); // Tails 05-19-2003
        
    // identify the main IWAD file to use
    IdentifyVersion ();

    setbuf (stdout, NULL);      // non-buffered output

	devparm = M_CheckParm ("-debug");

//    nomonsters = M_CheckParm ("-noenemies");

    //added:11-01-98:removed the repeated spaces in title strings,
    //               because GCC doesn't expand the TABS from my text editor.
    //  Now the string is centered in a larger one just before output,
    //  and the date and time of compilation is added. (see below)
    strcpy (title,"Sonic Robo Blast 2"); // Tails 03-26-2001

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

	P_ClearMapHeaderInfo();

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
	    // little hack to allow a different config file for opengl
	    // may be a problem if opengl cannot really be started
	    if(M_CheckParm("-opengl"))
	    {
		sprintf(configfile, "%s/gl"CONFIGFILENAME, legacyhome);
	    }
	    else
	    {
		sprintf(configfile, "%s/"CONFIGFILENAME, legacyhome);
	    }
	    
            // can't use sprintf since there is %d in savegamename
            strcatbf(savegamename,legacyhome,"/");
            I_mkdir(legacyhome, 0700);
        }
    }
	
	// Do special stuff around Christmas time Tails 11-15-2001
	if( (t1 = time( (time_t *) 0 )) != (time_t) -1 )
	{
		if(!M_CheckParm("-noxmas"))
		{
			tptr = localtime(&t1);
			if((tptr->tm_mon < 1 && tptr->tm_mday < 6) || (tptr->tm_mon == 11 && tptr->tm_mday > 24)) // Christmas to Epiphany
			{
				D_AddFile("3drend.dll");
				xmasmode = true;
				xmasoverride = true;
				modifiedgame = false;
			}
		}
	}

	// Do special stuff for Eastertime!
	if( (t1 = time( (time_t *) 0 )) != (time_t) -1 )
	{
		tptr = localtime(&t1);
		if(tptr->tm_mon == 3) // Easter changes every year, so just have it for all of April
		{
			eastermode = true;
			modifiedgame = false;
		}
	}

    if (M_CheckParm ("-xmas"))
	{
		D_AddFile("3drend.dll");
		xmasmode = true;
		xmasoverride = true;
		modifiedgame = false;
	}
/*	else if (M_CheckParm ("-easter"))
	{
		eastermode = true; // Hunt for the eggs!
	}*/

    // add any files specified on the command line with -file wadfile
    // to the wad list
    //
    // convenience hack to allow -wart e m to add a wad file
    // prepend a tilde to the filename so wadfile will be reloadable
/*    p = M_CheckParm ("-wart");
    if (p)
    {
        myargv[p][4] = 'p';     // big hack, change to -warp

        // Map name handling.
        p = atoi (myargv[p+1]);
        if (p<10)
          sprintf (file,"~"DEVMAPS"cdata/map0%i.wad", p);
        else
          sprintf (file,"~"DEVMAPS"cdata/map%i.wad", p);

        D_AddFile (file);
		modifiedgame = true;
    }*/

	p = M_CheckParm ("-password");
    if (p && p < myargc-1)
    {
		strncpy(adminpassword, myargv[p+1], 8);
		if(strlen(myargv[p+1]) < 8)
		{
			int z;
			for(z=strlen(myargv[p+1]);z<8;z++)
				adminpassword[z] = 'a';
		}
    }

    if (M_CheckParm ("-file"))
    {
        // the parms after p are wadfile/lump names,
        // until end of parms or another - preceded parm
        while (M_IsNextParm())
            D_AddFile (M_GetNextParm());

		modifiedgame = true;
    }

    // load dehacked file
    p = M_CheckParm ("-objcfg");
    if(p!=0)
    {
        while (M_IsNextParm())
            D_AddFile (M_GetNextParm());

		modifiedgame = true;
    }

    // get skill / episode / map from parms
    gameskill = sk_medium;
    startepisode = 1;
    startmap = 1;
    autostart = false;

    p = M_CheckParm ("-skill");
    if (p && p < myargc-1)
    {
		if(modifiedgame || netgame || M_CheckParm ("-server"))
		{
			gameskill = myargv[p+1][0]-'1';
			autostart = true;
		}
    }

    p = M_CheckParm ("-warp");
    if (p && p < myargc-1)
    {
		if(modifiedgame || netgame || M_CheckParm ("-server"))
		{
			startmap = atoi (myargv[p+1]);

			autostart = true;
		}
    }

    CONS_Printf (text[Z_INIT_NUM]);
    Z_Init ();

    // adapt tables to legacy needs
    P_PatchInfoTables();

    CONS_Printf (text[W_INIT_NUM]);
    // load wad, including the main wad file
    if(!W_InitMultipleFiles (startupwadfiles))
        CONS_Error("A WAD file was not found\n");

    // Check and print which version is executed.
    CONS_Printf (text[COMERCIAL_NUM]);

	cht_Init();

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

    // we need to check for dedicated before initialization of some subsystems
    dedicated = M_CheckParm("-dedicated")!=0;

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

    D_RegisterClientCommands (); //Hurdler: be sure that this is called before D_CheckNetGame
    D_AddDeathmatchCommands ();
    R_RegisterEngineStuff ();
    S_RegisterSoundStuff ();

    //Fab:29-04-98: do some dirty chatmacros strings initialisation
    HU_HackChatmacros ();
  //--------------------------------------------------------- CONFIG.CFG
    M_FirstLoadConfig(); // WARNING : this do a "COM_BufExecute()"

	G_LoadGameData(); // Tails 12-08-2002

#ifdef LINUX
    VID_PrepareModeList(); // Regenerate Modelist according to cv_fullscreen
#endif

    // set user default mode or mode set at cmdline
    SCR_CheckDefaultMode ();

    wipegamestate = gamestate;

	P_InitMapHeaders(); // Tails 04-09-2003
	
	//------------------------------------------------ COMMAND LINE PARAMS

	// check for gametype definition Tails 05-19-2001
	p = M_CheckParm ("-gametype");
	if(p && p < myargc-1)
	{
		COM_BufAddText ("gametype ");
		COM_BufAddText (myargv[p+1]);
		COM_BufAddText ("\n");
	}

	if (M_CheckParm ("-noadvance")) COM_BufAddText ("advancemap no\n");

	if (M_CheckParm ("-nospecrings")) COM_BufAddText ("specialrings off\n");

	if (M_CheckParm ("-noadvertise")) COM_BufAddText ("internetserver no\n");

    if (M_CheckParm ("-autoctf"))   COM_BufAddText ("autoctf yes\n"); // Tails 08-04-2001

    if (M_CheckParm ("-forceskin"))   COM_BufAddText ("forceskin yes\n"); // Tails 04-30-2002

	if (M_CheckParm ("-willtherealeggmanpleasestandup")) grade = 0; // Tails 10-23-2002

	p = M_CheckParm ("-matchmonitors");
	if(p && p < myargc-1)
	{
		COM_BufAddText ("matchboxes ");
		COM_BufAddText (myargv[p+1]);
		COM_BufAddText ("\n");
	}

	p = M_CheckParm ("-racemonitors");
	if(p && p < myargc-1)
	{
		COM_BufAddText ("raceitemboxes ");
		COM_BufAddText (myargv[p+1]);
		COM_BufAddText ("\n");
	}

	p = M_CheckParm ("-circuitmonitors"); // Graue 12-06-2003
	if(p && p < myargc-1)
	{
		COM_BufAddText ("circuit_itemboxes ");
		COM_BufAddText (myargv[p+1]);
		COM_BufAddText ("\n");
	}

	p = M_CheckParm ("-ctfteam"); // Tails 08-04-2001
    if (p && p < myargc-1) // Tails 08-04-2001
    { // Tails 08-04-2001
		ctfteam = myargv[p+1]; // Tails 08-04-2001
		COM_BufAddText("preferredteam \""); // Tails 08-04-2001
		COM_BufAddText(ctfteam); // Tails 08-04-2001
		COM_BufAddText("\"\n"); // Tails 08-04-2001
    } // Tails 08-04-2001

	p = M_CheckParm ("-ctfteam2"); // Tails 08-04-2001
    if (p && p < myargc-1) // Tails 08-04-2001
    { // Tails 08-04-2001
		ctfteam = myargv[p+1]; // Tails 08-04-2001
		COM_BufAddText("preferredteam2 \""); // Tails 08-04-2001
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

// Sudden death setting Tails 11-18-2002
	if (M_CheckParm ("-suddendeath")) COM_BufAddText ("suddendeath on\n");

    // Initialize CD-Audio
    if (M_CheckParm ("-usecd"))        I_InitCD ();
    if (M_CheckParm("-teamplay"))      COM_BufAddText ("teamplay 1\n");
    if (M_CheckParm("-teamskin"))      COM_BufAddText ("teamplay 2\n");
    if (M_CheckParm("-splitscreen"))   CV_SetValue(&cv_splitscreen,1);

    if (M_CheckParm ("-analog"))         COM_BufAddText ("analog 1\n"); // Tails
    if (M_CheckParm ("-analog2"))        COM_BufAddText ("analog2 1\n"); // Tails

    if (M_CheckParm ("-timer"))
    {
        char *s=M_GetNextParm();
        COM_BufAddText(va("timelimit %s\n",s ));
    }

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
    nofmod = M_CheckParm("-nodigmusic"); // WARNING: DOS version initmusic in I_StartupSound
    I_StartupSound ();
    I_InitMusic ();  // setup music buffer for quick mus2mid
    S_Init (cv_soundvolume.value, cv_musicvolume.value);

    CONS_Printf (text[ST_INIT_NUM]);
    ST_Init ();

    // init all NETWORK
    CONS_Printf (text[D_CHECKNET_NUM]);
    if( D_CheckNetGame () )
        autostart = true;

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
		CV_Set(&cv_timetic, "ON");
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
        if(false/*dedicated && server*/)
        {
            pagename = "TITLESKY";
            gamestate = GS_DEDICATEDSERVER;
        }
        else
            if (autostart || netgame || M_CheckParm("+connect") || M_CheckParm("-connect"))
            {
                gameaction = ga_nothing;
                if(server && !M_CheckParm("+map"))
                   COM_BufAddText (va("map \"%s\" -fromcode\n",G_BuildMapName(startmap)));
            }
            else
				F_StartIntro(); // Tails 03-03-2002
//F_StartDemoEnd();
//                D_StartTitle ();                // start up intro loop  // 4.32 patch

    }
}
