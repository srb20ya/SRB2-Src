// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
/// \file
/// \brief SRB2 game states

#ifndef __G_STATE__
#define __G_STATE__

#include "doomtype.h"

// skill levels
typedef enum
{
	sk_baby,
	sk_easy,
	sk_medium,
	sk_hard,
	sk_nightmare,
	sk_insane,
} skill_t;

// the current state of the game
typedef enum
{
	GS_NULL = 0,        // At beginning.
	GS_LEVEL,           // Playing, in a level.
	GS_INTERMISSION,    // Gazing at the intermission screen.
	GS_FINALE,          // Game final animation.
	GS_DEMOSCREEN,      // Watching a demo.
	GS_EVALUATION,      // Evaluation at the end of a game.

	GS_INTRO,           // introduction
	GS_INTRO2,          // introduction 2, for forcing wipes
	GS_CUTSCENE,        // custom cutscene
	GS_DEMOEND,         // demo end screen
	GS_TITLESCREEN,     // title screen
	GS_CREDITS,         // credit sequence
	GS_DEDICATEDSERVER, // new state for dedicated server
	GS_WAITINGPLAYERS   // waiting for players in a net game
} gamestate_t;

typedef enum
{
	ga_nothing,
	ga_completed,
	ga_worlddone,
} gameaction_t;

extern gamestate_t gamestate;
extern gameaction_t gameaction;

#endif //__G_STATE__
