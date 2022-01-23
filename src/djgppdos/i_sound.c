// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: I_sound.c,v 1.5 2003/07/13 13:18:59 hurdler Exp $
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
// $Log: I_sound.c,v $
// Revision 1.5  2003/07/13 13:18:59  hurdler
// go RC1
//
// Revision 1.4  2001/03/30 17:12:52  bpereira
// no message
//
// Revision 1.3  2000/03/06 15:32:56  hurdler
// compiler warning removed
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//-----------------------------------------------------------------------------
/// \file
/// \brief interface level code for sound

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <math.h>

#include "../doomdef.h"
#include "../doomstat.h"
#include "../i_system.h"
#include "../i_sound.h"
#include "../z_zone.h"
#include "../m_argv.h"
#include "../m_misc.h"
#include "../w_wad.h"
#include "../s_sound.h"
#include "../console.h"

//### let's try with Allegro ###
#define  alleg_mouse_unused
#define  alleg_timer_unused
#define  ALLEGRO_NO_KEY_DEFINES
#define  alleg_keyboard_unused
#define  alleg_joystick_unused
#define  alleg_gfx_driver_unused
#define  alleg_palette_unused
#define  alleg_graphics_unused
#define  alleg_vidmem_unused
#define  alleg_flic_unused
//#define  alleg_sound_unused    we use it
#define  alleg_file_unused
#define  alleg_datafile_unused
#define  alleg_math_unused
#define  alleg_gui_unused
#include <allegro.h>
//### end of Allegro include ###

//allegro has 256 virtual voices
// warning should by a power of 2
#define VIRTUAL_VOICES 256
#define VOICESSHIFT 8

// Needed for calling the actual sound output.
#define SAMPLECOUNT    512



//
// this function converts raw 11khz, 8-bit data to a SAMPLE* that allegro uses
// it is need cuz allegro only loads samples from wavs and vocs
//added:11-01-98: now reads the frequency from the rawdata header.
//   dsdata points a 4 unsigned short header:
//    +0 : value 3 what does it mean?
//    +2 : sample rate, either 11025 or 22050.
//    +4 : number of samples, each sample is a single byte since it's 8bit
//    +6 : value 0
static inline SAMPLE *raw2SAMPLE(unsigned char *dsdata, int len)
{
	SAMPLE *spl;

	spl=Z_Malloc(sizeof(SAMPLE),PU_STATIC,NULL);
	if(spl==NULL)
		I_Error("Raw2Sample : no more free mem");
	spl->bits = 8;
	spl->stereo = 0;
	spl->freq = *((unsigned short*)dsdata+1);   //mostly 11025, but some at 22050.
	spl->len = len-8;
	spl->priority = 255;                //priority;
	spl->loop_start = 0;
	spl->loop_end = len-8;
	spl->param = -1;
	spl->data=(void *)(dsdata+8);       //skip the 8bytes header

	return spl;
}


//  This function loads the sound data from the WAD lump,
//  for single sound.
//
void* I_GetSfx (sfxinfo_t*  sfx)
{
	byte*               dssfx;
	int                 size;

	if (sfx->lumpnum<0)
		sfx->lumpnum = S_GetSfxLumpNum (sfx);

	size = W_LumpLength (sfx->lumpnum);

	dssfx = (byte*) W_CacheLumpNum (sfx->lumpnum, PU_STATIC);
	//_go32_dpmi_lock_data(dssfx,size);

	// convert raw data and header from Doom sfx to a SAMPLE for Allegro
	return (void *)raw2SAMPLE (dssfx, size);
}


void I_FreeSfx (sfxinfo_t* sfx)
{
	byte*    dssfx;

	if (sfx->lumpnum<0)
		return;

	// free sample data
	if( sfx->data )
	{
		dssfx = (byte*) ((SAMPLE *)sfx->data)->data - 8;
		Z_Free (dssfx);
		// Allegro SAMPLE structure
		Z_Free (sfx->data);
	}

	sfx->data = NULL;
	sfx->lumpnum = -1;
}

static inline int Volset(int vol)
{
	return (vol*255/31);
}


void I_SetSfxVolume(int volume)
{
	if(nosound)
		return;

	set_volume (Volset(volume),-1);
}

void I_SetMIDIMusicVolume(int volume)
{
	if(nomusic)
		return;

	// Now set volume on output device.
	set_volume (-1, Volset(volume));
}

