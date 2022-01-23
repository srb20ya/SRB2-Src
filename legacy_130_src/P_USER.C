// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_user.c,v 1.6 2000/08/03 17:57:42 bpereira Exp $
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
// $Log: p_user.c,v $
// Revision 1.6  2000/08/03 17:57:42  bpereira
// no message
//
// Revision 1.5  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.4  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.3  2000/03/29 19:39:48  bpereira
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
//      Player related stuff.
//      Bobbing POV/weapon, movement.
//      Pending weapon.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "d_event.h"
#include "g_game.h"
#include "p_local.h"
#include "r_main.h"
#include "s_sound.h"
#include "r_things.h" // Tails 03-01-2000
#include "m_random.h" // Protoyping to remove warnings Tails
#include "i_sound.h" // Prototyping to remove warnings Tails
#include "d_think.h" // Tails 04-18-2001
#include "r_sky.h" // Tails 11-18-2001

boolean  P_NukeEnemies (player_t* player);
boolean  PIT_NukeEnemies (mobj_t* thing);
boolean  P_LookForEnemies (player_t* player);
boolean  P_HomingAttack (player_t* player, mobj_t* enemy);
void D_StartTitle(); // Tails
void P_FindEmerald();


// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP         32

//
// Movement.
//

// 16 pixels of bob
#define MAXBOB  0x100000

boolean         onground;



//
// P_Thrust
// Moves the given origin along a given angle.
//
void P_Thrust ( mobj_t*       mo,
                angle_t       angle,
                fixed_t       move )
{
    angle >>= ANGLETOFINESHIFT;

    mo->momx += FixedMul(move,finecosine[angle]);
    mo->momy += FixedMul(move,finesine[angle]);
}

//
// P_InstaThrust // Tails
// Moves the given origin along a given angle instantly.
//
void P_InstaThrust ( mobj_t*       mo,
                     angle_t       angle,
                     fixed_t       move )
{
    angle >>= ANGLETOFINESHIFT;

    mo->momx = FixedMul(move,finecosine[angle]);
    mo->momy = FixedMul(move,finesine[angle]);
}

// Returns a location (hard to explain - go see how it is used)
fixed_t P_ReturnThrustX ( mobj_t*       mo,
                     angle_t       angle,
                     fixed_t       move )
{
    angle >>= ANGLETOFINESHIFT;

    return FixedMul(move,finecosine[angle]);
}
fixed_t P_ReturnThrustY ( mobj_t*       mo,
                     angle_t       angle,
                     fixed_t       move )
{
    angle >>= ANGLETOFINESHIFT;

    return FixedMul(move,finesine[angle]);
}



//
// P_CalcHeight
// Calculate the walking / running height adjustment
//
void P_CalcHeight (player_t* player)
{
    int         angle;
    fixed_t     bob;
    fixed_t     viewheight;


    // Regular movement bobbing
    // (needs to be calculated for gun swing
    // even if not on ground)
    // OPTIMIZE: tablify angle
    // Note: a LUT allows for effects
    //  like a ramp with low health.
#ifndef CLIENTPREDICTION2
    player->bob = FixedMul (player->mo->momx,player->mo->momx)
                + FixedMul (player->mo->momy,player->mo->momy);
#else
    player->bob = FixedMul (player->spirit->momx,player->spirit->momx)
                + FixedMul (player->spirit->momy,player->spirit->momy);
#endif

    player->bob >>= 2;

    if (player->bob>MAXBOB)
        player->bob = MAXBOB;

    if ((player->cheats & CF_NOMOMENTUM) || !onground)
    {
        //added:15-02-98: it seems to be useless code!
        //player->viewz = player->mo->z + (cv_viewheight.value<<FRACBITS);

        //if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
        //    player->viewz = player->mo->ceilingz-4*FRACUNIT;
#ifndef CLIENTPREDICTION2
        player->viewz = player->mo->z + player->viewheight;
#else
        player->viewz = player->spirit->z + player->viewheight;
#endif
        return;
    }

    angle = (FINEANGLES/20*leveltime)&FINEMASK;
    bob = FixedMul ( player->bob/2, finesine[angle]);


    // move viewheight
    viewheight = cv_viewheight.value << FRACBITS; // default eye view height

    if (player->playerstate == PST_LIVE)
    {
        player->viewheight += player->deltaviewheight;

        if (player->viewheight > viewheight)
        {
            player->viewheight = viewheight;
            player->deltaviewheight = 0;
        }

        if (player->viewheight < viewheight/2)
        {
            player->viewheight = viewheight/2;
            if (player->deltaviewheight <= 0)
                player->deltaviewheight = 1;
        }

        if (player->deltaviewheight)
        {
            player->deltaviewheight += FRACUNIT/4;
            if (!player->deltaviewheight)
                player->deltaviewheight = 1;
        }
    }   
#ifndef CLIENTPREDICTION2
    player->viewz = player->mo->z + player->viewheight + bob;
    if (player->viewz > player->mo->ceilingz-4*FRACUNIT)
        player->viewz = player->mo->ceilingz-4*FRACUNIT;
#else
    player->viewz = player->spirit->z + player->viewheight + bob;
    if (player->viewz > player->spirit->ceilingz-4*FRACUNIT)
        player->viewz = player->spirit->ceilingz-4*FRACUNIT;
#endif
}


extern int ticruned,ticmiss;

