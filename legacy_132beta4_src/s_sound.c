// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: s_sound.c,v 1.27 2001/08/20 20:40:39 metzgermeister Exp $
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
// $Log: s_sound.c,v $
// Revision 1.27  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.26  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.25  2001/04/30 17:19:24  stroggonmeth
// HW fix and misc. changes
//
// Revision 1.24  2001/04/18 19:32:26  hurdler
// no message
//
// Revision 1.23  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.22  2001/04/04 20:24:21  judgecutor
// Added support for the 3D Sound
//
// Revision 1.21  2001/04/02 18:54:32  bpereira
// no message
//
// Revision 1.20  2001/04/01 17:35:07  bpereira
// no message
//
// Revision 1.19  2001/03/03 11:11:49  hurdler
// I hate warnigs ;)
//
// Revision 1.18  2001/02/24 13:35:21  bpereira
// no message
//
// Revision 1.17  2001/01/27 11:02:36  bpereira
// no message
//
// Revision 1.16  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.15  2000/11/21 21:13:18  stroggonmeth
// Optimised 3D floors and fixed crashing bug in high resolutions.
//
// Revision 1.14  2000/11/12 21:59:53  hurdler
// Please verify that sound bug
//
// Revision 1.13  2000/11/03 11:48:40  hurdler
// Fix compiling problem under win32 with 3D-Floors and FragglScript (to verify!)
//
// Revision 1.12  2000/11/02 17:50:10  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.11  2000/10/27 20:38:20  judgecutor
// - Added the SurroundSound support
//
// Revision 1.10  2000/09/28 20:57:18  bpereira
// no message
//
// Revision 1.9  2000/05/07 08:27:57  metzgermeister
// no message
//
// Revision 1.8  2000/04/22 16:16:50  emanne
// Correction de l'interface.
// Une erreur s'y était glissé, d'où un segfault si on compilait sans SDL.
//
// Revision 1.7  2000/04/21 08:23:47  emanne
// To have SDL working.
// Makefile: made the hiding by "@" optional. See the CC variable at
// the begining. Sorry, but I like to see what's going on while building
//
// qmus2mid.h: force include of qmus2mid_sdl.h when needed.
// s_sound.c: ??!
// s_sound.h: with it.
// (sorry for s_sound.* : I had problems with cvs...)
//
// Revision 1.6  2000/03/29 19:39:48  bpereira
// no message
//
// Revision 1.5  2000/03/22 18:51:08  metzgermeister
// introduced I_PauseCD() for Linux
//
// Revision 1.4  2000/03/12 23:21:10  linuxcub
// Added consvars which hold the filenames and arguments which will be used
// when running the soundserver and musicserver (under Linux). I hope I
// didn't break anything ... Erling Jacobsen, linuxcub@email.dk
//
// Revision 1.3  2000/03/06 15:13:08  hurdler
// maybe a bug detected
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:  
//
//
//-----------------------------------------------------------------------------


#ifdef MUSSERV
#include <sys/msg.h>
struct musmsg {
  long msg_type;
  char msg_text[12];
};
extern int msg_id;
#endif

#include "doomdef.h"
#include "doomstat.h"
#include "command.h"
#include "g_game.h"
#include "m_argv.h"
#include "r_main.h"     //R_PointToAngle2() used to calc stereo sep.
#include "r_things.h"     // for skins
#include "p_info.h"

#include "i_sound.h"
#include "s_sound.h"
#include "w_wad.h"
#include "z_zone.h"
#include "d_main.h"


// 3D Sound Interface
#include "hardware/hw3sound.h"

// commands for music and sound servers
#ifdef MUSSERV
consvar_t musserver_cmd = {"musserver_cmd","musserver",CV_SAVE};
consvar_t musserver_arg = {"musserver_arg","-t 20 -f -u 0",CV_SAVE};
#endif
#ifdef SNDSERV
consvar_t sndserver_cmd = {"sndserver_cmd","llsndserv",CV_SAVE};
consvar_t sndserver_arg = {"sndserver_arg","-quiet",CV_SAVE};
#endif

#if defined (__WIN32__) && !defined (SURROUND)
#define SURROUND
#endif

#ifdef __MACOS__
consvar_t  play_mode = {"play_mode","0",CV_SAVE,CV_Unsigned};
#endif

#ifdef __WIN32__ // Special FMOD support Tails 11-21-2002
boolean I_StartFMODSong(char* musicname, int looping);
#endif

extern int skyflatnum;
fixed_t P_AproxDistance(fixed_t dx, fixed_t dy);

