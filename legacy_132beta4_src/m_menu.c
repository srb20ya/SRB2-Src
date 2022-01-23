// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_menu.c,v 1.47 2001/12/31 13:47:46 hurdler Exp $
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
// $Log: m_menu.c,v $
// Revision 1.47  2001/12/31 13:47:46  hurdler
// Add setcorona FS command and prepare the code for beta 4
//
// Revision 1.46  2001/12/26 17:24:46  hurdler
// Update Linux version
//
// Revision 1.45  2001/12/15 18:41:35  hurdler
// small commit, mainly splitscreen fix
//
// Revision 1.44  2001/11/02 23:29:13  judgecutor
// Fixed "secondary player controls" bug
//
// Revision 1.43  2001/11/02 21:44:05  judgecutor
// Added Frag's weapon falling
//
// Revision 1.42  2001/08/20 21:37:34  hurdler
// fix palette in splitscreen + hardware mode
//
// Revision 1.41  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.40  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.39  2001/06/10 21:16:01  bpereira
// no message
//
// Revision 1.38  2001/05/27 13:42:47  bpereira
// no message
//
// Revision 1.37  2001/05/16 22:00:10  hurdler
// fix compiling problem
//
// Revision 1.36  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.35  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.34  2001/04/29 14:25:26  hurdler
// small fix
//
// Revision 1.33  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.32  2001/03/03 06:17:33  bpereira
// no message
//
// Revision 1.31  2001/02/24 13:35:20  bpereira
// no message
//
// Revision 1.30  2001/02/10 12:27:14  bpereira
// no message
//
// Revision 1.29  2001/01/25 22:15:42  bpereira
// added heretic support
//
// Revision 1.28  2000/11/26 20:36:14  hurdler
// Adding autorun2
//
// Revision 1.27  2000/10/21 08:43:29  bpereira
// no message
//
// Revision 1.26  2000/10/17 10:09:27  hurdler
// Update master server code for easy connect from menu
//
// Revision 1.25  2000/10/16 20:02:29  bpereira
// no message
//
// Revision 1.24  2000/10/08 13:30:01  bpereira
// no message
//
// Revision 1.23  2000/10/02 18:25:45  bpereira
// no message
//
// Revision 1.22  2000/10/01 15:20:23  hurdler
// Add private server
//
// Revision 1.21  2000/10/01 10:18:17  bpereira
// no message
//
// Revision 1.20  2000/10/01 09:09:36  hurdler
// Put the md2 code in #ifdef TANDL
//
// Revision 1.19  2000/09/15 19:49:22  bpereira
// no message
//
// Revision 1.18  2000/09/08 22:28:30  hurdler
// merge masterserver_ip/port in one cvar, add -private
//
// Revision 1.17  2000/09/02 15:38:24  hurdler
// Add master server to menus (temporaray)
//
// Revision 1.16  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.15  2000/04/24 15:10:56  hurdler
// Support colormap for text
//
// Revision 1.14  2000/04/23 00:29:28  hurdler
// fix a small bug in skin color
//
// Revision 1.13  2000/04/23 00:25:20  hurdler
// fix a small bug in skin color
//
// Revision 1.12  2000/04/22 21:12:15  hurdler
// I like it better like that
//
// Revision 1.11  2000/04/22 20:27:35  metzgermeister
// support for immediate fullscreen switching
//
// Revision 1.10  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.9  2000/04/13 16:26:41  hurdler
// looks better like that
//
// Revision 1.8  2000/04/12 19:31:37  metzgermeister
// added use_mouse to menu
//
// Revision 1.7  2000/04/08 17:29:24  stroggonmeth
// no message
//
// Revision 1.6  2000/04/07 23:11:17  metzgermeister
// added mouse move
//
// Revision 1.5  2000/04/04 10:44:00  hurdler
// Remove a warning message in Dos/Windows
//
// Revision 1.4  2000/04/04 00:32:46  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.3  2000/03/23 22:54:00  metzgermeister
// added support for HOME/.legacy under Linux
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      DOOM selection menu, options, episode etc.
//      Sliders and icons. Kinda widget stuff.
//
// NOTE:
//      All V_DrawPatchDirect () has been replaced by V_DrawScaledPatch ()
//      so that the menu is scaled to the screen size. The scaling is always
//      an integer multiple of the original size, so that the graphics look
//      good.
//
//-----------------------------------------------------------------------------

#ifndef __WIN32__
#include <unistd.h>
#endif
#include <fcntl.h>

#include "am_map.h"

#include "doomdef.h"
#include "dstrings.h"
#include "d_main.h"

#include "console.h"

#include "r_local.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "g_input.h"

#include "m_argv.h"

// Data.
#include "sounds.h"
#include "s_sound.h"
#include "i_system.h"

#include "m_menu.h"
#include "v_video.h"
#include "i_video.h"

#include "keys.h"
#include "z_zone.h"
#include "w_wad.h"
#include "p_local.h"
#include "p_fab.h"

#ifdef HWRENDER
#include "hardware/hw_main.h"
#endif

#include "d_net.h"
#include "mserv.h"
#include "p_inter.h"
#include "m_misc.h"

#include "byteptr.h"

// -1 = no quicksave slot picked!
int                     quickSaveSlot;
boolean                 menuactive;

int lastmapnum;
int oldlastmapnum;

extern consvar_t cv_autoaim;
extern consvar_t cv_autoaim2;
extern consvar_t cv_numsnow; // Tails 12-25-2001
extern consvar_t cv_raindensity; // Tails 08-25-2002
extern consvar_t cv_precipdist; // Tails 08-20-2002
extern consvar_t cv_inttime;
extern consvar_t cv_advancemap;
extern consvar_t cv_racetype;
extern consvar_t cv_raceitemboxes;
extern consvar_t cv_matchboxes;
extern consvar_t cv_specialrings;
extern consvar_t cv_chaos_bluecrawla;
extern consvar_t cv_chaos_redcrawla;
extern consvar_t cv_chaos_crawlacommander;
extern consvar_t cv_chaos_jettysynbomber;
extern consvar_t cv_chaos_jettysyngunner;
extern consvar_t cv_chaos_eggmobile1;
extern consvar_t cv_chaos_eggmobile2;
extern consvar_t cv_teleporters;
extern consvar_t cv_superring;
extern consvar_t cv_silverring;
extern consvar_t cv_supersneakers;
extern consvar_t cv_invincibility;
extern consvar_t cv_blueshield;
extern consvar_t cv_greenshield;
extern consvar_t cv_yellowshield;
extern consvar_t cv_redshield;
extern consvar_t cv_blackshield;
extern consvar_t cv_1up;
extern consvar_t cv_eggmanbox;
extern consvar_t cv_chaos_spawnrate;
extern consvar_t cv_playerspeed;
extern consvar_t cv_ringslinger;
extern consvar_t cv_startrings;
extern consvar_t cv_startlives;
extern consvar_t cv_startcontinues;
extern consvar_t cv_circuit_itemboxes;
extern consvar_t cv_circuit_ringthrow;
extern consvar_t cv_circuit_specmoves;
extern consvar_t cv_circuit_spin;
extern consvar_t cv_match_scoring;
extern consvar_t cv_ctf_scoring;

#define SKULLXOFF       -32
#define LINEHEIGHT       16
#define STRINGHEIGHT     10
#define FONTBHEIGHT      20
#define SMALLLINEHEIGHT   8
#define SLIDER_RANGE     10
#define SLIDER_WIDTH    (8*SLIDER_RANGE+6)
#define MAXSTRINGLENGTH  32

// Stuff for customizing the player select screen Tails 09-22-2003
description_t description[8] = {

	{{"             Fastest\n                 Speed Thok\n             Not a good pick\nfor starters, but when\ncontrolled properly,\nSonic is the most\npowerful of the three."}, {"SONCCHAR"}},
	{{"             Slowest\n                 Fly/Swim\n             Good for\nbeginners. Tails\nhandles the best. His\nflying and swimming\nwill come in handy."}, {"TAILCHAR"}},
	{{"             Medium\n                 Glide/Climb\n             A well rounded\nchoice, Knuckles\ncompromises the speed\nof Sonic with the\nhandling of Tails."}, {"KNUXCHAR"}},
	{{"             Unknown\n                 Unknown\n             None"}, {"SONCCHAR"}},
	{{"             Unknown\n                 Unknown\n             None"}, {"SONCCHAR"}},
	{{"             Unknown\n                 Unknown\n             None"}, {"SONCCHAR"}},
	{{"             Unknown\n                 Unknown\n             None"}, {"SONCCHAR"}},
	{{"             Unknown\n                 Unknown\n             None"}, {"SONCCHAR"}},
};

// we are going to be entering a savegame string
int     saveStringEnter;
int     saveSlot;       // which slot to save in
int     saveSlotSelected; // Slot that the cursor is currently on Tails 05-29-2003
int     saveCharIndex;  // which char we're editing
// old save description before edit
char    saveOldString[SAVESTRINGSIZE];
char    savegamestrings[10][SAVESTRINGSIZE];

typedef struct
{
	char playername[SKINNAMESIZE];
	char levelname[32];
	byte actnum;
	byte skincolor;
	byte skinnum;
	byte numemeralds;
	int lives;
	int continues;
} saveinfo_t;

saveinfo_t     savegameinfo[10]; // Extra info about the save games.

static  char       setupm_ip[16]; // Tails 11-19-2002
int startmap; // Mario, NiGHTS, or just a plain old normal game?

// flags for items in the menu
// menu handle (what we do when key is pressed
#define  IT_TYPE             14     // (2+4+8)
#define  IT_CALL              0     // call the function
#define  IT_ARROWS            2     // call function with 0 for left arrow and 1 for right arrow in param
#define  IT_KEYHANDLER        4     // call with the key in param
#define  IT_SUBMENU           6     // go to sub menu
#define  IT_CVAR              8     // hangdle as a cvar
#define  IT_SPACE            10     // no handling
#define  IT_MSGHANDLER       12     // same as key but with event and sometime can handle y/n key (special for message

#define  IT_DISPLAY  (48+64+128)    // 16+32+64
#define  IT_NOTHING           0     // space
#define  IT_PATCH            16     // a patch or a string with big font
#define  IT_STRING           32     // little string (spaced with 10)
#define  IT_WHITESTRING      48     // little string in white
#define  IT_DYBIGSPACE       64     // same as noting
#define  IT_DYLITLSPACE  (16+64)    // little space
#define  IT_STRING2      (32+64)    // a simple string
#define  IT_GRAYPATCH    (16+32+64) // grayed patch or big font string
#define  IT_BIGSLIDER     (128)     // volume sound use this

//consvar specific
#define  IT_CVARTYPE   (256+512+1024)
#define  IT_CV_NORMAL         0
#define  IT_CV_SLIDER       256
#define  IT_CV_STRING       512
#define  IT_CV_NOPRINT (256+512)
#define  IT_CV_NOMOD       1024

// in short for some common use
#define  IT_BIGSPACE    (IT_SPACE  +IT_DYBIGSPACE)
#define  IT_LITLSPACE   (IT_SPACE  +IT_DYLITLSPACE)
#define  IT_CONTROL     (IT_STRING2+IT_CALL)
#define  IT_CVARMAX     (IT_CVAR   +IT_CV_NOMOD)
#define  IT_DISABLED    (IT_SPACE  +IT_GRAYPATCH)

typedef union
{
    struct menu_s     *submenu;               // IT_SUBMENU
    consvar_t         *cvar;                  // IT_CVAR
    void             (*routine)(int choice);  // IT_CALL, IT_KEYHANDLER, IT_ARROWS
} itemaction_t;

//
// MENU TYPEDEFS
//


typedef struct menu_s
{
    char            *menutitlepic;
    char            *menutitle;             // title as string for display with fontb if present
    short           numitems;               // # of menu items
    struct menu_s*  prevMenu;               // previous menu
    menuitem_t*     menuitems;              // menu items
    void            (*drawroutine)(void);   // draw routine
    short           x;
    short           y;                      // x,y of menu
    short           lastOn;                 // last item user was on in menu
    boolean         (*quitroutine)(void);   // called before quit a menu return true if we can
} menu_t;

// current menudef
menu_t*   currentMenu;
short     itemOn;                       // menu item skull is on
short     skullAnimCounter;             // skull animation counter
int       SkullBaseLump;

//
// PROTOTYPES
//
void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);

void M_DrawTextBox (int x, int y, int width, int lines);     //added:06-02-98:
void M_DrawThermo(int x,int y,consvar_t *cv);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_DrawSlider (int x, int y, int range);
void M_CentreText(int y, char* string);        //added:30-01-98:writetext centered

void M_StartControlPanel(void);
void M_StopMessage(int choice);
void M_ClearMenus (boolean callexitmenufunc);
int  M_StringHeight(char* string);
void M_GameOption(int choice);
void M_NetOption(int choice);
void M_MatchOptions(int choice);
void M_RaceOptions(int choice);
void M_TagOptions(int choice);
void M_CTFOptions(int choice);
void M_ChaosOptions(int choice);
#ifdef CIRCUITMODE
void M_CircuitOptions(int choice);
#endif
//28/08/99: added by Hurdler
void M_OpenGLOption(int choice);

void I_PlayCD(int track, boolean looping); // Tails 07-04-2002
void P_DestroyRobots(void); // Tails 08-14-2002

void M_ExitGameResponse(int ch);

menu_t MainDef,SinglePlayerDef,MultiPlayerDef,SetupMultiPlayerDef,
       EpiDef,NewDef,OptionsDef,VidModeDef,ControlDef,SoundDef,
       ReadDef2,ReadDef1,SaveDef,LoadDef,ControlDef2,GameOptionDef,
       NetOptionDef,EnemyToggleDef,MonitorToggleDef,SecretsDef,VideoOptionsDef,MouseOptionsDef,ServerOptionsDef,
	   PlayerDef,ConnectIPDef,RewardDef,LevelSelectDef; // Tails 03-02-2002

const char *ALLREADYPLAYING="You are already playing.\nDo you wish to end the\ncurrent game? (Y/N)\n";

//===========================================================================
//Generic Stuffs (more easy to create menus :))
//===========================================================================

void M_DrawMenuTitle(void)
{
    if( FontBBaseLump && currentMenu->menutitle )
    {
        int xtitle = (BASEVIDWIDTH-V_TextBWidth(currentMenu->menutitle))/2;
        int ytitle = (currentMenu->y-V_TextBHeight(currentMenu->menutitle))/2;
        if(xtitle<0) xtitle=0;
        if(ytitle<0) ytitle=0;

        V_DrawTextB(currentMenu->menutitle, xtitle, ytitle);
    }
    else
    if( currentMenu->menutitlepic )
    {
        patch_t* p = W_CachePatchName(currentMenu->menutitlepic,PU_CACHE);

        int xtitle = (BASEVIDWIDTH-p->width)/2;
        int ytitle = (currentMenu->y-p->height)/2;

        if(xtitle<0) xtitle=0;
        if(ytitle<0) ytitle=0;
        V_DrawScaledPatch (xtitle,ytitle,0,p);
    }
}

void M_DrawGenericMenu(void)
{
    int x,y,i,cursory=0;

    // DRAW MENU
    x = currentMenu->x;
    y = currentMenu->y;

    // draw title (or big pic)
    M_DrawMenuTitle();

    for (i=0;i<currentMenu->numitems;i++)
    {
        if (i==itemOn)
            cursory=y;
        switch (currentMenu->menuitems[i].status & IT_DISPLAY) {
           case IT_PATCH  :
               if( FontBBaseLump && currentMenu->menuitems[i].text )
               {
                   V_DrawTextB(currentMenu->menuitems[i].text, x, y);
                   y += FONTBHEIGHT-LINEHEIGHT;
               }
               else 
               if( currentMenu->menuitems[i].patch &&
                   currentMenu->menuitems[i].patch[0] )
                   V_DrawScaledPatch (x,y,0,
                                      W_CachePatchName(currentMenu->menuitems[i].patch ,PU_CACHE));
           case IT_NOTHING:
           case IT_DYBIGSPACE:
               y += LINEHEIGHT;
               break;
           case IT_BIGSLIDER :
               M_DrawThermo( x, y, (consvar_t *)currentMenu->menuitems[i].itemaction);
               y += LINEHEIGHT;
               break;
           case IT_STRING :
           case IT_WHITESTRING :
               if( currentMenu->menuitems[i].alphaKey )
                   y = currentMenu->y+currentMenu->menuitems[i].alphaKey;
               if (i==itemOn)
                   cursory=y;

               if( (currentMenu->menuitems[i].status & IT_DISPLAY)==IT_STRING ) 
                   V_DrawString(x,y,0,currentMenu->menuitems[i].text);
               else
                   V_DrawString(x,y,V_WHITEMAP,currentMenu->menuitems[i].text);

               // Cvar specific handling
               switch (currentMenu->menuitems[i].status & IT_TYPE)
                   case IT_CVAR:
                   {
                    consvar_t *cv=(consvar_t *)currentMenu->menuitems[i].itemaction;
                    switch (currentMenu->menuitems[i].status & IT_CVARTYPE) {
                       case IT_CV_SLIDER :
                           M_DrawSlider (BASEVIDWIDTH-x-SLIDER_WIDTH,
                                         y,
                                         ( (cv->value - cv->PossibleValue[0].value) * 100 /
                                         (cv->PossibleValue[1].value - cv->PossibleValue[0].value)));
                       case IT_CV_NOPRINT: // color use this 
                           break;
                       case IT_CV_STRING:
                           M_DrawTextBox(x,y+4,MAXSTRINGLENGTH,1);
                           V_DrawString (x+8,y+12,0,cv->string);
                           if( skullAnimCounter<4 && i==itemOn )
                               V_DrawCharacter( x+8+V_StringWidth(cv->string),
                                                y+12,
                                                '_' | 0x80);
                           y+=16;
                           break;
                       default:
                           V_DrawString(BASEVIDWIDTH-x-V_StringWidth (cv->string),
                                        y, V_WHITEMAP, 
                                        cv->string);
                           break;
                   }
                   break;
               }
               y+=STRINGHEIGHT;
               break;
           case IT_STRING2:
               V_DrawString (x,y,0,currentMenu->menuitems[i].text);
           case IT_DYLITLSPACE:
               y+=SMALLLINEHEIGHT;
               break;
           case IT_GRAYPATCH:
               if( FontBBaseLump && currentMenu->menuitems[i].text )
               {
                   V_DrawTextBGray(currentMenu->menuitems[i].text, x, y);
                   y += FONTBHEIGHT-LINEHEIGHT;
               }
               else 
               if( currentMenu->menuitems[i].patch &&
                   currentMenu->menuitems[i].patch[0] )
                   V_DrawMappedPatch (x,y,0,
                                      W_CachePatchName(currentMenu->menuitems[i].patch ,PU_CACHE),
                                      graymap);
               y += LINEHEIGHT;
               break;

        }
    }

    // DRAW THE SKULL CURSOR
    if (((currentMenu->menuitems[itemOn].status & IT_DISPLAY)==IT_PATCH)
      ||((currentMenu->menuitems[itemOn].status & IT_DISPLAY)==IT_NOTHING) )
    {
        V_DrawScaledPatch(currentMenu->x + SKULLXOFF,
                          cursory-5,
                          0,
                          W_CachePatchName("M_CURSOR",PU_CACHE) );
    }
    else
    {
		V_DrawScaledPatch(currentMenu->x - 24, cursory, 0, W_CachePatchName("M_CURSOR", PU_CACHE));
		V_DrawString (currentMenu->x,cursory, V_WHITEMAP,
                                    currentMenu->menuitems[itemOn].text);
    }

}

//===========================================================================
//MAIN MENU
//===========================================================================

void M_QuitDOOM(int choice);
void M_OptionsMenu(int choice); // Tails
void M_SecretsMenu (int choice);
// big overhaul Tails 11-30-2000
enum
{
	secrets = 0,
    singleplr,
    multiplr,
    options,
	dummy, // Eh? Why does it want this here? Tails 09-15-2002
    quitdoom,
    main_end
} main_e;

menuitem_t MainMenu[]=
{
	{IT_CALL   | IT_STRING,0, "        ???", M_SecretsMenu, 84},
    {IT_SUBMENU | IT_STRING,0,"   1  player" ,&SinglePlayerDef, 92},
    {IT_SUBMENU | IT_STRING,0,"multiplayer",&MultiPlayerDef ,100},
    {IT_CALL    | IT_STRING,0,"    options"  ,M_OptionsMenu     ,108},
    {IT_CALL    | IT_STRING,0,"  quit  game" ,M_QuitDOOM      ,116}
};

menu_t  MainDef =
{
    NULL, // Tails 12-01-2000
    NULL, // Tails 03-26-2001
    main_end,
    NULL,
    MainMenu,
    M_DrawGenericMenu,
    116,64, // Tails
    0
};

void M_DrawStats(void);
void M_DrawStats2(void);
void M_DrawStats3(void);
void M_DrawStats4(void);
void M_DrawStats5(void);
void M_Stats2(int choice);
void M_Stats3(int choice);
void M_Stats4(int choice);
void M_Stats5(int choice);

menu_t  StatsDef;
menu_t  Stats2Def;
menu_t  Stats3Def;
menu_t  Stats4Def;

// Empty thingy for stats5 menu
enum
{
    statsempty5,
    stats5_end
} stats5_e;

menuitem_t Stats5Menu[] =
{
    {IT_SUBMENU | IT_STRING,0,"NEXT",&StatsDef, 192}
};

menu_t  Stats5Def =
{
    NULL,
    NULL,
    stats5_end,
    &Stats4Def,
    Stats5Menu,
    M_DrawStats5,
    280,185,
    0
};

// Empty thingy for stats4 menu
enum
{
    statsempty4,
    stats4_end
} stats4_e;

menuitem_t Stats4Menu[] =
{
    {IT_SUBMENU | IT_STRING,0,"NEXT",&Stats5Def, 192}
};

menu_t  Stats4Def =
{
    NULL,
    NULL,
    stats4_end,
    &Stats3Def,
    Stats4Menu,
    M_DrawStats4,
    280,185,
    0
};

// Empty thingy for stats3 menu
enum
{
    statsempty3,
    stats3_end
} stats3_e;

menuitem_t Stats3Menu[] =
{
    {IT_CALL | IT_STRING,0,"NEXT",M_Stats4, 192}
};

menu_t  Stats3Def =
{
    NULL,
    NULL,
    stats3_end,
    &Stats2Def,
    Stats3Menu,
    M_DrawStats3,
    280,185,
    0
};

// Empty thingy for stats2 menu
enum
{
    statsempty2,
    stats2_end
} stats2_e;

menuitem_t Stats2Menu[] =
{
    {IT_CALL | IT_STRING,0,"NEXT",M_Stats3, 192}
};

menu_t  Stats2Def =
{
    NULL,
    NULL,
    stats2_end,
    &StatsDef,
    Stats2Menu,
    M_DrawStats2,
    280,185,
    0
};

// Empty thingy for stats menu
enum
{
    statsempty1,
    stats_end
} stats_e;

menuitem_t StatsMenu[] =
{
    {IT_CALL | IT_STRING,0,"NEXT",M_Stats2, 192}
};

menu_t  StatsDef =
{
    NULL,
    NULL,
    stats_end,
    &MainDef,
    StatsMenu,
    M_DrawStats,
    280,185,
    0
};

