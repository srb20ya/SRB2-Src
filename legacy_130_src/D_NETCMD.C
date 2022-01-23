// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_netcmd.c,v 1.12 2000/08/10 14:51:25 ydario Exp $
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
// $Log: d_netcmd.c,v $
// Revision 1.12  2000/08/10 14:51:25  ydario
// OS/2 port
//
// Revision 1.11  2000/05/13 19:52:10  metzgermeister
// cd vol jiggle
//
// Revision 1.10  2000/04/23 16:19:52  bpereira
// no message
//
// Revision 1.9  2000/04/16 18:38:07  bpereira
// no message
//
// Revision 1.8  2000/04/07 23:11:17  metzgermeister
// added mouse move
//
// Revision 1.7  2000/04/04 00:32:45  stroggonmeth
// Initial Boom compatability plus few misc changes all around.
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.5  2000/03/06 15:58:47  hurdler
// Add Bell Kin's changes
//
// Revision 1.4  2000/03/05 17:10:56  bpereira
// no message
//
// Revision 1.3  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      host/client network commands
//      commands are executed through the command buffer
//      like console commands
//      other miscellaneous commands (at the end)
//
//-----------------------------------------------------------------------------


#include "doomdef.h"

#include "console.h"
#include "command.h"

#include "d_netcmd.h"
#include "i_system.h"
#include "dstrings.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "g_input.h"
#include "m_menu.h"
#include "r_local.h"
#include "r_things.h"
#include "p_inter.h"
#include "p_setup.h"
#include "s_sound.h"
#include "m_misc.h"
#include "am_map.h"
#include "byteptr.h"
#include "d_netfil.h"
#include "p_spec.h"
#include "m_cheat.h"
#include "d_clisrv.h"

// ------
// protos
// ------
byte P_Random(); // Tails 07-22-2001
void Command_Color_f (void);
void Command_Name_f (void);

void Command_WeaponPref(void);

void Got_NameAndcolor(char **cp,int playernum);
void Got_WeaponPref  (char **cp,int playernum);
void Got_Mapcmd      (char **cp,int playernum);
void Got_ExitLevelcmd(char **cp,int playernum);
void Got_LoadGamecmd (char **cp,int playernum);
void Got_SaveGamecmd (char **cp,int playernum);
void Got_Pause       (char **cp,int playernum);

void TeamPlay_OnChange(void);
void FragLimit_OnChange(void);
void Deahtmatch_OnChange(void);
void GameType_OnChange(void); // Tails 03-13-2001
void TimeLimit_OnChange(void);


void Command_Playdemo_f (void);
void Command_Timedemo_f (void);
void Command_Stopdemo_f (void);
void Command_Map_f (void);
void Command_Teleport_f (void); // Tails 10-02-2001
void Command_Restart_f (void);

void Command_Addfile (void);
void Command_Pause(void);

void Command_Frags_f (void);
void Command_TeamFrags_f(void);
void Command_Version_f (void);
void Command_Quit_f (void);

void Command_Water_f (void);
void Command_ExitLevel_f(void);
void Command_Load_f(void);
void Command_Save_f(void);

boolean P_TeleportMove(); // Tails 10-02-2001

// =========================================================================
//                           CLIENT VARIABLES
// =========================================================================

void SendWeaponPref(void);
void SendNameAndColor(void);
void SendNameAndColor2(void);

// these two are just meant to be saved to the config
consvar_t cv_playername           = {"name"                ,"sonic"       ,CV_CALL | CV_NOINIT,NULL,SendNameAndColor};
consvar_t cv_playercolor          = {"color"               ,"7"        ,CV_CALL | CV_NOINIT,Color_cons_t,SendNameAndColor}; // Don't save Tails 03-26-2001
// player's skin, saved for commodity, when using a favorite skins wad..
consvar_t cv_skin                 = {"skin"                ,DEFAULTSKIN,CV_CALL | CV_NOINIT,NULL /*skin_cons_t*/,SendNameAndColor}; // Don't save Tails 03-26-2001
consvar_t cv_weaponpref           = {"weaponpref"          ,"014576328",CV_SAVE | CV_CALL | CV_NOINIT,NULL,SendWeaponPref};
consvar_t cv_autoaim              = {"autoaim"             ,"1"        ,CV_SAVE | CV_CALL | CV_NOINIT,CV_OnOff,SendWeaponPref};
consvar_t cv_originalweaponswitch = {"originalweaponswitch","0"        ,CV_SAVE | CV_CALL | CV_NOINIT,CV_OnOff,SendWeaponPref};
// secondary player for splitscreen mode
consvar_t cv_playername2          = {"name2"               ,"sonic"    ,CV_CALL | CV_NOINIT,NULL,SendNameAndColor2}; // Tails
consvar_t cv_playercolor2         = {"color2"              ,"7"        ,CV_CALL | CV_NOINIT,Color_cons_t,SendNameAndColor2};// Don't save Tails 03-26-2001
consvar_t cv_skin2                = {"skin2"               ,DEFAULTSKIN,CV_CALL | CV_NOINIT,NULL /*skin_cons_t*/,SendNameAndColor2};// Don't save Tails 03-26-2001

