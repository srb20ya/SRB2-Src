// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: s_fmod.c,v 1.1 2004/06/07 19:28:35 alam_gbc Exp $
//
// Copyright (C) 2001 by DooM Legacy Team.
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
// $Log: s_fmod.c,v $
//
// Revision 1.1  2004/09/18 19:49:44  alam_gbc
// Start of s_openal.dll
//-----------------------------------------------------------------------------
/// \file
/// \brief  General driver for 3D sound system
///
///	Implementend via OpenAL API

#ifdef _WINDOWS
//#define WIN32_LEAN_AND_MEAN
#define RPC_NO_WINDOWS_H
#include <windows.h>
HANDLE logstream = (HANDLE)-1;
#else
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifndef SDL
int logstream = -1;
#endif
#endif

#ifdef __MACH__
#include <al.h> //Main AL
#include <alc.h> //Helpers
#else
#include <AL/al.h> //Main AL
#include <AL/alc.h> //Helpers
#endif

#define  _CREATE_DLL_
#include "../../doomdef.h"
#include "../hw3dsdrv.h"

#undef DEBUG_TO_FILE
#ifndef SDLIO
#define DEBUG_TO_FILE
#endif

// Internal sound stack
typedef struct stack_snd_s
{
	ALuint ALbuffer;// Sound data

	ALuint ALsource;// 3D data of 3D source

	ALint sfx_id;// Currently unused

	ALint LRU;// Currently unused

	ALboolean permanent;// Flag of static source
} stack_snd_t;

#define AL3D_INVALID (ALuint)-1

static I_Error_t    I_ErrorOpenAl    = NULL;
static stack_snd_t *ALstack          = NULL; // Sound stack
static ALuint       allocated_sounds = 0;    // Size of stack
static ALuint       allocate_delta   = 16;
static ALCdevice   *ALCDevice        = NULL;
static ALCcontext  *ALContext        = NULL;
static ALenum       ALo_Lasterror    = AL_NO_ERROR;
static ALenum       ALCo_Lasterror   = ALC_NO_ERROR;

static ALenum ALo_GetError(ALvoid)
{
	ALo_Lasterror = alGetError();
	return ALo_Lasterror;
}

static ALenum ALCo_GetError(ALvoid)
{
	ALCo_Lasterror = alcGetError(ALCDevice);
	return ALCo_Lasterror;
}

static ALfloat Alvol(ALint vol, ALsizei step)
{
	return ((ALfloat)vol)/(ALfloat)step;
}

static ALfloat Alpitch(ALint pitch)
{
#if 1
	pitch = NORMAL_PITCH;
#endif
	return pitch < NORMAL_PITCH ?
		(float)(pitch + NORMAL_PITCH) / (NORMAL_PITCH * 2)
		:(float)pitch / (float)NORMAL_PITCH;
}

static const ALbyte *GetALErrorString(ALenum err)
{
	switch(err)
	{
		case AL_NO_ERROR:
			return "There no error";
			break;

		case AL_INVALID_NAME:
			return "Invalid name";
			break;

		case AL_INVALID_ENUM:
			return "Invalid enum";
			break;

		case AL_INVALID_VALUE:
			return "Invalid value";
			break;

		case AL_INVALID_OPERATION:
			return "Invalid operation";
			break;

		case AL_OUT_OF_MEMORY:
			return "Out Of Memory";
			break;

		default:
			return alGetString(err);
			break;
	}
}

static const ALbyte *GetALCErrorString(ALenum err)
{
	switch(err)
	{
		case ALC_NO_ERROR:
			return "There no error";
			break;

		case ALC_INVALID_DEVICE:
			return "Invalid Device";
			break;

		case ALC_INVALID_CONTEXT:
			return "Invalid Context";
			break;

		case ALC_INVALID_ENUM:
			return "Invalid enum";
			break;

		case ALC_INVALID_VALUE:
			return "Invalid value";
			break;

		case ALC_OUT_OF_MEMORY:
			return "Out Of Memory";
			break;

		default:
			return alcGetString(ALCDevice,err);
			break;
	}
}

/***************************************************************
 *
 * DBG_Printf
 * Output error messages to debug log if DEBUG_TO_FILE is defined,
 * else do nothing
 *
 **************************************************************
 */