//===========================================================================
//SINGLE PLAYER MENU
//===========================================================================
// Menu Revamp! Tails 11-30-2000
void M_NewGame(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_EndGame(int choice);
void M_Statistics(int choice);

enum
{
    newgame = 0,
    loadgame,
    savegame,
    endgame,
	statistics,
    single_end
} single_e;

menuitem_t SinglePlayerMenu[] =
{
    {IT_CALL | IT_STRING,0,"New Game",M_NewGame, 80},
    {IT_CALL | IT_STRING,0,"Load Game",M_LoadGame, 90},
    {IT_CALL | IT_STRING,0,"Save Game",M_SaveGame, 100},
    {IT_CALL | IT_STRING,0,"End Game",M_EndGame, 110},
	{IT_CALL | IT_STRING,0,"Statistics", M_Statistics, 120},
};

menu_t  SinglePlayerDef =
{
    0,
    "Single Player",
    single_end,
    &MainDef,
    SinglePlayerMenu,
    M_DrawGenericMenu,
    130,64, // Tails 11-30-2000
    0
};

//===========================================================================
// Connect Menu
//===========================================================================

CV_PossibleValue_t serversearch_cons_t[] = {{0,"Local Lan"}
//                                           ,{1,"Internet"}
                                           ,{0,NULL}};


consvar_t cv_serversearch = {"serversearch"    ,"0",CV_HIDEN,serversearch_cons_t};

#define FIRSTSERVERLINE 3

void M_Connect( int choice )
{
    // do not call menuexitfunc 
    M_ClearMenus(false);

    COM_BufAddText(va("connect node %d\n", serverlist[choice-FIRSTSERVERLINE].node));
}

// Tails 11-19-2002
void M_ConnectIP( int choice )
{
    COM_BufAddText(va("connect %s\n", setupm_ip));
}

static int localservercount;

void M_Refresh( int choise )
{
    CL_UpdateServerList( cv_serversearch.value );
}

menuitem_t  ConnectMenu[] =
{
    {IT_STRING | IT_CVAR ,0,"Search On"       ,&cv_serversearch       ,0},
    {IT_STRING | IT_CALL ,0,"Refresh"         ,M_Refresh              ,0},
    {IT_WHITESTRING | IT_SPACE,0,"Server Name                      ping plys gt" ,0 ,0}, // Tails 01-18-2001
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
    {IT_STRING | IT_SPACE,0,""             ,M_Connect              ,0},
};

void M_DrawConnectMenu( void )
{
    int i;
    char *p;

    for( i=FIRSTSERVERLINE; i<localservercount+FIRSTSERVERLINE; i++ )
        ConnectMenu[i].status = IT_STRING | IT_SPACE;

    if( serverlistcount <= 0 )
        V_DrawString (currentMenu->x,currentMenu->y+FIRSTSERVERLINE*STRINGHEIGHT,0,"No server found");
    else
    for( i=0;i<serverlistcount && i+FIRSTSERVERLINE<sizeof(ConnectMenu)/sizeof(menuitem_t);i++ )
    {
        V_DrawString (currentMenu->x,currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT,0,serverlist[i].info.servername);
        p = va("%d", serverlist[i].info.time);
        V_DrawString (currentMenu->x+200-V_StringWidth(p),currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT,0,p);
        p = va("%d/%d  %d", serverlist[i].info.numberofplayer,
                            serverlist[i].info.maxplayer,
                            serverlist[i].info.gametype); // Tails 01-18-2001
        V_DrawString (currentMenu->x+250-V_StringWidth(p),currentMenu->y+(FIRSTSERVERLINE+i)*STRINGHEIGHT,0,p);

        ConnectMenu[i+FIRSTSERVERLINE].status = IT_STRING | IT_CALL;
    }
    localservercount = serverlistcount;

    M_DrawGenericMenu();
}

boolean M_CancelConnect(void)
{
    D_CloseConnection();
    return true;
}

menu_t  Connectdef =
{
    0,
    "Connect Server",
    sizeof(ConnectMenu)/sizeof(menuitem_t),
    &MultiPlayerDef,
    ConnectMenu,
    M_DrawConnectMenu,
    27,40,
    0,
    M_CancelConnect
};

// Connect using IP address Tails 11-19-2002
void M_HandleConnectIP(int choice);
menuitem_t  ConnectIPMenu[] =
{
    {IT_KEYHANDLER | IT_STRING          ,0,"  IP Address:" , M_HandleConnectIP, 0},
};

void M_DrawConnectIPMenu(void);

menu_t  ConnectIPdef =
{
    0,
    "Connect Server",
    sizeof(ConnectIPMenu)/sizeof(menuitem_t),
    &MultiPlayerDef,
    ConnectIPMenu,
    M_DrawConnectIPMenu,
    27,40,
    0,
    M_CancelConnect
};

void M_ConnectMenu(int choise)
{
    if( Playing() )
    {
        M_StartMessage(ALLREADYPLAYING,M_ExitGameResponse,MM_YESNO);
        return;
    }

    M_SetupNextMenu(&Connectdef);
    M_Refresh(0);
}

// Connect using IP address Tails 11-19-2002
void M_ConnectIPMenu(int choise)
{
    if( Playing() )
    {
        M_StartMessage(ALLREADYPLAYING,M_ExitGameResponse,MM_YESNO);
        return;
    }

    M_SetupNextMenu(&ConnectIPdef);
    M_Refresh(0);
}

//===========================================================================
// Start Server Menu
//===========================================================================

CV_PossibleValue_t skill_cons_t[] = {{2,"Easy"} // Tails 01-18-2001
                                    ,{3,"Normal"} // Tails 01-18-2001
                                    ,{4,"Hard"} // Tails 01-18-2001
                                    ,{5,"Very Hard" } // Tails 01-18-2001
                                    ,{0,NULL}};

CV_PossibleValue_t map_cons_t[LEVELARRAYSIZE] = {{1,"MAP01"},
												{2,"MAP02"},
												{3,"MAP03"},
												{4,"MAP04"},
												{5,"MAP05"},
												{6,"MAP06"},
												{7,"MAP07"},
												{8,"MAP08"},
												{9,"MAP09"},
												{10,"MAP10"},
												{11,"MAP11"},
												{12,"MAP12"},
												{13,"MAP13"},
												{14,"MAP14"},
												{15,"MAP15"},
												{16,"MAP16"},
												{17,"MAP17"},
												{18,"MAP18"},
												{19,"MAP19"},
												{20,"MAP20"},
												{21,"MAP21"},
												{22,"MAP22"},
												{23,"MAP23"},
												{24,"MAP24"},
												{25,"MAP25"},
												{26,"MAP26"},
												{27,"MAP27"},
												{28,"MAP28"},
												{29,"MAP29"},
												{30,"MAP30"},
												{31,"MAP31"},
												{32,"MAP32"},
												{33,"MAP33"},
												{34,"MAP34"},
												{35,"MAP35"},
												{36,"MAP36"},
												{37,"MAP37"},
												{38,"MAP38"},
												{39,"MAP39"},
												{40,"MAP40"},
												{41,"MAP41"},
												{42,"MAP42"},
												{43,"MAP43"},
												{44,"MAP44"},
												{45,"MAP45"},
												{46,"MAP46"},
												{47,"MAP47"},
												{48,"MAP48"},
												{49,"MAP49"},
												{50,"MAP50"},
												{51,"MAP51"},
												{52,"MAP52"},
												{53,"MAP53"},
												{54,"MAP54"},
												{55,"MAP55"},
												{56,"MAP56"},
												{57,"MAP57"},
												{58,"MAP58"},
												{59,"MAP59"},
												{60,"MAP60"},
												{61,"MAP61"},
												{62,"MAP62"},
												{63,"MAP63"},
												{64,"MAP64"},
												{65,"MAP65"},
												{66,"MAP66"},
												{67,"MAP67"},
												{68,"MAP68"},
												{69,"MAP69"},
												{70,"MAP70"},
												{71,"MAP71"},
												{72,"MAP72"},
												{73,"MAP73"},
												{74,"MAP74"},
												{75,"MAP75"},
												{76,"MAP76"},
												{77,"MAP77"},
												{78,"MAP78"},
												{79,"MAP79"},
												{80,"MAP80"},
												{81,"MAP81"},
												{82,"MAP82"},
												{83,"MAP83"},
												{84,"MAP84"},
												{85,"MAP85"},
												{86,"MAP86"},
												{87,"MAP87"},
												{88,"MAP88"},
												{89,"MAP89"},
												{90,"MAP90"},
												{91,"MAP91"},
												{92,"MAP92"},
												{93,"MAP93"},
												{94,"MAP94"},
												{95,"MAP95"},
												{96,"MAP96"},
												{97,"MAP97"},
												{98,"MAP98"},
												{99,"MAP99"},
												{100, "MAP100"},
												{101, "MAP101"},
												{102, "MAP102"},
												{103, "MAP103"},
												{104, "MAP104"},
												{105, "MAP105"},
												{106, "MAP106"},
												{107, "MAP107"},
												{108, "MAP108"},
												{109, "MAP109"},
												{110, "MAP110"},
												{111, "MAP111"},
												{112, "MAP112"},
												{113, "MAP113"},
												{114, "MAP114"},
												{115, "MAP115"},
												{116, "MAP116"},
												{117, "MAP117"},
												{118, "MAP118"},
												{119, "MAP119"},
												{120, "MAP120"},
												{121, "MAP121"},
												{122, "MAP122"},
												{123, "MAP123"},
												{124, "MAP124"},
												{125, "MAP125"},
												{126, "MAP126"},
												{127, "MAP127"},
												{128, "MAP128"},
												{129, "MAP129"},
												{130, "MAP130"},
												{131, "MAP131"},
												{132, "MAP132"},
												{133, "MAP133"},
												{134, "MAP134"},
												{135, "MAP135"},
												{136, "MAP136"},
												{137, "MAP137"},
												{138, "MAP138"},
												{139, "MAP139"},
												{140, "MAP140"},
												{141, "MAP141"},
												{142, "MAP142"},
												{143, "MAP143"},
												{144, "MAP144"},
												{145, "MAP145"},
												{146, "MAP146"},
												{147, "MAP147"},
												{148, "MAP148"},
												{149, "MAP149"},
												{150, "MAP150"},
												{151, "MAP151"},
												{152, "MAP152"},
												{153, "MAP153"},
												{154, "MAP154"},
												{155, "MAP155"},
												{156, "MAP156"},
												{157, "MAP157"},
												{158, "MAP158"},
												{159, "MAP159"},
												{160, "MAP160"},
												{161, "MAP161"},
												{162, "MAP162"},
												{163, "MAP163"},
												{164, "MAP164"},
												{165, "MAP165"},
												{166, "MAP166"},
												{167, "MAP167"},
												{168, "MAP168"},
												{169, "MAP169"},
												{170, "MAP170"},
												{171, "MAP171"},
												{172, "MAP172"},
												{173, "MAP173"},
												{174, "MAP174"},
												{175, "MAP175"},
												{176, "MAP176"},
												{177, "MAP177"},
												{178, "MAP178"},
												{179, "MAP179"},
												{180, "MAP180"},
												{181, "MAP181"},
												{182, "MAP182"},
												{183, "MAP183"},
												{184, "MAP184"},
												{185, "MAP185"},
												{186, "MAP186"},
												{187, "MAP187"},
												{188, "MAP188"},
												{189, "MAP189"},
												{190, "MAP190"},
												{191, "MAP191"},
												{192, "MAP192"},
												{193, "MAP193"},
												{194, "MAP194"},
												{195, "MAP195"},
												{196, "MAP196"},
												{197, "MAP197"},
												{198, "MAP198"},
												{199, "MAP199"},
												{200, "MAP200"},
												{201, "MAP201"},
												{202, "MAP202"},
												{203, "MAP203"},
												{204, "MAP204"},
												{205, "MAP205"},
												{206, "MAP206"},
												{207, "MAP207"},
												{208, "MAP208"},
												{209, "MAP209"},
												{210, "MAP210"},
												{211, "MAP211"},
												{212, "MAP212"},
												{213, "MAP213"},
												{214, "MAP214"},
												{215, "MAP215"},
												{216, "MAP216"},
												{217, "MAP217"},
												{218, "MAP218"},
												{219, "MAP219"},
												{220, "MAP220"},
												{221, "MAP221"},
												{222, "MAP222"},
												{223, "MAP223"},
												{224, "MAP224"},
												{225, "MAP225"},
												{226, "MAP226"},
												{227, "MAP227"},
												{228, "MAP228"},
												{229, "MAP229"},
												{230, "MAP230"},
												{231, "MAP231"},
												{232, "MAP232"},
												{233, "MAP233"},
												{234, "MAP234"},
												{235, "MAP235"},
												{236, "MAP236"},
												{237, "MAP237"},
												{238, "MAP238"},
												{239, "MAP239"},
												{240, "MAP240"},
												{241, "MAP241"},
												{242, "MAP242"},
												{243, "MAP243"},
												{244, "MAP244"},
												{245, "MAP245"},
												{246, "MAP246"},
												{247, "MAP247"},
												{248, "MAP248"},
												{249, "MAP249"},
												{250, "MAP250"},
												{251, "MAP251"},
												{252, "MAP252"},
												{253, "MAP253"},
												{254, "MAP254"},
												{255, "MAP255"},
												{256, "MAP256"},
												{257, "MAP257"},
												{258, "MAP258"},
												{259, "MAP259"},
												{260, "MAP260"},
												{261, "MAP261"},
												{262, "MAP262"},
												{263, "MAP263"},
												{264, "MAP264"},
												{265, "MAP265"},
												{266, "MAP266"},
												{267, "MAP267"},
												{268, "MAP268"},
												{269, "MAP269"},
												{270, "MAP270"},
												{271, "MAP271"},
												{272, "MAP272"},
												{273, "MAP273"},
												{274, "MAP274"},
												{275, "MAP275"},
												{276, "MAP276"},
												{277, "MAP277"},
												{278, "MAP278"},
												{279, "MAP279"},
												{280, "MAP280"},
												{281, "MAP281"},
												{282, "MAP282"},
												{283, "MAP283"},
												{284, "MAP284"},
												{285, "MAP285"},
												{286, "MAP286"},
												{287, "MAP287"},
												{288, "MAP288"},
												{289, "MAP289"},
												{290, "MAP290"},
												{291, "MAP291"},
												{292, "MAP292"},
												{293, "MAP293"},
												{294, "MAP294"},
												{295, "MAP295"},
												{296, "MAP296"},
												{297, "MAP297"},
												{298, "MAP298"},
												{299, "MAP299"},
												{300, "MAP300"},
												{301, "MAP301"},
												{302, "MAP302"},
												{303, "MAP303"},
												{304, "MAP304"},
												{305, "MAP305"},
												{306, "MAP306"},
												{307, "MAP307"},
												{308, "MAP308"},
												{309, "MAP309"},
												{310, "MAP310"},
												{311, "MAP311"},
												{312, "MAP312"},
												{313, "MAP313"},
												{314, "MAP314"},
												{315, "MAP315"},
												{316, "MAP316"},
												{317, "MAP317"},
												{318, "MAP318"},
												{319, "MAP319"},
												{320, "MAP320"},
												{321, "MAP321"},
												{322, "MAP322"},
												{323, "MAP323"},
												{324, "MAP324"},
												{325, "MAP325"},
												{326, "MAP326"},
												{327, "MAP327"},
												{328, "MAP328"},
												{329, "MAP329"},
												{330, "MAP330"},
												{331, "MAP331"},
												{332, "MAP332"},
												{333, "MAP333"},
												{334, "MAP334"},
												{335, "MAP335"},
												{336, "MAP336"},
												{337, "MAP337"},
												{338, "MAP338"},
												{339, "MAP339"},
												{340, "MAP340"},
												{341, "MAP341"},
												{342, "MAP342"},
												{343, "MAP343"},
												{344, "MAP344"},
												{345, "MAP345"},
												{346, "MAP346"},
												{347, "MAP347"},
												{348, "MAP348"},
												{349, "MAP349"},
												{350, "MAP350"},
												{351, "MAP351"},
												{352, "MAP352"},
												{353, "MAP353"},
												{354, "MAP354"},
												{355, "MAP355"},
												{356, "MAP356"},
												{357, "MAP357"},
												{358, "MAP358"},
												{359, "MAP359"},
												{360, "MAP360"},
												{361, "MAP361"},
												{362, "MAP362"},
												{363, "MAP363"},
												{364, "MAP364"},
												{365, "MAP365"},
												{366, "MAP366"},
												{367, "MAP367"},
												{368, "MAP368"},
												{369, "MAP369"},
												{370, "MAP370"},
												{371, "MAP371"},
												{372, "MAP372"},
												{373, "MAP373"},
												{374, "MAP374"},
												{375, "MAP375"},
												{376, "MAP376"},
												{377, "MAP377"},
												{378, "MAP378"},
												{379, "MAP379"},
												{380, "MAP380"},
												{381, "MAP381"},
												{382, "MAP382"},
												{383, "MAP383"},
												{384, "MAP384"},
												{385, "MAP385"},
												{386, "MAP386"},
												{387, "MAP387"},
												{388, "MAP388"},
												{389, "MAP389"},
												{390, "MAP390"},
												{391, "MAP391"},
												{392, "MAP392"},
												{393, "MAP393"},
												{394, "MAP394"},
												{395, "MAP395"},
												{396, "MAP396"},
												{397, "MAP397"},
												{398, "MAP398"},
												{399, "MAP399"},
												{400, "MAP400"},
												{401, "MAP401"},
												{402, "MAP402"},
												{403, "MAP403"},
												{404, "MAP404"},
												{405, "MAP405"},
												{406, "MAP406"},
												{407, "MAP407"},
												{408, "MAP408"},
												{409, "MAP409"},
												{410, "MAP410"},
												{411, "MAP411"},
												{412, "MAP412"},
												{413, "MAP413"},
												{414, "MAP414"},
												{415, "MAP415"},
												{416, "MAP416"},
												{417, "MAP417"},
												{418, "MAP418"},
												{419, "MAP419"},
												{420, "MAP420"},
												{421, "MAP421"},
												{422, "MAP422"},
												{423, "MAP423"},
												{424, "MAP424"},
												{425, "MAP425"},
												{426, "MAP426"},
												{427, "MAP427"},
												{428, "MAP428"},
												{429, "MAP429"},
												{430, "MAP430"},
												{431, "MAP431"},
												{432, "MAP432"},
												{433, "MAP433"},
												{434, "MAP434"},
												{435, "MAP435"},
												{436, "MAP436"},
												{437, "MAP437"},
												{438, "MAP438"},
												{439, "MAP439"},
												{440, "MAP440"},
												{441, "MAP441"},
												{442, "MAP442"},
												{443, "MAP443"},
												{444, "MAP444"},
												{445, "MAP445"},
												{446, "MAP446"},
												{447, "MAP447"},
												{448, "MAP448"},
												{449, "MAP449"},
												{450, "MAP450"},
												{451, "MAP451"},
												{452, "MAP452"},
												{453, "MAP453"},
												{454, "MAP454"},
												{455, "MAP455"},
												{456, "MAP456"},
												{457, "MAP457"},
												{458, "MAP458"},
												{459, "MAP459"},
												{460, "MAP460"},
												{461, "MAP461"},
												{462, "MAP462"},
												{463, "MAP463"},
												{464, "MAP464"},
												{465, "MAP465"},
												{466, "MAP466"},
												{467, "MAP467"},
												{468, "MAP468"},
												{469, "MAP469"},
												{470, "MAP470"},
												{471, "MAP471"},
												{472, "MAP472"},
												{473, "MAP473"},
												{474, "MAP474"},
												{475, "MAP475"},
												{476, "MAP476"},
												{477, "MAP477"},
												{478, "MAP478"},
												{479, "MAP479"},
												{480, "MAP480"},
												{481, "MAP481"},
												{482, "MAP482"},
												{483, "MAP483"},
												{484, "MAP484"},
												{485, "MAP485"},
												{486, "MAP486"},
												{487, "MAP487"},
												{488, "MAP488"},
												{489, "MAP489"},
												{490, "MAP490"},
												{491, "MAP491"},
												{492, "MAP492"},
												{493, "MAP493"},
												{494, "MAP494"},
												{495, "MAP495"},
												{496, "MAP496"},
												{497, "MAP497"},
												{498, "MAP498"},
												{499, "MAP499"},
												{500, "MAP500"},
												{501, "MAP501"},
												{502, "MAP502"},
												{503, "MAP503"},
												{504, "MAP504"},
												{505, "MAP505"},
												{506, "MAP506"},
												{507, "MAP507"},
												{508, "MAP508"},
												{509, "MAP509"},
												{510, "MAP510"},
												{511, "MAP511"},
												{512, "MAP512"},
												{513, "MAP513"},
												{514, "MAP514"},
												{515, "MAP515"},
												{516, "MAP516"},
												{517, "MAP517"},
												{518, "MAP518"},
												{519, "MAP519"},
												{520, "MAP520"},
												{521, "MAP521"},
												{522, "MAP522"},
												{523, "MAP523"},
												{524, "MAP524"},
												{525, "MAP525"},
												{526, "MAP526"},
												{527, "MAP527"},
												{528, "MAP528"},
												{529, "MAP529"},
												{530, "MAP530"},
												{531, "MAP531"},
												{532, "MAP532"},
												{533, "MAP533"},
												{534, "MAP534"},
												{535, "MAP535"},
												{536, "MAP536"},
												{537, "MAP537"},
												{538, "MAP538"},
												{539, "MAP539"},
												{540, "MAP540"},
												{541, "MAP541"},
												{542, "MAP542"},
												{543, "MAP543"},
												{544, "MAP544"},
												{545, "MAP545"},
												{546, "MAP546"},
												{547, "MAP547"},
												{548, "MAP548"},
												{549, "MAP549"},
												{550, "MAP550"},
												{551, "MAP551"},
												{552, "MAP552"},
												{553, "MAP553"},
												{554, "MAP554"},
												{555, "MAP555"},
												{556, "MAP556"},
												{557, "MAP557"},
												{558, "MAP558"},
												{559, "MAP559"},
												{560, "MAP560"},
												{561, "MAP561"},
												{562, "MAP562"},
												{563, "MAP563"},
												{564, "MAP564"},
												{565, "MAP565"},
												{566, "MAP566"},
												{567, "MAP567"},
												{568, "MAP568"},
												{569, "MAP569"},
												{570, "MAP570"},
												{571, "MAP571"},
												{572, "MAP572"},
												{573, "MAP573"},
												{574, "MAP574"},
												{575, "MAP575"},
												{576, "MAP576"},
												{577, "MAP577"},
												{578, "MAP578"},
												{579, "MAP579"},
												{580, "MAP580"},
												{581, "MAP581"},
												{582, "MAP582"},
												{583, "MAP583"},
												{584, "MAP584"},
												{585, "MAP585"},
												{586, "MAP586"},
												{587, "MAP587"},
												{588, "MAP588"},
												{589, "MAP589"},
												{590, "MAP590"},
												{591, "MAP591"},
												{592, "MAP592"},
												{593, "MAP593"},
												{594, "MAP594"},
												{595, "MAP595"},
												{596, "MAP596"},
												{597, "MAP597"},
												{598, "MAP598"},
												{599, "MAP599"},
												{600, "MAP600"},
												{601, "MAP601"},
												{602, "MAP602"},
												{603, "MAP603"},
												{604, "MAP604"},
												{605, "MAP605"},
												{606, "MAP606"},
												{607, "MAP607"},
												{608, "MAP608"},
												{609, "MAP609"},
												{610, "MAP610"},
												{611, "MAP611"},
												{612, "MAP612"},
												{613, "MAP613"},
												{614, "MAP614"},
												{615, "MAP615"},
												{616, "MAP616"},
												{617, "MAP617"},
												{618, "MAP618"},
												{619, "MAP619"},
												{620, "MAP620"},
												{621, "MAP621"},
												{622, "MAP622"},
												{623, "MAP623"},
												{624, "MAP624"},
												{625, "MAP625"},
												{626, "MAP626"},
												{627, "MAP627"},
												{628, "MAP628"},
												{629, "MAP629"},
												{630, "MAP630"},
												{631, "MAP631"},
												{632, "MAP632"},
												{633, "MAP633"},
												{634, "MAP634"},
												{635, "MAP635"},
												{636, "MAP636"},
												{637, "MAP637"},
												{638, "MAP638"},
												{639, "MAP639"},
												{640, "MAP640"},
												{641, "MAP641"},
												{642, "MAP642"},
												{643, "MAP643"},
												{644, "MAP644"},
												{645, "MAP645"},
												{646, "MAP646"},
												{647, "MAP647"},
												{648, "MAP648"},
												{649, "MAP649"},
												{650, "MAP650"},
												{651, "MAP651"},
												{652, "MAP652"},
												{653, "MAP653"},
												{654, "MAP654"},
												{655, "MAP655"},
												{656, "MAP656"},
												{657, "MAP657"},
												{658, "MAP658"},
												{659, "MAP659"},
												{660, "MAP660"},
												{661, "MAP661"},
												{662, "MAP662"},
												{663, "MAP663"},
												{664, "MAP664"},
												{665, "MAP665"},
												{666, "MAP666"},
												{667, "MAP667"},
												{668, "MAP668"},
												{669, "MAP669"},
												{670, "MAP670"},
												{671, "MAP671"},
												{672, "MAP672"},
												{673, "MAP673"},
												{674, "MAP674"},
												{675, "MAP675"},
												{676, "MAP676"},
												{677, "MAP677"},
												{678, "MAP678"},
												{679, "MAP679"},
												{680, "MAP680"},
												{681, "MAP681"},
												{682, "MAP682"},
												{683, "MAP683"},
												{684, "MAP684"},
												{685, "MAP685"},
												{686, "MAP686"},
												{687, "MAP687"},
												{688, "MAP688"},
												{689, "MAP689"},
												{690, "MAP690"},
												{691, "MAP691"},
												{692, "MAP692"},
												{693, "MAP693"},
												{694, "MAP694"},
												{695, "MAP695"},
												{696, "MAP696"},
												{697, "MAP697"},
												{698, "MAP698"},
												{699, "MAP699"},
												{700, "MAP700"},
												{701, "MAP701"},
												{702, "MAP702"},
												{703, "MAP703"},
												{704, "MAP704"},
												{705, "MAP705"},
												{706, "MAP706"},
												{707, "MAP707"},
												{708, "MAP708"},
												{709, "MAP709"},
												{710, "MAP710"},
												{711, "MAP711"},
												{712, "MAP712"},
												{713, "MAP713"},
												{714, "MAP714"},
												{715, "MAP715"},
												{716, "MAP716"},
												{717, "MAP717"},
												{718, "MAP718"},
												{719, "MAP719"},
												{720, "MAP720"},
												{721, "MAP721"},
												{722, "MAP722"},
												{723, "MAP723"},
												{724, "MAP724"},
												{725, "MAP725"},
												{726, "MAP726"},
												{727, "MAP727"},
												{728, "MAP728"},
												{729, "MAP729"},
												{730, "MAP730"},
												{731, "MAP731"},
												{732, "MAP732"},
												{733, "MAP733"},
												{734, "MAP734"},
												{735, "MAP735"},
												{736, "MAP736"},
												{737, "MAP737"},
												{738, "MAP738"},
												{739, "MAP739"},
												{740, "MAP740"},
												{741, "MAP741"},
												{742, "MAP742"},
												{743, "MAP743"},
												{744, "MAP744"},
												{745, "MAP745"},
												{746, "MAP746"},
												{747, "MAP747"},
												{748, "MAP748"},
												{749, "MAP749"},
												{750, "MAP750"},
												{751, "MAP751"},
												{752, "MAP752"},
												{753, "MAP753"},
												{754, "MAP754"},
												{755, "MAP755"},
												{756, "MAP756"},
												{757, "MAP757"},
												{758, "MAP758"},
												{759, "MAP759"},
												{760, "MAP760"},
												{761, "MAP761"},
												{762, "MAP762"},
												{763, "MAP763"},
												{764, "MAP764"},
												{765, "MAP765"},
												{766, "MAP766"},
												{767, "MAP767"},
												{768, "MAP768"},
												{769, "MAP769"},
												{770, "MAP770"},
												{771, "MAP771"},
												{772, "MAP772"},
												{773, "MAP773"},
												{774, "MAP774"},
												{775, "MAP775"},
												{776, "MAP776"},
												{777, "MAP777"},
												{778, "MAP778"},
												{779, "MAP779"},
												{780, "MAP780"},
												{781, "MAP781"},
												{782, "MAP782"},
												{783, "MAP783"},
												{784, "MAP784"},
												{785, "MAP785"},
												{786, "MAP786"},
												{787, "MAP787"},
												{788, "MAP788"},
												{789, "MAP789"},
												{790, "MAP790"},
												{791, "MAP791"},
												{792, "MAP792"},
												{793, "MAP793"},
												{794, "MAP794"},
												{795, "MAP795"},
												{796, "MAP796"},
												{797, "MAP797"},
												{798, "MAP798"},
												{799, "MAP799"},
												{800, "MAP800"},
												{801, "MAP801"},
												{802, "MAP802"},
												{803, "MAP803"},
												{804, "MAP804"},
												{805, "MAP805"},
												{806, "MAP806"},
												{807, "MAP807"},
												{808, "MAP808"},
												{809, "MAP809"},
												{810, "MAP810"},
												{811, "MAP811"},
												{812, "MAP812"},
												{813, "MAP813"},
												{814, "MAP814"},
												{815, "MAP815"},
												{816, "MAP816"},
												{817, "MAP817"},
												{818, "MAP818"},
												{819, "MAP819"},
												{820, "MAP820"},
												{821, "MAP821"},
												{822, "MAP822"},
												{823, "MAP823"},
												{824, "MAP824"},
												{825, "MAP825"},
												{826, "MAP826"},
												{827, "MAP827"},
												{828, "MAP828"},
												{829, "MAP829"},
												{830, "MAP830"},
												{831, "MAP831"},
												{832, "MAP832"},
												{833, "MAP833"},
												{834, "MAP834"},
												{835, "MAP835"},
												{836, "MAP836"},
												{837, "MAP837"},
												{838, "MAP838"},
												{839, "MAP839"},
												{840, "MAP840"},
												{841, "MAP841"},
												{842, "MAP842"},
												{843, "MAP843"},
												{844, "MAP844"},
												{845, "MAP845"},
												{846, "MAP846"},
												{847, "MAP847"},
												{848, "MAP848"},
												{849, "MAP849"},
												{850, "MAP850"},
												{851, "MAP851"},
												{852, "MAP852"},
												{853, "MAP853"},
												{854, "MAP854"},
												{855, "MAP855"},
												{856, "MAP856"},
												{857, "MAP857"},
												{858, "MAP858"},
												{859, "MAP859"},
												{860, "MAP860"},
												{861, "MAP861"},
												{862, "MAP862"},
												{863, "MAP863"},
												{864, "MAP864"},
												{865, "MAP865"},
												{866, "MAP866"},
												{867, "MAP867"},
												{868, "MAP868"},
												{869, "MAP869"},
												{870, "MAP870"},
												{871, "MAP871"},
												{872, "MAP872"},
												{873, "MAP873"},
												{874, "MAP874"},
												{875, "MAP875"},
												{876, "MAP876"},
												{877, "MAP877"},
												{878, "MAP878"},
												{879, "MAP879"},
												{880, "MAP880"},
												{881, "MAP881"},
												{882, "MAP882"},
												{883, "MAP883"},
												{884, "MAP884"},
												{885, "MAP885"},
												{886, "MAP886"},
												{887, "MAP887"},
												{888, "MAP888"},
												{889, "MAP889"},
												{890, "MAP890"},
												{891, "MAP891"},
												{892, "MAP892"},
												{893, "MAP893"},
												{894, "MAP894"},
												{895, "MAP895"},
												{896, "MAP896"},
												{897, "MAP897"},
												{898, "MAP898"},
												{899, "MAP899"},
												{900, "MAP900"},
												{901, "MAP901"},
												{902, "MAP902"},
												{903, "MAP903"},
												{904, "MAP904"},
												{905, "MAP905"},
												{906, "MAP906"},
												{907, "MAP907"},
												{908, "MAP908"},
												{909, "MAP909"},
												{910, "MAP910"},
												{911, "MAP911"},
												{912, "MAP912"},
												{913, "MAP913"},
												{914, "MAP914"},
												{915, "MAP915"},
												{916, "MAP916"},
												{917, "MAP917"},
												{918, "MAP918"},
												{919, "MAP919"},
												{920, "MAP920"},
												{921, "MAP921"},
												{922, "MAP922"},
												{923, "MAP923"},
												{924, "MAP924"},
												{925, "MAP925"},
												{926, "MAP926"},
												{927, "MAP927"},
												{928, "MAP928"},
												{929, "MAP929"},
												{930, "MAP930"},
												{931, "MAP931"},
												{932, "MAP932"},
												{933, "MAP933"},
												{934, "MAP934"},
												{935, "MAP935"},
												{936, "MAP936"},
												{937, "MAP937"},
												{938, "MAP938"},
												{939, "MAP939"},
												{940, "MAP940"},
												{941, "MAP941"},
												{942, "MAP942"},
												{943, "MAP943"},
												{944, "MAP944"},
												{945, "MAP945"},
												{946, "MAP946"},
												{947, "MAP947"},
												{948, "MAP948"},
												{949, "MAP949"},
												{950, "MAP950"},
												{951, "MAP951"},
												{952, "MAP952"},
												{953, "MAP953"},
												{954, "MAP954"},
												{955, "MAP955"},
												{956, "MAP956"},
												{957, "MAP957"},
												{958, "MAP958"},
												{959, "MAP959"},
												{960, "MAP960"},
												{961, "MAP961"},
												{962, "MAP962"},
												{963, "MAP963"},
												{964, "MAP964"},
												{965, "MAP965"},
												{966, "MAP966"},
												{967, "MAP967"},
												{968, "MAP968"},
												{969, "MAP969"},
												{970, "MAP970"},
												{971, "MAP971"},
												{972, "MAP972"},
												{973, "MAP973"},
												{974, "MAP974"},
												{975, "MAP975"},
												{976, "MAP976"},
												{977, "MAP977"},
												{978, "MAP978"},
												{979, "MAP979"},
												{980, "MAP980"},
												{981, "MAP981"},
												{982, "MAP982"},
												{983, "MAP983"},
												{984, "MAP984"},
												{985, "MAP985"},
												{986, "MAP986"},
												{987, "MAP987"},
												{988, "MAP988"},
												{989, "MAP989"},
												{990, "MAP990"},
												{991, "MAP991"},
												{992, "MAP992"},
												{993, "MAP993"},
												{994, "MAP994"},
												{995, "MAP995"},
												{996, "MAP996"},
												{997, "MAP997"},
												{998, "MAP998"},
												{999, "MAP999"},
												{1000, "MAP1000"},
												{1001, "MAP1001"},
												{1002, "MAP1002"},
												{1003, "MAP1003"},
												{1004, "MAP1004"},
												{1005, "MAP1005"},
												{1006, "MAP1006"},
												{1007, "MAP1007"},
												{1008, "MAP1008"},
												{1009, "MAP1009"},
												{1010, "MAP1010"},
												{1011, "MAP1011"},
												{1012, "MAP1012"},
												{1013, "MAP1013"},
												{1014, "MAP1014"},
												{1015, "MAP1015"},
												{1016, "MAP1016"},
												{1017, "MAP1017"},
												{1018, "MAP1018"},
												{1019, "MAP1019"},
												{1020, "MAP1020"},
												{1021, "MAP1021"},
												{1022, "MAP1022"},
												{1023, "MAP1023"},
												{1024, "MAP1024"},
												{1025, "MAP1025"},
												{1026, "MAP1026"},
												{1027, "MAP1027"},
												{1028, "MAP1028"},
												{1029, "MAP1029"},
												{1030, "MAP1030"},
												{1031, "MAP1031"},
												{1032, "MAP1032"},
												{1033, "MAP1033"},
												{1034, "MAP1034"},
												{1035, "MAP1035"},
												{32767, NULL}
};

consvar_t cv_skill    = {"skill"    ,"3",CV_HIDEN,skill_cons_t};
consvar_t cv_nextmap  = {"nextmap"  ,"1",CV_HIDEN,map_cons_t};
extern CV_PossibleValue_t gametype_cons_t[];
consvar_t cv_newgametype  = {"newgametype"  ,"1",CV_HIDEN,gametype_cons_t}; // Tails 01-18-2001
static boolean StartSplitScreenGame;

void M_StartServer( int choise )
{
	if(StartSplitScreenGame == false)
		netgame = true;

    multiplayer = true;

	if(StartSplitScreenGame == false)
    COM_BufAddText(va("stopdemo;splitscreen %d;gametype %d;map \"%s\" skill %d\n",  // Tails 01-18-2001
                      StartSplitScreenGame, cv_newgametype.value,  // Tails 01-18-2001
                      G_BuildMapName(cv_nextmap.value), cv_skill.value));
	else
	{
		G_DeferedInitNew(cv_skill.value-1, G_BuildMapName(cv_nextmap.value), cv_skin.value, StartSplitScreenGame);
		if(cv_newgametype.value != 0)
		{
			COM_BufAddText(va("gametype %d;map \"%s\"",  // Tails 01-18-2001
                      cv_newgametype.value,  // Tails 01-18-2001
                      G_BuildMapName(cv_nextmap.value) ));
		}
	}

    M_ClearMenus(true);
}

menuitem_t  ServerMenu[] =
{
    {IT_STRING | IT_CVAR,0,"Level"             ,&cv_nextmap          ,0},
    {IT_STRING | IT_CVAR,0,"Skill"           ,&cv_skill            ,0},

    {IT_STRING | IT_CVAR,0,"Game Type" ,&cv_newgametype    ,0}, // Tails 01-18-2001
                         
    {IT_STRING | IT_CVAR,0,"Advertise on Internet" ,&cv_internetserver   ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_STRING     ,0,"Server Name"     ,&cv_servername       ,0},
    {IT_WHITESTRING 
               | IT_CALL,0,"Start"           ,M_StartServer        ,120}
};

menu_t  Serverdef =
{
    0,
    "Start Server",
    sizeof(ServerMenu)/sizeof(menuitem_t),
    &MultiPlayerDef,
    ServerMenu,
    M_DrawGenericMenu,
    27,40,
    0,
};

void M_PatchLevelNameTable()
{
	int i,j;
	int currentmap;
	int origindex = 0;
	char typeofmap[8]; // Change from 7 to 8 Graue 12-07-2003
	int z;

	for(j=0; j<LEVELARRAYSIZE-2; j++)
	{
		i = 0;
		currentmap = map_cons_t[j].value-1;

		if(mapheaderinfo[currentmap].lvlttl[0] != '\0')
		{
			strncpy (lvltable[j], mapheaderinfo[currentmap].lvlttl, strlen(mapheaderinfo[currentmap].lvlttl));

			i += strlen(mapheaderinfo[currentmap].lvlttl);

			lvltable[j][i] = ' ';

			i++;

			if(mapheaderinfo[currentmap].actnum)
			{
				lvltable[j][i] = '0' + mapheaderinfo[currentmap].actnum;
				i++;
			}

			lvltable[j][i] = ' ';
			i++;

			if(!mapheaderinfo[currentmap].typeoflevel)
			{
				lvltable[j][i] = '(';
				i++;
				lvltable[j][i] = 'A';
				i++;
				lvltable[j][i] = 'L';
				i++;
				lvltable[j][i] = 'L';
				i++;
			}
			else
			{
				z = 0;

				if(mapheaderinfo[currentmap].typeoflevel & TOL_COOP)
					typeofmap[z++] = 'C';

				if(mapheaderinfo[currentmap].typeoflevel & TOL_MATCH)
					typeofmap[z++] = 'M';

				if(mapheaderinfo[currentmap].typeoflevel & TOL_RACE)
					typeofmap[z++] = 'R';

				if(mapheaderinfo[currentmap].typeoflevel & TOL_TAG)
					typeofmap[z++] = 'T';

				if(mapheaderinfo[currentmap].typeoflevel & TOL_CTF)
					typeofmap[z++] = 'F';

				if(mapheaderinfo[currentmap].typeoflevel & TOL_CHAOS)
					typeofmap[z++] = 'A'; // changed from S to A Graue 12-14-2003

				// Graue 12-07-2003
				if(mapheaderinfo[currentmap].typeoflevel & TOL_CIRCUIT)
					typeofmap[z++] = 'U';

				typeofmap[z] = '\0';

				lvltable[j][i] = '(';
				i++;

				z = 0;
				while(typeofmap[z] != '\0')
				{
					lvltable[j][i] = typeofmap[z];
					i++;
					z++;
				}
			}

			lvltable[j][i] = ')';
			i++;

			lvltable[j][i] = '\0';
		}
		else
			lvltable[j][0] = '\0';

		if(lvltable[j][0] == '\0')
			map_cons_t[j].strvalue = NULL;
		else
			map_cons_t[j].strvalue = lvltable[j];
	}

	CV_SetValue(&cv_nextmap, cv_nextmap.value);	
}

void M_StartSplitServerMenu()
{
    if( Playing() )
    {
        M_StartMessage(ALLREADYPLAYING,M_ExitGameResponse,MM_YESNO);
        return;
    }
	
	M_PatchLevelNameTable();
    StartSplitScreenGame = true;
    M_SetupNextMenu(&Serverdef);
}

void M_StartServerMenu(int choise)
{
    if( Playing() )
    {
        M_StartMessage(ALLREADYPLAYING,M_ExitGameResponse,MM_YESNO);
        return;
    }
	
	M_PatchLevelNameTable();
    StartSplitScreenGame = (choise != 0);
    M_SetupNextMenu(&Serverdef);
}

//===========================================================================
//                            MULTI PLAYER MENU
//===========================================================================
void M_SetupMultiPlayer (int choice);
void M_SetupMultiPlayerBis (int choice);
void M_Splitscreen(int choise);

enum {
    startserver=0,
    connectmultiplayermenu,
	connectip,
	startsplitscreengame,
	multiplayeroptions,
    setupplayer1,
    setupplayer2,
    end_game,
    multiplayer_end
} multiplayer_e;

menuitem_t MultiPlayerMenu[] =
{
    {IT_CALL | IT_STRING,0,"HOST GAME"    ,M_StartServerMenu     , 10},
    {IT_CALL | IT_STRING,0,"JOIN GAME (Search)",M_ConnectMenu         , 20},
	{IT_CALL | IT_STRING,0,"JOIN GAME (Specify IP)",M_ConnectIPMenu         , 30},
    {IT_CALL | IT_STRING,0,"TWO PLAYER GAME"  ,M_Splitscreen         , 50},
    {IT_CALL | IT_STRING,0,"NETWORK OPTIONS"   ,M_NetOption    , 70},
    {IT_CALL | IT_STRING,0,"SETUP PLAYER"   ,M_SetupMultiPlayer    , 90},
    {IT_CALL | IT_STRING,0,"SETUP PLAYER 2"   ,M_SetupMultiPlayerBis , 100},
    {IT_CALL | IT_STRING,0,"END GAME"         ,M_EndGame             ,120}
};

menu_t  MultiPlayerDef =
{
    "M_MULTI",
    "Multiplayer",
    multiplayer_end,
    &MainDef,
    MultiPlayerMenu,
    M_DrawGenericMenu,
    85,40,
    0
};


void M_Splitscreen(int choise)
{
    M_StartSplitServerMenu(1);
}

//===========================================================================
// Seconde mouse config for the splitscreen player
//===========================================================================

menuitem_t  SecondMouseCfgMenu[] =
{
    {IT_STRING | IT_CVAR,0,"Second Mouse Serial Port",&cv_mouse2port,0}, // Tails 01-18-2001
    {IT_STRING | IT_CVAR,0,"Use Mouse 2"     ,&cv_usemouse2        ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,"Mouse2 Speed"    ,&cv_mousesens2       ,0},
    {IT_STRING | IT_CVAR,0,"Always MouseLook",&cv_alwaysfreelook2  ,0},
    {IT_STRING | IT_CVAR,0,"Mouse Move",      &cv_mousemove2       ,0},
    {IT_STRING | IT_CVAR,0,"Invert Mouse2"   ,&cv_invertmouse2     ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,"Mlook Speed"     ,&cv_mlooksens2       ,0},
};

menu_t  SecondMouseCfgdef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(SecondMouseCfgMenu)/sizeof(menuitem_t),
    &SetupMultiPlayerDef,
    SecondMouseCfgMenu,
    M_DrawGenericMenu,
    27,40,
    0,
};

