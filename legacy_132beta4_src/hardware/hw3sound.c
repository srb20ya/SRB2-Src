// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: hw3sound.c,v 1.4 2001/09/10 19:46:10 judgecutor Exp $
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
// $Log: hw3sound.c,v $
// Revision 1.4  2001/09/10 19:46:10  judgecutor
// Minor bug fixed (sound volume of static sources)
//
// Revision 1.3  2001/08/21 21:51:00  judgecutor
// Added "static" sources
//
// Revision 1.2  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.1  2001/04/04 19:41:16  judgecutor
// Initial release of 3D Sound Support
//
// 
//
// DESCRIPTION:
//      Hardware 3D sound general code
//      
//
//-----------------------------------------------------------------------------

#include "../doomdef.h"
#include "hw3sound.h"
#include "../i_sound.h"
#include "../s_sound.h"
#include "../w_wad.h"

#include "../z_zone.h"
#include "../g_game.h"
#include "../tables.h"
#include "../sounds.h"
#include "../r_main.h" 
#include "../r_things.h"

#define ANGLE2DEG(x) (((double)(x)) / ((double)ANG45/45))


#define MIN_DISTANCE 160.0f
#define MAX_DISTANCE 1200.0f

struct hardware3ds_s hw3ds_driver;


typedef enum 
{
    CT_NORMAL = 0,
    CT_ATTACK,
    CT_SCREAM,
    CT_AMBIENT
} channel_type_t;


typedef struct source_s
{
    sfxinfo_t       *sfxinfo;
    void            *origin;
    int             handle;     // Internal source handle
    channel_type_t  type;       // Sound type (attack, scream, etc)
} source_t;


typedef struct ambient_sdata_s
{
    source3D_data_t left;
    source3D_data_t right;
} ambient_sdata_t;


typedef struct ambient_source_s
{
    source_t    left;
    source_t    right;
} ambient_source_t;

// Static sources
// This sources always creates
static source_t p_attack_source;        // Player attack source
static source_t p_scream_source;        // Player scream source

// abuse?
#define STATIC_SOURCES_NUM      4       // Total number of static sources

static ambient_source_t ambient_source; // Ambinet sound sources

//  abuse???
//static source3D_data_t  p_attack_sdata; // ?? Now just holds precalculated player attack source data
//static source3D_data_t  p_default_sdata;// ?? ---- // ---- // ---- // ---- player scream source data

static ambient_sdata_t  ambient_sdata;  // Precalculated datas of an ambient sound sources

// Stack of dynamic sources
static source_t      *sources       = NULL;     // Much like original channels
static int           num_sources    = 0;

// Current mode of 3D sound system
// Default is original (stereo) mode
int                  hws_mode       = HWS_DEFAULT_MODE;



//=============================================================================
void HW3S_SetSourcesNum()
{
    int i;

    // Allocating the internal channels for mixing
    // (the maximum number of sounds rendered
    // simultaneously) within zone memory.
    if (sources)
    {
        HW3S_StopSounds();
        Z_Free(sources);
    }

    if (cv_numChannels.value <= STATIC_SOURCES_NUM)
        I_Error("HW3S_SetSourcesNum: Number of sound sources cannot be less than %d\n", STATIC_SOURCES_NUM + 1);

    num_sources = cv_numChannels.value - STATIC_SOURCES_NUM;

    sources =
      (source_t*) Z_Malloc(num_sources * sizeof(source_t), PU_STATIC, 0);

    // Free all channels for use
    for (i=0 ; i < num_sources; i++)
    {
        sources[i].sfxinfo = NULL;
        sources[i].type = CT_NORMAL;
        sources[i].handle = -1;
    }
}


//=============================================================================
static void HW3S_KillSource(int snum)
{
    source_t*   s = &sources[snum];

    if (s->sfxinfo)
    {
        HW3DS.pfnStopSource(s->handle);
        HW3DS.pfnKillSource(s->handle);
        s->handle = -1;
        s->sfxinfo->usefulness--;
        s->origin = NULL;
        s->sfxinfo = NULL;
    }
}


