// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: console.c,v 1.18 2001/12/26 17:24:46 hurdler Exp $
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
// $Log: console.c,v $
// Revision 1.18  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.17  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.16  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.15  2001/03/03 19:41:22  ydario
// I_OutputMsg not implemented in OS/2
//
// Revision 1.14  2001/03/03 06:17:33  bpereira
// no message
//
// Revision 1.13  2001/02/24 13:35:19  bpereira
// no message
//
// Revision 1.12  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.11  2000/11/12 09:48:15  bpereira
// no message
//
// Revision 1.10  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.9  2000/09/28 20:57:14  bpereira
// no message
//
// Revision 1.8  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.7  2000/08/10 15:01:06  ydario
// OS/2 port
//
// Revision 1.6  2000/08/03 17:57:41  bpereira
// no message
//
// Revision 1.5  2000/04/24 15:10:56  hurdler
// Support colormap for text
//
// Revision 1.4  2000/04/16 18:38:06  bpereira
// no message
//
// Revision 1.3  2000/04/07 23:09:12  metzgermeister
// fixed array boundary error
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      console for Doom LEGACY
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "console.h"
#include "g_game.h"
#include "g_input.h"
#include "hu_stuff.h"
#include "keys.h"
#include "r_defs.h"
#include "sounds.h"
#include "st_stuff.h"
#include "s_sound.h"
#include "v_video.h"
#include "i_video.h"
#include "z_zone.h"
#include "i_system.h"
#include "d_main.h"

#ifdef __WIN32__
#include "win32/win_main.h"
void     I_LoadingScreen ( LPCSTR msg );
#endif
#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

boolean  con_started=false;  // console has been initialised
boolean  con_startup=false;  // true at game startup, screen need refreshing
boolean  con_forcepic=true;  // at startup toggle console transulcency when
                             // first off
boolean  con_recalc;     // set true when screen size has changed

int      con_tick;       // console ticker for anim or blinking prompt cursor
                         // con_scrollup should use time (currenttime - lasttime)..

boolean  consoletoggle;  // true when console key pushed, ticker will handle
boolean  consoleready;   // console prompt is ready

int      con_destlines;  // vid lines used by console at final position
int      con_curlines;   // vid lines currently used by console

int      con_clipviewtop;// clip value for planes & sprites, so that the
                         // part of the view covered by the console is not
                         // drawn when not needed, this must be -1 when
                         // console is off

// TODO: choose max hud msg lines
#define  CON_MAXHUDLINES      5

static int      con_hudlines;        // number of console heads up message lines
int      con_hudtime[5];      // remaining time of display for hud msg lines

int      con_clearlines; // top screen lines to refresh when view reduced
boolean  con_hudupdate;  // when messages scroll, we need a backgrnd refresh


// console text output
char*    con_line;       // console text output current line
int      con_cx;         // cursor position in current line
int      con_cy;         // cursor line number in con_buffer, is always
                         //  increasing, and wrapped around in the text
                         //  buffer using modulo.

int      con_totallines; // lines of console text into the console buffer
int      con_width;      // columns of chars, depend on vid mode width

int      con_scrollup;   // how many rows of text to scroll up (pgup/pgdn)


// hold 32 last lines of input for history
#define  CON_MAXPROMPTCHARS    256
#define  CON_PROMPTCHAR        '>'

char     inputlines[32][CON_MAXPROMPTCHARS]; // hold last 32 prompt lines

int      inputline;      // current input line number
int      inputhist;      // line number of history input line to restore
int      input_cx;       // position in current input line

patch_t*   con_backpic;    // console background picture, loaded static
pic_t*   con_bordleft;
pic_t*   con_bordright;  // console borders in translucent mode


// protos.
static void CON_InputInit (void);
static void CON_RecalcSize (void);

static void CONS_speed_Change (void);
static void CON_DrawBackpic (patch_t *pic, int startx, int destwidth);
static void CON_DrawBackpic2 (pic_t *pic, int startx, int destwidth);


//======================================================================
//                   CONSOLE VARS AND COMMANDS
//======================================================================
#ifdef __MACOS__
#define  CON_BUFFERSIZE   4096  //my compiler cant handle local vars >32k
#else
#define  CON_BUFFERSIZE   16384
#endif