// -----------------+
// DBG_Printf       : Output error messages to debug log if DEBUG_TO_FILE is defined,
//                  : else do nothing
// Returns          :
// -----------------+
FUNCPRINTF void DBG_Printf(const char* lpFmt, ... )
{
#ifdef DEBUG_TO_FILE
	char    str[4096];
	va_list arglist;

	va_start(arglist, lpFmt);
	vsnprintf(str, 4096, lpFmt, arglist);
	va_end(arglist);
#ifdef _WINDOWS
	{
		DWORD bytesWritten;
		if(logstream != (HANDLE)-1)
			WriteFile(logstream, str, lstrlen(str), &bytesWritten, NULL);
	}
#else
	if(logstream!=-1)
		write(logstream, str, strlen(str));
#endif
#else
	lpFmt = NULL;
#endif
}

/***************************************************************
 *
 * Grow internal sound stack by allocate_delta amount
 *
 ***************************************************************
 */
static ALboolean reallocate_stack(void)
{
	stack_snd_t *new_stack;
	if(ALstack)
		new_stack = realloc(ALstack, sizeof(stack_snd_t) * (allocated_sounds + allocate_delta));
	else
		new_stack = malloc(sizeof(stack_snd_t) * (allocate_delta));
	if(new_stack)
	{
		ALsizei clean_stack;
		ALstack = new_stack;
		memset(&ALstack[allocated_sounds],0, allocate_delta * sizeof(stack_snd_t));
		for(clean_stack = allocated_sounds; clean_stack < allocated_sounds; clean_stack++)
			ALstack[clean_stack].ALsource = ALstack[clean_stack].ALbuffer = AL3D_INVALID;
		allocated_sounds += allocate_delta;
		return AL_TRUE;
	}
	return AL_FALSE;
}


/***************************************************************
 *
 * Destroys source in stack
 *
 ***************************************************************
 */
static ALvoid kill_sound(stack_snd_t *snd)
{
	if (alIsSource(snd->ALsource))
		alDeleteSources(1,&snd->ALsource);
	ALo_GetError();
	if (alIsBuffer(snd->ALbuffer))
		alDeleteBuffers(1,&snd->ALbuffer);
	ALo_GetError();
	memset(snd,0, sizeof(stack_snd_t));
	snd->ALsource = snd->ALbuffer = AL3D_INVALID;
}

/***************************************************************
 *
 * Returns free (unused) source stack slot
 * If none available sound stack will be increased
 *
 ***************************************************************
 */
static ALint find_handle(void)
{
	ALuint free_sfx = 0;
	stack_snd_t *snd;

	for (snd = ALstack; free_sfx < allocated_sounds; snd++, free_sfx++)
	{
		if(snd->permanent)
			continue;
		if(snd->ALsource==AL3D_INVALID)
			break;
		if(!alIsSource(snd->ALsource))
			break;
		ALo_GetError();
		if(snd->sfx_id == 0)
			break;
	}

	// No suitable resource found so increase sound stack
	//DBG_Printf("sfx chan %d\n", free_sfx);
	if (free_sfx == allocated_sounds)
	{
		DBG_Printf("No free or same sfx found so increase stack (currently %d srcs)\n", allocated_sounds);
		free_sfx = reallocate_stack() ? free_sfx : -1;
	}
	return free_sfx;
}

static ALvoid ALSetPan(ALuint source,int sep)
{
	ALfloat facing[3] ={0.0f,0.0f,0.0f}; //Alam: bad?
	if(sep)
	{
		facing[0] = (ALfloat)sep/(ALfloat)NORMAL_SEP;
		facing[2] = 1.0f;
	}
	alSourcefv(source,AL_POSITION,facing);
}


/******************************************************************************
 *
 * Initialise driver and listener
 *
 *****************************************************************************/
