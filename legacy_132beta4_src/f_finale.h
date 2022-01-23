// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: f_finale.h,v 1.2 2000/02/27 00:42:10 hurdler Exp $
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
// $Log: f_finale.h,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//
//    
//-----------------------------------------------------------------------------


#ifndef __F_FINALE__
#define __F_FINALE__


#include "doomtype.h"
#include "d_event.h"
//
// FINALE
//

// Called by main loop.
boolean F_Responder (event_t* ev);
boolean F_IntroResponder (event_t* ev); // Tails 02-17-2002
boolean F_CreditResponder (event_t* ev);

// Called by main loop.
void F_Ticker (void);
void F_DemoEndTicker (void); // Tails 09-01-2002
void F_IntroTicker (void); // Tails 02-15-2002
void F_TitleScreenTicker(void); // Tails 11-30-2002
void F_CutsceneTicker(void);

// Called by main loop.
void F_Drawer (void);
void F_DemoEndDrawer (void); // Tails 09-01-2002
void F_IntroDrawer (void); // Tails 02-15-2002
void F_TitleScreenDrawer (void); // Tails 11-30-2002

void F_GameEvaluationDrawer(void);
void F_StartGameEvaluation(void);
void F_GameEvaluationTicker(void);

void F_CreditTicker(void);
void F_CreditDrawer(void);

void F_StartCustomCutscene(int cutnum);
void F_CutsceneDrawer(void);
void F_EndCutScene(void);

void F_StartFinale (void);
void F_StartDemoEnd (void); // Tails 09-01-2002
void F_StartIntro (void); // Tails 02-15-2002
void F_StartTitleScreen(void); // Tails 11-30-2002

typedef struct
{
	char header[32];
	byte numnames;
	char fakenames[8][32];
	char realnames[8][32];
} credit_t;

credit_t credits[19];

#endif