//===========================================================================
// Second options for the splitscreen player
//===========================================================================

menuitem_t  SecondOptionsMenu[] =
{
    //Hurdler: for now, only autorun is implemented 
    //         others should be implemented as well if we want to be complete
//    {IT_STRING | IT_CVAR,"Messages:"       ,&cv_showmessages2    ,0},
    {IT_STRING | IT_CVAR,0,"Always Run"      ,&cv_skin         ,0},
//    {IT_STRING | IT_CVAR,"Crosshair"       ,&cv_crosshair2       ,0},
//    {IT_STRING | IT_CVAR,"Control per key" ,&cv_controlperkey2   ,0},
};

menu_t  SecondOptionsdef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(SecondOptionsMenu)/sizeof(menuitem_t),
    &SetupMultiPlayerDef,
    SecondOptionsMenu,
    M_DrawGenericMenu,
    27,40,
    0,
};

//===========================================================================
//MULTI PLAYER SETUP MENU
//===========================================================================
void M_DrawSetupMultiPlayerMenu(void);
void M_HandleSetupMultiPlayer(int choice);
void M_Setup1PControlsMenu(int choice);
void M_Setup2PControlsMenu(int choice);
boolean M_QuitMultiPlayerMenu(void);

menuitem_t SetupMultiPlayerMenu[] =
{
    {IT_KEYHANDLER | IT_STRING          ,0,"Your name" ,M_HandleSetupMultiPlayer,0},
    {IT_CVAR | IT_STRING | IT_CV_NOPRINT,0,"Your color",&cv_playercolor         ,16},
    {IT_KEYHANDLER | IT_STRING          ,0,"Your player" ,M_HandleSetupMultiPlayer,96}, // Tails 01-18-2001
    /* this line calls the setup controls for secondary player, only if numitems is > 3 */
    //Hurdler: uncomment this line when other options are available
//    {IT_SUBMENU | IT_WHITESTRING, 0,"Second Player config...", &SecondOptionsdef, 110},
    //... and remove this one
    {IT_CALL | IT_WHITESTRING, 0,"Setup Controls...", M_Setup2PControlsMenu, 120},
    {IT_SUBMENU | IT_WHITESTRING, 0,"Second Mouse config...", &SecondMouseCfgdef, 130}
};

enum {
    setupmultiplayer_name = 0,
    setupmultiplayer_color,
    setupmultiplayer_skin,
    setupmultiplayer_controls,
    setupmultiplayer_mouse2,
    setupmulti_end
};

menu_t  SetupMultiPlayerDef =
{
    "M_MULTI",
    "Multiplayer",
    sizeof(SetupMultiPlayerMenu)/sizeof(menuitem_t),
    &MultiPlayerDef,
    SetupMultiPlayerMenu,
    M_DrawSetupMultiPlayerMenu,
    27,40,
    0,
    M_QuitMultiPlayerMenu
};

// Tails 03-02-2002
void M_DrawSetupChoosePlayerMenu(void);
void M_HandleSetupChoosePlayer(int choice);
boolean M_QuitChoosePlayerMenu(void);
void M_ChoosePlayer(int choice);
int skillnum;
enum
{
    Player1,
    Player2,
    Player3,
	Player4,
	Player5,
	Player6,
	Player7,
	Player8,
    player_end
} players_e;

menuitem_t PlayerMenu[]=
{
    {IT_CALL | IT_STRING,0,"SONIC",M_ChoosePlayer, 20},
    {IT_CALL | IT_STRING,0,"TAILS",M_ChoosePlayer, 40},
	{IT_CALL | IT_STRING,0,"KNUCKLES",M_ChoosePlayer, 60},
	{IT_DISABLED,0,"PLAYER4",M_ChoosePlayer, 60},
	{IT_DISABLED,0,"PLAYER5",M_ChoosePlayer, 60},
	{IT_DISABLED,0,"PLAYER6",M_ChoosePlayer, 60},
	{IT_DISABLED,0,"PLAYER7",M_ChoosePlayer, 60},
	{IT_DISABLED,0,"PLAYER8",M_ChoosePlayer, 60},
};

menu_t  PlayerDef =
{
    "M_MULTI",
    "Choose Your Character",
    sizeof(PlayerMenu)/sizeof(menuitem_t),//player_end,
    &NewDef,
    PlayerMenu,
    M_DrawSetupChoosePlayerMenu,
    24,32,
    0,
    M_QuitChoosePlayerMenu
};
// Tails 03-02-2002


#define PLBOXW    8
#define PLBOXH    9

static  int       multi_tics;
static  state_t*  multi_state;

// this is set before entering the MultiPlayer setup menu,
// for either player 1 or 2
static  char       setupm_name[MAXPLAYERNAME+1];
static  player_t*  setupm_player;
static  consvar_t* setupm_cvskin;
static  consvar_t* setupm_cvcolor;
static  consvar_t* setupm_cvname;

