// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: mid2strm.h,v 1.2 2000/02/27 00:42:12 hurdler Exp $
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
// $Log: mid2strm.h,v $
// Revision 1.2  2000/02/27 00:42:12  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Midi 2 Stream
//
//-----------------------------------------------------------------------------


#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>


// undefine this to remove warning messages
//#define DEBUGMIDISTREAM

#define OUT_BUFFER_SIZE             1024 // Max stream buffer size in bytes
#define BUFFER_TIME_LENGTH                  60   // Amount to fill in milliseconds
#define NUM_STREAM_BUFFERS          2

#define CONVERTERR_NOERROR              0               // No error occured
#define CONVERTF_STATUS_DONE            0x00000001

#define CONVERTERR_DONE                 -103        // Converter is done
#define CONVERTERR_BUFFERFULL       -104            // The buffer is full

#define CONVERTF_RESET                      0x00000001
#define CONVERTF_STATUS_STUCK           0x00000002
#define CONVERTF_STATUS_GOTEVENT        0x00000004
#define CONVERTERR_CORRUPT              -101        // The input file is corrupt

// The converter has already encountered a corrupt file and cannot convert any
// more of this file -- you must reset the converter
#define CONVERTERR_STUCK                -102
#define CONVERTERR_METASKIP             -105        // Skipping unknown meta event

#define MAX_MIDI_IN_TRACKS          16

// MIDI file constants
//
#define MThd            0x6468544D                      // Start of file
#define MTrk            0x6B72544D                      // Start of track

#define MIDI_SYSEX      ((BYTE)0xF0)                    // SysEx begin
#define MIDI_SYSEXEND   ((BYTE)0xF7)                    // SysEx begin
#define MIDI_META       ((BYTE)0xFF)                    // Meta event begin
#define MIDI_META_TEMPO ((BYTE)0x51)
#define MIDI_META_EOT   ((BYTE)0x2F)                    // End-of-track

#define MIDI_NOTEOFF    ((BYTE)0x80)                    // + note + velocity
#define MIDI_NOTEON     ((BYTE)0x90)                    // + note + velocity
#define MIDI_POLYPRESS  ((BYTE)0xA0)                    // + pressure (2 UBYTEs)
#define MIDI_CTRLCHANGE ((BYTE)0xB0)                    // + ctrlr + value
#define MIDI_PRGMCHANGE ((BYTE)0xC0)                    // + new patch
#define MIDI_CHANPRESS  ((BYTE)0xD0)                    // + pressure (1 UBYTE)
#define MIDI_PITCHBEND  ((BYTE)0xE0)                    // + pitch bend (2 UBYTEs)

// used by MIDI stream callback
//
#define MIDICTRL_VOLUME             ((BYTE)0x07)
#define MIDICTRL_VOLUME_LSB         ((BYTE)0x27)
#define MIDICTRL_PAN                ((BYTE)0x0A)
#define MIDIEVENT_CHANNEL(dw)   (dw & 0x0000000F)
#define MIDIEVENT_TYPE(dw)          (dw & 0x000000F0)
#define MIDIEVENT_DATA1(dw)         ((dw & 0x0000FF00) >> 8)
#define MIDIEVENT_VOLUME(dw)    ((dw & 0x007F0000) >> 16)


typedef struct
{
    LONG               fdwTrack;               // Track status
    LONG               iTrackLen;              // Total UBYTEs in track
    LONG               iBytesLeft;             // Bytes left unread in track
    unsigned char*     pTrackData;             // -> start of track data
    unsigned char*     pTrackPointer;          // -> next UBYTE to read
    DWORD              tkNextEventDue;         // Absolute time of next event in track
    BYTE               bRunningStatus;         // Running status from last channel msg
#ifdef DEBUGMIDISTREAM
    LONG               nTrack;                 // # of this track for debugging
#endif
} INTRACKSTATE, *PINTRACKSTATE;


// Description of the input MIDI file
//
typedef struct
{
    int                FileSize;               // Total UBYTEs in file
    unsigned char*     pFile;                  // -> entire file in memory
    int                iBytesLeft;             // Bytes left unread
    unsigned char*     pFilePointer;           // -> next UBYTE to read
    
    int                dwTimeDivision;         // Original time division
    int                dwFormat;               // Original format
    unsigned long      nTracks;                // Track count (specifies pTracks size)
    //faB: made it static
    INTRACKSTATE       pTracks[MAX_MIDI_IN_TRACKS];   // -> array of tracks in this file
} INFILESTATE;

// This is shared between the mid2strm.c conversion module,
// and the midi playback code in win_snd.c
typedef struct _ConvertInfo
{
    MIDIHDR     mhBuffer;               // Standard Windows stream buffer header
    DWORD       dwStartOffset;      // Start offset from mhStreamBuffer.lpStart
    DWORD       dwMaxLength;        // Max length to convert on this pass

    DWORD       dwBytesRecorded;    // Used internally by the mid2strm.c module
    DWORD       tkStart;                // Used internally by the mid2strm.c module
    BOOL        bTimesUp;               // Used internally by the mid2strm.c module
} CONVERTINFO, *LPCONVERTINFO;

// mid2strm.c
extern  INFILESTATE      ifs;
extern  DWORD   dwProgressBytes;

void    Mid2StreamConverterCleanup (void);
BOOL    Mid2StreamConverterInit( unsigned char* pMidiData, ULONG iMidiSize );
int     Mid2StreamConvertToBuffer( DWORD dwFlags, LPCONVERTINFO lpciInfo );