//=============================================================================
static void HW3S_StopSource(int snum)
{
    source_t*  s = &sources[snum];

    if (s->sfxinfo)
    {
        // stop the sound playing
        HW3DS.pfnStopSource(s->handle);
    }
}


//=============================================================================
void HW3S_StopSound(void *origin)
{
    int snum;

    for (snum=0 ; snum < num_sources ; snum++)
    {
        if (sources[snum].sfxinfo && sources[snum].origin == origin)
        {
            HW3S_KillSource(snum);
            break;
        }
    }
}


//=============================================================================
void HW3S_StopSounds()
{
    int snum;

    // kill all playing sounds at start of level
    //  (trust me - a good idea)
    for (snum = 0 ; snum < num_sources; snum++)
        if (sources[snum].sfxinfo)
            HW3S_KillSource(snum);

    // Also stop all static sources
    HW3DS.pfnStopSource(p_attack_source.handle);
    HW3DS.pfnStopSource(p_scream_source.handle);
    HW3DS.pfnStopSource(ambient_source.left.handle);
    HW3DS.pfnStopSource(ambient_source.right.handle);
}


//=============================================================================
static int HW3S_GetSource( void* origin, sfxinfo_t* sfxinfo )
{
    //
    //   If none available, return -1.  Otherwise source #.
    //   source number to use

    int         snum;
    source_t*   src;

    // Find an open source
    for (snum = 0, src = sources ; snum < num_sources; src++, snum++)
    {
        if (!src->sfxinfo)
            break;

        if (origin && src->origin ==  origin)
        {
            HW3S_KillSource(snum);
            break;
        }
    }

    // None available
    if (snum == num_sources)
    {
        // Look for lower priority
        for (snum = 0, src = sources ; snum < num_sources ; src++, snum++)
            if (src->sfxinfo->priority >= sfxinfo->priority) 
                break;

        if (snum == num_sources)
        {
            // FUCK!  No lower priority.  Sorry, Charlie.
            return -1;
        }
        else
        {
            // Otherwise, kick out lower priority
            
            HW3S_KillSource(snum);
        }
    }
    
    return snum;
}


//=============================================================================
static void HW3S_FillSourceParameters(mobj_t           *origin, 
                               source3D_data_t  *data, 
                               channel_type_t   c_type)
{
    fixed_t an;
    fixed_t x, y, z;
    
    if (origin != players[displayplayer].mo)
    {
        ZeroMemory(data, sizeof(source3D_data_t));

        data->max_distance = MAX_DISTANCE;
        data->min_distance = MIN_DISTANCE;
        data->volume = 255;
        data->pos.momx = FIXED_TO_FLOAT(origin->momx);
        data->pos.momy = FIXED_TO_FLOAT(origin->momy);
        data->pos.momz = FIXED_TO_FLOAT(origin->momz);

        x = origin->x;
        y = origin->y;
        z = origin->z;

        if (c_type == CT_ATTACK)
        {
            an = origin->angle >> ANGLETOFINESHIFT;
            x += FixedMul(16*FRACUNIT, finecosine[an]);
            y += FixedMul(16*FRACUNIT, finesine[an]);
            z += origin->height >> 1;
        }

        else if (c_type == CT_SCREAM)
            z += origin->height - (5 * FRACUNIT);

        data->pos.x = FIXED_TO_FLOAT(x);
        data->pos.y = FIXED_TO_FLOAT(y);
        data->pos.z = FIXED_TO_FLOAT(z);
                    
    }
            
}