// stereo reverse 1=true, 0=false
consvar_t stereoreverse = {"stereoreverse","0",CV_SAVE ,CV_OnOff};

// if true, all sounds are loaded at game startup
consvar_t precachesound = {"precachesound","0",CV_SAVE ,CV_OnOff};

CV_PossibleValue_t soundvolume_cons_t[]={{0,"MIN"},{31,"MAX"},{0,NULL}};
// actual general (maximum) sound & music volume, saved into the config
consvar_t cv_soundvolume = {"soundvolume","15",CV_SAVE,soundvolume_cons_t};
consvar_t cv_musicvolume = {"musicvolume","31",CV_SAVE,soundvolume_cons_t};

// number of channels available
void SetChannelsNum(void);
consvar_t cv_numChannels = {"snd_channels","32",CV_SAVE | CV_CALL, CV_Unsigned,SetChannelsNum};

#ifdef SURROUND
consvar_t surround = {"surround", "0", CV_SAVE, CV_OnOff};
#endif

#define S_MAX_VOLUME            127

// when to clip out sounds
// Does not fit the large outdoor areas.
// added 2-2-98 in 8 bit volume control (befort  (1200*0x10000))
#define S_CLIPPING_DIST         (1200*0x10000)

// Distance tp origin when sounds should be maxed out.
// This should relate to movement clipping resolution
// (see BLOCKMAP handling).
// Originally: (200*0x10000).
// added 2-2-98 in 8 bit volume control (befort  (160*0x10000))
#define S_CLOSE_DIST            (160*0x10000)

// added 2-2-98 in 8 bit volume control (befort  remove the +4)
#define S_ATTENUATOR            ((S_CLIPPING_DIST-S_CLOSE_DIST)>>(FRACBITS+4))

// Adjustable by menu.
#define NORM_VOLUME             snd_MaxVolume

#define NORM_PITCH              128
#define NORM_PRIORITY           64
#define NORM_SEP                128

#define S_PITCH_PERTURB         1
#define S_STEREO_SWING          (96*0x10000)

#ifdef SURROUND
#define SURROUND_SEP            -128
#endif

// percent attenuation from front to back
#define S_IFRACVOL              30

typedef struct
{
    // sound information (if null, channel avail.)
    sfxinfo_t*  sfxinfo;

    // origin of sound
    void*       origin;

    // handle of the sound being played
    int         handle;

} channel_t;


// the set of channels available
static channel_t*       channels;


// whether songs are mus_paused
static boolean          mus_paused;

// music currently being played
static musicinfo_t*     mus_playing=0;

static int              nextcleanup;


//
// Internals.
//
int
S_getChannel
( void*         origin,
  sfxinfo_t*    sfxinfo );


int
S_AdjustSoundParams
( mobj_t*       listener,
  mobj_t*       source,
  int*          vol,
  int*          sep,
  int*          pitch,
  sfxinfo_t*    sfxinfo); // Handy! Tails 10-08-2002

static void S_StopChannel(int cnum);


void S_RegisterSoundStuff (void)
{
    if(dedicated)
	return;
    
	//added:11-04-98: stereoreverse
    CV_RegisterVar (&stereoreverse);
    CV_RegisterVar (&precachesound);

#ifdef SNDSERV
    CV_RegisterVar (&sndserver_cmd);
    CV_RegisterVar (&sndserver_arg);
#endif
#ifdef MUSSERV
    CV_RegisterVar (&musserver_cmd);
    CV_RegisterVar (&musserver_arg);
#endif
#ifdef SURROUND
    CV_RegisterVar (&surround);
#endif

#ifdef __MACOS__        //mp3 playlist stuff
    {
        int i;
        for (i=0;i<PLAYLIST_LENGTH;i++)
        {
            user_songs[i].name = malloc(7);
            sprintf(user_songs[i].name, "song%i%i",i/10,i%10);
            user_songs[i].defaultvalue = malloc(1);
            *user_songs[i].defaultvalue = 0;
            user_songs[i].flags = CV_SAVE;
            user_songs[i].PossibleValue = NULL;
            CV_RegisterVar (&user_songs[i]);
        }
        CV_RegisterVar (&play_mode);
    }
#endif
}

void SetChannelsNum(void)
{
    int i;

    // Allocating the internal channels for mixing
    // (the maximum number of sounds rendered
    // simultaneously) within zone memory.
    if(channels)
      Z_Free(channels);

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_SetSourcesNum();
        return;
    }