CV_PossibleValue_t PreferredTeam_cons_t[]={{1,"Red"},{2,"Blue"},{0,NULL}}; // Tails 07-22-2001

consvar_t cv_preferredteam        = {"preferredteam"       ,"1",CV_CALL,PreferredTeam_cons_t,SendNameAndColor}; // Tails 07-22-2001

CV_PossibleValue_t usemouse_cons_t[]={{0,"Off"},{1,"On"},{2,"Force"},{0,NULL}};

#ifdef LMOUSE2
CV_PossibleValue_t mouse2port_cons_t[]={{0,"/dev/gpmdata"},{1,"/dev/ttyS0"},{2,"/dev/ttyS1"},{3,"/dev/ttyS2"},{4,"/dev/ttyS3"},{0,NULL}};
#else
CV_PossibleValue_t mouse2port_cons_t[]={{1,"COM1"},{2,"COM2"},{3,"COM3"},{4,"COM4"},{0,NULL}};
#endif

#ifdef LJOYSTICK
CV_PossibleValue_t joyport_cons_t[]={{1,"/dev/js0"},{2,"/dev/js1"},{3,"/dev/js2"},{4,"/dev/js3"},{0,NULL}};
#endif

#ifdef __WIN32__
#define usejoystick_cons_t  NULL        // accept whatever value
                                        // it is in fact the joystick device number
#else
#ifdef __GO32__
CV_PossibleValue_t usejoystick_cons_t[]={{0,"Off"}
                                        ,{1,"4 BUttons"}
                                        ,{2,"Standart"}
                                        ,{3,"6 Buttons"}
                                        ,{4,"Wingman Extreme"}
                                        ,{5,"Flightstick Pro"}
                                        ,{6,"8 Buttons"}
                                        ,{7,"Sidewinder"}
                                        ,{8,"GamePad Pro"}
                                        ,{9,"Snes lpt1"}
                                        ,{10,"Snes lpt2"}
                                        ,{11,"Snes lpt3"}
                                        ,{12,"Wingman Warrior"}
                                        ,{0,NULL}};
#else
#ifdef LINUX
#define usejoystick_cons_t  NULL
#else
#ifdef __OS2__
#define usejoystick_cons_t  NULL
#else
#error "cv_usejoystick don't have possible value for this OS !"
#endif
#endif
#endif
#endif

consvar_t cv_usemouse    = {"use_mouse","1", CV_SAVE | CV_CALL,usemouse_cons_t,I_StartupMouse};
consvar_t cv_usemouse2   = {"use_mouse2","0", CV_SAVE | CV_CALL,usemouse_cons_t,I_StartupMouse2};
consvar_t cv_usejoystick = {"use_joystick","0",CV_SAVE | CV_CALL,usejoystick_cons_t,I_InitJoystick};
#ifdef LJOYSTICK
extern void I_JoyScale();
consvar_t cv_joyport = {"joyport","/dev/js0", CV_SAVE, joyport_cons_t};
consvar_t cv_joyscale = {"joyscale","0",CV_SAVE | CV_CALL,NULL,I_JoyScale};
#endif
#ifdef LMOUSE2
consvar_t cv_mouse2port  = {"mouse2port","/dev/gpmdata", CV_SAVE, mouse2port_cons_t };
consvar_t cv_mouse2opt = {"mouse2opt","0", CV_SAVE, NULL};
#else
consvar_t cv_mouse2port  = {"mouse2port","COM2", CV_SAVE, mouse2port_cons_t };
#endif
CV_PossibleValue_t teamplay_cons_t[]={{0,"Off"},{1,"Color"},{2,"Skin"},{3,NULL}};
CV_PossibleValue_t deathmatch_cons_t[]={{0,"Coop"},{1,"1"},{2,"2"},{3,"3"},{0,NULL}};
CV_PossibleValue_t gametype_cons_t[]={{0,"Coop"},{1,"Match"},{2,"Race"},{3,"Tag"},{4,"CTF"},{0,NULL}}; // Tails 03-13-2001
CV_PossibleValue_t fraglimit_cons_t[]={{0,"MIN"},{1000,"MAX"},{0,NULL}};

consvar_t cv_teamplay   = {"teamplay"  ,"0",CV_NETVAR | CV_CALL,teamplay_cons_t, TeamPlay_OnChange};
consvar_t cv_teamdamage = {"teamdamage","0",CV_NETVAR,CV_OnOff};
consvar_t cv_timetic = {"timetic","0",0,CV_OnOff}; // Tails 04-01-2001
consvar_t cv_debug = {"debug","0",0,CV_OnOff}; // Tails 06-17-2001

