// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_telept.c,v 1.8 2001/08/06 23:57:09 stroggonmeth Exp $
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
// $Log: p_telept.c,v $
// Revision 1.8  2001/08/06 23:57:09  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.7  2001/03/13 22:14:19  stroggonmeth
// Long time no commit. 3D floors, FraggleScript, portals, ect.
//
// Revision 1.6  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.5  2000/11/04 16:23:43  bpereira
// no message
//
// Revision 1.4  2000/11/02 17:50:09  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.3  2000/04/04 00:32:47  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Teleportation.
//
//-----------------------------------------------------------------------------

#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "r_state.h"
#include "s_sound.h"

#include "r_main.h" //SoM: 3/16/2000

extern consvar_t cv_chasecam;
extern consvar_t cv_chasecam2;

boolean P_Teleport(mobj_t *thing, fixed_t x, fixed_t y, angle_t angle)
{
    fixed_t     oldx;
    fixed_t     oldy;
    fixed_t     oldz;
    fixed_t     aboveFloor,fogDelta;
    unsigned    an;

    oldx = thing->x;
    oldy = thing->y;
    oldz = thing->z;
    fogDelta = 0;

    aboveFloor = thing->z-thing->floorz;
    
    if (!P_TeleportMove (thing, x, y))
        return 0;
    
    thing->z = thing->floorz;  //fixme: not needed?
    if (thing->player)
    {
        thing->player->viewz = thing->z+thing->player->viewheight;
    }
    else if(thing->flags&MF_MISSILE) // heretic stuff
    {
        thing->z = thing->floorz+aboveFloor;
        if(thing->z+thing->height > thing->ceilingz)
            thing->z = thing->ceilingz-thing->height;
    }
    
    // spawn teleport fog at source and destination
/*    fog = P_SpawnMobj (oldx, oldy, oldz+fogDelta, MT_TFOG);
    S_StartSound (fog, sfx_telept);*/
    an = angle >> ANGLETOFINESHIFT;
//    fog = P_SpawnMobj (x+20*finecosine[an], y+20*finesine[an]
//       , thing->z+fogDelta, MT_TFOG);
    
    // emit sound, where?
//    S_StartSound (fog, sfx_telept);
    
    // don't move for a bit
    if (thing->player)
    {
        thing->reactiontime = 18;
        // added : absolute angle position
        if(thing==players[consoleplayer].mo)
            localangle = angle;
        if(cv_splitscreen.value && thing==players[secondarydisplayplayer].mo)
            localangle2 = angle;
#ifdef CLIENTPREDICTION2
        if(thing==players[consoleplayer].mo)
        {
            players[consoleplayer].spirit->reactiontime = thing->reactiontime;
            CL_ResetSpiritPosition(thing);
        }
#endif
        // move chasecam at new player location
		if(cv_splitscreen.value && cv_chasecam2.value && thing->player == &players[secondarydisplayplayer])
			P_ResetCamera(thing->player, &camera2);
		else if(cv_chasecam.value && thing->player == &players[displayplayer])
			P_ResetCamera(thing->player, &camera);
    }
    
    thing->angle = angle;

    if(thing->flags&MF_MISSILE)
    {
        thing->momx = FixedMul(thing->info->speed, finecosine[an]);
        thing->momy = FixedMul(thing->info->speed, finesine[an]);
    }
    else
        thing->momx = thing->momy = thing->momz = 0;
    
    return 1;
}