#define HEADER_SIZE     8
//==============================================================
static void make_outphase_sfx(void *dest, void *src, int size)
{
    signed char    *s = (signed char*)src + HEADER_SIZE, *d = (signed char*)dest + HEADER_SIZE;

    CopyMemory(dest, src, HEADER_SIZE);
    size -= HEADER_SIZE;

    while (size--)
        *d++ = -(*s++);
}

//=============================================================================
static void HW3S_StartSoundTypeAtVolume(void *origin_p, int sfx_id, channel_type_t c_type, int volume)
{
    
    sfxinfo_t       *sfx;
    mobj_t          *origin = (mobj_t*)origin_p;
    source3D_data_t source3d_data;
    source2D_data_t source2d_data;
    sfx_data_t      sfx_data;
    //int             volume  = 255;
    int             s_num;
    source_t        *source;

    if(nosound || (origin && origin->type == MT_SPIRIT))
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
      //pitch = sfx->pitch;
      //priority = sfx->priority;
      volume += sfx->volume;

      if (volume < 1)
          return;

      sfx->data = sfx->link->data;

    }
   
    if (!sfx->data)
    {
        if (!sfx->link)
            sfx->data = I_GetSfx (sfx);
        else
        {
            sfx->data = I_GetSfx (sfx->link);
            sfx->link->data = sfx->data;
        }
    }

    sfx_data.data = sfx->data;
    sfx_data.id = sfx_id;
    //sfx_data.length = W_LumpLength(sfx->lumpnum);
    sfx_data.length = *((unsigned short*) sfx->data + 2) + 4 * sizeof(unsigned short);
    sfx_data.priority = sfx->priority;
        
    if (origin && origin == players[displayplayer].mo)
    {

        if (c_type == CT_ATTACK)
            source = &p_attack_source;
        else
            source = &p_scream_source;

        

        if (source->sfxinfo != sfx)
        {
            HW3DS.pfnStopSource(source->handle);
            source->handle = HW3DS.pfnReload3DSource(source->handle, &sfx_data);
            //CONS_Printf("PlayerSound data reloaded\n");
        }
    }
    else if (c_type == CT_AMBIENT)
    {
//        sfx_data_t  outphased_sfx;

        if (ambient_source.left.sfxinfo != sfx)
        {
            HW3DS.pfnStopSource(ambient_source.left.handle);
            HW3DS.pfnStopSource(ambient_source.right.handle);

            // judgecutor:
            // Outphased sfx's temporary not used!!!
/*
                outphased_sfx.data = Z_Malloc(sfx_data.length, PU_STATIC, 0);
                make_outphase_sfx(outphased_sfx.data, sfx_data.data, sfx_data.length);
                outphased_sfx.length = sfx_data.length;
                outphased_sfx.id = sfx_data.id;
*/            
            ambient_source.left.handle = HW3DS.pfnReload3DSource(ambient_source.left.handle, &sfx_data);
//              ambient_source.right.handle = HW3DS.pfnReload3DSource(ambient_source.right.handle, &outphased_sfx);
            ambient_source.right.handle = HW3DS.pfnReload3DSource(ambient_source.right.handle, &sfx_data);
            ambient_source.left.sfxinfo = ambient_source.right.sfxinfo = sfx;
//              Z_Free(outphased_sfx.data);
        }

        HW3DS.pfnUpdateSourceVolume(ambient_source.left.handle, volume);
        HW3DS.pfnUpdateSourceVolume(ambient_source.right.handle, volume);

        if (sfx->usefulness++ < 0)
            sfx->usefulness = -1;

        HW3DS.pfnStartSource(ambient_source.left.handle);
        HW3DS.pfnStartSource(ambient_source.right.handle);

        // Ambient sound is special case
        return;
    } 
    else
    {
        s_num = HW3S_GetSource(origin, sfx);

        if (s_num  < 0)
        {
            //CONS_Printf("No free source, aborting\n");
            return;
        }

        source = &sources[s_num];

        if (origin)
        {
            // It is a positional sound
            HW3S_FillSourceParameters(origin, &source3d_data, c_type);
            source3d_data.volume = volume;
            
            source->handle = HW3DS.pfnAdd3DSource(&source3d_data, &sfx_data);
        
        }
        else
        {
            // It is just an 2D sound
            source2d_data.volume = volume;
            source2d_data.sep = 128;
            source->handle = HW3DS.pfnAdd2DSource(&source2d_data, &sfx_data);
            //CONS_Printf("2D sound created (handle %d)\n", source->handle);
        }
    }
    
     // increase the usefulness
    if (sfx->usefulness++ < 0)
        sfx->usefulness = -1;

    source->sfxinfo = sfx;
    source->origin = origin;
    HW3DS.pfnStartSource(source->handle);

}


