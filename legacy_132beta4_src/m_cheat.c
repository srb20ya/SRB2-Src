// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: m_cheat.c,v 1.5 2001/08/20 21:37:34 hurdler Exp $
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
// $Log: m_cheat.c,v $
// Revision 1.5  2001/08/20 21:37:34  hurdler
// fix palette in splitscreen + hardware mode
//
// Revision 1.4  2001/02/10 12:27:14  bpereira
// no message
//
// Revision 1.3  2000/11/02 17:50:07  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Cheat sequence checking.
//
//-----------------------------------------------------------------------------
 

#include "doomdef.h"
#include "dstrings.h"

#include "am_map.h"
#include "m_cheat.h"
#include "g_game.h"

#include "r_local.h"
#include "p_local.h"
#include "p_inter.h"

#include "m_cheat.h"

#include "i_sound.h" // for I_PlayCD()
#include "s_sound.h"
#include "v_video.h"
#include "st_stuff.h"
#include "w_wad.h"

// ==========================================================================
//                             CHEAT Structures
// ==========================================================================

byte   cheat_mus_seq[] =
{
    SCRAMBLE('t'), SCRAMBLE('u'), SCRAMBLE('n'), SCRAMBLE('e'), SCRAMBLE('s'), 1, 0, 0, 0xff
};

//Fab:19-07-98: idcd xx : change cd track
byte   cheat_cd_seq[] =
{
    0xb2, 0x26, 0xe2, 0x26, 1, 0, 0, 0xff
};

unsigned char   cheat_choppers_seq[] = // Demo Tails
{
    SCRAMBLE('b'), SCRAMBLE('e'), SCRAMBLE('e'), SCRAMBLE('d'), SCRAMBLE('e'), SCRAMBLE('e'), 0xff // id...
};

byte   cheat_god_seq[] =
{
    SCRAMBLE('h'), SCRAMBLE('e'), SCRAMBLE('l'), SCRAMBLE('e'), SCRAMBLE('n'), 0xff  // HELEN - Johnny's love! Tails
};

unsigned char   cheat_ammo_seq[] = // Demo Tails
{
    SCRAMBLE('f'), SCRAMBLE('i'), SCRAMBLE('s'), SCRAMBLE('h'), SCRAMBLE('b'), SCRAMBLE('a'), SCRAMBLE('k'), SCRAMBLE('e'), 0xff  // idkfa
};

byte   cheat_ammonokey_seq[] =
{
    0xb2, 0x26, 0x66, 0xa2, 0xff        // idfa
};


// Smashing Pumpkins Into Small Pieces Of Putrid Debris.
byte   cheat_noclip_seq[] =
{
    0xb2, 0x26, 0xea, 0x2a, 0xb2,       // idspispopd
    0xea, 0x2a, 0xf6, 0x2a, 0x26, 0xff
};

//
byte   cheat_commercial_noclip_seq[] =
{
    SCRAMBLE('d'), SCRAMBLE('e'), SCRAMBLE('v'), SCRAMBLE('m'), SCRAMBLE('o'), SCRAMBLE('d'), SCRAMBLE('e'), 0xff // id...
};

//added:28-02-98: new cheat to fly around levels using jump !!
byte   cheat_fly_around_seq[] =
{
    0xb2, 0x26, SCRAMBLE('f'), SCRAMBLE('l'), SCRAMBLE('y'), 0xff  // idfly
};



byte   cheat_powerup_seq[7][10] =
{
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x6e, 0xff },     // beholdv
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xea, 0xff },     // beholds
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xb2, 0xff },     // beholdi
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x6a, 0xff },     // beholdr
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xa2, 0xff },     // beholda
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0x36, 0xff },     // beholdl
    { 0xb2, 0x26, 0x62, 0xa6, 0x32, 0xf6, 0x36, 0x26, 0xff }            // behold
};


byte   cheat_clev_seq[] =
{
    0xb2, 0x26,  0xe2, 0x36, 0xa6, 0x6e, 1, 0, 0, 0xff  // idclev
};


// my position cheat
byte   cheat_mypos_seq[] =
{
    0xb2, 0x26, 0xb6, 0xba, 0x2a, 0xf6, 0xea, 0xff      // idmypos
};

byte cheat_amap_seq[] = { 0xb2, 0x26, 0x26, 0x2e, 0xff };
cheatseq_t cheat_amap = { cheat_amap_seq, 0 };

