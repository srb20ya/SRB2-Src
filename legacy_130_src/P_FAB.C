// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_fab.c,v 1.4 2000/07/01 09:23:49 bpereira Exp $
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

    if (gametic & 3)
        return;

    // spawn a puff of smoke behind the rocket
    if (demoversion<125 && // rocket trails spawnpuff from v1.11 to v1.24
        demoversion>=111) // skull trails since v1.25
        P_SpawnPuff (actor->x, actor->y, actor->z);

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

    //revenant fireball
    R_SetTrans (S_TRACER    , S_TRACER2    , tr_transfir);
    R_SetTrans (S_TRACEEXP1 , S_TRACEEXP3  , tr_transmed);
                                           
    //rev. fireball. smoke trail           
    R_SetTrans (S_SMOKE1    , S_SMOKE5     , tr_transmed);
                                           
    //imp fireball                         
    R_SetTrans (S_TBALL1    , S_TBALL2     , tr_transfir);
    R_SetTrans (S_TBALLX1   , S_TBALLX3    , tr_transmed);
                                           
    //archvile attack                      
    R_SetTrans (S_FIRE1     , S_FIRE30     , tr_transfir);
                                           
    //bfg ball                             
    R_SetTrans (S_BFGSHOT   , S_BFGSHOT2   , tr_transfir);
    R_SetTrans (S_BFGLAND   , S_BFGLAND3   , tr_transmed);
    R_SetTrans (S_BFGLAND4  , S_BFGLAND6   , tr_transmor);
    R_SetTrans (S_BFGEXP    , 0            , tr_transmed);
    R_SetTrans (S_BFGEXP2   , S_BFGEXP4    , tr_transmor);
                                           
    //plasma bullet                        
//    R_SetTrans (S_PLASBALL  , S_PLASBALL2  , tr_transfir);
//    R_SetTrans (S_PLASEXP   , S_PLASEXP2   , tr_transmed);
//    R_SetTrans (S_PLASEXP3  , S_PLASEXP5   , tr_transmor);
                                           
    //bullet puff                          
    R_SetTrans (S_PUFF1     , S_PUFF4      , tr_transmor);
                                           
    //teleport fog                         
    R_SetTrans (S_TFOG      , S_TFOG5      , tr_transmed);
    R_SetTrans (S_TFOG6     , S_TFOG10     , tr_transmor);
                                           
    //respawn item fog                     
    R_SetTrans (S_IFOG      , S_IFOG5      , tr_transmed);
                                           
    //mobjinfo[MT_EMMY].flags |= MF_SHADOW;         //emerald transies tails
    R_SetTrans (S_TOKEN, 0, tr_transfir); // emerald transies tails
    //mobjinfo[MT_INV].flags |= MF_SHADOW;            //invulnerability
//    R_SetTrans (S_PINV, S_PINV4, tr_transmed);
                            
    R_SetTrans (S_GREENTORCH, S_REDTORCH4  , tr_transfx1); // blue torch
    R_SetTrans (S_RTORCHSHRT4, 0, tr_transfx1); // short blue torch

    // flaming barrel !!
//    R_SetTrans (S_BBAR1, S_BBAR3, tr_transfx1); Don't make the seaweed trans!! Tails 10-31-99

    //lost soul
//    R_SetTrans (S_SKULL_STND, S_SKULL_DIE6 , tr_transfx1);
    //baron shot
    R_SetTrans (S_BRBALL1   , S_BRBALL2    , tr_transfir);
     R_SetTrans (S_BRBALLX1 , S_BRBALLX3   , tr_transmed);
    //demon spawnfire
    R_SetTrans (S_SPAWNFIRE1, S_SPAWNFIRE3 , tr_transfir);
    R_SetTrans (S_SPAWNFIRE4, S_SPAWNFIRE8 , tr_transmed);
    //caco fireball
    R_SetTrans (S_RBALL1    , S_RBALL2     , tr_transfir);
    R_SetTrans (S_RBALLX1   , S_RBALLX3    , tr_transmed);

    //arachno shot
    R_SetTrans (S_ARACH_PLAZ, S_ARACH_PLAZ2, tr_transfir);
    R_SetTrans (S_ARACH_PLEX, S_ARACH_PLEX2, tr_transmed);
    R_SetTrans (S_ARACH_PLEX3,S_ARACH_PLEX4, tr_transmor);
    R_SetTrans (S_ARACH_PLEX5,            0, tr_transhi);

    //blood puffs!
    //R_SetTrans (S_BLOOD1   ,            0, tr_transmed);
    //R_SetTrans (S_BLOOD2   , S_BLOOD3    , tr_transmor);

    //eye in symbol
    R_SetTrans (S_EVILEYE    , S_EVILEYE4  , tr_transmed);
                                          
    //mancubus fireball
    R_SetTrans (S_FATSHOT1   , S_FATSHOT2  , tr_transfir);
    R_SetTrans (S_FATSHOTX1  , S_FATSHOTX3 , tr_transmed);

    // rockets explosion
    R_SetTrans (S_EXPLODE1   , S_EXPLODE2  , tr_transfir);
    R_SetTrans (S_EXPLODE3   ,            0, tr_transmed);

    //Fab: lava/slime damage smoke test
    R_SetTrans (S_SMOK1      , S_SMOK5     , tr_transmed);
    R_SetTrans (S_SPLASH1    , S_SPLASH3   , tr_transmor);

	R_SetTrans (S_GTHOK1, S_GRTHOK1, tr_transmed); // Thok! mobj Tails 12-05-99
	R_SetTrans (S_PCTHOK1, S_DRTHOK1, tr_transmed); // Thok! mobj Tails 12-05-99
	R_SetTrans (S_STHOK1, S_OTHOK1, tr_transmed); // Thok! mobj Tails 12-05-99
	R_SetTrans (S_RTHOK1, S_BTHOK1, tr_transmed); // Thok! mobj Tails 12-05-99
	R_SetTrans (S_PTHOK1, S_DBTHOK1, tr_transmed); // Thok! mobj Tails 12-05-99
	R_SetTrans (S_BGTHOK1, 0, tr_transmed); // Thok! mobj Tails 12-05-99

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

    R_SetTrans (S_SPRK1, S_SPRK2, tr_transfir); // start trans spark tails
    R_SetTrans (S_SPRK3, S_SPRK4, tr_transfir);
    R_SetTrans (S_SPRK5, S_SPRK6, tr_transmed);
    R_SetTrans (S_SPRK7, S_SPRK8, tr_transmed);
    R_SetTrans (S_SPRK9, S_SPRK10, tr_transmor);
    R_SetTrans (S_SPRK11, S_SPRK12, tr_transmor);
    R_SetTrans (S_SPRK13, S_SPRK14, tr_transhi);
    R_SetTrans (S_SPRK15, S_SPRK16, tr_transhi); // end trans spark tails