EXPORT int HWRAPI( Startup ) (I_Error_t FatalErrorFunction, snddev_t *snd_dev)
{
	ALCboolean      inited     = ALC_FALSE;
	ALCint          AlSetup[8] = {ALC_FREQUENCY,22050,ALC_REFRESH,35,ALC_SYNC,AL_FALSE,ALC_INVALID,ALC_INVALID};
#if defined(_WIN32) || defined(_WIN64)
	const ALCubyte *ALCdriver  = alcGetString(NULL,ALC_DEFAULT_DEVICE_SPECIFIER);
	const ALCubyte *DSdriver = "DirectSound";
	
	if(!strcmp((const ALCbyte *)ALCdriver,"DirectSound3D")) //Alam: OpenAL's DS3D is buggy
		ALCDevice = alcOpenDevice(DSdriver); //Open DirectSound
	else //Alam: The OpenAl device
		ALCDevice = alcOpenDevice(ALCdriver); //Open Default
#else
	ALCDevice = alcOpenDevice(alcGetString(NULL,ALC_DEFAULT_DEVICE_SPECIFIER));
#endif
	if(!ALCDevice || ALCo_GetError() != AL_NO_ERROR)
	{
		DBG_Printf("S_OpenAl: Error %s when opening device!\n",GetALCErrorString(ALCo_Lasterror));
		return inited;
	}
	else
	{
		DBG_Printf("Driver Name : %s\n", alcGetString(ALCDevice,ALC_DEVICE_SPECIFIER));
		DBG_Printf("Extensions  : %s\n", alcGetString(ALCDevice,ALC_EXTENSIONS));
		DBG_Printf("S_OpenAl: OpenAl Device %s opened\n",alcGetString(ALCDevice, ALC_DEVICE_SPECIFIER));
	}

	I_ErrorOpenAl = FatalErrorFunction;

	AlSetup[1] = snd_dev->sample_rate; //ALC_FREQUENCY
	AlSetup[3] = 35;                   //ALC_REFRESH
	AlSetup[5] = AL_FALSE;             //ALC_SYNC
	
	//Alam: The Environment?
	ALContext = alcCreateContext(ALCDevice,AlSetup);
	if(!ALContext || ALCo_GetError() != AL_NO_ERROR)
	{
		DBG_Printf("S_OpenAl: Error %s when making the environment!\n",GetALCErrorString(ALCo_Lasterror));
		return inited;
	}
	else
	{
		DBG_Printf("S_OpenAl: OpenAl environment made\n");
	}

	alcMakeContextCurrent(ALContext);
	if(ALCo_GetError() != AL_NO_ERROR)
	{
		DBG_Printf("S_OpenAl: Error %s when setting the environment!\n",GetALCErrorString(ALCo_Lasterror));
		return inited;
	}
	else
	{
		DBG_Printf("S_OpenAl: OpenAl environment setted up\n");
	}

	DBG_Printf("Vendor      : %s\n", alGetString(AL_VENDOR));
	DBG_Printf("Renderer    : %s\n", alGetString(AL_RENDERER));
	DBG_Printf("Version     : %s\n", alGetString(AL_VERSION));
	DBG_Printf("Extensions  : %s\n", alGetString(AL_EXTENSIONS));

	alDopplerFactor(1.6f);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		DBG_Printf("S_OpenAl: Error %s when setting Doppler Factor!\n",GetALErrorString(ALo_Lasterror));
		return inited;
	}
	else
	{
		DBG_Printf("S_OpenAl: Doppler Factor of 1.6f setted\n");
	}
	
	alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		DBG_Printf("S_OpenAl: Error %s when setting Distance Model!\n",GetALErrorString(ALo_Lasterror));
		return inited;
	}
	else
	{
		DBG_Printf("S_OpenAl: Clamped Distance Model mode setted\n");
	}

	inited = reallocate_stack();

	return inited;
}

/******************************************************************************
 *
 * Shutdown 3D Sound
 *
 ******************************************************************************/
EXPORT void HWRAPI( Shutdown ) ( void )
{
	ALuint i;

	DBG_Printf ("S_OpenAL Shutdown()\n");

	for (i = 0; i < allocated_sounds; i++)
	{
		StopSource(i);
		kill_sound(ALstack + i);
	}

	if(ALstack)
		free(ALstack);

#ifndef LINUX //defined(__MACH__)?
	alcMakeContextCurrent(NULL);
	//TODO:check?
#endif

	if(ALContext)
	{
		alcDestroyContext(ALContext);
		ALContext = NULL;
	}
	//TODO:check?
	if(ALCDevice)
	{
		alcCloseDevice(ALCDevice);
		ALContext = NULL;
	}
	//TODO:check?
}

static ALsizei makedata(ALvoid *sfxdata, ALsizei len)
{
	ALsizei   i;
	ALubyte  *data     = sfxdata;
	ALushort *checksfx = (ALushort *)sfxdata+3;

	if(sfxdata && len)
	{
		for (i=8;i;--i)
		{ // Alam: Skip the first 8 bytes
			*data++;
			len--;
		}
		if(*checksfx == 0x0080) return (*((ALushort *)sfxdata+1)); //Alam: Already done
		for ( i=len; i; --i )
			*data++ ^= 0x80; //Alam: note that the data is unsigned and the OpenAL wants signed?
		*checksfx = 0x0080; // Alam: All done
		return (*((ALushort *)sfxdata+1));
	}
	else
		return 0;
}