// Start normal sound
//=============================================================================
void HW3S_StartSound(void *origin, int sfx_id)
{
    HW3S_StartSoundTypeAtVolume(origin, sfx_id, CT_NORMAL, 255);
}


//=============================================================================
void S_StartAttackSound(void *origin, int sfx_id)
{
    if (hws_mode != HWS_DEFAULT_MODE)
        HW3S_StartSoundTypeAtVolume(origin, sfx_id, CT_ATTACK, 255);
    else
        S_StartSound(origin, sfx_id);
}

void S_StartScreamSound(void *origin, int sfx_id)
{   
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        HW3S_StartSoundTypeAtVolume(origin, sfx_id, CT_SCREAM, 255);
    }
    else
        S_StartSound(origin, sfx_id);
}

void S_StartAmbientSound(int sfx_id, int volume)
{
    if (hws_mode != HWS_DEFAULT_MODE)
    {
        //CONS_Printf("Ambient at %d\n", volume);
        volume += 30;
        if (volume > 255)
            volume = 255;
        //CONS_Printf("Ambient amplified to %d\n", volume);
        HW3S_StartSoundTypeAtVolume(NULL, sfx_id, CT_AMBIENT, volume);
    }
    else
        S_StartSoundAtVolume(NULL, sfx_id, volume);
}



//=============================================================================
BOOL HW3S_Init(I_Error_t FatalErrorFunction, snddev_t *snd_dev)
{
    int                succ;
    int                an;
    source3D_data_t    source_data; 
  
    if (HW3DS.pfnStartup(FatalErrorFunction, snd_dev))
    {
       
        // Creating player sources
        ZeroMemory(&source_data, sizeof(source_data));
        
        // Attack source
        source_data.head_relative = 1;
        source_data.pos.y = 16;
        source_data.pos.z = -FIXED_TO_FLOAT(mobjinfo[MT_PLAYER].height >> 1);
        source_data.min_distance = MIN_DISTANCE;
        source_data.max_distance = MAX_DISTANCE;
        source_data.permanent = 1;
        source_data.volume = 255;

        p_attack_source.sfxinfo = NULL;

        p_attack_source.handle = HW3DS.pfnAdd3DSource(&source_data, NULL);

        // Scream source
        source_data.pos.y = 0;
        source_data.pos.z = 0;

        p_scream_source.sfxinfo = NULL;

        p_scream_source.handle = HW3DS.pfnAdd3DSource(&source_data, NULL);

        //FIXED_TO_FLOAT(mobjinfo[MT_PLAYER].height - (5 * FRACUNIT));
        
        // Ambient sources (left and right) at 210 and 330 degree
        // relative to listener
        ZeroMemory(&ambient_sdata, sizeof(ambient_sdata));

        an = (ANG180 + ANG90/3) >> ANGLETOFINESHIFT;

        ambient_sdata.left.head_relative = 1;
        ambient_sdata.left.pos.x 
            = FIXED_TO_FLOAT(FixedMul((MIN_DISTANCE * FRACUNIT), finecosine[an]));
        ambient_sdata.left.pos.y 
            = FIXED_TO_FLOAT(FixedMul((MIN_DISTANCE * FRACUNIT), finesine[an]));
        ambient_sdata.left.max_distance = MAX_DISTANCE;
        ambient_sdata.left.min_distance = MIN_DISTANCE;
        ambient_sdata.left.volume = 255;
        ambient_sdata.left.permanent = 1;

        CopyMemory(&ambient_sdata.right, &ambient_sdata.left, sizeof(source3D_data_t));

        ambient_sdata.right.pos.x = -ambient_sdata.left.pos.x;
        ambient_source.left.handle = HW3DS.pfnAdd3DSource(&ambient_sdata.left, NULL);
        ambient_source.right.handle = HW3DS.pfnAdd3DSource(&ambient_sdata.right, NULL);

        succ = p_attack_source.handle > -1 && p_scream_source.handle > -1 
            && ambient_source.left.handle > -1 && ambient_source.right.handle > -1;

        //CONS_Printf("Player handles: attack %d, default %d\n", p_attack_source.handle, p_scream_source.handle);
        return succ;
    }
    return 0;
}




