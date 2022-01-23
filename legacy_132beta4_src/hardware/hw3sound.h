// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: hw3sound.h,v 1.2 2001/05/27 13:42:48 bpereira Exp $
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
// $Log: hw3sound.h,v $
// Revision 1.2  2001/05/27 13:42:48  bpereira
// no message
//
// Revision 1.1  2001/04/04 19:41:16  judgecutor
// Initial release of 3D Sound Support
//
//
//
// DESCRIPTION:
//      High-level functions of hardware 3D sound
//
//-----------------------------------------------------------------------------


#ifndef __HW3_SOUND_H__
#define __HW3_SOUND_H__

#ifdef HW3SOUND
#include "hw3dsdrv.h"
//#include "../s_sound.h"
//#include "../p_mobj.h"

// Default sound mode (original stereo mode)
enum { HWS_DEFAULT_MODE = 0 };

extern int  hws_mode;           // Current sound mode

extern int  HW3S_Init(I_Error_t FatalErrorFunction, snddev_t *snd_dev);
extern void HW3S_Shutdown(void);
extern int  HW3S_GetVersion(void);

// General case - start 3D or 2D source
extern void HW3S_StartSound(void *origin, int sfx_id);

// Special cases of 3D sources
extern void S_StartAmbientSound(int sfx_id, int volume);
extern void S_StartAttackSound(void *origin, int sfx_id);
extern void S_StartScreamSound(void *origin, int sfx_id);

extern void HW3S_StopSound(void *origin);
extern void HW3S_StopSounds(void);

extern void HW3S_BeginFrameUpdate();
extern void HW3S_EndFrameUpdate();
//extern void HW3S_UpdateListener(mobj_t *listener);
extern void HW3S_UpdateSources(void);

extern void HW3S_SetSfxVolume(int volume);

// Utility functions
extern int  HW3S_SoundIsPlaying(int handle);
extern void HW3S_SetSourcesNum(void);
extern int  HW3S_SoundPlaying(void *origin, int id);

#else // HW3SOUND

#define S_StartAmbientSound(i,v) S_StartSoundAtVolume(NULL,i,v)
#define S_StartAttackSound  S_StartSound
#define S_StartScreamSound  S_StartSound

#endif // HW3SOUND


#endif // __HW3_SOUND_H__