char     con_buffer[CON_BUFFERSIZE];


// how many seconds the hud messages lasts on the screen
consvar_t   cons_msgtimeout = {"con_hudtime","5",CV_SAVE,CV_Unsigned};

// number of lines console move per frame
consvar_t   cons_speed = {"con_speed","8",CV_CALL|CV_SAVE,CV_Unsigned,&CONS_speed_Change};

// percentage of screen height to use for console
consvar_t   cons_height = {"con_height","50",CV_SAVE,CV_Unsigned};

CV_PossibleValue_t backpic_cons_t[]={{0,"translucent"},{1,"picture"},{0,NULL}};
// whether to use console background picture, or translucent mode
consvar_t   cons_backpic = {"con_backpic","0",CV_SAVE,backpic_cons_t};

void CON_Print (char *msg);

//  Check CONS_speed value (must be positive and >0)
//
static void CONS_speed_Change (void)
{
    if (cons_speed.value<1)
        CV_SetValue (&cons_speed,1);
}


//  Clear console text buffer
//
static void CONS_Clear_f (void)
{
    if (con_buffer)
        memset(con_buffer,0,CON_BUFFERSIZE);

    con_cx = 0;
    con_cy = con_totallines-1;
    con_line = &con_buffer[con_cy*con_width];
    con_scrollup = 0;
}

//  Choose english keymap
//
static void CONS_English_f (void)
{
    shiftxform = english_shiftxform;
    CONS_Printf("English keymap.\n");
}

char *bindtable[NUMINPUTS];

void CONS_Bind_f(void)
{
    int  na,key;

    na=COM_Argc();

    if ( na!= 2 && na!=3)
    {
        CONS_Printf ("bind <keyname> [<command>]\n");
        CONS_Printf("\2bind table :\n");
        na=0;
        for(key=0;key<NUMINPUTS;key++)
            if(bindtable[key])
            {
                CONS_Printf("%s : \"%s\"\n",G_KeynumToString (key),bindtable[key]);
                na=1;
            }
        if(!na)
            CONS_Printf("Empty\n");
        return;
    }

    key=G_KeyStringtoNum(COM_Argv(1));
    if(!key)
    {
        CONS_Printf("Invalid key name\n");
        return;
    }

    if(bindtable[key]!=NULL)
    {
        Z_Free(bindtable[key]);
        bindtable[key]=NULL;
    }

    if( na==3 )
        bindtable[key]=Z_StrDup(COM_Argv(2));
}


//======================================================================
//                          CONSOLE SETUP
//======================================================================

// Prepare a colormap for GREEN ONLY translucency over background
//
byte*   whitemap;
byte*   purplemap;
byte*   lgreenmap;
byte*   bluemap;
byte*   greenmap;
byte*   graymap;
static void CON_SetupBackColormap (void)
{
    int   i,j,k;
    byte* pal;

//
//  setup the green translucent background colormap
//
    greenmap = (byte *) Z_Malloc(256,PU_STATIC,NULL);
    whitemap = (byte *) Z_Malloc(256,PU_STATIC,NULL);
    graymap  = (byte *) Z_Malloc(256,PU_STATIC,NULL);
	purplemap = (byte *) Z_Malloc(256,PU_STATIC,NULL);
	lgreenmap = (byte *) Z_Malloc(256,PU_STATIC,NULL);
	bluemap = (byte *) Z_Malloc(256,PU_STATIC,NULL);

    pal = W_CacheLumpName ("PLAYPAL",PU_CACHE);

    for(i=0,k=0; i<768; i+=3,k++)
    {
        j = pal[i] + pal[i+1] + pal[i+2];
        greenmap[k] = 127 - (j>>6);
    }

//
//  setup the white and gray text colormap
//
    // this one doesn't need to be aligned, unless you convert the
    // V_DrawMappedPatch() into optimised asm.

    for(i=0; i<256; i++)
    {
        whitemap[i] = i;        //remap each color to itself...
        graymap[i]  = i;

		purplemap[i]  = i;
		lgreenmap[i]  = i;
		bluemap[i]  = i;
    }

	for(i=82;i<83;i++)
    {
        whitemap[i]=i+149; // Tails     //remaps reds(168-192) to whites(80-104)
		purplemap[i]=i+170; // Tails     //remaps reds(168-192) to whites(80-104)
		lgreenmap[i]=i+32; // Tails     //remaps reds(168-192) to whites(80-104)
		bluemap[i]=i+118; // Tails     //remaps reds(168-192) to whites(80-104)
        graymap[i]=i-80;      //remaps reds(168-192) to gray(88-...)
    }
    whitemap[45]=190-88; // the color[45]=color[190] !
    graymap [45]=190-80;
    whitemap[47]=191-88; // the color[47]=color[191] !
    graymap [47]=191-80;
}


