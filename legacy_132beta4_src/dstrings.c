// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: dstrings.c,v 1.9 2001/08/20 20:40:39 metzgermeister Exp $
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
// $Log: dstrings.c,v $
// Revision 1.9  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.8  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.7  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.6  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.5  2000/04/20 21:49:53  hurdler
// fix a bug in dehacked
//
// Revision 1.4  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.3  2000/04/04 00:32:45  stroggonmeth
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
//      Globally defined strings.
// 
//-----------------------------------------------------------------------------

#include "dstrings.h"

char *text[NUMTEXT] = {
   "Development mode ON.\n",
   "CD-ROM Version: default.cfg from c:\\srb2data\n",
   "press a key.",
   "press y or n.",
   "only the server can do a load net game!\n\npress a key.",
   "you can't quickload during a netgame!\n\npress a key.",
   "you haven't picked a quicksave slot yet!\n\npress a key.",
   "you can't save if you aren't playing!\n\npress a key.",
   "quicksave over your game named\n\n'%s'?\n\npress y or n.",
   "do you want to quickload the game named\n\n'%s'?\n\npress y or n.",
   "You are in a network game.\n""End it?\n(Y/N).\n",
   "are you sure? this skill level\nisn't even remotely fair.\n\npress y or n.",
   "Messages OFF",
   "Messages ON",
   "you can't end a netgame!\n\npress a key.",
   "are you sure you want to end the game?\n\npress y or n.",

   "%s\n\n(press 'y' to quit)",

   "Gamma correction OFF",
   "Gamma correction level 1",
   "Gamma correction level 2",
   "Gamma correction level 3",
   "Gamma correction level 4",
   "empty slot",

   "game saved.",
   "[Message unsent]",

   "I'm ready to kick ro-butt!", // Changed by Tails: 9-14-99
   "I'm OK.",
   "I'm not looking too good!",
   "Help!",
   "You stink!", // Changed by Tails: 9-14-99
   "Next time I'll win!", // Changed by Tails: 9-14-99
   "Come here!",
   "I'll take care of it.",
   "Yes",
   "No",

   "Music Change",
   "IMPOSSIBLE SELECTION, you stupid-head!", // Changed by Tails: 9-14-99

   "Changing Level...",

   " #",

   "Two months had passed since Dr. Eggman\n"\
   "tried to take over the world using his\n"\
   "Ring Satellite.\n#",

   "As it was about to drain the rings\n"\
   "away from the planet, Sonic burst into\n"\
   "the Satellite and for what he thought\n"\
   "would be the last time, defeated\n"\
   "Dr. Eggman.\n#",

   "\nWhat Sonic, Tails, and Knuckles had\n"\
   "not anticipated was that Eggman would\n"
   "return, bringing an all new threat.\n#",

   "About once every year, a strange asteroid\n"\
   "hovers around the planet. it suddenly\n"\
   "appears from nowhere, circles around, and\n"\
   "- just as mysteriously as it arrives, it\n"\
   "vanishes after about two months.\n"\
   "No one knows why it appears, or how.\n#",

   "\"Curses!\" Eggman yelled. \"That hedgehog\n"\
   "and his ridiculous friends will pay\n"\
   "dearly for this!\" Just then his scanner\n"\
   "blipped as the Black Rock made its\n"\
   "appearance from nowhere. Eggman looked at\n"\
   "the screen, and just shrugged it off.\n#",

   "It was only later\n"\
   "that he had an\n"\
   "idea. \"The Black\n"\
   "Rock usually has a\n"\
   "lot of energy\n"\
   "within it... if I\n"\
   "can somehow\n"\
   "harness this, I\n"\
   "can turn it into\n"\
   "the ultimate\n"\
   "battle station,\n"\
   "and every last\n"\
   "person will be\n"\
   "begging for mercy,\n"\
   "including Sonic!\"\n#",

   "\n\nBefore beginning his scheme,\n"\
   "Eggman decided to give Sonic\n"\
   "a reunion party...\n#",

   "\"We're ready to fire in 15 seconds!\"\n"\
   "The robot said, his voice crackling a\n"
   "little down the com-link. \"Good!\"\n"\
   "Eggman sat back in his Egg-Mobile and\n"\
   "began to count down as he saw the\n"\
   "GreenFlower city on the main monitor.\n#",

   "\"10...9...8...\"\n"\
   "Meanwhile, Sonic was tearing across the\n"\
   "zones, and everything became nothing but\n"\
   "a blur as he ran around loops, skimmed\n"\
   "over water, and catapulted himself off\n"\
   "rocks with his phenominal speed.\n#",

   "\"5...4...3...\"\n"\
   "Sonic knew he was getting closer to the\n"\
   "City, and pushed himself harder. Finally,\n"\
   "the city appeared in the horizon.\n"\
   "\"2...1...Zero.\"\n#",

   "GreenFlower City was gone.\n"\
   "Sonic arrived just in time to see what\n"\
   "little of the 'ruins' were left. Everyone\n"\
   "and everything in the city had been\n"\
   "obliterated.\n#",

   "\"You're not quite as dead as we thought,\n"\
   "huh? Are you going to tell us your plan as\n"\
   "usual or will I 'have to work it out' or\n"\
   "something?\"                         \n"\
   "\"We'll see... let's give you a quick warm\n"\
   "up, Sonic! JETTYSYNS! Open fire!\"\n#",

   "Eggman took this\n"\
   "as his cue and\n"\
   "blasted off,\n"\
   "leaving Sonic\n"\
   "and Tails behind.\n"\
   "Tails looked at\n"\
   "the ruins of the\n"\
   "Greenflower City\n"\
   "with a grim face\n"\
   "and sighed.           \n"\
   "\"Now what do we\n"\
   "do?\", he asked.\n#",

   "\"Easy! We go\n"\
   "find Eggman\n"\
   "and stop his\n"\
   "latest\n"\
   "insane plan.\n"\
   "Just like\n"\
   "we've always\n"\
   "done, right?                 \n\n"\
   "...                    \n\n"\
   "\"Tails,what\n"\
   "*ARE* you\n"\
   "doing?\"\n#",

   "\"I'm just finding what mission obje...\n"\
   "a-ha! Here it is! This will only give\n"\
   "the robot's primary objective. It says,\n"\
   "* LOCATE AND RETRIEVE CHAOS EMERALD.\n"\
   "ESTIMATED LOCATION: GREENFLOWER ZONE *\"\n"\
   "\"All right, then let's go!\"\n#",

/*
"What are waiting for? The first emerald is ours!" Sonic was about to
run, when he saw a shadow pass over him, he recognized the silloheute
instantly.
	"Knuckles!" Sonic said. The echidna stopped his glide and landed
facing Sonic. "What are you doing here?"
	He replied, "This crisis affects the Floating Island,
if that explosion I saw is anything to go by."
	"If you're willing to help then... let's go!"
	*/

  // DOOM1
  "Eggman's tied explosives\nto your girlfriend, and\nwill activate them if\nyou press the 'Y' key!\nPress 'N' to save her!", // Changed by Tails: 9-14-99
  "What would Tails say if\nhe saw you quitting the game?", // Changed by Tails: 9-14-99
  "Hey!\nWhere do ya think you're goin'?", // Changed by Tails: 9-14-99
  "Forget your studies!\nPlay some more!", // Changed by Tails: 9-14-99
  "You're trying to say you\nlike Sonic Adventure better than\nthis, right?", // Changed by Tails: 9-14-99
  "don't leave yet -- there's a\nsuper emerald around that corner!", // Changed by Tails: 9-14-99
  "You'd rather work than play?", // Changed by Tails: 9-14-99
  "go ahead and leave. see if i care...\n*sniffle*", // Changed by Tails: 9-14-99

  // QuitDOOM II messages
  "If you leave now,\nEggman will take over the world!", // Changed by Tails: 9-14-99
  "Don't quit!\nThere are animals\nto save!", // Changed by Tails: 9-14-99
  "Aw c'mon, just bop\na few more robots!", // Changed by Tails: 9-14-99
  "Just because you can't\nget that Chaos Emerald...", // Changed by Tails: 9-14-99
  "If you leave, i'll use\nmy spin attack on you!", // Changed by Tails: 9-14-99
  "Don't go!\nYou might find the hidden\nlevels!", // Changed by Tails: 9-14-99
  "Hit the 'N' key sonic!\nThe 'N' key!", // Changed by Tails: 9-14-99

  "===========================================================================\n"
  "                       Sonic Robo Blast II!\n"                                  // Changed by Tails: 9-14-99
  "                       by Sonic Team Junior\n"                                  // Changed by Tails: 9-14-99
  "                      http://www.srb2.org\n"                              // Changed by Tails: 9-14-99
  "      This is a modified version. Go to our site for the original.\n"           // Changed by Tails: 9-14-99
  "===========================================================================\n",

  "===========================================================================\n"
  "                   We hope you enjoy this game as\n"                            // Changed by Tails: 9-14-99
  "                     much as we did making it!\n"                               // Changed by Tails: 9-14-99
  "===========================================================================\n",

  "M_LoadDefaults: Load system defaults.\n",
  "Z_Init: Init zone memory allocation daemon. \n",
  "W_Init: Init WADfiles.\n",
  "M_Init: Init miscellaneous info.\n",
  "R_Init: Init SRB2 refresh daemon - ",
  "\nP_Init: Init Playloop state.\n",
  "I_Init: Setting up machine state.\n",
  "D_CheckNetGame: Checking network game status.\n",
  "S_Init: Setting up sound.\n",
  "HU_Init: Setting up heads up display.\n",
  "ST_Init: Init status bar.\n",

  "srb2.srb",

  "c:\\srb2data\\"SAVEGAMENAME"%d.ssg", // Tails 06-10-2001
  SAVEGAMENAME"%d.ssg", // Tails 06-10-2001

  //BP: here is special dehacked handling, include centring and version
  "Sonic Robo Blast 2: V0.70", // Changed by Tails: 9-14-99

};

char savegamename[256];
