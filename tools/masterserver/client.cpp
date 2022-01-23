// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: client.cpp,v 1.7 2001/02/15 17:00:34 crashrl Exp $
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
// $Log: client.cpp,v $
// Revision 1.7  2001/02/15 17:00:34  crashrl
// added htonl and ntohl for interendieness compatibility
//
// Revision 1.6  2000/10/16 21:10:45  hurdler
// Improving stats
//
// Revision 1.5  2000/09/01 23:30:31  hurdler
// minor changes (stats, log & timeout)
//
// Revision 1.4  2000/08/20 14:12:47  hurdler
// Adding statistics
//
// Revision 1.3  2000/08/19 14:33:05  hurdler
// changes on log file management
//
// Revision 1.2  2000/08/19 11:31:03  hurdler
// Adding password and log file access tru server
//
// Revision 1.1.1.1  2000/08/18 10:32:09  hurdler
// Initial import of the Master Server code into CVS
//
//
// DESCRIPTION:
//
//
//-----------------------------------------------------------------------------

#include <time.h>
#include <string.h>
#include "ipcs.h"
#include "common.h"

//=============================================================================

static CClientSocket   client_socket;
//static msg_t           msg;
FILE            *logfile;

//=============================================================================

static msg_t *getData(int argc, char *argv[])
{
    static msg_t    msg;
    msg_server_t    *info = (msg_server_t *) msg.buffer;

#if defined (__WIN32__)
    strcpy(info->header, "Unknow");
#else
    strcpy(info->header,   getpass("Enter password: "));
#endif
    
    // default values if one of the param is not entered
    strcpy(info->ip,       "68.100.130.21");
    strcpy(info->port,     "5029");
    strcpy(info->name,     "December Contest - DONT DL");
    strcpy(info->version,  "1.42.20");
    msg.type = ADD_PSERVER_MSG;
    msg.length = sizeof(msg_server_t);
    
    for (int i=0; i<argc; i++)
    {
        // first: extra parameters (must be after all -param)
        if (!strcasecmp(argv[i], "get"))
        {
            msg.type = GET_SERVER_MSG;
            msg.length = 0;
        }
        else if (!strcasecmp(argv[i], "log"))
        {
            msg.type = GET_LOGFILE_MSG;
            msg.length = sizeof(msg_server_t);
        }
        else if (!strcasecmp(argv[i], "erase"))
        {
            msg.type = ERASE_LOGFILE_MSG;
            msg.length = sizeof(msg_server_t);
        }
        else if (!strcasecmp(argv[i], "remove"))
        {
            msg.type = REMOVE_PSERVER_MSG;
            msg.length = sizeof(msg_server_t);
        }

        if (i>=argc-1)
            continue;

        // second: parameter requiring extra parameters
        if (!strcasecmp(argv[i], "-ip"))
        {
            strcpy(info->ip, argv[i+1]);
        }
        else if (!strcasecmp(argv[i], "-port"))
        {
            strcpy(info->port, argv[i+1]);
        }
        else if (!strcasecmp(argv[i], "-hostname"))
        {
            strcpy(info->name, argv[i+1]);
        }
        else if (!strcasecmp(argv[i], "-pass"))
        {
            strcpy(info->header, argv[i+1]);
        }
    }
    return &msg;
}

int main(int argc, char *argv[])
{
    if (argc == 2)
    {
        printf("encrypt: %s\n", pCrypt(argv[1], "04"));
        return 0;
    }
    else if (argc > 2)
    {
        if (client_socket.connect(argv[1], argv[2]) < 0)
        {
            conPrintf(RED, "Connect failed.\n");
            return -1;
        }
        
        msg_t *msg = getData(argc, argv);

        if (client_socket.write(msg) < 0)
        {
            dbgPrintf(RED, "Write failed.\n");
            return -1;
        }

        switch (ntohl(msg->type))
        {
            case GET_SERVER_MSG:
            case GET_LOGFILE_MSG:
                while (client_socket.read(msg) >= 0)
                {
                    if (ntohl(msg->length) == 0)
                        break;
                    printf(msg->buffer);
                }
                break;
                
            case REMOVE_PSERVER_MSG:
            default:
                break;
        }
    }
    return 0;
}