consvar_t cv_fraglimit  = {"fraglimit" ,"0",CV_NETVAR | CV_CALL | CV_NOINIT,fraglimit_cons_t, FragLimit_OnChange};
consvar_t cv_timelimit  = {"timelimit" ,"0",CV_NETVAR | CV_CALL | CV_NOINIT,CV_Unsigned, TimeLimit_OnChange};
consvar_t cv_deathmatch = {"deathmatch","0",CV_NETVAR | CV_CALL,deathmatch_cons_t, Deahtmatch_OnChange};
consvar_t cv_gametype   = {"gametype","0",CV_NETVAR | CV_CALL,gametype_cons_t, GameType_OnChange};
consvar_t cv_autoctf   = {"autoctf","0",CV_NETVAR, CV_YesNo, NULL}; // Tails 07-22-2001
consvar_t cv_allowexitlevel = {"allowexitlevel"  ,"1",CV_NETVAR , CV_YesNo, NULL};

consvar_t cv_netstat = {"netstat","0",0,CV_OnOff};


// =========================================================================
//                           CLIENT STARTUP
// =========================================================================

// register client and server commands
//
void D_RegisterClientCommands (void)
{
    int i;

    for(i=0;i<MAXSKINCOLORS;i++)
        Color_cons_t[i].strvalue=Color_Names[i];

  //
  // register commands
  //
    RegisterNetXCmd(XD_NAMEANDCOLOR,Got_NameAndcolor);
    RegisterNetXCmd(XD_WEAPONPREF,Got_WeaponPref);
    RegisterNetXCmd(XD_MAP,Got_Mapcmd);
    RegisterNetXCmd(XD_EXITLEVEL,Got_ExitLevelcmd);
    RegisterNetXCmd(XD_PAUSE,Got_Pause);

    COM_AddCommand ("playdemo", Command_Playdemo_f);
    COM_AddCommand ("timedemo", Command_Timedemo_f);
    COM_AddCommand ("stopdemo", Command_Stopdemo_f);
    COM_AddCommand ("map", Command_Map_f);
    COM_AddCommand ("teleport", Command_Teleport_f); // Tails 10-02-2001
    COM_AddCommand ("restartlevel", Command_Restart_f);
    COM_AddCommand ("exitlevel",Command_ExitLevel_f);

    COM_AddCommand ("addfile", Command_Addfile);
    COM_AddCommand ("pause", Command_Pause);

//    COM_AddCommand ("turbo", Command_Turbo_f);     // turbo speed
    COM_AddCommand ("version", Command_Version_f);
    COM_AddCommand ("quit", Command_Quit_f);

    COM_AddCommand ("chatmacro", Command_Chatmacro_f); // hu_stuff.c
    COM_AddCommand ("setcontrol", Command_Setcontrol_f);
    COM_AddCommand ("setcontrol2", Command_Setcontrol2_f);

    COM_AddCommand ("frags",Command_Frags_f);
    COM_AddCommand ("teamfrags",Command_TeamFrags_f);

    COM_AddCommand ("saveconfig",Command_SaveConfig_f);
    COM_AddCommand ("loadconfig",Command_LoadConfig_f);
    COM_AddCommand ("changeconfig",Command_ChangeConfig_f);
    COM_AddCommand ("screenshot",M_ScreenShot);

  //
  // register main variables
  //
    //register these so it is saved to config
    cv_playername.defaultvalue=I_GetUserName();
    if( cv_playername.defaultvalue == NULL )
        cv_playername.defaultvalue = "Player"; // Tails 03-26-2001
    CV_RegisterVar (&cv_playername);
    CV_RegisterVar (&cv_playercolor);
    CV_RegisterVar (&cv_weaponpref);
    CV_RegisterVar (&cv_autoaim);
    CV_RegisterVar (&cv_originalweaponswitch);

    // r_things.c (skin NAME)
    CV_RegisterVar (&cv_skin);
    // secondary player (splitscreen)
    CV_RegisterVar (&cv_skin2);
    CV_RegisterVar (&cv_playername2);
    CV_RegisterVar (&cv_playercolor2);

	CV_RegisterVar (&cv_preferredteam); // Tails 07-22-2001

    //FIXME: not to be here.. but needs be done for config loading
    CV_RegisterVar (&cv_usegamma);
    CV_RegisterVar (&cv_viewsize);

    //m_menu.c
    CV_RegisterVar (&cv_crosshair);
    CV_RegisterVar (&cv_autorun);
    CV_RegisterVar (&cv_invertmouse);
    CV_RegisterVar (&cv_alwaysfreelook);
    CV_RegisterVar (&cv_mousemove);
    CV_RegisterVar (&cv_showmessages);

    //g_input.c
    CV_RegisterVar (&cv_usemouse2);
    CV_RegisterVar (&cv_invertmouse2);
    CV_RegisterVar (&cv_alwaysfreelook2);
    CV_RegisterVar (&cv_mousemove2);
    CV_RegisterVar (&cv_mousesens2);
    CV_RegisterVar (&cv_mlooksens2);
    // WARNING : the order is important when inititing mouse2 
    //           we need the mouse2port
    CV_RegisterVar (&cv_mouse2port); 
#ifdef LMOUSE2
    CV_RegisterVar (&cv_mouse2opt);
#endif
    CV_RegisterVar (&cv_mousesens);
    CV_RegisterVar (&cv_mlooksens);
    CV_RegisterVar (&cv_controlperkey);

    CV_RegisterVar (&cv_usemouse);
    CV_RegisterVar (&cv_usejoystick);
#ifdef LJOYSTICK
    CV_RegisterVar (&cv_joyport);
    CV_RegisterVar (&cv_joyscale);
#endif
    CV_RegisterVar (&cv_allowjump);
    CV_RegisterVar (&cv_allowrocketjump);
    CV_RegisterVar (&cv_allowautoaim);
//    CV_RegisterVar (&cv_allowturbo);
    CV_RegisterVar (&cv_allowexitlevel);

	CV_RegisterVar (&cv_analog); // Analog Test Tails 06-10-2001

    //s_sound.c
    CV_RegisterVar (&cv_soundvolume);
    CV_RegisterVar (&cv_musicvolume);
    CV_RegisterVar (&cv_numChannels);

    //i_cdmus.c
    CV_RegisterVar (&cd_volume);
    CV_RegisterVar (&cdUpdate);
#ifdef LINUX
    CV_RegisterVar (&cv_jigglecdvol);
#endif

    // screen.c ?
    CV_RegisterVar (&cv_fullscreen);  // only for opengl so use differant name please and move it to differant place
    CV_RegisterVar (&cv_scr_depth);
    CV_RegisterVar (&cv_scr_width);
    CV_RegisterVar (&cv_scr_height);

    // add cheat commands, I'm bored of deh patches renaming the idclev ! :-)
    COM_AddCommand ("noclip", Command_CheatNoClip_f);
    COM_AddCommand ("god", Command_CheatGod_f);
    COM_AddCommand ("gimme", Command_CheatGimme_f);

    // p_mobj.c
    CV_RegisterVar (&cv_itemrespawntime);
    CV_RegisterVar (&cv_itemrespawn);
    CV_RegisterVar (&cv_flagtime); // Tails 08-03-2001
    CV_RegisterVar (&cv_respawnmonsters);
    CV_RegisterVar (&cv_respawnmonsterstime);
    CV_RegisterVar (&cv_fastmonsters);
    CV_RegisterVar (&cv_splats);

    // WATER HACK TEST UNTIL FULLY FINISHED
    COM_AddCommand ("dev_water", Command_Water_f);

    //misc
    CV_RegisterVar (&cv_teamplay);
    CV_RegisterVar (&cv_teamdamage);
    CV_RegisterVar (&cv_fraglimit);
    CV_RegisterVar (&cv_deathmatch);
	CV_RegisterVar (&cv_gametype); // Tails 03-13-2001
	CV_RegisterVar (&cv_timetic); // Tails 04-01-2001
	CV_RegisterVar (&cv_debug); // Tails 06-17-2001
	CV_RegisterVar (&cv_autoctf); // Tails 06-17-2001
    CV_RegisterVar (&cv_timelimit);
    CV_RegisterVar (&cv_playdemospeed);
    CV_RegisterVar (&cv_netstat);
  
    COM_AddCommand ("load",Command_Load_f);
    RegisterNetXCmd(XD_LOADGAME,Got_LoadGamecmd);
    COM_AddCommand ("save",Command_Save_f);
    RegisterNetXCmd(XD_SAVEGAME,Got_SaveGamecmd);

/* ideas of commands names from Quake
    "status"
    "notarget"
    "fly"
    "restart"
    "changelevel"
    "connect"
    "reconnect"
    "noclip"
    "tell"
    "kill"
    "spawn"
    "begin"
    "prespawn"
    "ping"
    "give"

    "startdemos"
    "demos"
    "stopdemo"
*/

}

