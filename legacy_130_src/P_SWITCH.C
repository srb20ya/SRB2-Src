// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: p_switch.c,v 1.5 2000/04/16 18:38:07 bpereira Exp $
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
// $Log: p_switch.c,v $
// Revision 1.5  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.4  2000/04/06 20:40:22  hurdler
// Mostly remove warnings under windows
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
//      Switches, buttons. Two-state animation. Exits.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "g_game.h"
#include "p_local.h"
#include "s_sound.h"
#include "r_main.h"
#include "w_wad.h" //SoM: 3/22/2000
#include "z_zone.h"

//
// CHANGE THE TEXTURE OF A WALL SWITCH TO ITS OPPOSITE
//
switchlist_t oldalphSwitchList[] =
{
    // Doom shareware episode 1 switches
/*    {"SW1BRCOM","SW2BRCOM",     1},
    {"SW1BRN1", "SW2BRN1",      1},
    {"SW1BRN2", "SW2BRN2",      1},
    {"SW1BRNGN","SW2BRNGN",     1},
    {"SW1BROWN","SW2BROWN",     1},
    {"SW1COMM", "SW2COMM",      1},
    {"SW1COMP", "SW2COMP",      1},
    {"SW1DIRT", "SW2DIRT",      1},
    {"SW1EXIT", "SW2EXIT",      1},
    {"SW1GRAY", "SW2GRAY",      1},
    {"SW1GRAY1","SW2GRAY1",     1},
    {"SW1METAL","SW2METAL",     1},
    {"SW1PIPE", "SW2PIPE",      1},
    {"SW1SLAD", "SW2SLAD",      1},
    {"SW1STARG","SW2STARG",     1},
    {"SW1STON1","SW2STON1",     1},
    {"SW1STON2","SW2STON2",     1},
    {"SW1STONE","SW2STONE",     1},
    {"SW1STRTN","SW2STRTN",     1},

    // Doom registered episodes 2&3 switches
    {"SW1BLUE", "SW2BLUE",      2},
    {"SW1CMT",  "SW2CMT",       2},
    {"SW1GARG", "SW2GARG",      2},
    {"SW1GSTON","SW2GSTON",     2},
    {"SW1HOT",  "SW2HOT",       2},
    {"SW1LION", "SW2LION",      2},
    {"SW1SATYR","SW2SATYR",     2},
    {"SW1SKIN", "SW2SKIN",      2},
    {"SW1VINE", "SW2VINE",      2},
    {"SW1WOOD", "SW2WOOD",      2},

    // Doom II switches
    {"SW1PANEL","SW2PANEL",     3},
    {"SW1ROCK", "SW2ROCK",      3},
    {"SW1MET2", "SW2MET2",      3},
    {"SW1WDMET","SW2WDMET",     3},
    {"SW1BRIK", "SW2BRIK",      3},
    {"SW1MOD1", "SW2MOD1",      3},
    {"SW1ZIM",  "SW2ZIM",       3},
    {"SW1STON6","SW2STON6",     3},
    {"SW1TEK",  "SW2TEK",       3},
    {"SW1MARB", "SW2MARB",      3},
    {"SW1SKULL","SW2SKULL",     3},*/

    {"\0",      "\0",           0}
};

//SoM: 3/22/2000: Switch limit removal
//int             switchlist[MAXSWITCHES * 2];

int             *switchlist;
int             max_numswitches;
int             numswitches;

button_t        buttonlist[MAXBUTTONS];