extern consvar_t cv_homing; // Tails 07-02-2001
extern consvar_t cv_nights; // Tails 07-02-2001
extern consvar_t cv_numsnow; // Tails 12-25-2001
//
// P_MovePlayer
//
void P_MovePlayer (player_t* player)
{
    ticcmd_t*           cmd;
	int					i; // Tails 04-25-2001
	int					tag = 0; // Tails 05-08-2001
	angle_t				movepushangle; // Analog Test Tails 06-10-2001
	angle_t				movepushsideangle; // Analog Test Tails 06-10-2001
	fixed_t tempx; // Tails 06-14-2001
	fixed_t tempy; // Tails 06-14-2001
	angle_t tempangle; // Tails 06-14-2001
	int snormalspeed;
	int swaterspeed;
	int sflyspeed;
	int normalspeed;
	int waterspeed;
	int flyspeed;
	msecnode_t *node;
	sector_t *sec;
    fixed_t   movepushforward=0,movepushside=0;

    cmd = &player->cmd;

//////////////////////
// MOVEMENT CODE	//
//////////////////////

	if(cv_nights.value)
	{
		movepushangle = R_PointToAngle2(player->mo->x, player->mo->y, 0, 0);
		movepushsideangle = R_PointToAngle2(player->mo->x, player->mo->y, 0, 0)-ANG90;
	}
	else if(cv_analog.value && camera.mo)
	{
		movepushangle = camera.mo->angle;
		movepushsideangle = camera.mo->angle-ANG90;
	}
	else
	{
		movepushangle = player->mo->angle;
		movepushsideangle = player->mo->angle-ANG90;
	}

	if(!cv_analog.value || (player->mfspinning)) // Analog Test Tails 06-10-2001
	{
		#ifndef ABSOLUTEANGLE
			player->mo->angle += (cmd->angleturn<<16);
		#else
			if(!player->climbing)
					player->mo->angle = (cmd->angleturn<<16);
		#endif
	}

// Start lots of cmomx/y stuff Tails 04-18-2001
// CMOMx stands for the conveyor belt speed.
	if(player->specialsector == 984)
	{
		if(player->mo->z > player->mo->waterz)
			player->cmomx = player->cmomy = 0;
	}

	else if(player->specialsector == 985 && player->mo->z > player->mo->floorz)
		player->cmomx = player->cmomy = 0;

	else
		player->cmomx = player->cmomy = 0;

	player->rmomx = player->mo->momx - player->cmomx;
	player->rmomy = player->mo->momy - player->cmomy;
// End lots of cmomx/y stuff Tails 04-18-2001

// Start various movement calculations Tails 10-08-2000
// This determines if the player is facing the direction they are travelling or not.
// Didn't you teacher say to pay attention in Geometry/Trigonometry class? ;)

// Calculates player's speed based on distance-of-a-line formula
	player->speed = (sqrt((abs(x1)*abs(x2)) + (abs(y1)*abs(y2)))); // Player's Speed Tails 08-22-2000

// forward
	if ((player->rmomx > 0 && player->rmomy > 0) && (player->mo->angle >= 0 && player->mo->angle < ANG90)) // Quadrant 1
		player->mforward = 1;
	else if ((player->rmomx < 0 && player->rmomy > 0) && (player->mo->angle >= ANG90 && player->mo->angle < ANG180)) // Quadrant 2
		player->mforward = 1;
	else if ((player->rmomx < 0 && player->rmomy < 0) && (player->mo->angle >= ANG180 && player->mo->angle < ANG270)) // Quadrant 3
		player->mforward = 1;
	else if ((player->rmomx > 0 && player->rmomy < 0) && ((player->mo->angle >= ANG270 && (player->mo->angle <= 65535 << FRACBITS)) || (player->mo->angle >= 0 && player->mo->angle <= ANG45))) // Quadrant 4
		player->mforward = 1;
	else if (player->rmomx > 0 && ((player->mo->angle >= ANG270+ANG45 && player->mo->angle <= 65535 << FRACBITS)))
		player->mforward = 1;
	else if (player->rmomx < 0 && (player->mo->angle >= ANG90+ANG45 && player->mo->angle <= ANG180+ANG45))
		player->mforward = 1;
	else if (player->rmomy > 0 && (player->mo->angle >= ANG45 && player->mo->angle <= ANG90+ANG45))
		player->mforward = 1;
	else if (player->rmomy < 0 && (player->mo->angle >= ANG180+ANG45 && player->mo->angle <= ANG270+ANG45))
		player->mforward = 1;
	else
		player->mforward = 0;
// backward
	if ((player->rmomx > 0 && player->rmomy > 0) && (player->mo->angle >= ANG180 && player->mo->angle < ANG270)) // Quadrant 3
		player->mbackward = 1;
	else if ((player->rmomx < 0 && player->rmomy > 0) && (player->mo->angle >= ANG270 && (player->mo->angle <= 65535 << FRACBITS))) // Quadrant 4
		player->mbackward = 1;
	else if ((player->rmomx < 0 && player->rmomy < 0) && (player->mo->angle >= 0 && player->mo->angle < ANG90)) // Quadrant 1
		player->mbackward = 1;
	else if ((player->rmomx > 0 && player->rmomy < 0) && (player->mo->angle >= ANG90 && player->mo->angle < ANG180)) // Quadrant 2
		player->mbackward = 1;
	else if (player->rmomx < 0 && ((player->mo->angle >= ANG270+ANG45 && player->mo->angle <= 65535 << FRACBITS) || (player->mo->angle >= 0 && player->mo->angle <= ANG45)))
		player->mbackward = 1;
	else if (player->rmomx > 0 && (player->mo->angle >= ANG90+ANG45 && player->mo->angle <= ANG180+ANG45))
		player->mbackward = 1;
	else if (player->rmomy < 0 && (player->mo->angle >= ANG45 && player->mo->angle <= ANG90+ANG45))
		player->mbackward = 1;
	else if (player->rmomy > 0 && (player->mo->angle >= ANG180+ANG45 && player->mo->angle <= ANG270+ANG45))
		player->mbackward = 1;
	else // Put in 'or' checks here!
		player->mbackward = 0;
// End various movement calculations Tails

    ticruned++;
    if( (cmd->angleturn & TICCMD_RECEIVED) == 0)
        ticmiss++;
    // Do not let the player control movement
    //  if not onground.
    onground = (player->mo->z <= player->mo->floorz) ||
               (player->cheats & CF_FLYAROUND);

    player->aiming = cmd->aiming<<16;

	// Set the player speeds.
	switch(player->charspeed)
	{
		case 0:
			snormalspeed = 52; // Super Sneakers
			swaterspeed = 20; // Super Sneakers & Underwater
			sflyspeed = 20; // Super Sneakers & Flying
			normalspeed = 26; // Normal ground
			waterspeed = 10; // Underwater
			flyspeed = 10; // Flying

			if (player->mo->eflags & MF_UNDERWATER || player->mo->eflags & MF_TOUCHWATER)
			{
				if(player->speed > 6)
					player->acceleration = 416;
				else if(player->speed > 5)
					player->acceleration = 384;
				else if(player->speed > 4)
					player->acceleration = 352;
				else if(player->speed > 3)
					player->acceleration = 320;
				else if(player->speed > 2)
					player->acceleration = 256;
				else if(player->speed > 1)
					player->acceleration = 192;
				else if(player->speed > 0)
					player->acceleration = 128;
				else
					player->acceleration = 64;
			}
			else
			{
				if(player->speed > 14)
					player->acceleration = 1024;
				else if(player->speed > 13)
					player->acceleration = 960;
				else if(player->speed > 12)
					player->acceleration = 896;
				else if(player->speed > 11)
					player->acceleration = 832;
				else if(player->speed > 10)
					player->acceleration = 768;
				else if(player->speed > 9)
					player->acceleration = 704;
				else if(player->speed > 8)
					player->acceleration = 640;
				else if(player->speed > 7)
					player->acceleration = 576;
				else if(player->speed > 6)
					player->acceleration = 512;
				else if(player->speed > 5)
					player->acceleration = 448;
				else if(player->speed > 4)
					player->acceleration = 416;
				else if(player->speed > 3)
					player->acceleration = 384;
				else if(player->speed > 2)
					player->acceleration = 320;
				else if(player->speed > 1)
					player->acceleration = 256;
				else if(player->speed > 0)
					player->acceleration = 192;
				else
					player->acceleration = 128;
			}
			break;

		case 1:
			snormalspeed = 34; // Super Sneakers
			swaterspeed = 14; // Super Sneakers & Underwater
			sflyspeed = 20; // Super Sneakers & Flying
			normalspeed = 17; // Normal ground
			waterspeed = 7; // Underwater
			flyspeed = 10; // Flying

			if (player->mo->eflags & MF_UNDERWATER || player->mo->eflags & MF_TOUCHWATER)
			{
				if(player->speed > 6)
					player->acceleration = 512;
				else if(player->speed > 5)
					player->acceleration = 448;
				else if(player->speed > 4)
					player->acceleration = 416;
				else if(player->speed > 3)
					player->acceleration = 384;
				else if(player->speed > 2)
					player->acceleration = 320;
				else if(player->speed > 1)
					player->acceleration = 256;
				else if(player->speed > 0)
					player->acceleration = 192;
				else
					player->acceleration = 128;
			}
			else
			{
				if(player->speed > 5)
					player->acceleration = 1024;
				else if(player->speed > 4)
					player->acceleration = 896;
				else if(player->speed > 3)
					player->acceleration = 768;
				else if(player->speed > 2)
					player->acceleration = 512;
				else if(player->speed > 1)
					player->acceleration = 384;
				else if(player->speed > 0)
					player->acceleration = 256;
				else
					player->acceleration = 192;
			}
			break;
		case 2:
			snormalspeed = 43; // Super Sneakers
			swaterspeed = 17; // Super Sneakers & Underwater
			sflyspeed = 20; // Super Sneakers & Flying
			normalspeed = 21; // Normal ground
			waterspeed = 9; // Underwater
			flyspeed = 10; // Flying

			if (player->mo->eflags & MF_UNDERWATER || player->mo->eflags & MF_TOUCHWATER)
			{
				if(player->speed > 6)
					player->acceleration = 512;
				else if(player->speed > 5)
					player->acceleration = 448;
				else if(player->speed > 4)
					player->acceleration = 416;
				else if(player->speed > 3)
					player->acceleration = 384;
				else if(player->speed > 2)
					player->acceleration = 320;
				else if(player->speed > 1)
					player->acceleration = 256;
				else if(player->speed > 0)
					player->acceleration = 192;
				else
					player->acceleration = 128;
			}
			else
			{
				if(player->speed > 10)
					player->acceleration = 1024;
				else if(player->speed > 9)
					player->acceleration = 944;
				else if(player->speed > 8)
					player->acceleration = 864;
				else if(player->speed > 7)
					player->acceleration = 784;
				else if(player->speed > 6)
					player->acceleration = 704;
				else if(player->speed > 5)
					player->acceleration = 624;
				else if(player->speed > 4)
					player->acceleration = 544;
				else if(player->speed > 3)
					player->acceleration = 464;
				else if(player->speed > 2)
					player->acceleration = 384;
				else if(player->speed > 1)
					player->acceleration = 304;
				else if(player->speed > 0)
					player->acceleration = 224;
				else
					player->acceleration = 128;
			}
			break;
		case 3:
			snormalspeed = 21; // Super Sneakers
			swaterspeed = 9; // Super Sneakers & Underwater
			sflyspeed = 10; // Super Sneakers & Flying
			normalspeed = 11; // Normal ground
			waterspeed = 5; // Underwater
			flyspeed = 5; // Flying
			player->acceleration = 1024;
			break;
		case 4:
			snormalspeed = 68; // Super Sneakers
			swaterspeed = 28; // Super Sneakers & Underwater
			sflyspeed = 40; // Super Sneakers & Flying
			normalspeed = 34; // Normal ground
			waterspeed = 14; // Underwater
			flyspeed = 20; // Flying
			player->acceleration = 1024;
			break;
		default: // Just for safety reasons Tails
			snormalspeed = 11; // Super Sneakers
			swaterspeed = 5; // Super Sneakers & Underwater
			sflyspeed = 5; // Super Sneakers & Flying
			normalspeed = 6; // Normal ground
			waterspeed = 3; // Underwater
			flyspeed = 3; // Flying
			player->acceleration = 1024;
			break;
	}

		if(cv_nights.value) // Tails 07-02-2001
			player->acceleration = 512*10;

		if (cmd->forwardmove && !(player->gliding || player->exiting || (player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_invisibility] && !onground)))
		{
			if(player->climbing)
			{
				player->mo->momz = (cmd->forwardmove/10)*FRACUNIT;
			}
			else if (cv_nights.value)
			{
				player->mo->momz += (cmd->forwardmove/50)*FRACUNIT;
			}

			else if(player->powers[pw_strength] || player->powers[pw_super]) // do you have super sneakers? Tails 02-28-2000
			{
				if (player->charspeed == 0)
				{
					movepushforward = cmd->forwardmove * (10*player->acceleration); // then go faster!! Tails 02-28-2000
				}
				else if (player->charspeed == 1)
				{
					movepushforward = cmd->forwardmove * (6*player->acceleration); // then go faster!! Tails 02-28-2000
				}
				else if (player->charspeed == 2)
				{
					movepushforward = cmd->forwardmove * (8*player->acceleration); // then go faster!! Tails 02-28-2000
				}
				else if (player->charspeed == 3)
				{
					movepushforward = cmd->forwardmove * (4*player->acceleration); // then go faster!! Tails 02-28-2000
				}
				else if (player->charspeed == 4)
				{
					movepushforward = cmd->forwardmove * (12*player->acceleration); // then go faster!! Tails 02-28-2000
				}
				else
				{
					movepushforward = cmd->forwardmove * (2*player->acceleration); // then go faster!! Tails 02-28-2000
				}
			}
			else // if not, then run normally Tails 02-28-2000
			{
				if (player->charspeed == 0)
				{
					movepushforward = cmd->forwardmove * (5*player->acceleration); // Changed by Tails: 9-14-99
				}
				else if (player->charspeed == 1)
				{
					movepushforward = cmd->forwardmove * (3*player->acceleration); // Changed by Tails: 9-14-99
				}
				else if (player->charspeed == 2)
				{
					movepushforward = cmd->forwardmove * (4*player->acceleration); // Changed by Tails: 9-14-99
				}
				else if (player->charspeed == 3)
				{
					movepushforward = cmd->forwardmove * (2*player->acceleration); // Changed by Tails: 9-14-99
				}
				else if (player->charspeed == 4)
				{
					movepushforward = cmd->forwardmove * (6*player->acceleration); // Changed by Tails: 9-14-99
				}
				else
				{
					movepushforward = cmd->forwardmove * (player->acceleration); // Changed by Tails: 9-14-99
				}
			}
        
			// allow very small movement while in air for gameplay
			if (!onground)
			{  
				movepushforward >>= 2; // Proper air movement - Changed by Tails: 9-13-99
			}

			// Allow a bit of movement while spinning Tails
			if (player->mfspinning)
			{
				if(player->mforward && cmd->forwardmove > 0)
					movepushforward = 0;
				else if(!player->mfstartdash)
					movepushforward=movepushforward/8;
				else
					movepushforward = 0;
			}

			if (player->powers[pw_super] || player->powers[pw_strength])
			{
				if(player->powers[pw_tailsfly])
				{
					if((player->speed < sflyspeed) && (player->mforward > 0) && (cmd->forwardmove > 0)) // Sonic's Speed
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if((player->mforward > 0) && (cmd->forwardmove < 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if((player->speed < sflyspeed) && (player->mbackward > 0) && (cmd->forwardmove < 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if((player->mbackward > 0) && (cmd->forwardmove > 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if ((player->mforward == 0) && (player->mbackward == 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
				}
				else if(player->mo->eflags & MF_UNDERWATER)
				{
					if((player->speed < swaterspeed) && (player->mforward > 0) && (cmd->forwardmove > 0)) // Sonic's Speed
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if((player->mforward > 0) && (cmd->forwardmove < 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if((player->speed < swaterspeed) && (player->mbackward > 0) && (cmd->forwardmove < 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if((player->mbackward > 0) && (cmd->forwardmove > 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if ((player->mforward == 0) && (player->mbackward == 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
				}
				else
				{
					if((player->speed < snormalspeed) && (player->mforward > 0) && (cmd->forwardmove > 0)) // Sonic's Speed
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if((player->mforward > 0) && (cmd->forwardmove < 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if((player->speed < snormalspeed) && (player->mbackward > 0) && (cmd->forwardmove < 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
					else if((player->mbackward > 0) && (cmd->forwardmove > 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
					 else if ((player->mforward == 0) && (player->mbackward == 0))
						P_Thrust (player->mo, movepushangle, movepushforward);
				}
			}
			else
			{
					if(player->powers[pw_tailsfly])
					{
						if((player->speed < flyspeed) && (player->mforward > 0) && (cmd->forwardmove > 0)) // Sonic's Speed
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if((player->mforward > 0) && (cmd->forwardmove < 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if((player->speed < flyspeed) && (player->mbackward > 0) && (cmd->forwardmove < 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if((player->mbackward > 0) && (cmd->forwardmove > 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if ((player->mforward == 0) && (player->mbackward == 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
					}
					else if (player->mo->eflags & MF_UNDERWATER)
					{
						if((player->speed < waterspeed) && (player->mforward > 0) && (cmd->forwardmove > 0)) // Sonic's Speed
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if((player->mforward > 0) && (cmd->forwardmove < 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if((player->speed < waterspeed) && (player->mbackward > 0) && (cmd->forwardmove < 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if((player->mbackward > 0) && (cmd->forwardmove > 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if ((player->mforward == 0) && (player->mbackward == 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
					}
					else
					{
						if((player->speed < normalspeed) && (player->mforward > 0) && (cmd->forwardmove > 0)) // Sonic's Speed
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if((player->mforward > 0) && (cmd->forwardmove < 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if((player->speed < normalspeed) && (player->mbackward > 0) && (cmd->forwardmove < 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if((player->mbackward > 0) && (cmd->forwardmove > 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
						else if ((player->mforward == 0) && (player->mbackward == 0))
							P_Thrust (player->mo, movepushangle, movepushforward);
					}
				}
			}
// Insert same code for sidemove here Tails
		if(cv_analog.value)
		{
			if (cmd->sidemove && !(player->gliding || player->exiting || (player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_invisibility])))
			{
				if(player->climbing)
				{
					P_InstaThrust (player->mo, player->mo->angle-ANG90, (cmd->sidemove/10)*FRACUNIT);
				}

				else if(player->powers[pw_strength] || player->powers[pw_super]) // do you have super sneakers? Tails 02-28-2000
				{
					if (player->charspeed == 0)
					{
						movepushforward = cmd->sidemove * (10*player->acceleration); // then go faster!! Tails 02-28-2000
					}
					else if (player->charspeed == 1)
					{
						movepushforward = cmd->sidemove * (6*player->acceleration); // then go faster!! Tails 02-28-2000
					}
					else if (player->charspeed == 2)
					{
						movepushforward = cmd->sidemove * (8*player->acceleration); // then go faster!! Tails 02-28-2000
					}
					else if (player->charspeed == 3)
					{
						movepushforward = cmd->sidemove * (4*player->acceleration); // then go faster!! Tails 02-28-2000
					}
					else if (player->charspeed == 4)
					{
						movepushforward = cmd->sidemove * (12*player->acceleration); // then go faster!! Tails 02-28-2000
					}
					else
					{
						movepushforward = cmd->sidemove * (2*player->acceleration); // then go faster!! Tails 02-28-2000
					}
				}
				else // if not, then run normally Tails 02-28-2000
				{
					if (player->charspeed == 0)
					{
						movepushforward = cmd->sidemove * (5*player->acceleration); // Changed by Tails: 9-14-99
					}
					else if (player->charspeed == 1)
					{
						movepushforward = cmd->sidemove * (3*player->acceleration); // Changed by Tails: 9-14-99
					}
					else if (player->charspeed == 2)
					{
						movepushforward = cmd->sidemove * (4*player->acceleration); // Changed by Tails: 9-14-99
					}
					else if (player->charspeed == 3)
					{
						movepushforward = cmd->sidemove * (2*player->acceleration); // Changed by Tails: 9-14-99
					}
					else if (player->charspeed == 4)
					{
						movepushforward = cmd->sidemove * (6*player->acceleration); // Changed by Tails: 9-14-99
					}
					else
					{
						movepushforward = cmd->sidemove * (player->acceleration); // Changed by Tails: 9-14-99
					}
				}
        
				// allow very small movement while in air for gameplay
				if (!onground && !cv_nights.value)
				{  
					movepushforward >>= 2; // Proper air movement - Changed by Tails: 9-13-99
				}

				// Allow a bit of movement while spinning Tails
				if (player->mfspinning)
				{
					if(!player->mfstartdash)
						movepushforward=movepushforward/8;
					else
						movepushforward = 0;
				}

				if (player->powers[pw_super] || player->powers[pw_strength])
				{
					if(player->powers[pw_tailsfly])
					{
						if((player->speed < sflyspeed) && (player->mforward > 0) && (cmd->sidemove > 0)) // Sonic's Speed
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mforward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->speed < sflyspeed) && (player->mbackward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mbackward > 0) && (cmd->sidemove > 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if ((player->mforward == 0) && (player->mbackward == 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
					}
					else if(player->mo->eflags & MF_UNDERWATER)
					{
						if((player->speed < swaterspeed) && (player->mforward > 0) && (cmd->sidemove > 0)) // Sonic's Speed
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mforward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->speed < swaterspeed) && (player->mbackward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mbackward > 0) && (cmd->sidemove > 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if ((player->mforward == 0) && (player->mbackward == 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
					}
					else
					{
						if((player->speed < snormalspeed) && (player->mforward > 0) && (cmd->sidemove > 0)) // Sonic's Speed
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mforward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->speed < snormalspeed) && (player->mbackward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mbackward > 0) && (cmd->sidemove > 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						 else if ((player->mforward == 0) && (player->mbackward == 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
					}
				}
				else
				{
					if(player->powers[pw_tailsfly])
					{
						if((player->speed < flyspeed) && (player->mforward > 0) && (cmd->sidemove > 0)) // Sonic's Speed
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mforward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->speed < flyspeed) && (player->mbackward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mbackward > 0) && (cmd->sidemove > 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if ((player->mforward == 0) && (player->mbackward == 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
					}
					else if (player->mo->eflags & MF_UNDERWATER)
					{
						if((player->speed < waterspeed) && (player->mforward > 0) && (cmd->sidemove > 0)) // Sonic's Speed
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mforward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->speed < waterspeed) && (player->mbackward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mbackward > 0) && (cmd->sidemove > 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if ((player->mforward == 0) && (player->mbackward == 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
					}
					else
					{
						if((player->speed < normalspeed) && (player->mforward > 0) && (cmd->sidemove > 0)) // Sonic's Speed
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mforward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->speed < normalspeed) && (player->mbackward > 0) && (cmd->sidemove < 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if((player->mbackward > 0) && (cmd->sidemove > 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
						else if ((player->mforward == 0) && (player->mbackward == 0))
							P_Thrust (player->mo, movepushsideangle, movepushforward);
					}
				}
			}
		}
		else
		{
			if(player->climbing)
				P_InstaThrust (player->mo, player->mo->angle-ANG90, (cmd->sidemove/10)*FRACUNIT);

			else if (cmd->sidemove && !player->gliding && !player->exiting && !player->climbing && !(player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_invisibility])) // Tails 04-12-2001
			{
				if(player->powers[pw_strength] || player->powers[pw_super])
				{
					if(player->mfstartdash || player->mfspinning)
						movepushside = 0;
					else if (player->charspeed == 0)
						movepushside = cmd->sidemove * (10*player->acceleration);
					else if(player->charspeed == 1)
						movepushside = cmd->sidemove * (6*player->acceleration);
					else if(player->charspeed == 2)
						movepushside = cmd->sidemove * (8*player->acceleration);
					else
						movepushside = cmd->sidemove * (4*player->acceleration);
				}
				else
				{
					if(player->mfstartdash || player->mfspinning)
						movepushside = 0;
					else if (player->charspeed == 0)
						movepushside = cmd->sidemove * (5*player->acceleration);
					else if(player->charspeed == 1)
						movepushside = cmd->sidemove * (3*player->acceleration);
					else if(player->charspeed == 2)
						movepushside = cmd->sidemove * (4*player->acceleration);
					else
						movepushside = cmd->sidemove * (2*player->acceleration);
				}

			if (!onground)
			{
				movepushside >>= 2;
			}
				
			// Allow a bit of movement while spinning Tails
			if (player->mfspinning)
			{
				if(!player->mfstartdash)
					movepushforward=movepushforward/8;
				else
					movepushforward = 0;
			}

			// Finally move the player now that his speed/direction has been decided.
			if((player->speed < 26) && (player->charspeed == 0)) // Sonic's Speed
				P_Thrust (player->mo, movepushsideangle, movepushside);
			else if((player->speed < 17) && (player->charspeed == 1)) // Tails's Speed
				P_Thrust (player->mo, movepushsideangle, movepushside);
			else if((player->speed < 21) && (player->charspeed == 2)) // Knuckles's Speed
				P_Thrust (player->mo, movepushsideangle, movepushside);
			else if((player->speed < 13) && (player->charspeed > 2)) // Other's Speed
				P_Thrust (player->mo, movepushsideangle, movepushside);
			}
			else if (player->climbing)
			{
				P_InstaThrust (player->mo, movepushsideangle, (cmd->sidemove/10)*FRACUNIT);
			}
		}		

/////////////////////////
// MOVEMENT ANIMATIONS //
/////////////////////////

// Flag variables so it's easy to check
// what state the player is in. Tails 08-19-2000
		if (player->mo->state == &states[S_PLAY_RUN1] || player->mo->state == &states[S_PLAY_RUN2] || player->mo->state == &states[S_PLAY_RUN3] || player->mo->state == &states[S_PLAY_RUN4] || player->mo->state == &states[S_PLAY_RUN5] || player->mo->state == &states[S_PLAY_RUN6] || player->mo->state == &states[S_PLAY_RUN7] || player->mo->state == &states[S_PLAY_RUN8])
		{
			player->walking = 1;
			player->running = player->spinning = 0;
		}
		else if (player->mo->state == &states[S_PLAY_SPD1] || player->mo->state == &states[S_PLAY_SPD2] || player->mo->state == &states[S_PLAY_SPD3] || player->mo->state == &states[S_PLAY_SPD4])
		{
			player->running = 1;
			player->walking = player->spinning = 0;
		}
		else if (player->mo->state == &states[S_PLAY_ATK1] || player->mo->state == &states[S_PLAY_ATK2] || player->mo->state == &states[S_PLAY_ATK3] || player->mo->state == &states[S_PLAY_ATK4])
		{
			player->spinning = 1;
			player->running = player->walking = 0;
		}
		else
			player->walking = player->running = player->spinning = 0;

		if(cmd->forwardmove || cmd->sidemove)
		{
	// If the player is moving fast enough,
	// break into a run!
			if((player->speed > 18) && player->walking && (onground))
				P_SetMobjState (player->mo, S_PLAY_SPD1);

	// Otherwise, just walk.
			else if((player->rmomx || player->rmomy) && (player->mo->state == &states[S_PLAY] || player->mo->state == &states[S_PLAY_TAP1] || player->mo->state == &states[S_PLAY_TAP2] || player->mo->state == &states[S_PLAY_TEETER1] || player->mo->state == &states[S_PLAY_TEETER2])/* && !(player->powers[pw_super])*/)
				P_SetMobjState (player->mo, S_PLAY_RUN1);
		}

// Prevent Super Sonic from showing Tails 03-25-2001
		if(player->skin==0 && (player->mo->state == &states[S_PLAY_SPC1] || player->mo->state == &states[S_PLAY_SPC2] || player->mo->state == &states[S_PLAY_SPC3] || player->mo->state == &states[S_PLAY_SPC4] || player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL2]))
			P_SetMobjState(player->mo, S_PLAY);

// Adjust the player's animation speed to
// match their velocity.
		if(onground) // Only if on the ground.
		{
			if(player->walking)
			{
				if(player->speed > 6)
					states[player->mo->state->nextstate].tics = 2;
				else if(player->speed > 3)
					states[player->mo->state->nextstate].tics = 3;
				else
					states[player->mo->state->nextstate].tics = 4;
			}
			else if(player->running)
			{
				if(player->speed > 35)
					states[player->mo->state->nextstate].tics = 1;
				else
					states[player->mo->state->nextstate].tics = 2;
			}
		}
		else if(player->mo->state == &states[S_PLAY_FALL1] || player->mo->state == &states[S_PLAY_FALL2])
		{
			fixed_t speed;
			speed = abs(player->mo->momz);
			if(speed < 10*FRACUNIT)
				states[player->mo->state->nextstate].tics = 4;
			else if(speed < 20*FRACUNIT)
				states[player->mo->state->nextstate].tics = 3;
			else if(speed < 30*FRACUNIT)
				states[player->mo->state->nextstate].tics = 2;
			else/* if(speed < 20*FRACUNIT)*/
				states[player->mo->state->nextstate].tics = 1;
		}

		if(player->spinning)
		{
			if(player->speed > 10)
				states[player->mo->state->nextstate].tics = 1;
			else
				states[player->mo->state->nextstate].tics = 2;
		}

		// Flash player after being hit. Tails 03-07-2000
		if(player->powers[pw_invisibility] & 1)
			player->mo->flags |= MF_SHADOW;
		else
			player->mo->flags &= ~MF_SHADOW;

		// "If the player is Super Sonic and is pressing the
		// forward/back or left/right keys and running, play
		// Super Sonic's running animation."
/*		if  (player->powers[pw_super] && (cmd->forwardmove || cmd->sidemove)
			&& player->running)
		        P_SetMobjState (player->mo, S_PLAY_ABL1);*/

		// If your running animation is playing, and you're
		// going too slow, switch back to the walking frames.
		if(player->running && !(player->speed > 18))
			P_SetMobjState (player->mo, S_PLAY_RUN1);

		// If Springing, but travelling DOWNWARD, change back!
		if(player->mo->state == &states[S_PLAY_PLG1] && player->mo->momz < 0)
			P_SetMobjState (player->mo, S_PLAY_FALL1);
/*
		// If Springing but on the ground, change back!
		else if((player->mo->state == &states[S_PLAY_PLG1] || player->mo->state == &states[S_PLAY_FALL1] || player->mo->state == &states[S_PLAY_FALL2]) && !player->mo->momz)
			P_SetMobjState(player->mo, S_PLAY);
*/
		// If you are stopped and are still walking, stand still!
		if(!(player->mo->momx && player->mo->momy) && !player->mo->momz && player->walking)
			P_SetMobjState(player->mo, S_PLAY);

		// Ledge teetering. Check if any nearby sectors are low enough from your current one.
		if(!(player->mo->momx && player->mo->momy) && (player->mo->z == player->mo->floorz) && !player->mo->momz && (player->mo->state == &states[S_PLAY]))
		{
			boolean teeter;
			subsector_t* a = R_PointInSubsector(player->mo->x + 5*FRACUNIT, player->mo->y + 5*FRACUNIT);
			subsector_t* b = R_PointInSubsector(player->mo->x - 5*FRACUNIT, player->mo->y + 5*FRACUNIT);
			subsector_t* c = R_PointInSubsector(player->mo->x + 5*FRACUNIT, player->mo->y - 5*FRACUNIT);
			subsector_t* d = R_PointInSubsector(player->mo->x - 5*FRACUNIT, player->mo->y - 5*FRACUNIT);
			teeter = false;
			if (a->sector->ffloors)
			{
	 			ffloor_t* rover;
				for(rover = a->sector->ffloors; rover; rover = rover->next)
				{
					if(*rover->topheight < player->mo->z - 32*FRACUNIT
						|| (*rover->bottomheight > player->mo->z + player->mo->height
						&& player->mo->z > a->sector->floorheight + 32*FRACUNIT))
					{
						teeter = true;
					}
					else
					{
						teeter = false;
						break;
					}
				}
			}
			else if (b->sector->ffloors)
			{
	 			ffloor_t* rover;
				for(rover = a->sector->ffloors; rover; rover = rover->next)
				{
					if(*rover->topheight < player->mo->z - 32*FRACUNIT
						|| (*rover->bottomheight > player->mo->z + player->mo->height
						&& player->mo->z > b->sector->floorheight + 32*FRACUNIT))
					{
						teeter = true;
					}
					else
					{
						teeter = false;
						break;
					}
				}
			}
			else if (c->sector->ffloors)
			{
	 			ffloor_t* rover;
				for(rover = a->sector->ffloors; rover; rover = rover->next)
				{
					if(*rover->topheight < player->mo->z - 32*FRACUNIT
						|| (*rover->bottomheight > player->mo->z + player->mo->height
						&& player->mo->z > c->sector->floorheight + 32*FRACUNIT))
					{
						teeter = true;
					}
					else
					{
						teeter = false;
						break;
					}
				}
			}
			else if (d->sector->ffloors)
			{
	 			ffloor_t* rover;
				for(rover = a->sector->ffloors; rover; rover = rover->next)
				{
					if(*rover->topheight < player->mo->z - 32*FRACUNIT
						|| (*rover->bottomheight > player->mo->z + player->mo->height
						&& player->mo->z > d->sector->floorheight + 32*FRACUNIT))
					{
						teeter = true;
					}
					else
					{
						teeter = false;
						break;
					}
				}
			}
			else if(a->sector->floorheight < player->mo->floorz - 32*FRACUNIT
				|| b->sector->floorheight < player->mo->floorz - 32*FRACUNIT
				|| c->sector->floorheight < player->mo->floorz - 32*FRACUNIT
				|| d->sector->floorheight < player->mo->floorz - 32*FRACUNIT)
					teeter = true;

			if(teeter)
				P_SetMobjState(player->mo, S_PLAY_TEETER1);
			else if(!(player->mo->state == &states[S_PLAY] || player->mo->state == &states[S_PLAY_TAP1] || player->mo->state == &states[S_PLAY_TAP2]))
				P_SetMobjState(player->mo, S_PLAY);
		}

//////////////////
//GAMEPLAY STUFF//
//////////////////

// Check if player is in the exit sector.
// If so, begin the level end process. Tails 12-15-2000
		if(player->mo->subsector->sector->special == 982 && !player->exiting)
		{
				if(cv_gametype.value == 2) // If in Race Mode, allow
				{                          // a 60-second wait ala Sonic 2. Tails 04-25-2001
					for(i=0; i<MAXPLAYERS; i++) // Tails 04-25-2001
					{ // Tails 04-25-2001
						if(playeringame[i])
							players[i].countdown = 60*TICRATE; // Tails 04-25-2001
					} // Tails 04-25-2001
					player->exiting = 3*TICRATE;
					player->countdown2 = 71*TICRATE;

					// Check if all the players in the race have finished. If so, end the level.
					for(i = 0; i < MAXPLAYERS; i++)
					{
						if(playeringame[i])
						{
							if(!players[i].exiting)
								break;
						}
					}

					if(i == MAXPLAYERS)  // finished
						player->exiting = 2.8*TICRATE + 2;
				}
				else
					player->exiting = 2.8*TICRATE + 2;
		}


		// If 10 seconds are left on the timer,
		// begin the drown music for countdown! // Tails 04-25-2001
		if(player->countdown == 11*TICRATE)
		{
			if(player==&players[consoleplayer])
				{
					S_ChangeMusic(mus_drown, false);
					I_PlayCD (35, false);
				}
		}

		// If you've hit the countdown and you haven't made
		//  it to the exit, you're a goner! Tails 04-25-2001
		else if(player->countdown == 1 && !player->exiting)
		{
			P_DamageMobj(player->mo, NULL, NULL, 10000);
			player->lives = 0;
			if(player==&players[consoleplayer])
			{
				S_ChangeMusic(mus_gmover, false);
				I_PlayCD(38, false);
			}
		}

		// If it is set, start subtracting Tails 12-15-2000
		if(player->exiting && player->exiting < 3*TICRATE)
		{
			player->exiting--;
		}

		if(player->exiting == 2 || player->countdown2 == 1)
		{
			SendNetXCmd(XD_EXITLEVEL,NULL,0); // Let everybody know it's time to exit
			if(!cv_gametype.value == 2)		  // so an inconsistency doesn't occur!
				leveltime -= 2.8*TICRATE;	  // Tails 12-15-2000
		}

		// Make sure you're not "jumping" on the ground Tails 11-05-2000
		if(onground && player->mfjumped == 1 && !player->mo->momz)
		{
			player->mfjumped = 0;
			P_SetMobjState(player->mo, S_PLAY);
		}
/*
		// Make sure player is in a ball when jumped Tails 03-13-2000
		if (player->mfjumped && !(player->gliding) && !(player->mo->state == &states[S_PLAY_ATK1] || player->mo->state == &states[S_PLAY_ATK2] || player->mo->state == &states[S_PLAY_ATK3] || player->mo->state == &states[S_PLAY_ATK4]))
		{
			P_SetMobjState(player->mo, S_PLAY_ATK1);
		}   
*/
		// Check if the camera is underwater
		// (used for palette changes) Tails 11-02-2000
		if((camera.chase && (camera.mo->z + 8*FRACUNIT) < camera.mo->waterz)
			|| (!camera.chase && (player->mo->z + player->viewheight) < player->mo->waterz))
			player->camunder = 1;
		else
			player->camunder = 0;

		// Cap the speed limit on a spindash Tails 11-01-2000
		// Up the 60*FRACUNIT number to boost faster, you speed demon you!
		// Note: You must change the MAXMOVE variable in p_local.h to see any effect over 60.
		if(player->dashspeed > 60*FRACUNIT)
			player->dashspeed = 60*FRACUNIT;

		else if(player->dashspeed > 0 && player->dashspeed < 15*FRACUNIT) // Don't let the spindash
			player->dashspeed = 15*FRACUNIT;						 // counter get too high!

		// Fly counter for Tails.
		if(player->fly1 && player->powers[pw_tailsfly])
		{
			if(player->mo->momz < 5*FRACUNIT)
			player->mo->momz += FRACUNIT/2;
			player->fly1--;
		}

// Glide MOMZ Tails 11-17-2000
// AKA my own gravity. =)
		if(player->gliding)
		{
			if(player->mo->momz == -2*FRACUNIT)
				player->mo->momz = -2*FRACUNIT;
			else if(player->mo->momz < -2*FRACUNIT)
				player->mo->momz += FRACUNIT*3/4;

			P_InstaThrust(player->mo, player->mo->angle, 20*FRACUNIT + player->glidetime*1000);
			player->glidetime++;

			if(!player->jumpdown)    // If not holding the jump button
			{
				player->gliding = 0; // down, stop gliding.
				P_SetMobjState(player->mo, S_PLAY_ATK1);
			}
		}
		else if(player->climbing) // 'Deceleration' for climbing on walls.
		{
			if(player->mo->momz > 0)
				player->mo->momz -= .5*FRACUNIT;
			else if(player->mo->momz < 0)
				player->mo->momz += .5*FRACUNIT;
		}

		if(player->charability != 2) // If you can't glide, then why
		{							  // the heck would you be gliding?
			player->gliding = 0;
			player->glidetime = 0;
			player->climbing = 0;
		}

// Jump out of water stuff Tails 12-06-2000
		if((player->mo->eflags & ~MF_UNDERWATER
			&& player->mo->momz > 0
			&& player->mo->z+(player->mo->height>>1) > player->mo->waterz
			&& player->mo->z+(player->mo->height>>1) - player->mo->momz < player->mo->waterz)
			|| (player->mo->eflags & MF_UNDERWATER
			&& player->mo->momz < 0
			&& player->mo->z+(player->mo->height>>1) < player->mo->waterz
			&& player->mo->z+(player->mo->height>>1) - player->mo->momz > player->mo->waterz))
		{
				if(player->mo->momz > 0)
					player->mo->momz = player->mo->momz*1.706783369803; // Give the player a little out-of-water boost.

				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->waterz, MT_SPLISH); // Spawn a splash
				S_StartSound(player->mo, sfx_splish); // And make a sound!
//				player->powers[pw_underwater] = 30*TICRATE + 1; // Take a breath
		}


		// If you're running fast enough, you can create splashes as you run in shallow water.
		if(player->mo->z + player->mo->height >= player->mo->waterz && player->mo->z <= player->mo->waterz && player->speed > 18 && leveltime % 5 == 1 && player->mo->momz == 0)
		{
			S_StartSound(P_SpawnMobj(player->mo->x, player->mo->y, player->mo->waterz, MT_SPLISH), sfx_wslap);
		}

//////////////////////////
// RING & SCORE			//
// EXTRA LIFE BONUSES	//
//////////////////////////

		if(!(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
			|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
			|| gamemap == SSSTAGE7 || cv_gametype.value == 1 || cv_gametype.value == 3
			|| cv_gametype.value == 4)) // Don't do it in special stages.
		{
			if ((player->health > 100) && (!player->xtralife))
			{
				player->lives += 1;
				if(player==&players[consoleplayer])
				{
				S_StopMusic();
				S_ChangeMusic(mus_xtlife, false);
				I_PlayCD(37, false);
				}
				player->powers[pw_extralife] = 4*TICRATE + 1;
				player->xtralife = 1;
			}

			if ((player->health > 200) && (player->xtralife > 0 && player->xtralife < 2))
			{
				player->lives += 1;
				if(player==&players[consoleplayer])
				{
				S_StopMusic();
				S_ChangeMusic(mus_xtlife, false);
				I_PlayCD(37, false);
				}
				player->powers[pw_extralife] = 4*TICRATE + 1;
				player->xtralife = 2;
			}
		}

		if(player->score >= 50000+player->xtralife2 && !(cv_gametype.value == 1
														|| cv_gametype.value == 3
														|| cv_gametype.value == 4))
		{
			player->lives++;
			if(player==&players[consoleplayer])
			{
			S_StopMusic();
			S_ChangeMusic(mus_xtlife, false);
			I_PlayCD(37, false);
			}
			player->powers[pw_extralife] = 4*TICRATE + 1;
			player->xtralife2 += 50000;
		}

//////////////////////////
// SUPER SONIC STUFF	//
//////////////////////////

// Does player have all emeralds? If so, flag the "Ready For Super!" Tails 04-08-2000
		if((player->emerald1) && (player->emerald2) && (player->emerald3) && (player->emerald4) && (player->emerald5) && (player->emerald6) && (player->emerald7) && (player->health > 50))
			player->superready = true;
		else
			player->superready = false;



		if(player->powers[pw_super])
		{
			// If you're super and not Sonic, de-superize!
			if(!(player->skin == 0))
				player->powers[pw_super] = 0;

			// Deplete one ring every second while super
			if((leveltime % TICRATE == 0) && !(player->exiting))
			{
				player->health--;
				player->mo->health--;
			}

			// Ran out of rings while super!
			if((player->powers[pw_super]) && (player->health <= 1))
			{
				player->powers[pw_super] = false;
				player->health = 1;
				player->mo->health = 1;
	//			P_SetMobjState(player->mo, S_PLAY); // Return to normal

				// If you had a shield, restore its visual significance.
				if(player->powers[pw_blueshield])
					P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BLUEORB)->target = player->mo;
				else if(player->powers[pw_yellowshield])
					P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_YELLOWORB)->target = player->mo;
				else if(player->powers[pw_greenshield])
					P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GREENORB)->target = player->mo;
				else if(player->powers[pw_blackshield])
					P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BLACKORB)->target = player->mo;
			}

			// If Super Sonic is moving fast enough, run across the water!
	/*		if((player->powers[pw_super]) && (player->mo->z < player->mo->waterz+10*FRACUNIT) && (player->mo->z > player->mo->waterz-10*FRACUNIT) && (cmd->forwardmove) && (player->rmomx) && (player->rmomy) && (player->mo->momz < 0) && (player->speed > 10))
			{
				 player->mo->z = player->mo->waterz;
				 player->mo->momz = 0;
	//			 P_SpawnSplash (player->mo, (player->mo->z));
			}*/
		}

/////////////////////////
//Special Music Changes//
/////////////////////////

		if(player->powers[pw_extralife] == 1) // Extra Life!
		{
			if(player->powers[pw_invulnerability] > 1)
			{
				if(player==&players[consoleplayer])
				{
					S_StopMusic();
					S_ChangeMusic(mus_invinc, false);
					I_PlayCD(36, false);
				}
			}
			else if(player==&players[consoleplayer] && player->powers[pw_underwater] <= 12*TICRATE + 1 && player->powers[pw_underwater] > 0)
			{
				S_ChangeMusic(mus_drown, false); // Tails 03-14-2000
				I_PlayCD (35, false);
			}
			else
			{
				S_ChangeMusic(mus_runnin + gamemap - 1, 1); // Tails 03-14-2000
				I_PlayCD (gamemap + 1, true);
			}
		}

///////////////////////////
//LOTS OF UNDERWATER CODE//
///////////////////////////

		// Spawn Sonic's bubbles Tails 03-07-2000
		if(player->mo->eflags & MF_UNDERWATER && !(P_Random() % 16) && !(player->powers[pw_greenshield]))
		{
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height / 1.25), MT_SMALLBUBBLE);
		}
		else if(player->mo->eflags & MF_UNDERWATER && !(P_Random() % 96) && !(player->powers[pw_greenshield]))
		{
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height / 1.25), MT_MEDIUMBUBBLE);
		}

		// Display the countdown drown numbers!
		if (player->powers[pw_underwater] == 12*TICRATE + 1 || player->powers[pw_underwater] == 11*TICRATE + 1)
		{            
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height), MT_FIVE);
		}
		else if (player->powers[pw_underwater] == 10*TICRATE + 1 || player->powers[pw_underwater] == 9*TICRATE + 1)
		{            
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height), MT_FOUR);
		}
		else if (player->powers[pw_underwater] == 8*TICRATE + 1 || player->powers[pw_underwater] == 7*TICRATE + 1)
		{            
		    P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height), MT_THREE);
		}
		else if (player->powers[pw_underwater] == 6*TICRATE + 1 || player->powers[pw_underwater] == 5*TICRATE + 1)
		{            
		    P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height), MT_TWO);
		}
		else if (player->powers[pw_underwater] == 4*TICRATE + 1 || player->powers[pw_underwater] == 3*TICRATE + 1)
		{            
		    P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height), MT_ONE);
		}
		else if (player->powers[pw_underwater] == 2*TICRATE + 1 || player->powers[pw_underwater] == TICRATE + 1)
		{            
		    P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z + (player->mo->height), MT_ZERO);
		}
		// Underwater timer runs out Tails 03-05-2000
		else if (player->powers[pw_underwater] == 1)
		{
		    P_DamageMobj(player->mo, NULL, NULL, 10000);

			if(player == &players[consoleplayer])
			{
				S_ChangeMusic(mus_runnin + gamemap - 1, 1);
				I_PlayCD(gamemap + 1, true);
			}
		}

		if(player == &players[consoleplayer])
		{
			if (((player->mo->z+player->mo->height/2) >= player->mo->waterz) && player->powers[pw_underwater])
			{
				if(!(player->powers[pw_invulnerability] > 1 || player->powers[pw_extralife] > 1) && (player->powers[pw_underwater] <= 12*TICRATE + 1))
				{
					S_ChangeMusic(mus_runnin + gamemap - 1, 1); // Tails 04-04-2000
					I_PlayCD(gamemap + 1, true); // Tails 04-05-2000
				}
				else if ((player->powers[pw_super] == false) && (player->powers[pw_invulnerability] > 1) && (player->powers[pw_underwater] <= 12*TICRATE + 1) && (player->powers[pw_extralife] <= 1))
				{
					S_ChangeMusic(mus_invinc, false);
					I_PlayCD(36, false);
				}

				player->powers[pw_underwater] = 0;
			}

			if (player->powers[pw_underwater] == 12*TICRATE + 1)
			{            
				S_StopMusic();
				S_ChangeMusic(mus_drown, false);
				I_PlayCD(35, false);
			}

			if (player->powers[pw_underwater] == 25*TICRATE + 1)
			{            
				S_StartSound (0, sfx_wtrdng);
			}
			else if (player->powers[pw_underwater] == 20*TICRATE + 1)
			{            
				S_StartSound (0, sfx_wtrdng);
			}
			else if (player->powers[pw_underwater] == 15*TICRATE + 1)
			{            
				S_StartSound (0, sfx_wtrdng);
			}
		}

////////////////
//TAILS FLYING//
////////////////

		// If not in a fly position, don't think you're flying! Tails 03-05-2000
		if (!(player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL2]))
			player->powers[pw_tailsfly] = 0;

		if(player->charability == 1)
		{
			// Tails Put-Put noise Tails 03-05-2000
			if (player->mo->state == &states[S_PLAY_ABL1] && player->powers[pw_tailsfly])
			{
				S_StartSound (player->mo, sfx_putput);
			}

			// Tails-gets-tired Stuff
			if ((player->powers[pw_tailsfly] == 1) || (player->powers[pw_tailsfly]== 0 && (player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL2])))
			{
				P_SetMobjState (player->mo, S_PLAY_SPC4);
			}
			if ((player->mo->state->nextstate == S_PLAY_SPC1 || player->mo->state->nextstate == S_PLAY_SPC3) && !player->powers[pw_tailsfly])
			{
				S_StartSound (player->mo, sfx_pudpud);
			}
		}

		// Cut momentum in half when you hit the ground and
		// aren't pressing any controls. Tails 03-03-2000
		if ((player->mo->eflags & MF_JUSTHITFLOOR) && !(cmd->forwardmove) && !(player->cmomx || player->cmomy))
		{
			player->mo->momx = player->mo->momx/2;
			player->mo->momy = player->mo->momy/2;
		}

		// Uncomment this to invoke a 10-minute time limit on levels.
		/*if(leveltime > 20999) // one tic off so the time doesn't display 10:00
		P_DamageMobj(player->mo, NULL, NULL, 10000);*/

		// Spawn Invincibility Sparkles
		if (player->powers[pw_invulnerability] && leveltime % 5 == 1 && player->powers[pw_super] == false)
		{
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z, MT_IVSP);
		}
		else if ((player->powers[pw_super]) && (cmd->forwardmove) && (leveltime % TICRATE == 0) && (player->mo->momx || player->mo->momy))
		{
			P_SpawnMobj (player->mo->x, player->mo->y, player->mo->z, MT_SUPERSPARK);
		}

		// Resume normal music stuff. Tails
		if ((player->powers[pw_invulnerability] == 1))
		{
			if(!(player->powers[pw_extralife] > 1) && ((player->powers[pw_underwater] > 12*TICRATE + 1) || (!player->powers[pw_underwater])))
			{
				S_ChangeMusic(mus_runnin + gamemap - 1, 1);
				I_PlayCD(gamemap + 1, true);
			}

			// If you have a shield, restore the visual significance.
			if(player->powers[pw_blueshield])
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BLUEORB)->target = player->mo;
			else if(player->powers[pw_yellowshield])
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_YELLOWORB)->target = player->mo;
			else if(player->powers[pw_greenshield])
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GREENORB)->target = player->mo;
			else if(player->powers[pw_blackshield])
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BLACKORB)->target = player->mo;
		}

		// Show the "THOK!" graphic when spinning quickly across the ground. Tails 11-01-2000
		if(player->mfspinning && player->speed > 15 && !player->mfjumped)
		{
			if(player->skincolor == 0)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GTHOK);
			else if(player->skincolor == 1)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GRTHOK);
			else if(player->skincolor == 2)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_PCTHOK);
			else if(player->skincolor == 3)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DRTHOK);
			else if(player->skincolor == 4)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_STHOK);
			else if(player->skincolor == 5)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_OTHOK);
			else if(player->skincolor == 6)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_RTHOK);
			else if(player->skincolor == 7)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BTHOK);
			else if(player->skincolor == 8)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DBTHOK);
			else if(player->skincolor == 9)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_PTHOK);
			else if(player->skincolor == 10)
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BGTHOK);
			else
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BTHOK);
		}


////////////////////////////
//SPINNING AND SPINDASHING//
////////////////////////////

		// If the player isn't on the ground, make sure they aren't in a "starting dash" position.
		if (!onground)
		   {
		     player->mfstartdash = 0;
		     player->dashspeed = 0;
		   }

		//Spinning and Spindashing
		if(cmd->buttons &BT_USE && !player->exiting && !(player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_invisibility])) // subsequent revs
		{
			if(player->speed < 5 && !player->mo->momz && onground && !player->usedown && !player->mfspinning)
			{
				player->scoreadd = 0; // Tails 11-03-2000
				player->mo->momx = player->cmomx;
				player->mo->momy = player->cmomy;
				player->mfstartdash = 1;
				player->mfspinning = 1;
				player->dashspeed+=FRACUNIT; // more speed as you rev more Tails 03-01-2000
				P_SetMobjState (player->mo, S_PLAY_ATK1);
			   player->usedown = true;
			}
			else if(player->mfstartdash)
			{
				player->dashspeed+=FRACUNIT;
				if (leveltime & 1)
				{
					S_StartSound (player->mo, sfx_spndsh); // Make the rev sound!
					// Now spawn the color thok circle.
					if(player->skincolor == 0)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GTHOK);
					else if(player->skincolor == 1)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GRTHOK);
					else if(player->skincolor == 2)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_PCTHOK);
					else if(player->skincolor == 3)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DRTHOK);
					else if(player->skincolor == 4)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_STHOK);
					else if(player->skincolor == 5)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_OTHOK);
					else if(player->skincolor == 6)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_RTHOK);
					else if(player->skincolor == 7)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BTHOK);
					else if(player->skincolor == 8)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DBTHOK);
					else if(player->skincolor == 9)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_PTHOK);
					else if(player->skincolor == 10)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BGTHOK);
					else
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BTHOK);
				}
			}
			// If not moving up or down, and travelling faster than a speed of four while not holding down the spin button and not spinning.
			// AKA Just go into a spin on the ground, you idiot. ;)
			else if(!player->mo->momz && player->speed > 4 && !player->usedown && !player->mfspinning)
			{
					player->scoreadd = 0; // Tails 11-03-2000
					player->mfspinning = 1;
					P_SetMobjState (player->mo, S_PLAY_ATK1);
					S_StartSound (player->mo, sfx_spin);
					player->usedown = true;
			}
		}

		// If you're on the ground and spinning too slowly, get up.
		if(onground && player->mfspinning && (player->rmomx < 5*FRACUNIT && player->rmomx > -5*FRACUNIT) && (player->rmomy < 5*FRACUNIT && player->rmomy > -5*FRACUNIT) && !player->mfstartdash)
		{
			player->mfspinning = 0;
			P_SetMobjState(player->mo, S_PLAY);
			player->mo->momx = player->cmomx;
			player->mo->momy = player->cmomy;
			player->scoreadd = 0; // Tails 11-03-2000
		}

		// Catapult the player from a spindash rev!
		if(onground && !player->usedown && player->dashspeed && player->mfstartdash && player->mfspinning)
		{
			player->mfstartdash = 0;
			P_InstaThrust (player->mo, player->mo->angle, 1*player->dashspeed); // catapult forward ho!! Tails 02-27-2000
			S_StartSound (player->mo, sfx_zoom);
			player->dashspeed = 0;
		}

		//added:22-02-98: jumping
		if (cmd->buttons & BT_JUMP && !player->jumpdown && !player->exiting && !(player->mo->state == &states[S_PLAY_PAIN] && player->powers[pw_invisibility]))
		{
			// can't jump while in air, can't jump while jumping
			if (onground || player->climbing)// || (player->mo->eflags & MF_UNDERWATER)) )
			{
				if(player->climbing)
				{
					// Jump this high.
					if(player->powers[pw_super])
						player->mo->momz = 5*FRACUNIT;
					else if (player->mo->eflags & MF_UNDERWATER)
						player->mo->momz = 2*FRACUNIT;
					else
						player->mo->momz = 3.75*FRACUNIT;

					player->mo->angle = player->mo->angle - ANG180; // Turn around from the wall you were climbing.

					if (player==&players[consoleplayer])
						localangle = player->mo->angle; // Adjust the local control angle.

					player->climbing = 0; // Stop climbing, duh!
					P_Thrust(player->mo, player->mo->angle, 6*FRACUNIT); // Jump off the wall.
				}
				else if(!(player->mfjumped)) // Tails 9-15-99 Spin Attack
				{
					// Jump this high.
					if(player->powers[pw_super])
						player->mo->momz = 13*FRACUNIT;
					else if (player->mo->eflags & MF_UNDERWATER)
						player->mo->momz = 5.7125*FRACUNIT; // jump this high
					else
						player->mo->momz = 9.75*FRACUNIT;

					player->jumping = 1;
				}

				player->mo->z++; // set just an eensy above the ground
				player->scoreadd = 0; // Tails 11-03-2000
				player->mfjumped = 1; // Tails 9-15-99 Spin Attack
				S_StartSound (player->mo, sfx_jump); // Play jump sound!
//				if(player->powers[pw_super])
//					P_SetMobjState (player->mo, S_PLAY_ABL1); // Tails 9-24-99
//				else
					P_SetMobjState (player->mo, S_PLAY_ATK1);
				player->jumpdown = true;
			}
			else
			{
				switch(player->charability)
				{
					case 0:
						// Now it's Sonic's abilities turn!
						if (player->mfjumped)
						{
							if(player->powers[pw_super])		// If you're Super Sonic,
							{									// do a little upward boost
								player->mo->momz += 2*FRACUNIT; // instead!
							}
							else if(player->superready) // If you can turn into Super
							{							// and aren't, do it!
								// Insert flashy transformation animation here.
								player->powers[pw_super] = true;
								P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_CAPE)->target = player->mo; // A cape... "Super" Sonic, get it? Ha...ha...
							}
							else // Otherwise, THOK!
							{
								P_InstaThrust (player->mo, player->mo->angle, 60*FRACUNIT); // Catapult the player
								S_StartSound (player->mo, sfx_thok); // Play the THOK sound
										
								// Now check the player's color so the right THOK object is displayed.
								if(player->skincolor == 0)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GTHOK);
								else if(player->skincolor == 1)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_GRTHOK);
								else if(player->skincolor == 2)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_PCTHOK);
								else if(player->skincolor == 3)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DRTHOK);
								else if(player->skincolor == 4)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_STHOK);
								else if(player->skincolor == 5)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_OTHOK);
								else if(player->skincolor == 6)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_RTHOK);
								else if(player->skincolor == 7)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BTHOK);
								else if(player->skincolor == 8)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_DBTHOK);
								else if(player->skincolor == 9)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_PTHOK);
								else if(player->skincolor == 10)
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BGTHOK);
								else
									P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z, MT_BTHOK);

								// Must press jump while holding down spin to activate.
								if(cv_homing.value && !player->homing && player->mfjumped)
								{
									if(P_LookForEnemies(player))
										if(player->mo->tracer)
											player->homing = 1;
								}
							}

							player->jumpdown = true;
						}
						break;

					case 1:
						// If currently in the air from a jump, and you pressed the
						// button again and have the ability to fly, do so!
						if(!(player->powers[pw_tailsfly]) && (player->mfjumped))
						{
							P_SetMobjState (player->mo, S_PLAY_ABL1); // Change to the flying animation
							player->jumpdown = true;
							player->powers[pw_tailsfly] = 8*TICRATE + 1; // Set the fly timer
							player->mfjumped = player->mfspinning = player->mfstartdash = 0;
						}
						// If currently flying, give an ascend boost.
						else if (player->powers[pw_tailsfly])
						{
							if(player->fly1 == 0)
								player->fly1 = 20;
							else
								player->fly1 = 2;

								player->jumpdown = true;
						}
						break;

					case 2:
						// Now Knuckles-type abilities are checked.
						if (player->mfjumped)
						{
							player->gliding = 1;
							player->glidetime = 0;
							P_SetMobjState(player->mo, S_PLAY_ABL1);
							P_InstaThrust(player->mo, player->mo->angle, 20*FRACUNIT);
							player->mfspinning = 0;
							player->mfstartdash = 0;
							player->jumpdown = true;
						}
						break;
					default:
						break;
				}
			}
		}
		else if(!(cmd->buttons & BT_JUMP))// If not pressing the jump button
			player->jumpdown = false;

		// If letting go of the jump button while still on ascent, cut the jump height.
		if(player->jumpdown == false && player->mfjumped && player->mo->momz > 0 && player->jumping == 1 && !player->cheats & CF_FLYAROUND)
		{
			player->mo->momz = player->mo->momz/2;
			player->jumping = 0;
		}

		// If you're not spinning, you'd better not be spindashing!
		if(!player->mfspinning)
			player->mfstartdash = 0;

		// Sets the minutes/seconds, and synchronizes the "real"
		// amount of time spent in the level. Tails 02-29-2000
		if(!player->exiting)
		{
            player->minutes = (leveltime/(60*TICRATE));
            player->seconds = (leveltime/TICRATE) % 60;
			player->realtime = leveltime;
		}