void M_SetupMultiPlayer (int choice)
{
	if(!(gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
	{
		M_StartMessage("You have to be in a game\nto do this.\n\nPress a key\n",NULL,MM_NOTHING);
		return;
	}

    multi_state = &states[mobjinfo[MT_PLAYER].seestate];
    multi_tics = multi_state->tics;
    strcpy(setupm_name, cv_playername.string);

    SetupMultiPlayerDef.numitems = setupmultiplayer_skin +1;      //remove player2 setup controls and mouse2 

    // set for player 1
    SetupMultiPlayerMenu[setupmultiplayer_color].itemaction = &cv_playercolor;
    setupm_player = &players[consoleplayer];
    setupm_cvskin = &cv_skin;
    setupm_cvcolor = &cv_playercolor;
    setupm_cvname = &cv_playername;
    M_SetupNextMenu (&SetupMultiPlayerDef);
}

// start the multiplayer setup menu, for secondary player (splitscreen mode)
void M_SetupMultiPlayerBis (int choice)
{
	if(!(gamestate == GS_LEVEL || gamestate == GS_INTERMISSION))
	{
		M_StartMessage("You have to be in a game\nto do this.\n\nPress a key\n",NULL,MM_NOTHING);
		return;
	}

    multi_state = &states[mobjinfo[MT_PLAYER].seestate];
    multi_tics = multi_state->tics;
    strcpy (setupm_name, cv_playername2.string);
    SetupMultiPlayerDef.numitems = setupmulti_end;          //activate the setup controls for player 2

    // set for splitscreen secondary player
    SetupMultiPlayerMenu[setupmultiplayer_color].itemaction = &cv_playercolor2;
    setupm_player = &players[secondarydisplayplayer];
    setupm_cvskin = &cv_skin2;
    setupm_cvcolor = &cv_playercolor2;
    setupm_cvname = &cv_playername2;
    M_SetupNextMenu (&SetupMultiPlayerDef);
}

// Draw the funky Connect IP menu. Tails 11-19-2002
// So much work for such a little thing!
void M_DrawConnectIPMenu(void)
{
    int             mx,my;

    mx = ConnectIPDef.x;
    my = ConnectIPDef.y;

    // use generic drawer for cursor, items and title
    M_DrawGenericMenu();

    // draw name string
//    M_DrawTextBox(mx+82,my-8,MAXPLAYERNAME,1);
    V_DrawString (mx+128,my+40,0,setupm_ip);

    // draw text cursor for name
    if (itemOn==0 &&
        skullAnimCounter<4)   //blink cursor
        V_DrawCharacter(mx+128+V_StringWidth(setupm_ip),my+40,'_');
}

// called at splitscreen changes
void M_SwitchSplitscreen(void)
{
// activate setup for player 2
    if (cv_splitscreen.value)
        MultiPlayerMenu[setupplayer2].status = IT_CALL | IT_STRING;
    else
        MultiPlayerMenu[setupplayer2].status = IT_DISABLED;

    if( MultiPlayerDef.lastOn==setupplayer2)
        MultiPlayerDef.lastOn=setupplayer1; 
}


//
//  Draw the multi player setup menu, had some fun with player anim
//
void M_DrawSetupMultiPlayerMenu(void)
{
    int             mx,my;
    spritedef_t*    sprdef;
    spriteframe_t*  sprframe;
    int             lump;
    patch_t*        patch;
    int             st;
    byte*           colormap;

    mx = SetupMultiPlayerDef.x;
    my = SetupMultiPlayerDef.y;

    // use generic drawer for cursor, items and title
    M_DrawGenericMenu();

    // draw name string
    M_DrawTextBox(mx+90,my-8,MAXPLAYERNAME,1);
    V_DrawString (mx+98,my,0,setupm_name);

    // draw skin string
    V_DrawString (mx+90, my+96,0, setupm_cvskin->string);

	// draw the name of the color you have chosen Tails 09-08-2002
	// Just so people don't go thinking that "Default" is Green.
	V_DrawString(208, 72, 0, setupm_cvcolor->string);

    // draw text cursor for name
    if (itemOn==0 &&
        skullAnimCounter<4)   //blink cursor
        V_DrawCharacter(mx+98+V_StringWidth(setupm_name),my,'_');

    // anim the player in the box
    if (--multi_tics<=0)
    {
        st = multi_state->nextstate;
        if (st!=S_NULL)
            multi_state = &states[st];
        multi_tics = multi_state->tics;
        if (multi_tics==-1)
            multi_tics=15;
    }

    // skin 0 is default player sprite
    sprdef    = &skins[R_SkinAvailable(setupm_cvskin->string)].spritedef;
    sprframe  = &sprdef->spriteframes[ multi_state->frame & FF_FRAMEMASK];
    lump  = sprframe->lumppat[0];
    patch = W_CachePatchNum (lump, PU_CACHE);

    // draw box around guy
    M_DrawTextBox(mx+90,my+8, PLBOXW, PLBOXH);

    if (setupm_cvcolor->value==0)
        colormap = colormaps;
    else
	{
			colormap = (byte *) translationtables[setupm_player->skin] - 256 + (setupm_cvcolor->value<<8); // Tails 06-07-2002
	}

    // draw player sprite
    V_DrawMappedPatch (mx+98+(PLBOXW*8/2),my+16+(PLBOXH*8)-8,0,patch,colormap);
}

// Tails 11-19-2002
void M_HandleConnectIP (int choice)
{
    int      l;
    boolean  exitmenu = false;  // exit to previous menu and send name change

    switch( choice )
    {
      case KEY_ENTER:
        S_StartSound(NULL,sfx_menu1); // Tails
        M_ClearMenus (true);
		M_ConnectIP(1);
        break;

      case KEY_ESCAPE:
        exitmenu = true;
        break;

      case KEY_BACKSPACE:
        if ( (l=strlen(setupm_ip))!=0 && itemOn==0)
        {
            S_StartSound(NULL,sfx_menu1); // Tails
            setupm_ip[l-1]=0;
        }
        break;

      default:
        l = strlen(setupm_ip);
        if (l<16-1 && (choice == 46 || (choice >= 48 && choice <= 57))) // Rudimentary number and period enforcing
        {
            S_StartSound(NULL,sfx_menu1); // Tails
            setupm_ip[l]=choice;
            setupm_ip[l+1]=0;
        }
        break;
    }

    if (exitmenu)
    {
        if (currentMenu->prevMenu)
            M_SetupNextMenu (currentMenu->prevMenu);
        else
            M_ClearMenus (true);
    }
}

//
// Handle Setup MultiPlayer Menu
//
void M_HandleSetupMultiPlayer (int choice)
{
    int      l;
    boolean  exitmenu = false;  // exit to previous menu and send name change
    int      myskin;

    myskin  = setupm_cvskin->value;

    switch( choice )
    {
      case KEY_DOWNARROW:
        S_StartSound(NULL,sfx_menu1); // Tails
        if (itemOn+1 >= SetupMultiPlayerDef.numitems)
            itemOn = 0;
        else itemOn++;
        break;

      case KEY_UPARROW:
        S_StartSound(NULL,sfx_menu1); // Tails
        if (!itemOn)
            itemOn = SetupMultiPlayerDef.numitems-1;
        else itemOn--;
        break;

      case KEY_LEFTARROW:
        if (itemOn==2)       //player skin
        {
            S_StartSound(NULL,sfx_menu1); // Tails
            myskin--;
        }
        break;

      case KEY_RIGHTARROW:
        if (itemOn==2)       //player skin
        {
            S_StartSound(NULL,sfx_menu1); // Tails
            myskin++;
        }
        break;

      case KEY_ENTER:
        S_StartSound(NULL,sfx_menu1); // Tails
        exitmenu = true;
        break;

      case KEY_ESCAPE:
        exitmenu = true;
        break;

      case KEY_BACKSPACE:
        if ( (l=strlen(setupm_name))!=0 && itemOn==0)
        {
            S_StartSound(NULL,sfx_menu1); // Tails
            setupm_name[l-1]=0;
        }
        break;

      default:
        if (choice < 32 || choice > 127 || itemOn!=0)
            break;
        l = strlen(setupm_name);
        if (l<MAXPLAYERNAME-1)
        {
            S_StartSound(NULL,sfx_menu1); // Tails
            setupm_name[l]=choice;
            setupm_name[l+1]=0;
        }
        break;
    }

    // check skin
    if (myskin <0)
        myskin = numskins-1;
    if (myskin >numskins-1)
        myskin = 0;

    // check skin change
    if (myskin != setupm_player->skin)
        COM_BufAddText ( va("%s \"%s\"",setupm_cvskin->name ,skins[myskin].name));

    if (exitmenu)
    {
        if (currentMenu->prevMenu)
            M_SetupNextMenu (currentMenu->prevMenu);
        else
            M_ClearMenus (true);
    }
}

boolean M_QuitMultiPlayerMenu(void)
{
    int      l;
    // send name if changed
    if (strcmp(setupm_name, setupm_cvname->string))
    {
        // remove trailing whitespaces
        for (l= strlen(setupm_name)-1;
             l>=0 && setupm_name[l]==' '; l--)
            setupm_name[l]=0;
        COM_BufAddText ( va("%s \"%s\"",setupm_cvname->name ,setupm_name));
        
    }
    return true;
}


////////////////////////////////////////////////////////////////
//                   CHARACTER SELECT SCREEN                  //
////////////////////////////////////////////////////////////////

void M_SetupChoosePlayer (int choice)
{
	if(Playing() == false)
	{
		S_StopMusic();
		S_ChangeMusic(mus_chrsel, true);
	}

    M_SetupNextMenu (&PlayerDef);
}

//
//  Draw the choose player setup menu, had some fun with player anim
//
void M_DrawSetupChoosePlayerMenu(void)
{
    int             mx,my;
	patch_t*        patch;

    mx = PlayerDef.x;
    my = PlayerDef.y;

	// Black BG
	V_DrawFill(0, 0, vid.width, vid.height, 0);

    // use generic drawer for cursor, items and title
    M_DrawGenericMenu();

    // TEXT BOX!
	// For the character
    M_DrawTextBox(mx+152,my-8, 16, 16);

	// For description
    M_DrawTextBox(mx-24, my+72, 20, 10);

	patch = W_CachePatchName(description[itemOn].picname, PU_CACHE);

	V_DrawString(mx-16, my+80, V_WHITEMAP, "Speed:\nAbility:\nNotes:");

	V_DrawScaledPatch(mx+160,my,0,patch);
	V_DrawString(mx-16, my+80, 0, description[itemOn].info);
}

//
// Handle Setup Choose Player Menu
//
void M_HandleSetupChoosePlayer (int choice)
{
    boolean  exitmenu = false;  // exit to previous menu and send name change

    switch( choice )
    {
      case KEY_DOWNARROW:
        S_StartSound(NULL,sfx_menu1); // Tails
        if (itemOn+1 >= SetupMultiPlayerDef.numitems)
            itemOn = 0;
        else itemOn++;
        break;

      case KEY_UPARROW:
        S_StartSound(NULL,sfx_menu1); // Tails
        if (!itemOn)
            itemOn = SetupMultiPlayerDef.numitems-1;
        else itemOn--;
        break;

      case KEY_ENTER:
        S_StartSound(NULL,sfx_menu1); // Tails
        exitmenu = true;
        break;

      case KEY_ESCAPE:
        exitmenu = true;
        break;

      default:
        break;
    }

    if (exitmenu)
    {
        if (currentMenu->prevMenu)
            M_SetupNextMenu (currentMenu->prevMenu);
        else
            M_ClearMenus (true);
    }
}

boolean M_QuitChoosePlayerMenu(void)
{
	// Stop music
	S_StopMusic();
    return true;
}


//===========================================================================
//                           NEW GAME FOR SINGLE PLAYER
//===========================================================================
void M_DrawNewGame(void);
// overhaul! Tails 11-30-2000
void M_ChooseSkill(int choice);

enum
{
    easy,
    normal,
    hard,
    veryhard,
    ultimate,
    newg_end
} newgame_e;

menuitem_t NewGameMenu[]=
{
	// Tails
    {IT_CALL | IT_STRING,0,"Easy"     ,M_ChooseSkill,  90},
    {IT_CALL | IT_STRING,0,"Normal"   ,M_ChooseSkill, 100},
    {IT_CALL | IT_STRING,0,"Hard"     ,M_ChooseSkill, 110},
    {IT_CALL | IT_STRING,0,"Very Hard"   ,M_ChooseSkill, 120},
	{IT_CALL | IT_STRING,0,"Ultimate" ,M_ChooseSkill, 130},
};

menu_t  NewDef =
{
    "M_NEWG",
    "NEW GAME",
    newg_end,           // # of menu items
    &EpiDef,            // previous menu
    NewGameMenu,        // menuitem_t ->
    M_DrawNewGame,      // drawing routine ->
    48,63,              // x,y
    normal            // lastOn // Tails
};

void M_DrawNewGame(void)
{
//    patch_t* p;

    //faB: testing with glide
//    p = W_CachePatchName("M_SKILL",PU_CACHE);
//    V_DrawScaledPatch ((BASEVIDWIDTH-p->width)/2,38,0,p);

    M_DrawGenericMenu();
}

void M_Statistics(int choice)
{
	M_SetupNextMenu(&StatsDef);
}

void M_Stats2(int choice)
{
	oldlastmapnum = lastmapnum;
	M_SetupNextMenu(&Stats2Def);
}

void M_Stats3(int choice)
{
	oldlastmapnum = lastmapnum;
	M_SetupNextMenu(&Stats3Def);
}

void M_Stats4(int choice)
{
	oldlastmapnum = lastmapnum;
	M_SetupNextMenu(&Stats4Def);
}

void M_Stats5(int choice)
{
	oldlastmapnum = lastmapnum;
	M_SetupNextMenu(&Stats5Def);
}

void M_DrawStats()
{
	int found = 0;
	char hours[3];
	char minutes[3];
	char seconds[3];

	if(gottenemblems & 1)
		found++;
	if(gottenemblems & 2)
		found++;
	if(gottenemblems & 4)
		found++;
	if(gottenemblems & 8)
		found++;
	if(gottenemblems & 16)
		found++;
	if(gottenemblems & 32)
		found++;
	if(gottenemblems & 64)
		found++;
	if(gottenemblems & 128)
		found++;
	if(gottenemblems & 256)
		found++;
	if(gottenemblems & 512)
		found++;
	if(gottenemblems & 1024)
		found++;
	if(gottenemblems & 2048)
		found++;
	if(gottenemblems & 4096)
		found++;
	if(gottenemblems & 8192)
		found++;
	if(gottenemblems & 16384)
		found++;
	if(gottenemblems & 32768)
		found++;
	if(gottenemblems & 65536)
		found++;
	if(gottenemblems & 131072)
		found++;
	if(gottenemblems & 262144)
		found++;
	if(gottenemblems & 524288)
		found++;

	V_DrawString(48, 32, 0, va("x %i/%i", found, NUMEMBLEMS));
	V_DrawScaledPatch(16, 32-4, 0, W_CachePatchName("EMBLICON", PU_STATIC));

	V_DrawCenteredString(256, 32, V_WHITEMAP, "Total Play Time:");

	if(totalplaytime/(3600*TICRATE) < 10)
		sprintf(hours, "0%i", totalplaytime/(3600*TICRATE));
	else
		sprintf(hours, "%i:", totalplaytime/(3600*TICRATE));

	if(totalplaytime/(60*TICRATE)%60 < 10)
		sprintf(minutes, "0%i", totalplaytime/(60*TICRATE)%60);
	else
		sprintf(minutes, "%i", totalplaytime/(60*TICRATE)%60);

	if(((totalplaytime/TICRATE) % 60) < 10)
		sprintf(seconds, "0%i", (totalplaytime/TICRATE) % 60);
	else
		sprintf(seconds, "%i", (totalplaytime/TICRATE) % 60);

	V_DrawCenteredString(256, 32, 0, "Total Play Time:");
	V_DrawCenteredString(256, 48, 0, va("%s:%s:%s", hours, minutes,seconds));

	if(eastermode)
	{
		found = 0;

		if(foundeggs & 1)
			found++;
		if(foundeggs & 2)
			found++;
		if(foundeggs & 4)
			found++;
		if(foundeggs & 8)
			found++;
		if(foundeggs & 16)
			found++;
		if(foundeggs & 32)
			found++;
		if(foundeggs & 64)
			found++;
		if(foundeggs & 128)
			found++;
		if(foundeggs & 256)
			found++;
		if(foundeggs & 512)
			found++;
		if(foundeggs & 1024)
			found++;
		if(foundeggs & 2048)
			found++;

		V_DrawCenteredString(BASEVIDWIDTH/2, 16, V_WHITEMAP, va("Eggs Found: %i/%i", found, NUMEGGS));
	}

	{
		int i;
		int y = 80;

		V_DrawString(32+36, y-16, 0, "LEVEL NAME");
		V_DrawString(224+28, y-16, 0, "BEST TIME");

		lastmapnum = 0;
		oldlastmapnum = 0;

		V_DrawString(32, y-16, 0, "S T K");

		for(i=oldlastmapnum; i<NUMMAPS; i++)
		{

			if(mapheaderinfo[i].lvlttl[0] == '\0')
				continue;

			if(!(mapheaderinfo[i].typeoflevel & TOL_COOP)
				&& !(mapheaderinfo[i].typeoflevel & TOL_NIGHTS))
				continue;

			lastmapnum = i;

			switch(i)
			{
				case 0:
					if(gottenemblems & 1)
						V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 2)
						V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 4)
						V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					break;
				case 1:
					if(gottenemblems & 8)
						V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 16)
						V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 32)
						V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					break;
				case 3:
					if(gottenemblems & 64)
						V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 128)
						V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 256)
						V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					break;
				case 4:
					if(gottenemblems & 512)
						V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 1024)
						V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 2048)
						V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					break;
				case 6:
					if(gottenemblems & 4096)
						V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 8192)
						V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 16384)
						V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					break;
				case 7:
					if(gottenemblems & 32768)
						V_DrawScaledPatch(30, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 65536)
						V_DrawScaledPatch(42, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					if(gottenemblems & 131072)
						V_DrawScaledPatch(54, y, 0, W_CachePatchName("GOTIT", PU_CACHE));
					break;
				default:
					break;
			}

			if(mapheaderinfo[i].actnum != 0)
				V_DrawString(32+36, y, V_WHITEMAP, va("%s %d", mapheaderinfo[i].lvlttl, mapheaderinfo[i].actnum));
			else
				V_DrawString(32+36, y, V_WHITEMAP, mapheaderinfo[i].lvlttl);

			if(timedata[i].time/(60*TICRATE) < 10)
				sprintf(minutes, "0%i", timedata[i].time/(60*TICRATE));
			else
				sprintf(minutes, "%i", timedata[i].time/(60*TICRATE));

			if(((timedata[i].time/TICRATE) % 60) < 10)
				sprintf(seconds, "0%i", (timedata[i].time/TICRATE) % 60);
			else
				sprintf(seconds, "%i", (timedata[i].time/TICRATE) % 60);

			if((timedata[i].time % TICRATE) < 10)
				sprintf(hours, "0%i", (timedata[i].time % TICRATE));
			else
				sprintf(hours, "%i", (timedata[i].time % TICRATE));

			V_DrawString(224+28, y, 0, va("%s:%s:%s", minutes,seconds,hours));

			y += 8;

			if(y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

void M_DrawStats2()
{
	char hours[3];
	char minutes[3];
	char seconds[3];

	{
		int i;
		int y = 16;

		V_DrawCenteredString(BASEVIDWIDTH/2, y-16, V_WHITEMAP, "BEST TIMES");
		V_DrawCenteredString(BASEVIDWIDTH/2, y-8, 0, "Page 2");

		for(i=oldlastmapnum+1; i<NUMMAPS; i++)
		{
			if(mapheaderinfo[i].lvlttl[0] == '\0')
				continue;

			if(!(mapheaderinfo[i].typeoflevel & TOL_COOP)
				&& !(mapheaderinfo[i].typeoflevel & TOL_NIGHTS))
				continue;

			lastmapnum = i;

			if(mapheaderinfo[i].actnum != 0)
				V_DrawString(32, y, V_WHITEMAP, va("%s %d", mapheaderinfo[i].lvlttl, mapheaderinfo[i].actnum));
			else
				V_DrawString(32, y, V_WHITEMAP, mapheaderinfo[i].lvlttl);

			if(timedata[i].time/(60*TICRATE) < 10)
				sprintf(minutes, "0%i", timedata[i].time/(60*TICRATE));
			else
				sprintf(minutes, "%i", timedata[i].time/(60*TICRATE));

			if(((timedata[i].time/TICRATE) % 60) < 10)
				sprintf(seconds, "0%i", (timedata[i].time/TICRATE) % 60);
			else
				sprintf(seconds, "%i", (timedata[i].time/TICRATE) % 60);

			if((timedata[i].time % TICRATE) < 10)
				sprintf(hours, "0%i", (timedata[i].time % TICRATE));
			else
				sprintf(hours, "%i", (timedata[i].time % TICRATE));

			V_DrawString(224, y, 0, va("%s:%s:%s", minutes,seconds,hours));

			y += 8;

			if(y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

void M_DrawStats3()
{
	char hours[3];
	char minutes[3];
	char seconds[3];

	{
		int i;
		int y = 16;

		V_DrawCenteredString(BASEVIDWIDTH/2, y-16, V_WHITEMAP, "BEST TIMES");
		V_DrawCenteredString(BASEVIDWIDTH/2, y-8, 0, "Page 3");

		for(i=oldlastmapnum+1; i<NUMMAPS; i++)
		{
			if(mapheaderinfo[i].lvlttl[0] == '\0')
				continue;

			if(!(mapheaderinfo[i].typeoflevel & TOL_COOP)
				&& !(mapheaderinfo[i].typeoflevel & TOL_NIGHTS))
				continue;

			lastmapnum = i;

			if(mapheaderinfo[i].actnum != 0)
				V_DrawString(32, y, V_WHITEMAP, va("%s %d", mapheaderinfo[i].lvlttl, mapheaderinfo[i].actnum));
			else
				V_DrawString(32, y, V_WHITEMAP, mapheaderinfo[i].lvlttl);

			if(timedata[i].time/(60*TICRATE) < 10)
				sprintf(minutes, "0%i", timedata[i].time/(60*TICRATE));
			else
				sprintf(minutes, "%i", timedata[i].time/(60*TICRATE));

			if(((timedata[i].time/TICRATE) % 60) < 10)
				sprintf(seconds, "0%i", (timedata[i].time/TICRATE) % 60);
			else
				sprintf(seconds, "%i", (timedata[i].time/TICRATE) % 60);

			if((timedata[i].time % TICRATE) < 10)
				sprintf(hours, "0%i", (timedata[i].time % TICRATE));
			else
				sprintf(hours, "%i", (timedata[i].time % TICRATE));

			V_DrawString(224, y, 0, va("%s:%s:%s", minutes,seconds,hours));

			y += 8;

			if(y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

void M_DrawStats4()
{
	char hours[3];
	char minutes[3];
	char seconds[3];

	{
		int i;
		int y = 16;

		V_DrawCenteredString(BASEVIDWIDTH/2, y-16, V_WHITEMAP, "BEST TIMES");
		V_DrawCenteredString(BASEVIDWIDTH/2, y-8, 0, "Page 4");

		for(i=oldlastmapnum+1; i<NUMMAPS; i++)
		{
			if(mapheaderinfo[i].lvlttl[0] == '\0')
				continue;

			if(!(mapheaderinfo[i].typeoflevel & TOL_COOP)
				&& !(mapheaderinfo[i].typeoflevel & TOL_NIGHTS))
				continue;

			lastmapnum = i;

			if(mapheaderinfo[i].actnum != 0)
				V_DrawString(32, y, V_WHITEMAP, va("%s %d", mapheaderinfo[i].lvlttl, mapheaderinfo[i].actnum));
			else
				V_DrawString(32, y, V_WHITEMAP, mapheaderinfo[i].lvlttl);

			if(timedata[i].time/(60*TICRATE) < 10)
				sprintf(minutes, "0%i", timedata[i].time/(60*TICRATE));
			else
				sprintf(minutes, "%i", timedata[i].time/(60*TICRATE));

			if(((timedata[i].time/TICRATE) % 60) < 10)
				sprintf(seconds, "0%i", (timedata[i].time/TICRATE) % 60);
			else
				sprintf(seconds, "%i", (timedata[i].time/TICRATE) % 60);

			if((timedata[i].time % TICRATE) < 10)
				sprintf(hours, "0%i", (timedata[i].time % TICRATE));
			else
				sprintf(hours, "%i", (timedata[i].time % TICRATE));

			V_DrawString(224, y, 0, va("%s:%s:%s", minutes,seconds,hours));

			y += 8;

			if(y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

void M_DrawStats5()
{
	char hours[3];
	char minutes[3];
	char seconds[3];

	{
		int i;
		int y = 16;

		V_DrawCenteredString(BASEVIDWIDTH/2, y-16, V_WHITEMAP, "BEST TIMES");
		V_DrawCenteredString(BASEVIDWIDTH/2, y-8, 0, "Page 5");

		for(i=oldlastmapnum+1; i<NUMMAPS; i++)
		{
			if(mapheaderinfo[i].lvlttl[0] == '\0')
				continue;

			if(!(mapheaderinfo[i].typeoflevel & TOL_COOP)
				&& !(mapheaderinfo[i].typeoflevel & TOL_NIGHTS))
				continue;

			lastmapnum = i;

			if(mapheaderinfo[i].actnum != 0)
				V_DrawString(32, y, V_WHITEMAP, va("%s %d", mapheaderinfo[i].lvlttl, mapheaderinfo[i].actnum));
			else
				V_DrawString(32, y, V_WHITEMAP, mapheaderinfo[i].lvlttl);

			if(timedata[i].time/(60*TICRATE) < 10)
				sprintf(minutes, "0%i", timedata[i].time/(60*TICRATE));
			else
				sprintf(minutes, "%i", timedata[i].time/(60*TICRATE));

			if(((timedata[i].time/TICRATE) % 60) < 10)
				sprintf(seconds, "0%i", (timedata[i].time/TICRATE) % 60);
			else
				sprintf(seconds, "%i", (timedata[i].time/TICRATE) % 60);

			if((timedata[i].time % TICRATE) < 10)
				sprintf(hours, "0%i", (timedata[i].time % TICRATE));
			else
				sprintf(hours, "%i", (timedata[i].time % TICRATE));

			V_DrawString(224, y, 0, va("%s:%s:%s", minutes,seconds,hours));

			y += 8;

			if(y >= BASEVIDHEIGHT-8)
				return;
		}
	}
}

void M_NewGame(int choice)
{
    if (netgame)
    {
        M_StartMessage(NEWGAME,M_ExitGameResponse,MM_YESNO);
        return;
    }

	if(veryhardcleared) // Ultimate skill level is unlockable Tails 05-19-2003
		NewGameMenu[ultimate].status = IT_STRING | IT_CALL;
	else
		NewGameMenu[ultimate].status = IT_DISABLED;

	startmap = spstage_start;

    M_SetupNextMenu(&NewDef);

    StartSplitScreenGame=false;
}

void M_AdventureGame(int choice)
{
    if (netgame)
    {
        M_StartMessage(NEWGAME,M_ExitGameResponse,MM_YESNO);
        return;
    }

	if(veryhardcleared) // Ultimate skill level is unlockable Tails 05-19-2003
		NewGameMenu[ultimate].status = IT_STRING | IT_CALL;
	else
		NewGameMenu[ultimate].status = IT_DISABLED;

	startmap = 42;

    M_SetupNextMenu(&NewDef);

    StartSplitScreenGame=false;
}

void M_ChristmasGame(int choice)
{
    if (netgame)
    {
        M_StartMessage(NEWGAME,M_ExitGameResponse,MM_YESNO);
        return;
    }

	if(veryhardcleared) // Ultimate skill level is unlockable Tails 05-19-2003
		NewGameMenu[ultimate].status = IT_STRING | IT_CALL;
	else
		NewGameMenu[ultimate].status = IT_DISABLED;

	startmap = 40;

    M_SetupNextMenu(&NewDef);

    StartSplitScreenGame=false;
}

void M_NightsGame(int choice)
{
    if (netgame)
    {
        M_StartMessage(NEWGAME,M_ExitGameResponse,MM_YESNO);
        return;
    }

	if(veryhardcleared) // Ultimate skill level is unlockable Tails 05-19-2003
		NewGameMenu[ultimate].status = IT_STRING | IT_CALL;
	else
		NewGameMenu[ultimate].status = IT_DISABLED;

	startmap = 29;

    M_SetupNextMenu(&NewDef);

    StartSplitScreenGame=false;
}

void M_MarioGame(int choice)
{
    if (netgame)
    {
        M_StartMessage(NEWGAME,M_ExitGameResponse,MM_YESNO);
        return;
    }

	if(veryhardcleared) // Ultimate skill level is unlockable Tails 05-19-2003
		NewGameMenu[ultimate].status = IT_STRING | IT_CALL;
	else
		NewGameMenu[ultimate].status = IT_DISABLED;

	startmap = 30;

    M_SetupNextMenu(&NewDef);

    StartSplitScreenGame=false;
}

// Graue 12-13-2003
void M_GolfGame(int choice)
{
    if (netgame)
    {
        M_StartMessage(NEWGAME,M_ExitGameResponse,MM_YESNO);
        return;
    }

	// Ultimate skill level is already unlocked, duh!
	NewGameMenu[ultimate].status = IT_STRING | IT_CALL;

	startmap = 152; // MAPC0

    M_SetupNextMenu(&NewDef);

    StartSplitScreenGame=false;
}

// Chosen the player you want to use Tails 03-02-2002
void M_ChoosePlayer(int choice)
{
	G_DeferedInitNew(skillnum, G_BuildMapName(startmap), choice, StartSplitScreenGame);
    M_ClearMenus (true);
}

void M_ChooseSkill(int choice)
{
	skillnum = choice+1; // Tails 03-02-2002
	M_SetupChoosePlayer(0);
}

void M_EraseData(int choice);
void M_TimeDataResponse(int ch);
void M_SecretsDataResponse(int ch);

// Tails 08-11-2002
//===========================================================================
//                        Data OPTIONS MENU
//===========================================================================

menuitem_t DataOptionsMenu[]=
{
    {IT_STRING | IT_CALL,0,"Erase Time Attack Data", M_EraseData,0},
    {IT_STRING | IT_CALL,0,"Erase Secrets Data", M_EraseData,0},
};

menu_t  DataOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(DataOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    DataOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

void M_TimeDataResponse(int ch)
{
	int i;
    if (ch != 'y')
        return;

	// Delete the data
	for(i=0; i<NUMMAPS; i++)
		timedata[i].time = 0;

    M_SetupNextMenu(&DataOptionsDef);
    M_Refresh(0);
}
void M_SecretsDataResponse(int ch)
{
    if (ch != 'y')
        return;

	// Delete the data
	gottenemblems = 0;
	foundeggs = 0;
	grade = 0;
	veryhardcleared = 0;

    M_SetupNextMenu(&DataOptionsDef);
    M_Refresh(0);
}

void M_EraseData(int choice)
{
    if( Playing() )
    {
        M_StartMessage("A game cannot be running.\nEnd it first.",NULL,MM_NOTHING);
        return;
    }
	
	else if(choice == 0)
		M_StartMessage( "Are you sure you want to delete\nthe time attack data?\n(Y/N)\n",M_TimeDataResponse,MM_YESNO);
	else // 1
		M_StartMessage( "Are you sure you want to delete\nthe secrets data?\n(Y/N)\n",M_SecretsDataResponse,MM_YESNO);
}

extern consvar_t cv_chasecam;
extern consvar_t cv_chasecam2;

menuitem_t ControlsMenu[]=
{
    {IT_CALL    | IT_STRING,0,"Player 1 Controls...",M_Setup1PControlsMenu,0},
    {IT_CALL    | IT_STRING,0,"Player 2 Controls...",M_Setup2PControlsMenu,10},

    {IT_STRING | IT_CVAR,0,"Analog Control(1P Only)" ,&cv_analog   ,30}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CVAR,0,"Analog Control(2P Only)" ,&cv_analog2   ,40}, // Changed all to normal string Tails 11-30-2000

	{IT_STRING | IT_CVAR,0,"Camera (1P)" ,&cv_chasecam   ,60}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING | IT_CVAR,0,"Autoaim (1P)" ,&cv_autoaim   ,70}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING | IT_CVAR,0,"Crosshair (1P)" ,&cv_crosshair   ,80}, // Changed all to normal string Tails 11-30-2000

	{IT_STRING | IT_CVAR,0,"Camera (2P)" ,&cv_chasecam2   ,100}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING | IT_CVAR,0,"Autoaim (2P)" ,&cv_autoaim2   ,110}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING | IT_CVAR,0,"Crosshair (2P)" ,&cv_crosshair2   ,120}, // Changed all to normal string Tails 11-30-2000

	{IT_STRING | IT_CVAR,0,"Control per key" ,&cv_controlperkey   ,140}, // Changed all to normal string Tails 11-30-2000
};

menu_t  ControlsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(ControlsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    ControlsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                             OPTIONS MENU
//===========================================================================
//
// M_Options
//

//added:10-02-98: note: alphaKey member is the y offset
menuitem_t OptionsMenu[]=
{
    {IT_SUBMENU | IT_STRING,0,"Setup Controls...",&ControlsDef,20},
    {IT_CALL   | IT_STRING,0,"Game Options..."  ,M_GameOption,40},
    {IT_SUBMENU | IT_STRING,0,"Server options...",&ServerOptionsDef  ,50},
    {IT_SUBMENU | IT_STRING,0,"Sound Volume..."  ,&SoundDef  ,60},
    {IT_SUBMENU | IT_STRING,0,"Video Options..." ,&VideoOptionsDef,70},
    {IT_SUBMENU | IT_STRING,0,"Mouse Options..." ,&MouseOptionsDef,80},
    {IT_SUBMENU | IT_STRING,0,"Data Options..." ,&DataOptionsDef,90},
};

menu_t  OptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OptionsMenu)/sizeof(menuitem_t),
    &MainDef,
    OptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

// Tails 08-18-2002
void M_OptionsMenu(int choice)
{
	M_SetupNextMenu (&OptionsDef);
}

void M_UltimateCheat(int choice)
{
	I_Quit ();
}

void M_DestroyRobotsResponse(int ch)
{
    if (ch != 'y')
        return;

	// Destroy all robots
	P_DestroyRobots();

    M_ClearMenus (true);
}

void M_DestroyRobots(int choice)
{
    if( !(Playing() && gamestate == GS_LEVEL) )
    {
        M_StartMessage("You need to be playing and in\na level to do this!",NULL,MM_NOTHING);
        return;
    }

	if(multiplayer || netgame)
	{
        M_StartMessage("You can't do this in\na network game!",NULL,MM_NOTHING);
        return;
	}

	M_StartMessage("Do you want to destroy all\nrobots in the current level?\n(Y/N)\n",M_DestroyRobotsResponse,MM_YESNO);
}

void M_LevelSelect (int choice)
{
	M_SetupNextMenu (&LevelSelectDef);
}

void M_LevelSelectWarp (int choice)
{
    if (netgame)
    {
        M_StartMessage(NEWGAME,M_ExitGameResponse,MM_YESNO);
        return;
    }

	if(veryhardcleared) // Ultimate skill level is unlockable Tails 05-19-2003
		NewGameMenu[ultimate].status = IT_STRING | IT_CALL;
	else
		NewGameMenu[ultimate].status = IT_DISABLED;

	startmap = choice+1;

    M_SetupNextMenu(&NewDef);

    StartSplitScreenGame=false;
}

typedef struct
{
	char* name;
	char* requirement;
	boolean unlocked;
} checklist_t;

// Tails 12-19-2003
void M_DrawUnlockChecklist(void)
{
#define NUMCHECKLIST 7
	checklist_t checklist[NUMCHECKLIST];
	int i;

	checklist[0].name = "Level Select";
	checklist[0].requirement = "Finish 1P";
	checklist[0].unlocked = (grade & 128);

	checklist[1].name = "Adventure Example";
	checklist[1].requirement = "Finish NiGHTS";
	checklist[1].unlocked = (grade & 64);

	checklist[2].name = "Mario Koopa Blast";
	checklist[2].requirement = "Finish 1P\nw/ Emeralds";
	checklist[2].unlocked = (grade & 8);

	checklist[3].name = "Sonic Into Dreams";
	checklist[3].requirement = "Find All Emblems";
	checklist[3].unlocked = (grade & 16);

	checklist[4].name = "Christmas Hunt";
	checklist[4].requirement = "Finish Mario";
	checklist[4].unlocked = (grade & 32);

	// It's not ready yet. Graue 12-31-2003
	//checklist[5].name = "Sonic Golf";
	//checklist[5].requirement = "Finish\nUltimate Mode";
	//checklist[5].unlocked = (grade & 1024);

	checklist[5].name = "Time Attack Bonus";
	checklist[5].requirement = "Finish 1P\nin 6 minutes";
	checklist[5].unlocked = (grade & 256);

	checklist[6].name = "Easter Egg Bonus";
	checklist[6].requirement = "Find all\nEaster Eggs";
	checklist[6].unlocked = (grade & 512);

	for(i=0; i<NUMCHECKLIST; i++)
	{
		V_DrawString(8, 8+(24*i), V_RETURN8, checklist[i].name);
		V_DrawString(160, 8+(24*i), V_RETURN8, checklist[i].requirement);

		if(checklist[i].unlocked)
			V_DrawString(308, 8+(24*i), V_WHITEMAP, "Y");
		else
			V_DrawString(308, 8+(24*i), V_WHITEMAP, "N");
	}
}

void M_DrawTimeAttackBonus(void)
{
	V_DrawSmallScaledPatch(0, 0, 0, W_CachePatchName("CONCEPT1", PU_CACHE), defaulttranslationtables);
}

void M_DrawEasterEggBonus(void)
{
	V_DrawSmallScaledPatch(0, 0, 0, W_CachePatchName("CONCEPT2", PU_CACHE), defaulttranslationtables);
}

// Empty thingy for checklist menu
enum
{
    unlockchecklistempty1,
    unlockchecklist_end
} timeattackbonus_e;

menuitem_t UnlockChecklistMenu[] =
{
    {IT_CALL | IT_STRING,0,"NEXT",M_SecretsMenu, 192}
};

menu_t  UnlockChecklistDef =
{
    NULL,
    NULL,
    unlockchecklist_end,
    &SecretsDef,
    UnlockChecklistMenu,
    M_DrawUnlockChecklist,
    280,185,
    0
};

// Empty thingy for bonus menu
enum
{
    timeattackbonusempty1,
    timeattackbonus_end
} timeattackbonus_e;

menuitem_t TimeAttackBonusMenu[] =
{
    {IT_CALL | IT_STRING,0,"NEXT",M_SecretsMenu, 192}
};

menu_t  TimeAttackBonusDef =
{
    NULL,
    NULL,
    timeattackbonus_end,
    &MainDef,
    TimeAttackBonusMenu,
    M_DrawTimeAttackBonus,
    280,185,
    0
};

// Empty thingy for bonus menu
enum
{
    eastereggbonusempty1,
    eastereggbonus_end
} easteregg_e;

menuitem_t EasterEggBonusMenu[] =
{
    {IT_CALL | IT_STRING,0,"NEXT",M_SecretsMenu, 192}
};

menu_t  EasterEggBonusDef =
{
    NULL,
    NULL,
    eastereggbonus_end,
    &MainDef,
    EasterEggBonusMenu,
    M_DrawEasterEggBonus,
    280,185,
    0
};

void M_UnlockChecklist (int choice)
{
	M_SetupNextMenu(&UnlockChecklistDef);
}

void M_TimeAttackBonus (int choice)
{
	M_SetupNextMenu(&TimeAttackBonusDef);
}

void M_EasterEggBonus (int choice)
{
	M_SetupNextMenu(&EasterEggBonusDef);
}

void M_BetaShowcase (int choice)
{}

//===========================================================================
//                             ??? MENU
//===========================================================================
//
// M_Options
//
void M_Reward(int choice);
enum {
	unlockchecklist=0,
    ultimatecheat,
    secretsgravity,
    playerspeed,
	ringslinger,
	levelselect,
	rings,
	lives,
	continues,
	norobots,
	betashowcase,
	reward,
	timeattackbonus,
	eastereggbonus,
    secrets_end
} secrets_e;

//added:10-02-98: note: alphaKey member is the y offset
menuitem_t SecretsMenu[]=
{
    {IT_STRING | IT_CALL,0,"Secrets Checklist" ,M_UnlockChecklist   ,0}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CALL,0,"Ultimate Cheat" ,M_UltimateCheat   ,20}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CVAR,0,"Gravity" ,&cv_gravity   ,30}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CVAR,0,"Player Speed" ,&cv_playerspeed   ,40}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CVAR,0,"Throw Rings" ,&cv_ringslinger   ,50}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_DISABLED | IT_CALL,0,"Level Select" ,M_LevelSelect   ,60}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CVAR,0,"Modify Rings" ,&cv_startrings   ,70}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CVAR,0,"Modify Lives" ,&cv_startlives   ,80}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CVAR,0,"Modify Continues" ,&cv_startcontinues   ,90}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CALL,0,"Destroy All Robots" ,M_DestroyRobots   ,100}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_DISABLED| IT_CALL,0,"Beta Showcase" , M_BetaShowcase , 110}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CALL,0,"Bonus Levels" , M_Reward   , 120}, // Changed all to normal string Tails 11-30-2000
	{IT_STRING | IT_CALL,0,"Time Attack Bonus", M_TimeAttackBonus, 130},
	{IT_STRING | IT_CALL,0,"Easter Egg Bonus", M_EasterEggBonus, 140},
};