//
// P_InitSwitchList
// - this is now called at P_SetupLevel () time.
//
//SoM: 3/22/2000: Use boom code.
void P_InitSwitchList(void)
{
  int            i, index = 0;
  int            episode = (gamemode == registered || gamemode==retail) ?
                            2 : gamemode == commercial ? 3 : 1;
  switchlist_t   *alphSwitchList;

  //SoM: 3/22/2000: No Switches lump? Use old table!
  if(W_CheckNumForName("SWITCHES") != -1)
    alphSwitchList = (switchlist_t *)W_CacheLumpName("SWITCHES",PU_STATIC);
  else 
    alphSwitchList = oldalphSwitchList;

  for (i=0;;i++)
  {
    if (index+1 >= max_numswitches)
      switchlist = realloc(switchlist, sizeof *switchlist *
          (max_numswitches = max_numswitches ? max_numswitches*2 : 8));
    if (SHORT(alphSwitchList[i].episode) <= episode) //jff 5/11/98 endianess
    {
      if (!SHORT(alphSwitchList[i].episode))
        break;
      switchlist[index++] = R_TextureNumForName(alphSwitchList[i].name1);
      switchlist[index++] = R_TextureNumForName(alphSwitchList[i].name2);
    }
  }

  numswitches = index/2;
  switchlist[index] = -1;

  //SoM: 3/22/2000: Don't change tag if not from lump
  if(alphSwitchList != oldalphSwitchList)
    Z_ChangeTag(alphSwitchList,PU_CACHE);
}


//
// Start a button counting down till it turns off.
//
void P_StartButton ( line_t*       line,
                     bwhere_e      w,
                     int           texture,
                     int           time )
{
    int         i;

    // See if button is already pressed
    for (i = 0;i < MAXBUTTONS;i++)
      if (buttonlist[i].btimer && buttonlist[i].line == line)
        return;

    for (i = 0;i < MAXBUTTONS;i++)
      if (!buttonlist[i].btimer)
      {
        buttonlist[i].line = line;
        buttonlist[i].where = w;
        buttonlist[i].btexture = texture;
        buttonlist[i].btimer = time;
        buttonlist[i].soundorg = (mobj_t *)&line->frontsector->soundorg;
        return;
      }

    I_Error("P_StartButton: no button slots left!");
}





//
// Function that changes wall texture.
// Tell it if switch is ok to use again (1=yes, it's a button).
//
void P_ChangeSwitchTexture ( line_t*       line,
                             int           useAgain )
{
    int     texTop;
    int     texMid;
    int     texBot;
    int     i;
    int     sound;

    if (!useAgain)
        line->special = 0;

    texTop = sides[line->sidenum[0]].toptexture;
    texMid = sides[line->sidenum[0]].midtexture;
    texBot = sides[line->sidenum[0]].bottomtexture;

    sound = sfx_swtchn;

    // EXIT SWITCH?
    if (line->special == 11)
        sound = sfx_swtchx;

    for (i = 0;i < numswitches*2;i++)
    {
        if (switchlist[i] == texTop)
        {
            S_StartSound(buttonlist->soundorg,sound);
            sides[line->sidenum[0]].toptexture = switchlist[i^1];

            if (useAgain)
                P_StartButton(line,top,switchlist[i],BUTTONTIME);

            return;
        }
        else
        {
            if (switchlist[i] == texMid)
            {
                S_StartSound(buttonlist->soundorg,sound);
                sides[line->sidenum[0]].midtexture = switchlist[i^1];
                if (useAgain)
                    P_StartButton(line, middle,switchlist[i],BUTTONTIME);

                return;
            }
            else
            {
                if (switchlist[i] == texBot)
                {
                    S_StartSound(buttonlist->soundorg,sound);
                    sides[line->sidenum[0]].bottomtexture = switchlist[i^1];

                    if (useAgain)
                        P_StartButton(line, bottom,switchlist[i],BUTTONTIME);

                    return;
                }
            }
        }
    }
}