//////////////////
//TAG MODE STUFF//
//////////////////
		// Tails 05-08-2001
		if(cv_gametype.value == 3)
		{
			for(i=0; i<MAXPLAYERS; i++)
			{
				if(tag == 1 && playeringame[i]) // If "IT"'s time is up, make the next player in line "IT" Tails 05-08-2001
				{
					players[i].tagit = 300*TICRATE + 1;
					player->tagit = 0;
					tag = 0;
					CONS_Printf("%s is it!\n", player_names[i]); // Tell everyone who is it! Tails 05-08-2001
				}
				if(players[i].tagit == 1)
					tag = 1;
			}

			// If nobody is it... find someone! Tails 05-08-2001
			if(!players[0].tagit && !players[1].tagit && !players[2].tagit && !players[3].tagit && !players[4].tagit && !players[5].tagit && !players[6].tagit && !players[7].tagit && !players[8].tagit && !players[9].tagit && !players[10].tagit && !players[11].tagit && !players[12].tagit && !players[13].tagit && !players[14].tagit && !players[15].tagit && !players[16].tagit && !players[17].tagit && !players[18].tagit && !players[19].tagit && !players[20].tagit && !players[21].tagit && !players[22].tagit && !players[23].tagit && !players[24].tagit && !players[25].tagit && !players[26].tagit && !players[27].tagit && !players[28].tagit && !players[29].tagit && !players[30].tagit && !players[31].tagit)
			{
				for(i = 0; i<MAXPLAYERS; i++)
				{
					if(playeringame[i])
					{
						players[i].tagit = 300*TICRATE + 1;
						CONS_Printf("%s is it!\n", player_names[i]); // Tell everyone who is it! Tails 05-08-2001
						break;
					}
				}
			}

			// If you're "IT", show a big "IT" over your head for others to see.
			if(player->tagit)
			{
				if(!(player == &players[consoleplayer])) // Don't display it on your own view.
				P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_TAG);

				player->tagit--;
			}

			// "No-Tag-Zone" Stuff Tails 05-11-2001
			// If in the No-Tag sector and don't have any "tagzone lag",
			// protect the player for 10 seconds.
			if(player->mo->subsector->sector->special == 987 && !player->tagzone && !player->taglag && !player->tagit)
				player->tagzone = 10*TICRATE;

			// If your time is up, set a certain time that you aren't
			// allowed back in, known as "tagzone lag".
			if(player->tagzone == 1)
				player->taglag = 60*TICRATE;

			// Or if you left the no-tag sector, do the same.
			if(player->mo->subsector->sector->special != 987 && player->tagzone)
				player->taglag = 60*TICRATE;

			// If you have "tagzone lag", you shouldn't be protected.
			if(player->taglag)
				player->tagzone = 0;
		}