#endif
    channels =
      (channel_t *) Z_Malloc(cv_numChannels.value*sizeof(channel_t), PU_STATIC, 0);

    // Free all channels for use
    for (i=0 ; i<cv_numChannels.value ; i++)
      channels[i].sfxinfo = 0;

    
}


void S_InitRuntimeMusic()
{
  int i;

  for(i = mus_firstfreeslot; i < mus_lastfreeslot; i++)
    S_music[i].name = NULL;
}


//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init ( int           sfxVolume,
              int           musicVolume )
{
    int           i;

    if(dedicated)
	return;
    
    //CONS_Printf( "S_Init: default sfx volume %d\n", sfxVolume);

    S_SetSfxVolume (sfxVolume);
    S_SetMusicVolume (musicVolume);

    SetChannelsNum();

    // no sounds are playing, and they are not mus_paused
    mus_paused = 0;

    // Note that sounds have not been cached (yet).
    for (i=1 ; i<NUMSFX ; i++)
        S_sfx[i].lumpnum = S_sfx[i].usefulness = -1;      // for I_GetSfx()

    //
    //  precache sounds if requested by cmdline, or precachesound var true
    //
    if(!nosound && (M_CheckParm("-precachesound") || precachesound.value) )
    {
        // Initialize external data (all sounds) at start, keep static.
        CONS_Printf("Loading sounds... ");

        for (i=1 ; i<NUMSFX ; i++)
        {
            // NOTE: linked sounds use the link's data at StartSound time
            if (S_sfx[i].name && !S_sfx[i].link)
                S_sfx[i].data = I_GetSfx (&S_sfx[i]);
                    
        }

        CONS_Printf(" pre-cached all sound data\n");
    }

    S_InitRuntimeMusic();
}

//  Retrieve the lump number of sfx
//
int S_GetSfxLumpNum (sfxinfo_t* sfx)
{
    char namebuf[9];
    int  sfxlump;

    sprintf(namebuf, "ds%s", sfx->name);
    
    sfxlump=W_CheckNumForName(namebuf);
    if (sfxlump>0)
        return sfxlump;

    strncpy(namebuf, sfx->name, 9);

    sfxlump=W_CheckNumForName(namebuf);
    if (sfxlump>0)
        return sfxlump;

    return W_GetNumForName ("dsthok");
}


//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//

//SoM: Stop all sounds, load level info, THEN start sounds.
void S_StopSounds()
{
  int cnum;

#ifdef HW3SOUND
  if (hws_mode != HWS_DEFAULT_MODE)
  {
      HW3S_StopSounds();
      return;
  }
#endif

  // kill all playing sounds at start of level
  //  (trust me - a good idea)
  for (cnum=0 ; cnum<cv_numChannels.value ; cnum++)
    if (channels[cnum].sfxinfo)
      S_StopChannel(cnum);
}


void S_Start(void)
{
  int mnum;

  // start new music for the level
  mus_paused = 0;

  mnum = mapheaderinfo[gamemap-1].musicslot;

  S_ChangeMusic(mnum, true);

  nextcleanup = 15;
}

void S_StartSoundAtVolume( void*         origin_p,
                           int           sfx_id,
                           int           volume )
{

    int           sep;
    int           pitch;
    int           priority;
    sfxinfo_t*    sfx;
    int           cnum;

    mobj_t*       origin = (mobj_t *) origin_p;

    if(nosound || (origin && origin->type == MT_SPIRIT))
        return;

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_StartSound(origin, sfx_id);
        return;
    };
#endif

    // Debug.
    /*fprintf( stderr,
             "S_StartSoundAtVolume: playing sound %d (%s)\n",
             sfx_id, S_sfx[sfx_id].name );*/

#ifdef PARANOIA
    // check for bogus sound #
    if (sfx_id < 1 || sfx_id > NUMSFX)
        I_Error("Bad sfx #: %d\n", sfx_id);