// =========================================================================
//                            CLIENT STUFF
// =========================================================================

//  name <playername> : name has changed
//
void SendNameAndColor(void)
{
    char     buf[MAXPLAYERNAME+1+SKINNAMESIZE+1],*p;
	int team; // Tails 07-31-2001
	int y; // Tails 07-31-2001
	int z; // Tails 07-31-2001
	int i; // Tails 07-31-2001

    p=buf;


	WRITEBYTE(p,cv_playercolor.value);

	if(cv_gametype.value == 4)
		{
			if(players[consoleplayer].ctfteam != 1 && players[consoleplayer].ctfteam != 2)
			{
			team = cv_preferredteam.value;

			if(team < 1)
				team = 1;

				if(cv_autoctf.value)
				{
					y = z = 0;
					for (i=0; i<MAXPLAYERS; i++)
					{
						if(players[i].ctfteam == 1)
							z++;
						else if (players[i].ctfteam == 2)
							y++;
					}
					
					if(z > y)
						team = 2;
					else if (y > z)
						team = 1;
					else
					{
						if(P_Random() &1)
							team = 2;
						else
							team = 1;
					}
				}
				WRITEBYTE(p,team); // CTF Tails 07-31-2001
			}
			else
				WRITEBYTE(p, players[consoleplayer].ctfteam);
		}
    WRITESTRINGN(p,cv_playername.string,MAXPLAYERNAME);
    *(p-1) = 0; // finish the string;

    // check if player has the skin loaded (cv_skin may have
    //  the name of a skin that was available in the previous game)
    cv_skin.value=R_SkinAvailable(cv_skin.string);

    if (!cv_skin.value)
	{
        WRITESTRINGN(p,skins[0].name,SKINNAMESIZE);
	}
    else
	{
        WRITESTRINGN(p,cv_skin.string,SKINNAMESIZE);
	}

    *(p-1) = 0; // finish the string;

    SendNetXCmd(XD_NAMEANDCOLOR,buf,p-buf);

	// Force normal player colors in Single Player and CTF modes Tails 06-15-2001
	if(!(multiplayer || netgame))
	{
		if(cv_skin.value == 0 && cv_playercolor.value != 7)
		{
			CV_SetValue(&cv_playercolor, 7);
		}
		else if(cv_skin.value == 1 && cv_playercolor.value != 5)
		{
			CV_SetValue(&cv_playercolor, 5);
		}
		else if(cv_skin.value == 2 && cv_playercolor.value != 0)
		{
			CV_SetValue(&cv_playercolor, 0);
		}
	}
	else if(cv_gametype.value == 4)
	{
		if(players[consoleplayer].ctfteam == 1 && cv_playercolor.value != 6)
		{
			CV_SetValue(&cv_playercolor, 6);
		}
		else if(players[consoleplayer].ctfteam == 2 && cv_playercolor.value != 7)
		{
			CV_SetValue(&cv_playercolor, 7);
		}
	}
}