//////////////////////////
//CAPTURE THE FLAG STUFF//
//////////////////////////

		else if(cv_gametype.value == 4)
		{
			if(player->gotflag) // If you have the flag (duh).
			{
				// Spawn a got-flag message over the head of the player that
				// has it (but not on your own screen if you have the flag).
				if(!(player == &players[consoleplayer]) && player->ctfteam != players[consoleplayer].ctfteam)
				{
					if(player->gotflag == 1)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_GOTFLAG);
					else if(player->gotflag == 2)
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_GOTFLAG2);
				}
			}

			// If the player isn't on a team, put them on one! Tails 08-04-2001
			if(player->ctfteam != 1 && player->ctfteam != 2)
			{
				CV_SetValue(&cv_preferredteam, cv_preferredteam.value);
			}
		}

//////////////////
//ANALOG CONTROL//
//////////////////

		if(cv_analog.value && (cmd->forwardmove || cmd->sidemove))
		{
			// If travelling slow enough, face the way the controls
			// point and not your direction of movement.
			if(player->speed < 2 || player->gliding)
			{
			tempx = tempy = 0;

			tempangle = camera.mo->angle;
			tempangle >>= ANGLETOFINESHIFT;
			tempx += FixedMul(cmd->forwardmove,finecosine[tempangle]);
			tempy += FixedMul(cmd->forwardmove,finesine[tempangle]);

			tempangle = camera.mo->angle-ANG90;
			tempangle >>= ANGLETOFINESHIFT;
			tempx += FixedMul(cmd->sidemove,finecosine[tempangle]);
			tempy += FixedMul(cmd->sidemove,finesine[tempangle]);

			tempx = tempx*FRACUNIT;
			tempy = tempy*FRACUNIT;

			player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, player->mo->x + tempx, player->mo->y + tempy);
			}
			// Otherwise, face the direction you're travelling.
			else if (player->walking || player->running || player->spinning || ((player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL1] || player->mo->state == &states[S_PLAY_ABL2] || player->mo->state == &states[S_PLAY_SPC1] || player->mo->state == &states[S_PLAY_SPC2] || player->mo->state == &states[S_PLAY_SPC3] || player->mo->state == &states[S_PLAY_SPC4]) && player->charability == 1))
				player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, player->rmomx + player->mo->x, player->rmomy + player->mo->y);

			// Update the local angle control.
			if (player==&players[consoleplayer])
				localangle = player->mo->angle;
		}


