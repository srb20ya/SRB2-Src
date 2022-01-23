#############################################################################
#
# $Id: makefile,v 1.27 2000/08/10 19:58:04 bpereira Exp $
#     GNU Make makefile for DOS/LINUX version
#				      of Doom LEGACY (legacy@frag.com)
#
# Copyright (C) 1998-2000 by DooM Legacy Team.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# $Log: makefile,v $
# Revision 1.27  2000/08/10 19:58:04  bpereira
# no message
#
# Revision 1.26  2000/08/10 14:57:26  ydario
# OS/2 port
#
# Revision 1.25  2000/08/03 17:57:42  bpereira
# no message
#
# Revision 1.24  2000/07/01 09:23:49  bpereira
# no message
#
# Revision 1.23  2000/05/07 08:27:57  metzgermeister
# no message
#
# Revision 1.20  2000/04/25 19:53:12  metzgermeister
# fixed the old tabs and spaces problem
#
# Revision 1.19  2000/04/24 20:24:38  bpereira
# no message
#
# Revision 1.18  2000/04/22 20:26:01  metzgermeister
# commandline compile options
#
# Revision 1.17  2000/04/21 08:23:47  emanne
# To have SDL working.
# Makefile: made the hiding by "@" optional. See the CC variable at
# the begining. Sorry, but I like to see what's going on while building
#
# qmus2mid.h: force include of qmus2mid_sdl.h when needed.
# s_sound.c: ??!
# s_sound.h: with it.
# (sorry for s_sound.* : I had problems with cvs...)
#
# Revision 1.16  2000/04/18 14:50:36  hurdler
# no message
#
# Revision 1.15  2000/04/16 18:38:07  bpereira
# no message
#
# Revision 1.14  2000/04/13 23:47:47  stroggonmeth
# See logs
#
# Revision 1.13  2000/04/12 19:32:04  metzgermeister
# added support for commandline OS switch
#
# Revision 1.12  2000/04/11 19:07:23  stroggonmeth
# Finished my logs, fixed a crashing bug.
#
# Revision 1.11  2000/04/09 02:30:57  stroggonmeth
# Fixed missing sprite def :\
#
# Revision 1.10  2000/04/08 17:29:24  stroggonmeth
# no message
#
# Revision 1.9	2000/04/07 23:10:15  metzgermeister
# fullscreen support under X in Linux
#
# Revision 1.8	2000/04/04 19:28:42  stroggonmeth
# Global colormaps working. Added a new linedef type 272.
#
# Revision 1.7	2000/04/04 00:32:46  stroggonmeth
# Initial Boom compatability plus few misc changes all around.
#
# Revision 1.6	2000/03/29 19:39:48  bpereira
# no message
#
# Revision 1.5	2000/03/07 03:32:24  hurdler
# fix linux compilation
#
# Revision 1.4	2000/03/05 17:17:03  bpereira
# no message
#
# Revision 1.3	2000/03/04 23:27:08  hurdler
# fix "missing operator" problem
#
# Revision 1.2	2000/02/27 00:42:10  hurdler
# fix CR+LF problem
#
# Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
# Initial import into CVS (v1.29 pr3)
#
#
#     -DUSEASM -> use assembly routines instead of C ones, where possible.
#     -DPC_DOS -> use dos specific code (eg:textmode stuff)...
#     -DLINUX  -> use for the linux specific
#     -D__OS2__  -> use for the OS/2 specific
#     -DNORMALUNIX -> well unused at this moment
#
#     Compile the Linux version with 'make LINUX=1'
#     Compile the OS/2 version with  'make OS2=1'
#     Compile a debug/profile version with 'make LINUX=1 PROFILEMODE=1'
#     Compile the SDL version with 'make LINUX=1 HAVE_MIXER=1'
#
#############################################################################

# gcc or g++
#CC=@gcc
CC=gcc

#if use PGCC or EGCS
PGCC=1

WFLAGS=-Wall # -W -Wno-unused -Wno-sign-compare

#indicate platform and what interface use with
# posible value for now : DJGPPDOS
#			 LINUX
ifndef OS2
ifndef LINUX
	DJGPPDOS=1
endif
endif


#determine the interface directory (where you put all i_*.c)
ifdef DJGPPDOS
       INTERFACE=DJGPPDOS
else
       ifdef LINUX
	     INTERFACE=linux_x
       endif
       ifdef OS2
	     INTERFACE=os2
       endif
endif

ifdef DJGPPDOS
	# subdirectory for objects

    O=objs
    BIN=bin

	#use the x86 asm code
	USEASM=1

	# options
	OPTS=-DPC_DOS

	DEBUGLIBS=-lalleg -lbcd -lsocket
    LIBS=-lalleg -lbcd -lsocket -s

	SFLAGS=

	OBJS=$(O)/i_video.o $(O)/vid_vesa.o

	#somes SDL messup
	qmus2mid_obj = $(O)/qmus2mid.o
	qmus2mid_h   = qmus2mid.h

	# name of the exefile
	EXENAME=DOOM3.exe