// Now what?
cheatseq_t      cheat_mus = { cheat_mus_seq, 0 };
cheatseq_t      cheat_cd = { cheat_cd_seq, 0 };
cheatseq_t      cheat_god = { cheat_god_seq, 0 };
cheatseq_t      cheat_ammo = { cheat_ammo_seq, 0 };
cheatseq_t      cheat_ammonokey = { cheat_ammonokey_seq, 0 };
cheatseq_t      cheat_noclip = { cheat_noclip_seq, 0 };
cheatseq_t      cheat_commercial_noclip = { cheat_commercial_noclip_seq, 0 };

//added:28-02-98:
cheatseq_t      cheat_fly_around = { cheat_fly_around_seq, 0 };

cheatseq_t      cheat_powerup[7] =
{
    { cheat_powerup_seq[0], 0 },
    { cheat_powerup_seq[1], 0 },
    { cheat_powerup_seq[2], 0 },
    { cheat_powerup_seq[3], 0 },
    { cheat_powerup_seq[4], 0 },
    { cheat_powerup_seq[5], 0 },
    { cheat_powerup_seq[6], 0 }
};

cheatseq_t      cheat_choppers = { cheat_choppers_seq, 0 };
cheatseq_t      cheat_clev = { cheat_clev_seq, 0 };
cheatseq_t      cheat_mypos = { cheat_mypos_seq, 0 };

// ==========================================================================
//                        CHEAT SEQUENCE PACKAGE
// ==========================================================================

static byte    cheat_xlate_table[256];

void cht_Init()
{
    int i;
    for (i=0;i<256;i++) 
        cheat_xlate_table[i] = SCRAMBLE(i);
}

//
// Called in st_stuff module, which handles the input.
// Returns a 1 if the cheat was successful, 0 if failed.
//
int cht_CheckCheat ( cheatseq_t*   cht,     char           key )
{
    int rc = 0;

    if (!cht->p)
        cht->p = cht->sequence; // initialize if first time

    if (*cht->p == 0)
        *(cht->p++) = key;
    else if
        (cheat_xlate_table[(byte)key] == *cht->p) cht->p++;
    else
        cht->p = cht->sequence;

    if (*cht->p == 1)
        cht->p++;
    else if (*cht->p == 0xff) // end of sequence character
    {
        cht->p = cht->sequence;
        rc = 1;
    }

    return rc;
}

void cht_GetParam ( cheatseq_t*   cht,
                    char*         buffer )
{

    byte *p, c;

    p = cht->sequence;
    while (*(p++) != 1);

    do
    {
        c = *p;
        *(buffer++) = c;
        *(p++) = 0;
    }
    while (c && *p!=0xff );

    if (*p==0xff)
        *buffer = 0;

}

// added 2-2-98 for compatibility with dehacked
int idfa_armor=200;
int idfa_armor_class=2;
int idkfa_armor=200;
int idkfa_armor_class=2;
int god_health=1; // Tails 12-23-2001

static player_t *plyr;