////////////////////////////
//BLACK SHIELD ACTIVATION,//
//HOMING, AND OTHER COOL  //
//STUFF!                  //
////////////////////////////

		// Black shield activation Tails 01-11-2001
		if(player->powers[pw_blackshield] && cmd->buttons & BT_USE && !player->usedown && (player->mfjumped || player->powers[pw_tailsfly])) // Let Tails use it while he's flying...can be useful!
		{
			// Don't let Super Sonic or invincibility use it
			if(!(player->powers[pw_super] || player->powers[pw_invulnerability]))
			{
			   player->blackow = 1; // This signals for the BOOM to take effect, as seen below.
			   player->powers[pw_blackshield] = false;
			}
		}

		// This is separate so that P_DamageMobj in p_inter.c can call it, too.
		if(player->blackow)
		{
			P_NukeEnemies(player); // Search for all nearby enemies and nuke their pants off!
			S_StartSound (player->mo, sfx_bkpoof); // Sound the BANG!
			player->bonuscount += 10; // Flash the palette.
			player->blackow = 0;
		}

		// Uber-secret HOMING option. Experimental!
		if(cv_homing.value)
		{
			// If you've got a target, chase after it!
			if(player->homing && player->mo->tracer)
				P_HomingAttack(player, player->mo->tracer);

			// But if you don't, then stop homing.
			if(player->mo->tracer && player->mo->tracer->health <= 0 && player->homing)
			{
				player->mo->momz = 10*FRACUNIT;
				player->mo->momx = player->mo->momy = player->homing = 0;
			}

			// If you're not jumping, then you obviously wouldn't be homing.
			if(!player->mfjumped && player->homing)
				player->homing = 0;
		}
