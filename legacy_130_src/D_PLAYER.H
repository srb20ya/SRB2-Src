// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_player.h,v 1.2 2000/02/27 00:42:10 hurdler Exp $
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
// $Log: d_player.h,v $
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      player data structures
//
//-----------------------------------------------------------------------------


#ifndef __D_PLAYER__
#define __D_PLAYER__


// The player data structure depends on a number
// of other structs: items (internal inventory),
// animation states (closely tied to the sprites
// used to represent them, unfortunately).
#include "d_items.h"
#include "p_pspr.h"

// In addition, the player is just a special
// case of the generic moving object/actor.
#include "p_mobj.h"

// Finally, for odd reasons, the player input
// is buffered within the player data struct,
// as commands per game tick.
#include "d_ticcmd.h"

//
// Player states.
//
typedef enum
{
    // Playing or camping.
    PST_LIVE,
    // Dead on the ground, view follows killer.
    PST_DEAD,
    // Ready to restart/respawn???
    PST_REBORN

} playerstate_t;


//
// Player internal flags, for cheats and debug.
//
typedef enum
{
    // No clipping, walk through barriers.
    CF_NOCLIP           = 1,
    // No damage, no health loss.
    CF_GODMODE          = 2,
    // Not really a cheat, just a debug aid.
    CF_NOMOMENTUM       = 4,

    //added:28-02-98: new cheats
    CF_FLYAROUND        = 8,

    //added:28-02-98: NOT REALLY A CHEAT
    // Allow player avatar to walk in-air
    //  if trying to get over a small wall (hack for playability)
    CF_JUMPOVER         = 16

} cheat_t;