//  Setup the console text buffer
//
void CON_Init(void)
{
    int i;

//    if(dedicated)
//	return;
    
    for(i=0;i<NUMINPUTS;i++)
        bindtable[i]=NULL;

    // clear all lines
    memset(con_buffer,0,CON_BUFFERSIZE);

    // make sure it is ready for the loading screen
    con_width = 0;
    CON_RecalcSize ();

    CON_SetupBackColormap ();

    //note: CON_Ticker should always execute at least once before D_Display()
    con_clipviewtop = -1;     // -1 does not clip

    con_hudlines = CON_MAXHUDLINES;

    // setup console input filtering
    CON_InputInit ();

    // load console background pic
    con_backpic = (patch_t*) W_CacheLumpName ("CONSBACK",PU_STATIC);

    // borders MUST be there
    con_bordleft  = (pic_t*) W_CacheLumpName ("CBLEFT",PU_STATIC);
    con_bordright = (pic_t*) W_CacheLumpName ("CBRIGHT",PU_STATIC);

    // register our commands
    //
    CV_RegisterVar (&cons_msgtimeout);
    CV_RegisterVar (&cons_speed);
    CV_RegisterVar (&cons_height);
    CV_RegisterVar (&cons_backpic);
    COM_AddCommand ("cls", CONS_Clear_f);
    COM_AddCommand ("english", CONS_English_f);
    COM_AddCommand ("bind", CONS_Bind_f);
    // set console full screen for game startup MAKE SURE VID_Init() done !!!
    con_destlines = vid.height;
    con_curlines = vid.height;
    consoletoggle = false;

    con_started = true;
    con_startup = true; // need explicit screen refresh
                        // until we are in Doomloop
}


//  Console input initialization
//
static void CON_InputInit (void)
{
    int    i;

    // prepare the first prompt line
    memset (inputlines,0,sizeof(inputlines));
    for (i=0; i<32; i++)
        inputlines[i][0] = CON_PROMPTCHAR;
    inputline = 0;
    input_cx = 1;

}



//======================================================================
//                        CONSOLE EXECUTION
//======================================================================


//  Called at screen size change to set the rows and line size of the
//  console text buffer.
//
static void CON_RecalcSize (void)
{
    int   conw, oldcon_width, oldnumlines, i, oldcon_cy;
    char  tmp_buffer[CON_BUFFERSIZE];
    char  string[CON_BUFFERSIZE]; // BP: it is a line but who know

    con_recalc = false;

    conw = (vid.width>>3)-2;

    if( con_curlines==200 )  // first init
    {
        con_curlines=vid.height;
        con_destlines=vid.height;
    }

    // check for change of video width
    if (conw == con_width)
        return;                 // didnt change

    oldcon_width = con_width;
    oldnumlines = con_totallines;
    oldcon_cy = con_cy;
    memcpy(tmp_buffer, con_buffer, CON_BUFFERSIZE);

    if (conw<1)
        con_width = (BASEVIDWIDTH>>3)-2;
    else
        con_width = conw;

    con_totallines = CON_BUFFERSIZE / con_width;
    memset (con_buffer,' ',CON_BUFFERSIZE);


    con_cx = 0;
    con_cy = con_totallines-1;
    con_line = &con_buffer[con_cy*con_width];
    con_scrollup = 0;

    // re-arrange console text buffer to keep text
    if(oldcon_width) // not the first time
    {
        for(i=oldcon_cy+1;i<oldcon_cy+oldnumlines;i++)
        {
            if( tmp_buffer[(i% oldnumlines)*oldcon_width])
            {
                memcpy(string, &tmp_buffer[(i% oldnumlines)*oldcon_width], oldcon_width);
                conw=oldcon_width-1;
                while(string[conw]==' ' && conw) conw--;
                string[conw+1]='\n';
                string[conw+2]='\0';
                CON_Print(string);
            }
        }
    }
}