/*
		// Makes the player point to location 0,0 on the map. To "go in a circle".
		if(cv_nights.value)
		{
			tempangle = R_PointToAngle2(player->mo->x , player->mo->y, 0, 0)-ANG90;
			tempangle >>= ANGLETOFINESHIFT;
			player->mo->momx += FixedMul(player->mo->momx,finecosine[tempangle]);
			player->mo->momy += FixedMul(player->mo->momy,finesine[tempangle]);
		}
*/

///////////////////////
//SPECIAL STAGE STUFF//
///////////////////////


		if(gamemap == SSSTAGE1 || gamemap == SSSTAGE2 || gamemap == SSSTAGE3
		|| gamemap == SSSTAGE4 || gamemap == SSSTAGE5 || gamemap == SSSTAGE6
		|| gamemap == SSSTAGE7)
		{
			if(player->sstimer < 4 && player->sstimer > 0) // The special stage time is up! Tails 08-11-2001
			{
				player->sstimer = 1;
				player->exiting = 2.8*TICRATE + 2;
				player->sstimer--;
			}

			if(player->sstimer > 1) // As long as time isn't up...
			{
				int ssrings = 0;
				// Count up the rings of all the players and see if
				// they've collected the required amount.
				if(multiplayer || netgame)
				{
					for(i=0; i<MAXPLAYERS; i++)
					{
						if(playeringame[i])
							ssrings += (players[i].mo->health-1);
					}
				}
				else
					ssrings = player->mo->health-1;

				if(ssrings == totalrings)
				{
					if(player == &players[consoleplayer])
						S_StartSound(0, sfx_cgot); // Got the emerald!

					player->mo->momx = player->mo->momy = 0;
					player->exiting = 2.8*TICRATE + 2;
					player->sstimer = 1;

					// Check what emeralds the player has so you know which one to award next.
					if(!(player->emerald1))
					{
						player->emerald1 = true;
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_GREENEMERALD);
					}
					else if((player->emerald1) && !(player->emerald2))
					{
						player->emerald2 = true;
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_ORANGEEMERALD);
					}
					else if((player->emerald2) && !(player->emerald3))
					{
						player->emerald3 = true;
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_PINKEMERALD);
					}
					else if((player->emerald3) && !(player->emerald4))
					{
						player->emerald4 = true;
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_BLUEEMERALD);
					}
					else if((player->emerald4) && !(player->emerald5))
					{
						player->emerald5 = true;
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_REDEMERALD);
					}
					else if((player->emerald5) && !(player->emerald6))
					{
						player->emerald6 = true;
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_LIGHTBLUEEMERALD);
					}
					else if((player->emerald6) && !(player->emerald7))
					{
						player->emerald7 = true;
						P_SpawnMobj(player->mo->x, player->mo->y, player->mo->z + player->mo->height, MT_GREYEMERALD);
					}
				}

				if(player->mo->z <= player->mo->waterz) // If in water, deplete timer 3x as fast.
				{
					player->sstimer--; // No idea why player->sstimer -= 3; doesn't work here...
					player->sstimer--;
					player->sstimer--;
				}
				else
					player->sstimer--;
			}
		}

		// The Super-Secret RedXVI thing!
		if(player->redxvi > 1)
			player->redxvi--;

		if(player->climbing == 1)
		{
			fixed_t platx;
			fixed_t platy;
			subsector_t* glidesector;

			if((player->mo->momx || player->mo->momy || player->mo->momz)
				&& !(player->mo->state == &states[S_PLAY_CLIMB2]
					|| player->mo->state == &states[S_PLAY_CLIMB3]
					|| player->mo->state == &states[S_PLAY_CLIMB4]
					|| player->mo->state == &states[S_PLAY_CLIMB5]))
				P_SetMobjState(player->mo, S_PLAY_CLIMB2);
			else if (!(player->mo->momx || player->mo->momy || player->mo->momz) && player->mo->state != &states[S_PLAY_CLIMB1])
				P_SetMobjState(player->mo, S_PLAY_CLIMB1);
			
			platx = P_ReturnThrustX(player->mo, player->mo->angle, player->mo->radius + 8*FRACUNIT);
			platy = P_ReturnThrustY(player->mo, player->mo->angle, player->mo->radius + 8*FRACUNIT);

			glidesector = R_PointInSubsector(player->mo->x + platx, player->mo->y + platy);

			if(glidesector->sector != player->mo->subsector->sector)
			{
				boolean floorclimb;
				floorclimb = false;
				
				if(glidesector->sector->ffloors)
				{
	 				ffloor_t* rover;
					floorclimb = true;
					for(rover = glidesector->sector->ffloors; rover; rover = rover->next)
					{
						if((*rover->bottomheight > player->mo->z) && ((player->mo->z - player->mo->momz) > *rover->bottomheight))
						{
							player->mo->momz = 0;
						}
						if(*rover->topheight < player->mo->z + 16*FRACUNIT)
						{
							floorclimb = false;
							player->climbing = 0;
							player->mfjumped = 1;
							player->mo->momz += 2*FRACUNIT;
							P_InstaThrust(player->mo, player->mo->angle, 4*FRACUNIT); // Lil' boost up.
							P_SetMobjState(player->mo, S_PLAY_ATK1);
						}
					}
				}

				if(floorclimb == false && glidesector->sector->ceilingheight > player->mo->z && glidesector->sector->floorheight < player->mo->z && player->mo->momz < 0)
				{
					player->mo->momz = 0;
				}
				else if(floorclimb == false && glidesector->sector->floorheight < player->mo->z + 16*FRACUNIT && (glidesector->sector->ceilingpic == skyflatnum || glidesector->sector->ceilingheight > (player->mo->z + player->mo->height + 8*FRACUNIT)))
				{
					player->climbing = 0;
					player->mfjumped = 1;
					player->mo->momz += 2*FRACUNIT;
					P_InstaThrust(player->mo, player->mo->angle, 4*FRACUNIT); // Lil' boost up.
					P_SetMobjState(player->mo, S_PLAY_ATK1);
					// Play climb-up animation here
				}
			}
			else
			{
				player->climbing = 0;
				player->mfjumped = 1;
				P_SetMobjState(player->mo, S_PLAY_ATK1);
			}

			if(cmd->buttons & BT_USE)
			{
				player->climbing = 0;
				player->mfjumped = 1;
				P_SetMobjState(player->mo, S_PLAY_ATK1);
				player->mo->momz = 4*FRACUNIT;
				P_InstaThrust(player->mo, player->mo->angle, -4*FRACUNIT);
			}

			if (player==&players[consoleplayer])
				localangle = player->mo->angle;

		}

		if(player->climbing > 1)
		{
			P_InstaThrust(player->mo, player->mo->angle, 4*FRACUNIT); // Shove up against the wall
			player->climbing--;
		}

		if(xmasmode && cv_numsnow.value <= 128 && !(netgame || multiplayer))
		{
			fixed_t x;
			fixed_t y;
			int z;
			subsector_t* snowsector;
			z = 0;

			for(i=0; i<cv_numsnow.value; i++)
			{
				x = ((rand() * 65535) - 32768) << FRACBITS;
				y = ((rand() * 65535) - 32768) << FRACBITS;

				if(rand() & 1)
					x = -x;

				if(rand() & 1)
					y = -y;

				snowsector = R_PointInSubsector(x, y);

				if(snowsector->sector->ceilingpic == skyflatnum && snowsector->sector->floorheight < snowsector->sector->ceilingheight)
				{
					z = P_Random();
					if(z < 64)
						P_SetMobjState(P_SpawnMobj(x, y, ONCEILINGZ, MT_SNOWFLAKE), S_SNOW3);
					else if (z < 144)
						P_SetMobjState(P_SpawnMobj(x, y, ONCEILINGZ, MT_SNOWFLAKE), S_SNOW2);
					else
						P_SpawnMobj(x, y, ONCEILINGZ, MT_SNOWFLAKE);
				}
			}
		}

		if(player->bustercount > 2*TICRATE && player->bustercount & 2)
			player->mo->flags |= MF_SHADOW;
		else if(!(player->powers[pw_invisibility] & 1))
			player->mo->flags &= ~MF_SHADOW;

		if(player->bustercount == TICRATE)
			S_StartSound(player->mo, sfx_posit1);
		else if (player->bustercount > 2*TICRATE && player->bustercount & 1)
			S_StartSound(player->mo, sfx_posit2);

		if(player->bustercount > 0 && (player->mo->health <= 5 || !player->snowbuster))
			player->bustercount = 0;

		if(leveltime == 2)
		{
			P_FindEmerald(player); // Look for emeralds to hunt for.
		}

		// Check for spikes!
		if(!player->mo->momz)
		{
			for (node = player->mo->touching_sectorlist; node; node = node->m_snext)
				if (((sec = node->m_sector)->special == 7) &&
					(player->mo->z <= sec->floorheight))
				{
					P_DamageMobj(player->mo, NULL, NULL, 1);
					break;
				}
		}
	}

//
// P_NukeEnemies
// Looks for something you can hit - Used for black shield Tails 07-12-2001
//
mobj_t*         bombsource;
mobj_t*         bombspot;
boolean P_NukeEnemies (player_t* player)
{
    int         x;
    int         y;

    int         xl;
    int         xh;
    int         yl;
    int         yh;

	int			i;
	mobj_t*		mo;

    fixed_t     dist;

    dist = 1536<<FRACBITS;
    yh = (player->mo->y + dist - bmaporgy)>>MAPBLOCKSHIFT;
    yl = (player->mo->y - dist - bmaporgy)>>MAPBLOCKSHIFT;
    xh = (player->mo->x + dist - bmaporgx)>>MAPBLOCKSHIFT;
    xl = (player->mo->x - dist - bmaporgx)>>MAPBLOCKSHIFT;
    bombspot = player->mo;
    bombsource = player->mo;

	for(i=0; i<16; i++)
	{
    mo = P_SpawnMobj(player->mo->x,
                     player->mo->y,
                     player->mo->z,
                     MT_SUPERSPARK);
	mo->momx = sin(i*22.5) * 32 * FRACUNIT;
	mo->momy = cos(i*22.5) * 32 * FRACUNIT;
	}

    for (y=yl ; y<=yh ; y++)
        for (x=xl ; x<=xh ; x++)
            P_BlockThingsIterator (x, y, PIT_NukeEnemies );

		return true;
}

boolean PIT_NukeEnemies (mobj_t* thing)
{
    fixed_t     dx;
    fixed_t     dy;
    fixed_t     dist;

    if (!(thing->flags & MF_SHOOTABLE) )
        return true;

    dx = abs(thing->x - bombspot->x);
    dy = abs(thing->y - bombspot->y);

    dist = dx>dy ? dx : dy;
    dist -= thing->radius;

	if(abs(thing->z+(thing->height>>1) - bombspot->z) > 1024*FRACUNIT)
		return true;

    dist >>= FRACBITS;

    if (dist < 0)
        dist = 0;

    if (dist > 1536)
        return true;    // out of range

	if(!cv_gametype.value && thing->type == MT_PLAYER)
		return true; // Don't hurt players in Co-Op!

	if(thing == bombsource) // Don't hurt yourself!!
			return true;

// Uncomment P_CheckSight to prevent black shield from going through walls Tails 07-13-2001
//    if ( P_CheckSight (thing, bombspot) )
	P_DamageMobj (thing, bombspot, bombsource, 2); // Tails 01-11-2001

    return true;
}


