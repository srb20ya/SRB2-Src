// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: s_ds3d.c,v 1.3 2001/08/21 21:50:06 judgecutor Exp $
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
// $Log: s_ds3d.c,v $
// Revision 1.3  2001/08/21 21:50:06  judgecutor
// Fixed distance effect
//
// Revision 1.2  2001/04/06 21:36:53  judgecutor
// Added detection for speaker configuration
//
// Revision 1.1  2001/04/04 19:49:44  judgecutor
// Initial release
//
//
// DESCRIPTION:
//      General driver for 3D sound system.
//      Implementend via DirectSound3D API
//
//-----------------------------------------------------------------------------

#define INITGUID
#include <windows.h>
#include <cguid.h>
#include <dsound.h>


#define  _CREATE_DLL_
#include "../hw3dsdrv.h"
#include "../../m_fixed.h"

#undef DEBUG_TO_FILE
#define DEBUG_TO_FILE


// Internal sound stack
typedef struct stack_snd_s
{
    // Sound data
    LPDIRECTSOUNDBUFFER     dsbuffer;

    // 3D data of 3D source
    LPDIRECTSOUND3DBUFFER   dsbuffer3D;     // 3D data

    // Current parameters of 3D source
    // Valid only when source is 3D source
    // (dsbuffer3D is not NULL)
    DS3DBUFFER              parameters;

    // Currently unused
    int                     sfx_id;

    // Currently unused
    int                     LRU;

    // Flag of static source
    // Driver does not manage intrenally such sources
    int                     permanent;
    
} stack_t;

// Just for now...
#define SOUND_ALLOCATE_DELTA    16       // Amount of sources per stack incrementation
#define MAX_LRU                 16       // Maximum iterations to keep source in stack

static stack_t  *_stack;                 // Sound stack
static int      allocated_sounds;        // Size of stack

static int      srate;                   // Default sample rate


// output all debugging messages to this file
#ifdef DEBUG_TO_FILE
static HANDLE  logstream;
#endif

static LPDIRECTSOUND            DSnd            = NULL;  // Main DirectSound object
static LPDIRECTSOUNDBUFFER      PrimaryBuffer   = NULL;  // 
static LPDIRECTSOUND3DLISTENER  Listener        = NULL;  //

static DS3DLISTENER             listener_parms;          // Listener papameters

static BOOL                     virtualization;          // TRUE if HRTF virtualization enabled
static DWORD                    update_mode;             // Current update mode of listener
//static DWORD                    max_3d_buffers;

static I_Error_t I_ErrorDS3D = NULL;

//static stack_snd_t sound_stack[MAX_SOUNDS];

// Safe buffer release
#define RELEASE_BUFFER(buf)     {if (buf) { IDirectSoundBuffer_Release(buf); (buf) = NULL; }}
#define RELEASE_3DBUFFER(buf)   {if (buf) { IDirectSound3DBuffer_Release(buf); (buf) = NULL; }}

// Default flags for buffers
#define _2DSOURCE_FLAGS (DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_STICKYFOCUS | DSBCAPS_STATIC)
#define _3DSOURCE_FLAGS (DSBCAPS_CTRL3D | DSBCAPS_STATIC | DSBCAPS_MUTE3DATMAXDISTANCE | DSBCAPS_CTRLVOLUME)


enum {IS_2DSOURCE = 0, IS_3DSOURCE = 1};

#define NEW_HANDLE  -1