// Handles Console moves in/out of screen (per frame)
//
static void CON_MoveConsole (void)
{
    // up/down move to dest
    if (con_curlines < con_destlines)
    {
        con_curlines+=cons_speed.value/NEWTICRATERATIO; // Tails
        if (con_curlines > con_destlines)
           con_curlines = con_destlines;
    }
    else if (con_curlines > con_destlines)
    {
        con_curlines-=cons_speed.value/NEWTICRATERATIO; // Tails
        if (con_curlines < con_destlines)
            con_curlines = con_destlines;
    }

}


//  Clear time of console heads up messages
//
void CON_ClearHUD (void)
{
    int    i;

    for(i=0; i<con_hudlines; i++)
        con_hudtime[i]=0;
}


// Force console to move out immediately
// note: con_ticker will set consoleready false
void CON_ToggleOff (void)
{
    if (!con_destlines)
        return;

    con_destlines = 0;
    con_curlines = 0;
    CON_ClearHUD ();
    con_forcepic = 0;
    con_clipviewtop = -1;       //remove console clipping of view
}


//  Console ticker : handles console move in/out, cursor blinking
//
void CON_Ticker (void)
{
    int    i;

//    if(dedicated)
//	return;
    
    // cursor blinking
    con_tick++;
    con_tick &= 7;

    // console key was pushed
    if (consoletoggle)
    {
        consoletoggle = false;

        // toggle off console
        if (con_destlines > 0)
        {
            con_destlines = 0;
            CON_ClearHUD ();
        }
        else
        {
            // toggle console in
            con_destlines = (cons_height.value*vid.height)/100;
            if (con_destlines < 20)
                con_destlines = 20;
            else
            if (con_destlines > vid.height)
                con_destlines = vid.height;

            con_destlines &= ~0x3;      // multiple of text row height
        }
    }

    // console movement
    if (con_destlines!=con_curlines)
        CON_MoveConsole ();


    // clip the view, so that the part under the console is not drawn
    con_clipviewtop = -1;
    if (cons_backpic.value)   // clip only when using an opaque background
    {
        if (con_curlines > 0)
            con_clipviewtop = con_curlines - viewwindowy - 1 - 10;
//NOTE: BIG HACK::SUBTRACT 10, SO THAT WATER DON'T COPY LINES OF THE CONSOLE
//      WINDOW!!! (draw some more lines behind the bottom of the console)
        if (con_clipviewtop<0)
            con_clipviewtop = -1;   //maybe not necessary, provided it's <0
    }

    // check if console ready for prompt
    if (/*(con_curlines==con_destlines) &&*/ (con_destlines>=20))
        consoleready = true;
    else
        consoleready = false;

    // make overlay messages disappear after a while
    for (i=0 ; i<con_hudlines; i++)
    {
        con_hudtime[i]--;
        if (con_hudtime[i]<0)
            con_hudtime[i]=0;
    }
}


