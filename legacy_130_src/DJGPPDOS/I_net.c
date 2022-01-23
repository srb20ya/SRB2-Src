// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id: I_net.c,v 1.3 2000/04/16 18:38:07 bpereira Exp $
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
// Revision 1.3  2000/04/16 18:38:07  bpereira
// no message
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


#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>

#include <go32.h>
#include <pc.h>
#include <dpmi.h>
#include <dos.h>
#include <sys/nearptr.h>

#include "../doomdef.h"

#include "../i_system.h"
#include "../d_event.h"
#include "../d_net.h"
#include "../m_argv.h"

#include "../doomstat.h"
#include "../z_zone.h"
#include "../i_net.h"
#include "../i_tcp.h"

//
// NETWORKING
//

typedef enum
{
    CMD_SEND    = 1,
    CMD_GET      = 2,
    CMD_FREENODE = 3  // new legacy 1.29 only supported in doomatic

} command_t;

static  int doomatic;

void External_Driver_Get(void);
void External_Driver_Send(void);
void External_Driver_FreeNode(int nodenum);

void Internal_Get(void)
{
     I_Error("Get without netgame\n");
}

void Internal_Send(void)
{
     I_Error("Send without netgame\n");
}

void Internal_FreeNodenum(int nodenum)
{
     if(nodenum)
         I_Error("FreeNode without netgame\n");
}


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
	  doomcom->gametype = false; // Tails 03-13-2001
          doomcom->consoleplayer = 0;
          doomcom->ticdup = 1;
          doomcom->extratics = 0;
          return;
      }
  }
  else
  {  // externals drivers specific

      __djgpp_nearptr_enable();

      // set up for network
      doomcom=(doomcom_t *)(__djgpp_conventional_base+atoi(myargv[netgamepar+1]));
      CONS_Printf("I_DosNet : Using int 0x%x for communication\n",doomcom->intnum);

      doomatic=M_CheckParm("-doomatic");
      server = (doomcom->consoleplayer == 0);
      if(!server)
          COM_BufAddText("connect any\n");

      // ipx + time + 4 (padding)
      packetheaderlength=30+4+4;

      // 13-5-99 : added to pass legacy version to doomatic
      if( doomatic )
      {
          doomcom->episode = VERSION;
          hardware_MAXPACKETLENGHT=1400;
      }
      else
          hardware_MAXPACKETLENGHT=512;
      I_NetGet  = External_Driver_Get;
      I_NetSend = External_Driver_Send;
      I_NetShutdown = NULL;
      I_NetFreeNodenum = External_Driver_FreeNode;
      if (M_CheckParm ("-extratic"))
          doomcom->extratics = 1;
      else
          doomcom->extratics = 0;
  }

  // net game

  netgame = true;
  multiplayer = false;

  // litle optimitation with doomatic
  // it store the boolean packetreceived in the doomcom->drone
  // (see External_Driver_Get)
  doomcom->ticdup = 1;
}

void External_Driver_Get(void)
{
    __dpmi_regs r;

    doomcom->command=CMD_GET;

    // it normaly save a task switch to the processor
    if(doomatic && !doomcom->drone)
    {
        doomcom->remotenode = -1;
        return;
    }

    __dpmi_int(doomcom->intnum,&r);
    if(doomatic && doomcom->remotenode==-2)
        I_Error("Doomatic error :%s\n",doomcom->data);
}

void External_Driver_Send(void)
{
    __dpmi_regs r;

    if((!doomatic) && (doomcom->remotenode==BROADCASTADDR))
    {
        int i;
        for(i=1;i<doomcom->numnodes;i++)
        {
            doomcom->remotenode=i;
            doomcom->command=CMD_SEND;
            __dpmi_int(doomcom->intnum,&r);
        }

        doomcom->remotenode=BROADCASTADDR;
    }
    else
    {
        doomcom->command=CMD_SEND;
        __dpmi_int(doomcom->intnum,&r);
    }
}

void External_Driver_FreeNode(int nodenum)
{
    __dpmi_regs r;

    if(doomatic && nodenum)
    {
        doomcom->command=CMD_FREENODE;
        doomcom->remotenode = nodenum;
        __dpmi_int(doomcom->intnum,&r);
    }
}