// splitscreen
void SendNameAndColor2(void)
{
    char     buf[MAXPLAYERNAME+1+SKINNAMESIZE+1],*p;

    p=buf;
    WRITEBYTE(p,cv_playercolor2.value);
    WRITESTRINGN(p,cv_playername2.string,MAXPLAYERNAME);
    *(p-1) = 0; // finish teh string;

    // check if player has the skin loaded (cv_skin may have
    //  the name of a skin that was available in the previous game)
    cv_skin2.value=R_SkinAvailable(cv_skin2.string);
    if (!cv_skin2.value)
        WRITESTRINGN(p,DEFAULTSKIN,SKINNAMESIZE)
    else
        WRITESTRINGN(p,cv_skin2.string,SKINNAMESIZE);
    *(p-1) = 0; // finish the string;

    SendNetXCmd2(XD_NAMEANDCOLOR,buf,p-buf);
}


void Got_NameAndcolor(char **cp,int playernum)
{
    player_t *p=&players[playernum];

    // color
    p->skincolor=READBYTE(*cp) % MAXSKINCOLORS;

    // a copy of color
    if(p->mo)
        p->mo->flags =  (p->mo->flags & ~MF_TRANSLATION)
                     | ((p->skincolor)<<MF_TRANSSHIFT);

	// CTF Stuff Tails 07-31-2001
	if(cv_gametype.value == 4)
		p->ctfteam=READBYTE(*cp); // CTF Tails 07-31-2001


    // name
    if(demoversion>=128)
        READSTRING(*cp,player_names[playernum])
    else
    {
        memcpy(player_names[playernum],*cp,MAXPLAYERNAME);
        *cp+=MAXPLAYERNAME;
    }

    // skin
    if (demoversion<120 || demoversion>=125)
    {
        if(demoversion>=128)
        {
            SetPlayerSkin(playernum,*cp);
            SKIPSTRING(*cp);
        }
        else
    {
        SetPlayerSkin(playernum,*cp);
        *cp+=(SKINNAMESIZE+1);
    }
    }
}

