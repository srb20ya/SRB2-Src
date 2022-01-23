// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: I_net.c,v 1.2 2000/08/10 11:07:51 ydario Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: I_net.c,v $
// Revision 1.2  2000/08/10 11:07:51  ydario
// fix CRLF
//
// Revision 1.1  2000/08/09 11:51:28  ydario
// OS/2 specific platform code
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      network interface
//      
//-----------------------------------------------------------------------------


#include <errno.h>

#include "../doomdef.h"

#include "../i_system.h"
#include "../d_event.h"
#include "../d_net.h"
#include "../m_argv.h"

#include "../doomstat.h"

#include "../i_net.h"

#include "../z_zone.h"

int I_InitTcpNetwork(void);
//
// NETWORKING
//

void Internal_Get(void)
{
     I_Error("Get without netgame\n");
}

void Internal_Send(void)
{
     I_Error("Send without netgame\n");
}

void Internal_FreeNodenum(int nodenum)
{}

//
// I_InitNetwork
//
void I_InitNetwork (void)
{
  int netgamepar;

  I_NetGet  = Internal_Get;
  I_NetSend = Internal_Send;
  I_NetShutdown = NULL;
  I_NetFreeNodenum = Internal_FreeNodenum;

  netgamepar = M_CheckParm ("-net");
  if(!netgamepar)
  {
      doomcom=Z_Malloc(sizeof(doomcom_t),PU_STATIC,NULL);
      memset(doomcom,0,sizeof(doomcom_t));
      doomcom->id = DOOMCOM_ID;

      if(!I_InitTcpNetwork())
      {
          netgame = false;
          server = true;
          multiplayer = false;

          doomcom->numplayers = doomcom->numnodes = 1;
          doomcom->deathmatch = false;
          doomcom->consoleplayer = 0;
          doomcom->ticdup = 1;
          doomcom->extratics = 0;
          return;
      }
  } // else net game
  else
  {
      I_Error("-net not supported, use -server and -connect\n"
              "see docs for more\n");
  }
  // net game

  netgame = true;
  multiplayer = true;
}
