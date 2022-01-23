// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_fab.c,v 1.5 2000/10/21 08:43:30 bpereira Exp $
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
// $Log: p_fab.c,v $
// Revision 1.5  2000/10/21 08:43:30  bpereira
// no message
//
// Revision 1.4  2000/07/01 09:23:49  bpereira
// no message
//
// Revision 1.3  2000/04/24 20:24:38  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      some new action routines, separated from the original doom
//      sources, so that you can include it or remove it easy.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "p_fab.h"
#include "m_random.h"

void Translucency_OnChange(void);

consvar_t cv_translucency  = {"translucency" ,"1",CV_CALL|CV_SAVE,CV_OnOff, Translucency_OnChange};

//
// Action routine, for the ROCKET thing.
// This one adds trails of smoke to the rocket.
// The action pointer of the S_ROCKET state must point here to take effect.
// This routine is based on the Revenant Fireball Tracer code A_Tracer()
//
void A_SmokeTrailer (mobj_t* actor)
{
    mobj_t*     th;

    if (gametic % (4 * NEWTICRATERATIO))
        return;

    // add the smoke behind the rocket
    th = P_SpawnMobj (actor->x-actor->momx,
                      actor->y-actor->momy,
                      actor->z, MT_SMOK);

    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;
    if (th->tics < 1)
        th->tics = 1;
}


static boolean resettrans=false;
//  Set the translucency map for each frame state of mobj
//
void R_SetTrans (statenum_t state1, statenum_t state2, transnum_t transmap)
{
    state_t*   state = &states[state1];

    do
    {
        state->frame &= ~FF_TRANSMASK;
        if(!resettrans)
            state->frame |= (transmap<<FF_TRANSSHIFT);
        state++;
    } while (state1++<state2);
}


//  hack the translucency in the states for a set of standard doom sprites
//
void P_SetTranslucencies (void)
{
    //Fab: lava/slime damage smoke test
    R_SetTrans (S_SMOK1      , S_SMOK5     , tr_transmed);
    R_SetTrans (S_SPLASH1    , 0, tr_transmed);
	R_SetTrans (S_SPLASH2    , 0, tr_transmor);
	R_SetTrans (S_SPLASH3    , 0, tr_transhi);

	R_SetTrans (S_THOK1, 0, tr_transmed); // Thok! mobj Tails 12-05-99

	R_SetTrans (S_FLAME1, S_FLAME4, tr_transfir); // Flame Tails 09-08-2002

	R_SetTrans (S_FOG1, S_FOG14, tr_transmed);

// if higher translucency needed, toy around with the other tr_trans variables

    R_SetTrans (S_BORB1, S_YORB1, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_GORB1, S_KORB1, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_BORB2, S_YORB2, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_GORB2, S_KORB2, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_BORB3, S_YORB3, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_GORB3, S_KORB3, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_BORB4, S_YORB4, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_GORB4, S_KORB4, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_BORB5, S_YORB5, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_GORB5, S_KORB5, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_BORB6, S_YORB6, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_GORB6, S_KORB6, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_BORB7, S_YORB7, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_GORB7, S_KORB7, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_BORB8, S_YORB8, tr_transmed); // Tails shield translucencies 12-30-99
    R_SetTrans (S_GORB8, S_KORB8, tr_transmed); // Tails shield translucencies 12-30-99
	R_SetTrans (S_RORB1, S_RORB8, tr_transmed);

    R_SetTrans (S_SPRK1, S_SPRK2, tr_transfir); // start trans spark tails
    R_SetTrans (S_SPRK3, S_SPRK4, tr_transfir);
    R_SetTrans (S_SPRK5, S_SPRK6, tr_transmed);
    R_SetTrans (S_SPRK7, S_SPRK8, tr_transmed);
    R_SetTrans (S_SPRK9, S_SPRK10, tr_transmor);
    R_SetTrans (S_SPRK11, S_SPRK12, tr_transmor);
    R_SetTrans (S_SPRK13, S_SPRK14, tr_transhi);
    R_SetTrans (S_SPRK15, S_SPRK16, tr_transhi); // end trans spark tails

    R_SetTrans (S_SMALLBUBBLE, S_SMALLBUBBLE1, tr_transmed);
    R_SetTrans (S_MEDIUMBUBBLE, S_MEDIUMBUBBLE1, tr_transmed);
    R_SetTrans (S_LARGEBUBBLE, S_EXTRALARGEBUBBLE, tr_transmed);
    R_SetTrans (S_EXTRALARGEBUBBLE1, 0, tr_transmed);

    R_SetTrans (S_SPLISH1, S_SPLISH2, tr_transmed);
    R_SetTrans (S_SPLISH3, S_SPLISH4, tr_transmed);
    R_SetTrans (S_SPLISH5, S_SPLISH6, tr_transmed);
    R_SetTrans (S_SPLISH7, S_SPLISH8, tr_transmed);
    R_SetTrans (S_SPLISH9, 0, tr_transmed);
    R_SetTrans (S_TOKEN, 0, tr_transmed);
	R_SetTrans (S_RAIN1, 0, tr_transmed);
}

void Translucency_OnChange(void)
{
    if( cv_translucency.value==0 )
        resettrans = true;
    if (!fuzzymode)
        P_SetTranslucencies();
    resettrans = false;
}


// =======================================================================
//                    FUNKY DEATHMATCH COMMANDS
// =======================================================================

void D_AddDeathmatchCommands (void)
{
    // BP:not realy in deathmatch but is just here
    CV_RegisterVar (&cv_translucency);
}