static int makechan(sfx_data_t *sfx, ALboolean perm)
{
	ALint chan = find_handle();
	ALsizei freq = 11025;

	if(chan < -1) return chan;

	ALstack[chan].ALbuffer = (ALuint)chan;
	alGenBuffers(1,&ALstack[chan].ALbuffer);
	if(ALo_GetError() != AL_NO_ERROR || !alIsBuffer(ALstack[chan].ALbuffer))
	{
		DBG_Printf("S_OpenAl: Error %s when make an ALBuffer!\n",GetALErrorString(ALo_Lasterror));
		ALstack[chan].ALbuffer = AL3D_INVALID;
		return -1;
	}

	ALstack[chan].ALsource = (ALuint)chan;
	alGenSources(1,&ALstack[chan].ALsource);
	if(ALo_GetError() != AL_NO_ERROR || !alIsSource(ALstack[chan].ALsource))
	{
		DBG_Printf("S_OpenAl: Error %s when make an ALSource!\n",GetALErrorString(ALo_Lasterror));
		ALstack[chan].ALsource = AL3D_INVALID;
		alDeleteBuffers(1,&ALstack[chan].ALbuffer);
		ALo_GetError();
		ALstack[chan].ALbuffer = AL3D_INVALID;
		return -1;
	}

	if(sfx)
	{
		freq = makedata(sfx->data,sfx->length);

		alBufferData(ALstack[chan].ALbuffer,AL_FORMAT_MONO8,(ALubyte *)sfx->data+8,sfx->length-8,freq);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			DBG_Printf("S_OpenAl: Error %s when setting up a alBuffer!\n",GetALErrorString(ALo_Lasterror));
		}

		alSourcef(ALstack[chan].ALsource,AL_PITCH,    Alpitch(sfx->pitch));
		if(ALo_GetError() != AL_NO_ERROR)
		{
			DBG_Printf("S_OpenAl: Error %s when setting AL_PITCH!\n",GetALErrorString(ALo_Lasterror));
		}

		alSourcef(ALstack[chan].ALsource,AL_GAIN,     Alvol(sfx->volume,256));
		if(ALo_GetError() != AL_NO_ERROR)
		{
			DBG_Printf("S_OpenAl: Error %s when setting AL_GAIN!\n",GetALErrorString(ALo_Lasterror));
		}

		alSourcei(ALstack[chan].ALsource,AL_BUFFER,   ALstack[chan].ALbuffer);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			DBG_Printf("S_OpenAl: Error %s when setting up an alSource!\n",GetALErrorString(ALo_Lasterror));
		}
		ALstack[chan].sfx_id = sfx->id;
	}
	else
		ALstack[chan].sfx_id = -1;

	ALstack[chan].permanent = perm;

	return chan;
}

/******************************************************************************
 *
 * Creates 3D source
 *
 ******************************************************************************/
EXPORT int HWRAPI ( Add3DSource ) (source3D_data_t *src, sfx_data_t *sfx)
{
	ALint chan = makechan(sfx, (ALboolean)src->permanent);

	if(chan > -1)
	{
#ifdef AL_MIN_DISTANCE //Alam: Maybe?
		alSourcef(ALstack[chan].ALsource,AL_MIN_DISTANCE,src->min_distance);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}
#endif
		alSourcef(ALstack[chan].ALsource,AL_MAX_DISTANCE,src->max_distance);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}
		if(src->head_relative)
		{
			alSourcei(ALstack[chan].ALsource,AL_SOURCE_TYPE,AL_SOURCE_RELATIVE);
			if(ALo_GetError() != AL_NO_ERROR)
			{
				//TODO
			}
		}

		alSource3f(ALstack[chan].ALsource,AL_POSITION,src->pos.x,   src->pos.z,   src->pos.y);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}
		alSource3f(ALstack[chan].ALsource,AL_VELOCITY,src->pos.momx,src->pos.momz,src->pos.momy);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}
	}

	return chan;
}

/******************************************************************************
 *
 * Creates 2D (stereo) source
 *
 ******************************************************************************/
EXPORT int HWRAPI ( Add2DSource ) (sfx_data_t *sfx)
{
	ALint chan = makechan(sfx, AL_FALSE);

	if(chan > -1)
	{
		alSourcei(ALstack[chan].ALsource,AL_SOURCE_TYPE,AL_SOURCE_RELATIVE);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}

		alSourcef(ALstack[chan].ALsource,AL_ROLLOFF_FACTOR, 0);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}

		ALSetPan(ALstack[chan].ALsource,sfx->sep-NORMAL_SEP);
		{
			//TODO
		}
	}

	return chan;
}