//  Handles console key input
//
boolean CON_Responder (event_t *ev)
{
//static boolean altdown;
static boolean shiftdown;


// sequential completions a la 4dos
static char    completion[80];
static int     comskips,varskips;

    char   *cmd;
    int     key;

    if(chat_on)
        return false; 

    // special keys state
    if (ev->data1 == KEY_SHIFT && ev->type == ev_keyup)
    {
        shiftdown = false;
        return false;
    }
    //else if (ev->data1 == KEY_ALT)
    //{
    //    altdown = (ev->type == ev_keydown);
    //    return false;
    //}

    // let go keyup events, don't eat them
    if (ev->type != ev_keydown)
        return false;

    key = ev->data1;

//
//  check for console toggle key
//
    if (key == gamecontrol[gc_console][0] ||
        key == gamecontrol[gc_console][1] )
    {
        consoletoggle = true;
        return true;
    }

//
//  check other keys only if console prompt is active
//
    if (!consoleready && key < NUMINPUTS) // metzgermeister: boundary check !!
    {
        if(bindtable[key])
        {
            COM_BufAddText (bindtable[key]);
            COM_BufAddText ("\n");
            return true;
        }
        return false;
    }

    // eat shift only if console active
    if (key == KEY_SHIFT)
    {
        shiftdown = true;
        return true;
    }

    // escape key toggle off console
    if (key == KEY_ESCAPE)
    {
        consoletoggle = true;
        return true;
    }

    // command completion forward (tab) and backward (shift-tab)
    if (key == KEY_TAB)
    {
        // TOTALLY UTTERLY UGLY NIGHT CODING BY FAB!!! :-)
        //
        // sequential command completion forward and backward

        // remember typing for several completions (�-la-4dos)
        if (inputlines[inputline][input_cx-1] != ' ')
        {
            if (strlen (inputlines[inputline]+1)<80)
                strcpy (completion, inputlines[inputline]+1);
            else
                completion[0] = 0;

            comskips = varskips = 0;
        }
        else
        {
            if (shiftdown)
            {
                if (comskips<0)
                {
                    if (--varskips<0)
                        comskips = -(comskips+2);
                }
                else
                if (comskips>0)
                    comskips--;
            }
            else
            {
                if (comskips<0)
                    varskips++;
                else
                    comskips++;
            }
        }

        if (comskips>=0)
        {
            cmd = COM_CompleteCommand (completion, comskips);
            if (!cmd)
                // dirty:make sure if comskips is zero, to have a neg value
                comskips = -(comskips+1);
        }
        if (comskips<0)
            cmd = CV_CompleteVar (completion, varskips);

        if (cmd)
        {
            memset(inputlines[inputline]+1,0,CON_MAXPROMPTCHARS-1);
            strcpy (inputlines[inputline]+1, cmd);
            input_cx = strlen(cmd)+1;
            inputlines[inputline][input_cx] = ' ';
            input_cx++;
            inputlines[inputline][input_cx] = 0;
        }
        else
        {
            if (comskips>0)
                comskips--;
            else
            if (varskips>0)
                varskips--;
        }

        return true;
    }

    // move up (backward) in console textbuffer
    if (key == KEY_PGUP)
    {
        if (con_scrollup < (con_totallines-((con_curlines-16)>>3)) )
            con_scrollup++;
        return true;
    }
    else
    if (key == KEY_PGDN)
    {
        if (con_scrollup>0)
            con_scrollup--;
        return true;
    }

    // oldset text in buffer
    if (key == KEY_HOME)
    {
        con_scrollup = (con_totallines-((con_curlines-16)>>3));
        return true;
    }
    else
    // most recent text in buffer
    if (key == KEY_END)
    {
        con_scrollup = 0;
        return true;
    }

    // command enter
    if (key == KEY_ENTER)
    {
        if (input_cx<2)
            return true;

        // push the command
        COM_BufAddText (inputlines[inputline]+1);
        COM_BufAddText ("\n");

        CONS_Printf("%s\n",inputlines[inputline]);

        inputline = (inputline+1) & 31;
        inputhist = inputline;

        memset(inputlines[inputline],0,CON_MAXPROMPTCHARS);
        inputlines[inputline][0] = CON_PROMPTCHAR;
        input_cx = 1;

        return true;
    }

    // backspace command prompt
    if (key == KEY_BACKSPACE)
    {
        if (input_cx>1)
        {
            input_cx--;
            inputlines[inputline][input_cx] = 0;
        }
        return true;
    }

    // move back in input history
    if (key == KEY_UPARROW)
    {
        // copy one of the previous inputlines to the current
        do{
            inputhist = (inputhist - 1) & 31;   // cycle back
        }while (inputhist!=inputline && !inputlines[inputhist][1]);

        // stop at the last history input line, which is the
        // current line + 1 because we cycle through the 32 input lines
        if (inputhist==inputline)
            inputhist = (inputline + 1) & 31;

        memcpy (inputlines[inputline],inputlines[inputhist],CON_MAXPROMPTCHARS);
        input_cx = strlen(inputlines[inputline]);

        return true;
    }

    // move forward in input history
    if (key == KEY_DOWNARROW)
    {
        if (inputhist==inputline) return true;
        do{
            inputhist = (inputhist + 1) & 31;
        } while (inputhist!=inputline && !inputlines[inputhist][1]);

        memset (inputlines[inputline],0,CON_MAXPROMPTCHARS);

        // back to currentline
        if (inputhist==inputline)
        {
            inputlines[inputline][0] = CON_PROMPTCHAR;
            input_cx = 1;
        }
        else
        {
            strcpy (inputlines[inputline],inputlines[inputhist]);
            input_cx = strlen(inputlines[inputline]);
        }
        return true;
    }

    // allow people to use keypad in console (good for typing IP addresses) - Calum
    if (key>=KEY_KEYPAD7 && key <= KEY_KPADDEL)
    {
        char keypad_translation[] = {   '7','8','9','-',
                                        '4','5','6','+',
                                        '1','2','3',
                                        '0','.'};
        
        key = keypad_translation[key - KEY_KEYPAD7];
    }
    else if (key == KEY_KPADSLASH)
        key = '/';

    if (shiftdown)
        key = shiftxform[key];
    
    // enter a char into the command prompt
    if (key<32 || key>127)
        return false;

    // add key to cmd line here
    if (input_cx<CON_MAXPROMPTCHARS)
    {
        // make sure letters are lowercase for commands & cvars
        if (key >= 'A' && key <= 'Z')
            key = key + 'a' - 'A';

        inputlines[inputline][input_cx] = key;
        inputlines[inputline][input_cx+1] = 0;
        input_cx++;
    }

    return true;
}