endif #ifdef DJGPPDOS

ifdef LINUX

	# subdirectory for objects
	O=../objs
	BIN=../bin

	#use the x86 asm code
	USEASM=1

ifdef HAVE_MIXER
	OPTS=-DNORMALUNIX -DLINUX -DHWRENDER -DDIRECTFULLSCREEN
	SDL=1
else
	OPTS=-DNORMALUNIX -DLINUX -DOLD_SOUND_DRIVER -DMUSSERV -DSNDSERV -DHWRENDER -DDIRECTFULLSCREEN
endif
	X=1

ifndef SDL
	qmus2mid_obj = $(O)/qmus2mid.o
	qmus2mid_h   = qmus2mid.h
endif
	ifdef X

		OPTS := $(OPTS) -DVID_X11 -DPOLL_POINTER -I.
		LDFLAGS=-L/usr/X11R6/lib
		LIBS=-lXext -lX11 -lm -ldl -lXxf86vm
		# name of the exefile
		EXENAME=llxdoom
		SFLAGS=-g $(OPTS)
		OBJS=$(O)/i_video_xshm.o $(O)/dosstr.o $(O)/searchp.o $(O)/endtxt.o \
		 $(O)/hw_bsp.o $(O)/hw_draw.o $(O)/hw_light.o $(O)/hw_main.o \
		 $(O)/hw_md2.o $(O)/hw_cache.o

	else
	    OPTS := $(OPTS) -DVID_GGI -I.
	    LDFLAGS=
	    LIBS=-lggi -lm
	    # name of the exefile
	    EXENAME=llggidoom
	    SFLAGS=-g $(OPTS)
	    OBJS=$(O)/i_video_ggi.o $(O)/dosstr.o $(O)/searchp.o $(O)/endtxt.o \
		 $(O)/hw_bsp.o $(O)/hw_draw.o $(O)/hw_light.o $(O)/hw_main.o \
		 $(O)/hw_md2.o $(O)/hw_cache.o

	endif
	ifdef SDL
		qmus2mid_obj = $(O)/qmus2mid_sdl.o
		qmus2mid_h   = qmus2mid_sdl.h
		OPTS := $(OPTS) -DSDL -DVID_X11 -DPOLL_POINTER -I. -I/usr/local/include/SDL
ifdef HAVE_MIXER
	OPTS := $(OPTS) -DHAVE_MIXER
endif
		LDFLAGS=-L/usr/X11R6/lib -L/usr/local/lib
		LIBS=-lXext -lX11 -lm -ldl -lXxf86vm -lSDL -lSDL_mixer -lpthread
		DEBUGLIBS=$(LIBS)
		# name of the exefile
		EXENAME=lsdldoom
	endif
	SFLAGS=-g $(OPTS)
endif

ifdef OS2

	# subdirectory for objects
	O=objs
	BIN=bin

	#use the x86 asm code
	NASM=nasm
	USEASM=1
	PROFILEMODE=1

	# options
	OPTS=-Zmt -D__OS2__ -I.. -I. -Ios2

	DEBUGLIBS=-lmmpm2 -lsocket -g
	LIBS= $(INTERFACE)/I_pm.def -lmmpm2 -lsocket -g

	SFLAGS=-g $(OPTS)

	OBJS=$(O)/i_video.o $(O)/i_dart.o $(O)/i_dive.o $(O)/I_pm.o \
		$(O)/printf.o $(O)/I_pm.res

	#somes SDL messup
	qmus2mid_obj = $(O)/qmus2mid2.o
	qmus2mid_h   = qmus2mid.h

	# name of the exefile
	EXENAME=Doom2.exe

endif

ifdef USEASM
   OBJS:= $(OBJS) $(O)/tmap.o $(O)/vid_copy.o
   OPTS:= $(OPTS) -DUSEASM
endif

ifdef PROFILEMODE

	# build with profiling information
	ifdef PGCC
		CFLAGS = -g -pg $(OPTS)
	else
		CFLAGS = -g -pg -m486 -O3 -ffast-math $(OPTS)
	endif
	LDFLAGS = -g -pg -L/usr/X11R6/lib
else

	# build a normal optimised version
	ifdef PGCC
		CFLAGS = $(OPTS) -mpentium -O6 -ffast-math -fomit-frame-pointer
		# -fgcse -fcse-follow-jumps -fcse-skip-blocks -frerun-cse-after-loop -ffunction-cse \
		# -fexpensive-optimizations -fforce-mem -fstrength-reduce -fthread-jumps  \
		# -fregmove -fschedule-insns -fschedule-insns2 -fmove-all-movables \
		# -fcaller-saves -finline-functions -mieee-fp -mfp-ret-in-387 -m80387 -mhard-float -finline
	else
		CFLAGS = -m486 -O3 -ffast-math -fomit-frame-pointer $(OPTS)
	endif