EXPORT int HWRAPI (StartSource) (int chan)
{
	ALint playing = AL_FALSE;

	if (chan < -1 || !alIsSource(ALstack[chan].ALsource))
		return playing;
	else
		ALo_GetError();
	
	alSourcePlay(ALstack[chan].ALsource);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}

	alGetSourcei(ALstack[chan].ALsource, AL_PLAYING, &playing);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}

	return playing;
}

EXPORT void HWRAPI ( StopSource) (int chan)
{
	if (chan < -1 || !alIsSource(ALstack[chan].ALsource))
		return;
	else
		ALo_GetError();

	alSourcePause(ALstack[chan].ALsource);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}
}

EXPORT int HWRAPI ( GetHW3DSVersion) (void)
{
	return VERSION;
}

EXPORT void HWRAPI (BeginFrameUpdate) (void)
{
	alcSuspendContext(ALContext);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}
}

EXPORT void HWRAPI (EndFrameUpdate) (void)
{
	alcProcessContext(ALContext);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}
}

EXPORT int HWRAPI (IsPlaying) (int chan)
{
	ALint playing = AL_FALSE;

	if (chan < -1 || !alIsSource(ALstack[chan].ALsource))
		return playing;
	else
		ALo_GetError();

	alGetSourcei(ALstack[chan].ALsource,AL_PLAYING, &playing);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}

	return playing;
}

/******************************************************************************
 * UpdateListener
 *
 * Set up main listener properties:
 * - position
 * - orientation
 * - velocity
 *****************************************************************************/
EXPORT void HWRAPI (UpdateListener) (listener_data_t* data)
{
	if(data)
	{
		ALfloat facing[6];
		ALdouble f_angle = 0;
		if(data->f_angle) f_angle = (data->f_angle) / 180 * PI;
		facing[0] = (ALfloat)cos(f_angle);
		facing[1] = 0.0f;
		facing[2] = (ALfloat)sin(f_angle);
		facing[3] = 0.0f;
		facing[4] = 1.0f;
		facing[5] = 0.0f;

		alListener3f(AL_POSITION,(ALfloat)data->x,(ALfloat)data->z,(ALfloat)data->y);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}

		alListener3f(AL_VELOCITY,(ALfloat)data->momx,(ALfloat)data->momz,(ALfloat)data->momy);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}

		alListenerfv(AL_ORIENTATION, facing);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}
	}
	else
	{
		DBG_Printf("Error: 1st listener data is missing\n");
	}
}

EXPORT void HWRAPI (UpdateListener2) (listener_data_t* data)
{
	if(data)
	{
		ALfloat facing[6];
		double f_angle = 0;
		if(data->f_angle) f_angle = (data->f_angle) / 180 * PI;
		facing[0] = (ALfloat)cos(f_angle);
		facing[1] = 0.0f;
		facing[2] = (ALfloat)sin(f_angle);
		facing[3] = 0.0f;
		facing[4] = 1.0f;
		facing[5] = 0.0f;

		//alListener3f(AL_POSITION,(ALfloat)data->x,(ALfloat)data->z,(ALfloat)data->y);
		//alListener3f(AL_VELOCITY,(ALfloat)data->momx,(ALfloat)data->momz,(ALfloat)data->momy);
		//alListenerfv(AL_ORIENTATION, facing);
	}
	else
	{
		//DBG_Printf("Error: 2st listener data is missing\n");
	}
}

EXPORT void HWRAPI (UpdateSourceVolume) (int chan, int vol)
{
	if(chan > 0 && alIsSource(ALstack[chan].ALsource))
		alSourcef(ALstack[chan].ALsource,AL_GAIN,Alvol(vol,256));
	ALo_GetError();
}

/******************************************************************************
 *
 * Update volume and separation (panning) of 2D source
 *
 *****************************************************************************/
EXPORT void HWRAPI (Update2DSoundParms) (int chan, int vol, int sep)
{
	if (chan > 0 && alIsSource(ALstack[chan].ALsource))
	{
		alSourcef(ALstack[chan].ALsource,AL_GAIN,Alvol(vol,256));
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}

		ALSetPan(ALstack[chan].ALsource,sep-NORMAL_SEP);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}
	}
}

// --------------------------------------------------------------------------
// Set the global volume for sound effects
// --------------------------------------------------------------------------
EXPORT void HWRAPI (SetGlobalSfxVolume) (int vol)
{
	alListenerf(AL_GAIN,Alvol(vol,32));
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}
}