//  Insert a new line in the console text buffer
//
static void CON_Linefeed (void)
{
    // set time for heads up messages
    con_hudtime[con_cy%con_hudlines] = cons_msgtimeout.value*TICRATE;

    con_cy++;
    con_cx = 0;

    con_line = &con_buffer[(con_cy%con_totallines)*con_width];
    memset(con_line,' ',con_width);

    // make sure the view borders are refreshed if hud messages scroll
    con_hudupdate = true;         // see HU_Erase()
}


//  Outputs text into the console text buffer
//
//TODO: fix this mess!!
void CON_Print (char *msg)
{
    int      l;
    int      mask=0;

    //TODO: finish text colors
    if (*msg<5)
    {
/*      if (*msg=='\2')  // set white color
          mask = 128;
      else */if (*msg=='\3')
      {
//          mask = 128;// white text + sound
          S_StartSound(0, sfx_radio);
      }
	  else if (*msg=='\4')
	  {
		  mask = 0x80;
		  S_StartSound(0, sfx_radio);
	  }
/*	  else if(*msg=='\5')
	  {
		  mask = V_GREENMAP;
		  S_StartSound(0, sfx_radio);
	  }
	  else if(*msg=='\6')
	  {
		  mask = V_BLUEMAP;
		  S_StartSound(0, sfx_radio);
	  }*/
    }

    while (*msg)
    {
        // skip non-printable characters and white spaces
        while (*msg && *msg<=' ')
        {

            // carriage return
            if (*msg=='\r')
            {
                con_cy--;
                CON_Linefeed ();
            }
            else
            // linefeed
            if (*msg=='\n')
                CON_Linefeed ();
            else
            if (*msg==' ')
            {
                con_line[con_cx++] = ' ';
                if (con_cx>=con_width)
                    CON_Linefeed();
            }
            else if (*msg=='\t')
            {
                //adds tab spaces for nice layout in console
                
                do
                {
                    con_line[con_cx++] = ' ';
                } while (con_cx%4 != 0);
                
                if (con_cx>=con_width)
                    CON_Linefeed();
            }
            msg++;
        }

        if (*msg==0)
            return;

        // printable character
        for (l=0; l<con_width && msg[l]>' '; l++)
            ;

        // word wrap
        if (con_cx+l>con_width)
            CON_Linefeed ();

        // a word at a time
        for ( ; l>0; l--)
            con_line[con_cx++] = *(msg++) | mask;

    }

}