void SendWeaponPref(void)
{
    char buf[NUMWEAPONS+2];

    if(strlen(cv_weaponpref.string)!=NUMWEAPONS)
    {
        CONS_Printf("weaponpref must have %d characters",NUMWEAPONS);
        return;
    }
    buf[0]=cv_originalweaponswitch.value;
    memcpy(buf+1,cv_weaponpref.string,NUMWEAPONS);
    buf[1+NUMWEAPONS]=cv_autoaim.value;
    SendNetXCmd(XD_WEAPONPREF,buf,NUMWEAPONS+2);
    // FIXME : the split screen player have the same weapon pref of the first player
    if(cv_splitscreen.value)
        SendNetXCmd2(XD_WEAPONPREF,buf,NUMWEAPONS+2);
}

void Got_WeaponPref(char **cp,int playernum)
{
    players[playernum].originalweaponswitch = *(*cp)++;
    memcpy(players[playernum].favoritweapon,*cp,NUMWEAPONS);
    *cp+=NUMWEAPONS;
    players[playernum].autoaim_toggle=*(*cp)++;
}

void D_SendPlayerConfig(void)
{
    SendNameAndColor();
    if(cv_splitscreen.value)
        SendNameAndColor2();
    SendWeaponPref();
}

// Teleport Command! Tails 10-02-2001
void Command_Teleport_f (void)
{
    int intx;
	int inty;
	int i;
    player_t *p=&players[consoleplayer];

	if(!cv_debug.value)
		return;

    if (COM_Argc()<3 || COM_Argc()>7)
    {
        CONS_Printf ("teleport -x <value> -y <value> : teleport to a location\n");
        return;
    }

    if(netgame)
    {
        CONS_Printf ("You can't teleport while in a netgame!\n");
        return;
    }

    if((i=COM_CheckParm("-x"))!=0)
        intx=atoi(COM_Argv(i+1));
    else
	{
		CONS_Printf("X value not specified\n");
        return;
	}

    if((i=COM_CheckParm("-y"))!=0)
        inty=atoi(COM_Argv(i+1));
    else
	{
		CONS_Printf("Y value not specified\n");
        return;
	}

    CONS_Printf ("Teleporting...");

    if (!P_TeleportMove (p->mo, intx*FRACUNIT, inty*FRACUNIT))
	{
		CONS_Printf("Unable to teleport to that spot!\n");
		return;
	}

	p->mo->z = p->mo->floorz;  //fixme: not needed?

	p->viewz = p->mo->z+p->viewheight;

	p->mo->momx = p->mo->momy = p->mo->momz = 0;

	return;
}


// ========================================================================

//  play a demo, add .lmp for external demos
//  eg: playdemo demo1 plays the internal game demo
//
// byte*   demofile;       //demo file buffer

void Command_Playdemo_f (void)
{
    char    name[256];

    if (COM_Argc() != 2)
    {
        CONS_Printf ("playdemo <demoname> : playback a demo\n");
        return;
    }

    // disconnect from server here ?
    if(demoplayback)
        G_StopDemo();
    if(netgame)
    {
        CONS_Printf("\nYou can't play a demo while in net game\n");
        return;
    }

    // open the demo file
    strcpy (name, COM_Argv(1));
    // dont add .lmp so internal game demos can be played
    //FIL_DefaultExtension (name, ".lmp");

    CONS_Printf ("Playing back demo '%s'.\n", name);

    G_DoPlayDemo(name);
}

void Command_Timedemo_f (void)
{
    char    name[256];

    if (COM_Argc() != 2)
    {
        CONS_Printf ("timedemo <demoname> : time a demo\n");
        return;
    }

    // disconnect from server here ?
    if(demoplayback)
        G_StopDemo();
    if(netgame)
    {
        CONS_Printf("\nYou can't play a demo while in net game\n");
        return;
    }

    // open the demo file
    strcpy (name, COM_Argv(1));
    // dont add .lmp so internal game demos can be played
    //FIL_DefaultExtension (name, ".lmp");

    CONS_Printf ("Timing demo '%s'.\n", name);

    G_TimeDemo (name);
}

//  stop current demo
//
void Command_Stopdemo_f (void)
{
    G_CheckDemoStatus ();
    CONS_Printf ("Stopped demo.\n");
}