boolean cht_Responder (event_t* ev)
{
    char*  msg;

    if (ev->type == ev_keydown)
    {
        msg = NULL;

        // added 17-5-98
        plyr = &players[consoleplayer];
        // b. - enabled for more debug fun.
        // if (gameskill != sk_nightmare) {

        if (cht_CheckCheat(&cheat_amap, ev->data1))
            am_cheating = (am_cheating+1) % 3;
        else

        // 'dqd' cheat for toggleable god mode
        if (cht_CheckCheat(&cheat_god, ev->data1))
        {
			modifiedgame = true;

            plyr->cheats ^= CF_GODMODE;
            if (plyr->cheats & CF_GODMODE)
            {
                if (plyr->mo)
                    plyr->mo->health = god_health;

                plyr->health = god_health;
                //plyr->message = STSTR_DQDON;
                msg = "Smells bad!\n";
            }
            else
                //plyr->message = STSTR_DQDOFF;
                msg = "...smells REALLY bad!\n";
        }
        // 'kfa' cheat for key full ammo
        else if (cht_CheckCheat(&cheat_ammo, ev->data1))
        {

			// Emerald cheat Tails
    if(!(emeralds & EMERALD1)){
        emeralds |= EMERALD1;
}
  else if((emeralds & EMERALD1) && !(emeralds & EMERALD2)){
        emeralds |= EMERALD2;
}
  else if((emeralds & EMERALD2) && !(emeralds & EMERALD3)){
        emeralds |= EMERALD3;
}
   else if((emeralds & EMERALD3) && !(emeralds & EMERALD4)){
        emeralds |= EMERALD4;
}
   else if((emeralds & EMERALD4) && !(emeralds & EMERALD5)){
        emeralds |= EMERALD5;
}
   else if((emeralds & EMERALD5) && !(emeralds & EMERALD6)){
        emeralds |= EMERALD6;
}
   else if((emeralds & EMERALD6) && !(emeralds & EMERALD7)){
        emeralds |= EMERALD7;
}
            //plyr->message = STSTR_KFAADDED;
            msg = "Got Emerald";
        }
        // 'mus' cheat for changing music
        else if (cht_CheckCheat(&cheat_mus, ev->data1))
        {
            char    buf[3];
            int             musnum;

            plyr->message = STSTR_MUS;
            cht_GetParam(&cheat_mus, buf);

            musnum = mus_map01m + (buf[0]-'0')*10 + buf[1]-'0' - 1;

            if (((buf[0]-'0')*10 + buf[1]-'0') > 35)
                //plyr->message = STSTR_NOMUS;
                msg = STSTR_NOMUS;
            else
                S_ChangeMusic(musnum, 1);

        }

        // 'cd' for changing cd track quickly
        //NOTE: the cheat uses the REAL track numbers, not remapped ones
        else if (cht_CheckCheat(&cheat_cd, ev->data1))
        {
            char    buf[3];

            cht_GetParam(&cheat_cd, buf);

            plyr->message = "Changing cd track...\n";
            I_PlayCD ((buf[0]-'0')*10 + (buf[1]-'0'), true);
        }


        // Simplified, accepting both "noclip" and "idspispopd".
        // no clipping mode cheat
        else
        if (cht_CheckCheat(&cheat_commercial_noclip,ev->data1))
        {
            if(!cv_debug)
				cv_debug = true;
			else
				cv_debug = false;
        }

        // 'behold' power-up menu
        if (cht_CheckCheat(&cheat_powerup[6], ev->data1))
        {
            //plyr->message = STSTR_BEHOLD;
//            msg = STSTR_BEHOLD;
        }
        // 'choppers' invulnerability & chainsaw
        else

        if (cht_CheckCheat(&cheat_choppers, ev->data1))
        {
            I_Error("Hohohoho!! *B^D");

			// Demo Tails
        }
        // 'mypos' for player position
        else

        if (cht_CheckCheat(&cheat_mypos, ev->data1))
        {
            //plyr->message = buf;
            CONS_Printf (va("ang=%i;x,y=(%i,%i)\n",
                  players[statusbarplayer].mo->angle / ANGLE_1,
                  players[statusbarplayer].mo->x >> FRACBITS,
                  players[statusbarplayer].mo->y >> FRACBITS));

        }
        else
/*
        //added:28-02-98: new fly cheat using jump key
        if (cht_CheckCheat(&cheat_fly_around, ev->data1))
        {
            plyr->cheats ^= CF_FLYAROUND;
            if (plyr->cheats & CF_FLYAROUND)
                //plyr->message = "FLY MODE ON : USE JUMP KEY";
                msg = "FLY MODE ON : USE JUMP KEY\n";
            else
                //plyr->message = "FLY MODE OFF";
                msg = "FLY MODE OFF\n";
        }
*/
        // 'clev' change-level cheat
        if (cht_CheckCheat(&cheat_clev, ev->data1))
        {
            char              buf[3];
            int               epsd;
            int               map;

            cht_GetParam(&cheat_clev, buf);

            epsd = 0;
            map = (buf[0] - '0')*10 + buf[1] - '0';

            // Catch invalid maps.
            //added:08-01-98:moved if (epsd<1)...  up
            if (map < 1)
                return false;

            // Ohmygod - this is not going to work.
            if (( epsd > 1) || (map > 34))
                return false;

            // So be it.
            //plyr->message = STSTR_CLEV;
            msg = STSTR_CLEV;
            G_DeferedInitNew(gameskill, G_BuildMapName(map), cv_skin.value, false); // Tails 03-02-2002
        }

        // append a newline to the original doom messages
        if (msg)
            CONS_Printf("%s\n",msg);
    }
    return false;
}


// command that can be typed at the console !

void Command_CheatNoClip_f (void)
{
    player_t*   plyr;
    if (multiplayer)
        return;

    plyr = &players[consoleplayer];

    plyr->cheats ^= CF_NOCLIP;

    if (plyr->cheats & CF_NOCLIP)
        CONS_Printf ("No Clipping On");
    else
        CONS_Printf ("No Clipping Off");

	modifiedgame = true;

	CONS_Printf("WARNING: Game must be restarted to record statistics.\n");
}

