// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: lmptools.cpp,v 1.2 2000/05/02 11:58:51 hurdler Exp $
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
// $Log: lmptools.cpp,v $
// Revision 1.2  2000/05/02 11:58:51  hurdler
// Updated License (GPL)
//
//
//
// DESCRIPTION:
//      set of useful routines for reading/writing additional info in LMPs
//      and playing them back
//
//-----------------------------------------------------------------------------

#include <afxwin.h>

#include <io.h>
#include <fcntl.h>

#include "tools.h"
#include "win_misc.h"
#include "guitools.h"

#include "lmptools.h"

#define LMPHEADERSIZE   (14+28)       //standard header size + 28 players for max.32 players

#define LMPCHUNK_IWAD   (('D'<<24) | ('A'<<16) | ('W'<<8) | 'I')    // IWAD name
#define LMPCHUNK_PWAD   (('D'<<24) | ('A'<<16) | ('W'<<8) | 'P')    // PWAD names
#define LMPCHUNK_COMM   (('M'<<24) | ('M'<<16) | ('O'<<8) | 'C')    // COMMents
#define LMPCHUNK_EXEC   (('C'<<24) | ('E'<<16) | ('X'<<8) | 'E')    // EXE name


// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------
BOOL    LMP_ReadInfo (char* demofile, lmpshowinfo_t* lmpinfo)
{
    int     fh, bytes;
    char    buffer[LMPHEADERSIZE];

    int     i,j;
    int     version, skill, episode, map, leveltimer, players;
    int     respawn, fast, nomonsters;
    char    *mode;

    char    quickinfo[4096];

    lmpextrainfo_t* extrainfo;

static  char*   notavail = "Undefined";
static  char*   single = "single player";
static  char*   coop = "cooperative";
static  char*   olddedm = "old deathmatch";
static  char*   dedm20 = "deathmatch 2";

    
    // open file
    if( (fh = _open(demofile, _O_RDONLY | _O_BINARY)) == -1 )
    {
        ErrorBox (va("File Error: Unable to open %s.",demofile));
        return FALSE;
    }
    
    // read header
    bytes = _read(fh, buffer, LMPHEADERSIZE);
    if ( bytes == -1 || bytes!=LMPHEADERSIZE)
    {
        ErrorBox (va("File Error: Unable to read from %s.",demofile));
        _close(fh);
        return FALSE;
    }

    // look for extra info saved by Doom Legacy Launcher, pass the file handle
    extrainfo = LMP_ReadExtraInfo (NULL, fh);

    // close file
    _close (fh);

    //
    // decode header
    //
    version = (int)( buffer[0] & 0xff );

    // DOOM1 until v1.2
    if (version < 5)
    {
        version = 0;
	skill = buffer[0] + 1;
	episode = buffer[1];
	map = buffer[2];
	respawn = fast = nomonsters = 0;    // undefined
        leveltimer = 0;

        for (i=3,players=0; i<7; i++)
            if (buffer[i])
                players++;
    }
    else
    {
        skill = buffer[1] + 1;
        episode = buffer[2];
        map = buffer[3];

        respawn = buffer[5];
        fast    = buffer[6];
        nomonsters = buffer[7];

        leveltimer = 0;
        i = 9;                  // start of player presence flags
        if (version<=109)
            j = i + 4;          // 4 players
        else {
            leveltimer = buffer[i++];       // Doom Legacy levelTimer
            if (version < 113)
                j = i + 8;                  // 8 players before v1.13
            else
                j = i + 32;                 // 32 players since v1.13
        }
        
        // count how many players are present
        for (players=0; i<j; i++)
            if (buffer[i])
                players++;
    }

    // determine game mode
    mode = single;
    if (version==0 && players>1)
        mode = notavail;    // either coop or deathmatch for Doom1 pre v1.2
    else
        // after 1.27 deathmatch is a cvar
        if (version<127 && players > 1)
        {
            switch (buffer[4])
            {
            case 0: mode = coop;    break;
            case 1: mode = olddedm; break;
            case 2: mode = dedm20;  break;
            }
        }
        else if (version>=127)
            mode = notavail;
    

    //
    // show information
    //
    if (lmpinfo->nQuickInfoID)
    {
        CEdit* txt = (CEdit*) lmpinfo->cwnd->GetDlgItem (lmpinfo->nQuickInfoID);

        if (version) {
            strcpy (quickinfo, va("DEMO VERSION %d.%d", version/100, version%100) );
            if ((version%100) >= 4 && (version%100) <= 6)
                strcat (quickinfo, " beta");
            if ((version%100) == 6)
		strcat (quickinfo, " or v1.666");
            strcat (quickinfo, "\r\n");
        }
        else
            strcpy (quickinfo, "Doom1 until v1.2\r\n");

        strcat (quickinfo, va("SKILL %d  ", skill) );
        
        if (episode>1)
            strcat (quickinfo, va("EPISODE %d  ", episode) );
        strcat (quickinfo, va("MAP %d\r\n", map) );

        if (version)
        {
            if (respawn)
                strcat (quickinfo, "+RESPAWNMONSTERS\r\n" );
            if (fast)
                strcat (quickinfo, "+FASTMONSTERS\r\n" );
            if (nomonsters)
                strcat (quickinfo, "+NOMONSTERS\r\n" );

            if (leveltimer)
                strcat (quickinfo, va("LEVELTIMER %d  ", leveltimer) );
        }

        strcat (quickinfo, va("%d PLAYER(S)\r\n", players) );

        // extra info if the demo was recorded through Doom Legacy Launcher
        if (extrainfo)
        {
            if (extrainfo->exe)
                strcat (quickinfo, va("EXE: %s  ",extrainfo->exe) );
            if (extrainfo->iwad)
                strcat (quickinfo, va("IWAD: %s",extrainfo->iwad) );
            if (extrainfo->pwads)
                strcat (quickinfo, va("\r\nPWADS: %s",extrainfo->pwads) );
            if (extrainfo->comments)
                strcat (quickinfo, va("\r\nCOMMENTS: %s\r\n",extrainfo->comments) );
            
            // we don't need it anymore
            LMP_FreeExtraInfo (extrainfo);
        }

        txt->SetWindowText (quickinfo);
    }
    
    return TRUE;
}


