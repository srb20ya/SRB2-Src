// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_player.h,v 1.4 2001/01/25 22:15:41 bpereira Exp $
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
// Revision 1.4  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.3  2000/08/31 14:30:55  bpereira
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

	angle_t             slope; // Ramp test Tails

    // This is only used between levels,
    // mo->health is used during levels.
    int                 health;

    // Power ups. invinc and invis are tic counters.
    int                 powers[NUMPOWERS];

    boolean             autoaim_toggle;

    // True if button down last tic.
    boolean             attackdown;
    boolean             usedown;
    boolean             jumpdown;   //added:19-03-98:dont jump like a monkey!

    // Bit flags, for cheats and debug.
    // See cheat_t, above.
    int                 cheats;

     // For intermission stats.
    int                 killcount;
    int                 itemcount;
    int                 secretcount;

    // Hint messages.
    char*               message;

    // For screen flashing (red or bright).
    int                 bonuscount;

    // Who did damage (NULL for floors/ceilings).
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

    int score; // player score Tails 03-01-2000
    int dashspeed; // dashing speed Tails 03-01-2000

	int normalspeed; // Normal ground

	int thrustfactor; // Thrust = thrustfactor * acceleration
	int accelstart; // Starting acceleration if speed = 0.
    int acceleration; // Acceleration Tails 04-24-2000

	// Graue 12-22-2003
	int oldnormalspeed;
	int oldthrustfactor;
	int oldaccelstart;
	int oldacceleration;
	int circnormalspeed;
	int circthrustfactor;
	int circaccelstart;
	int circacceleration;

	int charability; // Ability definition Tails 11-15-2000
	int charspin; // Is the player allowed to spin/spindash?
	int jumpfactor; // How high can the player jump?

	// Start and end positions for the changeable color of a skin Tails 06-07-2002
	int starttranscolor;
	int endtranscolor;

	// Tails 12-15-2003
	int prefcolor;

    int lives; // do lives now, worry about continues later Tails 03-09-2000
    int continues; // continues that player has acquired Tails 03-11-2000

    int timebonus; // Time Bonus Tails 03-10-2000
    int ringbonus; // Ring Bonus Tails 03-10-2000
    int fscore; // Fake score for intermissions Tails 03-12-2000
    int seconds; // Tails 06-13-2000
    int minutes; // Tails 06-13-2000

    int superready; // Ready for Super? Tails 04-08-2000

    int xtralife; // Ring Extra Life counter

	int xtralife2; // Score xtra life counter

    int walking; // Are the walking frames playing? Tails 08-18-2000
    int running; // Are the running frames playing? Tails 08-18-2000
    int spinning; // Are the spinning frames playing? Tails 08-18-2000
    int speed; // Player's speed (distance formula of MOMX and MOMY values) Tails 08-21-2000
    int jumping; // Jump counter Tails 10-14-2000

	// Moved eflags to player ints Tails 10-30-2000
	int mfjumped;
	int mfspinning;
	int mfstartdash;

	int fly1; // Tails flying Tails 11-01-2000
	int scoreadd; // Used for multiple enemy attack bonus Tails 11-03-2000
	int gliding; // Are you gliding? Tails 11-15-2000
	int glidetime; // Glide counter for thrust Tails 11-17-2000
	int climbing; // Climbing on the wall Tails 11-18-2000
	int deadtimer; // End game if game over lasts too long Tails 11-21-2000
	int splish; // Don't make splish repeat tons Tails 12-08-2000
	int exiting; // Exitlevel timer Tails 12-15-2000
	int blackow; // Tails 01-11-2001

	boolean homing; // Are you homing? Tails 06-20-2001

	////////////////////////////
	// Conveyor Belt Movement //
	////////////////////////////
	fixed_t cmomx; // Conveyor momx Tails 04-13-2001
	fixed_t cmomy; // Conveyor momy Tails 04-13-2001
	//fixed_t cmomz; // Conveyor momz Graue 12-12-2003
	fixed_t rmomx; // "Real" momx (momx - cmomx) Tails 04-13-2001
	fixed_t rmomy; // "Real" momy (momy - cmomy) Tails 04-13-2001
	//fixed_t rmomz; // "Real" momz (momz - cmomz) Graue 12-12-2003

	/////////////////////
	// Race Mode Stuff //
	/////////////////////
	int numboxes; // Number of item boxes obtained for Race Mode Tails 04-25-2001
	int totalring; // Total number of rings obtained for Race Mode Tails 04-25-2001
	int realtime; // integer replacement for leveltime Tails 04-25-2001
	int racescore; // Total of won categories Tails 05-01-2001

	////////////////////
	// Tag Mode Stuff //
	////////////////////
	int tagit; // The player is it! For Tag Mode Tails 05-08-2001
	int tagcount; // Number of tags player has avoided Tails 05-09-2001
	int tagzone; // Tag Zone timer Tails 05-11-2001
	int taglag; // Don't go back in the tag zone too early Tails 05-11-2001

	////////////////////
	// CTF Mode Stuff //
	////////////////////
	int ctfteam; // 1 == Red, 2 == Blue Tails 07-22-2001
	unsigned short gotflag; // 1 == Red  2 == Blue Do you have the flag? Tails 07-22-2001

	int redxvi; // RedXVI

	int emeraldhunt; // # of emeralds found Tails 12-12-2001

	boolean snowbuster; // Snow Buster upgrade! Tails 12-12-2001
	int bustercount; // Charge for Snow Buster Tails 12-12-2001

	int weapondelay; // Delay (if any) to fire the weapon again Tails 07-12-2002
	int taunttimer; // Delay before you can use the taunt again Tails 09-06-2002

	// Starpost information Tails 07-03-2002
	int starpostx;
	int starposty;
	int starpostz;
	int starpostnum; // The number of the last starpost you hit
	int starposttime; // Your time when you hit the starpost
	int starpostangle; // Angle that the starpost is facing - you respawn facing this way
	unsigned short starpostbit; // List of starposts hit

	double angle_speed; // Speed for NiGHTS! Tails 05-08-2002
	double angle_pos;
	double old_angle_pos;
	boolean nightsmode; // Is the player in NiGHTS mode? Tails 10-15-2002

	boolean axishit;
	boolean axistransferred;
	boolean transfertoclosest;
	int flyangle;
	int drilltimer;
	int linkcount;
	int linktimer;
	int anotherflyangle;
	int transferangle;
	int nightstime; // How long you can fly as NiGHTS.
	boolean nightsfall; // Spill rings after falling
	mobj_t* lastaxis;
	int drillmeter;
	byte drilldelay;
	boolean drilling;
	boolean skiddown;
	boolean bonustime; // Capsule destroyed, now it's bonus time!
	mobj_t* capsule; // Go inside the capsule
	byte mare; // Current mare

	int lastsidehit;
	int lastlinehit;

	boolean carried; // Tails pickup!

	int currentthing; // For the object placement mode Tails

	boolean lightdash; // Experimental fun thing
	boolean lightdashallowed;

	boolean thokked; // You already thokked on this jump.

	int laps; // For circuit mode Graue 11-15-2003
	int onconveyor; // You are on a conveyor belt if nonzero Graue 12-26-2003
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
    int         score;  // current score on entry, modified on return

} wbplayerstruct_t;

typedef struct
{
    int         epsd;   // episode # (0-2)

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

void A_TicWeapon( player_t *player,  pspdef_t *psp );

#endif
