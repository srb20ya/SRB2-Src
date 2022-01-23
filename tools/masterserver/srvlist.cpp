// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: srvlist.cpp,v 1.4 2000/10/22 00:24:12 hurdler Exp $
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
// $Log: srvlist.cpp,v $
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

#include <unistd.h>
#include <typeinfo>
#include "common.h"
#include "srvlist.h"


//=============================================================================

/*
**
*/
CList::CList()
{
    //num = 0;
    list = NULL;
    current = NULL;
}


/*
**
*/
CList::~CList()
{
    CItem   *p;

    while (list)
    {
        p = list;
        list = list->next;
        delete(p);
    }
}


/*
**
*/
int CList::insert(CItem *item)
{
    item->next = list;
    list = item;
    //num++;
    return 0;
}


/*
**
*/
int CList::remove(CItem *item)
{
    CItem   *p, *q;

    q = NULL;
    p = list;
    while (p && (p != item))
    {
        q = p;
        p = p->next;
    }
    if (p)
    {
        if (q)
            q->next = p->next;
        else
            list = p->next;
        delete p;
        //num--;
        return 1;
    }
    return 0;
}


/*
**
*/
CItem *CList::getFirst()
{
    current = list;
    return current;
}


/*
**
*/
CItem *CList::getNext()
{
    if (current)
    {
        current = current->next;
    }
    return current;
}


/*
**
*/
void CList::show()
{
    CItem   *p;

    p = list;
    while (p)
    {
        p->print();
        p = p->next;
    }
}


//=============================================================================

/*
**
*/
CItem::CItem()
{
    next = NULL;
}


//=============================================================================

/*
**
*/
CInetAddr::CInetAddr(char *ip, char *port)
{
    strcpy(this->ip, ip);
    strcpy(this->port, port);
}


/*
**
*/
const char *CInetAddr::getIP()
{
    return ip;
}


/*
**
*/
const char *CInetAddr::getPort()
{
    return port;
}


//=============================================================================

/*
**
*/
CPlayerItem::CPlayerItem(char *ip, char *port, char *nickname) : CInetAddr(ip, port)
{
    strcpy(this->nickname, nickname);
}


/*
**
*/
void CPlayerItem::print()
{
    dbgPrintf(GREEN, "\tIP\t\t: %s\n\tPort\t\t: %s\n\tNickname\t: %s\n", ip, port, nickname);
}


/*
**
*/
char *CPlayerItem::getString()
{
    static char tmpbuf[256];

    sprintf(tmpbuf, "\tIP\t\t: %s\n\tPort\t\t: %s\n\tNickname\t: %s\n", ip, port, nickname);
    return tmpbuf;
}


//=============================================================================

/*
**
*/
CServerItem::CServerItem(char *ip, char *port, char *hostname, char *version, ServerType type) : CInetAddr(ip, port)
{
    time_t timenow = time(NULL);
    const tm *timeGMT = gmtime(&timenow);
    strcpy(this->hostname, hostname);
    strcpy(this->version, version);
    this->type = type;
    strftime(reg_time, REG_TIME_SIZE+1, "%Y-%m-%dT%H:%MZ",timeGMT);
    {
        int i;
        memset(guid,'\0',GUID_SIZE);
        strcpy(&guid[0], ip);
        strcpy(&guid[15], port);//GenUID
        for(i=0;i<=GUID_SIZE-1;i++)
        {
            if(guid[i]=='\0' || guid[i]=='.')
             guid[i] = '0'+(rand()/(RAND_MAX/15));
            if(guid[i] > '9')
             guid[i] += 'A'-'9';
        }
        guid[GUID_SIZE]='\0';
    }
    ping_time = time(NULL);
}


/*
**
*/
void CServerItem::print()
{
    dbgPrintf(GREEN, "IP\t\t: %s\nPort\t\t: %s\nHostname\t: %s\nVersion\t: %s\nPermanent\t: %s\n", 
                     ip, port, hostname, version, (type==ST_PERMANENT) ? "Yes" : "No");
}


/*
**
*/
const char *CServerItem::getString()
{
    static char tmpbuf[256];

    sprintf(tmpbuf, "IP\t\t: %s\nPort\t\t: %s\nHostname\t: %s\nVersion\t: %s\nPermanent\t: %s\n", 
                    ip, port, hostname, version, (type==ST_PERMANENT) ? "Yes" : "No");
    return tmpbuf;
}


/*
**
*/
const char *CServerItem::getName()
{
    return hostname;
}


/*
**
*/
const char *CServerItem::getVersion()
{
    return version;
}


/*
**
*/
const char *CServerItem::getGuid()
{
    return guid;
}
/*
**
*/
const char *CServerItem::getRegtime()
{
    return reg_time;
}    


/*
**
*/
//=============================================================================

/*
**
*/
void CServerList::insertPlayer(CServerItem *server, CPlayerItem *player)
{
    server->players_list.insert(player);
}


/*
**
*/
void CServerList::removePlayer(CServerItem *server, CPlayerItem *player)
{
    server->players_list.remove(player);
}


/*
**
*/
int CServerList::insert(CServerItem *server)
{
    CList::insert((CItem *)server);
    return 0;
}


/*
**
*/
int CServerList::insert(char *ip, char *port, char *hostname, char *version, ServerType type)
{
    CServerItem *server;

    server = new CServerItem(ip, port, hostname, version, type);
    CList::insert(server);
    return 0;
}


/*
**
*/
int CServerList::remove(CServerItem *server)
{
    return CList::remove((CItem *)server);
}


/*
**
*/
int CServerList::remove(char *ip, char *port, char *hostname, char *version, ServerType type)
{
    //TODO
    CServerItem     *p, *q;

    q = NULL;
    p = (CServerItem *) list;
    while (p && strcmp(p->ip, ip) && strcmp(p->port, port) && 
           strcmp(p->hostname, hostname) && strcmp(p->version, version) && (p->type != type))
    {
        q = p;
        p = (CServerItem *) p->next;
    }
    if (p)
    {
        if (q)
            q->next = p->next;
        else
            list = p->next;
        delete p;
        //num--;
        return 1;
    }
    return 0;
}


/*
**
*/
void CServerList::show()
{
    CServerItem *p;

    p = (CServerItem *) list;
    while (p)
    {
        p->print();
        p->players_list.show();
        p = (CServerItem *) p->next;
    }
}