//  Warp to map code.
//  Called either from map <mapname> console command, or idclev cheat.
//
void Command_Map_f (void)
{
    char buf[MAX_WADPATH+3];
#define MAPNAME &buf[2]
    int i;

    if (COM_Argc()<2 || COM_Argc()>7)
    {
        CONS_Printf ("map <mapname[.wad]> [-skill <1..5>] [-monsters <0/1>] [-noresetplayers]: warp to map\n");
        return;
    }

    if(!server)
    {
        CONS_Printf ("Only the server can change the map\n");
        return;
    }

    strcpy(MAPNAME,COM_Argv(1));

    if (FIL_CheckExtension(MAPNAME))
    {
        // here check if file exist !!!
        if( !recsearch(MAPNAME,0,false) )
        {
            CONS_Printf("\2File %s' not found\n",MAPNAME);
            return;
        }
    }
    else
    {
        // internal wad lump
        if(W_CheckNumForName(MAPNAME)==-1)
        {
            CONS_Printf("\2Internal game map '%s' not found\n"
                        "(use .wad extension for external maps)\n",MAPNAME);
            return;
        }
    }

    if((i=COM_CheckParm("-skill"))!=0)
        buf[0]=atoi(COM_Argv(i+1))-1;
    else
        buf[0]=gameskill;

    if((i=COM_CheckParm("-monsters"))!=0)
        buf[1]=(atoi(COM_Argv(i+1))==0);
    else
        buf[1]=(nomonsters!=0);

    // use only one bit
    if(buf[1])
        buf[1]=1;

    if(COM_CheckParm("-noresetplayers"))
        buf[1]|=2;

    // spaw the server if needed
    // reset players if there is a new one
    if( SV_SpawnServer() )
        buf[1]&=~2;

    SendNetXCmd(XD_MAP,buf,2+strlen(MAPNAME)+1);
}

void Got_Mapcmd(char **cp,int playernum)
{
    char mapname[MAX_WADPATH];
    int skill,resetplayer=1;

    skill=READBYTE(*cp);
    if(demoversion>=128)
        nomonsters=READBYTE(*cp);

    if(demoversion>=129)
    {
        resetplayer=((nomonsters & 2)==0);
        nomonsters&=1;
    }
    strcpy(mapname,*cp);
    *cp+=strlen(mapname)+1;

    CONS_Printf ("Warping to map...\n");
    if(demoplayback && !timingdemo)
        precache=false;
    G_InitNew (skill, mapname,resetplayer);
    if(demoplayback && !timingdemo)
        precache=true;
    CON_ToggleOff ();
}

void Command_Restart_f (void)
{
	if(!server)
	{
		CONS_Printf("Only the server can restart the level\n");
		return;
	}
    else if(!netgame)
    {
        CONS_Printf("Restartlevel is not allowed in single player\n");
        return;
    }

    if( gamestate == GS_LEVEL )
        COM_BufAddText (va("map \"%s\"\n",G_BuildMapName(gameepisode, gamemap)));
    else
        CONS_Printf("You should be in a level to restart it !\n");
}

void Command_Pause(void)
{
    SendNetXCmd(XD_PAUSE,NULL,0);
}

void Got_Pause(char **cp,int playernum)
{
    paused ^= 1;
    if(!demoplayback)
    {
        if(netgame)
        {
            if( paused )
                CONS_Printf("Game paused by %s\n",player_names[playernum]);
            else
                CONS_Printf("Game unpaused by %s\n",player_names[playernum]);
        }

        if (paused) {
            if(!menuactive || netgame)
                S_PauseSound ();
        }
        else
            S_ResumeSound ();
    }
}

//  Add a pwad at run-time
//  Search for sounds, maps, musics, etc..
//
void Command_Addfile (void)
{
    if (COM_Argc()!=2)
    {
        CONS_Printf ("addfile <wadfile.wad> : load wad file\n");
        return;
    }

    P_AddWadFile (COM_Argv(1),NULL);
}



// =========================================================================
//                            MISC. COMMANDS
// =========================================================================


void Command_Frags_f (void)
{
    int i,j;

    if(!cv_deathmatch.value)
    {
        CONS_Printf("Frags : show the frag table\n");
        CONS_Printf("Only for deathmatch games\n");
        return;
    }

    for(i=0;i<MAXPLAYERS;i++)
        if(playeringame[i])
        {
            CONS_Printf("%-16s",player_names[i]);
            for(j=0;j<MAXPLAYERS;j++)
                if(playeringame[j])
                    CONS_Printf(" %3d",players[i].frags[j]);
            CONS_Printf("\n");
    }
}

void Command_TeamFrags_f(void)
{
    int i,j;
    fragsort_t unused[MAXPLAYERS];
    int frags[MAXPLAYERS];
    int fragtbl[MAXPLAYERS][MAXPLAYERS];

    if(!cv_deathmatch.value && !cv_teamplay.value)
    {
        CONS_Printf("teamfrags : show the frag table for teams\n");
        CONS_Printf("Only for deathmatch teamplay games\n");
        return;
    }

    HU_CreateTeamFragTbl(unused,frags,fragtbl);

    for(i=0;i<11;i++)
        if(teamingame(i))
        {
            CONS_Printf("%-8s",team_names[i]);
            for(j=0;j<11;j++)
                if(teamingame(j))
                    CONS_Printf(" %3d",fragtbl[i][j]);
            CONS_Printf("\n");
        }
}