//
// P_UseSpecialLine
// Called when a thing uses a special line.
// Only the front sides of lines are usable.
//
boolean P_UseSpecialLine ( mobj_t*       thing,
                           line_t*       line,
                           int           side )
{

    // Err...
    // Use the back sides of VERY SPECIAL lines...
    if (side)
    {
      return false;
    }

    //SoM: 3/18/2000: Add check for Generalized linedefs.
    if (boomsupport)
    {
      // pointer to line function is NULL by default, set non-null if
      // line special is push or switch generalized linedef type
      int (*linefunc)(line_t *line)=NULL;

      // check each range of generalized linedefs
      if ((unsigned)line->special >= GenFloorBase)
      {
        if (!thing->player)
          if ((line->special & FloorChange) || !(line->special & FloorModel))
            return false; // FloorModel is "Allow Monsters" if FloorChange is 0
        if (!line->tag && ((line->special&6)!=6)) //all non-manual
          return false;                           //generalized types require tag
        linefunc = EV_DoGenFloor;
      }
      else if ((unsigned)line->special >= GenCeilingBase)
      {
        if (!thing->player)
          if ((line->special & CeilingChange) || !(line->special & CeilingModel))
            return false;   // CeilingModel is "Allow Monsters" if CeilingChange is 0
        if (!line->tag && ((line->special&6)!=6)) //all non-manual
          return false;                           //generalized types require tag
        linefunc = EV_DoGenCeiling;
      }
      else if ((unsigned)line->special >= GenDoorBase)
      {
        if (!thing->player)
        {
          if (!(line->special & DoorMonster))
            return false;   // monsters disallowed from this door
          if (line->flags & ML_SECRET) // they can't open secret doors either
            return false;
        }
        if (!line->tag && ((line->special&6)!=6)) //all non-manual
          return false;                           //generalized types require tag
        linefunc = EV_DoGenDoor;
      }
      else if ((unsigned)line->special >= GenLockedBase)
      {
        if (!thing->player)
          return false;   // monsters disallowed from unlocking doors
        if (!P_CanUnlockGenDoor(line,thing->player))
          return false;
        if (!line->tag && ((line->special&6)!=6)) //all non-manual
          return false;                           //generalized types require tag

        linefunc = EV_DoGenLockedDoor;
      }
      else if ((unsigned)line->special >= GenLiftBase)
      {
        if (!thing->player)
          if (!(line->special & LiftMonster))
            return false; // monsters disallowed
        if (!line->tag && ((line->special&6)!=6)) //all non-manual
          return false;                           //generalized types require tag
        linefunc = EV_DoGenLift;
      }
      else if ((unsigned)line->special >= GenStairsBase)
      {
        if (!thing->player)
          if (!(line->special & StairMonster))
            return false; // monsters disallowed
        if (!line->tag && ((line->special&6)!=6)) //all non-manual
          return false;                           //generalized types require tag
        linefunc = EV_DoGenStairs;
      }
      else if ((unsigned)line->special >= GenCrusherBase)
      {
        if (!thing->player)
          if (!(line->special & CrusherMonster))
            return false; // monsters disallowed
        if (!line->tag && ((line->special&6)!=6)) //all non-manual
          return false;                           //generalized types require tag
        linefunc = EV_DoGenCrusher;
      }

      if (linefunc)
        switch((line->special & TriggerType) >> TriggerTypeShift)
        {
          case PushOnce:
            if (!side)
              if (linefunc(line))
                line->special = 0;
            return true;
          case PushMany:
            if (!side)
              linefunc(line);
            return true;
          case SwitchOnce:
            if (linefunc(line))
              P_ChangeSwitchTexture(line,0);
            return true;
          case SwitchMany:
            if (linefunc(line))
              P_ChangeSwitchTexture(line,1);
            return true;
          default:  // if not a switch/push type, do nothing here
            return false;
        }
    }



    // Switches that other things can activate.
    if (!thing->player)
    {
        // never open secret doors
        if (line->flags & ML_SECRET)
            return false;

        switch(line->special)
        {
          case 1:       // MANUAL DOOR RAISE
          case 32:      // MANUAL BLUE
          case 33:      // MANUAL RED
          case 34:      // MANUAL YELLOW
          //SoM: 3/18/2000: add ability to use teleporters for monsters
          case 195:       // switch teleporters
          case 174:
          case 210:       // silent switch teleporters
          case 209:
            break;

          default:
            return false;
            break;
        }
    }

    if (!P_CheckTag(line) && boomsupport)  //disallow zero tag on some types
      return false;

    // do something
    switch (line->special)
    {
        // MANUALS
      case 1:           // Vertical Door
      case 26:          // Blue Door/Locked
      case 27:          // Yellow Door /Locked
      case 28:          // Red Door /Locked

      case 31:          // Manual door open
      case 32:          // Blue locked door open
      case 33:          // Red locked door open
      case 34:          // Yellow locked door open

      case 117:         // Blazing door raise
      case 118:         // Blazing door open
        EV_VerticalDoor (line, thing);
        break;

        //UNUSED - Door Slide Open&Close
        // case 124:
        // EV_SlidingDoor (line, thing);
        // break;

        // SWITCHES
      case 7:
        // Build Stairs
        if (EV_BuildStairs(line,build8))
            P_ChangeSwitchTexture(line,0);
        break;

      case 9:
        // Change Donut
        if (EV_DoDonut(line))
            P_ChangeSwitchTexture(line,0);
        break;

      case 11:
        // Exit level
        if(cv_allowexitlevel.value)
        {
            P_ChangeSwitchTexture(line,0);
            G_ExitLevel ();
        }
        break;

      case 14:
        // Raise Floor 32 and change texture
        if (EV_DoPlat(line,raiseAndChange,32))
            P_ChangeSwitchTexture(line,0);
        break;

      case 15:
        // Raise Floor 24 and change texture
        if (EV_DoPlat(line,raiseAndChange,24))
            P_ChangeSwitchTexture(line,0);
        break;

      case 18:
        // Raise Floor to next highest floor
        if (EV_DoFloor(line, raiseFloorToNearest))
            P_ChangeSwitchTexture(line,0);
        break;

      case 20:
        // Raise Plat next highest floor and change texture
        if (EV_DoPlat(line,raiseToNearestAndChange,0))
            P_ChangeSwitchTexture(line,0);
        break;

      case 21:
        // PlatDownWaitUpStay
        if (EV_DoPlat(line,downWaitUpStay,0))
            P_ChangeSwitchTexture(line,0);
        break;

      case 23:
        // Lower Floor to Lowest
        if (EV_DoFloor(line,lowerFloorToLowest))
            P_ChangeSwitchTexture(line,0);
        break;

      case 29:
        // Raise Door
        if (EV_DoDoor(line,normal))
            P_ChangeSwitchTexture(line,0);
        break;

      case 41:
        // Lower Ceiling to Floor
        if (EV_DoCeiling(line,lowerToFloor))
            P_ChangeSwitchTexture(line,0);
        break;

      case 71:
        // Turbo Lower Floor
        if (EV_DoFloor(line,turboLower))
            P_ChangeSwitchTexture(line,0);
        break;

      case 49:
        // Ceiling Crush And Raise
        if (EV_DoCeiling(line,crushAndRaise))
            P_ChangeSwitchTexture(line,0);
        break;

      case 50:
        // Close Door
        if (EV_DoDoor(line,doorclose))
            P_ChangeSwitchTexture(line,0);
        break;

      case 51:
        // Secret EXIT
        P_ChangeSwitchTexture(line,0);
        G_SecretExitLevel ();
        break;

      case 55:
        // Raise Floor Crush
        if (EV_DoFloor(line,raiseFloorCrush))
            P_ChangeSwitchTexture(line,0);
        break;

      case 101:
        // Raise Floor
        if (EV_DoFloor(line,raiseFloor))
            P_ChangeSwitchTexture(line,0);
        break;

      case 102:
        // Lower Floor to Surrounding floor height
        if (EV_DoFloor(line,lowerFloor))
            P_ChangeSwitchTexture(line,0);
        break;

      case 103:
        // Open Door
        if (EV_DoDoor(line,dooropen))
            P_ChangeSwitchTexture(line,0);
        break;

      case 111:
        // Blazing Door Raise (faster than TURBO!)
        if (EV_DoDoor (line,blazeRaise))
            P_ChangeSwitchTexture(line,0);
        break;

      case 112:
        // Blazing Door Open (faster than TURBO!)
        if (EV_DoDoor (line,blazeOpen))
            P_ChangeSwitchTexture(line,0);
        break;

      case 113:
        // Blazing Door Close (faster than TURBO!)
        if (EV_DoDoor (line,blazeClose))
            P_ChangeSwitchTexture(line,0);
        break;

      case 122:
        // Blazing PlatDownWaitUpStay
        if (EV_DoPlat(line,blazeDWUS,0))
            P_ChangeSwitchTexture(line,0);
        break;

      case 127:
        // Build Stairs Turbo 16
        if (EV_BuildStairs(line,turbo16))
            P_ChangeSwitchTexture(line,0);
        break;

      case 131:
        // Raise Floor Turbo
        if (EV_DoFloor(line,raiseFloorTurbo))
            P_ChangeSwitchTexture(line,0);
        break;

      case 133:
        // BlzOpenDoor BLUE
      case 135:
        // BlzOpenDoor RED
      case 137:
        // BlzOpenDoor YELLOW
        if (EV_DoLockedDoor (line,blazeOpen,thing))
            P_ChangeSwitchTexture(line,0);
        break;

      case 140:
        // Raise Floor 512
        if (EV_DoFloor(line,raiseFloor512))
            P_ChangeSwitchTexture(line,0);
        break;

      default:
        if (boomsupport)
          switch (line->special)
          {
            // added linedef types to fill all functions out so that
            // all possess SR, S1, WR, W1 types

            case 158:
              // Raise Floor to shortest lower texture
              if (EV_DoFloor(line,raiseToTexture))
                P_ChangeSwitchTexture(line,0);
              break;
  
            case 159:
              // Raise Floor to shortest lower texture
              if (EV_DoFloor(line,lowerAndChange))
                P_ChangeSwitchTexture(line,0);
              break;
        
            case 160:
              // Raise Floor 24 and change
              if (EV_DoFloor(line,raiseFloor24AndChange))
                P_ChangeSwitchTexture(line,0);
              break;

            case 161:
              // Raise Floor 24
              if (EV_DoFloor(line,raiseFloor24))
                P_ChangeSwitchTexture(line,0);
              break;

            case 162:
              // Moving floor min n to max n
              if (EV_DoPlat(line,perpetualRaise,0))
                P_ChangeSwitchTexture(line,0);
              break;

            case 163:
              // Stop Moving floor
              EV_StopPlat(line);
              P_ChangeSwitchTexture(line,0);
              break;

            case 164:
              // Start fast crusher
              if (EV_DoCeiling(line,fastCrushAndRaise))
                P_ChangeSwitchTexture(line,0);
              break;

            case 165:
              // Start slow silent crusher
              if (EV_DoCeiling(line,silentCrushAndRaise))
                P_ChangeSwitchTexture(line,0);
              break;

            case 166:
              // Raise ceiling, Lower floor
              if (EV_DoCeiling(line, raiseToHighest) ||
                  EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line,0);
              break;

            case 167:
              // Lower floor and Crush
              if (EV_DoCeiling(line, lowerAndCrush))
                P_ChangeSwitchTexture(line,0);
              break;

            case 168:
              // Stop crusher
              if (EV_CeilingCrushStop(line))
                P_ChangeSwitchTexture(line,0);
              break;

            case 169:
              // Lights to brightest neighbor sector
              EV_LightTurnOn(line,0);
              P_ChangeSwitchTexture(line,0);
              break;

            case 170:
              // Lights to near dark
              EV_LightTurnOn(line,35);
              P_ChangeSwitchTexture(line,0);
              break;

            case 171:
              // Lights on full
              EV_LightTurnOn(line,255);
              P_ChangeSwitchTexture(line,0);
              break;

            case 172:
              // Start Lights Strobing
              EV_StartLightStrobing(line);
              P_ChangeSwitchTexture(line,0);
              break;

            case 173:
              // Lights to Dimmest Near
              EV_TurnTagLightsOff(line);
              P_ChangeSwitchTexture(line,0);
              break;

            case 174:
              // Teleport
              if (EV_Teleport(line,side,thing))
                P_ChangeSwitchTexture(line,0);
              break;

            case 175:
              // Close Door, Open in 30 secs
              if (EV_DoDoor(line,close30ThenOpen))
                P_ChangeSwitchTexture(line,0);
              break;

            case 189: //create texture change no motion type
              // Texture Change Only (Trigger)
              if (EV_DoChange(line,trigChangeOnly))
                P_ChangeSwitchTexture(line,0);
              break;

            case 203:
              // Lower ceiling to lowest surrounding ceiling
              if (EV_DoCeiling(line,lowerToLowest))
                P_ChangeSwitchTexture(line,0);
              break;

            case 204:
              // Lower ceiling to highest surrounding floor
              if (EV_DoCeiling(line,lowerToMaxFloor))
                P_ChangeSwitchTexture(line,0);
              break;

            case 209:
              // killough 1/31/98: silent teleporter
              if (EV_SilentTeleport(line, side, thing))
                P_ChangeSwitchTexture(line,0);
              break;

            case 241: //jff 3/15/98 create texture change no motion type
              // Texture Change Only (Numeric)
              if (EV_DoChange(line,numChangeOnly))
                P_ChangeSwitchTexture(line,0);
              break;

            case 221:
              // Lower floor to next lowest floor
              if (EV_DoFloor(line,lowerFloorToNearest))
                P_ChangeSwitchTexture(line,0);
              break;

            case 229:
              // Raise elevator next floor
              if (EV_DoElevator(line,elevateUp))
                P_ChangeSwitchTexture(line,0);
              break;

            case 233:
              // Lower elevator next floor
              if (EV_DoElevator(line,elevateDown))
                P_ChangeSwitchTexture(line,0);
              break;

            case 237:
              // Elevator to current floor
              if (EV_DoElevator(line,elevateCurrent))
                P_ChangeSwitchTexture(line,0);
              break;


            //end of added S1 linedef types

            //added linedef types to fill all functions out so that
            //all possess SR, S1, WR, W1 types
            
            case 78:
              // Texture/type Change Only (Numeric)
              if (EV_DoChange(line,numChangeOnly))
                P_ChangeSwitchTexture(line,1);
              break;

            case 176:
              // Raise Floor to shortest lower texture
              if (EV_DoFloor(line,raiseToTexture))
                P_ChangeSwitchTexture(line,1);
              break;

            case 177:
              // Raise Floor to shortest lower texture
              if (EV_DoFloor(line,lowerAndChange))
                P_ChangeSwitchTexture(line,1);
              break;

            case 178:
              // Raise Floor 512
              if (EV_DoFloor(line,raiseFloor512))
                P_ChangeSwitchTexture(line,1);
              break;

            case 179:
              // Raise Floor 24 and change
              if (EV_DoFloor(line,raiseFloor24AndChange))
                P_ChangeSwitchTexture(line,1);
              break;

            case 180:
              // Raise Floor 24
              if (EV_DoFloor(line,raiseFloor24))
                P_ChangeSwitchTexture(line,1);
              break;

            case 181:
              // Moving floor min n to max n
              EV_DoPlat(line,perpetualRaise,0);
              P_ChangeSwitchTexture(line,1);
              break;

            case 182:
              // Stop Moving floor
              EV_StopPlat(line);
              P_ChangeSwitchTexture(line,1);
              break;

            case 183:
              // Start fast crusher
              if (EV_DoCeiling(line,fastCrushAndRaise))
                P_ChangeSwitchTexture(line,1);
              break;

            case 184:
              // Start slow crusher
              if (EV_DoCeiling(line,crushAndRaise))
                P_ChangeSwitchTexture(line,1);
              break;

            case 185:
              // Start slow silent crusher
              if (EV_DoCeiling(line,silentCrushAndRaise))
                P_ChangeSwitchTexture(line,1);
              break;

            case 186:
              // Raise ceiling, Lower floor
              if (EV_DoCeiling(line, raiseToHighest) ||
                  EV_DoFloor(line, lowerFloorToLowest))
                P_ChangeSwitchTexture(line,1);
              break;

            case 187:
              // Lower floor and Crush
              if (EV_DoCeiling(line, lowerAndCrush))
                P_ChangeSwitchTexture(line,1);
              break;

            case 188:
              // Stop crusher
              if (EV_CeilingCrushStop(line))
                P_ChangeSwitchTexture(line,1);
              break;

            case 190: //jff 3/15/98 create texture change no motion type
              // Texture Change Only (Trigger)
              if (EV_DoChange(line,trigChangeOnly))
                P_ChangeSwitchTexture(line,1);
              break;

            case 191:
              // Lower Pillar, Raise Donut
              if (EV_DoDonut(line))
                P_ChangeSwitchTexture(line,1);
              break;

            case 192:
              // Lights to brightest neighbor sector
              EV_LightTurnOn(line,0);
              P_ChangeSwitchTexture(line,1);
              break;

            case 193:
              // Start Lights Strobing
              EV_StartLightStrobing(line);
              P_ChangeSwitchTexture(line,1);
              break;

            case 194:
              // Lights to Dimmest Near
              EV_TurnTagLightsOff(line);
              P_ChangeSwitchTexture(line,1);
              break;

            case 195:
              // Teleport
              if (EV_Teleport(line,side,thing))
                P_ChangeSwitchTexture(line,1);
              break;

            case 196:
              // Close Door, Open in 30 secs
              if (EV_DoDoor(line,close30ThenOpen))
                P_ChangeSwitchTexture(line,1);
              break;

            case 205:
              // Lower ceiling to lowest surrounding ceiling
              if (EV_DoCeiling(line,lowerToLowest))
                P_ChangeSwitchTexture(line,1);
              break;

            case 206:
              // Lower ceiling to highest surrounding floor
              if (EV_DoCeiling(line,lowerToMaxFloor))
                P_ChangeSwitchTexture(line,1);
              break;

            case 210:
              // Silent teleporter
              if (EV_SilentTeleport(line, side, thing))
                P_ChangeSwitchTexture(line,1);
              break;

            case 211:
              // Toggle Floor Between C and F Instantly
              if (EV_DoPlat(line,toggleUpDn,0))
                P_ChangeSwitchTexture(line,1);
              break;

            case 222:
              // Lower floor to next lowest floor
              if (EV_DoFloor(line,lowerFloorToNearest))
                P_ChangeSwitchTexture(line,1);
              break;

            case 230:
              // Raise elevator next floor
              if (EV_DoElevator(line,elevateUp))
                P_ChangeSwitchTexture(line,1);
              break;

            case 234:
              // Lower elevator next floor
              if (EV_DoElevator(line,elevateDown))
                P_ChangeSwitchTexture(line,1);
              break;

            case 238:
              // Elevator to current floor
              if (EV_DoElevator(line,elevateCurrent))
                P_ChangeSwitchTexture(line,1);
              break;

            case 258:
              // Build stairs, step 8
              if (EV_BuildStairs(line,build8))
                P_ChangeSwitchTexture(line,1);
              break;

            case 259:
              // Build stairs, step 16
              if (EV_BuildStairs(line,turbo16))
                P_ChangeSwitchTexture(line,1);
              break;

            // end of added SR linedef types

          }
        break;


        // BUTTONS
      case 42:
        // Close Door
        if (EV_DoDoor(line,doorclose))
            P_ChangeSwitchTexture(line,1);
        break;

      case 43:
        // Lower Ceiling to Floor
        if (EV_DoCeiling(line,lowerToFloor))
            P_ChangeSwitchTexture(line,1);
        break;

      case 45:
        // Lower Floor to Surrounding floor height
        if (EV_DoFloor(line,lowerFloor))
            P_ChangeSwitchTexture(line,1);
        break;

      case 60:
        // Lower Floor to Lowest
        if (EV_DoFloor(line,lowerFloorToLowest))
            P_ChangeSwitchTexture(line,1);
        break;

      case 61:
        // Open Door
        if (EV_DoDoor(line,dooropen))
            P_ChangeSwitchTexture(line,1);
        break;

      case 62:
        // PlatDownWaitUpStay
        if (EV_DoPlat(line,downWaitUpStay,1))
            P_ChangeSwitchTexture(line,1);
        break;

      case 63:
        // Raise Door
        if (EV_DoDoor(line,normal))
            P_ChangeSwitchTexture(line,1);
        break;

      case 64:
        // Raise Floor to ceiling
        if (EV_DoFloor(line,raiseFloor))
            P_ChangeSwitchTexture(line,1);
        break;

      case 66:
        // Raise Floor 24 and change texture
        if (EV_DoPlat(line,raiseAndChange,24))
            P_ChangeSwitchTexture(line,1);
        break;

      case 67:
        // Raise Floor 32 and change texture
        if (EV_DoPlat(line,raiseAndChange,32))
            P_ChangeSwitchTexture(line,1);
        break;

      case 65:
        // Raise Floor Crush
        if (EV_DoFloor(line,raiseFloorCrush))
            P_ChangeSwitchTexture(line,1);
        break;

      case 68:
        // Raise Plat to next highest floor and change texture
        if (EV_DoPlat(line,raiseToNearestAndChange,0))
            P_ChangeSwitchTexture(line,1);
        break;

      case 69:
        // Raise Floor to next highest floor
        if (EV_DoFloor(line, raiseFloorToNearest))
            P_ChangeSwitchTexture(line,1);
        break;

      case 70:
        // Turbo Lower Floor
        if (EV_DoFloor(line,turboLower))
            P_ChangeSwitchTexture(line,1);
        break;

      case 114:
        // Blazing Door Raise (faster than TURBO!)
        if (EV_DoDoor (line,blazeRaise))
            P_ChangeSwitchTexture(line,1);
        break;

      case 115:
        // Blazing Door Open (faster than TURBO!)
        if (EV_DoDoor (line,blazeOpen))
            P_ChangeSwitchTexture(line,1);
        break;

      case 116:
        // Blazing Door Close (faster than TURBO!)
        if (EV_DoDoor (line,blazeClose))
            P_ChangeSwitchTexture(line,1);
        break;

      case 123:
        // Blazing PlatDownWaitUpStay
        if (EV_DoPlat(line,blazeDWUS,0))
            P_ChangeSwitchTexture(line,1);
        break;

      case 132:
        // Raise Floor Turbo
        if (EV_DoFloor(line,raiseFloorTurbo))
            P_ChangeSwitchTexture(line,1);
        break;

      case 99:
        // BlzOpenDoor BLUE
      case 134:
        // BlzOpenDoor RED
      case 136:
        // BlzOpenDoor YELLOW
        if (EV_DoLockedDoor (line,blazeOpen,thing))
            P_ChangeSwitchTexture(line,1);
        break;

      case 138:
        // Light Turn On
        EV_LightTurnOn(line,255);
        P_ChangeSwitchTexture(line,1);
        break;

      case 139:
        // Light Turn Off
        EV_LightTurnOn(line,35);
        P_ChangeSwitchTexture(line,1);
        break;

    }

    return true;
}