//  Console print! Wahooo! Lots o fun!
//
void CONS_Printf (char *fmt, ...)
{
    va_list     argptr;
    char        txt[512];

    va_start (argptr,fmt);
    vsprintf (txt,fmt,argptr);
    va_end   (argptr);

    // echo console prints to log file
#if defined(__WIN32__) && defined(LOGMESSAGES)
    if (logstream != INVALID_HANDLE_VALUE)
        FPrintf (logstream, "%s", txt);     // uses win_dbg.c FPrintf()
#endif
    DEBFILE(txt);

    if (!con_started/* || !graphics_started*/)
    {
#if !defined( __WIN32__) && !defined( __OS2__)
        I_OutputMsg ("%s",txt);
#endif
        return;
    }
    else
        // write message in con text buffer
        CON_Print (txt);

    // make sure new text is visible
    con_scrollup = 0;

    // if not in display loop, force screen update
    if (con_startup)
    {
#if defined( __WIN32__) || defined( __OS2__) 
        // show startup screen and message using only 'software' graphics
        // (rendermode may be hardware accelerated, but the video mode is not set yet)
        CON_DrawBackpic (con_backpic, 0, vid.width);    // put console background
        I_LoadingScreen ( txt );
#else
        // here we display the console background and console text
        // (no hardware accelerated support for these versions)
        CON_Drawer ();
        I_FinishUpdate ();              // page flip or blit buffer
#endif
    }
}


//  Print an error message, and wait for ENTER key to continue.
//  To make sure the user has seen the message
//
void CONS_Error (char *msg)
{
#ifdef WIN32
    extern  HWND    hWndMain;
    if(!graphics_started)
    {
        MessageBox (hWndMain, msg, "SRB2 Warning", MB_OK); // Tails
        return;
    }
#endif
    CONS_Printf ("\2%s",msg);   // write error msg in different colour
    CONS_Printf ("Press ENTER to continue\n");

    // dirty quick hack, but for the good cause
    while (I_GetKey() != KEY_ENTER)
        ;
}


//======================================================================
//                          CONSOLE DRAW
//======================================================================


// draw console prompt line
//
static void CON_DrawInput (void)
{
    char    *p;
    int     x,y;

    // input line scrolls left if it gets too long
    //
    p = inputlines[inputline];
    if (input_cx>=con_width)
        p += input_cx - con_width + 1;

    y = con_curlines - 12;

    for(x=0; x<con_width; x++)
        V_DrawCharacter( (x+1)<<3, y, p[x] |V_NOSCALEPATCH|V_NOSCALESTART);

    // draw the blinking cursor
    //
    x = (input_cx>=con_width) ? con_width - 1 : input_cx;
    if (con_tick<4)
        V_DrawCharacter( (x+1)<<3, y, '_' |V_NOSCALEPATCH|V_NOSCALESTART);
}


// draw the last lines of console text to the top of the screen
//
static void CON_DrawHudlines (void)
{
    char       *p;
    int        i,x,y;

    if (con_hudlines<=0)
        return;

    if (chat_on)
        y = 8;   // leave place for chat input in the first row of text
    else
        y = 0;

    for (i= con_cy-con_hudlines+1; i<=con_cy; i++)
    {
        if (i < 0)
            continue;
        if (con_hudtime[i%con_hudlines] == 0)
            continue;

        p = &con_buffer[(i%con_totallines)*con_width];

        for (x=0; x<con_width; x++)
            V_DrawCharacter ( x<<3, y, p[x] |V_NOSCALEPATCH|V_NOSCALESTART);

        y += 8;
    }

    // top screen lines that might need clearing when view is reduced
    con_clearlines = y;      // this is handled by HU_Erase ();
}


