// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_englsh.h,v 1.6 2001/05/16 21:21:14 bpereira Exp $
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
// $Log: d_englsh.h,v $
// Revision 1.6  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.5  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.4  2000/04/16 18:38:06  bpereira
// no message
//
// Revision 1.3  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Printed strings for translation.
//      English language support (default).
//
//-----------------------------------------------------------------------------

#ifndef __D_TEXT__
#define __D_TEXT__

typedef enum {
 D_DEVSTR_NUM,
 D_CDROM_NUM,
 PRESSKEY_NUM,
 PRESSYN_NUM,
 LOADNET_NUM,
 QLOADNET_NUM,
 QSAVESPOT_NUM,
 SAVEDEAD_NUM,
 QSPROMPT_NUM,
 QLPROMPT_NUM,
 NEWGAME_NUM,
 NIGHTMARE_NUM,
 MSGOFF_NUM,
 MSGON_NUM,
 NETEND_NUM,
 ENDGAME_NUM,
 DOSY_NUM,
 GAMMALVL0_NUM,
 GAMMALVL1_NUM,
 GAMMALVL2_NUM,
 GAMMALVL3_NUM,
 GAMMALVL4_NUM,
 EMPTYSTRING_NUM,
 GGSAVED_NUM,
 HUSTR_MSGU_NUM,
 HUSTR_CHATMACRO1_NUM,
 HUSTR_CHATMACRO2_NUM,
 HUSTR_CHATMACRO3_NUM,
 HUSTR_CHATMACRO4_NUM,
 HUSTR_CHATMACRO5_NUM,
 HUSTR_CHATMACRO6_NUM,
 HUSTR_CHATMACRO7_NUM,
 HUSTR_CHATMACRO8_NUM,
 HUSTR_CHATMACRO9_NUM,
 HUSTR_CHATMACRO0_NUM,
 STSTR_MUS_NUM,
 STSTR_NOMUS_NUM,
 STSTR_CLEV_NUM,
 E0TEXT_NUM,
 E1TEXT_NUM,
 E2TEXT_NUM,
 E3TEXT_NUM,
 E4TEXT_NUM,
 C1TEXT_NUM,
 C2TEXT_NUM,
 C3TEXT_NUM,
 C4TEXT_NUM,
 C5TEXT_NUM,
 C6TEXT_NUM,
 T1TEXT_NUM,
 T2TEXT_NUM,
 T3TEXT_NUM,
 T4TEXT_NUM,
 T5TEXT_NUM,

 QUITMSG_NUM,
 QUITMSG1_NUM,
 QUITMSG2_NUM,
 QUITMSG3_NUM,
 QUITMSG4_NUM,
 QUITMSG5_NUM,
 QUITMSG6_NUM,
 QUITMSG7_NUM,

 QUIT2MSG_NUM,
 QUIT2MSG1_NUM,
 QUIT2MSG2_NUM,
 QUIT2MSG3_NUM,
 QUIT2MSG4_NUM,
 QUIT2MSG5_NUM,
 QUIT2MSG6_NUM,

 MODIFIED_NUM,
 COMERCIAL_NUM,

 M_LOAD_NUM,
 Z_INIT_NUM,
 W_INIT_NUM,
 M_INIT_NUM,
 R_INIT_NUM,
 P_INIT_NUM,
 I_INIT_NUM,
 D_CHECKNET_NUM,
 S_SETSOUND_NUM,
 HU_INIT_NUM,
 ST_INIT_NUM,

 DOOM2WAD_NUM,

 CDROM_SAVEI_NUM,
 NORM_SAVEI_NUM,

 SPECIALDEHACKED,

 DOOM2TITLE_NUM = SPECIALDEHACKED,

 NUMTEXT
} text_enum;

extern char *text[];


//
//      Printed strings for translation
//

//
// D_Main.C
//
#define D_DEVSTR          text[D_DEVSTR_NUM]
#define D_CDROM           text[D_CDROM_NUM]

