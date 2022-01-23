// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: stats.cpp,v 1.5 2000/10/23 17:47:57 hurdler Exp $
//
// Copyright (C) 2000 by DooM Legacy Team.
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
// $Log: stats.cpp,v $
// Revision 1.5  2000/10/23 17:47:57  hurdler
// Update for version info
//
// Revision 1.4  2000/10/16 21:10:45  hurdler
// Improving stats
//
// Revision 1.3  2000/09/01 23:30:32  hurdler
// minor changes (stats, log & timeout)
//
// Revision 1.2  2000/08/21 11:08:46  hurdler
// Improve the stats
//
// Revision 1.1  2000/08/20 14:12:47  hurdler
// Adding statistics
//
//
//
// DESCRIPTION:
//
//
//-----------------------------------------------------------------------------

#include <unistd.h>      //
#include <string.h>      // strcat(),...
#include "common.h"
#include "ipcs.h"
#include "stats.h"

//=============================================================================


/*
** CServerStats():
*/
CServerStats::CServerStats()
{
    uptime = time(NULL);
    num_connections = 0;
    num_http_con    = 0;
    num_servers     = 0;
    strcpy(motd, "Welcome to the SRB2 Master Server!");
    num_add       = 0;
    num_removal   = 0;
    num_retrieval = 0;
    num_autoremoval = 0;
    num_badconnection = 0;
    for (int i=0; i<5; i++)
    {
        strcpy(last_server[i].port, "0");
        strcpy(last_server[i].ip,   "0.0.0.0");
        strcpy(last_server[i].name, "Non-Existing SRB2 server");
        strcpy(last_server[i].version, "0.0.0");
    }
    sprintf(version, "%s %s", __DATE__, __TIME__);
}


/*
** ~CServerStats():
*/
CServerStats::~CServerStats()
{
}


/*
** getUptime():
*/
const char *CServerStats::getUptime()
{
    char *res = ctime(&uptime);
    res[strlen(res)-1] = '\0'; // remove the '\n' at the end
    return res;
}


/*
** getHours():
*/
int CServerStats::getHours()
{
    return (int)(((time(NULL)-uptime)/(60*60))%24);
}


/*
** getDays():
*/
int CServerStats::getDays()
{
    return (int)((time(NULL)-uptime)/(60*60*24));
}


/*
** getMotd():
*/
const char *CServerStats::getMotd()
{
    return motd;
}


/*
** getLastServers():
*/
const char *CServerStats::getLastServers()
{
    static char res[800];
    
    res[0] = '\0';
    for (int i=0; i<5; i++)
    {
        char str[160];
        char *ct;

        ct = ctime(&last_time[i]);
        ct[strlen(ct)-1] = '\0';
        sprintf(str, "Address: %15s:%-5s   Name: %-32s   v%-7s   (%s %s)\n000012340000", 
                last_server[i].ip, last_server[i].port, last_server[i].name, last_server[i].version, ct, tzname[0]);
        strcat(res, str);
    }

    return res;
}


/*
** getVersion():
*/
const char *CServerStats::getVersion()
{
    return version;
}


/*
** putMotd():
*/
void CServerStats::putMotd(char *motd)
{
    strcpy(this->motd, motd);
}


/*
** putLastServer():
*/
void CServerStats::putLastServer(msg_server_t *server)
{
    for (int i=4; i>0; i--)
    {
        memcpy(&last_server[i], &last_server[i-1], sizeof(msg_server_t));
        last_time[i] = last_time[i-1];
    }
    memcpy(&last_server[0], server, sizeof(msg_server_t));
    last_time[0] = time(NULL);
}