#endif

	// check for bogus sound #
	if(sfx_id == 0 || sfx_id == sfx_None)
		return;

    sfx = &S_sfx[sfx_id];

    if (sfx->skinsound!=-1 && origin && origin->skin)
    {
        // it redirect player sound to the sound in the skin table
        sfx_id = ((skin_t *)origin->skin)->soundsid[sfx->skinsound];
        sfx    = &S_sfx[sfx_id];
    }

    // Initialize sound parameters
    if (sfx->link)
    {
      pitch = sfx->pitch;
      priority = sfx->priority;
      volume += sfx->volume;

      if (volume < 1)
        return;

    // added 2-2-98 SfxVolume is now the hardware volume, don't mix up
    //    if (volume > SfxVolume)
    //      volume = SfxVolume;
    }
    else
    {
      pitch = NORM_PITCH;
      priority = NORM_PRIORITY;
    }


    // Check to see if it is audible,
    //  and if not, modify the params

    //added:16-01-98:changed consoleplayer to displayplayer
    if (origin && origin != players[displayplayer].mo && !(cv_splitscreen.value && origin == players[secondarydisplayplayer].mo))
    {
        int           rc,rc2;
        int volume2=volume,sep2/*=sep*/,pitch2=pitch;
        rc=S_AdjustSoundParams(players[displayplayer].mo,
                               origin,
                               &volume,
                               &sep,
                               &pitch,
							   sfx);
        if(cv_splitscreen.value)
        {
             rc2=S_AdjustSoundParams(players[secondarydisplayplayer].mo,
                                     origin,
                                     &volume2,
                                     &sep2,
                                     &pitch2,
									 sfx);
             if(!rc2)
             {
                 if( !rc )
                     return;
             }
             else
             if(!rc || (rc && volume2>volume))
             {
                 volume=volume2;
                 sep=sep2;
                 pitch=pitch2;
                 if ( origin->x == players[secondarydisplayplayer].mo->x &&
                      origin->y == players[secondarydisplayplayer].mo->y )
                 {
                     sep       = NORM_SEP;
                 }
             }
        }
        else
            if(!rc) return;

        if ( origin->x == players[displayplayer].mo->x &&
             origin->y == players[displayplayer].mo->y )
      {
        sep       = NORM_SEP;
      }
    }
    else
    {
      sep = NORM_SEP;
    }

    // hacks to vary the sfx pitches

    //added:16-02-98: removed by Fab, because it used M_Random() and it
    //                was a big bug, and then it doesnt change anything
    //                dont hear any diff. maybe I'll put it back later
    //                but of course not using M_Random().

    // kill old sound
	// No way! Tails 08-20-2002
//    S_StopSound(origin);

    // try to find a channel
    cnum = S_getChannel(origin, sfx);

    if (cnum<0)
      return;

    //
    // This is supposed to handle the loading/caching.
    // For some odd reason, the caching is done nearly
    //  each time the sound is needed?
    //

    // cache data if necessary
    // NOTE : set sfx->data NULL sfx->lump -1 to force a reload
    if (sfx->link)
        sfx->data = sfx->link->data;

    if (!sfx->data)
    {
        //CONS_Printf ("cached sound %s\n", sfx->name);
        if (!sfx->link)
            sfx->data = I_GetSfx (sfx);
        else
        {
            sfx->data = I_GetSfx (sfx->link);
            sfx->link->data = sfx->data;
        }
    }

    // increase the usefulness
    if (sfx->usefulness++ < 0)
        sfx->usefulness = -1;

    
#ifdef SURROUND
    // judgecutor:
    // Avoid channel reverse if surround
    if (stereoreverse.value && sep != SURROUND_SEP)
        sep = (~sep) & 255;
#else
    //added:11-04-98:
    if (stereoreverse.value)
        sep = (~sep) & 255;
#endif

    //CONS_Printf("stereo %d reverse %d\n", sep, stereoreverse.value);

    // Assigns the handle to one of the channels in the
    //  mix/output buffer.
    channels[cnum].handle = I_StartSound(sfx_id,
                                         /*sfx->data,*/
                                         volume,
                                         sep,
                                         pitch,
                                         priority);
}

void S_StartSound( void*         origin,
                   int           sfx_id )
{

	if(mariomode) // Sounds change in Mario mode!
	{
		switch(sfx_id)
		{
			case sfx_altow1:
			case sfx_altow2:
			case sfx_altow3:
			case sfx_altow4:
				sfx_id = sfx_mario8;
				break;
			case sfx_thok:
				sfx_id = sfx_mario7;
				break;
			case sfx_pop:
				sfx_id = sfx_mario5;
				break;
			case sfx_jump:
				sfx_id = sfx_mario6;
				break;
			case sfx_shield:
				sfx_id = sfx_mario3;
				break;
			case sfx_itemup:
				sfx_id = sfx_mario4;
				break;
			case sfx_tink:
				sfx_id = sfx_mario1;
				break;
			case sfx_cgot:
				sfx_id = sfx_mario9;
				break;
			case sfx_lose:
				sfx_id = sfx_mario2;
				break;
			default:
				break;
		}

	}

    // the volume is handled 8 bits
#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
        HW3S_StartSound(origin, sfx_id);
    else
#endif
        S_StartSoundAtVolume(origin, sfx_id, 255);
}