//
//      M_Menu.C
//
#define PRESSKEY          text[PRESSKEY_NUM]
#define PRESSYN           text[PRESSYN_NUM]
#define LOADNET           text[LOADNET_NUM]
#define QLOADNET          text[QLOADNET_NUM]
#define QSAVESPOT         text[QSAVESPOT_NUM]
#define SAVEDEAD          text[SAVEDEAD_NUM]
#define QSPROMPT          text[QSPROMPT_NUM]
#define QLPROMPT          text[QLPROMPT_NUM]
#define NEWGAME           text[NEWGAME_NUM]
#define NIGHTMARE         text[NIGHTMARE_NUM]
#define MSGOFF            text[MSGOFF_NUM]
#define MSGON             text[MSGON_NUM]
#define NETEND            text[NETEND_NUM]
#define ENDGAME           text[ENDGAME_NUM]
#define DOSY              text[DOSY_NUM]
#define GAMMALVL0         text[GAMMALVL0_NUM]
#define GAMMALVL1         text[GAMMALVL1_NUM]
#define GAMMALVL2         text[GAMMALVL2_NUM]
#define GAMMALVL3         text[GAMMALVL3_NUM]
#define GAMMALVL4         text[GAMMALVL4_NUM]
#define EMPTYSTRING       text[EMPTYSTRING_NUM]

//
//      G_game.C
//
#define GGSAVED           text[GGSAVED_NUM]

//
//      HU_stuff.C
//
#define HUSTR_MSGU        text[HUSTR_MSGU_NUM]
#define HUSTR_CHATMACRO1  text[HUSTR_CHATMACRO1_NUM]
#define HUSTR_CHATMACRO2  text[HUSTR_CHATMACRO2_NUM]
#define HUSTR_CHATMACRO3  text[HUSTR_CHATMACRO3_NUM]
#define HUSTR_CHATMACRO4  text[HUSTR_CHATMACRO4_NUM]
#define HUSTR_CHATMACRO5  text[HUSTR_CHATMACRO5_NUM]
#define HUSTR_CHATMACRO6  text[HUSTR_CHATMACRO6_NUM]
#define HUSTR_CHATMACRO7  text[HUSTR_CHATMACRO7_NUM]
#define HUSTR_CHATMACRO8  text[HUSTR_CHATMACRO8_NUM]
#define HUSTR_CHATMACRO9  text[HUSTR_CHATMACRO9_NUM]
#define HUSTR_CHATMACRO0  text[HUSTR_CHATMACRO0_NUM]

// The following should NOT be changed unless it seems
// just AWFULLY necessary

#define HUSTR_KEYGREEN  'g'
#define HUSTR_KEYINDIGO 'i'
#define HUSTR_KEYBROWN  'b'
#define HUSTR_KEYRED    'r'

//
//      ST_stuff.C
//

#define STSTR_MUS          text[STSTR_MUS_NUM]
#define STSTR_NOMUS        text[STSTR_NOMUS_NUM]
#define STSTR_CLEV         text[STSTR_CLEV_NUM]

//
//      F_Finale.C
//
#define E0TEXT             text[E0TEXT_NUM]
#define E1TEXT             text[E1TEXT_NUM]
#define E2TEXT             text[E2TEXT_NUM]
#define E3TEXT             text[E3TEXT_NUM]
#define E4TEXT             text[E4TEXT_NUM]

// after level 6] put this:
#define C1TEXT             text[C1TEXT_NUM]

// After level 11] put this:
#define C2TEXT             text[C2TEXT_NUM]

// After level 20] put this:
#define C3TEXT             text[C3TEXT_NUM]

// After level 29] put this:
#define C4TEXT             text[C4TEXT_NUM]

// Before level 31] put this:
#define C5TEXT             text[C5TEXT_NUM]

// Before level 32] put this:

#define C6TEXT             text[C6TEXT_NUM]

#define T1TEXT             text[T1TEXT_NUM]
#define T2TEXT             text[T2TEXT_NUM]
#define T3TEXT             text[T3TEXT_NUM]
#define T4TEXT             text[T4TEXT_NUM]
#define T5TEXT             text[T5TEXT_NUM]
#define T6TEXT             text[T6TEXT_NUM]

#endif