//Alam: Not Used?
EXPORT int HWRAPI (SetCone) (int chan, cone_def_t *cone_def)
{
	if (chan < 0 || !alIsSource(ALstack[chan].ALsource))
		return AL_FALSE;
	else
		ALo_GetError();
	if(cone_def)
	{
		alSourcef(ALstack[chan].ALsource,AL_CONE_INNER_ANGLE,cone_def->inner);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}

		alSourcef(ALstack[chan].ALsource,AL_CONE_OUTER_ANGLE,cone_def->outer);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}

		alSourcei(ALstack[chan].ALsource,AL_CONE_OUTER_GAIN, cone_def->outer_gain);
		if(ALo_GetError() != AL_NO_ERROR)
		{
			//TODO
		}

		return AL_TRUE;
	}
	else
	{
		return AL_FALSE;
	}
}

EXPORT void HWRAPI (Update3DSource) (int chan, source3D_pos_t *sfx)
{
	if (chan < 0 || !alIsSource(ALstack[chan].ALsource))
		return;
	else
		ALo_GetError();
	alSource3f(ALstack[chan].ALsource,AL_POSITION,sfx->x,   sfx->z,   sfx->y);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}

	alSource3f(ALstack[chan].ALsource,AL_VELOCITY,sfx->momx,sfx->momz,sfx->momy);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}
}

//-------------------------------------------------------------
// Load new sound data into source
//-------------------------------------------------------------
EXPORT int HWRAPI (Reload3DSource) (int chan, sfx_data_t *sfx)
{
	ALsizei freq = 0;

	if (chan < -1 || !sfx || !alIsSource(ALstack[chan].ALsource) || !alIsBuffer(ALstack[chan].ALbuffer))
		return -1;
	else
		ALo_GetError();
	freq = makedata(sfx->data,sfx->length);
	alBufferData(ALstack[chan].ALbuffer,AL_FORMAT_MONO8,(ALbyte *)sfx->data+8,sfx->length-8,freq);
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}
	alSourcef(ALstack[chan].ALsource,AL_PITCH,Alpitch(sfx->pitch));
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}
	alSourcef(ALstack[chan].ALsource,AL_GAIN,Alvol(sfx->volume,256));
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}
	alSourcei(ALstack[chan].ALsource,AL_BUFFER,ALstack[chan].ALbuffer); //Alam: not needed?
	if(ALo_GetError() != AL_NO_ERROR)
	{
		//TODO
	}
	return chan;
}

/******************************************************************************
 *
 * Destroy source and remove it from stack if it is a 2D source.
 * Otherwise put source into cache
 *
 *****************************************************************************/
EXPORT void HWRAPI (KillSource) (int chan)
{
	if (chan >= 0 && (ALuint)chan <= allocated_sounds)
		kill_sound(ALstack + chan);
}

#ifdef _WINDOWS
BOOL WINAPI DllMain( HANDLE hModule,      // handle to DLL module
                       DWORD fdwReason,     // reason for calling function
                       LPVOID lpReserved )  // reserved
{
	// Perform actions based on the reason for calling.
	hModule = (HANDLE)-1;
	lpReserved = NULL;
	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
#ifdef DEBUG_TO_FILE
			logstream = CreateFile ("s_openal.log", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
			                     FILE_ATTRIBUTE_NORMAL/*|FILE_FLAG_WRITE_THROUGH*/, NULL);
			if (logstream == (HANDLE)-1)
				return FALSE;
#endif
		break;

		case DLL_THREAD_ATTACH:
			// Do thread-specific initialization.
			break;

		case DLL_THREAD_DETACH:
			// Do thread-specific cleanup.
			break;

		case DLL_PROCESS_DETACH:
			// Perform any necessary cleanup.
#ifdef DEBUG_TO_FILE
			if ( logstream != (HANDLE)-1 )
			{
				CloseHandle ( logstream );
				logstream  = (HANDLE)-1;
			}
#endif
			break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
#elif !defined(SDL)

// **************************************************************************
//                                                                  FUNCTIONS
// **************************************************************************

EXPORT void _init()
{
#ifdef DEBUG_TO_FILE
	logstream = open("s_openal.log",O_WRONLY|O_CREAT);
#endif
}

EXPORT void _fini()
{
#ifdef DEBUG_TO_FILE
	if(logstream!=-1)
		close(logstream);
#endif
}
#endif