void Command_CheatGod_f (void)
{
    player_t*   plyr;

    if (multiplayer)
        return;

    plyr = &players[consoleplayer];

    plyr->cheats ^= CF_GODMODE;
    if (plyr->cheats & CF_GODMODE)
    {
        if (plyr->mo)
            plyr->mo->health = god_health;

        plyr->health = god_health;
        CONS_Printf ("Sissy Mode On\n");
    }
    else
        CONS_Printf ("Sissy Mode Off\n");

	modifiedgame = true;

	CONS_Printf("WARNING: Game must be restarted to record statistics.\n");
}

// heretic cheat

#define CHEAT_ENCRYPT(a) SCRAMBLE(a)

typedef struct Cheat_s
{
        void (*func)(player_t *player, struct Cheat_s *cheat);
        byte *sequence;
        byte *pos;
        int args[2];
        int currentArg;
} Cheat_t;


static boolean CheatAddKey(Cheat_t *cheat, byte key, boolean *eat);
static void CheatGodFunc(player_t *player, Cheat_t *cheat);
static void CheatNoClipFunc(player_t *player, Cheat_t *cheat);
static void CheatWeaponsFunc(player_t *player, Cheat_t *cheat);
static void CheatHealthFunc(player_t *player, Cheat_t *cheat);
//static void CheatSoundFunc(player_t *player, Cheat_t *cheat);
static void CheatTickerFunc(player_t *player, Cheat_t *cheat);
static void CheatWarpFunc(player_t *player, Cheat_t *cheat);
static void CheatMassacreFunc(player_t *player, Cheat_t *cheat);
static void CheatIDDQDFunc(player_t *player, Cheat_t *cheat);
/*
// Toggle god mode
static byte CheatGodSeq[] =
{
        CHEAT_ENCRYPT('q'),
        CHEAT_ENCRYPT('u'),
        CHEAT_ENCRYPT('i'),
        CHEAT_ENCRYPT('c'),
        CHEAT_ENCRYPT('k'),
        CHEAT_ENCRYPT('e'),
        CHEAT_ENCRYPT('n'),
        0xff
};

// Toggle no clipping mode
static byte CheatNoClipSeq[] =
{
        CHEAT_ENCRYPT('k'),
        CHEAT_ENCRYPT('i'),
        CHEAT_ENCRYPT('t'),
        CHEAT_ENCRYPT('t'),
        CHEAT_ENCRYPT('y'),
        0xff
};

// Get all weapons and ammo
static byte CheatWeaponsSeq[] =
{
        CHEAT_ENCRYPT('r'),
        CHEAT_ENCRYPT('a'),
        CHEAT_ENCRYPT('m'),
        CHEAT_ENCRYPT('b'),
        CHEAT_ENCRYPT('o'),
        0xff
};

// Get full health
static byte CheatHealthSeq[] =
{
        CHEAT_ENCRYPT('p'),
        CHEAT_ENCRYPT('o'),
        CHEAT_ENCRYPT('n'),
        CHEAT_ENCRYPT('c'),
        CHEAT_ENCRYPT('e'),
        0xff
};

// Toggle ticker
static byte CheatTickerSeq[] =
{
        CHEAT_ENCRYPT('t'),
        CHEAT_ENCRYPT('i'),
        CHEAT_ENCRYPT('c'),
        CHEAT_ENCRYPT('k'),
        CHEAT_ENCRYPT('e'),
        CHEAT_ENCRYPT('r'),
        0xff, 0
};

// Warp to new level
static byte CheatWarpSeq[] =
{
        CHEAT_ENCRYPT('e'),
        CHEAT_ENCRYPT('n'),
        CHEAT_ENCRYPT('g'),
        CHEAT_ENCRYPT('a'),
        CHEAT_ENCRYPT('g'),
        CHEAT_ENCRYPT('e'),
        0, 0, 0xff, 0
};

// Kill all monsters
static byte CheatMassacreSeq[] =
{
        CHEAT_ENCRYPT('m'),
        CHEAT_ENCRYPT('a'),
        CHEAT_ENCRYPT('s'),
        CHEAT_ENCRYPT('s'),
        CHEAT_ENCRYPT('a'),
        CHEAT_ENCRYPT('c'),
        CHEAT_ENCRYPT('r'),
        CHEAT_ENCRYPT('e'),
        0xff, 0
};

static byte CheatIDDQDSeq[] =
{
        CHEAT_ENCRYPT('i'),
        CHEAT_ENCRYPT('d'),
        CHEAT_ENCRYPT('d'),
        CHEAT_ENCRYPT('q'),
        CHEAT_ENCRYPT('d'),
        0xff, 0
};

static Cheat_t Cheats[] =
{
    { CheatGodFunc, CheatGodSeq, NULL, {0, 0}, 0 },
    { CheatNoClipFunc, CheatNoClipSeq, NULL, {0, 0}, 0 },
    { CheatWeaponsFunc, CheatWeaponsSeq, NULL, {0, 0}, 0 },
    { CheatHealthFunc, CheatHealthSeq, NULL, {0, 0}, 0 },
//      { CheatSoundFunc, CheatSoundSeq, NULL, 0, 0, 0 },
    { CheatTickerFunc, CheatTickerSeq, NULL, {0, 0}, 0 },
    { CheatWarpFunc, CheatWarpSeq, NULL, {0, 0}, 0 },
    { CheatMassacreFunc, CheatMassacreSeq, NULL, {0, 0}, 0 },
    { CheatIDDQDFunc, CheatIDDQDSeq, NULL, {0, 0}, 0 },
    { NULL, NULL, NULL, {0, 0}, 0 } // Terminator
};
*/