void S_StopSound(void *origin)
{
    int cnum;

    // SoM: Sounds without origion can have multiple sources, they shouldn't
    // be stoped by new sounds.
    if(!origin)
      return;

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_StopSound(origin);
        return;
    }
    else
    {
#endif
       for (cnum=0 ; cnum<cv_numChannels.value ; cnum++)
       {
            if (channels[cnum].sfxinfo && channels[cnum].origin == origin)
            {
                S_StopChannel(cnum);
                break;
            }
        }
#ifdef HW3SOUND
    }
#endif
}


//
// Stop and resume music, during game PAUSE.
//
void S_PauseSound(void)
{
#ifdef __WIN32__
	if(!nofmod)
		I_PauseSong(0);

#endif
	if (mus_playing && !mus_paused)
    {
        I_PauseSong(mus_playing->handle);
        mus_paused = true;
    }

    // pause cd music
#ifdef LINUX
    I_PauseCD ();
#else
    I_StopCD ();
#endif
}

void S_ResumeSound(void)
{
#ifdef __WIN32__
	if(!nofmod)
		I_ResumeSong(0);
    else
#endif
    if (mus_playing && mus_paused)
    {
        I_ResumeSong(mus_playing->handle);
        mus_paused = false;
    }

    // resume cd music
    I_ResumeCD ();
}


//
// Updates music & sounds
//
static int      actualsfxvolume;        //check for change through console
static int      actualmusicvolume;

void S_UpdateSounds(void)
{
    int         audible;
    int         cnum;
    int         volume;
    int         sep;
    int         pitch;
    sfxinfo_t*  sfx;
    channel_t*  c;

    mobj_t*     listener = players[displayplayer].mo;

    if(dedicated)
	return;
    
    // Update sound/music volumes, if changed manually at console
    if (actualsfxvolume != cv_soundvolume.value)
        S_SetSfxVolume (cv_soundvolume.value);
    if (actualmusicvolume != cv_musicvolume.value)
        S_SetMusicVolume (cv_musicvolume.value);

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_UpdateSources();
        return;
    }
#endif

    /* Clean up unused data.
    if (gametic > nextcleanup)
    {
        for (i=1 ; i<NUMSFX ; i++)
        {
            if (S_sfx[i].usefulness==0)
            {
                //S_sfx[i].usefulness--;

                // don't forget to unlock it !!!
                // __dmpi_unlock_....
                //Z_ChangeTag(S_sfx[i].data, PU_CACHE);
                //S_sfx[i].data = 0;

                CONS_Printf ("\2flushed sfx %.6s\n", S_sfx[i].name);
            }
        }
        nextcleanup = gametic + 15;
    }*/

    for (cnum=0 ; cnum<cv_numChannels.value ; cnum++)
    {
        c = &channels[cnum];
        sfx = c->sfxinfo;

        if (c->sfxinfo)
        {
            if (I_SoundIsPlaying(c->handle))
            {
                // initialize parameters
                volume = 255;            //8 bits internal volume precision
                pitch = NORM_PITCH;
                sep = NORM_SEP;

                if (sfx->link) // strange (BP)
                {
                    pitch = sfx->pitch;
                    volume += sfx->volume;
                    if (volume < 1)
                    {
                        S_StopChannel(cnum);
                        continue;
                    }
                }

                // check non-local sounds for distance clipping
                //  or modify their params
                if (c->origin && listener != c->origin && !(cv_splitscreen.value && c->origin==players[secondarydisplayplayer].mo))
                {
                    int audible2;
                    int volume2=volume,sep2=sep,pitch2=pitch;
                    audible = S_AdjustSoundParams(listener,
                                                  c->origin,
                                                  &volume,
                                                  &sep,
                                                  &pitch,
												  c->sfxinfo);

                    if(cv_splitscreen.value)
                    {
                         audible2=S_AdjustSoundParams(players[secondarydisplayplayer].mo,
                                                      c->origin,
                                                      &volume2,
                                                      &sep2,
                                                      &pitch2,
													  c->sfxinfo);
                         if(audible2 && (!audible || (audible && volume2>volume)))
                         {
                             audible=true;
                             volume=volume2;
                             sep=sep2;
                             pitch=pitch2;
                         }
                    }
            
                    if (!audible)
                    {
                        S_StopChannel(cnum);
                    }
                    else
					{
                        I_UpdateSoundParams(c->handle, volume, sep, pitch);
					}
                }
            }
            else
            {
                // if channel is allocated but sound has stopped,
                //  free it
				S_StopChannel(cnum);
            }
        }
    }
    // kill music if it is a single-play && finished
    // if (     mus_playing
    //      && !I_QrySongPlaying(mus_playing->handle)
    //      && !mus_paused )
    // S_StopMusic();
}