// --------------------------------------------------------------------------
// Frees the buffers allocated in lmpextrainfo, when they are no more needed
// --------------------------------------------------------------------------
void LMP_FreeExtraInfo (lmpextrainfo_t* extra)
{
    if (extra->exe)
        free (extra->exe);
    if (extra->iwad)
        free (extra->iwad);
    if (extra->pwads)
        free (extra->pwads);
    if (extra->comments)
        free (extra->comments);
    
    extra->exe = NULL;
    extra->iwad = NULL;
    extra->pwads = NULL;
    extra->comments = NULL;
}


// --------------------------------------------------------------------------
// Read extra info appended at the end of a LMP file
// - if filename is NULL, you must pass the file handle
// --------------------------------------------------------------------------
lmpextrainfo_t* LMP_ReadExtraInfo (char* demofile, int handle)
{
    lmpextrachunk_t chunk;
static    lmpextrainfo_t  extra;
    BOOL            myhandle = false;
    long            offset;
    char*           buffer = NULL;
    char**          ptoextra;

    ZeroMemory (&extra, sizeof(extra));

    // open the file
    if (demofile!=NULL) {
        if( (handle = _open(demofile, _O_RDONLY | _O_BINARY)) == -1 )
        {
            ErrorBox (va("File Error: Unable to open %s for reading.",demofile));
            return NULL;
        }
        myhandle = true;
    }
    else
        demofile = "demo file"; // in case it is used in error message, and handle was used

    // read chunks backwards, starting at end of file
    offset = _lseek (handle, -(signed)sizeof(chunk), SEEK_END);
    if (offset == -1)
        goto error;

    while (1)
    {
        // read chunk marker
        if (_read (handle, &chunk, sizeof(chunk)) != sizeof(chunk))
            goto error;

        switch (chunk.chunkID)
        {
            case LMPCHUNK_IWAD:     ptoextra = &extra.iwad; break;
            case LMPCHUNK_PWAD:     ptoextra = &extra.pwads; break;
            case LMPCHUNK_COMM:     ptoextra = &extra.comments; break;
            case LMPCHUNK_EXEC:     ptoextra = &extra.exe; break;
            default:
                ptoextra = NULL;
                break;
        }

        // unknown data, no more chunks
        if (ptoextra == NULL)
            break;
        
        // read the known chunk data
        offset = _lseek (handle, offset - chunk.chunksize, SEEK_SET);
        if (offset==-1)
            goto error;
        buffer = (char*) malloc (chunk.chunksize+1);
        if (buffer==NULL)
            goto error;
        buffer[chunk.chunksize] = '\0';
        if (_read (handle, buffer, chunk.chunksize) != chunk.chunksize)
            goto error;
        *ptoextra = buffer;
        buffer = NULL;

        // position to read next chunk, backwards from last chunk data
        offset = _lseek (handle, offset - sizeof(chunk), SEEK_SET);
        if (offset == -1)
            goto error;
    }

finished:
    if (buffer)
        free (buffer);
    if (myhandle)
        _close (handle);

    return &extra;

error:
    ErrorBox (va("File Error: Could not retrieve extra lmp info from %s.", demofile));
    goto finished;
}