//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
int I_StartSound ( int           id,
                   int           vol,
                   int           sep,
                   int           pitch,
                   int           priority )
{
	int voice;

	if(nosound)
	return 0;

	// UNUSED
	priority = 0;

	pitch = (pitch-128)/2+128;
	voice = play_sample(S_sfx[id].data,vol,sep,(pitch*1000)/128,0);

	// Returns a handle
	return (id<<VOICESSHIFT)+voice;
}

void I_StopSound (int handle)
{
	// You need the handle returned by StartSound.
	// Would be looping all channels,
	//  tracking down the handle,
	//  an setting the channel to zero.
	int voice=handle & (VIRTUAL_VOICES-1);

	if(nosound)
		return;

	if(voice_check(voice)==S_sfx[handle>>VOICESSHIFT].data)
		deallocate_voice(voice);
}

int I_SoundIsPlaying(int handle)
{
	if(nosound)
		return FALSE;

	if(voice_check(handle & (VIRTUAL_VOICES-1))==S_sfx[handle>>VOICESSHIFT].data)
		return TRUE;

	return FALSE;
}

// cut and past from ALLEGRO he don't share it :(
static inline int absolute_freq(int freq, SAMPLE *spl)
{
	if (freq == 1000)
		return spl->freq;
	else
		return (spl->freq * freq) / 1000;
}

void I_UpdateSoundParams( int   handle,
                          int   vol,
                          int   sep,
                          int   pitch)
{
	// I fail too see that this is used.
	// Would be using the handle to identify
	//  on which channel the sound might be active,
	//  and resetting the channel parameters.
	int voice=handle & (VIRTUAL_VOICES-1);
	int numsfx=handle>>VOICESSHIFT;

	if(nosound)
		return;

	if(voice_check(voice)==S_sfx[numsfx].data)
	{
		voice_set_volume(voice, vol);
		voice_set_pan(voice, sep);
		voice_set_frequency(voice, absolute_freq(pitch*1000/128,
		                    S_sfx[numsfx].data));
  }
}


void I_ShutdownSound(void)
{
	// Wait till all pending sounds are finished.

	//added:03-01-98:
	if( !sound_started )
		return;

	//added:08-01-98: remove_sound() explicitly because we don't use
	//                Allegro's allegro_exit();
	remove_sound();
	sound_started = false;
}

static char soundheader[] = "sound";
#if ALLEGRO_VERSION == 3
static char soundvar[] = "sb_freq";
#else
static char soundvar[] = "sound_freq";
#endif

void I_StartupSound(void)
{
	int    sfxcard,midicard;
	char   err[255];

	if (nosound)
		sfxcard=DIGI_NONE;
	else
		sfxcard=DIGI_AUTODETECT;

	if (nomusic)
		midicard=MIDI_NONE;
	else
		midicard=MIDI_AUTODETECT; //DetectMusicCard();

	nofmod=true; //Alam: No OGG/MP3/IT/MOD support

	// Secure and configure sound device first.
	CONS_Printf("I_StartupSound: ");

	//Fab:25-04-98:note:install_sound will check for sound settings
	//    in the sound.cfg or allegro.cfg, in the current directory,
	//    or the directory pointed by 'ALLEGRO' env var.
#if ALLEGRO_VERSION == 3
	if (install_sound(sfxcard,midicard,NULL)!=0)
	{
		sprintf (err,"Sound init error : %s\n",allegro_error);
		CONS_Error (err);
		nosound=true;
		nomusic=true;
	}
	else
	{
		CONS_Printf(" configured audio device\n" );
	}

	//added:08-01-98:we use a similar startup/shutdown scheme as Allegro.
	I_AddExitFunc(I_ShutdownSound);
#endif
	sound_started = true;
	CV_SetValue(&cv_samplerate,get_config_int(soundheader,soundvar,cv_samplerate.value));
}




//
// MUSIC API.
// Still no music done.
// Remains. Dummies.
//

static MIDI* currsong;   //im assuming only 1 song will be played at once
static int      islooping=0;
static int      musicdies=-1;
byte      music_started=0;


/* load_midi_mem:
 *  Loads a standard MIDI file from memory, returning a pointer to
 *  a MIDI structure, *  or NULL on error.
 *  It is the load_midi from Allegro modified to load it from memory
 */
