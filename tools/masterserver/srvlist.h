// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: srvlist.h,v 1.4 2000/10/22 00:24:12 hurdler Exp $
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
// $Log: srvlist.h,v $
// Revision 1.4  2000/10/22 00:24:12  hurdler
// Add version support
//
// Revision 1.3  2000/10/16 21:10:45  hurdler
// Improving stats
//
// Revision 1.2  2000/08/21 11:08:46  hurdler
// Improve the stats
//
// Revision 1.1.1.1  2000/08/18 10:32:13  hurdler
// Initial import of the Master Server code into CVS
//
//
// DESCRIPTION:
//
//
//-----------------------------------------------------------------------------

#ifndef SRVLIST_H
#define SRVLIST_H

#include <time.h>

enum ServerType
{
    ST_PERMANENT = 0,
    ST_TEMPORARY,
};

//=============================================================================

class CItem
{
friend class CList;
protected:
    CItem             *next;
public:
                       CItem();
    virtual            ~CItem() {};    // destructor must exist and be virtual
    virtual void       print() = 0;
    virtual const char *getString() = 0;
};

//=============================================================================

class CList
{
protected:
//  int         num;
    CItem       *list;
    CItem       *current;   // the current item we are pointing on (for getfirst/getnext)
public:
                CList();
    virtual     ~CList();
    int         insert(CItem *);
    int         remove(CItem *);
    CItem       *getFirst();
    CItem       *getNext();
    void        show();
};

//=============================================================================

class CInetAddr
{
protected:
    char        ip[16];
    char        port[8];
public:
                CInetAddr(char *ip, char *port);
    const char  *getIP();
    const char  *getPort();
};

//=============================================================================

class CPlayerItem : public CItem, public CInetAddr
{
friend class CPlayerList;
private:
    char        nickname[32];
public:
                CPlayerItem(char *ip, char *port, char *nickname);
    void        print();
    char        *getString();
};

//=============================================================================

class CPlayerList : public CList
{
};


//=============================================================================

#define REG_TIME_SIZE 24   //1970-00-00T00:00:00.0+00:00
#define GUID_SIZE 24-1 // format is 12700000000105000

class CServerItem : public CItem, public CInetAddr
{
friend class CServerList;
private:
    char        hostname[32];
    char        version[8];     // format is: x.yy.z (like 1.30.2 or 1.31)
    CPlayerList players_list;
    char        reg_time[REG_TIME_SIZE];
    char        guid[GUID_SIZE+1];
public:
    ServerType  type;
    time_t      ping_time;
    
                CServerItem(char *ip, char *port, char *hostname, char *version, ServerType type);
    void        print();
    const char  *getString();
    const char  *getName();
    const char  *getVersion();
    const char  *getRegtime();
    const char  *getGuid();
};

//=============================================================================

class CServerList : public CList
{
public:
    void        insertPlayer(CServerItem *server, CPlayerItem *player);
    void        removePlayer(CServerItem *server, CPlayerItem *player);
    int         insert(char *ip, char *port, char *hostname, char *version, ServerType type);
    int         insert(CServerItem * server);
    int         remove(char *ip, char *port, char *hostname, char *version, ServerType type);
    int         remove(CServerItem * server);
    void        show();
};


#endif