menu_t  SecretsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    secrets_end,
    &MainDef,
    SecretsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                             Reward MENU
//===========================================================================
//
// M_Reward
//
enum {
    nights,
	mario,
	adventure,
	xmasfind,
	golfm,
    reward_end
} reward_e;

//added:10-02-98: note: alphaKey member is the y offset
menuitem_t RewardMenu[]=
{
    {IT_STRING | IT_CALL,0,"Sonic Into Dreams", M_NightsGame,    30},
	{IT_STRING | IT_CALL,0,"Mario Koopa Blast", M_MarioGame,     50},
	{IT_STRING | IT_CALL,0,"Adventure Example", M_AdventureGame, 70},
	{IT_STRING | IT_CALL,0,"Christmas Hunt",    M_ChristmasGame, 90},
	{IT_STRING | IT_CALL,0,"Sonic Golf",        M_GolfGame,     110},
};

menu_t  RewardDef =
{
    "M_OPTTTL",
    "OPTIONS",
    reward_end,
    &SecretsDef,
    RewardMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

void M_Reward (int choice)
{
	if(grade & 8)
		RewardMenu[mario].status = IT_STRING | IT_CALL;
	else
		RewardMenu[mario].status |= IT_DISABLED;

	if(grade & 16)
		RewardMenu[nights].status = IT_STRING | IT_CALL;
	else
		RewardMenu[nights].status |= IT_DISABLED;

	if(grade & 32)
		RewardMenu[xmasfind].status = IT_STRING | IT_CALL;
	else
		RewardMenu[xmasfind].status |= IT_DISABLED;

	if(grade & 64)
		RewardMenu[adventure].status = IT_STRING | IT_CALL;
	else
		RewardMenu[adventure].status |= IT_DISABLED;

	// Disable it because it's not ready yet Graue 12-31-2003
	//if(grade & 1024)
	//	RewardMenu[golfm].status = IT_STRING | IT_CALL;
	//else
		RewardMenu[golfm].status |= IT_DISABLED;

	M_SetupNextMenu (&RewardDef);
}

//===========================================================================
//                             Level Select Menu
//===========================================================================
//
// M_LevelSelect
//

enum {
    gfz1,
	gfz2,
	gfz3,
	thz1,
	thz2,
	thz3,
	cez1,
	cez2,
	cez3,
    levelselect_end
} levelselect_e;

//added:10-02-98: note: alphaKey member is the y offset
menuitem_t LevelSelectMenu[]=
{
    {IT_STRING | IT_CALL,0,"Greenflower 1" ,M_LevelSelectWarp   ,10},
	{IT_STRING | IT_CALL,0,"Greenflower 2" ,M_LevelSelectWarp   ,20},
	{IT_STRING | IT_CALL,0,"Greenflower 3" ,M_LevelSelectWarp   ,30},
	{IT_STRING | IT_CALL,0,"Techno Hill 1" ,M_LevelSelectWarp   ,50},
	{IT_STRING | IT_CALL,0,"Techno Hill 2" ,M_LevelSelectWarp   ,60},
	{IT_STRING | IT_CALL,0,"Techno Hill 3" ,M_LevelSelectWarp   ,70},
	{IT_STRING | IT_CALL,0,"Castle Eggman 1" ,M_LevelSelectWarp   ,90},
	{IT_STRING | IT_CALL,0,"Castle Eggman 2" ,M_LevelSelectWarp   ,100},
	{IT_STRING | IT_CALL,0,"Castle Eggman 3" ,M_LevelSelectWarp   ,110},
};

menu_t  LevelSelectDef =
{
    "M_OPTTTL",
    "OPTIONS",
    levelselect_end,
    &SecretsDef,
    LevelSelectMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

void M_SecretsMenu(int choice)
{
	int i;

	// Disable all the menu choices
	for(i=ultimatecheat;i<secrets_end;i++)
		SecretsMenu[i].status = IT_DISABLED;

	// Check grade and enable options as appropriate
	switch(grade&7)
	{
		case 7:
		case 6:
//			SecretsMenu[betashowcase].status = IT_STRING | IT_CALL;
			SecretsMenu[norobots].status = IT_STRING | IT_CALL;
			SecretsMenu[rings].status = IT_STRING | IT_CVAR;
		case 5:
		case 4:
			SecretsMenu[lives].status = IT_STRING | IT_CVAR;
			SecretsMenu[continues].status = IT_STRING | IT_CVAR;
		case 3:
			SecretsMenu[ringslinger].status = IT_STRING | IT_CVAR;
		case 2:
			SecretsMenu[playerspeed].status = IT_STRING | IT_CVAR;
			SecretsMenu[secretsgravity].status = IT_STRING | IT_CVAR;
			SecretsMenu[ultimatecheat].status = IT_STRING | IT_CALL;
		case 1:
//			SecretsMenu[reward].status = IT_STRING | IT_CALL;
			break;
		default: // You shouldn't even be here, weirdo.
			return;
			break;
	}

	if((grade & 8) ||
	(grade & 16) ||
	(grade & 32) ||
	(grade & 64) ||
	(grade & 1024))
		SecretsMenu[reward].status = IT_STRING | IT_CALL;
	else
		SecretsMenu[reward].status = IT_DISABLED;


	if(grade & 128)
		SecretsMenu[levelselect].status = IT_STRING | IT_CALL;

	if(grade & 256)
		SecretsMenu[timeattackbonus].status = IT_STRING | IT_CALL;

	if(grade & 512)
		SecretsMenu[eastereggbonus].status = IT_STRING | IT_CALL;

    M_SetupNextMenu(&SecretsDef);
}

//
//  A smaller 'Thermo', with range given as percents (0-100)
//
void M_DrawSlider (int x, int y, int range)
{
    int i;

    if (range < 0)
        range = 0;
    if (range > 100)
        range = 100;

    V_DrawScaledPatch (x-8, y, 0, W_CachePatchName( "M_SLIDEL" ,PU_CACHE) );

    for (i=0 ; i<SLIDER_RANGE ; i++)
        V_DrawScaledPatch (x+i*8, y, 0,
                           W_CachePatchName( "M_SLIDEM" ,PU_CACHE) );

    V_DrawScaledPatch (x+SLIDER_RANGE*8, y, 0,
                       W_CachePatchName( "M_SLIDER" ,PU_CACHE) );

    // draw the slider cursor
    V_DrawMappedPatch (x + ((SLIDER_RANGE-1)*8*range)/100, y, 0,
                       W_CachePatchName( "M_SLIDEC" ,PU_CACHE),
                       whitemap);
}

//===========================================================================
//                        Video OPTIONS MENU
//===========================================================================

//added:10-02-98: note: alphaKey member is the y offset
menuitem_t VideoOptionsMenu[]=
{
	// Tails
    {IT_STRING | IT_SUBMENU,0, "Video Modes..."   , &VidModeDef       , 0},
#ifdef HWRENDER
    //17/10/99: added by Hurdler
    {IT_CALL|IT_WHITESTRING,0, "3D Card Options...", M_OpenGLOption    ,20},
#endif
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,    "Brightness"       , &cv_usegamma      , 40},

    {IT_STRING | IT_CVAR,0,    "V-SYNC"     , &cv_vidwait       , 50},

    {IT_STRING | IT_CVAR,0,"Snow Density" ,&cv_numsnow   ,70}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CVAR,0,"Rain Density" ,&cv_raindensity   ,80}, // Changed all to normal string Tails 11-30-2000
    {IT_STRING | IT_CVAR,0,"Rain/Snow Draw Dist" ,&cv_precipdist   ,90} // Changed all to normal string Tails 11-30-2000
};

menu_t  VideoOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(VideoOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    VideoOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Mouse OPTIONS MENU
//===========================================================================

//added:24-03-00: note: alphaKey member is the y offset
menuitem_t MouseOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Use Mouse",        &cv_usemouse        ,0},
    {IT_STRING | IT_CVAR,0,"Always MouseLook", &cv_alwaysfreelook  ,0},
    {IT_STRING | IT_CVAR,0,"Mouse Move"      , &cv_mousemove       ,0},
    {IT_STRING | IT_CVAR,0,"Invert Mouse"    , &cv_invertmouse     ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,"Mouse Speed"     , &cv_mousesens       ,0},
    {IT_STRING | IT_CVAR
     | IT_CV_SLIDER     ,0,"Mlook Speed"     , &cv_mlooksens       ,0}
#ifdef __MACOS__
        ,{IT_CALL   | IT_WHITESTRING,0,"Configure Input Sprocket..."  ,macConfigureInput     ,60}
#endif
};

menu_t  MouseOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(MouseOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    MouseOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Game OPTIONS MENU
//===========================================================================

menuitem_t GameOptionsMenu[]=
{
	// Tails
    {IT_STRING | IT_CVAR,0,"Number of Sound Channels"        ,&cv_numChannels        ,0},
    {IT_STRING | IT_CVAR,0,"Camera Speed"        ,&cv_cam_speed        ,0},
    {IT_STRING | IT_CVAR,0,"Camera Height"        ,&cv_cam_height        ,0},
    {IT_STRING | IT_CVAR,0,"Camera Distance"        ,&cv_cam_dist        ,0},
    {IT_STRING | IT_CVAR,0,"Hold Camera Angle"        ,&cv_cam_still        ,0},
    {IT_STRING | IT_CVAR,0,"Camera Rotation"        ,&cv_cam_rotate        ,0},
	{IT_STRING | IT_CV_SLIDER | IT_CVAR,0,"Camera Rotation Speed"  ,&cv_cam_rotspeed      ,0},
    {IT_STRING | IT_CVAR,0,"2P Camera Speed"        ,&cv_cam2_speed        ,0},
    {IT_STRING | IT_CVAR,0,"2P Camera Height"        ,&cv_cam2_height        ,0},
    {IT_STRING | IT_CVAR,0,"2P Camera Distance"        ,&cv_cam2_dist        ,0},
    {IT_STRING | IT_CVAR,0,"2P Hold Camera Angle"        ,&cv_cam2_still        ,0},
    {IT_STRING | IT_CVAR,0,"2P Camera Rotation"        ,&cv_cam2_rotate        ,0},
	{IT_STRING | IT_CV_SLIDER | IT_CVAR,0,"2P Camera Rotation Speed" ,&cv_cam2_rotspeed     ,0},
    {IT_STRING | IT_CVAR,0,"High Resolution Timer"        ,&cv_timetic        ,0},
    {IT_CALL   | IT_WHITESTRING,0,"Network Options..."  ,M_NetOption     ,0}
};

menu_t  GameOptionDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(GameOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    GameOptionsMenu,
    M_DrawGenericMenu,
    24,32,
    0
};

void M_GameOption(int choice)
{
    if(!(server || admin))
    {
        M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
        return;
    }
    M_SetupNextMenu(&GameOptionDef);
}

void M_MonitorToggles(int choice)
{
    if(!(server || admin))
    {
        M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
        return;
    }
    M_SetupNextMenu(&MonitorToggleDef);
}

//===========================================================================
//                        Network OPTIONS MENU
//===========================================================================

// Graue 12-06-2003
// Added circuit mode and moved down Monitor Toggles so everything would fit.
menuitem_t NetOptionsMenu[]=
{
	// Tails
    {IT_STRING | IT_CVAR,0,"Game Type",               &cv_gametype,        10}, // Tails

	{IT_STRING | IT_CALL,0, "Match Mode Options...",  M_MatchOptions,      20},
	{IT_STRING | IT_CALL,0, "Race Mode Options..." ,  M_RaceOptions,       30},
	{IT_STRING | IT_CALL,0, "Tag Mode Options..."  ,  M_TagOptions,        40},
	{IT_STRING | IT_CALL,0, "CTF Mode Options..."  ,  M_CTFOptions,        50},
	{IT_STRING | IT_CALL,0, "Chaos Mode Options...",  M_ChaosOptions,      60},
#ifdef CIRCUITMODE
	{IT_STRING | IT_CALL,0, "Circuit Mode Options...",M_CircuitOptions,    70}, // ifdef readded by Graue 12-31-2003
#endif
	{IT_STRING | IT_CVAR,0,"Server controls skin",    &cv_forceskin,       90},
	{IT_STRING | IT_CVAR,0,"Allow autoaim",           &cv_allowautoaim,   100},
    {IT_STRING | IT_CVAR,0,"Allow join player",       &cv_allownewplayer, 110},
	// Move level timelimit to the individual mode options Graue 12-13-2003
    {IT_STRING | IT_CVAR,0,"Max Players",             &cv_maxplayers,     120},

	{IT_STRING | IT_CALL,0, "Monitor Toggles...",     M_MonitorToggles,   130},

    {IT_CALL   | IT_WHITESTRING,0,"Game Options...",  M_GameOption,       150},
};

menu_t  NetOptionDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(NetOptionsMenu)/sizeof(menuitem_t),
    &MultiPlayerDef,
    NetOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Match Mode OPTIONS MENU
//===========================================================================

menuitem_t MatchOptionsMenu[]=
{
	{IT_STRING | IT_CVAR,0,"Special Ring Weapons", &cv_specialrings, 10},
    {IT_STRING | IT_CVAR,0,"Item Boxes"   ,&cv_matchboxes    ,20},
    {IT_STRING | IT_CVAR,0,"Item Respawn"        ,&cv_itemrespawn        ,30},
    {IT_STRING | IT_CVAR,0,"Item Respawn time"   ,&cv_itemrespawntime    ,40},
	{IT_STRING | IT_CVAR,0,"Scoring Type", &cv_match_scoring, 50},
	{IT_STRING | IT_CVAR,0,"Sudden Death" , &cv_suddendeath, 60},

	{IT_STRING | IT_CVAR,0,"Time Limit", &cv_timelimit, 80}, // Graue 12-13-2003
	{IT_STRING | IT_CVAR,0,"Point Limit", &cv_fraglimit, 90}, // Graue 12-13-2003

	{IT_STRING | IT_CVAR,0,"Intermission Timer"      ,&cv_inttime      ,110},
	{IT_STRING | IT_CVAR,0,"Advance to next level"      ,&cv_advancemap      ,120},
};

menu_t  MatchOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(MatchOptionsMenu)/sizeof(menuitem_t),
    &NetOptionDef,
    MatchOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Race Mode OPTIONS MENU
//===========================================================================

menuitem_t RaceOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Type of Race"        ,&cv_racetype        ,30},
    {IT_STRING | IT_CVAR,0,"Item Boxes"   ,&cv_raceitemboxes    ,40},
};

menu_t  RaceOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(RaceOptionsMenu)/sizeof(menuitem_t),
    &NetOptionDef,
    RaceOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Circuit Mode OPTIONS MENU
//===========================================================================
// Graue 12-06-2003
#ifdef CIRCUITMODE
menuitem_t CircuitOptionsMenu[]=
{
	{IT_STRING | IT_CVAR,0,"Throw rings",          &cv_circuit_ringthrow, 10},
	{IT_STRING | IT_CVAR,0,"Special ring weapons", &cv_specialrings,      20},
    {IT_STRING | IT_CVAR,0,"Item boxes",           &cv_circuit_itemboxes, 30},
    {IT_STRING | IT_CVAR,0,"Item respawn",         &cv_itemrespawn,       40},
    {IT_STRING | IT_CVAR,0,"Item respawn time",    &cv_itemrespawntime,   50},
	{IT_STRING | IT_CVAR,0,"Intermission timer",   &cv_inttime,           60},

	{IT_STRING | IT_CVAR,0,"Allow special moves",  &cv_circuit_specmoves, 80}, // Graue 12-08-2003
	{IT_STRING | IT_CVAR,0,"Allow spindashing",    &cv_circuit_spin,      90},
	//{IT_STRING | IT_CVAR,0,"Advance to next level",&cv_advancemap,        70},
	//{IT_STRING | IT_CVAR,0,"Sudden death",         &cv_suddendeath,       80},
};