//  Scale a pic_t at 'startx' pos, to 'destwidth' columns.
//                startx,destwidth is resolution dependent
//  Used to draw console borders, console background.
//  The pic must be sized BASEVIDHEIGHT height.
//
//  TODO: ASM routine!!! lazy Fab!!
//
static void CON_DrawBackpic (patch_t *pic, int startx, int destwidth)
{
	V_DrawScaledPatch(0,0,0,pic);
/*    int         x, y;
    int         v;
    byte        *src, *dest;
    int         frac, fracstep;

    dest = vid.buffer+startx;

    for (y=0 ; y<con_curlines ; y++, dest += vid.width)
    {
        // scale the picture to the resolution
        v = SHORT(pic->height) - ((con_curlines - y)*(BASEVIDHEIGHT-1)/vid.height) - 1;

        src = pic->data + v*SHORT(pic->width);

        // in case of the console backpic, simplify
        if (pic->width==destwidth)
            memcpy (dest, src, destwidth);
        else
        {
            // scale pic to screen width
            frac = 0;
            fracstep = (SHORT(pic->width)<<16)/destwidth;
            for (x=0 ; x<destwidth ; x+=4)
            {
                dest[x] = src[frac>>16];
                frac += fracstep;
                dest[x+1] = src[frac>>16];
                frac += fracstep;
                dest[x+2] = src[frac>>16];
                frac += fracstep;
                dest[x+3] = src[frac>>16];
                frac += fracstep;
            }
        }
    }
*/
}
static void CON_DrawBackpic2 (pic_t *pic, int startx, int destwidth)
{
    int         x, y;
    int         v;
    byte        *src, *dest;
    int         frac, fracstep;

    dest = vid.buffer+startx;

    for (y=0 ; y<con_curlines ; y++, dest += vid.width)
    {
        // scale the picture to the resolution
        v = SHORT(pic->height) - ((con_curlines - y)*(BASEVIDHEIGHT-1)/vid.height) - 1;

        src = pic->data + v*SHORT(pic->width);

        // in case of the console backpic, simplify
        if (pic->width==destwidth)
            memcpy (dest, src, destwidth);
        else
        {
            // scale pic to screen width
            frac = 0;
            fracstep = (SHORT(pic->width)<<16)/destwidth;
            for (x=0 ; x<destwidth ; x+=4)
            {
                dest[x] = src[frac>>16];
                frac += fracstep;
                dest[x+1] = src[frac>>16];
                frac += fracstep;
                dest[x+2] = src[frac>>16];
                frac += fracstep;
                dest[x+3] = src[frac>>16];
                frac += fracstep;
            }
        }
    }
}


// draw the console background, text, and prompt if enough place
//
static void CON_DrawConsole (void)
{
    char       *p;
    int        i,x,y;
    int        w = 0, x2 = 0;

    if (con_curlines <= 0)
        return;

    //FIXME: refresh borders only when console bg is translucent
    con_clearlines = con_curlines;    // clear console draw from view borders
    con_hudupdate = true;             // always refresh while console is on

    // draw console background
    if (cons_backpic.value || con_forcepic)
    {
#ifdef HWRENDER // not win32 only 19990829 by Kin
        if (rendermode!=render_soft)
            V_DrawScaledPatch (0, 0, 0, con_backpic );
        else
#endif
            CON_DrawBackpic (con_backpic,0,vid.width);   // picture as background
    }
    else
    {
#ifdef HWRENDER // not win32 only 19990829 by Kin
        if (rendermode==render_soft)
#endif
        {
            w = 8*vid.dupx;
            x2 = vid.width - w;
            CON_DrawBackpic2 (con_bordleft,0,w);
            CON_DrawBackpic2 (con_bordright,x2,w);
        }
        //Hurdler: what's the correct value of w and x2 in hardware mode ???
        V_DrawFadeConsBack (w,0,x2,con_curlines);     // translucent background
    }

    // draw console text lines from bottom to top
    // (going backward in console buffer text)
    //
    if (con_curlines <20)       //8+8+4
        return;

    i = con_cy - con_scrollup;

    // skip the last empty line due to the cursor being at the start
    // of a new line
    if (!con_scrollup && !con_cx)
        i--;

    for (y=con_curlines-20; y>=0; y-=8,i--)
    {
        if (i<0)
            i=0;

        p = &con_buffer[(i%con_totallines)*con_width];

        for (x=0;x<con_width;x++)
            V_DrawCharacter( (x+1)<<3, y, p[x] |V_NOSCALEPATCH|V_NOSCALESTART);
    }


    // draw prompt if enough place (not while game startup)
    //
    if ((con_curlines==con_destlines) && (con_curlines>=20) && !con_startup)
        CON_DrawInput ();
}


//  Console refresh drawer, call each frame
//
void CON_Drawer (void)
{
    if (!con_started)
        return;

    if (con_recalc)
        CON_RecalcSize ();

    //Fab: bighack: patch 'I' letter leftoffset so it centers
    hu_font['I'-HU_FONTSTART]->leftoffset = -2;

    if (con_curlines>0)
        CON_DrawConsole ();
    else
    if (gamestate==GS_LEVEL || gamestate==GS_INTERMISSION)
        CON_DrawHudlines ();

    hu_font['I'-HU_FONTSTART]->leftoffset = 0;
}