BOOL APIENTRY DllMain( HANDLE hModule,      // handle to DLL module
                       DWORD fdwReason,     // reason for calling function
                       LPVOID lpReserved )  // reserved
{
    // Perform actions based on the reason for calling.
    switch( fdwReason )
    {
        case DLL_PROCESS_ATTACH:
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
#ifdef DEBUG_TO_FILE
            logstream = INVALID_HANDLE_VALUE;
            logstream = CreateFile ("s_ds3d.log", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL/*|FILE_FLAG_WRITE_THROUGH*/, NULL);
            if (logstream == INVALID_HANDLE_VALUE)
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
            if ( logstream != INVALID_HANDLE_VALUE ) {
                CloseHandle ( logstream );
                logstream  = INVALID_HANDLE_VALUE;
            }
#endif
            break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

/***************************************************************
 *
 * DBG_Printf
 * Output error messages to debug log if DEBUG_TO_FILE is defined,
 * else do nothing
 *
 ***************************************************************
 */
void DBG_Printf (LPCTSTR lpFmt, ...)
{
#ifdef DEBUG_TO_FILE
    char    str[1999];
    va_list arglist;
    DWORD   bytesWritten;

    va_start  (arglist, lpFmt);
    vsprintf (str, lpFmt, arglist);
    va_end    (arglist);

    if ( logstream != INVALID_HANDLE_VALUE )
        WriteFile (logstream, str, lstrlen(str), &bytesWritten, NULL);
#endif
}


/***************************************************************
 *
 * Grow intrenal sound stack by SOUND_ALLOCATE_DELTA amount
 *
 ***************************************************************
 */
static BOOL reallocate_stack(void)
{
    stack_t*   new_stack;

    new_stack = realloc(_stack, sizeof(stack_t) * (allocated_sounds + SOUND_ALLOCATE_DELTA));
    if (new_stack)
    {
       _stack = new_stack;
       ZeroMemory(&_stack[allocated_sounds], SOUND_ALLOCATE_DELTA * sizeof(stack_t));
       allocated_sounds += SOUND_ALLOCATE_DELTA;
    }
    return (new_stack != NULL);
}


/***************************************************************
 *
 * Destroys source in stack
 *
 ***************************************************************
 */
static void kill_sound(stack_t *snd)
{
    //stack_t *snd = _stack + handle;
    
    if (snd->dsbuffer3D)
        RELEASE_3DBUFFER(snd->dsbuffer3D);
    
    if (snd->dsbuffer)
        RELEASE_BUFFER(snd->dsbuffer);

    ZeroMemory(snd, sizeof(stack_t));

}


/***************************************************************
 *
 * Returns TRUE if source currently playing
 *
 ***************************************************************
 */
BOOL is_playing(stack_t *snd)
{
    DWORD               status;
    LPDIRECTSOUNDBUFFER dsbuffer = snd->dsbuffer;

    if (dsbuffer == NULL)
        return 0;

	IDirectSoundBuffer_GetStatus(dsbuffer, &status);
    return (status & (DSBSTATUS_PLAYING | DSBSTATUS_LOOPING));

}


/***************************************************************
 *
 * Creates DirectSound buffer and fills with sound data
 * NULL sound data pointer are valid (empty buffer
 * will be created)
 *
 ***************************************************************
 */
static LPDIRECTSOUNDBUFFER create_buffer (void *data, int length, BOOL as3d)
{
    LPDIRECTSOUNDBUFFER     dsbuffer;
    HRESULT                 hr;
    WAVEFORMATEX            wfm;
    DSBUFFERDESC            dsbdesc;
    LPVOID                  lpvAudio1;              // receives address of lock start
    DWORD                   dwBytes1;               // receives number of bytes locked
    LPVOID                  lpvAudio2;              // receives address of lock start
    DWORD                   dwBytes2;               // receives number of bytes locked

    ZeroMemory (&wfm, sizeof(WAVEFORMATEX));
    wfm.wFormatTag = WAVE_FORMAT_PCM;
    wfm.nChannels = 1;
    wfm.nSamplesPerSec = data?*((unsigned short*)data+1):srate;      //mostly 11025, but some at 22050.
    wfm.wBitsPerSample = 8;
    wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
    wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

    // Set up DSBUFFERDESC structure.
    ZeroMemory (&dsbdesc, sizeof(DSBUFFERDESC) );
    dsbdesc.dwSize = sizeof (DSBUFFERDESC);
    dsbdesc.dwFlags = as3d?_3DSOURCE_FLAGS:_2DSOURCE_FLAGS;
    dsbdesc.dwBufferBytes = length;
    dsbdesc.lpwfxFormat = &wfm;

    // DirectX 7.0 and above!
    // Try to enable full HRTF virtualization algorithm for 
    // two-speakers or headphones
    if (as3d) 
        dsbdesc.guid3DAlgorithm = (virtualization?DS3DALG_HRTF_FULL:DS3DALG_DEFAULT);

    hr = IDirectSound_CreateSoundBuffer (DSnd, &dsbdesc, &dsbuffer, NULL);

    // CreateSoundBuffer might return DS_NO_VIRTUALIZATION so uses FAILED
    // macro rather than check explictly for DS_OK value
    if (FAILED(hr))
    {
        DBG_Printf("CreateSoundBuffer FAILED. Code %d\n", hr);
        return NULL;
    }

    if (data)
    {
        hr = IDirectSoundBuffer_Lock (dsbuffer, 0, length, &lpvAudio1, &dwBytes1, &lpvAudio2, &dwBytes2, 0);

        // If DSERR_BUFFERLOST is returned, restore and retry lock. 
        if (hr == DSERR_BUFFERLOST) 
        { 
            hr = IDirectSoundBuffer_Restore (dsbuffer);
            if( FAILED (hr) )
                I_ErrorDS3D("Restore fail on %x, code %d\n",dsbuffer, hr);
            hr = IDirectSoundBuffer_Lock (dsbuffer, 0, length, &lpvAudio1, &dwBytes1, NULL, NULL, 0);
            if( FAILED (hr) )
                I_ErrorDS3D("Lock fail(2) on %x, code %d\n",dsbuffer, hr);
        }
        else
            if( FAILED (hr) )
                I_ErrorDS3D("Lock fail(1) on %x, code %d\n",dsbuffer, hr);

        // copy wave data into the buffer (note: dwBytes1 should equal to dsbdesc->dwBufferBytes ...)
        CopyMemory (lpvAudio1, (byte*)data+8, dwBytes1);

        if ( dwBytes2 && lpvAudio2)  
            CopyMemory(lpvAudio2, ((byte*)data + 8) + dwBytes1, dwBytes2); 

    
        // finally, unlock the buffer
        hr = IDirectSoundBuffer_Unlock (dsbuffer, lpvAudio1, dwBytes1, lpvAudio2, dwBytes2);

        if( FAILED (hr) )
            I_ErrorDS3D("Unlock fail on %x, code %d\n",dsbuffer, hr);
    }

    return dsbuffer;
}


/***************************************************************
 *
 * Creates 3D source data buffer
 *
 ***************************************************************
 */
static LPDIRECTSOUND3DBUFFER create_3dbuffer(LPDIRECTSOUNDBUFFER dsbuffer, LPDIRECTSOUND3DBUFFER source3d)
{
    HRESULT hr;

    hr = IDirectSoundBuffer_QueryInterface(dsbuffer,
            &IID_IDirectSound3DBuffer, (void **)&source3d);

    if (FAILED(hr))
    {
        DBG_Printf("Couldn't obtain 3D Buffer interface. Code %d\n", hr);
        return NULL;
    }

    if (hr == DS_NO_VIRTUALIZATION)
    {
        DBG_Printf("The 3D virtualization not supported under this OS.\n");
            virtualization = FALSE;
    }
    return source3d;
}


/***************************************************************
 *
 * Returns free (unused) source stack slot
 * If none available sound stack will be increased
 *
 ***************************************************************
 */
static int find_handle(int new_sfx_id, int new_is3d)
{
	int			    free_sfx;
	stack_t         *snd;
    
    
    // At first do look for sound with same sfx ID and reuse it
    for (free_sfx = 0, snd = _stack; free_sfx < allocated_sounds; snd++, free_sfx++)
    {
        
        if (snd->permanent)
            continue;
/*
        if (!is_playing(snd)) 
        {
            if (same_sfx == 0 && snd->sfx_id == new_sfx_id && snd->dsbuffer)
            {
                same_sfx = i;
                continue;
            }

            if (snd->sfx_id && ++snd->LRU >= MAX_LRU)
                kill_sound(snd);
        }
*/
        if (snd->dsbuffer == 0)
            break;
            //free_sfx = i;

    }

    // No suitable resource found so increase sound stack
    if (free_sfx == allocated_sounds)
    {
        DBG_Printf("No free or same sfx found so increase stack (currently %d srcs)\n", allocated_sounds);
        free_sfx = reallocate_stack() ? free_sfx : -1;
    };
   
    return free_sfx;
        
}


/***************************************************************
 *
 * Recalculates volume from Doom scale to DirectSound scale
 *
 ***************************************************************
 */
static int recalc_volume(int base_vol, int steps)
{
    return (base_vol * ((DSBVOLUME_MAX-DSBVOLUME_MIN)/4)) / steps
           + (DSBVOLUME_MAX - ((DSBVOLUME_MAX-DSBVOLUME_MIN)/4));
}


/***************************************************************
 *
 * Updates sound volume of the DirectSound buffer
 *
 ***************************************************************
 */
static void UpdateSoundVolume (LPDIRECTSOUNDBUFFER lpSnd, int volume)
{
    /*volume = (volume * ((DSBVOLUME_MAX-DSBVOLUME_MIN)/4)) / 256 +
                        (DSBVOLUME_MAX - ((DSBVOLUME_MAX-DSBVOLUME_MIN)/4));

    IDirectSoundBuffer_SetVolume (lpSnd, volume);*/
    IDirectSoundBuffer_SetVolume (lpSnd, recalc_volume(volume, 256));
}


// --------------------------------------------------------------------------
// Update the panning for a secondary buffer, make sure it was created with
// DSBCAPS_CTRLPAN
// --------------------------------------------------------------------------
#define DSBPAN_RANGE    (DSBPAN_RIGHT-(DSBPAN_LEFT))

//Doom sounds pan range 0-255 (128 is centre)
#define SEP_RANGE       256             
static void Update2DSoundPanning (LPDIRECTSOUNDBUFFER lpSnd, int sep)
{
    HRESULT hr;
    hr = IDirectSoundBuffer_SetPan (lpSnd, (sep * DSBPAN_RANGE)/SEP_RANGE - DSBPAN_RIGHT);
    //if (FAILED(hr))
    //    CONS_Printf ("SetPan FAILED for sep %d pan %d\n", sep, (sep * DSBPAN_RANGE)/SEP_RANGE - DSBPAN_RIGHT);
}



/******************************************************************************
 *
 * Initialise driver and listener
 *
 *****************************************************************************/
EXPORT BOOL HWRAPI( Startup ) (I_Error_t FatalErrorFunction, snddev_t *snd_dev)
{
    HRESULT         hr;
    DSBUFFERDESC    desc;
    WAVEFORMATEX    wfm;
    DSCAPS          dscaps;
    DWORD           speakers;
    DWORD           speaker_config;
    DWORD           speaker_geometry;
    
    I_ErrorDS3D = FatalErrorFunction;
    DBG_Printf ("S_DS3D Init(): DirectSound3D driver for SRB2 v1.10"); // Tails

    DBG_Printf("Initialising DirectSound3D...\n");
    hr = DirectSoundCreate( NULL, &DSnd, NULL);
    if (FAILED(hr))
    {
        DBG_Printf("Failed to obtain DirectSound\n");
        return FALSE;
    }

    hr = IDirectSound_SetCooperativeLevel(DSnd, snd_dev->hWnd, snd_dev->cooplevel);
    if (FAILED (hr))
    {
        DBG_Printf("Couldn't set coopertive level\n");
        return FALSE;
    }

    dscaps.dwSize = sizeof(DSCAPS);

    IDirectSound_GetCaps(DSnd, &dscaps);
    IDirectSound_GetSpeakerConfig(DSnd, &speakers);

    DBG_Printf("Sound hardware capabilities:\n");
    DBG_Printf("   Driver is %scertified.\n", (dscaps.dwFlags & DSCAPS_CERTIFIED) == DSCAPS_CERTIFIED?"":"not ");
    DBG_Printf("   Maximum hardware mixing buffers %d\n", dscaps.dwMaxHwMixingAllBuffers);
    DBG_Printf("   Maximum hardware 3D buffers %d\n", dscaps.dwFreeHw3DAllBuffers);

    speaker_config = DSSPEAKER_CONFIG(speakers);
    speaker_geometry = DSSPEAKER_GEOMETRY(speakers);

    DBG_Printf("Current speaker configuration: ");

    switch (speaker_config)
    {
        case DSSPEAKER_5POINT1:
            DBG_Printf("5.1 (5 speakers with subwoofer).\n");
            break;

        case DSSPEAKER_HEADPHONE:
            DBG_Printf("headphone.\n");
            break;

        case DSSPEAKER_MONO:
            DBG_Printf("single speaker (mono).\n");
            break;

        case DSSPEAKER_QUAD:
            DBG_Printf("quadrophonic\n");
            break;

        case DSSPEAKER_SURROUND:
            DBG_Printf("surround.\n");
            break;

        case DSSPEAKER_STEREO:
            DBG_Printf("stereo with %s geometry ",
                speaker_geometry == DSSPEAKER_GEOMETRY_WIDE
                    ? "wide (arc of 20 deg.)"
                    : speaker_geometry == DSSPEAKER_GEOMETRY_NARROW
                        ? "narrow (arc of 10 deg.)"
                        : speaker_geometry == DSSPEAKER_GEOMETRY_MIN
                            ? "min (arc of 5 deg.)"
                            : speaker_geometry == DSSPEAKER_GEOMETRY_MAX
                                ? "max (arc of 180 deg.)"
                                : "unknown"); 
            break;
        default:
            DBG_Printf("undetectable.\n");

    }

    update_mode = DS3D_IMMEDIATE;

    // Create primary sound buffer
    ZeroMemory(&desc, sizeof(DSBUFFERDESC));
    desc.dwSize = sizeof(DSBUFFERDESC);
    desc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
    desc.dwBufferBytes = 0;
    desc.lpwfxFormat = NULL;

    hr = IDirectSound_CreateSoundBuffer(DSnd, &desc, &PrimaryBuffer, NULL);
    if (FAILED(hr))
    {
        DBG_Printf("CreateSoundBuffer FAILED (ErrNo %d)\n", hr);
        return FALSE;
    }

    // Query for 3D Listener object
    hr = IDirectSoundBuffer_QueryInterface(PrimaryBuffer,
            &IID_IDirectSound3DListener, (void **)&Listener); 
            
    if (FAILED( hr ) )
    {
        DBG_Printf("Couldn't obtain 3D Listener interface (ErrNo %d)\n", hr);
        return FALSE;
    }

    // Set up initial listsner parameters
    IDirectSound3DListener_SetDistanceFactor(Listener, (float)1/(float)72.0, DS3D_IMMEDIATE);
    //IDirectSound3DListener_SetRolloffFactor(Listener, DS3D_MAXROLLOFFFACTOR, DS3D_IMMEDIATE);
    IDirectSound3DListener_SetRolloffFactor(Listener, 1.7, DS3D_IMMEDIATE);
    listener_parms.dwSize = sizeof(DS3DLISTENER);
    IDirectSound3DListener_GetAllParameters(Listener, &listener_parms);
    
    ZeroMemory (&wfm, sizeof(WAVEFORMATEX));
    wfm.wFormatTag = WAVE_FORMAT_PCM;
    wfm.nChannels = 2;
    wfm.nSamplesPerSec = srate = snd_dev->sample_rate;
    wfm.wBitsPerSample = snd_dev->bps;
    wfm.nBlockAlign = wfm.wBitsPerSample / 8 * wfm.nChannels;
    wfm.nAvgBytesPerSec = wfm.nSamplesPerSec * wfm.nBlockAlign;

    if (snd_dev->cooplevel >= DSSCL_PRIORITY)
    {
        hr = IDirectSoundBuffer_SetFormat (PrimaryBuffer, &wfm);
        if (FAILED(hr))
            DBG_Printf ("Couldn't set primary buffer format.\n");
        
        DBG_Printf (" Compacting onboard sound-memory...");
        hr = IDirectSound_Compact (DSnd);
        DBG_Printf (" %s\n", SUCCEEDED(hr) ? "done" : "FAILED");
    }

    // Initially enables HRTF virtualization (may be changed later)
    virtualization = FALSE;

	_stack = NULL;
	allocated_sounds = 0;
    IDirectSoundBuffer_Play(PrimaryBuffer, 0, 0, DSBPLAY_LOOPING);
    return reallocate_stack();
}



/***************************************************************
 *
 * Shutdown driver
 *
 ***************************************************************
 */
EXPORT void HWRAPI( Shutdown ) (void)
{
    int     i;

    DBG_Printf ("S_DS3D Shutdown()\n");

    for (i = 0; i < allocated_sounds; i++)
    {
        StopSource(i);
        kill_sound(_stack + i);
    }

    if (_stack)
        free(_stack);

    if (Listener)
    {
        IDirectSound3DListener_Release(Listener);
        Listener = NULL;
    }

    if (PrimaryBuffer)
        RELEASE_BUFFER(PrimaryBuffer);

    if (DSnd)
    {
        IDirectSound_Release(DSnd);
        DSnd = NULL;
    }
    
}



EXPORT int HWRAPI (IsPlaying) (int handle)
{
    if (handle < 0 || handle >= allocated_sounds)
        return FALSE;

    return is_playing(_stack + handle);
}



static stack_t *setup_source(int handle, sfx_data_t *sfx, BOOL is_3dsource)
{
    stack_t                 *snd;
    //int                     handle;
    int                     data_length;
    LPDIRECTSOUNDBUFFER     dsbuffer = NULL;
    LPDIRECTSOUND3DBUFFER   ds3dbuffer = NULL;

    if (handle == NEW_HANDLE)
        handle = find_handle(sfx?sfx->id:0, is_3dsource);

    snd = _stack + handle;

    // Check for reused source
    if (snd->dsbuffer)
        return snd;

    data_length = sfx?sfx->length - 8:DSBSIZE_MIN;

    dsbuffer = create_buffer(sfx ? sfx->data : NULL, data_length, is_3dsource);
    if (dsbuffer)
    {
        if (is_3dsource)
        {
            ds3dbuffer = create_3dbuffer(dsbuffer, ds3dbuffer);
            if (!ds3dbuffer)
            {
                RELEASE_BUFFER(dsbuffer);
                return NULL;
            }
            snd->parameters.dwSize = sizeof(DS3DBUFFER);
            IDirectSound3DBuffer_GetAllParameters(ds3dbuffer, &snd->parameters);

        }
        snd->dsbuffer = dsbuffer;
        snd->dsbuffer3D = ds3dbuffer;
        snd->LRU = 0;
        snd->sfx_id = sfx ? sfx->id : 0;
        if (!is_3dsource)
            snd->permanent = 0;
        return snd;
    }
    return NULL;

}

/******************************************************************************
 *
 * Creates 2D (stereo) source
 *
 ******************************************************************************/
EXPORT int HWRAPI ( Add2DSource ) (source2D_data_t *src, sfx_data_t *sfx)
{
    stack_t *snd;

    snd = setup_source(NEW_HANDLE, sfx, IS_2DSOURCE);
    if (snd)
    {
        UpdateSoundVolume(snd->dsbuffer, src->volume);
        Update2DSoundPanning(snd->dsbuffer, src->sep);
    }
    return snd - _stack;
}

/******************************************************************************
 *
 * Creates 3D source
 *
 ******************************************************************************/

EXPORT int HWRAPI ( Add3DSource ) (source3D_data_t *src, sfx_data_t *sfx)
{
    stack_t                 *snd;
   
    snd = setup_source(NEW_HANDLE, sfx, IS_3DSOURCE);
    
    if (snd)
    {
     
        /*x = src->pos.x;
        y = src->pos.z;
        z = src->pos.y;

        IDirectSound3DBuffer_SetPosition(source, x, y, z, update_mode);
        IDirectSound3DBuffer_SetVelocity(source, src->pos.momx, src->pos.momz, src->pos.momy, update_mode);
        IDirectSound3DBuffer_SetMode(source, src->head_relative?DS3DMODE_HEADRELATIVE:DS3DMODE_NORMAL, update_mode);*/

        snd->parameters.vPosition.x = src->pos.x;
        snd->parameters.vPosition.y = src->pos.z;
        snd->parameters.vPosition.z = src->pos.y;

        snd->parameters.vVelocity.x = src->pos.momx;
        snd->parameters.vVelocity.y = src->pos.momz;
        snd->parameters.vVelocity.z = src->pos.momy;

        snd->parameters.dwMode = src->head_relative ? DS3DMODE_HEADRELATIVE : DS3DMODE_NORMAL;

        snd->parameters.flMinDistance = src->min_distance;
        snd->parameters.flMaxDistance = src->max_distance;
        //snd->parameters.flMaxDistance = DS3D_DEFAULTMAXDISTANCE;

        snd->permanent = src->permanent;

        UpdateSoundVolume(snd->dsbuffer, src->volume);
        IDirectSound3DBuffer_SetAllParameters(snd->dsbuffer3D, &snd->parameters, DS3D_IMMEDIATE);
        
        IDirectSoundBuffer_SetCurrentPosition(snd->dsbuffer, 0);
            
    }
        
    return (snd - _stack);
}

/******************************************************************************
 *
 * Destroy source and remove it from stack if it is a 2D source.
 * Otherwise put source into cache
 *
 *****************************************************************************/
EXPORT void HWRAPI (KillSource) (int handle)
{

    if (handle < 0 || handle >= allocated_sounds)
        return;
/*
     if (_stack[handle].dsbuffer3D)
    {
        // It's a 3D source so let him chance to be reused :-)
        _stack[handle].LRU = 1;
    }
    else
    { */
        // No, it is a 2D source so kill him
        kill_sound(_stack + handle);
    //}
}


/******************************************************************************
 *
 * Update volume and separation (panning) of 2D source
 *
 *****************************************************************************/
EXPORT void HWRAPI (Update2DSoundParms) (int handle, int vol, int sep)
{
    LPDIRECTSOUNDBUFFER dsbuffer;

    if (handle < 0 || handle >= allocated_sounds)
        return;

    if ((_stack+handle)->dsbuffer3D)
        return;
    dsbuffer = (_stack + handle)->dsbuffer;
    UpdateSoundVolume(dsbuffer, vol);
    Update2DSoundPanning(dsbuffer, sep);
}



// --------------------------------------------------------------------------
// Set the global volume for sound effects
// --------------------------------------------------------------------------
EXPORT void HWRAPI (SetGlobalSfxVolume) (int volume)
{
    int     vol;
    HRESULT hr;
   
    // use the last quarter of volume range
    if (volume)
        vol = recalc_volume(volume, 31);
    else
        vol = DSBVOLUME_MIN;
    
    hr = IDirectSoundBuffer_SetVolume (PrimaryBuffer, vol);
    
}

EXPORT void HWRAPI ( StopSource) (int handle)
{
    LPDIRECTSOUNDBUFFER dsbuffer;
    
    if (handle < 0 || handle >= allocated_sounds)
        return;
    dsbuffer = (_stack + handle)->dsbuffer;

    if (dsbuffer)
    {
        IDirectSoundBuffer_Stop(dsbuffer);
        IDirectSoundBuffer_SetCurrentPosition(dsbuffer, 0);
    }
}

EXPORT int HWRAPI ( GetHW3DSVersion) (void)
{
    return VERSION;
}


EXPORT void HWRAPI (BeginFrameUpdate) (void)
{
    update_mode = DS3D_DEFERRED;
}

EXPORT void HWRAPI (EndFrameUpdate) (void)
{
    if (update_mode == DS3D_DEFERRED)
        IDirectSound3DListener_CommitDeferredSettings(Listener);

    update_mode = DS3D_IMMEDIATE;
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
    D3DVECTOR pos;
    D3DVECTOR face;
    D3DVECTOR head;
    D3DVECTOR velocity;
    
    double      f_angle;
    //double      h_angle, t_angle;
    //double      f_cos, f_sin, t_sin;

    pos.x = data->x;
    pos.y = data->z;
    pos.z = data->y;

    velocity.x = data->momx;
    velocity.y = data->momz;
    velocity.z = data->momy;

    f_angle = (data->f_angle) / 180 * PI;
    //h_angle = data->h_angle / 180 * PI;
    //h_angle = 90 / 180 * PI;
    
    // Treat listener orientation angles as spherical coordinates.
    // x = sin h * cos f
    // y = sin h * sin f
    // z = cos h

    // Front vector
    //t_angle = (PI/2 - h_angle);
    //t_angle = PI/2;
        
    //face.x = (t_sin = sin(t_angle)) * (f_cos = cos(f_angle));
    //face.z = t_sin * (f_sin = sin(f_angle));
    //face.y = cos(t_angle);
    face.x = cos(f_angle);
    face.z = sin(f_angle);
    face.y = 0;

    head.x = 0.0;
    head.y = 1.0;
    head.z = 0.0;
   

    // Top vector
    //h_angle = (-data->h_angle) / 180 * PI;

/*
    t_angle = -h_angle;
    
    head.z = (t_sin = sin(t_angle)) * f_cos;
    head.x = t_sin * f_sin;
    head.y = cos(t_angle);
*/
 
     // Update at once
    memcpy(&listener_parms.vPosition, &pos, sizeof(D3DVECTOR));
    memcpy(&listener_parms.vOrientFront, &face, sizeof(D3DVECTOR));
    memcpy(&listener_parms.vOrientTop, &head, sizeof(D3DVECTOR));
    memcpy(&listener_parms.vVelocity, &velocity, sizeof(D3DVECTOR));
    //IDirectSound3DListener_SetAllParameters(Listener, &listener_parms, DS3D_IMMEDIATE);
    IDirectSound3DListener_SetAllParameters(Listener, &listener_parms, update_mode);
}

EXPORT int HWRAPI (SetCone) (int handle, cone_def_t *cone_def)
{
    stack_t     *snd;
    //DS3DBUFFER  parms;

    if (handle < 0 || handle >= allocated_sounds)
        return -1;

    snd = _stack + handle;

    if (snd->dsbuffer3D)
    {
        
     /*f_angle = cone_def->f_angle / 180 * PI;
        h_angle = (90 - cone_def->h_angle) / 180 * PI;
        parms.vConeOrientation.x = sin(h_angle) * cos(f_angle);
        parms.vConeOrientation.z = sin(h_angle) * sin(f_angle);
        parms.vConeOrientation.y = cos(h_angle);*/

        snd->parameters.dwInsideConeAngle = cone_def->inner;
        snd->parameters.dwOutsideConeAngle = cone_def->outer;
        snd->parameters.lConeOutsideVolume = recalc_volume(cone_def->outer_gain, 256);

        return IDirectSound3DBuffer_SetAllParameters(snd->dsbuffer3D, &snd->parameters, update_mode);
    }

    return -1;
}

EXPORT void HWRAPI (Update3DSource) (int handle, source3D_pos_t *data)
{
    stack_t     *snd;   
    
    if (handle < 0 || handle >= allocated_sounds)
        return;

    snd = _stack + handle;
    if (snd->dsbuffer3D)
    {
    
        /*parms.dwSize = sizeof(DS3DBUFFER);
        IDirectSound3DBuffer_GetAllParameters(snd, &parms);*/

        //angle = data->angle * 180 / PI;

        snd->parameters.vPosition.x = data->x;
        snd->parameters.vPosition.y = data->z;
        snd->parameters.vPosition.z = data->y;

        /*parms.vConeOrientation.x = cos(angle);
        parms.vConeOrientation.z = sin(angle);
        parms.vConeOrientation.y = 0;*/

        snd->parameters.vVelocity.x = data->momx;
        snd->parameters.vVelocity.y = data->momz;
        snd->parameters.vVelocity.z = data->momy;

        //snd->parameters.flMinDistance = data->min_distance;
        //snd->parameters.flMaxDistance = data->max_distance;
        //snd->parameters.dwMode = pos->head_realtive?DS3DMODE_HEADREALTIVE:DS3DMODE_NORMAL;

        IDirectSound3DBuffer_SetAllParameters(snd->dsbuffer3D, &snd->parameters, update_mode);
    }
}


EXPORT int HWRAPI (StartSource) (int handle)
{
    LPDIRECTSOUNDBUFFER snd;

    if (handle < 0 || handle >= allocated_sounds)
        return -1;
   
    snd = (_stack + handle)->dsbuffer;
    IDirectSoundBuffer_SetCurrentPosition(snd, 0);
    return IDirectSoundBuffer_Play(snd, 0, 0, 0);

}

//-------------------------------------------------------------
// Load new sound data into source
//-------------------------------------------------------------
EXPORT int HWRAPI (Reload3DSource) (int handle, sfx_data_t *data)
{
    DS3DBUFFER  temp;
    stack_t     *snd;
    int         perm;

    // DirectX could not load new sound data into source
    // so recreate sound buffers
    if (handle < 0 || handle >= allocated_sounds)
        return -1;

    snd = _stack + handle;
    CopyMemory(&temp, &snd->parameters, sizeof(DS3DBUFFER));
    perm = snd->permanent;

    kill_sound(snd);

    snd = setup_source(handle, data, IS_3DSOURCE);
    /*snd->dsbuffer = create_buffer(data->data, data->length, true);
    if (snd->dsbuffer == NULL)
        return -1;
    snd->dsbuffer3D = create_3dbuffer(snd->dsbuffer, snd->dsbuffer3D);
    if (snd->dsbuffer3D == NULL)
    {
        RELEASE_BUFFER(snd->dsbuffer);
        return -1;
    }

    snd->sfx_id = data->id;
    snd->LRU = 0;*/
    if (snd)
    {
        snd->permanent = perm;
        IDirectSound3DBuffer_SetAllParameters(snd->dsbuffer3D, &temp, DS3D_IMMEDIATE);
        CopyMemory(&snd->parameters, &temp, sizeof(DS3DBUFFER));
    }

    return (snd - _stack);
}

EXPORT void HWRAPI (UpdateSourceVolume) (int handle, int volume)
{
    if (handle < 0 || handle >= allocated_sounds)
        return;
    UpdateSoundVolume((_stack + handle)->dsbuffer, volume);
}