menu_t  CircuitOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(CircuitOptionsMenu)/sizeof(menuitem_t),
    &NetOptionDef,
    CircuitOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};
#endif
//===========================================================================
//                        Tag Mode OPTIONS MENU
//===========================================================================

menuitem_t TagOptionsMenu[]=
{
	{IT_STRING | IT_CVAR,0,"Special Ring Weapons", &cv_specialrings, 10},
    {IT_STRING | IT_CVAR,0,"Item Boxes"   ,&cv_matchboxes    ,20},
    {IT_STRING | IT_CVAR,0,"Item Respawn"        ,&cv_itemrespawn        ,30},
    {IT_STRING | IT_CVAR,0,"Item Respawn time"   ,&cv_itemrespawntime    ,40},

	{IT_STRING | IT_CVAR,0,"Time Limit",  &cv_timelimit, 60}, // Graue 12-13-2003
	{IT_STRING | IT_CVAR,0,"Point Limit", &cv_fraglimit, 70}, // Graue 12-13-2003

	{IT_STRING | IT_CVAR,0,"Intermission Timer"      ,&cv_inttime      ,90},
	{IT_STRING | IT_CVAR,0,"Advance to next level"      ,&cv_advancemap      ,100},
};

menu_t  TagOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(TagOptionsMenu)/sizeof(menuitem_t),
    &NetOptionDef,
    TagOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        CTF Mode OPTIONS MENU
//===========================================================================

menuitem_t CTFOptionsMenu[]=
{ // Reordered the options nicely Graue 12-13-2003
	{IT_STRING | IT_CVAR,0,"Special Ring Weapons", &cv_specialrings, 10},
    {IT_STRING | IT_CVAR,0,"Item Boxes"   ,&cv_matchboxes    ,20},
    {IT_STRING | IT_CVAR,0,"Item Respawn"        ,&cv_itemrespawn        ,30},
    {IT_STRING | IT_CVAR,0,"Item Respawn time"   ,&cv_itemrespawntime    ,40},
	{IT_STRING | IT_CVAR,0,"Flag Respawn Time"      ,&cv_flagtime      ,50},
	{IT_STRING | IT_CVAR,0,"Scoring Type", &cv_ctf_scoring, 60},

	{IT_STRING | IT_CVAR,0,"Auto-sort CTF Teams"      ,&cv_autoctf      ,80},

	{IT_STRING | IT_CVAR,0,"Time Limit", &cv_timelimit, 100},
	{IT_STRING | IT_CVAR,0,"Point Limit", &cv_fraglimit, 110},
	
	{IT_STRING | IT_CVAR,0,"Intermission Timer"      ,&cv_inttime      ,130},
	{IT_STRING | IT_CVAR,0,"Advance to next level"      ,&cv_advancemap      ,140},
};

menu_t  CTFOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(CTFOptionsMenu)/sizeof(menuitem_t),
    &NetOptionDef,
    CTFOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Chaos Mode OPTIONS MENU
//===========================================================================

menuitem_t ChaosOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Item Boxes"   ,&cv_matchboxes    ,20},
    {IT_STRING | IT_CVAR,0,"Item Respawn"        ,&cv_itemrespawn        ,30},
    {IT_STRING | IT_CVAR,0,"Item Respawn time"   ,&cv_itemrespawntime    ,40},
	{IT_STRING | IT_CVAR,0,"Enemy respawn rate"   ,&cv_chaos_spawnrate, 50},
    {IT_SUBMENU | IT_STRING,0,"Enemy Toggles..." ,&EnemyToggleDef, 60},

	{IT_STRING | IT_CVAR,0,"Time Limit", &cv_timelimit, 80},
	{IT_STRING | IT_CVAR,0,"Point Limit", &cv_fraglimit, 90},

	{IT_STRING | IT_CVAR,0,"Intermission Timer"      ,&cv_inttime      ,110},
	{IT_STRING | IT_CVAR,0,"Advance to next level"      ,&cv_advancemap      ,120},

};

menu_t  ChaosOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(ChaosOptionsMenu)/sizeof(menuitem_t),
    &NetOptionDef,
    ChaosOptionsMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Enemy Toggle MENU
//===========================================================================

menuitem_t EnemyToggleMenu[]=
{
    {IT_STRING | IT_CVAR,0,"Blue Crawla"     ,&cv_chaos_bluecrawla     ,30},
    {IT_STRING | IT_CVAR,0,"Red Crawla"      ,&cv_chaos_redcrawla      ,40},
	{IT_STRING | IT_CVAR,0,"Crawla Commander",&cv_chaos_crawlacommander,50},
	{IT_STRING | IT_CVAR,0,"JettySyn Bomber" ,&cv_chaos_jettysynbomber ,60},
	{IT_STRING | IT_CVAR,0,"JettySyn Gunner" ,&cv_chaos_jettysyngunner ,70},
	{IT_STRING | IT_CVAR,0,"Boss 1" ,&cv_chaos_eggmobile1 ,80},
	{IT_STRING | IT_CVAR,0,"Boss 2" ,&cv_chaos_eggmobile2 ,90},
};

menu_t  EnemyToggleDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(EnemyToggleMenu)/sizeof(menuitem_t),
    &ChaosOptionsDef,
    EnemyToggleMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

//===========================================================================
//                        Monitor Toggle MENU
//===========================================================================

menuitem_t MonitorToggleMenu[]=
{
	{IT_STRING | IT_CVAR,0,"Teleporters"  , &cv_teleporters, 20},
    {IT_STRING | IT_CVAR,0,"Super Ring"     ,&cv_superring     ,30},
    {IT_STRING | IT_CVAR,0,"Silver Ring"      ,&cv_silverring      ,40},
	{IT_STRING | IT_CVAR,0,"Super Sneakers",&cv_supersneakers,50},
	{IT_STRING | IT_CVAR,0,"Invincibility" ,&cv_invincibility ,60},
	{IT_STRING | IT_CVAR,0,"Basic Shield" ,&cv_blueshield ,70},
	{IT_STRING | IT_CVAR,0,"Liquid Shield" ,&cv_greenshield ,80},
	{IT_STRING | IT_CVAR,0,"Attraction Shield" ,&cv_yellowshield ,90},
	{IT_STRING | IT_CVAR,0,"Inferno Shield" ,&cv_redshield ,100},
	{IT_STRING | IT_CVAR,0,"Armageddon Shield" ,&cv_blackshield ,110},
	{IT_STRING | IT_CVAR,0,"1 Up" ,&cv_1up ,120},
	{IT_STRING | IT_CVAR,0,"Eggman Box" ,&cv_eggmanbox, 130},
};

menu_t  MonitorToggleDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(MonitorToggleMenu)/sizeof(menuitem_t),
    &NetOptionDef,
    MonitorToggleMenu,
    M_DrawGenericMenu,
    60,40,
    0
};

void M_NetOption(int choice)
{
    if(!(server || admin))
    {
        M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
        return;
    }
    M_SetupNextMenu(&NetOptionDef);
}

// Tails 08-12-2002
void M_MatchOptions(int choice)
{
    if(!(server || admin))
    {
        M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
        return;
    }
    M_SetupNextMenu(&MatchOptionsDef);
}
void M_RaceOptions(int choice)
{
    if(!(server || admin))
    {
        M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
        return;
    }
    M_SetupNextMenu(&RaceOptionsDef);
}
void M_TagOptions(int choice)
{
    if(!(server || admin))
    {
        M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
        return;
    }
    M_SetupNextMenu(&TagOptionsDef);
}
void M_CTFOptions(int choice)
{
    if(!(server || admin))
    {
        M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
        return;
    }
    M_SetupNextMenu(&CTFOptionsDef);
}
void M_ChaosOptions(int choice)
{
    if(!(server || admin))
    {
        M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
        return;
    }
    M_SetupNextMenu(&ChaosOptionsDef);
}
#ifdef CIRCUITMODE
void M_CircuitOptions(int choice)
{
	if(!(server || admin))
	{
		M_StartMessage("You are not the server\nYou can't change the options\n",NULL,MM_NOTHING);
		return;
	}
	M_SetupNextMenu(&CircuitOptionsDef);
}
#endif

//===========================================================================
//                        Server OPTIONS MENU
//===========================================================================
menuitem_t ServerOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0, "Internet server", &cv_internetserver   ,  0},
    {IT_STRING | IT_CVAR
        | IT_CV_STRING  ,0, "Master server",   &cv_masterserver     ,  0},
    {IT_STRING | IT_CVAR
        | IT_CV_STRING  ,0, "Server name",     &cv_servername       ,  0},
};

menu_t  ServerOptionsDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(ServerOptionsMenu)/sizeof(menuitem_t),
    &OptionsDef,
    ServerOptionsMenu,
    M_DrawGenericMenu,
    28,40,
    0
};

//===========================================================================
//                          Read This! MENU 1
//===========================================================================

void M_DrawReadThis1(void);
void M_DrawReadThis2(void);

enum
{
    rdthsempty1,
    read1_end
} read_e;

menuitem_t ReadMenu1[] =
{
    {IT_SUBMENU | IT_NOTHING,0,"",&ReadDef2,0}
};

menu_t  ReadDef1 =
{
    NULL,
    NULL,
    read1_end,
    &MainDef,
    ReadMenu1,
    M_DrawReadThis1,
    280,185,
    0
};

//
// Read This Menus
// Had a "quick hack to fix romero bug"
//
void M_DrawReadThis1(void)
{
    V_DrawScaledPatch (0,0,0,W_CachePatchName("HELP",PU_CACHE));
    return;
}

//===========================================================================
//                          Read This! MENU 2
//===========================================================================

enum
{
    rdthsempty2,
    read2_end
} read_e2;

menuitem_t ReadMenu2[]=
{
    {IT_SUBMENU | IT_NOTHING,0,"",&MainDef,0}
};

menu_t  ReadDef2 =
{
    NULL,
    NULL,
    read2_end,
    &ReadDef1,
    ReadMenu2,
    M_DrawReadThis2,
    330,175,
    0
};


//
// Read This Menus - optional second page.
//
void M_DrawReadThis2(void)
{
    // This hack keeps us from having to change menus.
//    V_DrawScaledPatch (0,0,0,W_CachePatchName("CREDIT",PU_CACHE));
    return;
}

//===========================================================================
//                        SOUND VOLUME MENU
//===========================================================================
void M_DrawSound(void);

void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_CDAudioVol (int choice);

enum
{
    sfx_vol,
    sfx_empty1,
    music_vol,
    sfx_empty2,
    cdaudio_vol,
    sfx_empty3,
    sound_end
} sound_e;

menuitem_t SoundMenu[]=
{
    {IT_CVARMAX   | IT_PATCH ,"M_SFXVOL","Sound Volume",&cv_soundvolume  ,'s'},
    {IT_BIGSLIDER | IT_SPACE ,NULL      ,NULL          ,&cv_soundvolume      },
    {IT_CVARMAX   | IT_PATCH ,"M_MUSVOL","Music Volume",&cv_musicvolume  ,'m'},
    {IT_BIGSLIDER | IT_SPACE ,NULL      ,NULL          ,&cv_musicvolume      },
    {IT_CVARMAX   | IT_PATCH ,"M_CDVOL" ,"CD Volume"   ,&cd_volume       ,'c'},
    {IT_BIGSLIDER | IT_SPACE ,NULL      ,NULL          ,&cd_volume           }
};

menu_t  SoundDef =
{
    "M_SVOL",
    "Sound Volume",
    sound_end,
    &OptionsDef,
    SoundMenu,
    M_DrawGenericMenu,
    80,50,
    0
};


//===========================================================================
//                          CONTROLS MENU
//===========================================================================
void M_DrawControl(void);               // added 3-1-98
void M_ChangeControl(int choice);

//
// this is the same for all control pages
//
menuitem_t ControlMenu[]=
{
    {IT_CALL | IT_STRING2,0,"Ring Throw"  ,M_ChangeControl,gc_fire       }, // Tails 11-09-99
	{IT_CALL | IT_STRING2,0,"Taunt"       ,M_ChangeControl,gc_taunt       }, // Tails 11-09-99
    {IT_CALL | IT_STRING2,0,"Spin"        ,M_ChangeControl,gc_use        }, // Tails 12-04-99
    {IT_CALL | IT_STRING2,0,"Jump"        ,M_ChangeControl,gc_jump       },
    {IT_CALL | IT_STRING2,0,"Light Dash"  ,M_ChangeControl,gc_lightdash       },
    {IT_CALL | IT_STRING2,0,"Forward"     ,M_ChangeControl,gc_forward    },
    {IT_CALL | IT_STRING2,0,"Backpedal"   ,M_ChangeControl,gc_backward   },
    {IT_CALL | IT_STRING2,0,"Turn Left"   ,M_ChangeControl,gc_turnleft   },
    {IT_CALL | IT_STRING2,0,"Turn Right"  ,M_ChangeControl,gc_turnright  },
    {IT_CALL | IT_STRING2,0,"Walk"         ,M_ChangeControl,gc_speed      },
    {IT_CALL | IT_STRING2,0,"Strafe On"   ,M_ChangeControl,gc_strafe     },
    {IT_CALL | IT_STRING2,0,"Strafe Left" ,M_ChangeControl,gc_strafeleft },
    {IT_CALL | IT_STRING2,0,"Strafe Right",M_ChangeControl,gc_straferight},
    {IT_CALL | IT_STRING2,0,"Look Up"     ,M_ChangeControl,gc_lookup     },
    {IT_CALL | IT_STRING2,0,"Look Down"   ,M_ChangeControl,gc_lookdown   },
    {IT_CALL | IT_STRING2,0,"Center View" ,M_ChangeControl,gc_centerview },
    {IT_CALL | IT_STRING2,0,"Mouselook"   ,M_ChangeControl,gc_mouseaiming},

    {IT_SUBMENU | IT_WHITESTRING,0,"next" ,&ControlDef2,144}
};

menu_t  ControlDef =
{
    "M_CONTRO",
    "Setup Controls",
    sizeof(ControlMenu)/sizeof(menuitem_t),
    &OptionsDef,
    ControlMenu,
    M_DrawControl,
    50,40,
    0
};

//
//  Controls page 1
//
menuitem_t ControlMenu2[]=
{
   // Tails
/*  {IT_CALL | IT_STRING2,0,"Fist/Chainsaw"  ,M_ChangeControl,gc_weapon1},
  {IT_CALL | IT_STRING2,0,"Pistol"         ,M_ChangeControl,gc_weapon2},
  {IT_CALL | IT_STRING2,0,"Shotgun/Double" ,M_ChangeControl,gc_weapon3},
  {IT_CALL | IT_STRING2,0,"Chaingun"       ,M_ChangeControl,gc_weapon4},
  {IT_CALL | IT_STRING2,0,"Rocket Launcher",M_ChangeControl,gc_weapon5},
  {IT_CALL | IT_STRING2,0,"Plasma rifle"   ,M_ChangeControl,gc_weapon6},
  {IT_CALL | IT_STRING2,0,"BFG"            ,M_ChangeControl,gc_weapon7},
  {IT_CALL | IT_STRING2,0,"Chainsaw"       ,M_ChangeControl,gc_weapon8},
  {IT_CALL | IT_STRING2,0,"Previous Weapon",M_ChangeControl,gc_prevweapon},
  {IT_CALL | IT_STRING2,0,"Next Weapon"    ,M_ChangeControl,gc_nextweapon},*/
  {IT_CALL | IT_STRING2,0,"Talk key"       ,M_ChangeControl,gc_talkkey},
  {IT_CALL | IT_STRING2,0,"Rankings/Scores",M_ChangeControl,gc_scores },
  {IT_CALL | IT_STRING2,0,"Console"        ,M_ChangeControl,gc_console},
  {IT_CALL | IT_STRING2,0,"Rotate Camera L",M_ChangeControl,gc_camleft},
  {IT_CALL | IT_STRING2,0,"Rotate Camera R",M_ChangeControl,gc_camright},
  {IT_CALL | IT_STRING2,0,"Reset Camera",M_ChangeControl, gc_camreset},
/*  {IT_CALL | IT_STRING2,0,"Inventory Left" ,M_ChangeControl,gc_invprev},  
  {IT_CALL | IT_STRING2,0,"Inventory Right",M_ChangeControl,gc_invnext},
  {IT_CALL | IT_STRING2,0,"Inventory Use"  ,M_ChangeControl,gc_invuse },
  {IT_CALL | IT_STRING2,0,"Fly down"       ,M_ChangeControl,gc_flydown},*/
                        
  {IT_SUBMENU | IT_WHITESTRING,0,"next"    ,&ControlDef,140}
};

menu_t  ControlDef2 =
{
    "M_CONTRO",
    "Setup Controls",
    sizeof(ControlMenu2)/sizeof(menuitem_t),
    &OptionsDef,
    ControlMenu2,
    M_DrawControl,
    50,40,
    0
};


//
// Start the controls menu, setting it up for either the console player,
// or the secondary splitscreen player
//
static  boolean setupcontrols_secondaryplayer;
static  int   (*setupcontrols)[2];  // pointer to the gamecontrols of the player being edited
void M_Setup1PControlsMenu (int choice)
{
    setupcontrols_secondaryplayer = false;
    setupcontrols = gamecontrol;        // was called from main Options (for console player, then)
    currentMenu->lastOn = itemOn;
    M_SetupNextMenu(&ControlDef);
}

void M_Setup2PControlsMenu(int choice)
{
	setupcontrols_secondaryplayer = true;
	setupcontrols = gamecontrolbis;
    currentMenu->lastOn = itemOn;
    M_SetupNextMenu(&ControlDef);
}


//
//  Draws the Customise Controls menu
//
void M_DrawControl(void)
{
    char     tmp[50];
    int      i;
    int      keys[2];

    // draw title, strings and submenu
    M_DrawGenericMenu();

    M_CentreText (ControlDef.y-12,
        (setupcontrols_secondaryplayer ? "SET CONTROLS FOR SECONDARY PLAYER" :
                                         "PRESS ENTER TO CHANGE, BACKSPACE TO CLEAR") );

    for(i=0;i<currentMenu->numitems;i++)
    {
        if (currentMenu->menuitems[i].status!=IT_CONTROL)
            continue;

        keys[0] = setupcontrols[currentMenu->menuitems[i].alphaKey][0];
        keys[1] = setupcontrols[currentMenu->menuitems[i].alphaKey][1];

        tmp[0]='\0';
        if (keys[0] == KEY_NULL && keys[1] == KEY_NULL)
        {
            strcpy(tmp, "---");
        }
        else
        {
            if( keys[0] != KEY_NULL )
                strcat (tmp, G_KeynumToString (keys[0]));

            if( keys[0] != KEY_NULL && keys[1] != KEY_NULL )
                strcat(tmp," or ");

            if( keys[1] != KEY_NULL )
                strcat (tmp, G_KeynumToString (keys[1]));


        }
        V_DrawString(ControlDef.x+220-V_StringWidth(tmp), ControlDef.y + i*8,V_WHITEMAP, tmp);
    }

}

static int controltochange;

void M_ChangecontrolResponse(event_t* ev)
{
    int        control;
    int        found;
    int        ch=ev->data1;

    // ESCAPE cancels
    if (ch!=KEY_ESCAPE && ch!=KEY_PAUSE)
    {

        switch (ev->type)
        {
          // ignore mouse/joy movements, just get buttons
          case ev_mouse:
               ch = KEY_NULL;      // no key
            break;
          case ev_joystick:
               ch = KEY_NULL;      // no key
            break;

          // keypad arrows are converted for the menu in cursor arrows
          // so use the event instead of ch
          case ev_keydown:
            ch = ev->data1;
            break;

          default:
            break;
        }

        control = controltochange;

        // check if we already entered this key
        found = -1;
        if (setupcontrols[control][0]==ch)
            found = 0;
        else
        if (setupcontrols[control][1]==ch)
            found = 1;
        if (found>=0)
        {
            // replace mouse and joy clicks by double clicks
            if (ch>=KEY_MOUSE1 && ch<=KEY_MOUSE1+MOUSEBUTTONS)
                setupcontrols[control][found] = ch-KEY_MOUSE1+KEY_DBLMOUSE1;
            else
              if (ch>=KEY_JOY1 && ch<=KEY_JOY1+JOYBUTTONS)
                setupcontrols[control][found] = ch-KEY_JOY1+KEY_DBLJOY1;
        }
        else
        {
            // check if change key1 or key2, or replace the two by the new
            found = 0;
            if (setupcontrols[control][0] == KEY_NULL)
                found++;
            if (setupcontrols[control][1] == KEY_NULL)
                found++;
            if (found==2)
            {
                found = 0;
                setupcontrols[control][1] = KEY_NULL;  //replace key 1 ,clear key2
            }
            G_CheckDoubleUsage(ch);
            setupcontrols[control][found] = ch;
        }

    }

    M_StopMessage(0);
}

void M_ChangeControl(int choice)
{
    static char tmp[55];

    controltochange = currentMenu->menuitems[choice].alphaKey;
    sprintf (tmp,"Hit the new key for\n%s\nESC for Cancel",currentMenu->menuitems[choice].text);

    M_StartMessage (tmp,M_ChangecontrolResponse,MM_EVENTHANDLER);
}

//===========================================================================
//                        VIDEO MODE MENU
//===========================================================================
void M_DrawVideoMode(void);             //added:30-01-98:

void M_HandleVideoMode (int ch);

menuitem_t VideoModeMenu[]=
{
    {IT_KEYHANDLER | IT_NOTHING, 0, "", M_HandleVideoMode, '\0'},     // dummy menuitem for the control func
};


menu_t  VidModeDef =
{
    "M_VIDEO",
    "Video Mode",
    1,                  // # of menu items
    //sizeof(VideoModeMenu)/sizeof(menuitem_t),
    &OptionsDef,        // previous menu
    VideoModeMenu,      // menuitem_t ->
    M_DrawVideoMode,    // drawing routine ->
    48,36,              // x,y
    0                   // lastOn
};

//added:30-01-98:
#define MAXCOLUMNMODES   10     //max modes displayed in one column
#define MAXMODEDESCS     (MAXCOLUMNMODES*3)

// shhh... what am I doing... nooooo!
static int vidm_testingmode=0;
static int vidm_previousmode;
static int vidm_current=0;
static int vidm_nummodes;
static int vidm_column_size;

typedef struct
{
    int     modenum;    //video mode number in the vidmodes list
    char    *desc;      //XXXxYYY
    int     iscur;      //1 if it is the current active mode
} modedesc_t;

static modedesc_t   modedescs[MAXMODEDESCS];


//
// Draw the video modes list, a-la-Quake
//
void M_DrawVideoMode(void)
{
    int     i,j,dup,row,col,nummodes;
    char    *desc;
    char    temp[80];

    // draw tittle
    M_DrawMenuTitle();

#ifdef LINUX
    VID_PrepareModeList(); // FIXME: hack
#endif
    vidm_nummodes = 0;
    nummodes = VID_NumModes ();

#ifdef __WIN32__
    //faB: clean that later : skip windowed mode 0, video modes menu only shows
    //     FULL SCREEN modes
    if (nummodes<1) {
        // put the windowed mode so that there is at least one mode
        modedescs[0].modenum = 0;
        modedescs[0].desc = VID_GetModeName (0);
        modedescs[0].iscur = 1;
        vidm_nummodes = 1;
    }
    for (i=1 ; i<=nummodes && vidm_nummodes<MAXMODEDESCS ; i++)
#else
    // DOS does not skip mode 0, because mode 0 is ALWAYS present
    for (i=0 ; i<nummodes && vidm_nummodes<MAXMODEDESCS ; i++)
#endif
    {
        desc = VID_GetModeName (i);
        if (desc)
        {
            dup = 0;

            //when a resolution exists both under VGA and VESA, keep the
            // VESA mode, which is always a higher modenum
            for (j=0 ; j<vidm_nummodes ; j++)
            {
                if (!strcmp (modedescs[j].desc, desc))
                {
                    //mode(0): 320x200 is always standard VGA, not vesa
                    if (modedescs[j].modenum != 0)
                    {
                        modedescs[j].modenum = i;
                        dup = 1;

                        if (i == vid.modenum)
                            modedescs[j].iscur = 1;
                    }
                    else
                    {
                        dup = 1;
                    }

                    break;
                }
            }

            if (!dup)
            {
                modedescs[vidm_nummodes].modenum = i;
                modedescs[vidm_nummodes].desc = desc;
                modedescs[vidm_nummodes].iscur = 0;

                if (i == vid.modenum)
                    modedescs[vidm_nummodes].iscur = 1;

                vidm_nummodes++;
            }
        }
    }

    vidm_column_size = (vidm_nummodes+2) / 3;


    row = 41; // was 16 Tails 11-30-2000
    col = VidModeDef.y;
    for(i=0; i<vidm_nummodes; i++)
    {
        V_DrawString (row, col, modedescs[i].iscur ? V_WHITEMAP : 0, modedescs[i].desc);

        col += 8;
        if((i % vidm_column_size) == (vidm_column_size-1))
        {
            row += 7*13; // was 8*13 Tails 11-30-2000
            col = 36;
        }
    }

    if (vidm_testingmode>0)
    {
        sprintf(temp, "TESTING MODE %s", modedescs[vidm_current].desc );
        M_CentreText(VidModeDef.y+80+24, temp );
        M_CentreText(VidModeDef.y+90+24, "Please wait 5 seconds..." );
    }
    else
    {
        M_CentreText(VidModeDef.y+60+24,"Press ENTER to set mode");

        M_CentreText(VidModeDef.y+70+24,"T to test mode for 5 seconds");

        sprintf(temp, "D to make %s the default", VID_GetModeName(vid.modenum));
        M_CentreText(VidModeDef.y+80+24,temp);

        sprintf(temp, "Current default is %dx%d (%d bits)", cv_scr_width.value, cv_scr_height.value, cv_scr_depth.value);
        M_CentreText(VidModeDef.y+90+24,temp);

        M_CentreText(VidModeDef.y+100+24,"Press ESC to exit");
    }

// Draw the cursor for the VidMode menu
    if (skullAnimCounter<4)    //use the Skull anim counter to blink the cursor // Tails
    {
        i = 41 - 10 + ((vidm_current / vidm_column_size)*7*13); // first integer was 16, second to last was 8 Tails 11-30-2000
        j = VidModeDef.y + ((vidm_current % vidm_column_size)*8);
        V_DrawCharacter( i-14, j, '*');
    }
}