void S_SetMusicVolume(int volume)
{
    if (volume < 0 || volume > 31)
        CONS_Printf("musicvolume should be between 0-31\n");

    CV_SetValue (&cv_musicvolume, volume&31);
    actualmusicvolume = cv_musicvolume.value;   //check for change of var

#ifndef __WIN32__
	I_SetMusicVolume(31);       //faB: this is a trick for buggy dos drivers.. I think.
                                //     win32 midistream playbakc doesn't need this
#endif

#ifdef __WIN32__
	if(!nofmod)
		I_SetFMODVolume(volume);
	else
		I_SetMusicVolume(volume&31);
#endif

    I_SetMusicVolume(volume&31);
}



void S_SetSfxVolume(int volume)
{
    if (volume < 0 || volume > 31)
        CONS_Printf("sfxvolume should be between 0-31\n");

    CV_SetValue (&cv_soundvolume, volume&31);
    actualsfxvolume = cv_soundvolume.value;       //check for change of var

#ifdef HW3SOUND
    hws_mode == HWS_DEFAULT_MODE 
        ? I_SetSfxVolume (volume&31)
        : HW3S_SetSfxVolume(volume & 31);
#else
    // now hardware volume
    I_SetSfxVolume(volume&31);
#endif

}

//
// Starts some music with the music id found in sounds.h.
//
void S_StartMusic(int m_id)
{
    S_ChangeMusic(m_id, false);
}

//
// S_ChangeMusicName
// Changes music by name
void S_ChangeMusicName(char *name, int looping)
{
  int  music;

  if(!strncmp(name, "-", 6))
  {
    S_StopMusic();
    return;
  }

  music = S_FindMusic(name);

  if(music > mus_None && music < NUMMUSIC)
    S_ChangeMusic(music, looping);
  else
    {
      CONS_Printf("music not found: %s\n", name);
      S_StopMusic(); // stop music anyway
    }
}



void S_ChangeMusic( int                   music_num,
                    int                   looping )
{
    musicinfo_t*  music;
	int i;

    if(dedicated)
	return;

#ifdef __WIN32__ // Special FMOD support Tails 11-21-2002
	if(!nofmod)
	{
		if ( (music_num <= mus_None) ||
			 (music_num >= NUMMUSIC) )
		{
			CONS_Printf ("ERROR: Bad music number %d\n", music_num);
			return;
		}
		else
			music = &S_music[music_num];


		if (mus_playing == music)
			return;

		if(!nomusic) // Make sure the MIDI is not playing
			S_StopMusic();

		if(!I_StartFMODSong(music->name, looping))
			goto resorttomidi;

		mus_playing = music;
		return;
	}
#endif
resorttomidi:

    if( nomusic )
        return;

    if ( (music_num <= mus_None) ||
         (music_num >= NUMMUSIC) )
    {
        CONS_Printf ("ERROR: Bad music number %d\n", music_num);
        return;
    }
    else
        music = &S_music[music_num];


    if (mus_playing == music)
        return;

    // shutdown old music
    S_StopMusic();

    // get lumpnum if neccessary
    if (!music->lumpnum)
    {
		i = W_CheckNumForName ( va("d_%s", music->name));
		if(i == -1)
		{
			CONS_Printf("Music file d_%s not found!\n", music->name);
			return;
		}
        music->lumpnum = W_GetNumForName( va("d_%s", music->name));
    }
    // load & register it
    music->data = (void *) W_CacheLumpNum(music->lumpnum, PU_MUSIC);
#ifdef __MACOS__
    music->handle = I_RegisterSong(music_num);
#else
    music->handle = I_RegisterSong(music->data,W_LumpLength(music->lumpnum));
#endif

#ifdef MUSSERV

    if (msg_id != -1) {
      struct musmsg msg_buffer;

      msg_buffer.msg_type=6;
      memset(msg_buffer.msg_text,0,sizeof(msg_buffer.msg_text));
      sprintf(msg_buffer.msg_text,"d_%s", music->name);
      msgsnd(msg_id,(struct msgbuf *)&msg_buffer,sizeof(msg_buffer.msg_text),IPC_NOWAIT);
    }

#endif /* #ifdef MUSSERV */

    // play it
    I_PlaySong(music->handle, looping);

    mus_playing = music;
}

#ifdef __WIN32__
void I_StopFMODSong(void);
#endif