//  Returns program version.
//
void Command_Version_f (void)
{
    CONS_Printf ("Doom LEGACY version %i.%i ("
                __TIME__" "__DATE__")\n"VERSIONSTRING,
                VERSION/100,VERSION%100);

}


//  Quit the game immediately
//
void Command_Quit_f (void)
{
    I_Quit();
}

void FragLimit_OnChange(void)
{
    int i;

    if(cv_fraglimit.value>0)
    {
        for(i=0;i<MAXPLAYERS;i++)
            P_CheckFragLimit(&players[i]);
    }
}

void TimeLimit_OnChange(void)
{
	if(!cv_gametype.value) // Don't allow timelimit in Single Player/Co-Op! Tails 09-06-2001
	{
		levelTimer = false;
		return;
	}

    if(cv_timelimit.value)
    {
        levelTimer = true;
        levelTimeCount = cv_timelimit.value * 60 * TICRATE;
        CONS_Printf("Levels will end after %d minute(s).\n",cv_timelimit.value);
    }
    else
        levelTimer = false;
}

void P_RespawnWeapons(void);
void Deahtmatch_OnChange(void)
{
    if(server)
    {
        if( cv_deathmatch.value>=2 )
            CV_SetValue(&cv_itemrespawn,1);
        else
            CV_SetValue(&cv_itemrespawn,0);
    }
    if( cv_deathmatch.value==1 || cv_deathmatch.value==3 )
        P_RespawnWeapons ();

    // give all key to the players
    if (cv_deathmatch.value)
    {
        int j;
        for(j=0;j<MAXPLAYERS;j++)
            if( playeringame[j] )
                players[j].cards = it_allkeys;
    }
}
void GameType_OnChange(void) // Tails 03-13-2001
{
    if(server && (multiplayer || netgame))
    {
        if(cv_gametype.value == 1 || cv_gametype.value == 3 || cv_gametype.value == 4)
            CV_SetValue(&cv_itemrespawn,1);
        else
            CV_SetValue(&cv_itemrespawn,0);

		if(cv_gametype.value == 4 && !(gamemap == 13 || gamemap == 14))
		{
            COM_BufAddText (va("map \"%s\"\n",G_BuildMapName(gameepisode, 13)));
			COM_BufAddText ("timelimit 10\"");
		}
		else if((cv_gametype.value == 1 || cv_gametype.value == 3) && !(gamemap == 6 || gamemap == 7 || gamemap == 8 || gamemap == 9 || gamemap == 10 || gamemap == 11 || gamemap == 12))
		{
            COM_BufAddText (va("map \"%s\"\n",G_BuildMapName(gameepisode, 6)));
			COM_BufAddText ("timelimit 5\"");
		}
    }
	else if (!(multiplayer || netgame) && cv_gametype.value)
	{
            CV_SetValue(&cv_gametype,0);
            CV_SetValue(&cv_itemrespawntime,0);
            CV_SetValue(&cv_itemrespawn,0);
	}
}// Tails 03-13-2001
void Command_ExitLevel_f(void)
{
    if(!server)
    {
        CONS_Printf("Only the server can exit the level\n");
        return;
    }
    if( gamestate != GS_LEVEL || demoplayback )
        CONS_Printf("You should be in a level to exit it !\n");

    SendNetXCmd(XD_EXITLEVEL,NULL,0);
}

void Got_ExitLevelcmd(char **cp,int playernum)
{
    G_ExitLevel();
}


void Command_Load_f(void)
{
    byte slot;

    if(COM_Argc()!=2)
    {
        CONS_Printf("load <slot>: load a saved game\n");
        return;
    }

    if(!server)
    {
        CONS_Printf("Only server can do a load game\n");
        return;
    }

    if (demoplayback)
        G_StopDemo();

    // spawn a server if needed
    SV_SpawnServer();

    slot=atoi(COM_Argv(1));
    SendNetXCmd(XD_LOADGAME,&slot,1);
}

void Got_LoadGamecmd(char **cp,int playernum)
{
    byte slot=*(*cp)++;
    G_DoLoadGame(slot);
}

void Command_Save_f(void)
{
    char p[SAVESTRINGSIZE+1];

    if(COM_Argc()!=3)
    {
        CONS_Printf("save <slot> <desciption>: save game\n");
        return;
    }

    if(!server)
    {
        CONS_Printf("Only server can do a save game\n");
        return;
    }

    p[0]=atoi(COM_Argv(1));
    strcpy(&p[1],COM_Argv(2));
    SendNetXCmd(XD_SAVEGAME,&p,strlen(&p[1])+2);
}

void Got_SaveGamecmd(char **cp,int playernum)
{
    byte slot;
    char description[SAVESTRINGSIZE];

    slot=*(*cp)++;
    strcpy(description,*cp);
    *cp+=strlen(description)+1;

    G_DoSaveGame(slot,description);
}