//added:30-01-98: special menuitem key handler for video mode list
void M_HandleVideoMode (int ch)
{
    if (vidm_testingmode>0)
    {
       // change back to the previous mode quickly
       if (ch==KEY_ESCAPE)
       {
           setmodeneeded = vidm_previousmode+1;
           vidm_testingmode = 0;
       }
       return;
    }

    switch( ch )
    {
      case KEY_DOWNARROW:
        S_StartSound(NULL,sfx_menu1); // Tails
        vidm_current++;
        if (vidm_current>=vidm_nummodes)
            vidm_current = 0;
        break;

      case KEY_UPARROW:
        S_StartSound(NULL,sfx_menu1); // Tails
        vidm_current--;
        if (vidm_current<0)
            vidm_current = vidm_nummodes-1;
        break;

      case KEY_LEFTARROW:
        S_StartSound(NULL,sfx_menu1); // Tails
        vidm_current -= vidm_column_size;
        if (vidm_current<0)
            vidm_current = (vidm_column_size*3) + vidm_current;
        if (vidm_current>=vidm_nummodes)
            vidm_current = vidm_nummodes-1;
        break;

      case KEY_RIGHTARROW:
        S_StartSound(NULL,sfx_menu1); // Tails
        vidm_current += vidm_column_size;
        if (vidm_current>=(vidm_column_size*3))
            vidm_current %= vidm_column_size;
        if (vidm_current>=vidm_nummodes)
            vidm_current = vidm_nummodes-1;
        break;

      case KEY_ENTER:
        S_StartSound(NULL,sfx_menu1); // Tails
        if (!setmodeneeded) //in case the previous setmode was not finished
            setmodeneeded = modedescs[vidm_current].modenum+1;
        break;

      case KEY_ESCAPE:      //this one same as M_Responder
// Tails        S_StartSound(NULL,sfx_swtchx);

        if (currentMenu->prevMenu)
            M_SetupNextMenu (currentMenu->prevMenu);
        else
            M_ClearMenus (true);
        return;

      case 'T':
      case 't':
// Tails        S_StartSound(NULL,sfx_swtchx);
        vidm_testingmode = TICRATE*5;
        vidm_previousmode = vid.modenum;
        if (!setmodeneeded) //in case the previous setmode was not finished
            setmodeneeded = modedescs[vidm_current].modenum+1;
        return;

      case 'D':
      case 'd':
        // current active mode becomes the default mode.
// Tails        S_StartSound(NULL,sfx_swtchx);
        SCR_SetDefaultMode ();
        return;

      default:
        break;
    }

}


//===========================================================================
//LOAD GAME MENU
//===========================================================================
void M_DrawLoad(void);

void M_LoadSelect(int choice);

enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load_end
} load_e;

menuitem_t LoadGameMenu[]=
{
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'1'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'2'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'3'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'4'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'5'},
    {IT_CALL | IT_NOTHING,"",0, M_LoadSelect,'6'}
};

menu_t  LoadDef =
{
    "M_LOADG",
    "Load Game",
    load_end,
    &MainDef,
    LoadGameMenu,
    M_DrawLoad,
    80,54,
    0
};

void M_DrawGameStats(void)
{
	byte* colormap;
	int ecks;
	saveSlotSelected = itemOn;

	ecks = LoadDef.x + 24;
	M_DrawTextBox(LoadDef.x-8,144, 23, 4);

	if(savegameinfo[saveSlotSelected].lives == -42) // Empty
	{
		V_DrawString(ecks + 16, 152, 0, "EMPTY");
		return;
	}

    if (savegameinfo[saveSlotSelected].skincolor==0)
        colormap = colormaps;
    else
	{
		colormap = (byte *) translationtables[savegameinfo[saveSlotSelected].skinnum] - 256 + (savegameinfo[saveSlotSelected].skincolor<<8);
	}

	V_DrawMappedPatch ((LoadDef.x)*vid.fdupx,(144+8)*vid.fdupy, V_NOSCALESTART,W_CachePatchName(skins[savegameinfo[saveSlotSelected].skinnum].faceprefix, PU_CACHE), colormap); // Tails 03-11-2000

	V_DrawString(ecks + 16, 152, 0, savegameinfo[saveSlotSelected].playername);
	V_DrawString(ecks + 16, 160, 0, va("%s %d", savegameinfo[saveSlotSelected].levelname, savegameinfo[saveSlotSelected].actnum));
	V_DrawScaledPatch(ecks + 16, 168, 0, W_CachePatchName("CHAOS1", PU_CACHE));
	V_DrawString(ecks + 36, 172, 0, va("x %d", savegameinfo[saveSlotSelected].numemeralds));

	V_DrawScaledPatch(ecks + 64, 169, 0, W_CachePatchName("ONEUP", PU_CACHE));
	V_DrawString(ecks + 84, 172, 0, va("x %d", savegameinfo[saveSlotSelected].lives));

	V_DrawScaledPatch(ecks + 120, 168, 0, W_CachePatchName("CONTINS", PU_CACHE));
	V_DrawString(ecks + 140, 172, 0, va("x %d", savegameinfo[saveSlotSelected].continues));
}

//
// M_LoadGame & Cie.
//
void M_DrawLoad(void)
{
    int             i;

    M_DrawGenericMenu();

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        V_DrawString(LoadDef.x,LoadDef.y+LINEHEIGHT*i,0,savegamestrings[i]);
    }
	M_DrawGameStats();
}

//
// User wants to load this game
//
void M_LoadSelect(int choice)
{
    G_LoadGame (choice);
    M_ClearMenus (true);
}

#define VERSIONSIZE             16
// Reads the save file to list lives, level, player, etc.
// Tails 05-29-2003
void M_ReadSavegameInfo (int slot)
{
    int         length;
    char        savename[255];
	byte*       savebuffer;
	byte*       save_p;
	int fake; // Dummy variable

    sprintf(savename, savegamename, slot);

    length = FIL_ReadFile (savename, &savebuffer);
    if (!length)
    {
        CONS_Printf ("Couldn't read file %s", savename);
		savegameinfo[slot].lives = -42;
        return;
    }

    // skip the description field
    save_p = savebuffer + SAVESTRINGSIZE;
    
    save_p += VERSIONSIZE;

    // dearchive all the modifications
	// P_UnArchiveMisc()
	{
		int i;

		READBYTE(save_p);
		READBYTE(save_p);

		fake = READSHORT(save_p);
		strcpy(savegameinfo[slot].levelname, mapheaderinfo[fake-1].lvlttl);

		savegameinfo[slot].actnum = mapheaderinfo[fake-1].actnum;

		READULONG(save_p);
		READULONG(save_p);

		fake    = READUSHORT(save_p);

		savegameinfo[slot].numemeralds = 0;

		if(fake & EMERALD1)
			savegameinfo[slot].numemeralds++;

		if(fake & EMERALD2)
			savegameinfo[slot].numemeralds++;

		if(fake & EMERALD3)
			savegameinfo[slot].numemeralds++;

		if(fake & EMERALD4)
			savegameinfo[slot].numemeralds++;

		if(fake & EMERALD5)
			savegameinfo[slot].numemeralds++;

		if(fake & EMERALD6)
			savegameinfo[slot].numemeralds++;

		if(fake & EMERALD7)
			savegameinfo[slot].numemeralds++;

		READBYTE(save_p);

		READULONG(save_p);

		for(i=0; i<NUMMAPS; i++)
			READBYTE(save_p);
	}

	//P_UnArchivePlayers()
	{
		READLONG(save_p);
		READLONG(save_p);
		READLONG(save_p);
		READLONG(save_p);
		
		savegameinfo[slot].skincolor = READBYTE(save_p);
		savegameinfo[slot].skinnum = READBYTE(save_p);
		strcpy(savegameinfo[slot].playername, skins[savegameinfo[slot].skinnum].name); // Tails 03-25-2001

		// Tails
		READLONG(save_p); // Score
		READBYTE(save_p);
		READBYTE(save_p);

		savegameinfo[slot].lives = READLONG(save_p);
		savegameinfo[slot].continues = READLONG(save_p);

		READBYTE(save_p);
		READBYTE(save_p);
		READLONG(save_p);
		READLONG(save_p);
	}

    // done
    Z_Free (savebuffer);
}

//
// M_ReadSaveStrings
//  read the strings from the savegame files
//  and put it in savegamestrings global variable
//
void M_ReadSaveStrings(void)
{
    int     handle;
    int     count;
    int     i;
    char    name[256];

    for (i = 0;i < load_end;i++)
    {
        sprintf(name,savegamename,i);

        handle = open (name, O_RDONLY | 0, 0666);
        if (handle == -1)
        {
            strcpy(&savegamestrings[i][0],EMPTYSTRING);
            LoadGameMenu[i].status = 0;
			savegameinfo[i].lives = -42;
            continue;
        }
        count = read (handle, &savegamestrings[i], SAVESTRINGSIZE);
        close (handle);
        LoadGameMenu[i].status = 1;
		M_ReadSavegameInfo(i);
    }
}

//
// Selected from DOOM menu
//
void M_LoadGame (int choice)
{
// change can't load message to can't load in server mode
    if (netgame && !server)
    {
        M_StartMessage(LOADNET,NULL,MM_NOTHING);
        return;
    }

    M_SetupNextMenu(&LoadDef);
    M_ReadSaveStrings();
}


//===========================================================================
//                                SAVE GAME MENU
//===========================================================================
void M_DrawSave(void);

void M_SaveSelect(int choice);

menuitem_t SaveMenu[]=
{
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'1'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'2'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'3'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'4'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'5'},
    {IT_CALL | IT_NOTHING,"",0, M_SaveSelect,'6'}
};

menu_t  SaveDef =
{
    "M_SAVEG",
    "Save Game",
    load_end,
    &MainDef,
    SaveMenu,
    M_DrawSave,
    80,54,
    0
};



//
// Draw border for the savegame description
//
void M_DrawSaveLoadBorder(int x,int y)
{
    int             i;

    V_DrawScaledPatch (x-8,y+7,0,W_CachePatchName("M_LSLEFT",PU_CACHE));
        
    for (i = 0;i < 24;i++)
    {
        V_DrawScaledPatch (x,y+7,0,W_CachePatchName("M_LSCNTR",PU_CACHE));
        x += 8;
    }
        
    V_DrawScaledPatch (x,y+7,0,W_CachePatchName("M_LSRGHT",PU_CACHE));
}

//
//  M_SaveGame & Cie.
//
void M_DrawSave(void)
{
    int             i;

    M_DrawGenericMenu();

    for (i = 0;i < load_end; i++)
    {
        M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
        V_DrawString(LoadDef.x,LoadDef.y+LINEHEIGHT*i,0,savegamestrings[i]);
    }

    if (saveStringEnter && skullAnimCounter<4) // Tails 11-30-2000
    {
        i = V_StringWidth(savegamestrings[saveSlot]);
        V_DrawString(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,0,"_");
    }

	M_DrawGameStats();
}

//
// M_Responder calls this when user is finished
//
void M_DoSave(int slot)
{
    G_SaveGame (slot,savegamestrings[slot]);
    M_ClearMenus (true);

    // PICK QUICKSAVE SLOT YET?
    if (quickSaveSlot == -2)
        quickSaveSlot = slot;
}

//
// User wants to save. Start string input for M_Responder
//
void M_SaveSelect(int choice)
{
    // we are going to be intercepting all chars
    saveStringEnter = 1;

    saveSlot = choice;
    strcpy(saveOldString,savegamestrings[choice]);
    if (!strcmp(savegamestrings[choice],EMPTYSTRING))
        savegamestrings[choice][0] = 0;
    saveCharIndex = strlen(savegamestrings[choice]);
}