endif

# not too sophisticated dependency
ifdef SDL
OBJS :=  $(OBJS) $(O)/i_sound_sdl.o
else
OBJS :=  $(OBJS) $(O)/i_sound.o
endif
# not too sophisticated dependency
OBJS :=  $(OBJS) \
		$(O)/dstrings.o  \
		$(O)/i_cdmus.o	 \
		$(O)/i_net.o	 \
		$(O)/i_tcp.o	 \
		$(O)/i_system.o  \
		$(O)/tables.o	 \
		$(O)/f_finale.o  \
		$(O)/f_wipe.o	 \
		$(O)/d_main.o	 \
		$(O)/d_net.o	 \
		$(O)/d_items.o	 \
		$(O)/g_game.o	 \
		$(O)/m_menu.o	 \
		$(O)/m_misc.o	 \
		$(O)/m_argv.o	 \
		$(O)/m_bbox.o	 \
		$(O)/m_fixed.o	 \
		$(O)/m_swap.o	 \
		$(O)/m_cheat.o	 \
		$(O)/m_random.o  \
		$(O)/am_map.o	 \
		$(O)/p_ceilng.o  \
		$(O)/p_doors.o	 \
		$(O)/p_enemy.o	 \
		$(O)/p_floor.o	 \
	$(O)/p_genlin.o  \
		$(O)/p_inter.o	 \
		$(O)/p_lights.o  \
		$(O)/p_map.o	 \
		$(O)/p_maputl.o  \
		$(O)/p_plats.o	 \
		$(O)/p_pspr.o	 \
		$(O)/p_setup.o	 \
		$(O)/p_sight.o	 \
		$(O)/p_spec.o	 \
		$(O)/p_switch.o  \
		$(O)/p_mobj.o	 \
		$(O)/p_telept.o  \
		$(O)/p_tick.o	 \
		$(O)/p_saveg.o	 \
		$(O)/p_user.o	 \
		$(O)/r_bsp.o	 \
		$(O)/r_data.o	 \
		$(O)/r_draw.o	 \
		$(O)/r_main.o	 \
		$(O)/r_plane.o	 \
		$(O)/r_segs.o	 \
		$(O)/r_sky.o	 \
		$(O)/r_things.o  \
		$(O)/w_wad.o	 \
		$(O)/wi_stuff.o  \
		$(O)/v_video.o	 \
		$(O)/st_lib.o	 \
		$(O)/st_stuff.o  \
		$(O)/hu_stuff.o  \
		$(O)/s_sound.o	 \
		$(O)/z_zone.o	 \
		$(O)/info.o	 \
		$(O)/sounds.o	 \
		$(O)/p_fab.o	 \
		$(O)/dehacked.o  \
		$(qmus2mid_obj)  \
		$(O)/g_input.o	 \
		$(O)/screen.o	 \
		$(O)/console.o	 \
		$(O)/command.o	 \
		$(O)/d_netcmd.o  \
		$(O)/d_clisrv.o  \
		$(O)/r_splats.o  \
		$(O)/g_state.o	 \
		$(O)/d_netfil.o
ifdef LINUX
all:	 $(BIN)/$(EXENAME) dll
else
all:	 $(BIN)/$(EXENAME)
endif