//
// P_LookForEnemies
// Looks for something you can hit - Used for homing attack Tails 06-20-2001
//
boolean P_LookForEnemies (player_t* player)
{
    int                 i;
    angle_t             an;

    // offset angles from its attack angle
    for (i=0 ; i<16384 ; i++)
    {
        an = player->mo->angle - ANG90/2 + ANG90/40*i;

        P_AimLineAttack (player->mo, an, 16*64*FRACUNIT);

        if (!linetarget)
            continue;

		if(P_AproxDistance(P_AproxDistance(player->mo->x - linetarget->x, player->mo->y - linetarget->y), player->mo->z - linetarget->z) > RING_DIST)
			continue;

		if(linetarget->type == MT_PLAYER)
			continue;

		player->mo->target = linetarget;
		player->mo->tracer = linetarget;

        return true;
    }
	return false;
}

int     HOMEANGLE = 0xc000000;

boolean P_HomingAttack (player_t* player, mobj_t* enemy) // Home in on your target Tails 06-19-2001
{
    fixed_t     dist;
    mobj_t*     dest;

	if(!(enemy->health))
		return false;

	player->mo->tracer = player->mo->target;

    // adjust direction
    dest = player->mo->tracer;

    if (!dest || dest->health <= 0)
        return false;

    // change angle
	player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, enemy->x, enemy->y);
	    if (player==&players[consoleplayer])
			localangle = player->mo->angle;


    // change slope
	dist = P_AproxDistance(P_AproxDistance(dest->x - player->mo->x, dest->y - player->mo->y), dest->z - player->mo->z);

    if (dist < 1)
        dist = 1;

	player->mo->momx = FixedMul(FixedDiv(dest->x - player->mo->x, dist), 30*FRACUNIT);
	player->mo->momy = FixedMul(FixedDiv(dest->y - player->mo->y, dist), 30*FRACUNIT);
	player->mo->momz = FixedMul(FixedDiv(dest->z - player->mo->z, dist), 30*FRACUNIT);

	return true; // There, you satisfied now, Mr. Compiler? Tails 06-20-2001
}

// Search for emeralds Tails 12-20-2001
void P_FindEmerald (player_t* player)
{
    thinker_t*  th;
    mobj_t*     mo2;

    // scan the remaining thinkers
    // to find all emeralds
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
        if (th->function.acp1 != (actionf_p1)P_MobjThinker)
            continue;

        mo2 = (mobj_t *)th;
        if (!player->hunt1 && (mo2->type == MT_EMERHUNT || mo2->type == MT_EMESHUNT || mo2->type == MT_EMETHUNT))
        {
            player->hunt1 = mo2; // Found it!
        }
		else if(!player->hunt2 && (mo2->type == MT_EMERHUNT || mo2->type == MT_EMESHUNT || mo2->type == MT_EMETHUNT))
        {
            player->hunt2 = mo2; // Found it!
        }
		else if(!player->hunt3 && (mo2->type == MT_EMERHUNT || mo2->type == MT_EMESHUNT || mo2->type == MT_EMETHUNT))
        {
            player->hunt3 = mo2; // Found it!
        }
    }
	return;
}

//
// P_DeathThink
// Fall on your face when dying.
// Decrease POV height to floor height.
//
#define ANG5    (ANG90/18)

void P_DeathThink (player_t* player)
{
    angle_t             angle;
    angle_t             delta;
    mobj_t*             attacker;       //added:22-02-98:
    fixed_t             dist;           //added:22-02-98:
    int                 pitch;          //added:22-02-98:

    P_MovePsprites (player);

    // fall to the ground
    if (player->viewheight > 6*FRACUNIT)
        player->viewheight -= FRACUNIT;

    if (player->viewheight < 6*FRACUNIT)
        player->viewheight = 6*FRACUNIT;

    player->deltaviewheight = 0;
    onground = player->mo->z <= player->mo->floorz;

    P_CalcHeight (player);

    attacker = player->attacker;

    // watch my killer (if there is one)
    if (attacker && attacker != player->mo)
    {
        angle = R_PointToAngle2 (player->mo->x,
                                 player->mo->y,
                                 player->attacker->x,
                                 player->attacker->y);

        delta = angle - player->mo->angle;

        if (delta < ANG5 || delta > (unsigned)-ANG5)
        {
            // Looking at killer,
            //  so fade damage flash down.
            player->mo->angle = angle;

            if (player->damagecount)
                player->damagecount--;
        }
        else if (delta < ANG180)
            player->mo->angle += ANG5;
        else
            player->mo->angle -= ANG5;

        //added:22-02-98:
        // change aiming to look up or down at the attacker (DOESNT WORK)
        // FIXME : the aiming returned seems to be too up or down... later


            dist = P_AproxDistance (attacker->x - player->mo->x, attacker->y - player->mo->y);
            //if (dist)
            //    pitch = FixedMul ((160<<FRACBITS), FixedDiv (attacker->z + (attacker->height>>1), dist)) >>FRACBITS;
            //else
            //    pitch = 0;
            pitch = (attacker->z - player->mo->z)>>17;
            player->aiming = G_ClipAimingPitch (&pitch);

    }
    else if (player->damagecount)
        player->damagecount--;

	player->deadtimer++;

		// Check if the camera is underwater
		// (used for palette changes) Tails 11-02-2000
		if(camera.chase && (camera.mo->z + 8*FRACUNIT) < camera.mo->waterz)
			player->camunder = 1;
		else
			player->camunder = 0;

//	player->tagit = 0; // Stay Tagged! Tails 08-17-2001

	if(!(netgame || multiplayer) && (player->deadtimer > 15*TICRATE) && !(player->lives > 0) && !(player->continues > 0)) // Tails 11-21-2000
			D_StartTitle (); // Tails 11-21-2000

	if(player->lives > 0)
	{
		if (player->cmd.buttons & BT_JUMP && player->deadtimer > TICRATE && !(cv_gametype.value == 1 || cv_gametype.value == 2)) // Respawn with Jump button Tails 12-04-99
			player->playerstate = PST_REBORN;
		else if (player->cmd.buttons & BT_JUMP && (cv_gametype.value == 1 || cv_gametype.value == 2)) // Tails 05-06-2001
			player->playerstate = PST_REBORN; // Tails 05-06-2001

		if(player->deadtimer > 4*TICRATE && !cv_gametype.value) // Tails 05-06-2001
			player->playerstate = PST_REBORN; // Tails 05-06-2001
		else if(player->deadtimer > 30*TICRATE && cv_gametype.value == 3) // Tails 08-17-2001
			player->playerstate = PST_REBORN; // Tails 08-17-2001

		if(player->mo->z < R_PointInSubsector(player->mo->x, player->mo->y)->sector->floorheight - 10000*FRACUNIT)
			player->playerstate = PST_REBORN;
	}

	if(player->mo->momz < -30*FRACUNIT)
		player->mo->momz = -30*FRACUNIT;

	if(player->mo->z + player->mo->momz < player->mo->subsector->sector->floorheight - 5120*FRACUNIT)
	{
		player->mo->momz = 0;
		player->mo->z = player->mo->subsector->sector->floorheight - 5120*FRACUNIT;
	}

}


//
// P_MoveCamera : make sure the camera is not outside the world
//                and looks at the player avatar
//

camera_t camera;

//#define VIEWCAM_DIST    (128<<FRACBITS)
//#define VIEWCAM_HEIGHT  (20<<FRACBITS)

consvar_t cv_cam_dist   = {"cam_dist"  ,"128"  ,CV_FLOAT,NULL};
consvar_t cv_cam_still  = {"cam_still"  ,"0", 0,CV_OnOff}; // Tails 07-02-2001
consvar_t cv_cam_height = {"cam_height", "20"   ,CV_FLOAT,NULL};
consvar_t cv_cam_speed  = {"cam_speed" ,  "0.25",CV_FLOAT,NULL};

void P_ResetCamera (player_t *player)
{
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;

    camera.chase = true;
    x = player->mo->x;
    y = player->mo->y;
    z = player->mo->z + (cv_viewheight.value<<FRACBITS);

    // hey we should make sure that the sounds are heard from the camera
    // instead of the marine's head : TO DO

    // set bits for the camera
    if (!camera.mo)
        camera.mo = P_SpawnMobj (x,y,z, MT_CHASECAM);
    else
    {
        camera.mo->x = x;
        camera.mo->y = y;
        camera.mo->z = z;
    }

    camera.mo->angle = player->mo->angle;
    camera.aiming = 0;
}

boolean PTR_FindCameraPoint (intercept_t* in)
{
/*    int         side;
    fixed_t             slope;
    fixed_t             dist;
    line_t*             li;

    li = in->d.line;

    if ( !(li->flags & ML_TWOSIDED) )
        return false;

    // crosses a two sided line
    //added:16-02-98: Fab comments : sets opentop, openbottom, openrange
    //                lowfloor is the height of the lowest floor
    //                         (be it front or back)
    P_LineOpening (li);

    dist = FixedMul (attackrange, in->frac);

    if (li->frontsector->floorheight != li->backsector->floorheight)
    {
        //added:18-02-98: comments :
        // find the slope aiming on the border between the two floors
        slope = FixedDiv (openbottom - cameraz , dist);
        if (slope > aimslope)
            return false;
    }

    if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
    {
        slope = FixedDiv (opentop - shootz , dist);
        if (slope < aimslope)
            goto hitline;
    }

    return true;

    // hit line
  hitline:*/
    // stop the search
    return false;
}

fixed_t cameraz;