void S_StopMusic(void)
{
    if (mus_playing)
    {
        if (mus_paused)
            I_ResumeSong(mus_playing->handle);

#ifdef __WIN32__
		if(!nofmod)
			I_StopFMODSong();
#endif
        I_StopSong(mus_playing->handle);
        I_UnRegisterSong(mus_playing->handle);

#ifdef __WIN32__
		if(nofmod)
			Z_ChangeTag(mus_playing->data, PU_CACHE);
#else
		Z_ChangeTag(mus_playing->data, PU_CACHE);
#endif

        mus_playing->data = 0;
        mus_playing = 0;
    }
}




static void S_StopChannel(int cnum)
{

    int         i;
    channel_t*  c = &channels[cnum];

    if (c->sfxinfo)
    {
        // stop the sound playing
        if (I_SoundIsPlaying(c->handle))
        {
            I_StopSound(c->handle);
        }

        // check to see
        //  if other channels are playing the sound
        for (i=0 ; i<cv_numChannels.value ; i++)
        {
            if (cnum != i
                && c->sfxinfo == channels[i].sfxinfo)
            {
                break;
            }
        }

        // degrade usefulness of sound data
        c->sfxinfo->usefulness--;

        c->sfxinfo = 0;
    }
}


extern fixed_t P_AproxDistance(fixed_t dx, fixed_t dy);
extern int skyflatnum;
//
// Changes volume, stereo-separation, and pitch variables
//  from the norm of a sound effect to be played.
// If the sound is not audible, returns a 0.
// Otherwise, modifies parameters and returns 1.
//
int S_AdjustSoundParams ( mobj_t*       listener,
                          mobj_t*       source,
                          int*          vol,
                          int*          sep,
                          int*          pitch,
						  sfxinfo_t*    sfxinfo) // Handy! Tails 10-08-2002
{
    fixed_t     approx_dist;
    fixed_t     adx;
    fixed_t     ady;
    angle_t     angle;

	if(sfxinfo->pitch == 43) // Rain special case
	{
		fixed_t x,y,yl,yh,xl,xh,closex,closey,newdist;

		if(listener->subsector->sector->ceilingpic == skyflatnum)
			approx_dist = 0;
		else
		{
			// Essentially check in a 1024 unit radius of the player for an outdoor area.
			yl = listener->y - 1024*FRACUNIT;
			yh = listener->y + 1024*FRACUNIT;
			xl = listener->x - 1024*FRACUNIT;
			xh = listener->x + 1024*FRACUNIT;
			closex = listener->x + 2048*FRACUNIT;
			closey = listener->y + 2048*FRACUNIT;
			approx_dist = 1024*FRACUNIT;
			for (y=yl ; y<=yh ; y += FRACUNIT*64)
				for (x=xl ; x<=xh ; x += FRACUNIT*64)
				{
					if(R_PointInSubsector(x,y)->sector->ceilingpic == skyflatnum) // Found the outdoors!
					{
						adx = abs(listener->x - x);
						ady = abs(listener->y - y);
						newdist = adx + ady - ((adx < ady ? adx : ady)>>1);
						if(newdist < approx_dist)
						{
							closex = x;
							closey = y;
							approx_dist = newdist;
						}
					}
				}
		}
	}
	else
	{
		// calculate the distance to sound origin
		//  and clip it if necessary
		adx = abs(listener->x - source->x);
		ady = abs(listener->y - source->y);

		// From _GG1_ p.428. Approx. eucledian distance fast.
		// Take Z into account Tails 08-25-2002
		adx = adx + ady - ((adx < ady ? adx : ady)>>1);
		ady = abs(listener->z - source->z);
		approx_dist = adx + ady - ((adx < ady ? adx : ady)>>1);
	}

	// Tails 01-06-2002
	if(sfxinfo->pitch == 67) // Taunts, deaths, etc, should all be heard louder.
		approx_dist /= 4;

	if (approx_dist > S_CLIPPING_DIST)
    {
        return 0;
    }

    // angle of source to listener
    angle = R_PointToAngle2(listener->x,
                            listener->y,
                            source->x,
                            source->y);

    if (angle > listener->angle)
        angle = angle - listener->angle;
    else
        angle = angle + (0xffffffff - listener->angle);

#ifdef SURROUND
    
    // Produce a surround sound for angle from 105 till 255
    if (surround.value == 1 && (angle > (ANG90 + (ANG45/3)) && angle < (ANG270 - (ANG45/3))))
        *sep = SURROUND_SEP;
    else
    {
#endif

    angle >>= ANGLETOFINESHIFT;

    // stereo separation
    *sep = 128 - (FixedMul(S_STEREO_SWING,finesine[angle])>>FRACBITS);

#ifdef SURROUND
    }
#endif

    // volume calculation
    if (approx_dist < S_CLOSE_DIST)
    {
        // added 2-2-98 SfxVolume is now hardware volume
        *vol = 255; //snd_SfxVolume;
    }
    // removed hack here for gamemap==8 (it made far sound still present)
    else
    {
        // distance effect
        *vol = (15
                * ((S_CLIPPING_DIST - approx_dist)>>FRACBITS))
            / S_ATTENUATOR;
    }

    return (*vol > 0);
}