// --------------------------------------------------------------------------
// Write a chunk marker and its data, make sure you are at the END of the
// file before calling this, the file pointer will advance after the written
// chunk
// --------------------------------------------------------------------------
BOOL LMP_WriteExtraChunk (int handle, int chunkID, char* chunkdata)
{
    lmpextrachunk_t chunky;

    chunky.chunkID = chunkID;
    chunky.chunksize = strlen(chunkdata);
    
    // write data
    if (_write (handle, chunkdata, chunky.chunksize) == -1)
    {
        ErrorBox ("LMP_WriteExtraChunk 1 FAILED");
        return FALSE;
    }

    // write chunk marker
    if (_write (handle, &chunky, sizeof(chunky)) == -1)
    {
        ErrorBox ("LMP_WriteExtraChunk 2 FAILED");
        return FALSE;
    }

    return TRUE;
}


// --------------------------------------------------------------------------
// Write extra info at the end of a lmp file, if extra info was already
// present, the chunks are truncated, and rewritten.
// --------------------------------------------------------------------------
BOOL LMP_WriteExtraInfo (lmpextrainfo_t* extra, char* demofile)
{
    int     handle;
    long    offset;
    lmpextrachunk_t chunk;

    if ( (handle = _open(demofile, _O_RDWR | _O_BINARY)) == -1 )
    {
        ErrorBox (va("File Error: Unable to open %s for writing.",demofile));
        return FALSE;
    }
    
    //
    // locate existing appended chunks, truncate em out of the file
    //
    offset = _lseek (handle, -(signed)sizeof(chunk), SEEK_END);
    while (offset > 0)
    {
        // read chunk marker
        if (_read (handle, &chunk, sizeof(chunk)) != sizeof(chunk)) {
            ErrorBox ("shit happens 1");
            goto error;
        }

        if (chunk.chunkID != LMPCHUNK_IWAD &&
            chunk.chunkID != LMPCHUNK_PWAD &&
            chunk.chunkID != LMPCHUNK_COMM &&
            chunk.chunkID != LMPCHUNK_EXEC )
            break;

        // go backwards this chunk data, to start of next chunk marker
        offset -= (chunk.chunksize + sizeof(chunk));
        offset = _lseek (handle, offset, SEEK_SET);
    }

    if (offset < 0)
        goto error;

    offset += sizeof(chunk);

    // truncate the extra info
    if (_chsize (handle, offset) == -1) {
            ErrorBox ("shit happens 2");
        goto error;
    }


    //
    // append the new extra info to the file
    //
    offset = _lseek (handle, 0, SEEK_END);
    if (offset == -1) {
                    ErrorBox ("shit happens 3");

        goto error;
    }
    if (extra->exe &&
        !LMP_WriteExtraChunk (handle, LMPCHUNK_EXEC, extra->exe))
        goto error;
    if (extra->iwad &&
        !LMP_WriteExtraChunk (handle, LMPCHUNK_IWAD, extra->iwad))
        goto error;
    if (extra->pwads &&
        !LMP_WriteExtraChunk (handle, LMPCHUNK_PWAD, extra->pwads))
        goto error;
    if (extra->comments &&
        !LMP_WriteExtraChunk (handle, LMPCHUNK_COMM, extra->comments))
        goto error;
    
    _close (handle);
    return TRUE;

error:
    ErrorBox (va("Error: could not write extra info to file %s.",demofile));
    _close (handle);
    return FALSE;
}