//=============================================================================
int HW3S_GetVersion(void)
{
    return HW3DS.pfnGetHW3DSVersion();
}


//=============================================================================
void HW3S_BeginFrameUpdate(void)
{
    if (hws_mode != HWS_DEFAULT_MODE)
        HW3DS.pfnBeginFrameUpdate();
}


//=============================================================================
void HW3S_EndFrameUpdate(void)
{
    if (hws_mode != HWS_DEFAULT_MODE)
        HW3DS.pfnEndFrameUpdate();
}



//=============================================================================
int HW3S_SoundIsPlaying(int handle)
{
    return HW3DS.pfnIsPlaying(handle);
}

int HW3S_SoundPlaying(void *origin, int id)
{
    int         snum;

    for (snum=0 ; snum<num_sources; snum++)
    {
        if (origin &&  sources[snum].origin ==  origin)
          return 1;
        if (id != -1 && sources[snum].sfxinfo - S_sfx == id)
          return 1;
    }
    return 0;
}

//=============================================================================
void HW3S_UpdateListener(mobj_t *listener)
{
    listener_data_t data;

    if (!(listener && listener->player))
        return;

    data.x = FIXED_TO_FLOAT(listener->x);
    data.y = FIXED_TO_FLOAT(listener->y);
    data.z = FIXED_TO_FLOAT(listener->z + listener->height - (5 * FRACUNIT));
    
    data.f_angle = ANGLE2DEG(listener->angle);
    data.h_angle = ANGLE2DEG(listener->player->aiming);

    data.momx = FIXED_TO_FLOAT(listener->momx);
    data.momy = FIXED_TO_FLOAT(listener->momy);
    data.momz = FIXED_TO_FLOAT(listener->momz);
    
    HW3DS.pfnUpdateListener(&data);
}

void HW3S_SetSfxVolume(int volume)
{
    HW3DS.pfnSetGlobalSfxVolume(volume);
}


static void HW3S_Update3DSource(source_t *src)
{
    source3D_data_t data;

    HW3S_FillSourceParameters(src->origin, &data, src->type);
    HW3DS.pfnUpdate3DSource(src->handle, &data.pos);

}

void HW3S_UpdateSources(void)
{
    mobj_t      *listener = players[displayplayer].mo;
    source_t    *src;
    int         snum;

    HW3S_UpdateListener(listener);

    for (snum = 0, src = sources; snum < num_sources; src++, snum++)
    {
        if (src->sfxinfo)
        {
            if (HW3DS.pfnIsPlaying(src->handle))
            {
                if (src->origin)
                {
                    // Update positional sources
                    HW3S_Update3DSource(src);
                }
            }
            else
            {
                // Source allocated but stopped. Kill.
                HW3S_KillSource(snum);
            }
        }
    }
    
}


void HW3S_Shutdown()
{
    HW3DS.pfnShutdown();
}