//
// S_getChannel :
//   If none available, return -1.  Otherwise channel #.
//
int S_getChannel( void*         origin,
                  sfxinfo_t*    sfxinfo )
{
    // channel number to use
    int         cnum;

    channel_t*  c;

    // Find an open channel
    for (cnum=0 ; cnum<cv_numChannels.value ; cnum++)
    {
        if (!channels[cnum].sfxinfo)
            break;

		// Now checks if same sound is being played, rather
		// than just one sound per mobj Tails 08-20-2002
		else if(sfxinfo->pitch == 42)
			break;
		else if(sfxinfo == channels[cnum].sfxinfo
			&& sfxinfo->singularity == true)
		{
			S_StopChannel(cnum);
			break;
		}
		else if(origin && channels[cnum].origin == origin
			&& channels[cnum].sfxinfo == sfxinfo)
        {
            S_StopChannel(cnum);
            break;
        }
        else if (origin && channels[cnum].origin == origin
			&& ((channels[cnum].sfxinfo == sfxinfo)
			|| (channels[cnum].sfxinfo->name != sfxinfo->name
			&& channels[cnum].sfxinfo->pitch != -1
			&& sfxinfo->pitch != -1
			&& channels[cnum].sfxinfo->pitch == sfxinfo->pitch)))
        {
            S_StopChannel(cnum);
            break;
        }
    }

    // None available
    if (cnum == cv_numChannels.value)
    {
        // Look for lower priority
        for (cnum=0 ; cnum<cv_numChannels.value ; cnum++)
            if (channels[cnum].sfxinfo->priority >= sfxinfo->priority) break;

        if (cnum == cv_numChannels.value)
        {
            // FUCK!  No lower priority.  Sorry, Charlie.
//			CONS_Printf("Exceeded set number of available sound channels.\nIncrease the value by changing 'SND_CHANNELS'\n");
            return -1;
        }
        else
        {
            // Otherwise, kick out lower priority.
            S_StopChannel(cnum);
        }
    }

    c = &channels[cnum];

    // channel is decided to be cnum.
    c->sfxinfo = sfxinfo;
    c->origin = origin;

    return cnum;
}


// SoM: Searches through the channels and checks for origin or id.
// returns 0 of not found, returns 1 if found.
// if id == -1, the don't check it...
int S_SoundPlaying(void *origin, int id)
{
    int         cnum;

#ifdef HW3SOUND
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        return HW3S_SoundPlaying(origin, id);
    }
#endif

    for (cnum=0 ; cnum<cv_numChannels.value ; cnum++)
    {
        if (origin &&  channels[cnum].origin ==  origin)
          return 1;
        if (id != -1 && channels[cnum].sfxinfo - S_sfx == id)
          return 1;
    }
    return 0;
}


//
// S_StartSoundName
// Starts a sound using the given name.
#define MAXNEWSOUNDS 10
int     newsounds[MAXNEWSOUNDS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void S_StartSoundName(void *mo, char *soundname)
{
  int  i;
  int  soundnum = 0;
  //Search existing sounds...
  for(i = sfx_None + 1; i < NUMSFX; i++)
  {
    if(!S_sfx[i].name)
      continue;
    if(!stricmp(S_sfx[i].name, soundname))
    {
      soundnum = i;
      break;
    }
  }

  if(!soundnum)
  {
    for(i = 0; i < MAXNEWSOUNDS; i++)
    {
      if(newsounds[i] == 0)
        break;
      if(!S_SoundPlaying(NULL, newsounds[i]))
        {S_RemoveSoundFx(newsounds[i]); break;}
    }

    if(i == MAXNEWSOUNDS)
    {
      CONS_Printf("Cannot load another extra sound!\n");
      return;
    }

    soundnum = S_AddSoundFx(soundname, false);
    newsounds[i] = soundnum;
  }

  S_StartSound(mo, soundnum);
}
