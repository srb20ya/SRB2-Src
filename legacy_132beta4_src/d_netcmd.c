// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: d_netcmd.c,v 1.31 2001/12/15 18:41:35 hurdler Exp $
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
// Revision 1.31  2001/12/15 18:41:35  hurdler
// small commit, mainly splitscreen fix
//
// Revision 1.30  2001/11/02 21:39:45  judgecutor
// Added Frag's weapon falling
//
// Revision 1.29  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.28  2001/08/12 15:21:04  bpereira
// see my log
//
// Revision 1.27  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.26  2001/05/21 14:57:04  crashrl
// Readded directory crawling file search function
//
// Revision 1.25  2001/05/16 17:12:52  crashrl
// Added md5-sum support, removed recursiv wad search
//
// Revision 1.24  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.23  2001/03/19 21:18:48  metzgermeister
//   * missing textures in HW mode are replaced by default texture
//   * fixed crash bug with P_SpawnMissile(.) returning NULL
//   * deep water trick and other nasty thing work now in HW mode (tested with tnt/map02 eternal/map02)
//   * added cvar gr_correcttricks
//
// Revision 1.22  2001/02/24 13:35:19  bpereira
// no message
//
// Revision 1.21  2001/02/10 12:27:13  bpereira
// no message
//
// Revision 1.20  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.19  2000/11/26 20:36:14  hurdler
// Adding autorun2
//
// Revision 1.18  2000/11/11 13:59:45  bpereira
// no message
//
// Revision 1.17  2000/11/02 19:49:35  bpereira
// no message
//
// Revision 1.16  2000/10/08 13:30:00  bpereira
// no message
//
// Revision 1.15  2000/09/10 10:39:06  metzgermeister
// *** empty log message ***
//
// Revision 1.14  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.13  2000/08/16 14:10:01  hurdler
// add master server code
//
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
#include "p_local.h" 
#include "p_setup.h"
#include "s_sound.h"
#include "m_misc.h"
#include "am_map.h"
#include "byteptr.h"
#include "d_netfil.h"
#include "p_spec.h"
#include "m_cheat.h"
#include "d_clisrv.h"
#include "v_video.h"
#include "d_main.h"

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

void Got_Teamchange  (char **cp,int playernum); // Tails 04-02-2003
void Got_Clearscores (char **cp, int playernum); // Tails 09-19-2003

void TeamPlay_OnChange(void);
void FragLimit_OnChange(void);
void GameType_OnChange(void); // Tails 03-13-2001
void TimeLimit_OnChange(void);

void ObjectPlace_OnChange(void);

// Tails 08-13-2002
void Playerspeed_OnChange(void);
void Ringslinger_OnChange(void);
void Startrings_OnChange(void);
void Startlives_OnChange(void);
void Startcontinues_OnChange(void);
void Norobots_OnChange(void);
void Gravity_OnChange(void);

void Numlaps_OnChange(void); // Graue 11-17-2003
//void Fishcake_OnChange(void); // Graue 12-13-2003

void Command_Playdemo_f (void);
void Command_Timedemo_f (void);
void Command_Stopdemo_f (void);
void Command_Map_f (void);
void Command_Teleport_f (void); // Tails 10-02-2001
//void Command_Restart_f (void);

void Command_Addfile (void);
void Command_Pause(void);

void Command_Version_f (void);
void Command_Nodes_f (void); // Tails 03-15-2003
void Command_Quit_f (void);
void Command_Playintro_f(void); // Tails 03-16-2003
void Command_Writethings_f(void);

void Command_Displayplayer_f(void);
void Command_Tunes_f(void);

//void Command_Water_f (void);
void Command_ExitLevel_f(void);
void Command_Load_f(void);
void Command_Save_f(void);
void Command_ExitGame_f(void);

void Command_Teamchange_f(void); // Tails 04-02-2003
void Command_ServerTeamChange_f(void);

void Command_Clearscores_f(void); // Tails 09-19-2003

void Command_Circspeeds_f(void); // Graue 12-22-2003
void Command_Oldspeeds_f(void); // Graue 12-22-2003

// Remote Administration Tails 09-22-2003
void Command_Changepassword_f(void);
void Command_Login_f(void);
void Got_Login(char **cp, int playernum);
void Got_Verification(char **cp, int playernum);
void Command_Verify_f(void);

boolean P_TeleportMove(); // Tails 10-02-2001

//Added by Hurdler for master server connection
void AddMServCommands(void);


// =========================================================================
//                           CLIENT VARIABLES
// =========================================================================

void SendWeaponPref(void);
void SendNameAndColor(void);
void SendNameAndColor2(void);

// these two are just meant to be saved to the config
consvar_t cv_playername           = {"name"                ,"sonic"       ,CV_CALL | CV_NOINIT,NULL,SendNameAndColor};
consvar_t cv_playercolor          = {"color"               ,"7"        ,CV_CALL | CV_NOINIT,Color_cons_t,SendNameAndColor};
// player's skin, saved for commodity, when using a favorite skins wad..
consvar_t cv_skin                 = {"skin"                ,DEFAULTSKIN,CV_CALL | CV_NOINIT,NULL /*skin_cons_t*/,SendNameAndColor};
consvar_t cv_autoaim              = {"autoaim"             ,"1"        ,CV_SAVE | CV_CALL | CV_NOINIT,CV_OnOff,SendWeaponPref};
consvar_t cv_autoaim2              = {"autoaim2"             ,"1"        ,CV_SAVE | CV_CALL | CV_NOINIT,CV_OnOff,SendWeaponPref};
// secondary player for splitscreen mode
consvar_t cv_playername2          = {"name2"               ,"tails"    ,CV_CALL | CV_NOINIT,NULL,SendNameAndColor2};
consvar_t cv_playercolor2         = {"color2"              ,"5"        ,CV_CALL | CV_NOINIT,Color_cons_t,SendNameAndColor2};
consvar_t cv_skin2                = {"skin2"               ,"TAILS",CV_CALL | CV_NOINIT,NULL /*skin_cons_t*/,SendNameAndColor2};

// Tails 08-20-2002
consvar_t cv_precipdist = {"precipdist","1024",CV_SAVE,CV_Unsigned, NULL};

CV_PossibleValue_t PreferredTeam_cons_t[]={{1,"Red"},{2,"Blue"},{0,NULL}}; // Tails 07-22-2001