/*
    R_SetTrans (S_IVSP1, S_IVSP2, tr_transmed); // start invincibility spark tails
    R_SetTrans (S_IVSP3, S_IVSP4, tr_transmed);
    R_SetTrans (S_IVSP5, S_IVSP6, tr_transmed);
    R_SetTrans (S_IVSP7, S_IVSP8, tr_transmed);
    R_SetTrans (S_IVSP9, S_IVSP10, tr_transmed);
    R_SetTrans (S_IVSP11, S_IVSP12, tr_transmed);
    R_SetTrans (S_IVSP13, S_IVSP14, tr_transmed);
    R_SetTrans (S_IVSP15, S_IVSP16, tr_transmed);
    R_SetTrans (S_IVSP17, S_IVSP18, tr_transmed);
    R_SetTrans (S_IVSP19, S_IVSP20, tr_transmed);
    R_SetTrans (S_IVSP21, S_IVSP22, tr_transmed);
    R_SetTrans (S_IVSP23, S_IVSP24, tr_transmed);
    R_SetTrans (S_IVSP25, S_IVSP26, tr_transmed);
    R_SetTrans (S_IVSP27, S_IVSP28, tr_transmed);
    R_SetTrans (S_IVSP29, 0, tr_transmed);
    R_SetTrans (S_IVSQ1, S_IVSQ2, tr_transmed);
    R_SetTrans (S_IVSQ3, 0, tr_transmed); // end invincibility spark tails
*/
    R_SetTrans (S_SMALLBUBBLE, S_SMALLBUBBLE1, tr_transmed);
    R_SetTrans (S_MEDIUMBUBBLE, S_MEDIUMBUBBLE1, tr_transmed);
    R_SetTrans (S_LARGEBUBBLE, S_EXTRALARGEBUBBLE, tr_transmed);
    R_SetTrans (S_EXTRALARGEBUBBLE1, 0, tr_transmed);

    R_SetTrans (S_SPLISH1, S_SPLISH2, tr_transmed);
    R_SetTrans (S_SPLISH3, S_SPLISH4, tr_transmed);
    R_SetTrans (S_SPLISH5, S_SPLISH6, tr_transmed);
    R_SetTrans (S_SPLISH7, S_SPLISH8, tr_transmed);
    R_SetTrans (S_SPLISH9, 0, tr_transmed);
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

void BloodTime_OnChange (void);

CV_PossibleValue_t bloodtime_cons_t[]={{1,"MIN"},{3600,"MAX"},{0,NULL}};
// how much tics to last for the last (third) frame of blood (S_BLOODx)
consvar_t cv_bloodtime = {"bloodtime","20",CV_NETVAR|CV_CALL|CV_SAVE,bloodtime_cons_t,BloodTime_OnChange};

// Called when var. 'bloodtime' is changed : set the blood states duration
//
void BloodTime_OnChange (void)
{
    states[S_BLOOD1].tics = 8;
    states[S_BLOOD2].tics = 8;
    states[S_BLOOD3].tics = (cv_bloodtime.value*TICRATE) - 16;

	// Don't make this show at startup! Tails 03-25-2001
//    CONS_Printf ("blood lasts for %d seconds\n", cv_bloodtime.value);
}


void D_AddDeathmatchCommands (void)
{
    CV_RegisterVar (&cv_solidcorpse);                 //p_enemy

    CV_RegisterVar (&cv_bloodtime);

    // BP:not realy in deathmatch but is just here
    CV_RegisterVar (&cv_translucency);
}