// ========================================================================
//                          PLAYER STRUCTURE
// ========================================================================
typedef struct player_s
{
    mobj_t*             mo;

	// Emerald hunting Tails 12-20-2001
	mobj_t*             hunt1;
	mobj_t*             hunt2;
	mobj_t*             hunt3;

    // added 1-6-98: for movement prediction
#ifdef CLIENTPREDICTION2
    mobj_t*             spirit;
#endif
    playerstate_t       playerstate;
    ticcmd_t            cmd;

    // Determine POV,
    //  including viewpoint bobbing during movement.
    // Focal origin above r.z
    fixed_t             viewz;
    // Base height above floor for viewz.
    fixed_t             viewheight;
    // Bob/squat speed.
    fixed_t             deltaviewheight;
    // bounded/scaled total momentum.
    fixed_t             bob;

    //added:16-02-98: mouse aiming, where the guy is looking at!
    //                 It is updated with cmd->aiming.
    angle_t             aiming;

    // This is only used between levels,
    // mo->health is used during levels.
    int                 health;
    int                 armorpoints;
    // Armor type is 0-2.
    byte                armortype;

    // Power ups. invinc and invis are tic counters.
    int                 powers[NUMPOWERS];
    byte                cards; // bit field see declration of card_t
    boolean             backpack;

    // Frags, kills of other players.
    USHORT              addfrags;   // player have killed a player but is gone
    USHORT              frags[MAXPLAYERS];
    weapontype_t        readyweapon;

    // Is wp_nochange if not changing.
    weapontype_t        pendingweapon;

    boolean             weaponowned[NUMWEAPONS];
    int                 ammo[NUMAMMO];
    int                 maxammo[NUMAMMO];
     // added by Boris : preferred weapons order stuff
     char                favoritweapon[NUMWEAPONS];
     boolean             originalweaponswitch;
     //added:28-02-98:
     boolean             autoaim_toggle;

    // True if button down last tic.
    boolean             attackdown;
    boolean             usedown;
    boolean             jumpdown;   //added:19-03-98:dont jump like a monkey!

    // Bit flags, for cheats and debug.
    // See cheat_t, above.
    int                 cheats;

    // Refired shots are less accurate.
    int                 refire;

     // For intermission stats.
    int                 killcount;
    int                 itemcount;
    int                 secretcount;

    // Hint messages.
    char*               message;

    // For screen flashing (red or bright).
    int                 damagecount;
    int                 bonuscount;

    // Who did damage (NULL for floors/ceilings).
    mobj_t*             attacker;
    int                 specialsector;      //lava/slime/water...

    // So gun flashes light up areas.
    int                 extralight;

    // Current PLAYPAL, ???
    //  can be set to REDCOLORMAP for pain, etc.
    int                 fixedcolormap;

    // Player skin colorshift,
    //  0-3 for which color to draw player.
    // adding 6-2-98 comment : unused by doom2 1.9 now is used
    int                 skincolor;

    // added 2/8/98
    int                 skin;

    // Overlay view sprites (gun, etc).
    pspdef_t            psprites[NUMPSPRITES];

    // True if secret level has been done.
    boolean             didsecret;

    int score; // player score Tails 03-01-2000
    int dashspeed; // dashing speed Tails 03-01-2000

    int charspeed; // Speed definition Tails 03-01-2000
	int charability; // Ability definition Tails 11-15-2000

    int lives; // do lives now, worry about continues later Tails 03-09-2000
    int continues; // continues that player has acquired Tails 03-11-2000

    int timebonus; // Time Bonus Tails 03-10-2000
    int ringbonus; // Ring Bonus Tails 03-10-2000
    int fscore; // Fake score for intermissions Tails 03-12-2000
    int seconds; // Tails 06-13-2000
    int minutes; // Tails 06-13-2000

// start emeralds Tails 04-08-2000
    int emerald1;
    int emerald2;
    int emerald3;
    int emerald4;
    int emerald5;
    int emerald6;
    int emerald7;
	int token; // Number of tokens collected in a level Tails 08-11-2001
	int lastmap; // Last level you were at Tails 08-11-2001
	int	sstimer; // Time allotted in the special stage Tails 08-11-2001
// end emeralds Tails 04-08-2000

    int superready; // Ready for Super? Tails 04-08-2000

    int acceleration; // Acceleration Tails 04-24-2000

    int xtralife; // Ring Extra Life counter

	int xtralife2; // Score xtra life counter

    int walking; // Are the walking frames playing? Tails 08-18-2000
    int running; // Are the running frames playing? Tails 08-18-2000
    int spinning; // Are the spinning frames playing? Tails 08-18-2000
    int speed; // Player's speed (distance formula of MOMX and MOMY values) Tails 08-21-2000
    int mforward; // Moving forward
    int mbackward; // Moving backward
    int jumping; // Jump counter Tails 10-14-2000

	// Moved eflags to player ints Tails 10-30-2000
	int mfjumped;
	int mfspinning;
	int mfstartdash;

	int fly1; // Tails flying Tails 11-01-2000
	int camunder; // Is the camera underwater? Tails 11-02-2000
	int scoreadd; // Used for multiple enemy attack bonus Tails 11-03-2000
	int gliding; // Are you gliding? Tails 11-15-2000
	int glidetime; // Glide counter for thrust Tails 11-17-2000
	int climbing; // Climbing on the wall Tails 11-18-2000
	int deadtimer; // End game if game over lasts too long Tails 11-21-2000
	int splish; // Don't make splish repeat tons Tails 12-08-2000
	int exiting; // Exitlevel timer Tails 12-15-2000
	int blackow; // Tails 01-11-2001

	int homing; // Are you homing? Tails 06-20-2001

	////////////////////////////
	// Conveyor Belt Movement //
	////////////////////////////
	fixed_t cmomx; // Conveyor momx Tails 04-13-2001
	fixed_t cmomy; // Conveyor momy Tails 04-13-2001
	fixed_t rmomx; // "Real" momx (momx - cmomx) Tails 04-13-2001
	fixed_t rmomy; // "Real" momy (momy - cmomy)Tails 04-13-2001


	/////////////////////
	// Race Mode Stuff //
	/////////////////////
	int numboxes; // Number of item boxes obtained for Race Mode Tails 04-25-2001
	int totalring; // Total number of rings obtained for Race Mode Tails 04-25-2001
	int countdown; // 60 second countdown for Race Mode Tails 04-25-2001
	int countdown2; // 2nd countdown variable for first to finish Tails 04-25-2001
	int realtime; // integer replacement for leveltime Tails 04-25-2001
	int racescore; // Total of won categories Tails 05-01-2001

	////////////////////
	// Tag Mode Stuff //
	////////////////////
	int tagit; // The player is it! For Tag Mode Tails 05-08-2001
	int tagcount; // Number of tags player has made Tails 05-09-2001
	int tagzone; // Tag Zone timer Tails 05-11-2001
	int taglag; // Don't go back in the tag zone too early Tails 05-11-2001

	////////////////////
	// CTF Mode Stuff //
	////////////////////
	int ctfteam; // 1 == Red, 2 == Blue Tails 07-22-2001
	int gotflag; // 1 == Red  2 == Blue Do you have the flag? Tails 07-22-2001
	int bluescore; // Team Scores Tails 07-31-2001
	int redscore; // Team Scores Tails 07-31-2001
    mapthing_t          *flagpoint; // Original flag spawn location Tails 08-02-2001

	int redxvi; // RedXVI

	int emeraldhunt; // # of emeralds found Tails 12-12-2001

	boolean snowbuster; // Snow Buster upgrade! Tails 12-12-2001
	int bustercount; // Charge for Snow Buster Tails 12-12-2001

} player_t;


//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct
{
    boolean     in;     // whether the player is in game

    // Player stats, kills, collected items etc.
    int         sscore; // score tally Tails 03-09-2000
    int         skills;
    int         sitems;
    int         ssecret;
	int			minutes; // Tails
	int			seconds; // Tails
    USHORT      frags[MAXPLAYERS]; // added 17-1-98 more than 4 players
    // BP: unused for now but don't forget....
    USHORT      addfrags;

} wbplayerstruct_t;

typedef struct
{
    int         epsd;   // episode # (0-2)

    // if true, splash the secret level
    boolean     didsecret;

    // previous and next levels, origin 0
    int         last;
    int         next;

    int         maxkills;
    int         maxitems;
    int         maxsecret;
    int         maxfrags;

    // the par time
    int         partime;

    // index of this player in game
    int         pnum;

    wbplayerstruct_t    plyr[MAXPLAYERS];

} wbstartstruct_t;


#endif