clean:
ifdef LINUX
	rm -f *.o *~ *.flc
	rm -f $(O)/*
else
	del $(O)\*.* >NUL
endif

asm:
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -g $(O)/i_main.o \
	-o $(O)/tmp.exe $(DEBUGLIBS)
	objdump  -d $(O)/tmp.exe --no-show-raw-insn > doom3.s
	del $(O)\tmp.exe

# executable

$(BIN)/$(EXENAME):	$(O) $(OBJS) $(O)/i_main.o
	@echo Linking...
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(O)/i_main.o \
	-o $(BIN)/$(EXENAME) $(LIBS)

$(O):
	@mkdir $(O)

#dll
dll : $(O)/r_opengl.o $(O)/ogl_x11.o
	$(CC) -mpentium -O6 -o $(BIN)/r_opengl.so -shared -nostartfiles $(O)/r_opengl.o $(O)/ogl_x11.o -L/usr/X11R6/lib -L/usr/local/lib -lGL -lX11 -lXext -lGLU -lm

$(O)/r_opengl.o:	hardware/r_opengl/r_opengl.c hardware/r_opengl/r_opengl.h
	$(CC) -mpentium -O6 -o $(O)/r_opengl.o	-DHWRENDER -DVID_X11 -DLINUX -DPOLL_POINTER -shared -nostartfiles -c hardware/r_opengl/r_opengl.c

$(O)/ogl_x11.o:  hardware/r_opengl/ogl_x11.c hardware/r_opengl/r_opengl.h
	$(CC) -mpentium -O6 -o $(O)/ogl_x11.o -DHWRENDER -DVID_X11 -DLINUX -DPOLL_POINTER -shared -nostartfiles -c hardware/r_opengl/ogl_x11.c


#dependecy made by gcc itself ! (see makedep.mak for more)
$(O)/dstrings.o: dstrings.c dstrings.h d_englsh.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/i_sound.o: $(INTERFACE)/i_sound.c doomdef.h \
 doomtype.h g_state.h m_swap.h doomstat.h doomdata.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h command.h i_system.h \
 d_event.h i_sound.h sounds.h z_zone.h m_argv.h m_misc.h \
 w_wad.h s_sound.h $(qmus2mid_h)
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/i_cdmus.o: $(INTERFACE)/i_cdmus.c doomdef.h \
 doomtype.h g_state.h m_swap.h	i_sound.h sounds.h command.h \
 i_system.h d_ticcmd.h d_event.h  s_sound.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/i_video.o: $(INTERFACE)/i_video.c doomdef.h \
 doomtype.h g_state.h m_swap.h i_system.h d_ticcmd.h d_event.h \
 v_video.h r_defs.h m_fixed.h d_think.h p_mobj.h tables.h \
 doomdata.h info.h screen.h command.h m_argv.h $(INTERFACE)/vid_vesa.h \
 i_video.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/i_net.o: $(INTERFACE)/i_net.c doomdef.h doomtype.h \
 g_state.h m_swap.h i_system.h d_ticcmd.h d_event.h d_net.h \
 m_argv.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_clisrv.h \
 d_netcmd.h command.h z_zone.h i_net.h i_tcp.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/i_tcp.o: i_tcp.c doomdef.h doomtype.h g_state.h m_swap.h i_system.h \
 d_ticcmd.h d_event.h i_net.h d_net.h m_argv.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h d_clisrv.h d_netcmd.h command.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/i_system.o: $(INTERFACE)/i_system.c doomdef.h \
 doomtype.h g_state.h m_swap.h \
 m_misc.h w_wad.h i_video.h \
 i_sound.h sounds.h d_net.h \
 g_game.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h \
 d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h command.h \
 d_event.h d_main.h m_argv.h \
 z_zone.h g_input.h \
 console.h i_system.h \
 i_joy.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/tables.o: tables.c tables.h m_fixed.h doomtype.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/f_finale.o: f_finale.c doomdef.h doomtype.h g_state.h m_swap.h \
 am_map.h d_event.h dstrings.h d_englsh.h d_main.h w_wad.h f_finale.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h command.h hu_stuff.h wi_stuff.h r_defs.h screen.h \
 r_local.h m_bbox.h r_main.h r_data.h r_state.h r_bsp.h r_segs.h \
 r_plane.h r_sky.h r_things.h sounds.h r_draw.h s_sound.h i_video.h \
 v_video.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/f_wipe.o: f_wipe.c doomdef.h doomtype.h g_state.h m_swap.h m_random.h \
 f_wipe.h i_system.h d_ticcmd.h d_event.h i_video.h v_video.h r_defs.h \
 m_fixed.h d_think.h p_mobj.h tables.h doomdata.h info.h screen.h \
 command.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/d_main.o: d_main.c doomdef.h doomtype.h g_state.h m_swap.h command.h \
 console.h d_event.h doomstat.h doomdata.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h am_map.h d_net.h dehacked.h dstrings.h \
 d_englsh.h f_wipe.h f_finale.h g_game.h g_input.h keys.h hu_stuff.h \
 w_wad.h wi_stuff.h r_defs.h screen.h i_sound.h sounds.h i_system.h \
 i_video.h m_argv.h m_menu.h m_misc.h p_setup.h p_fab.h r_main.h \
 r_data.h r_state.h r_local.h m_bbox.h r_bsp.h r_segs.h r_plane.h \
 r_sky.h r_things.h r_draw.h s_sound.h st_stuff.h v_video.h z_zone.h \
 d_main.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/d_net.o: d_net.c doomdef.h doomtype.h g_state.h m_swap.h d_clisrv.h \
 d_ticcmd.h d_netcmd.h command.h g_game.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h d_event.h i_net.h i_system.h m_argv.h d_net.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/d_items.o: d_items.c info.h d_think.h d_items.h doomdef.h doomtype.h \
 g_state.h m_swap.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/g_game.o: g_game.c doomdef.h doomtype.h g_state.h m_swap.h command.h \
 console.h d_event.h dstrings.h d_englsh.h d_main.h w_wad.h d_net.h \
 d_netcmd.h f_finale.h p_setup.h doomdata.h p_saveg.h i_system.h \
 d_ticcmd.h wi_stuff.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h am_map.h m_random.h p_local.h \
 m_bbox.h p_tick.h r_defs.h screen.h p_maputl.h p_spec.h r_data.h \
 r_state.h r_draw.h r_main.h r_sky.h s_sound.h sounds.h g_game.h \
 doomstat.h d_clisrv.h g_input.h keys.h p_fab.h m_cheat.h m_misc.h \
 m_menu.h m_argv.h hu_stuff.h st_stuff.h z_zone.h i_video.h p_inter.h \
 byteptr.h i_joy.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/m_menu.o: m_menu.c am_map.h d_event.h doomtype.h g_state.h doomdef.h \
 m_swap.h dstrings.h d_englsh.h d_main.h w_wad.h console.h r_local.h \
 tables.h m_fixed.h screen.h command.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h hu_stuff.h wi_stuff.h g_game.h \
 doomstat.h d_clisrv.h d_netcmd.h g_input.h keys.h m_argv.h s_sound.h \
 i_system.h m_menu.h v_video.h i_video.h z_zone.h p_local.h p_tick.h \
 p_maputl.h p_spec.h p_fab.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/m_misc.o: m_misc.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h m_misc.h w_wad.h hu_stuff.h wi_stuff.h r_defs.h \
 screen.h v_video.h z_zone.h g_input.h keys.h i_video.h d_main.h \
 m_argv.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/m_argv.o: m_argv.c doomdef.h doomtype.h g_state.h m_swap.h command.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/m_bbox.o: m_bbox.c doomtype.h m_bbox.h m_fixed.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/m_fixed.o: m_fixed.c i_system.h d_ticcmd.h doomtype.h d_event.h \
 g_state.h m_fixed.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/m_swap.o: m_swap.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/m_cheat.o: m_cheat.c doomdef.h doomtype.h g_state.h m_swap.h \
 dstrings.h d_englsh.h am_map.h d_event.h m_cheat.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h r_local.h screen.h m_bbox.h r_main.h r_data.h r_defs.h \
 r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h sounds.h \
 r_draw.h p_local.h p_tick.h p_maputl.h p_spec.h p_inter.h i_sound.h \
 s_sound.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/m_random.o: m_random.c doomdef.h doomtype.h g_state.h m_swap.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/am_map.o: am_map.c doomdef.h doomtype.h g_state.h m_swap.h doomstat.h \
 doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h command.h \
 g_game.h d_event.h am_map.h g_input.h keys.h m_cheat.h p_local.h \
 m_bbox.h p_tick.h r_defs.h screen.h p_maputl.h p_spec.h v_video.h \
 st_stuff.h i_system.h i_video.h r_state.h r_data.h dstrings.h \
 d_englsh.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_ceilng.o: p_ceilng.c doomdef.h doomtype.h g_state.h m_swap.h \
 p_local.h command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h \
 r_defs.h screen.h p_maputl.h p_spec.h r_state.h r_data.h s_sound.h \
 sounds.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_doors.o: p_doors.c doomdef.h doomtype.h g_state.h m_swap.h \
 dstrings.h d_englsh.h p_local.h command.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h doomdata.h \
 d_ticcmd.h m_bbox.h p_tick.h r_defs.h screen.h p_maputl.h p_spec.h \
 r_state.h r_data.h s_sound.h sounds.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_enemy.o: p_enemy.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h r_main.h r_data.h r_state.h s_sound.h sounds.h \
 m_random.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_floor.o: p_floor.c doomdef.h doomtype.h g_state.h m_swap.h p_local.h \
 command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h r_state.h r_data.h s_sound.h sounds.h \
 z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_genlin.o: p_genlin.c doomdef.h g_game.h p_local.h r_data.h m_random.h \
 m_random.h s_sound.h z_zone.h command.h d_player.h d_think.h m_fixed.h \
 m_bbox.h p_tick.h r_defs.h p_maputl.h p_mobj.h screen.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@


$(O)/p_inter.o: p_inter.c doomdef.h doomtype.h g_state.h m_swap.h \
 i_system.h d_ticcmd.h d_event.h am_map.h dstrings.h d_englsh.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_clisrv.h d_netcmd.h \
 command.h m_random.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h p_inter.h s_sound.h sounds.h r_main.h r_data.h \
 r_state.h st_stuff.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_lights.o: p_lights.c doomdef.h doomtype.h g_state.h m_swap.h \
 p_local.h command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h \
 r_defs.h screen.h p_maputl.h p_spec.h r_state.h r_data.h z_zone.h \
 m_random.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_map.o: p_map.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h m_bbox.h m_random.h p_local.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h p_inter.h r_state.h r_data.h r_main.h \
 r_sky.h s_sound.h sounds.h r_splats.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_maputl.o: p_maputl.c p_local.h command.h doomtype.h d_player.h \
 d_items.h doomdef.h g_state.h m_swap.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h \
 r_defs.h screen.h p_maputl.h p_spec.h r_main.h r_data.h r_state.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_plats.o: p_plats.c doomdef.h doomtype.h g_state.h m_swap.h p_local.h \
 command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h s_sound.h sounds.h r_state.h r_data.h \
 z_zone.h m_random.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_pspr.o: p_pspr.c doomdef.h doomtype.h g_state.h m_swap.h d_event.h \
 p_local.h command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h \
 r_defs.h screen.h p_maputl.h p_spec.h p_inter.h s_sound.h sounds.h \
 g_game.h doomstat.h d_clisrv.h d_netcmd.h g_input.h keys.h r_main.h \
 r_data.h r_state.h m_random.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_setup.o: p_setup.c doomdef.h doomtype.h g_state.h m_swap.h d_main.h \
 d_event.h w_wad.h g_game.h doomstat.h doomdata.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h command.h p_local.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h p_setup.h i_sound.h sounds.h r_sky.h \
 r_data.h r_state.h r_things.h s_sound.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_sight.o: p_sight.c doomdef.h doomtype.h g_state.h m_swap.h p_local.h \
 command.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h doomdata.h d_ticcmd.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h r_main.h r_data.h r_state.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_spec.o: p_spec.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h p_setup.h r_data.h r_state.h m_random.h s_sound.h \
 sounds.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_switch.o: p_switch.c doomdef.h doomtype.h g_state.h m_swap.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h s_sound.h sounds.h r_main.h r_data.h \
 r_state.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_mobj.o: p_mobj.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h g_input.h keys.h st_stuff.h hu_stuff.h w_wad.h \
 wi_stuff.h r_defs.h screen.h p_local.h m_bbox.h p_tick.h p_maputl.h \
 p_spec.h p_inter.h p_setup.h r_main.h r_data.h r_state.h r_things.h \
 sounds.h r_sky.h s_sound.h z_zone.h m_random.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_telept.o: p_telept.c doomdef.h doomtype.h g_state.h m_swap.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h \
 screen.h p_maputl.h p_spec.h r_state.h r_data.h s_sound.h sounds.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_tick.o: p_tick.c doomstat.h doomdata.h doomtype.h doomdef.h \
 g_state.h m_swap.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h command.h \
 g_game.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_saveg.o: p_saveg.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h r_state.h r_data.h z_zone.h w_wad.h p_setup.h \
 byteptr.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/p_user.o: p_user.c doomdef.h doomtype.h g_state.h m_swap.h d_event.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h command.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h r_main.h r_data.h r_state.h s_sound.h sounds.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/r_bsp.o: r_bsp.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h r_local.h screen.h m_bbox.h r_main.h r_data.h \
 r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h \
 sounds.h r_draw.h r_splats.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/r_data.o: r_data.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h i_video.h r_local.h screen.h m_bbox.h r_main.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h p_local.h p_tick.h p_maputl.h p_spec.h \
 w_wad.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/r_draw.o: r_draw.c doomdef.h doomtype.h g_state.h m_swap.h r_local.h \
 tables.h m_fixed.h screen.h command.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h st_stuff.h d_event.h i_video.h \
 v_video.h w_wad.h z_zone.h r_draw8.c r_draw16.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/r_main.o: r_main.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h g_input.h keys.h r_local.h screen.h m_bbox.h \
 r_main.h r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h \
 r_sky.h r_things.h sounds.h r_draw.h r_splats.h st_stuff.h p_local.h \
 p_tick.h p_maputl.h p_spec.h i_video.h m_menu.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/r_plane.o: r_plane.c doomdef.h doomtype.h g_state.h m_swap.h console.h \
 d_event.h g_game.h doomstat.h doomdata.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h command.h r_data.h r_defs.h screen.h r_state.h \
 r_local.h m_bbox.h r_main.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h r_splats.h v_video.h w_wad.h \
 z_zone.h p_setup.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/r_segs.o: r_segs.c doomdef.h doomtype.h g_state.h m_swap.h r_local.h \
 tables.h m_fixed.h screen.h command.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h r_splats.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/r_sky.o: r_sky.c doomdef.h doomtype.h g_state.h m_swap.h r_sky.h \
 m_fixed.h r_local.h tables.h screen.h command.h m_bbox.h r_main.h \
 d_player.h d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h \
 d_ticcmd.h r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h \
 r_things.h sounds.h r_draw.h w_wad.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/r_things.o: r_things.c doomdef.h doomtype.h g_state.h m_swap.h \
 console.h d_event.h g_game.h doomstat.h doomdata.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h \
 d_ticcmd.h d_clisrv.h d_netcmd.h command.h r_local.h screen.h \
 m_bbox.h r_main.h r_data.h r_defs.h r_state.h r_bsp.h r_segs.h \
 r_plane.h r_sky.h r_things.h sounds.h r_draw.h st_stuff.h w_wad.h \
 z_zone.h i_video.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/w_wad.o: w_wad.c doomdef.h doomtype.h g_state.h m_swap.h w_wad.h \
 z_zone.h i_video.h d_netfil.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/wi_stuff.o: wi_stuff.c doomdef.h doomtype.h g_state.h m_swap.h \
 wi_stuff.h d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h \
 d_think.h p_mobj.h doomdata.h d_ticcmd.h g_game.h doomstat.h \
 d_clisrv.h d_netcmd.h command.h d_event.h hu_stuff.h w_wad.h r_defs.h \
 screen.h m_random.h r_local.h m_bbox.h r_main.h r_data.h r_state.h \
 r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h sounds.h r_draw.h \
 s_sound.h st_stuff.h i_video.h v_video.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/v_video.o: v_video.c doomdef.h doomtype.h g_state.h m_swap.h r_local.h \
 tables.h m_fixed.h screen.h command.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_data.h r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h \
 r_things.h sounds.h r_draw.h v_video.h hu_stuff.h d_event.h w_wad.h \
 wi_stuff.h i_video.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/st_lib.o: st_lib.c doomdef.h doomtype.h g_state.h m_swap.h st_lib.h \
 r_defs.h m_fixed.h d_think.h p_mobj.h tables.h doomdata.h info.h \
 screen.h command.h w_wad.h st_stuff.h d_event.h d_player.h d_items.h \
 p_pspr.h d_ticcmd.h v_video.h z_zone.h i_video.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/st_stuff.o: st_stuff.c doomdef.h doomtype.h g_state.h m_swap.h \
 am_map.h d_event.h g_game.h doomstat.h doomdata.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h \
 d_ticcmd.h d_clisrv.h d_netcmd.h command.h m_cheat.h r_local.h \
 screen.h m_bbox.h r_main.h r_data.h r_defs.h r_state.h r_bsp.h \
 r_segs.h r_plane.h r_sky.h r_things.h sounds.h r_draw.h p_local.h \
 p_tick.h p_maputl.h p_spec.h p_inter.h m_random.h st_stuff.h st_lib.h \
 w_wad.h i_video.h v_video.h keys.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/hu_stuff.o: hu_stuff.c doomdef.h doomtype.h g_state.h m_swap.h \
 hu_stuff.h d_event.h w_wad.h wi_stuff.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h doomdata.h d_ticcmd.h \
 r_defs.h screen.h command.h d_netcmd.h d_clisrv.h g_game.h doomstat.h \
 g_input.h keys.h i_video.h dstrings.h d_englsh.h st_stuff.h r_local.h \
 m_bbox.h r_main.h r_data.h r_state.h r_bsp.h r_segs.h r_plane.h \
 r_sky.h r_things.h sounds.h r_draw.h v_video.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/s_sound.o: s_sound.c doomdef.h doomtype.h g_state.h m_swap.h command.h \
 g_game.h doomstat.h doomdata.h d_player.h d_items.h p_pspr.h \
 m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h \
 d_netcmd.h d_event.h m_argv.h r_main.h r_data.h r_defs.h screen.h \
 r_state.h r_things.h sounds.h i_sound.h s_sound.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/z_zone.o: z_zone.c doomdef.h doomtype.h g_state.h m_swap.h z_zone.h \
 i_system.h d_ticcmd.h d_event.h command.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/info.o: info.c doomdef.h doomtype.h g_state.h m_swap.h sounds.h \
 m_fixed.h p_mobj.h tables.h d_think.h doomdata.h info.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/sounds.o: sounds.c doomtype.h sounds.h r_defs.h m_fixed.h d_think.h \
 p_mobj.h tables.h doomdata.h doomdef.h g_state.h m_swap.h info.h \
 screen.h command.h r_things.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/tmap.o: tmap.s asm_defs.inc
	$(CC) $(OPTS) $(SFLAGS) -x assembler-with-cpp -c $< -o $@

$(O)/p_fab.o: p_fab.c doomdef.h doomtype.h g_state.h m_swap.h g_game.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h d_event.h p_local.h m_bbox.h p_tick.h r_defs.h screen.h \
 p_maputl.h p_spec.h r_state.h r_data.h p_fab.h m_random.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/dehacked.o: dehacked.c doomdef.h doomtype.h g_state.h m_swap.h \
 command.h console.h d_event.h g_game.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h sounds.h m_cheat.h \
 dstrings.h d_englsh.h m_argv.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/qmus2mid.o: qmus2mid.c doomdef.h doomtype.h g_state.h m_swap.h \
 i_system.h d_ticcmd.h d_event.h byteptr.h m_swap.h $(qmus2mid_h)
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/qmus2mid_sdl.o: qmus2mid_sdl.c doomdef.h doomtype.h g_state.h m_swap.h \
 i_system.h d_ticcmd.h d_event.h byteptr.h m_swap.h $(qmus2mid_h)
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/vid_copy.o: vid_copy.s asm_defs.inc
	$(CC) $(OPTS) $(SFLAGS) -x assembler-with-cpp -c $< -o $@

$(O)/vid_vesa.o: $(INTERFACE)/vid_vesa.c i_system.h \
 d_ticcmd.h doomtype.h d_event.h \
 g_state.h $(INTERFACE)/vid_vesa.h doomdef.h \
 m_swap.h screen.h command.h \
 st_stuff.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h \
 info.h d_think.h p_mobj.h \
 doomdata.h console.h i_video.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/g_input.o: g_input.c doomdef.h doomtype.h g_state.h m_swap.h g_input.h \
 d_event.h keys.h command.h hu_stuff.h w_wad.h wi_stuff.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h \
 doomdata.h d_ticcmd.h r_defs.h screen.h d_net.h console.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/screen.o: screen.c doomdef.h doomtype.h g_state.h m_swap.h screen.h \
 command.h console.h d_event.h am_map.h i_system.h d_ticcmd.h \
 i_video.h r_local.h tables.h m_fixed.h m_bbox.h r_main.h d_player.h \
 d_items.h p_pspr.h info.h d_think.h p_mobj.h doomdata.h r_data.h \
 r_defs.h r_state.h r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h \
 sounds.h r_draw.h m_argv.h v_video.h st_stuff.h hu_stuff.h w_wad.h \
 wi_stuff.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/console.o: console.c doomdef.h doomtype.h g_state.h m_swap.h console.h \
 d_event.h g_game.h doomstat.h doomdata.h d_player.h d_items.h \
 p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h d_ticcmd.h \
 d_clisrv.h d_netcmd.h command.h g_input.h keys.h hu_stuff.h w_wad.h \
 wi_stuff.h r_defs.h screen.h sounds.h st_stuff.h s_sound.h v_video.h \
 i_video.h z_zone.h i_system.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/command.o: command.c doomdef.h doomtype.h g_state.h m_swap.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h console.h d_event.h z_zone.h m_misc.h w_wad.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/d_netcmd.o: d_netcmd.c doomdef.h doomtype.h g_state.h m_swap.h \
 console.h d_event.h command.h d_netcmd.h d_clisrv.h d_ticcmd.h \
 i_system.h dstrings.h d_englsh.h g_game.h doomstat.h doomdata.h \
 d_player.h d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h \
 p_mobj.h hu_stuff.h w_wad.h wi_stuff.h r_defs.h screen.h g_input.h \
 keys.h m_menu.h r_local.h m_bbox.h r_main.h r_data.h r_state.h \
 r_bsp.h r_segs.h r_plane.h r_sky.h r_things.h sounds.h r_draw.h \
 p_inter.h p_setup.h s_sound.h m_misc.h am_map.h byteptr.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/d_clisrv.o: d_clisrv.c doomdef.h doomtype.h g_state.h m_swap.h \
 command.h i_net.h i_system.h d_ticcmd.h d_event.h d_net.h d_netcmd.h \
 d_clisrv.h d_main.h w_wad.h g_game.h doomstat.h doomdata.h d_player.h \
 d_items.h p_pspr.h m_fixed.h tables.h info.h d_think.h p_mobj.h \
 hu_stuff.h wi_stuff.h r_defs.h screen.h keys.h m_argv.h m_menu.h \
 console.h d_netfil.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/r_splats.o: r_splats.c r_draw.h r_defs.h m_fixed.h doomtype.h \
 d_think.h p_mobj.h tables.h doomdata.h doomdef.h g_state.h m_swap.h \
 info.h screen.h command.h r_main.h d_player.h d_items.h p_pspr.h \
 d_ticcmd.h r_data.h r_state.h r_plane.h r_splats.h w_wad.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/g_state.o: g_state.c g_state.h doomtype.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/d_netfil.o: d_netfil.c doomdef.h doomtype.h g_state.h m_swap.h \
 doomstat.h doomdata.h d_player.h d_items.h p_pspr.h m_fixed.h \
 tables.h info.h d_think.h p_mobj.h d_ticcmd.h d_clisrv.h d_netcmd.h \
 command.h g_game.h d_event.h i_net.h i_system.h m_argv.h d_net.h \
 d_netfil.h z_zone.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/i_main.o: $(INTERFACE)/i_main.c doomdef.h \
 doomtype.h g_state.h m_swap.h \
 m_argv.h d_main.h d_event.h \
 w_wad.h i_system.h d_ticcmd.h
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/%.o: $(INTERFACE)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

$(O)/%.o: hardware/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(WFLAGS) -c $< -o $@

ifdef OS2
$(O)/%.res: os2/%.rc
	rc.exe -i .\os2 -r $< $@
endif

#############################################################
#
#############################################################