static MIDI *load_midi_mem(char *mempointer,int *e)
{
	int c = *e;
	long data=0;
	char *fp;
	MIDI *midi;
	int num_tracks=0;

	fp = mempointer;
	if (!fp)
		return NULL;

	midi = malloc(sizeof(MIDI));              /* get some memory */
	if (!midi)
		return NULL;

	for (c=0; c<MIDI_TRACKS; c++)
	{
		midi->track[c].data = NULL;
		midi->track[c].len = 0;
	}

	fp+=4+4;   // header size + 'chunk' size

	swab(fp,&data,2);     // convert to intel-endian
	fp+=2;                                      /* MIDI file type */
	if ((data != 0) && (data != 1)) // only type 0 and 1 are suported
		return NULL;

	swab(fp,&num_tracks,2);                     /* number of tracks */
	fp+=2;
	if ((num_tracks < 1) || (num_tracks > MIDI_TRACKS))
		return NULL;

	swab(fp,&data,2);                          /* beat divisions */
	fp+=2;
	midi->divisions = ABS(data);

	for (c=0; c<num_tracks; c++)
	{            /* read each track */
		if (memcmp(fp, "MTrk", 4))
			return NULL;
		fp+=4;

		//swab(fp,&data,4);       don't work !!!!??
		((char *)&data)[0]=fp[3];
		((char *)&data)[1]=fp[2];
		((char *)&data)[2]=fp[1];
		((char *)&data)[3]=fp[0];
		fp+=4;

		midi->track[c].len = data;

		midi->track[c].data=fp;
		fp+=data;
	}

	lock_midi(midi);
	return midi;
}

void I_InitMIDIMusic(void)
{
	if(nomusic)
		return;

	I_AddExitFunc(I_ShutdownMusic);
	music_started = true;
}

void I_ShutdownMIDIMusic(void)
{
	if( !music_started )
		return;

	I_StopSong(1);

	music_started=false;
}

void I_InitDigMusic(void)
{
//	CONS_Printf("Digital music not yet supported under DOS.\n");
}

void I_ShutdownDigMusic(void)
{
//	CONS_Printf("Digital music not yet supported under DOS.\n");
}

void I_InitMusic(void)
{
	if(!nofmod)
		I_InitDigMusic();
	if(!nomusic)
		I_InitMIDIMusic();
}

void I_ShutdownMusic(void)
{
	I_ShutdownMIDIMusic();
	I_ShutdownDigMusic();
}

boolean I_PlaySong(int handle, int looping)
{
	handle = 0;
	if(nomusic)
		return false;

	islooping = looping;
	musicdies = gametic + TICRATE*30;
	if(play_midi(currsong,looping)==0)
		return true;
	return false;
}

void I_PauseSong (int handle)
{
	handle = 0;
	if(nomusic)
		return;

	midi_pause();
}

void I_ResumeSong (int handle)
{
	handle = 0;
	if(nomusic)
		return;

	midi_resume();
}

void I_StopSong(int handle)
{
	handle = 0;
	if(nomusic)
		return;

	islooping = 0;
	musicdies = 0;
	stop_midi();
}

// Is the song playing?
#if 0
int I_QrySongPlaying(int handle)
{
	if(nomusic)
		return 0;

	//return islooping || musicdies > gametic;
	return (midi_pos==-1);
}
#endif

void I_UnRegisterSong(int handle)
{
	handle = 0;
	if(nomusic)
		return;

	//destroy_midi(currsong);
}

int I_RegisterSong(void* data,int len)
{
	int e = len; //Alam: For error
	if(nomusic)
		return 1;

	if(memcmp(data,"MThd",4)==0) // support mid file in WAD !!!
	{
		currsong=load_midi_mem(data,&e);
	}
	else
	{
		CONS_Printf("Music Lump is not a MIDI lump\n");
		return 0;
	}

	if(currsong==NULL)
	{
		CONS_Printf("Not a valid mid file : %d\n",e);
		return 0;
	}

	return 1;
}

/// \todo Add OGG/MP3 support for dos
boolean I_StartDigSong(const char* musicname, int looping)
{
	musicname = NULL;
	looping = 0;
	//CONS_Printf("I_StartDigSong: Not yet supported under DOS.\n");
	return false;
}

void I_StopDigSong(void)
{
//	CONS_Printf("I_StopDigSong: Not yet supported under DOS.\n");
}

void I_SetDigMusicVolume(int volume)
{
	volume = 0;
	if(nofmod)
		return;

	// Now set volume on output device.
//	CONS_Printf("Digital music not yet supported under DOS.\n");
}