//--------------------------------------------------------------------------
//
// FUNC CheatAddkey
//
// Returns true if the added key completed the cheat, false otherwise.
//
//--------------------------------------------------------------------------

static boolean CheatAddKey(Cheat_t *cheat, byte key, boolean *eat)
{
        if(!cheat->pos)
        {
                cheat->pos = cheat->sequence;
                cheat->currentArg = 0;
        }
        if(*cheat->pos == 0)
        {
                *eat = true;
                cheat->args[cheat->currentArg++] = key;
                cheat->pos++;
        }
        else if( cheat_xlate_table[key] == *cheat->pos)
        {
                cheat->pos++;
        }
        else
        {
                cheat->pos = cheat->sequence;
                cheat->currentArg = 0;
        }
        if(*cheat->pos == 0xff)
        {
                cheat->pos = cheat->sequence;
                cheat->currentArg = 0;
                return(true);
        }
        return(false);
}

//--------------------------------------------------------------------------
//
// CHEAT FUNCTIONS
//
//--------------------------------------------------------------------------

static void CheatGodFunc(player_t *player, Cheat_t *cheat)
{
        player->cheats ^= CF_GODMODE;
        if(player->cheats&CF_GODMODE)
        {
                P_SetMessage(player, "Sissy Mode On", false);
        }
        else
        {
                P_SetMessage(player, "Sissy Mode Off", false);
        }
			modifiedgame = true;

	CONS_Printf("WARNING: Game must be restarted to record statistics.\n");
}

static void CheatNoClipFunc(player_t *player, Cheat_t *cheat)
{
        player->cheats ^= CF_NOCLIP;
        if(player->cheats&CF_NOCLIP)
        {
                P_SetMessage(player, "No clipping on", false);
        }
        else
        {
                P_SetMessage(player, "No clipping off", false);
        }
			modifiedgame = true;

	CONS_Printf("WARNING: Game must be restarted to record statistics.\n");
}

static void CheatWeaponsFunc(player_t *player, Cheat_t *cheat)
{

}

static void CheatHealthFunc(player_t *player, Cheat_t *cheat)
{
	// Har har har. Now you have 0 rings.
    player->health = player->mo->health = 1;
}

static void CheatTickerFunc(player_t *player, Cheat_t *cheat)
{
        cv_ticrate.value = !cv_ticrate.value;
        if(cv_ticrate.value)
        {
        }
        else
        {
        }
}

static void CheatWarpFunc(player_t *player, Cheat_t *cheat)
{
        int episode;
        int map;
        char *mapname;

        episode = cheat->args[0]-'0';
        map = cheat->args[1]-'0';
        mapname = G_BuildMapName(map);
        if( W_CheckNumForName( mapname )>0 )
        {
                G_DeferedInitNew(gameskill, mapname, cv_skin.value, false); // Tails 03-02-2002
        }
}

static void CheatMassacreFunc(player_t *player, Cheat_t *cheat)
{
}

static void CheatIDDQDFunc(player_t *player, Cheat_t *cheat)
{
        P_DamageMobj(player->mo, NULL, player->mo, 10000);
}