//
// Selected from DOOM menu
//
void M_SaveGame (int choice)
{
    if(demorecording)
    {
        M_StartMessage("You can't save while recording demos\n\nPress a key\n",NULL,MM_NOTHING);
        return;
    }

	if((gamemap >= sstage_start) && (gamemap <= sstage_end))
	{
		M_StartMessage("You can't save while in a special\nstage!\nPress a key\n",NULL,MM_NOTHING);
		return;
	}

	if(!netgame && !multiplayer && players[consoleplayer].lives <= 0)
	{
		M_StartMessage("You can't save a game over!\nPress a key\n",NULL,MM_NOTHING);
		return;
	}

	if(cv_gametype.value != GT_COOP) // Tails 04-25-2001
	{ // Tails 04-25-2001
        M_StartMessage("You can't save non-co-op games!\nPress a key\n",NULL,MM_NOTHING); // Tails 04-25-2001
        return; // Tails 04-25-2001
    } // Tails 04-25-2001

    if (demoplayback || demorecording)
    {
        M_StartMessage(SAVEDEAD,NULL,MM_NOTHING);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (netgame && !server)
    {
        M_StartMessage("You are not the server",NULL,MM_NOTHING);
        return;
    }

    M_SetupNextMenu(&SaveDef);
    M_ReadSaveStrings();
}

//===========================================================================
//                            QuickSAVE & QuickLOAD
//===========================================================================

//
//      M_QuickSave
//
char    tempstring[80];

void M_QuickSaveResponse(int ch)
{
    if (ch == 'y')
    {
        M_DoSave(quickSaveSlot);
// Tails        S_StartSound(NULL,sfx_swtchx);
    }
}

void M_QuickSave(void)
{
    if (demoplayback || demorecording)
    {
//        S_StartSound(NULL,sfx_oof);
        return;
    }

    if (gamestate != GS_LEVEL)
        return;

    if (quickSaveSlot < 0)
    {
        M_StartControlPanel();
        M_ReadSaveStrings();
        M_SetupNextMenu(&SaveDef);
        quickSaveSlot = -2;     // means to pick a slot now
        return;
    }
    sprintf(tempstring,QSPROMPT,savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickSaveResponse,MM_YESNO);
}



//
// M_QuickLoad
//
void M_QuickLoadResponse(int ch)
{
    if (ch == 'y')
    {
        M_LoadSelect(quickSaveSlot);
// Tails        S_StartSound(NULL,sfx_swtchx);
    }
}


void M_QuickLoad(void)
{
    if (netgame)
    {
        M_StartMessage(QLOADNET,NULL,MM_NOTHING);
        return;
    }

    if (quickSaveSlot < 0)
    {
        M_StartMessage(QSAVESPOT,NULL,MM_NOTHING);
        return;
    }
    sprintf(tempstring,QLPROMPT,savegamestrings[quickSaveSlot]);
    M_StartMessage(tempstring,M_QuickLoadResponse,MM_YESNO);
}


//===========================================================================
//                                 END GAME
//===========================================================================

//
// M_EndGame
//
void M_EndGameResponse(int ch)
{
    if (ch != 'y')
        return;

    currentMenu->lastOn = itemOn;
    M_ClearMenus (true);
    COM_BufAddText("exitgame\n");
}

void M_EndGame(int choice)
{
    choice = 0;
    if (demoplayback || demorecording)
    {
//        S_StartSound(NULL,sfx_oof);
        return;
    }
/*
    if (netgame)
    {
        M_StartMessage(NETEND,NULL,MM_NOTHING);
        return;
    }
*/
    M_StartMessage(ENDGAME,M_EndGameResponse,MM_YESNO);
}

//===========================================================================
//                                 Quit Game
//===========================================================================

//
// M_QuitDOOM
//
int     quitsounds2[8] =
{
    sfx_spring, // Tails 11-09-99
    sfx_itemup, // Tails 11-09-99
    sfx_jump, // Tails 11-09-99
    sfx_pop,
    sfx_gloop, // Tails 11-09-99
    sfx_splash, // Tails 11-09-99
    sfx_floush, // Tails 11-09-99
    sfx_chchng // Tails 11-09-99
};

void M_ExitGameResponse(int ch)
{
	if (ch != 'y')
		return;

	COM_BufAddText("exitgame\n");
}

void M_QuitResponse(int ch)
{
    tic_t   time;
    if (ch != 'y')
        return;
    if (!(netgame || cv_debug))
    {
        S_StartSound(NULL,quitsounds2[(gametic>>2)&7]); // Use quitsounds2, not quitsounds Tails 11-09-99

        //added:12-02-98: do that instead of I_WaitVbl which does not work
//        if(!nosound)
//        {
            time = I_GetTime() + TICRATE*3; // Shortened the quit time, used to be 2 seconds Tails 03-26-2001
            while (time > I_GetTime())
			{
				V_DrawScaledPatch (0,0,0,W_CachePatchName("DEMOQUIT",PU_CACHE)); // Demo 3 Quit Screen Tails 06-16-2001
				I_FinishUpdate (); // Update the screen with the image Tails 06-19-2001
			}
//        }
        //I_WaitVBL(105);
    }
    I_Quit ();
}

void M_QuitDOOM(int choice)
{
  // We pick index 0 which is language sensitive,
  //  or one at random, between 1 and maximum number.
  static char s[200];
  sprintf(s,text[DOSY_NUM],text[ QUITMSG_NUM+(gametic%NUM_QUITMESSAGES)]);
  M_StartMessage( s,M_QuitResponse,MM_YESNO);
}


//===========================================================================
//                              Some Draw routine
//===========================================================================

//
//      Menu Functions
//
void M_DrawThermo ( int   x,
                    int   y,
                    consvar_t *cv)
{
    int xx,i;
    int leftlump,rightlump,centerlump[2],cursorlump;
    patch_t *p;

    xx = x;
    leftlump      = W_GetNumForName("M_THERML");
    rightlump     = W_GetNumForName("M_THERMR"); 
    centerlump[0] = W_GetNumForName("M_THERMM"); 
    centerlump[1] = W_GetNumForName("M_THERMM"); 
    cursorlump    = W_GetNumForName("M_THERMO");  

    V_DrawScaledPatch (xx,y,0,p=W_CachePatchNum(leftlump,PU_CACHE));
    xx += SHORT(p->width)-SHORT(p->leftoffset);
    for (i=0;i<16;i++)
    {
        V_DrawScaledPatch (xx,y,0,W_CachePatchNum(centerlump[i & 1],PU_CACHE));
        xx += 8;
    }
    V_DrawScaledPatch (xx,y,0,W_CachePatchNum(rightlump,PU_CACHE));

    xx = (cv->value - cv->PossibleValue[0].value) * (15*8) /
         (cv->PossibleValue[1].value - cv->PossibleValue[0].value);

    V_DrawScaledPatch ((x+8) + xx, y,
                       0,W_CachePatchNum(cursorlump,PU_CACHE));
}


void M_DrawEmptyCell( menu_t*       menu,
                      int           item )
{
    V_DrawScaledPatch (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
                       W_CachePatchName("M_CELL1",PU_CACHE));
}

void M_DrawSelCell ( menu_t*       menu,
                     int           item )
{
    V_DrawScaledPatch (menu->x - 10,        menu->y+item*LINEHEIGHT - 1, 0,
                       W_CachePatchName("M_CELL2",PU_CACHE));
}


//
//  Draw a textbox, like Quake does, because sometimes it's difficult
//  to read the text with all the stuff in the background...
//
//added:06-02-98:
extern int st_borderpatchnum;   //st_stuff.c (for Glide)
void M_DrawTextBox (int x, int y, int width, int lines)
{
    patch_t  *p;
    int      cx, cy;
    int      n;
    int      step,boff; 

    step = 8;
    boff = 8;

    // draw left side
    cx = x;
    cy = y;
    V_DrawScaledPatch (cx, cy, 0, 
        W_CachePatchNum(viewborderlump[BRDR_TL],PU_CACHE));
    cy += boff;
    p = W_CachePatchNum (viewborderlump[BRDR_L],PU_CACHE);
    for (n = 0; n < lines; n++)
    {
        V_DrawScaledPatch (cx, cy, 0, p);
        cy += step;
    }
    V_DrawScaledPatch (cx, cy, 0, 
        W_CachePatchNum (viewborderlump[BRDR_BL],PU_CACHE));

    // draw middle
    V_DrawFlatFill (x+boff, y+boff ,width*step,lines*step,st_borderpatchnum);

    cx += boff;
    cy = y;
    while (width > 0)
    {
        V_DrawScaledPatch (cx, cy, 0, 
            W_CachePatchNum (viewborderlump[BRDR_T],PU_CACHE));

        V_DrawScaledPatch (cx, y+boff+lines*step, 0, 
            W_CachePatchNum (viewborderlump[BRDR_B],PU_CACHE));
        width --;
        cx += step;
    }

    // draw right side
    cy = y;
    V_DrawScaledPatch (cx, cy, 0, 
        W_CachePatchNum (viewborderlump[BRDR_TR],PU_CACHE));
    cy += boff;
    p = W_CachePatchNum (viewborderlump[BRDR_R],PU_CACHE);
    for (n = 0; n < lines; n++)
    {
        V_DrawScaledPatch (cx, cy, 0, p);
        cy += step;
    }
    V_DrawScaledPatch (cx, cy, 0, 
        W_CachePatchNum (viewborderlump[BRDR_BR],PU_CACHE));
}

//==========================================================================
//                        Message is now a (hackeble) Menu
//==========================================================================
void M_DrawMessageMenu(void);

menuitem_t MessageMenu[]=
{
    // TO HACK
    {0 ,NULL , 0, NULL ,0}
};

menu_t MessageDef =
{
    NULL,               // title
    NULL,
    1,                  // # of menu items
    NULL,               // previous menu       (TO HACK)
    MessageMenu,        // menuitem_t ->
    M_DrawMessageMenu,  // drawing routine ->
    0,0,                // x,y                 (TO HACK)
    0                   // lastOn, flags       (TO HACK)
};


void M_StartMessage ( const char*       string,
                      void*             routine,
                      menumessagetype_t itemtype )
{
    int   max,start,i,lines;
#define message MessageDef.menuitems[0].text
    if( message )
        Z_Free( message );
    message = Z_StrDup(string);
    DEBFILE(message);

    M_StartControlPanel(); // can't put menuactiv to true
    MessageDef.prevMenu = currentMenu;
    MessageDef.menuitems[0].text     = message;
    MessageDef.menuitems[0].alphaKey = itemtype;
    switch(itemtype) {
        case MM_NOTHING:
             MessageDef.menuitems[0].status     = IT_MSGHANDLER;
             MessageDef.menuitems[0].itemaction = M_StopMessage;
             break;
        case MM_YESNO:
             MessageDef.menuitems[0].status     = IT_MSGHANDLER;
             MessageDef.menuitems[0].itemaction = routine;
             break;
        case MM_EVENTHANDLER:
             MessageDef.menuitems[0].status     = IT_MSGHANDLER;
             MessageDef.menuitems[0].itemaction = routine;
             break;
    }
    //added:06-02-98: now draw a textbox around the message
    // compute lenght max and the numbers of lines
    max = 0;
    start = 0;
    for (lines=0; *(message+start); lines++)
    {
        for (i = 0;i < (int)strlen(message+start);i++)
        {
            if (*(message+start+i) == '\n')
            {
                if (i > max)
                    max = i;
                start += i+1;
                i = -1; //added:07-02-98:damned!
                break;
            }
        }

        if (i == (int)strlen(message+start))
            start += i;
    }

    MessageDef.x=(BASEVIDWIDTH-8*max-16)/2;
    MessageDef.y=(BASEVIDHEIGHT - M_StringHeight(message))/2;

    MessageDef.lastOn = (lines<<8)+max;

//    M_SetupNextMenu();
    currentMenu = &MessageDef;
    itemOn=0;
}

#define MAXMSGLINELEN 256

void M_DrawMessageMenu(void)
{
    int    y;
    short  i,max;
    char   string[MAXMSGLINELEN];
    int    start,lines;
    char   *msg=currentMenu->menuitems[0].text;

    y=currentMenu->y;
    start = 0;
    lines = currentMenu->lastOn>>8;
    max = (currentMenu->lastOn & 0xFF)*8;
    M_DrawTextBox (currentMenu->x,y-8,(max+7)>>3,lines);

    while(*(msg+start))
    {
        for (i = 0;i < (int)strlen(msg+start);i++)
        {
            if (*(msg+start+i) == '\n')
            {
                memset(string,0,MAXMSGLINELEN);
                if(i >= MAXMSGLINELEN)
                {
                    CONS_Printf("M_DrawMessageMenu: too long segment in %s\n", msg);
                    return;
                }
                else
                {
                    strncpy(string,msg+start,i);
                    start += i+1;
                    i = -1; //added:07-02-98:damned!
                }
                
                break;
            }
        }

        if (i == (int)strlen(msg+start))
        {
            if(i >= MAXMSGLINELEN)
            {
                CONS_Printf("M_DrawMessageMenu: too long segment in %s\n", msg);
                return;
            }
            else
            {
                strcpy(string,msg+start);
                start += i;
            }
        }

        V_DrawString((BASEVIDWIDTH - V_StringWidth(string))/2,y,0,string);
        y += 8; //SHORT(hu_font[0]->height);
    }
}

// default message handler
void M_StopMessage(int choice)
{
    M_SetupNextMenu(MessageDef.prevMenu);
// Tails    S_StartSound(NULL,sfx_swtchx);
}

//==========================================================================
//                        Menu stuffs
//==========================================================================

//added:30-01-98:
//
//  Write a string centered using the hu_font
//
void M_CentreText (int y, char* string)
{
    int x;
    //added:02-02-98:centre on 320, because V_DrawString centers on vid.width...
    x = (BASEVIDWIDTH - V_StringWidth(string))>>1;
    V_DrawString(x,y,0,string);
}


//
// CONTROL PANEL
//

void M_ChangeCvar(int choise)
{
    consvar_t *cv=(consvar_t *)currentMenu->menuitems[itemOn].itemaction;

    if(((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_SLIDER )
     ||((currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_NOMOD  ))
    {
        CV_SetValue(cv,cv->value+choise*2-1);
    }
    else
        if(cv->flags & CV_FLOAT)
        {
            char s[20];
            sprintf(s,"%f",(float)cv->value/FRACUNIT+(choise*2-1)*(1.0/16.0));
            CV_Set(cv,s);
        }
        else
            CV_AddValue(cv,choise*2-1);
}

boolean M_ChangeStringCvar(int choise)
{
    consvar_t *cv=(consvar_t *)currentMenu->menuitems[itemOn].itemaction;
    char buf[255];
    int  len;

    switch( choise ) {
        case KEY_BACKSPACE :
                len=strlen(cv->string);
                if( len>0 )
                {
                    memcpy(buf,cv->string,len);
                    buf[len-1]=0;
                    CV_Set(cv, buf);
                }
                return true;
        default:
            if( choise >= 32 && choise <= 127 )
            {
                len=strlen(cv->string);
                if( len<MAXSTRINGLENGTH-1 )
                {
                    memcpy(buf,cv->string,len);
                    buf[len++] = choise;
                    buf[len] = 0;
                    CV_Set(cv, buf);
                }
                return true;
            }
            break;
    }
    return false;
}

//
// M_Responder
//
boolean M_Responder (event_t* ev)
{
    int             ch;
    int             i;
    static  tic_t   joywait = 0;
    static  tic_t   mousewait = 0;
    static  int     mousey = 0;
    static  int     lasty = 0;
    static  int     mousex = 0;
    static  int     lastx = 0;
    void  (*routine)(int choice);  // for some casting problem

    ch = -1;

	if(gamestate == GS_INTRO || gamestate == GS_INTRO2 || gamestate == GS_CUTSCENE) // Tails 03-02-2002
		return false; // Tails 03-02-2002

    if (ev->type == ev_keydown)
    {
        ch = ev->data1;
        
        // added 5-2-98 remap virtual keys (mouse & joystick buttons)
        switch(ch) {
        case KEY_MOUSE1   : ch=KEY_ENTER;break;
        case KEY_MOUSE1+1 : ch=KEY_BACKSPACE;break;
        case KEY_JOY1     :
        case KEY_JOY1+2   :
        case KEY_JOY1+3   : ch=KEY_ENTER;break;
        case KEY_JOY1+1   : ch=KEY_BACKSPACE;break;
        }
    }
    else if( menuactive )
    {
        if (ev->type == ev_joystick && joywait < I_GetTime())
        {
            if (ev->data3 == -1)
            {
                ch = KEY_UPARROW;
                joywait = I_GetTime() + TICRATE/7;
            }
            else if (ev->data3 == 1)
            {
                ch = KEY_DOWNARROW;
                joywait = I_GetTime() + TICRATE/7;
            }
            
            if (ev->data2 == -1)
            {
                ch = KEY_LEFTARROW;
                joywait = I_GetTime() + TICRATE/17;
            }
            else if (ev->data2 == 1)
            {
                ch = KEY_RIGHTARROW;
                joywait = I_GetTime() + TICRATE/17;
            }
        }
        else
        {
            if (ev->type == ev_mouse && mousewait < I_GetTime())
            {
                mousey += ev->data3;
                if (mousey < lasty-30)
                {
                    ch = KEY_DOWNARROW;
                    mousewait = I_GetTime() + TICRATE/7;
                    mousey = lasty -= 30;
                }
                else if (mousey > lasty+30)
                {
                    ch = KEY_UPARROW;
                    mousewait = I_GetTime() + TICRATE/7;
                    mousey = lasty += 30;
                }
                
                mousex += ev->data2;
                if (mousex < lastx-30)
                {
                    ch = KEY_LEFTARROW;
                    mousewait = I_GetTime() + TICRATE/7;
                    mousex = lastx -= 30;
                }
                else if (mousex > lastx+30)
                {
                    ch = KEY_RIGHTARROW;
                    mousewait = I_GetTime() + TICRATE/7;
                    mousex = lastx += 30;
                }
            }
        }
    }

    if (ch == -1)
        return false;


    // Save Game string input
    if (saveStringEnter)
    {
        switch(ch)
        {
          case KEY_BACKSPACE:
            if (saveCharIndex > 0)
            {
                saveCharIndex--;
                savegamestrings[saveSlot][saveCharIndex] = 0;
            }
            break;

          case KEY_ESCAPE:
            saveStringEnter = 0;
            strcpy(&savegamestrings[saveSlot][0],saveOldString);
            break;

          case KEY_ENTER:
            saveStringEnter = 0;
            if (savegamestrings[saveSlot][0])
                M_DoSave(saveSlot);
            break;

          default:
            ch = toupper(ch);
            if (ch != 32)
                if (ch-HU_FONTSTART < 0 || ch-HU_FONTSTART >= HU_FONTSIZE)
                    break;
            if (ch >= 32 && ch <= 127 &&
                saveCharIndex < SAVESTRINGSIZE-1 &&
                V_StringWidth(savegamestrings[saveSlot]) <
                (SAVESTRINGSIZE-2)*8)
            {
                savegamestrings[saveSlot][saveCharIndex++] = ch;
                savegamestrings[saveSlot][saveCharIndex] = 0;
            }
            break;
        }
        return true;
    }

    if (devparm && ch == KEY_F1)
    {
        COM_BufAddText("screenshot\n");
        return true;
    }


    // F-Keys
    if (!menuactive)
    {
        switch(ch)
        {
//          case KEY_MINUS:         // Screen size down Tails 11-09-99
//            if (automapactive || chat_on || con_destlines)     // DIRTY !!! Tails 11-09-99
//                return false; Tails 11-09-99
//            CV_SetValue (&cv_viewsize, cv_viewsize.value-1); Tails 11-09-99
//            S_StartSound(NULL,sfx_stnmov); Tails 11-09-99
//            return true; Tails 11-09-99

//          case KEY_EQUALS:        // Screen size up Tails 11-09-99
//            if (automapactive || chat_on || con_destlines)     // DIRTY !!! Tails 11-09-99
//                return false; Tails 11-09-99
//            CV_SetValue (&cv_viewsize, cv_viewsize.value+1); Tails 11-09-99
//            S_StartSound(NULL,sfx_stnmov); Tails 11-09-99
//            return true; Tails 11-09-99

          case KEY_F1:            // Help key
            M_StartControlPanel ();
            currentMenu = &ReadDef1;
            itemOn = 0;
            S_StartSound(NULL,sfx_swtchn);
            return true;

          case KEY_F2:            // Save
            M_StartControlPanel();
// Tails            S_StartSound(NULL,sfx_swtchn);
            M_SaveGame(0);
            return true;

          case KEY_F3:            // Load
            M_StartControlPanel();
// Tails            S_StartSound(NULL,sfx_swtchn);
            M_LoadGame(0);
            return true;

          case KEY_F4:            // Sound Volume
            M_StartControlPanel ();
            currentMenu = &SoundDef;
            itemOn = sfx_vol;
// Tails            S_StartSound(NULL,sfx_swtchn);
            return true;

          //added:26-02-98: now F5 calls the Video Menu
          case KEY_F5:
// Tails            S_StartSound(NULL,sfx_swtchn);
            M_StartControlPanel();
            M_SetupNextMenu (&VidModeDef);
            //M_ChangeDetail(0);
            return true;

          case KEY_F6:            // Quicksave
// Tails            S_StartSound(NULL,sfx_swtchn);
            M_QuickSave();
            return true;

          //added:26-02-98: F7 changed to Options menu
          case KEY_F7:            // End game
// Tails            S_StartSound(NULL,sfx_swtchn);
            M_StartControlPanel();
            M_OptionsMenu(0); // Tails 08-18-2002
            //M_EndGame(0);
            return true;

          case KEY_F8:            // Toggle messages
            CV_AddValue(&cv_showmessages,+1);
// Tails            S_StartSound(NULL,sfx_swtchn);
            return true;

          case KEY_F9:            // Quickload
// Tails            S_StartSound(NULL,sfx_swtchn);
            M_QuickLoad();
            return true;

          case KEY_F10:           // Quit DOOM
// Tails            S_StartSound(NULL,sfx_swtchn);
            M_QuitDOOM(0);
            return true;

          //added:10-02-98: the gamma toggle is now also in the Options menu
          case KEY_F11:
// Tails            S_StartSound(NULL,sfx_swtchn);
            CV_AddValue (&cv_usegamma,+1);
            return true;

          // Pop-up menu
          case KEY_ESCAPE:
            M_StartControlPanel ();
// Tails            S_StartSound(NULL,sfx_swtchn);
            return true;
        }
        return false;
    }

    routine = currentMenu->menuitems[itemOn].itemaction;

    //added:30-01-98:
    // Handle menuitems which need a specific key handling
    if(routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_KEYHANDLER )
    {
        routine(ch);
        return true;
    }

    if(currentMenu->menuitems[itemOn].status==IT_MSGHANDLER)
    {
        if(currentMenu->menuitems[itemOn].alphaKey==true)
        {
            if(ch == ' ' || ch == 'n' || ch == 'y' || ch == KEY_ESCAPE)
            {
                if(routine) routine(ch);
                M_StopMessage(0);
                return true;
            }
            return true;
        }
        else
        {
            //added:07-02-98:dirty hak:for the customise controls, I want only
            //      buttons/keys, not moves
            if (ev->type==ev_mouse || ev->type==ev_joystick )
                return true;
            if(routine) routine((int)ev);
            return true;
        }
    }

    // BP: one of the more big hack i have never made
    if( routine && (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR )
    {
        if( (currentMenu->menuitems[itemOn].status & IT_CVARTYPE) == IT_CV_STRING )
        {
            if( M_ChangeStringCvar(ch) )
                return true;
            else
                routine = NULL;
        }
        else
            routine=M_ChangeCvar;
    }
    // Keys usable within menu
    switch (ch)
    {
      case KEY_DOWNARROW:
        do
        {
            if (itemOn+1 > currentMenu->numitems-1)
                itemOn = 0;
            else itemOn++;
        } while((currentMenu->menuitems[itemOn].status & IT_TYPE)==IT_SPACE);
        S_StartSound(NULL,sfx_menu1); // was pstop Tails 11-30-2000
        return true;

      case KEY_UPARROW:
        do
        {
            if (!itemOn)
                itemOn = currentMenu->numitems-1;
            else itemOn--;
        } while((currentMenu->menuitems[itemOn].status & IT_TYPE)==IT_SPACE);
        S_StartSound(NULL,sfx_menu1); // was pstop Tails 11-30-2000
        return true;

      case KEY_LEFTARROW:
        if (  routine &&
            ( (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
            ||(currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR   ))
        {
        S_StartSound(NULL,sfx_menu1); // was stnmov Tails 11-30-2000
            routine(0);
        }
        return true;

      case KEY_RIGHTARROW:
        if ( routine &&
            ( (currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_ARROWS
            ||(currentMenu->menuitems[itemOn].status & IT_TYPE) == IT_CVAR   ))
        {
            S_StartSound(NULL,sfx_menu1); // Tails
            routine(1);
        }
        return true;

      case KEY_ENTER:
        currentMenu->lastOn = itemOn;
        if ( routine )
        {
            switch (currentMenu->menuitems[itemOn].status & IT_TYPE)  {
                case IT_CVAR:
                case IT_ARROWS:
                    routine(1);            // right arrow
                    S_StartSound(NULL,sfx_menu1); // Tails
                    break;
                case IT_CALL:
                    routine(itemOn);
					S_StartSound(NULL,sfx_menu1); // Tails
                    break;
                case IT_SUBMENU:
                    currentMenu->lastOn = itemOn;
                    M_SetupNextMenu((menu_t *)currentMenu->menuitems[itemOn].itemaction);
					S_StartSound(NULL,sfx_menu1); // Tails
                    break;
            }
        }
        return true;

      case KEY_ESCAPE:
        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            currentMenu = currentMenu->prevMenu;
            itemOn = currentMenu->lastOn;
// Tails	    S_StartSound(NULL,sfx_swtchx); // it´s a matter of taste which sound to choose
            //S_StartSound(NULL,sfx_swtchn);
        }
	else
	{
	    M_ClearMenus (true);
	    S_StartSound(NULL,sfx_swtchx);
	}
	
        return true;
      case KEY_BACKSPACE:
        if((currentMenu->menuitems[itemOn].status)==IT_CONTROL)
        {
            S_StartSound(NULL,sfx_stnmov);
            // detach any keys associated to the game control
            G_ClearControlKeys (setupcontrols, currentMenu->menuitems[itemOn].alphaKey);
            return true;
        }
        currentMenu->lastOn = itemOn;
        if (currentMenu->prevMenu)
        {
            currentMenu = currentMenu->prevMenu;
            itemOn = currentMenu->lastOn;
// Tails           S_StartSound(NULL,sfx_swtchn);
        }
        return true;

      default:
        for (i = itemOn+1;i < currentMenu->numitems;i++)
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
                return true;
            }
        for (i = 0;i <= itemOn;i++)
            if (currentMenu->menuitems[i].alphaKey == ch)
            {
                itemOn = i;
                S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
                return true;
            }
        break;

    }

    return true;
}



//
//      Find string height from hu_font chars
//
int M_StringHeight(char* string)
{
    int      i;
    int      h;
    int      height = 8; //(hu_font[0]->height);

    h = height;
    for (i = 0;i < (int)strlen(string);i++)
        if (string[i] == '\n')
            h += height;

    return h;
}


//
// M_Drawer
// Called after the view has been rendered,
// but before it has been blitted.
//
void M_Drawer (void)
{
    if (!menuactive)
        return;

    //added:18-02-98:
    // center the scaled graphics for the menu,
    //  set it 0 again before return!!!
    scaledofs = vid.centerofs;

    // now that's more readable with a faded background (yeah like Quake...)
    V_DrawFadeScreen ();

    if (currentMenu->drawroutine)
        currentMenu->drawroutine();      // call current menu Draw routine

    //added:18-02-98: it should always be 0 for non-menu scaled graphics.
    scaledofs = 0;

}

//
// M_StartControlPanel
//
void M_StartControlPanel (void)
{
    // intro might call this repeatedly
    if (menuactive)
        return;

    menuactive = 1;

	MainMenu[secrets].status = IT_DISABLED;

	// Check for the ??? menu
	if(grade > 0)
		MainMenu[secrets].status = IT_STRING | IT_CALL;

    currentMenu = &MainDef;         // JDC
    itemOn = singleplr;   // JDC

    CON_ToggleOff ();   // dirty hack : move away console
}

//
// M_ClearMenus
//
void M_ClearMenus (boolean callexitmenufunc)
{
    if(!menuactive)
        return;

    if( currentMenu->quitroutine && callexitmenufunc)
    {
        if( !currentMenu->quitroutine())
            return; // we can't quit this menu (also used to set parameter from the menu)
    }

	COM_BufAddText (va("saveconfig \"%s\"\n",configfile)); // Save the config file. I'm sick of crashing the game later
						 // and losing all my changes!
	// I fixed the above. It was saving to CONFIGFILENAME instead of configfile. Graue 11-07-2003

    menuactive = 0;
}


//
// M_SetupNextMenu
//
void M_SetupNextMenu(menu_t *menudef)
{
	int i;

    if( currentMenu->quitroutine )
    {
        if( !currentMenu->quitroutine())
            return; // we can't quit this menu (also used to set parameter from the menu)
    }
    currentMenu = menudef;
    itemOn = currentMenu->lastOn;

    // in case of...
    if (itemOn >= currentMenu->numitems)
        itemOn = currentMenu->numitems - 1;

    // the curent item can be desabled,
    // this code go up until a enabled item found
	if(currentMenu->menuitems[itemOn].status==IT_DISABLED)
	{
		for(i=0; i<currentMenu->numitems; i++)
		{
			if((currentMenu->menuitems[i].status != IT_DISABLED))
			{
				itemOn = i;
				break;
			}
		}
	}
}


//
// M_Ticker
//
void M_Ticker (void)
{
//    if(dedicated)
//	return;
    
    if (--skullAnimCounter <= 0)
    {
        skullAnimCounter = 8 * NEWTICRATERATIO;
    }

    //added:30-01-98:test mode for five seconds
    if( vidm_testingmode>0 )
    {
        // restore the previous video mode
        if (--vidm_testingmode==0)
            setmodeneeded = vidm_previousmode+1;
    }
}


//
// M_Init
//
void M_Init (void)
{
//    if(dedicated)
//	return;
    
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = 1; // Hack Tails 09-18-2002

    skullAnimCounter = 10;

    quickSaveSlot = -1;

    // This is used because DOOM 2 had only one HELP
    //  page. I use CREDIT as second page now, but
    //  kept this hack for educational purposes.
//  MainMenu[readthis] = MainMenu[quitdoom]; // Tails 01-18-2001
    MainDef.numitems--;
    MainDef.y += 8;
    NewDef.prevMenu = &MainDef;
    ReadDef1.drawroutine = M_DrawReadThis1;
    ReadDef1.x = 330;
    ReadDef1.y = 165;
    ReadMenu1[0].itemaction = &MainDef;

    CV_RegisterVar(&cv_skill);
//    CV_RegisterVar(&cv_monsters);
    CV_RegisterVar(&cv_nextmap );
    CV_RegisterVar(&cv_newgametype); // Tails 01-18-2001
    CV_RegisterVar(&cv_serversearch);
}


//======================================================================
// OpenGL specifics options
//======================================================================

#ifdef HWRENDER

void M_DrawOpenGLMenu(void);
void M_OGL_DrawFogMenu(void);
void M_OGL_DrawColorMenu(void);
void M_HandleFogColor (int choice);
void M_HandleScreenDepth(int choice);
menu_t OGL_LightingDef, OGL_FogDef, OGL_ColorDef, OGL_DevDef;

menuitem_t OpenGLOptionsMenu[]=
{
    {IT_STRING | IT_CVAR,0, "Mouse look"          , &cv_grcrappymlook     ,  0},
    {IT_STRING | IT_CVAR,0, "Field of view"       , &cv_grfov             , 10},
    {IT_STRING | IT_CVAR,0, "Quality"             , &cv_scr_depth         , 20},
    {IT_STRING | IT_CVAR,0, "Texture Filter"      , &cv_grfiltermode      , 30},
#ifndef __DJGPP__
    {IT_STRING | IT_CVAR,0,    "Fullscreen"       , &cv_fullscreen    , 40},
#endif
                         
    {IT_SUBMENU|IT_WHITESTRING,0, "Lighting..."       , &OGL_LightingDef   , 60},
    {IT_SUBMENU|IT_WHITESTRING,0, "Fog..."            , &OGL_FogDef        , 70},
    {IT_SUBMENU|IT_WHITESTRING,0, "Gamma..."          , &OGL_ColorDef      , 80},
    {IT_SUBMENU|IT_WHITESTRING,0, "Development..."    , &OGL_DevDef        , 90},
};                                         

menuitem_t OGL_LightingMenu[]=
{
    {IT_STRING | IT_CVAR,0, "Coronas"                 , &cv_grcoronas         ,  0},
    {IT_STRING | IT_CVAR,0, "Coronas size"            , &cv_grcoronasize      , 10},
    {IT_STRING | IT_CVAR,0, "Dynamic lighting"        , &cv_grdynamiclighting , 20},
    {IT_STRING | IT_CVAR,0, "Static lighting"         , &cv_grstaticlighting  , 30},
//    {IT_STRING | IT_CVAR,0, "Monsters' balls lighting", &cv_grmblighting      , 40},
};                                         

menuitem_t OGL_FogMenu[]=
{
    {IT_STRING | IT_CVAR, 0,"Fog"             , &cv_grfog              ,  0},
    {IT_STRING | IT_KEYHANDLER,0, "Fog color" , M_HandleFogColor       , 10},
    {IT_STRING | IT_CVAR, 0,"Fog density"     , &cv_grfogdensity       , 20},
};                                         

menuitem_t OGL_ColorMenu[]=
{
    //{IT_STRING | NOTHING, "Gamma correction", NULL                   ,  0},
    {IT_STRING | IT_CVAR | IT_CV_SLIDER, 0,"red"  , &cv_grgammared     , 10},
    {IT_STRING | IT_CVAR | IT_CV_SLIDER, 0,"green", &cv_grgammagreen   , 20},
    {IT_STRING | IT_CVAR | IT_CV_SLIDER, 0,"blue" , &cv_grgammablue    , 30},
    //{IT_STRING | IT_CVAR | IT_CV_SLIDER, "Constrast", &cv_grcontrast , 50},
};                                         

menuitem_t OGL_DevMenu[]=
{
//    {IT_STRING | IT_CVAR, "Polygon smooth"  , &cv_grpolygonsmooth    ,  0},
//    {IT_STRING | IT_CVAR, 0, "MD2 models"      , &cv_grmd2              , 10},
    {IT_STRING | IT_CVAR, 0, "Translucent walls", &cv_grtranswall       , 20},
};

menu_t  OpenGLOptionDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OpenGLOptionsMenu)/sizeof(menuitem_t),
    &VideoOptionsDef,
    OpenGLOptionsMenu,
    M_DrawOpenGLMenu,
    60,40,
    0
};

menu_t  OGL_LightingDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OGL_LightingMenu)/sizeof(menuitem_t),
    &OpenGLOptionDef,
    OGL_LightingMenu,
    M_DrawGenericMenu,
    60,40,
    0,
};

menu_t  OGL_FogDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OGL_FogMenu)/sizeof(menuitem_t),
    &OpenGLOptionDef,
    OGL_FogMenu,
    M_OGL_DrawFogMenu,
    60,40,
    0,
};

menu_t  OGL_ColorDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OGL_ColorMenu)/sizeof(menuitem_t),
    &OpenGLOptionDef,
    OGL_ColorMenu,
    M_OGL_DrawColorMenu,
    60,40,
    0,
};

menu_t  OGL_DevDef =
{
    "M_OPTTTL",
    "OPTIONS",
    sizeof(OGL_DevMenu)/sizeof(menuitem_t),
    &OpenGLOptionDef,
    OGL_DevMenu,
    M_DrawGenericMenu,
    60,40,
    0,
};


//======================================================================
// M_DrawOpenGLMenu()
//======================================================================
void M_DrawOpenGLMenu(void)
{
    int             mx,my;

    mx = OpenGLOptionDef.x;
    my = OpenGLOptionDef.y;
    M_DrawGenericMenu(); // use generic drawer for cursor, items and title
    V_DrawString(BASEVIDWIDTH-mx-V_StringWidth(cv_scr_depth.string),
                 my+currentMenu->menuitems[2].alphaKey,
                 V_WHITEMAP,
                 cv_scr_depth.string);
}


#define FOG_COLOR_ITEM  1
//======================================================================
// M_OGL_DrawFogMenu()
//======================================================================
void M_OGL_DrawFogMenu(void)
{
    int             mx,my;

    mx = OGL_FogDef.x;
    my = OGL_FogDef.y;
    M_DrawGenericMenu(); // use generic drawer for cursor, items and title
    V_DrawString(BASEVIDWIDTH-mx-V_StringWidth (cv_grfogcolor.string),
                 my+currentMenu->menuitems[FOG_COLOR_ITEM].alphaKey,
                 V_WHITEMAP,
                 cv_grfogcolor.string);
    if (itemOn==FOG_COLOR_ITEM && skullAnimCounter<4) //blink cursor on FOG_COLOR_ITEM if selected
        V_DrawCharacter( BASEVIDWIDTH-mx, my+currentMenu->menuitems[FOG_COLOR_ITEM].alphaKey, '_' | 0x80);
}


//======================================================================
// M_OGL_DrawColorMenu()
//======================================================================
void M_OGL_DrawColorMenu(void)
{
    int             mx,my;

    mx = OGL_ColorDef.x;
    my = OGL_ColorDef.y;
    M_DrawGenericMenu(); // use generic drawer for cursor, items and title
    V_DrawString(mx, my+currentMenu->menuitems[0].alphaKey-10,
                 V_WHITEMAP,"Gamma correction");
}


//======================================================================
// M_OpenGLOption()
//======================================================================
void M_OpenGLOption(int choice)
{
    if (rendermode != render_soft )
        M_SetupNextMenu(&OpenGLOptionDef);
    else
        M_StartMessage("You are in software mode\nYou can't change the options\n",NULL,MM_NOTHING);
}


//======================================================================
// M_HandleFogColor()
//======================================================================
void M_HandleFogColor (int choice)
{
    int      i, l;
    char     temp[8];
    boolean  exitmenu = false;  // exit to previous menu and send name change

    switch( choice )
    {
      case KEY_DOWNARROW:
        S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
        itemOn++;
        break;

      case KEY_UPARROW:
        S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
        itemOn--;
        break;

      case KEY_ESCAPE:
        S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
        exitmenu = true;
        break;

      case KEY_BACKSPACE:
        S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
        strcpy(temp, cv_grfogcolor.string);
        strcpy(cv_grfogcolor.string, "000000");
        l = strlen(temp)-1;
        for (i=0; i<l; i++)
            cv_grfogcolor.string[i+6-l] = temp[i];
        break;

      default:
        if ((choice >= '0' && choice <= '9') ||
            (choice >= 'a' && choice <= 'f') ||
            (choice >= 'A' && choice <= 'F')) {
            S_StartSound(NULL,sfx_menu1); // Tails 11-30-2000
            strcpy(temp, cv_grfogcolor.string);
            strcpy(cv_grfogcolor.string, "000000");
            l = strlen(temp);
            for (i=0; i<l; i++)
                cv_grfogcolor.string[5-i] = temp[l-i];
            cv_grfogcolor.string[5] = choice;
        }
        break;
    }
    if (exitmenu)
    {
        if (currentMenu->prevMenu)
            M_SetupNextMenu (currentMenu->prevMenu);
        else
            M_ClearMenus (true);
    }
}

#endif
