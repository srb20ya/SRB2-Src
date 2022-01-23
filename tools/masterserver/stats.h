// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: stats.h,v 1.3 2000/10/16 21:10:45 hurdler Exp $
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
// $Log: stats.h,v $
// Revision 1.3  2000/10/16 21:10:45  hurdler
// Improving stats
//
// Revision 1.2  2000/09/01 23:30:32  hurdler
// minor changes (stats, log & timeout)
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

#ifndef _STATS_H_
#define _STATS_H_
#ifndef _IPCS_H_
#error "You forget to include ipcs.h"
#endif
#include <time.h>

// ================================ DEFINITIONS ===============================

class CServerStats
{
    time_t          uptime;         // master server uptime
    char            motd[2048];     // increase that if not enough
    msg_server_t    last_server[5]; // keep last 5 named registered servers
    time_t          last_time[5];   // keep date/time of registration of those servers
    char            version[32];    // master server version
    
public:
    int        num_connections;
    int        num_http_con;
    int        num_text_con;
    int        num_RSS92_con;
    int        num_RSS10_con;
    int        num_servers;
    int        num_add;
    int        num_removal;
    int        num_retrieval;
    int        num_autoremoval;
    int        num_badconnection;

               CServerStats();
               ~CServerStats();
    const char *getUptime();
    int        getHours();
    int        getDays();
    const char *getMotd();
    const char *getLastServers();
    const char *getVersion();
    void       putMotd(char *);
    void       putLastServer(msg_server_t *);
};

// ================================== PROTOS ==================================


// ================================== EXTERNS =================================


#endif
