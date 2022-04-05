// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright (C) 1993-1996 by id Software, Inc.
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
//-----------------------------------------------------------------------------
/// \file
/// \brief Menu widget stuff, skill selection and such

#ifndef __M_MENU__
#define __M_MENU__

#include "d_event.h"
#include "command.h"

//
// MENUS
//
// Called by main loop,
// saves config file and calls I_Quit when user exits.
// Even when the menu is not displayed,
// this can resize the view and change game parameters.
// Does all the real work of the menu interaction.
boolean M_Responder(event_t *ev);

// Called by main loop, only used for menu (skull cursor) animation.
void M_Ticker(void);

// Called by main loop, draws the menus directly into the screen buffer.
void M_Drawer(void);

// Called by D_SRB2Main, loads the config file.
void M_Init(void);

// Called by intro code to force menu up upon a keypress,
// does nothing if menu is already up.
void M_StartControlPanel(void);

// Draws a box with a texture inside as background for messages
void M_DrawTextBox(int x, int y, int width, int lines);
// show or hide the setup for player 2 (called at splitscreen change)
void M_SwitchSplitscreen(void);

// the function to show a message box typing with the string inside
// string must be static (not in the stack)
// routine is a function taking a int in parameter
typedef enum
{
	MM_NOTHING = 0, // is just displayed until the user do someting
	MM_YESNO,       // routine is called with only 'y' or 'n' in param
	MM_EVENTHANDLER // the same of above but without 'y' or 'n' restriction
	                // and routine is void routine(event_t *) (ex: set control)
} menumessagetype_t;
void M_StartMessage(const char* string, void* routine, menumessagetype_t itemtype);

// Called by linux_x/i_video_xshm.c
void M_QuitResponse(int ch);


// flags for items in the menu
// menu handle (what we do when key is pressed
#define IT_TYPE             14     // (2+4+8)
#define IT_CALL              0     // call the function
#define IT_ARROWS            2     // call function with 0 for left arrow and 1 for right arrow in param
#define IT_KEYHANDLER        4     // call with the key in param
#define IT_SUBMENU           6     // go to sub menu
#define IT_CVAR              8     // hangdle as a cvar
#define IT_SPACE            10     // no handling
#define IT_MSGHANDLER       12     // same as key but with event and sometime can handle y/n key (special for message

#define IT_DISPLAY  (48+64+128)    // 16+32+64
#define IT_NOTHING           0     // space
#define IT_PATCH            16     // a patch or a string with big font
#define IT_STRING           32     // little string (spaced with 10)
#define IT_WHITESTRING      48     // little string in white
#define IT_DYBIGSPACE       64     // same as noting
#define IT_DYLITLSPACE  (16+64)    // little space
#define IT_STRING2      (32+64)    // a simple string
#define IT_GRAYPATCH    (16+32+64) // grayed patch or big font string
#define IT_BIGSLIDER     (128)     // volume sound use this
#define IT_CENTER       (2048)     // if IT_PATCH, center it on screen

//consvar specific
#define IT_CVARTYPE   (256+512+1024)
#define IT_CV_NORMAL         0
#define IT_CV_SLIDER       256
#define IT_CV_STRING       512
#define IT_CV_NOPRINT (256+512)
#define IT_CV_NOMOD       1024

// in short for some common use
#define IT_BIGSPACE    (IT_SPACE  +IT_DYBIGSPACE)
#define IT_LITLSPACE   (IT_SPACE  +IT_DYLITLSPACE)
#define IT_CONTROL     (IT_STRING2+IT_CALL)
#define IT_CVARMAX     (IT_CVAR   +IT_CV_NOMOD)
#define IT_DISABLED    (IT_SPACE  +IT_GRAYPATCH)

typedef union
{
	struct menu_s *submenu;      // IT_SUBMENU
	consvar_t *cvar;             // IT_CVAR
	void (*routine)(int choice); // IT_CALL, IT_KEYHANDLER, IT_ARROWS
} itemaction_t;

//
// MENU TYPEDEFS
//
typedef struct menuitem_s
{
	// show IT_xxx
	short status;

	const char* patch;
	const char* text; // used when FONTBxx lump is found

// FIXME: should be itemaction_t
	void* itemaction;

	// hotkey in menu or y of the item
	byte alphaKey;
} menuitem_t;

extern menuitem_t PlayerMenu[];

typedef struct menu_s
{
	const char      *menutitlepic;
	const char      *menutitle;             // title as string for display with fontb if present
	short           numitems;               // # of menu items
	struct menu_s*  prevMenu;               // previous menu
	menuitem_t*     menuitems;              // menu items
	void            (*drawroutine)(void);   // draw routine
	short           x;
	short           y;                      // x,y of menu
	short           lastOn;                 // last item user was on in menu
	boolean         (*quitroutine)(void);   // called before quit a menu return true if we can
} menu_t;

void M_SetupNextMenu(menu_t *menudef);
void M_ClearMenus(boolean callexitmenufunc);
void M_ExitGameResponse(int ch);
void M_EndGame(int choice);
void M_DrawGenericMenu(void);
extern menu_t* currentMenu;
extern menu_t PlayerDef;

// Stuff for customizing the player select screen
typedef struct
{
	char info[255];
	char picname[9];
	char text[64];
	char skinname[16];
} description_t;

extern description_t description[15];

extern consvar_t cv_newgametype, cv_nextmap;
extern CV_PossibleValue_t gametype_cons_t[], skill_cons_t[];

extern int startmap, skillnum;
extern boolean StartSplitScreenGame;
#endif