consvar_t cv_preferredteam        = {"preferredteam"       ,"1",CV_CALL,PreferredTeam_cons_t,SendNameAndColor}; // Tails 07-22-2001
consvar_t cv_preferredteam2        = {"preferredteam2"       ,"1",CV_CALL,PreferredTeam_cons_t,SendNameAndColor}; // Tails 07-22-2001

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
#ifdef __DJGPP__
CV_PossibleValue_t usejoystick_cons_t[]={{0,"Off"}
                                        ,{1,"4 Buttons"}
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
#define usejoystick_cons_t  NULL

//#error "cv_usejoystick don't have possible value for this OS !"
#endif
#endif

boolean cv_debug;

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
CV_PossibleValue_t gametype_cons_t[]={{0,"Coop"},{1,"Match"},{2,"Race"},{3,"Tag"},{4,"CTF"},{5,"Chaos"},
#ifdef CIRCUITMODE
{6,"Circuit"},
#endif
{0,NULL}}; // Tails 03-13-2001 and circuit mode added by Graue 12-06-2003

// Tails 08-13-2002
CV_PossibleValue_t ringlimit_cons_t[]={{0,"MIN"},{999999,"MAX"},{0,NULL}};
CV_PossibleValue_t liveslimit_cons_t[]={{0,"MIN"},{99,"MAX"},{0,NULL}};

// Tails 08-12-2002
CV_PossibleValue_t racetype_cons_t[]={{0,"Full"},{1,"Time_Only"},{0,NULL}};
CV_PossibleValue_t raceitemboxes_cons_t[]={{0,"Normal"},{1,"Random"},{2,"Teleports"},{3,"None"},{0,NULL}};
consvar_t cv_racetype      = {"racetype","0",CV_NETVAR|CV_SAVE, racetype_cons_t, NULL};
consvar_t cv_raceitemboxes = {"race_itemboxes","1",CV_NETVAR|CV_SAVE, raceitemboxes_cons_t, NULL};

// Graue wealthyfrog 12-06-2003, 12-08-2003
CV_PossibleValue_t circuit_itemboxes_cons_t[]={{0,"Normal"},{1,"Random"},{2,"None"},{0,NULL}};
consvar_t cv_circuit_itemboxes = {"circuit_itemboxes","0",CV_NETVAR|CV_SAVE, circuit_itemboxes_cons_t, NULL};
consvar_t cv_circuit_ringthrow = {"circuit_ringthrow","1",CV_NETVAR|CV_SAVE, CV_OnOff, NULL};
consvar_t cv_circuit_specmoves = {"circuit_specmoves","0",CV_NETVAR|CV_SAVE, CV_OnOff, NULL};
consvar_t cv_circuit_spin = {"circuit_spin","1",CV_NETVAR|CV_SAVE, CV_OnOff, NULL};

CV_PossibleValue_t matchboxes_cons_t[]={{0,"Normal"},{1,"Random"},{2,"Non-Random"},{3,"None"},{0,NULL}};
consvar_t cv_matchboxes = {"matchboxes","0",CV_NETVAR|CV_SAVE, matchboxes_cons_t, NULL};

consvar_t cv_specialrings = {"specialrings","1",CV_NETVAR, CV_OnOff, NULL};

CV_PossibleValue_t chaoschances_cons_t[]={{0,"Off"},{1,"Low"},{2,"Medium"},{3,"High"},{0,NULL}};

consvar_t cv_chaos_bluecrawla      = {"chaos_bluecrawla"     ,"3",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_chaos_redcrawla       = {"chaos_redcrawla"      ,"3",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_chaos_crawlacommander = {"chaos_crawlacommander","1",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_chaos_jettysynbomber  = {"chaos_jettysynbomber" ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_chaos_jettysyngunner  = {"chaos_jettysyngunner" ,"1",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_chaos_eggmobile1      = {"chaos_eggmobile1"     ,"1",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_chaos_eggmobile2      = {"chaos_eggmobile2"     ,"1",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_chaos_spawnrate       = {"chaos_spawnrate"      ,"30",CV_NETVAR|CV_SAVE, CV_Unsigned, NULL};

consvar_t cv_teleporters    = {"teleporters"   ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_superring      = {"superring"     ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_silverring     = {"silverring"    ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_supersneakers  = {"supersneakers" ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_invincibility  = {"invincibility" ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_blueshield     = {"blueshield"    ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_greenshield    = {"greenshield"   ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_yellowshield   = {"yellowshield"  ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_redshield      = {"redshield"     ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_blackshield    = {"blackshield"   ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_1up            = {"1up"           ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};
consvar_t cv_eggmanbox      = {"eggmantv"     ,"2",CV_NETVAR|CV_SAVE, chaoschances_cons_t, NULL};

// Tails 08-13-2002
consvar_t cv_playerspeed    = {"playerspeed"    ,"1"     ,CV_NETVAR|CV_FLOAT|CV_NOSHOWHELP|CV_CALL, 0                , Playerspeed_OnChange};
consvar_t cv_ringslinger    = {"ringslinger"    ,"No"    ,CV_NETVAR|CV_NOSHOWHELP|CV_CALL                       , CV_YesNo         , Ringslinger_OnChange};
consvar_t cv_startrings     = {"startrings"     ,"0"     ,CV_NETVAR|CV_NOSHOWHELP|CV_CALL         , ringlimit_cons_t, Startrings_OnChange};
consvar_t cv_startlives     = {"startlives"     ,"0"     ,CV_NETVAR|CV_NOSHOWHELP|CV_CALL         , liveslimit_cons_t, Startlives_OnChange};
consvar_t cv_startcontinues = {"startcontinues" ,"0"     ,CV_NETVAR|CV_NOSHOWHELP|CV_CALL         , liveslimit_cons_t, Startcontinues_OnChange};
consvar_t cv_gravity = {"gravity","0.5",CV_NETVAR|CV_FLOAT|CV_CALL,NULL, Gravity_OnChange}; // No longer required in autoexec.cfg! Tails 12-01-99

// Graue 11-17-2003, 11-20-2003, and 12-06-2003
CV_PossibleValue_t numlapslimit_cons_t[]={{1,"MIN"},{30,"MAX"},{0,NULL}};
consvar_t cv_numlaps = {"numlaps","3",CV_NETVAR|CV_CALL|CV_NOINIT,numlapslimit_cons_t,Numlaps_OnChange}; // Circuit mode Graue 11-17-2003
consvar_t cv_countdowntime = {"countdowntime","60",CV_NETVAR,CV_Unsigned,NULL}; // Graue 11-20-2003

consvar_t cv_teamplay   = {"teamplay"  ,"0",CV_NETVAR | CV_CALL,teamplay_cons_t, TeamPlay_OnChange};
consvar_t cv_teamdamage = {"teamdamage","0",CV_NETVAR,CV_OnOff};
consvar_t cv_timetic = {"timetic","0",0,CV_OnOff}; // Tails 04-01-2001
consvar_t cv_objectplace = {"objectplace","0",CV_CALL|CV_NETVAR,CV_OnOff, ObjectPlace_OnChange}; // Tails 06-17-2001

CV_PossibleValue_t snapto_cons_t[]={{0,"Off"},{1,"Floor"},{2,"Ceiling"},{3, "Halfway"}, {4,NULL}};

// Graue 12-13-2003: why did cv_snapto work without a NULL at the end?
consvar_t cv_snapto = {"snapto","0",CV_NETVAR,snapto_cons_t, NULL}; // Tails 06-17-2001
consvar_t cv_speed = {"speed","1",CV_NETVAR, NULL};
consvar_t cv_objflags = {"objflags", "7", CV_NETVAR, NULL};
consvar_t cv_mapthingnum = {"mapthingnum", "0", CV_NETVAR, NULL};
consvar_t cv_grid = {"grid", "0", CV_NETVAR, NULL};

// Scoring type options Graue 12-13-2003
CV_PossibleValue_t match_scoring_cons_t[]={{0,"Normal"},{1,"Classic"}, {2,NULL}};
CV_PossibleValue_t ctf_scoring_cons_t[]={{0,"Normal"},{1,"Classic"}, {2,NULL}};
consvar_t cv_match_scoring = {"match_scoring", "0", CV_NETVAR, match_scoring_cons_t, NULL};
consvar_t cv_ctf_scoring = {"ctf_scoring", "0", CV_NETVAR, ctf_scoring_cons_t, NULL};

consvar_t cv_realnames = {"realnames", "0", CV_NOSHOWHELP, CV_OnOff}; // Tails 05-05-2003

// Change fraglimit to pointlimit and use CV_Unsigned so huge values work Graue 12-13-2003
consvar_t cv_fraglimit  = {"pointlimit" ,"0",CV_NETVAR | CV_CALL | CV_NOINIT,CV_Unsigned, FragLimit_OnChange};
consvar_t cv_timelimit  = {"timelimit" ,"0",CV_NETVAR | CV_CALL | CV_NOINIT,CV_Unsigned, TimeLimit_OnChange};
consvar_t cv_gametype   = {"gametype","0",CV_NETVAR | CV_CALL,gametype_cons_t, GameType_OnChange};
consvar_t cv_autoctf   = {"autoctf","Yes",CV_NETVAR, CV_YesNo, NULL}; // Tails 07-22-2001
consvar_t cv_forceskin   = {"forceskin","0",CV_NETVAR, CV_YesNo, NULL}; // Tails 04-30-2002
consvar_t cv_allowexitlevel = {"allowexitlevel"  ,"0",CV_NETVAR , CV_YesNo, NULL};
consvar_t cv_allowteamchange = {"allowteamchange"  ,"1",CV_NETVAR , CV_YesNo, NULL}; // Tails 04-02-2003

consvar_t cv_killingdead = {"killingdead", "0", CV_NETVAR, CV_OnOff, NULL}; // Tails 09-01-2003

consvar_t cv_netstat = {"netstat","0",0,CV_OnOff};

// Intermission time Tails 04-19-2002
consvar_t cv_inttime = {"inttime","15",CV_NETVAR, CV_Unsigned, NULL};
consvar_t cv_advancemap = {"advancemap","Yes",CV_NETVAR, CV_YesNo, NULL};

consvar_t cv_runscripts = {"runscripts","Yes",CV_NETVAR, CV_YesNo, NULL}; // Graue 12-13-2003
//consvar_t cv_fishcake = {"fishcake","0",CV_NETVAR|CV_CALL|CV_NOSHOWHELP, CV_OnOff, Fishcake_OnChange}; // Graue 12-13-2003
consvar_t cv_lapcounteridiocy = {"mysticshouldbedrugoutintothestreetandshot","1",CV_NETVAR|CV_NOSHOWHELP,CV_OnOff}; // Graue 12-25-2003

boolean usecircspeeds = false; // Graue 12-22-2003

consvar_t cv_friendlyfire = {"friendlyfire","Yes",CV_NETVAR,CV_YesNo,NULL}; // Graue 12-28-2003

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

	// Remote Administration Tails 09-22-2003
	COM_AddCommand("password", Command_Changepassword_f);
	RegisterNetXCmd(XD_LOGIN, Got_Login);
	COM_AddCommand("login", Command_Login_f);
	COM_AddCommand("verify", Command_Verify_f);
	RegisterNetXCmd(XD_VERIFIED, Got_Verification);

	RegisterNetXCmd(XD_TEAMCHANGE,Got_Teamchange); // Tails 04-02-2003
	COM_AddCommand("changeteam", Command_Teamchange_f); // Tails 04-02-2003
	COM_AddCommand("serverchangeteam", Command_ServerTeamChange_f);

	// Tails 09-19-2003
	RegisterNetXCmd(XD_CLEARSCORES, Got_Clearscores);
	COM_AddCommand("clearscores", Command_Clearscores_f);

    COM_AddCommand ("playdemo", Command_Playdemo_f);
    COM_AddCommand ("timedemo", Command_Timedemo_f);
    COM_AddCommand ("stopdemo", Command_Stopdemo_f);
    COM_AddCommand ("map", Command_Map_f);
    COM_AddCommand ("teleport", Command_Teleport_f); // Tails 10-02-2001
//    COM_AddCommand ("restartlevel", Command_Restart_f);
    COM_AddCommand ("exitgame",Command_ExitGame_f);
    COM_AddCommand ("exitlevel",Command_ExitLevel_f);

    COM_AddCommand ("addfile", Command_Addfile);
    COM_AddCommand ("pause", Command_Pause);

//    COM_AddCommand ("turbo", Command_Turbo_f);     // turbo speed
    COM_AddCommand ("version", Command_Version_f);
    COM_AddCommand ("quit", Command_Quit_f);
	COM_AddCommand ("playintro", Command_Playintro_f); // Tails 03-16-2003
	COM_AddCommand ("writethings", Command_Writethings_f);

    COM_AddCommand ("chatmacro", Command_Chatmacro_f); // hu_stuff.c
    COM_AddCommand ("setcontrol", Command_Setcontrol_f);
    COM_AddCommand ("setcontrol2", Command_Setcontrol2_f);

    COM_AddCommand ("saveconfig",Command_SaveConfig_f);
    COM_AddCommand ("loadconfig",Command_LoadConfig_f);
    COM_AddCommand ("changeconfig",Command_ChangeConfig_f);
    COM_AddCommand ("screenshot",M_ScreenShot);

//	COM_AddCommand ("circspeeds",Command_Circspeeds_f); // Graue 12-22-2003
//	COM_AddCommand ("oldspeeds",Command_Oldspeeds_f); // Graue 12-22-2003

	COM_AddCommand ("nodes", Command_Nodes_f);

    //Added by Hurdler for master server connection
    AddMServCommands();

    // p_mobj.c
    CV_RegisterVar (&cv_itemrespawntime);
    CV_RegisterVar (&cv_itemrespawn);
    CV_RegisterVar (&cv_flagtime); // Tails 08-03-2001
	CV_RegisterVar (&cv_suddendeath); // Tails 11-18-2002
    CV_RegisterVar (&cv_splats);

	CV_RegisterVar (&cv_precipdist); // Tails 08-20-2002

  //
  // register main variables
  //
    //register these so it is saved to config
    cv_playername.defaultvalue=I_GetUserName();
    if( cv_playername.defaultvalue == NULL )
        cv_playername.defaultvalue = "Player"; // Tails 03-26-2001
    CV_RegisterVar (&cv_playername);
    CV_RegisterVar (&cv_playercolor);

    // WATER HACK TEST UNTIL FULLY FINISHED
//    COM_AddCommand ("dev_water", Command_Water_f);

    //misc
    CV_RegisterVar (&cv_teamplay);
    CV_RegisterVar (&cv_teamdamage);
    CV_RegisterVar (&cv_fraglimit);
	CV_RegisterVar (&cv_gametype); // Tails 03-13-2001
	CV_RegisterVar (&cv_timetic); // Tails 04-01-2001

	// In-game thing placing stuff Tails
	CV_RegisterVar (&cv_objectplace);
	CV_RegisterVar (&cv_snapto);
	CV_RegisterVar (&cv_speed);
	CV_RegisterVar (&cv_objflags);
	CV_RegisterVar (&cv_mapthingnum);
	CV_RegisterVar (&cv_grid);

	CV_RegisterVar (&cv_realnames);
	
	CV_RegisterVar (&cv_autoctf); // Tails 06-17-2001
	CV_RegisterVar (&cv_inttime); // Tails 04-19-2002
	CV_RegisterVar (&cv_advancemap); // Tails 10-05-2002
    CV_RegisterVar (&cv_timelimit);
    CV_RegisterVar (&cv_playdemospeed);
    CV_RegisterVar (&cv_netstat);
	CV_RegisterVar (&cv_forceskin); // Force skin Tails 04-30-2002

	// Tails 08-12-2002
	CV_RegisterVar (&cv_specialrings);
	CV_RegisterVar (&cv_racetype);
	CV_RegisterVar (&cv_raceitemboxes);
	CV_RegisterVar (&cv_matchboxes);
	CV_RegisterVar (&cv_chaos_bluecrawla);
	CV_RegisterVar (&cv_chaos_redcrawla);
	CV_RegisterVar (&cv_chaos_crawlacommander);
	CV_RegisterVar (&cv_chaos_jettysynbomber);
	CV_RegisterVar (&cv_chaos_jettysyngunner);
	CV_RegisterVar (&cv_chaos_eggmobile1);
	CV_RegisterVar (&cv_chaos_eggmobile2);
	CV_RegisterVar (&cv_chaos_spawnrate);

	CV_RegisterVar (&cv_teleporters);
	CV_RegisterVar (&cv_superring);
	CV_RegisterVar (&cv_silverring);
	CV_RegisterVar (&cv_supersneakers);
	CV_RegisterVar (&cv_invincibility);
	CV_RegisterVar (&cv_blueshield);
	CV_RegisterVar (&cv_greenshield);
	CV_RegisterVar (&cv_yellowshield);
	CV_RegisterVar (&cv_redshield);
	CV_RegisterVar (&cv_blackshield);
	CV_RegisterVar (&cv_1up);
	CV_RegisterVar (&cv_eggmanbox);

	// Tails 08-13-2002
	CV_RegisterVar (&cv_playerspeed);
	CV_RegisterVar (&cv_ringslinger);
	CV_RegisterVar (&cv_startrings);
	CV_RegisterVar (&cv_startlives);
	CV_RegisterVar (&cv_startcontinues);

	CV_RegisterVar (&cv_numlaps); // Graue 11-17-2003
	CV_RegisterVar (&cv_countdowntime); // Graue 11-20-2003
	CV_RegisterVar (&cv_circuit_itemboxes); // Graue 12-06-2003
	CV_RegisterVar (&cv_circuit_ringthrow);
	CV_RegisterVar (&cv_circuit_specmoves); // Graue 12-08-2003
	CV_RegisterVar (&cv_circuit_spin);

	// Graue 12-13-2003
	CV_RegisterVar (&cv_runscripts);
//	CV_RegisterVar (&cv_fishcake);
	CV_RegisterVar (&cv_match_scoring);
	CV_RegisterVar (&cv_ctf_scoring);

	CV_RegisterVar (&cv_lapcounteridiocy); // Graue 12-25-2003
	CV_RegisterVar (&cv_friendlyfire); // Graue 12-28-2003
    
	COM_AddCommand ("displayplayer", Command_Displayplayer_f);

	COM_AddCommand ("tunes",Command_Tunes_f);

    COM_AddCommand ("load",Command_Load_f);
    RegisterNetXCmd(XD_LOADGAME,Got_LoadGamecmd);
    COM_AddCommand ("save",Command_Save_f);
    RegisterNetXCmd(XD_SAVEGAME,Got_SaveGamecmd);
    // r_things.c (skin NAME)
    CV_RegisterVar (&cv_skin);
    // secondary player (splitscreen)
    CV_RegisterVar (&cv_skin2);
    CV_RegisterVar (&cv_playername2);
    CV_RegisterVar (&cv_playercolor2);

	CV_RegisterVar (&cv_preferredteam); // Tails 07-22-2001
	CV_RegisterVar (&cv_preferredteam2); // Tails 07-22-2001

    //FIXME: not to be here.. but needs be done for config loading
    CV_RegisterVar (&cv_usegamma);

    //m_menu.c
    CV_RegisterVar (&cv_crosshair);
    //CV_RegisterVar (&cv_crosshairscale); // doesn't work for now
    CV_RegisterVar (&cv_invertmouse);
    CV_RegisterVar (&cv_alwaysfreelook);
    CV_RegisterVar (&cv_mousemove);
    CV_RegisterVar (&cv_showmessages);

    //see m_menu.c
    //CV_RegisterVar (&cv_showmessages2);
    CV_RegisterVar (&cv_crosshair2);
	CV_RegisterVar (&cv_autoaim);
    CV_RegisterVar (&cv_autoaim2);
    //CV_RegisterVar (&cv_controlperkey2);

    //g_input.c
    CV_RegisterVar (&cv_usemouse2);
    CV_RegisterVar (&cv_invertmouse2);
    CV_RegisterVar (&cv_alwaysfreelook2);
    CV_RegisterVar (&cv_mousemove2);
    CV_RegisterVar (&cv_mousesens2);
    CV_RegisterVar (&cv_mlooksens2);
    CV_RegisterVar (&cv_joystickfreelook);

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

    CV_RegisterVar (&cv_allowexitlevel);
	CV_RegisterVar (&cv_allowautoaim);

	CV_RegisterVar (&cv_allowteamchange); // Tails 04-02-2003

	CV_RegisterVar (&cv_killingdead); // Tails 09-01-2003

	CV_RegisterVar (&cv_analog); // Analog Test Tails 06-10-2001
	CV_RegisterVar (&cv_analog2); // Analog Test Tails 12-16-2001

    //s_sound.c
    CV_RegisterVar (&cv_soundvolume);
    CV_RegisterVar (&cv_musicvolume);
    CV_RegisterVar (&cv_numChannels);

    //i_cdmus.c
    CV_RegisterVar (&cd_volume);
    CV_RegisterVar (&cdUpdate);
#if defined (LINUX) && !defined (SDL)
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

/* ideas of commands names from Quake
    "status"
    "notarget"
    "fly"
    "changelevel"
    "reconnect"
    "tell"
    "kill"
    "spawn"
    "begin"
    "prespawn"
    "ping"

    "startdemos"
    "demos"
    "stopdemo"
*/

}

// =========================================================================
//                            CLIENT STUFF
// =========================================================================

//  name, color, or skin has changed
//
void SendNameAndColor(void)
{
    char     buf[MAXPLAYERNAME+1+SKINNAMESIZE+1],*p;
	int team; // Tails 07-31-2001
	int y; // Tails 07-31-2001
	int z; // Tails 07-31-2001
	int i; // Tails 07-31-2001
	byte extrainfo;

	if(!delayoverride)
	{
		if(playerchangedelay && gamestate == GS_LEVEL && !menuactive && leveltime > TICRATE)
		{
			CONS_Printf("You can't change your player info for another %d seconds.\n", playerchangedelay/TICRATE);
			return;
		}

		if(gamestate == GS_LEVEL && !menuactive)
			playerchangedelay = 0;
	}
	else
		delayoverride = false;

    p=buf;

	extrainfo = cv_playercolor.value;

	if(cv_gametype.value == GT_CTF)
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
						if(!playeringame[i])
							continue;

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
				extrainfo += team << 5; // CTF Tails 07-31-2001
			}
			else
				extrainfo += players[consoleplayer].ctfteam << 5;
		}

	WRITEBYTE(p, extrainfo);

    WRITESTRINGN(p,cv_playername.string,MAXPLAYERNAME);
    *(p-1) = 0; // finish teh string;

	// Don't change skin if the server doesn't want you to.
	if(!(server || admin) && cv_forceskin.value)
	{
		SendNetXCmd(XD_NAMEANDCOLOR,buf,p-buf);
		return;
	}

    // check if player has the skin loaded (cv_skin may have
    //  the name of a skin that was available in the previous game)
    cv_skin.value=R_SkinAvailable(cv_skin.string);
    if (!cv_skin.value)
        WRITESTRINGN(p,DEFAULTSKIN,SKINNAMESIZE)
    else
        WRITESTRINGN(p,cv_skin.string,SKINNAMESIZE);
    *(p-1) = 0; // finish the string;

    SendNetXCmd(XD_NAMEANDCOLOR,buf,p-buf);

	// Force normal player colors in Single Player and CTF modes Tails 06-15-2001
	if(!(multiplayer || netgame))
	{
		// Use the prefcolor Tails 12-15-2003
		if(cv_playercolor.value != players[consoleplayer].prefcolor)
		{
			delayoverride = true;
			CV_SetValue(&cv_playercolor, players[consoleplayer].prefcolor);
		}
	/*	if(cv_skin.value == 0 && cv_playercolor.value != 7)
		{
			delayoverride = true;
			CV_SetValue(&cv_playercolor, 7);
		}
		else if(cv_skin.value == 1 && cv_playercolor.value != 5)
		{
			delayoverride = true;
			CV_SetValue(&cv_playercolor, 5);
		}
		else if(cv_skin.value == 2 && cv_playercolor.value != 6)
		{
			delayoverride = true;
			CV_SetValue(&cv_playercolor, 6);
		}*/
	}
	else if(cv_gametype.value == GT_CTF)
	{
		if(players[consoleplayer].ctfteam == 1 && cv_playercolor.value != 6)
		{
			delayoverride = true;
			CV_SetValue(&cv_playercolor, 6);
		}
		else if(players[consoleplayer].ctfteam == 2 && cv_playercolor.value != 7)
		{
			delayoverride = true;
			CV_SetValue(&cv_playercolor, 7);
		}
	}
}

// splitscreen
void SendNameAndColor2(void)
{
    char     buf[MAXPLAYERNAME+1+SKINNAMESIZE+1],*p;
	int team; // Tails 07-31-2001
	int y; // Tails 07-31-2001
	int z; // Tails 07-31-2001
	int i; // Tails 07-31-2001
	byte extrainfo;

    p=buf;

	extrainfo = cv_playercolor2.value;

	if(cv_gametype.value == GT_CTF)
		{
			if(players[secondarydisplayplayer].ctfteam != 1 && players[secondarydisplayplayer].ctfteam != 2)
			{
				team = cv_preferredteam2.value;

				if(team < 1)
					team = 1;

				if(cv_autoctf.value)
				{
					y = z = 0;
					for (i=0; i<MAXPLAYERS; i++)
					{
						if(!playeringame[i])
							continue;

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
				extrainfo += team << 5; // CTF Tails 07-31-2001
			}
			else
				extrainfo += players[consoleplayer].ctfteam << 5;
		}

	WRITEBYTE(p, extrainfo);

    WRITESTRINGN(p,cv_playername2.string,MAXPLAYERNAME);
    *(p-1) = 0; // finish teh string;

	// Don't change skin if the server doesn't want you to.
	if(!(server || admin) && cv_forceskin.value)
	{
		SendNetXCmd2(XD_NAMEANDCOLOR,buf,p-buf);
		return;
	}

    // check if player has the skin loaded (cv_skin may have
    //  the name of a skin that was available in the previous game)
    cv_skin2.value=R_SkinAvailable(cv_skin2.string);
    if (!cv_skin2.value)
        WRITESTRINGN(p,DEFAULTSKIN,SKINNAMESIZE)
    else
        WRITESTRINGN(p,cv_skin2.string,SKINNAMESIZE);
    *(p-1) = 0; // finish the string;

    SendNetXCmd2(XD_NAMEANDCOLOR,buf,p-buf);

	if(cv_gametype.value == GT_CTF)
	{
		if(players[secondarydisplayplayer].ctfteam == 1 && cv_playercolor2.value != 6)
		{
			delayoverride = true;
			CV_SetValue(&cv_playercolor2, 6);
		}
		else if(players[secondarydisplayplayer].ctfteam == 2 && cv_playercolor2.value != 7)
		{
			delayoverride = true;
			CV_SetValue(&cv_playercolor2, 7);
		}
	}
}


void Got_NameAndcolor(char **cp,int playernum)
{
    player_t *p=&players[playernum];
	int i;
	byte extrainfo;

	if(!(cv_debug || devparm || modifiedgame) && !(multiplayer || netgame))
	{
		READBYTE(*cp);
		READSTRING(*cp,player_names[playernum]);
		SKIPSTRING(*cp);
		return;
	}

	extrainfo = READBYTE(*cp);

    // color
    p->skincolor=(extrainfo&31) % MAXSKINCOLORS;

    // a copy of color
    if(p->mo)
        p->mo->flags =  (p->mo->flags & ~MF_TRANSLATION)
                     | ((p->skincolor)<<MF_TRANSSHIFT);

//	// CTF Stuff Tails 07-31-2001
	p->ctfteam=extrainfo >> 5; // CTF Tails 07-31-2001

    // name
    if(netgame && stricmp(player_names[playernum], *cp) ) // Tails
        CONS_Printf("%s renamed to %s\n",player_names[playernum], *cp);
    READSTRING(*cp,player_names[playernum])

    // skin
	if(cv_forceskin.value) // Server wants everyone to use the same player Tails 04-30-2002
	{
		if(playernum == 0)
			SetPlayerSkin(playernum, *cp); // This should always be 0.

		for(i=1; i<MAXPLAYERS; i++) // Start at the location AFTER the server (0).
		{
			if(playeringame[i])
			{
				SetPlayerSkinByNum(i, players[0].skin); // Player 0 is always in the game.
			}
		}
		SKIPSTRING(*cp);
	}
	else
	{
		SetPlayerSkin(playernum,*cp);
		SKIPSTRING(*cp);
	}
}

void SendWeaponPref(void)
{
    char buf[1];

    buf[0]=cv_autoaim.value;
    SendNetXCmd(XD_WEAPONPREF,buf,1);

    if(cv_splitscreen.value)
	{
		buf[0]=cv_autoaim2.value;
        SendNetXCmd2(XD_WEAPONPREF,buf,1);
	}
}

void Got_WeaponPref(char **cp,int playernum)
{
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

	if(!(cv_debug || devparm))
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
/*
	if(!(cv_debug || devparm) && !(multiplayer || netgame))
	{
		CONS_Printf("Warping not allowed in single player.\n");
		return;
	}
*/
    if (COM_Argc()<2 || COM_Argc()>7)
    {
        CONS_Printf ("map <mapname[.wad]> [-skill <1..5>] [-monsters <0/1>] [-noresetplayers]: warp to map\n");
        return;
    }

    if(!(server || admin))
    {
        CONS_Printf ("Only the server can change the map\n");
        return;
    }

    strncpy(MAPNAME,COM_Argv(1),MAX_WADPATH);
/*
    if (FIL_CheckExtension(MAPNAME))
    {
        // here check if file exist !!!
        if( !findfile(MAPNAME,NULL,false) )
        {
            CONS_Printf("\2File %s' not found\n",MAPNAME);
            return;
        }
    }
    else*/
    {
        // internal wad lump
        if(W_CheckNumForName(MAPNAME)==-1)
        {
            CONS_Printf("\2Internal game map '%s' not found\n",MAPNAME);
            return;
        }
    }

	if(COM_CheckParm("-fromcode") == 0)
	{
		if(!netgame)
		{
			if(!modifiedgame)
			{
				if(cv_splitscreen.value && cv_gametype.value != GT_COOP)
					;
				else
				{
					CONS_Printf("Sorry, map change disabled in single player.\n");
					return;
				}
			}
		}
	}

    if((i=COM_CheckParm("-skill"))!=0)
        buf[0]=atoi(COM_Argv(i+1))-1;
    else
        buf[0]=gameskill;
/*
    if((i=COM_CheckParm("-monsters"))!=0)
        buf[1]=(atoi(COM_Argv(i+1))==0);
    else
        buf[1]=(nomonsters!=0);
*/
    // use only one bit
    if(buf[1])
        buf[1]=1;

    if(COM_CheckParm("-noresetplayers"))
        buf[1]|=2;

    // spaw the server if needed
    // reset players if there is a new one
    if( !admin && SV_SpawnServer() )
        buf[1]&=~2;

    SendNetXCmd(XD_MAP,buf,2+strlen(MAPNAME)+1);
}

void Got_Mapcmd(char **cp,int playernum)
{
    char mapname[MAX_WADPATH];
    int skill,resetplayer=1;

    skill=READBYTE(*cp);
//    nomonsters=READBYTE(*cp);

    resetplayer=((READBYTE(*cp) & 2)==0);
//    nomonsters&=1;

    strcpy(mapname,*cp);
    *cp+=strlen(mapname)+1;

    CONS_Printf ("Warping to map...\n");
    if(demoplayback && !timingdemo)
        precache=false;
    G_InitNew (skill, mapname,resetplayer);
    if(demoplayback && !timingdemo)
        precache=true;
    CON_ToggleOff ();
    if( timingdemo )
        G_DoneLevelLoad();
}
/*
void Command_Restart_f (void)
{
    if( netgame )
    {
        CONS_Printf("Restartlevel doesn't work in network\n");
        return;
    }

    if( gamestate == GS_LEVEL )
        G_DoLoadLevel (true);
    else
        CONS_Printf("You should be in a level to restart it !\n"); // Tails 03-16-2002
}
*/
extern consvar_t cv_pause;
void Command_Pause(void)
{
    char buf;
    if( COM_Argc()>1 )
        buf = atoi(COM_Argv(1))!=0;
    else
        buf = !paused;

	if(cv_pause.value != 1 && server)
		SendNetXCmd(XD_PAUSE,&buf,1);
	else
		CONS_Printf("Only the server can pause the game.\n");
}

void Got_Pause(char **cp,int playernum)
{
    paused = READBYTE(*cp);

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

// Tails 09-19-2003
void Command_Clearscores_f(void)
{
	if(!(server || admin))
		return;

	SendNetXCmd(XD_CLEARSCORES, NULL, 1);
}

// Tails 09-19-2003
void Got_Clearscores(char **cp, int playernum)
{
	int i;

	for(i=0; i<MAXPLAYERS; i++)
		players[i].score = 0;

	CONS_Printf("Scores have been reset by the server.\n");
}

void Command_Teamchange_f(void)
{
    char buf;
    if( COM_Argc()>1 )
        buf = atoi(COM_Argv(1));

	if(!(buf == 1 || buf == 2))
	{
		CONS_Printf("Specify a team you want! 1 = Red, 2 = Blue\n");
		return;
	}

	if(buf == players[consoleplayer].ctfteam)
	{
		CONS_Printf("You're already on that team! %d\n %d\n", players[consoleplayer].ctfteam, buf);
		return;
	}

	if(!cv_allowteamchange.value)
	{
		CONS_Printf("Server does not allow team change.\n");
		return;
	}
    
    SendNetXCmd(XD_TEAMCHANGE,&buf,1);
}

void Command_ServerTeamChange_f(void)
{
    char buf;
	char playernum;

	if(!(server || admin))
	{
		CONS_Printf("You're not the server. You can't change players' teams.\n");
		return;
	}

    if (COM_Argc()<2)
    {
        CONS_Printf ("serverteamchange <playernum> <team> : change a player's team\n");
        return;
    }

	playernum = atoi(COM_Argv(1));
    
	buf = atoi(COM_Argv(2));

	if(!(buf == 1 || buf == 2))
	{
		CONS_Printf("Specify a team you want! 1 = Red, 2 = Blue\n");
		return;
	}

	if(buf == players[playernum].ctfteam)
	{
		CONS_Printf("The player is already on that team! %d\n %d\n", players[playernum].ctfteam, buf);
		return;
	}

	buf |= 4; // This signals that it's a server change

	buf += playernum << 3;
    
    SendNetXCmd(XD_TEAMCHANGE,&buf,1);
}

void Got_Teamchange(char **cp,int playernum)
{
	int newteam;
    newteam = READBYTE(*cp);

	if(newteam & 4) // Special marker that the server sent the request
	{
		playernum = newteam >> 3;
	}

	newteam &= 3; // We do this either way, since... who cares?

	if(players[playernum].mo) // Safety first!
		P_DamageMobj(players[playernum].mo, NULL, NULL, 10000);

	players[playernum].ctfteam = newteam;

	if(players[playernum].ctfteam == 1)
		CONS_Printf("%s switched to the red team\n", player_names[playernum]);
	else if(players[playernum].ctfteam == 2)
		CONS_Printf("%s switched to the blue team\n", player_names[playernum]);
}

// Remote Administration Tails 09-22-2003
void Command_Changepassword_f(void)
{
	if(!(server || admin))
	{
		CONS_Printf("You're not the server. You can't change this.\n");
		return;
	}

    if (COM_Argc() != 2)
    {
		CONS_Printf("password <password> : change password\n");
        return;
    }

	strncpy (adminpassword, COM_Argv(1), 8);

	// Pad the password
	if(strlen(COM_Argv(1)) < 8)
	{
		int i;
		for(i=strlen(COM_Argv(1)); i<8; i++)
			adminpassword[i] = 'a';
	}
}
void Command_Login_f(void)
{
    char    password[8];
/*
	if(server)
	{
		CONS_Printf("You're the server, silly.\n");
		return;
	}*/

    if (COM_Argc() != 2)
    {
		CONS_Printf("Login <password> : Administrator login\n");
        return;
    }

	strncpy (password, COM_Argv(1), 8);

	// Pad the password
	if(strlen(COM_Argv(1)) < 8)
	{
		int i;
		for(i=strlen(COM_Argv(1)); i<8; i++)
			password[i] = 'a';
	}

	CONS_Printf("Sending Login...\n(Notice only given if password is correct.)\n");

	SendNetXCmd(XD_LOGIN,password,8);
}

void Got_Login(char **cp, int playernum)
{
	char compareword[9];

	if(!server)
		return;

	READSTRING(*cp,compareword);

	compareword[8] = '\0';

	if(!strcmp(compareword, adminpassword))
	{
		verifyplayer = playernum+100;
		CONS_Printf("%s passed authentication.\n", player_names[playernum]);
	}
}

void Command_Verify_f(void)
{
	char buf[8]; // Should be plenty
	char *temp;
	int playernum;

	if(!server)
		return;

    if (COM_Argc() != 2)
    {
		CONS_Printf("verify <node> : give admin privileges to a node\n");
        return;
    }

	strncpy (buf, COM_Argv(1), 7);

	playernum = atoi(buf);

	temp = buf;

	WRITEBYTE(temp, playernum);

	SendNetXCmd(XD_VERIFIED, buf, 1);
}

void Got_Verification(char **cp, int playernum)
{
//	if(server)
//		return; // You don't need this anyway
	int num = READBYTE(*cp);

	if(num == consoleplayer)
		admin = true;
	else
	{
		CONS_Printf("Playernum %d didn't match.\n",num);
		return;
	}


	CONS_Printf("Password correct. You are now an administrator.\n");
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

	modifiedgame = true;
    P_AddWadFile (COM_Argv(1),NULL);
}



// =========================================================================
//                            MISC. COMMANDS
// =========================================================================


//  Returns program version.
//
void Command_Version_f (void)
{
    CONS_Printf ("SRB2 v1.08 ("
                __TIME__" "__DATE__")\n");

}

// Lists all of the players and their node #s
// (Useful for server)
// Tails 03-15-2003
void Command_Nodes_f (void)
{
	int i;

	for(i=0; i<MAXPLAYERS; i++)
	{
		if(playeringame[i])
			CONS_Printf("%d : %s\n", i, player_names[i]);
	}
}

void F_StartIntro(void);
void F_StartCredits(void);

// Tails 03-16-2003
void Command_Playintro_f (void)
{
	F_StartIntro();
}

void P_WriteThings(int lump);

void Command_Writethings_f(void)
{
	P_WriteThings(W_GetNumForName(G_BuildMapName(gamemap))+ML_THINGS);
}


//  Quit the game immediately
//
void Command_Quit_f (void)
{
    I_Quit();
}

//#define JOHNNYFUNCODE
void ObjectPlace_OnChange(void)
{
#ifndef JOHNNYFUNCODE
	if((netgame || multiplayer) && cv_objectplace.value != 0) // You spoon!
	{
		cv_objectplace.value = 0;
		CV_SetValue(&cv_objectplace, 0);
		CONS_Printf("No, dummy, you can't use this in multiplayer!\n");
		return;
	}
#endif

#ifdef JOHNNYFUNCODE
	if(cv_objectplace.value)
	{
		int i;

		for(i=0; i<MAXPLAYERS; i++)
		{
			if(!playeringame[i])
				continue;

			if(!players[i].mo)
				continue;

			if(players[i].nightsmode == true)
				continue;

			players[i].mo->flags2 |= MF2_DONTDRAW;
			players[i].mo->flags |= MF_NOCLIP;
			players[i].mo->flags |= MF_NOGRAVITY;
			P_UnsetThingPosition(players[i].mo);
			players[i].mo->flags |= MF_NOBLOCKMAP;
			P_SetThingPosition(players[i].mo);
			if(players[i].currentthing == 0)
				players[i].currentthing = 1;
			modifiedgame = true;
		}
	}
	else if(players[0].mo)
	{
		int i;

		for(i=0; i<MAXPLAYERS; i++)
		{
			if(!playeringame[i])
				continue;

			if(!players[i].mo)
				continue;

			if(players[i].nightsmode == false)
			{
				if(players[i].mo->target)
					P_SetMobjState(players[i].mo->target, S_DISS);

				players[i].mo->flags2 &= ~MF2_DONTDRAW;
				players[i].mo->flags &= ~MF_NOGRAVITY;
			}

			players[i].mo->flags &= ~MF_NOCLIP;
			P_UnsetThingPosition(players[i].mo);
			players[i].mo->flags &= ~MF_NOBLOCKMAP;
			P_SetThingPosition(players[i].mo);
		}
	}
	return;
#endif

	if(cv_objectplace.value)
	{
		if(players[0].nightsmode == true)
			return;

		players[0].mo->flags2 |= MF2_DONTDRAW;
		players[0].mo->flags |= MF_NOCLIP;
		players[0].mo->flags |= MF_NOGRAVITY;
		P_UnsetThingPosition(players[0].mo);
		players[0].mo->flags |= MF_NOBLOCKMAP;
		P_SetThingPosition(players[0].mo);
		if(players[0].currentthing == 0)
			players[0].currentthing = 1;
	}
	else if(players[0].mo)
	{
		if(players[0].nightsmode == false)
		{
			if(players[0].mo->target)
				P_SetMobjState(players[0].mo->target, S_DISS);

			players[0].mo->flags2 &= ~MF2_DONTDRAW;
			players[0].mo->flags &= ~MF_NOGRAVITY;
		}

		players[0].mo->flags &= ~MF_NOCLIP;
		P_UnsetThingPosition(players[0].mo);
		players[0].mo->flags &= ~MF_NOBLOCKMAP;
		P_SetThingPosition(players[0].mo);
	}
}

// Change fraglimit to pointlimit Graue 12-13-2003
void FragLimit_OnChange(void)
{
    int i;

	if(!(server || admin))
		return;

	// Don't allow pointlimit in Single Player/Co-Op/Race/Circuit! Graue 12-13-2003
	if(cv_gametype.value == GT_COOP || cv_gametype.value == GT_RACE || cv_gametype.value == GT_CIRCUIT)
	{
		CV_SetValue(&cv_fraglimit, 0);
		return;
	}

    if(cv_fraglimit.value) // Has to be greater than 0, since it's a CV_Unsigned
    {
		/* if(cv_fraglimit.value % 25) // Scores are always multiples of 25
		{                                 (...not in tag or CTF with classic options)
			CV_SetValue(&cv_fraglimit, cv_fraglimit.value + 25 - (cv_fraglimit.value % 25));
			return;
		} */

		CONS_Printf("Levels will end after someone scores %d point%s.\n",cv_fraglimit.value,cv_fraglimit.value > 1 ? "s" : ""); // Graue 12-23-2003
        for(i=0;i<MAXPLAYERS;i++)
            P_CheckFragLimit(&players[i]);

		if(cv_timelimit.value) // Don't use a pointlimit and a timelimit at the same time!
			CV_SetValue(&cv_timelimit, 0);
    }
	else
		CONS_Printf("Point limit disabled\n");
}

ULONG timelimitintics = 0;

void TimeLimit_OnChange(void)
{
	if(!(server || admin))
		return;
 
	// Don't allow timelimit in Single Player/Co-Op/Race/Circuit! Tails 09-06-2001
	if(cv_gametype.value == GT_COOP || cv_gametype.value == GT_RACE || cv_gametype.value == GT_CIRCUIT)
	{
		CV_SetValue(&cv_timelimit, 0);
		return;
	}

    if(cv_timelimit.value)
    {
        CONS_Printf("Levels will end after %d minute%s.\n",cv_timelimit.value,cv_timelimit.value == 1 ? "" : "s"); // Graue 11-17-2003
        timelimitintics = cv_timelimit.value * 60 * TICRATE;

		if(cv_fraglimit.value) // Don't use a pointlimit and a timelimit at the same time! Graue 12-13-2003
			CV_SetValue(&cv_fraglimit, 0);
    }
    else
        CONS_Printf("Time limit disabled\n");
}

// Graue 11-17-2003
void Numlaps_OnChange(void)
{
	if(!(server || admin))
		return;

	if(cv_gametype.value != GT_CIRCUIT)
	{
		CV_SetValue(&cv_numlaps, 3);
		return;
	}

	if(!cv_numlaps.value)
		CV_SetValue(&cv_numlaps, 1);

    CONS_Printf("Races will end after %d lap%s\n",cv_numlaps.value,cv_numlaps.value == 1 ? "" : "s");
}

//void P_RespawnWeapons(void);

void GameType_OnChange(void) // Tails 03-13-2001
{
	extern boolean circintrodone; // Graue 12-24-2003

    if((server || admin) && (multiplayer || netgame))
    {
        if(cv_gametype.value == GT_MATCH || cv_gametype.value == GT_TAG
			|| cv_gametype.value == GT_CTF || cv_gametype.value == GT_CHAOS
			|| cv_gametype.value == GT_CIRCUIT)
            CV_SetValue(&cv_itemrespawn,1);
        else
            CV_SetValue(&cv_itemrespawn,0);

		if(cv_gametype.value == GT_CHAOS)
		{
			CV_SetValue(&cv_timelimit, 2);
			CV_SetValue(&cv_itemrespawntime, 90);
		}
		else
			CV_Set(&cv_itemrespawntime, cv_itemrespawntime.defaultvalue);
    }
	else if (!(multiplayer || netgame) && cv_gametype.value != GT_COOP)
	{
            CV_SetValue(&cv_gametype,0);
            CV_Set(&cv_itemrespawntime, cv_itemrespawntime.defaultvalue);
            CV_SetValue(&cv_itemrespawn,0);
	}

#ifdef CIRCUITMODE
	// Graue 12-22-2003: FIXTHIS: crappy way of setting circuit speeds
	if(cv_gametype.value == GT_CIRCUIT)
		Command_Circspeeds_f();
	else
		Command_Oldspeeds_f();
#endif

	// Either way, forget about doing a circintro Graue 12-24-2003
	circintrodone = true;

}// Tails 03-13-2001

void Playerspeed_OnChange(void)
{
	// If you've got a grade less than 2, you can't use this.
	if(grade < 2 && cv_playerspeed.value != 1)
		CV_SetValue(&cv_playerspeed, 1);
}

void Ringslinger_OnChange(void)
{
	// If you've got a grade less than 3, you can't use this.
	if(grade < 3 && cv_ringslinger.value != 0)
		CV_Set(&cv_ringslinger, "No");
}

void Startrings_OnChange(void)
{
	// If you've got a grade less than 5, you can't use this.
	if(grade < 5 && cv_startrings.value != 0)
		CV_SetValue(&cv_startrings, 0);

	if(cv_startrings.value != 0)
	{
		int i;
		for(i=0;i<MAXPLAYERS;i++)
		{
			if(playeringame[i] && players[i].mo)
			{
				players[i].mo->health = cv_startrings.value + 1;
				players[i].health = players[i].mo->health;
			}
		}

	}
}

void Startlives_OnChange(void)
{
	// If you've got a grade less than 4, you can't use this.
	if(grade < 4 && cv_startlives.value != 0)
		CV_SetValue(&cv_startlives, 0);

	if(cv_startlives.value != 0)
	{
		int i;
		for(i=0;i<MAXPLAYERS;i++)
		{
			if(playeringame[i])
				players[i].lives = cv_startlives.value;
		}

	}
}

void Startcontinues_OnChange(void)
{
	// If you've got a grade less than 4, you can't use this.
	if(grade < 4 && cv_startcontinues.value != 0)
	{
		CV_SetValue(&cv_startcontinues, 0);
		return;
	}

	if(cv_startcontinues.value != 0)
	{
		int i;
		for(i=0;i<MAXPLAYERS;i++)
		{
			if(playeringame[i])
				players[i].continues = cv_startcontinues.value;
		}

	}
}

// Tails 08-20-2002
void Gravity_OnChange(void)
{
	gravity = cv_gravity.value;
	if(!(grade & 2) || netgame)
		return;
	else if (cv_gravity.value != FRACUNIT/2)
		CV_Set(&cv_gravity, "0.5");
}

void Command_ExitLevel_f(void)
{
	if(!netgame && !cv_debug)
	{
		CONS_Printf("You can't use this in single player!\n");
		return;
	}
    if(!(server || admin))
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

void Command_Displayplayer_f(void)
{
	CONS_Printf("Displayplayer is %d\n", displayplayer);
}

void Command_Tunes_f(void)
{
    if(COM_Argc()!=2)
    {
        CONS_Printf("tunes <slot>: play a slot\n");
        return;
    }

    S_ChangeMusic(atoi(COM_Argv(1)), true);
}

void Command_Load_f(void)
{
    byte slot;

    if(COM_Argc()!=2)
    {
        CONS_Printf("load <slot>: load a saved game\n");
        return;
    }

    if(!(server || admin))
    {
        CONS_Printf("Only server can do a load game\n");
        return;
    }

    if (demoplayback)
        G_StopDemo();

    // spawn a server if needed
	if(!admin)
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

    if(!(server || admin))
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

void Command_ExitGame_f(void)
{
    D_QuitNetGame();
    CL_Reset();
    D_StartTitle();
}

/*// Fishcake is back, but only if you've cleared Very Hard mode Graue 12-13-2003
void Fishcake_OnChange(void)
{
	if(veryhardcleared)
	{
		cv_debug = cv_fishcake.value;
		modifiedgame = true;
	}

	else
		CV_SetValue(&cv_fishcake, cv_debug);
}*/

void Command_Circspeeds_f(void)
{
	int i;

	for(i=0;i<MAXPLAYERS;i++)
	{
		if(playeringame[i])
		{
			players[i].normalspeed = players[i].circnormalspeed;
			players[i].thrustfactor = players[i].circthrustfactor;
			players[i].accelstart = players[i].circaccelstart;
			players[i].acceleration = players[i].circacceleration;
		}
	}
	usecircspeeds = true;
}

void Command_Oldspeeds_f(void)
{
	int i;

	for(i=0;i<MAXPLAYERS;i++)
	{
		if(playeringame[i])
		{
			players[i].normalspeed = players[i].oldnormalspeed;
			players[i].thrustfactor = players[i].oldthrustfactor;
			players[i].accelstart = players[i].oldaccelstart;
			players[i].acceleration = players[i].oldacceleration;
		}
	}
	usecircspeeds = false;
}