void P_MoveChaseCamera (player_t *player)
{
    angle_t             angle;
    fixed_t             x,y,z ,viewpointx,viewpointy;
    fixed_t             dist;
    mobj_t*             mo;
    subsector_t*        newsubsec;
    float               f1,f2;

    if (!camera.mo)
        P_ResetCamera (player);
    mo = player->mo;

	if (cv_cam_still.value == true) // Tails 07-02-2001
		angle = camera.mo->angle;
	else if(cv_analog.value /*&& !cv_nights.value*/) // Analog Test Tails 06-10-2001
		angle = R_PointToAngle2(camera.mo->x, camera.mo->y, mo->x, mo->y);
//	else if(cv_nights.value) // NiGHTS Boss Level Tails 06-10-2001
//		angle = R_PointToAngle2(camera.mo->x, camera.mo->y, 0, 0);
	else
		angle = mo->angle;
/*
	// Grr stupid camera buttons won't work!
	if(player->mo && cv_analog.value)
	{
	if(player->cmd.cammove) // Tails 06-20-2001
		P_Thrust(camera.mo, camera.mo->angle-ANG90, player->cmd.cammove);
	}
*/
    // sets ideal cam pos
    dist  = cv_cam_dist.value;
    x = mo->x - FixedMul( finecosine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
    y = mo->y - FixedMul(   finesine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
    z = mo->z + (cv_viewheight.value<<FRACBITS) + cv_cam_height.value;

/*    P_PathTraverse ( mo->x, mo->y, x, y, PT_ADDLINES, PTR_UseTraverse );*/

    // move camera down to move under lower ceilings
    newsubsec = R_IsPointInSubsector ((mo->x + camera.mo->x)>>1,(mo->y + camera.mo->y)>>1);
              
	if(!player->playerstate == PST_DEAD)
	{
    if (!newsubsec)
    {
        // use player sector 
        if (mo->subsector->sector->ceilingheight - camera.mo->height < z)
            z = mo->subsector->sector->ceilingheight - camera.mo->height-11*FRACUNIT; // don't be blocked by a opened door
    }
    else
    // camera fit ?
    if (newsubsec->sector->ceilingheight - camera.mo->height < z)
        // no fit
        z = newsubsec->sector->ceilingheight - camera.mo->height-11*FRACUNIT;
        // is the camera fit is there own sector
    newsubsec = R_PointInSubsector (camera.mo->x,camera.mo->y);
    if (newsubsec->sector->ceilingheight - camera.mo->height < z)
        z = newsubsec->sector->ceilingheight - camera.mo->height-11*FRACUNIT;
	}

    // point viewed by the camera
    // this point is just 64 unit forward the player
/*
	if(cv_nights.value)
	{
		dist = 64 << FRACBITS;
		viewpointx = 0;
		viewpointy = 0;
	}
	else
	{*/
    dist = 64 << FRACBITS;
    viewpointx = mo->x + FixedMul( finecosine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
    viewpointy = mo->y + FixedMul( finesine[(angle>>ANGLETOFINESHIFT) & FINEMASK], dist);
//	}

if(!cv_cam_still.value) // Tails 07-02-2001
    camera.mo->angle = R_PointToAngle2(camera.mo->x,camera.mo->y,
                                       viewpointx,viewpointy);

    // follow the player
    if(!player->playerstate == PST_DEAD && cv_cam_speed.value != 0 && (abs(camera.mo->x - mo->x) > cv_cam_dist.value*3 || 
       abs(camera.mo->y - mo->y) > cv_cam_dist.value*3 || abs(camera.mo->z - mo->z) > cv_cam_dist.value*3))
                P_ResetCamera(player); // Tails
    camera.mo->momx = FixedMul(x - camera.mo->x,cv_cam_speed.value);
    camera.mo->momy = FixedMul(y - camera.mo->y,cv_cam_speed.value);
    camera.mo->momz = FixedMul(z - camera.mo->z,cv_cam_speed.value);

    // compute aming to look the viewed point
    f1=FIXED_TO_FLOAT(viewpointx-camera.mo->x);
    f2=FIXED_TO_FLOAT(viewpointy-camera.mo->y);
    dist=sqrt(f1*f1+f2*f2)*FRACUNIT;
    angle=R_PointToAngle2(0,camera.mo->z, dist
                         ,mo->z+(mo->height>>1)+finesine[(player->aiming>>ANGLETOFINESHIFT) & FINEMASK] * 64);

    G_ClipAimingPitch(&angle);
    dist=camera.aiming-angle;
    camera.aiming-=(dist>>3);
}

#ifdef CLIENTPREDICTION
void P_MoveSpirit (player_t* player,ticcmd_t *cmd)
{

#ifdef PARANOIA
    if(!player)
        I_Error("P_MoveSpirit : player null");
    if(!player->spirit)
        I_Error("P_MoveSpirit : player->spirit null");
    if(!cmd)
        I_Error("P_MoveSpirit : cmd null");
#endif

//    player->spirit->angle = localangle;

    if (cmd->forwardmove)
        P_Thrust (player->spirit, player->spirit->angle, cmd->forwardmove*2048);

    if (cmd->sidemove && onground)
        P_Thrust (player->spirit, player->spirit->angle-ANG90, cmd->sidemove*2048);

//    camera.aiming = (signed char)cmd->aiming;
}
#endif
/* Weapon fart Tails
byte weapontobutton[NUMWEAPONS]={wp_fist    <<BT_WEAPONSHIFT,
                                 wp_pistol  <<BT_WEAPONSHIFT,
                                 wp_shotgun <<BT_WEAPONSHIFT,
                                 wp_chaingun<<BT_WEAPONSHIFT,
                                 wp_missile <<BT_WEAPONSHIFT,
                                 wp_plasma  <<BT_WEAPONSHIFT,
                                 wp_bfg     <<BT_WEAPONSHIFT,
                                (wp_fist    <<BT_WEAPONSHIFT) | BT_EXTRAWEAPON,// wp_chainsaw
                                (wp_shotgun <<BT_WEAPONSHIFT) | BT_EXTRAWEAPON};//wp_supershotgun
*/
#ifdef CLIENTPREDICTION2

void CL_ResetSpiritPosition(mobj_t *mobj)
{
    P_UnsetThingPosition(mobj->player->spirit);
    mobj->player->spirit->x=mobj->x;
    mobj->player->spirit->y=mobj->y;
    mobj->player->spirit->z=mobj->z;
    mobj->player->spirit->momx=0;
    mobj->player->spirit->momy=0;
    mobj->player->spirit->momz=0;
    P_SetThingPosition(mobj->player->spirit);
}

void P_MoveSpirit (player_t* player,ticcmd_t *cmd)
{
    fixed_t   movepushforward=0,movepushside=0;
#ifdef PARANOIA
    if(!player)
        I_Error("P_MoveSpirit : player null");
    if(!player->spirit)
        I_Error("P_MoveSpirit : player->spirit null");
    if(!cmd)
        I_Error("P_MoveSpirit : cmd null");
#endif

    // don't move if dead
    if( player->playerstate != PST_LIVE )
    {
        cmd->angleturn &= ~TICCMD_XY;
        return;
    }
    onground = (player->spirit->z <= player->spirit->floorz) ||
               (player->cheats & CF_FLYAROUND);

    if (player->spirit->reactiontime)
    {
        player->spirit->reactiontime--;
        return;
    }

    player->spirit->angle = cmd->angleturn<<16;
    cmd->angleturn |= TICCMD_XY;
/*
    // now weapon is allways send change is detected at receiver side
    if(cmd->buttons & BT_CHANGE) 
    {
        player->spirit->movedir = cmd->buttons & (BT_WEAPONMASK | BT_EXTRAWEAPON);
        cmd->buttons &=~BT_CHANGE;
    }
    else
    {
        if( player->pendingweapon!=wp_nochange )
            player->spirit->movedir=weapontobutton[player->pendingweapon];
        cmd->buttons&=~(BT_WEAPONMASK | BT_EXTRAWEAPON);
        cmd->buttons|=player->spirit->movedir;
    }
*/
    if (cmd->forwardmove)
    {
        movepushforward = cmd->forwardmove * 2048;
        
        if (player->spirit->eflags & MF_UNDERWATER)
        {
            // half forward speed when waist under water
            // a little better grip if feets touch the ground
            if (!onground)
                movepushforward >>= 1;
            else
                movepushforward = movepushforward *3/4;
        }
        else
        {
            // allow very small movement while in air for gameplay
            if (!onground)
                movepushforward >>= 3;
        }
        
        P_Thrust (player->spirit, player->spirit->angle, movepushforward);
    }
    
    if (cmd->sidemove)
    {
        movepushside = cmd->sidemove * 2048;
        if (player->spirit->eflags & MF_UNDERWATER)
        {
            if (!onground)
                movepushside >>= 1;
            else
                movepushside = movepushside *3/4;
        }
        else 
            if (!onground)
                movepushside >>= 3;
            
        P_Thrust (player->spirit, player->spirit->angle-ANG90, movepushside);
    }
    
    // mouselook swim when waist underwater
    player->spirit->eflags &= ~MF_SWIMMING;
    if (player->spirit->eflags & MF_UNDERWATER)
    {
        fixed_t a;
        // swim up/down full move when forward full speed
        a = FixedMul( movepushforward*50, finesine[ (cmd->aiming>>(ANGLETOFINESHIFT-16)) ] >>5 );
        
        
        /* a little hack to don't have screen moving
        if( a > cv_gravity.value>>2 || a < 0 )*/
        if ( a != 0 ) {
            player->spirit->eflags |= MF_SWIMMING;
            player->spirit->momz += a;
        }
    }

    //added:22-02-98: jumping
    if (cmd->buttons & BT_JUMP)
    {
        // can't jump while in air, can't jump while jumping
        if (!(player->jumpdown & 2) &&
             (onground || (player->spirit->eflags & MF_UNDERWATER)) )
        {
            if (onground)
                player->spirit->momz = JUMPGRAVITY;
            else //water content
                player->spirit->momz = JUMPGRAVITY/2;

            //TODO: goub gloub when push up in water
            
            if ( !(player->cheats & CF_FLYAROUND) && onground && !(player->spirit->eflags & MF_UNDERWATER))
            {
                S_StartSound (player->spirit, sfx_jump);

                // keep jumping ok if FLY mode.
                player->jumpdown |= 2;
            }
        }
    }
    else
        player->jumpdown &= ~2;

}
#endif


//
// P_PlayerThink
//

boolean playerdeadview; //Fab:25-04-98:show dm rankings while in death view

void P_PlayerThink (player_t* player)
{
    ticcmd_t*           cmd;
//    weapontype_t        newweapon; // Weapon fart Tails
//    int                 waterz;

#ifdef PARANOIA
    if(!player->mo) I_Error("p_playerthink : players[%d].mo == NULL",player-players);
#endif

    // fixme: do this in the cheat code
    if (player->cheats & CF_NOCLIP)
        player->mo->flags |= MF_NOCLIP;
    else
        player->mo->flags &= ~MF_NOCLIP;

    // chain saw run forward
    cmd = &player->cmd;
    if (player->mo->flags & MF_JUSTATTACKED)
    {
// added : now angle turn is a absolute value not relative
#ifndef ABSOLUTEANGLE
        cmd->angleturn = 0;
#endif
        cmd->forwardmove = 0xc800/512;
        cmd->sidemove = 0;
        player->mo->flags &= ~MF_JUSTATTACKED;
    }

    if (player->playerstate == PST_REBORN)
#ifdef PARANOIA
        I_Error("player %d is in PST_REBORN\n");
#else
        // it is not "normal" but far to be critical
        return;
#endif

    if (player->playerstate == PST_DEAD)
    {

		player->mo->flags &= ~MF_SHADOW; // Tails 05-06-2001
        //Fab:25-04-98: show the dm rankings while dead, only in deathmatch
        if (player==&players[displayplayer])
            playerdeadview = true;

        P_DeathThink (player);

        //added:26-02-98:camera may still move when guy is dead
        if (camera.chase)
            P_MoveChaseCamera (&players[displayplayer]);
        return;
    }
    else
        if (player==&players[displayplayer])
            playerdeadview = false;

    // check water content, set stuff in mobj
    P_MobjCheckWater (player->mo);

    // Move around.
    // Reactiontime is used to prevent movement
    //  for a bit after a teleport.
    if (player->mo->reactiontime)
        player->mo->reactiontime--;
    else
        P_MovePlayer (player);

    //added:22-02-98: bob view only if looking by the marine's eyes
#ifndef CLIENTPREDICTION2
    if (!camera.chase)
        P_CalcHeight (player);
#endif

    //added:26-02-98: calculate the camera movement
    if (camera.chase && player==&players[displayplayer])
        P_MoveChaseCamera (&players[displayplayer]);

    // check special sectors : damage & secrets
    P_PlayerInSpecialSector (player);

    //
    // water splashes
    //
	/* Killed this Tails
    if (demoversion>=125 && player->specialsector >= 887 &&
                            player->specialsector <= 888)
    {
        if ((player->mo->momx >  (2*FRACUNIT) ||
             player->mo->momx < (-2*FRACUNIT) ||
             player->mo->momy >  (2*FRACUNIT) ||
             player->mo->momy < (-2*FRACUNIT) ||
             player->mo->momz >  (2*FRACUNIT)) &&  // jump out of water
             !(gametic & 31)          )
        {
            //
            // make sur we disturb the surface of water (we touch it)
            //
            if (player->specialsector==887)
                //FLAT TEXTURE 'FWATER'
                waterz = player->mo->subsector->sector->floorheight + (FRACUNIT/4);
            else
                //faB's current water hack using negative sector tags
                waterz = - (player->mo->subsector->sector->tag << FRACBITS);

            // half in the water
            if(player->mo->eflags & MF_TOUCHWATER)
            {
                if (player->mo->z <= player->mo->floorz) // onground
                {
                    fixed_t whater_height=waterz-player->mo->subsector->sector->floorheight;

                    if( whater_height<(player->mo->height>>2 ))
                        S_StartSound (player->mo, sfx_splash);
                    else
                        S_StartSound (player->mo, sfx_floush);
                }
                else
                    S_StartSound (player->mo, sfx_floush);
            }                   
        }
    }*/
/* Weapon fart Tails
    // Check for weapon change.
//#ifndef CLIENTPREDICTION2
    if (cmd->buttons & BT_CHANGE)
//#endif
    {

        // The actual changing of the weapon is done
        //  when the weapon psprite can do it
        //  (read: not in the middle of an attack).
        newweapon = (cmd->buttons&BT_WEAPONMASK)>>BT_WEAPONSHIFT;
        if(demoversion<128)
        {
            if (newweapon == wp_fist
                && player->weaponowned[wp_chainsaw]
                && !(player->readyweapon == wp_chainsaw
                     && player->powers[pw_strength]))
            {
                newweapon = wp_chainsaw;
            }
        
            if ( (gamemode == commercial)
                && newweapon == wp_shotgun
                && player->weaponowned[wp_supershotgun]
                && player->readyweapon != wp_supershotgun)
            {
                newweapon = wp_supershotgun;
            }
        }
        else
        {
            if(cmd->buttons&BT_EXTRAWEAPON)
               switch(newweapon) {
                  case wp_shotgun : 
                       if( gamemode == commercial && player->weaponowned[wp_supershotgun])
                           newweapon = wp_supershotgun;
                       break;
                  case wp_fist :
                       if( player->weaponowned[wp_chainsaw])
                           newweapon = wp_chainsaw;
                       break;
                  default:
                       break;
               }
        }

        if (player->weaponowned[newweapon]
            && newweapon != player->readyweapon)
        {
            // Do not go to plasma or BFG in shareware,
            //  even if cheated.
            if ((newweapon != wp_plasma
                 && newweapon != wp_bfg)
                || (gamemode != shareware) )
            {
                player->pendingweapon = newweapon;
            }
        }
    }
*/
    // check for use
    if (cmd->buttons & BT_USE)
    {
        if (!player->usedown)
        {
            P_UseLines (player);
            player->usedown = true;
        }
    }
    else
        player->usedown = false;

    // cycle psprites
    P_MovePsprites (player);
    // Counters, time dependent power ups.

	// Start Time Bonus & Ring Bonus count settings Tails 03-10-2000
	player->fscore = player->score; // Tails 03-12-2000

	if(player->countdown)
		player->countdown--;

	if(player->countdown2)
		player->countdown2--;

	if(player->splish)
		player->splish--;

	if(player->tagzone)
		player->tagzone--;

	if(player->taglag)
		player->taglag--;

    // Strength counts up to diminish fade.
    if (player->powers[pw_strength])
        player->powers[pw_strength]--;

    if (player->powers[pw_invulnerability])
        player->powers[pw_invulnerability]--;

    // the MF_SHADOW activates the tr_transhi translucency while it is set
    // (it doesnt use a preset value through FF_TRANSMASK)
    if (player->powers[pw_invisibility])
        if (! --player->powers[pw_invisibility] )
            player->mo->flags &= ~MF_SHADOW;

    if (player->powers[pw_infrared])
        player->powers[pw_infrared]--;

    if (player->powers[pw_ironfeet])
        player->powers[pw_ironfeet]--;

    if (player->powers[pw_tailsfly]) // tails fly
        player->powers[pw_tailsfly]--; // counter Tails 03-05-2000

    if (player->powers[pw_underwater]) // underwater
        player->powers[pw_underwater]--; // timer Tails 03-06-2000

    if (player->powers[pw_extralife]) // what's it look like pal?
        player->powers[pw_extralife]--; // duuuuh Tails 03-14-2000

    if (player->damagecount)
        player->damagecount--;

    if (player->bonuscount)
        player->bonuscount--;

    // Handling colormaps.
	// DIEE!!!! Tails 01-06-2001
/*
    if (player->powers[pw_invulnerability])
    {
        if (player->powers[pw_invulnerability] > 4*TICRATE
            || (player->powers[pw_invulnerability]&8) )
            player->fixedcolormap = INVERSECOLORMAP;
        else
            player->fixedcolormap = 0;
    }
    else if (player->powers[pw_infrared])
    {
        if (player->powers[pw_infrared] > 4*TICRATE
            || (player->powers[pw_infrared]&8) )
        {
            // almost full bright
            player->fixedcolormap = 1;
        }
        else
            player->fixedcolormap = 0;
    }
    else*/
        player->fixedcolormap = 0;

}
