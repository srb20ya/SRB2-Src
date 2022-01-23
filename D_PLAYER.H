
// d_player.h : player data structures

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
    mobj_t*             spirit;
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
    //                actually, this is a constant between -100  +100
    //                (its the line number of a 320x200 screen, which
    //                 is drawn at the exact y center of the view)
    //                 It is updated with cmd->aiming.
    angle_t             aiming;

    // This is only used between levels,
    // mo->health is used during levels.
    int                 health;
    int                 armorpoints;
    // Armor type is 0-2.
    int                 armortype;

    // Power ups. invinc and invis are tic counters.
    int                 powers[NUMPOWERS];
    boolean             cards[NUMCARDS];
    boolean             backpack;

    // Frags, kills of other players.
    int                 frags[MAXPLAYERS];
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
    int                 attackdown;
    int                 usedown;
    int                 jumpdown;   //added:19-03-98:dont jump like a monkey!

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

    int wants_to_thok;     //Stealth: Homing Attack 12-31-99
    int thok_dist;
    angle_t thok_angle;

    int score; // player score Tails 03-01-2000
    int releasedash; // dash stuff Tails 02-27-2000
    int dashspeed; // dashing speed Tails 03-01-2000

    int typechar; // Which character? Tails 03-01-2000

    int lives; // do lives now, worry about continues later Tails 03-09-2000
    int continues; // continues that player has acquired Tails 03-11-2000

    int timebonus; // Time Bonus Tails 03-10-2000
    int ringbonus; // Ring Bonus Tails 03-10-2000
    int fscore; // Fake score for intermissions Tails 03-12-2000
    int seconds; // Tails 06-13-2000
    int minutes; // Tails 06-13-2000

    int ringblink; // used as a ring blinking timer Tails 03-14-2000

// start emeralds Tails 04-08-2000
    int emerald1;
    int emerald2;
    int emerald3;
    int emerald4;
    int emerald5;
    int emerald6;
    int emerald7;
// end emeralds Tails 04-08-2000

    int superready; // Ready for Super? Tails 04-08-2000
    int tailsfly; // Lots easier than stupid MF flags Tails 06-13-2000

    int acceleration; // Acceleration Tails 04-24-2000

    int xtralife1; // Extra Life
    int xtralife2; // via rings status Tails 04-25-2000
    int xtralife3; // 50,000 points
    int xtralife4; // 100,000 points
    int xtralife5; // 150,000 points
    int xtralife6; // 200,000 points
    int xtralife7; // 250,000 points
    int xtralife8; // 300,000 points
    int xtralife9; // 350,000 points
    int xtralife10; // 400,000 points
    int xtralife11; // 450,000 points
    int xtralife12; // 500,000 points
    int xtralife13; // 550,000 points
    int xtralife14; // 600,000 points
    int xtralife15; // 650,000 points
    int xtralife16; // 700,000 points
    int xtralife17; // 750,000 points
    int xtralife18; // 800,000 points
    int xtralife19; // 850,000 points
    int xtralife20; // 900,000 points
    int xtralife21; // 950,000 points
    int xtralife22; // 1,000,000 points

    int walking; // Are the walking frames playing? Tails 08-18-2000
    int running; // Are the walking frames playing? Tails 08-18-2000
    int spinning; // Are the walking frames playing? Tails 08-18-2000
    int speed; // Player's speed (distance formula of MOMX and MOMY values) Tails 08-21-2000
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
    int         stimeb; // Time Bonus Tails 03-10-2000
    int         sringb; // Ring Bonus Tails 03-10-2000
    int         skills;
    int         sitems;
    int         ssecret;
    int         stime;
    int         frags[MAXPLAYERS]; // added 17-1-98 more than 4 players
    int         score;  // current score on entry, modified on return

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
