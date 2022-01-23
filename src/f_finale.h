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
/// \brief Title screen, intro, game evaluation, and credits.
///	Also includes protos for screen wipe functions.

#ifndef __F_FINALE__
#define __F_FINALE__

#include "doomtype.h"
#include "d_event.h"

//
// FINALE
//

// Called by main loop.
boolean F_Responder(event_t* ev);
boolean F_IntroResponder(event_t* ev);
boolean F_CutsceneResponder(event_t* ev);
boolean F_CreditResponder(event_t* ev);

// Called by main loop.
void F_Ticker(void);
void F_DemoEndTicker(void);
void F_IntroTicker(void);
void F_TitleScreenTicker(void);
void F_CutsceneTicker(void);

// Called by main loop.
void F_Drawer(void);
void F_DemoEndDrawer(void);
void F_IntroDrawer(void);
void F_TitleScreenDrawer(void);

void F_GameEvaluationDrawer(void);
void F_StartGameEvaluation(void);
void F_GameEvaluationTicker(void);

void F_CreditTicker(void);
void F_CreditDrawer(void);

void F_StartCustomCutscene(int cutscenenum, boolean precutscene, boolean resetplayer);
void F_CutsceneDrawer(void);
void F_EndCutScene(void);

void F_StartFinale(void);
void F_StartDemoEnd(void);
void F_StartIntro(void);
void F_StartTitleScreen(void);
void F_StartCredits(void);

/** A screen of credits.
  */
typedef struct
{
	char header[32];       ///< Header text.
	byte numnames;         ///< Number of names on this screen.
	char fakenames[8][32]; ///< Nicknames, e.g. Graue, Alam_GBC.
	char realnames[8][32]; ///< Real names.
} credit_t;

/**	\brief the array of creadit of the game and mod
*/
extern credit_t credits[19];

//
// WIPE
//
extern boolean WipeInAction;

void F_WipeStartScreen(void);
void F_WipeEndScreen(int x, int y, int width, int height);
int  F_ScreenWipe(int x, int y, int width, int height, tic_t ticks);

#endif
