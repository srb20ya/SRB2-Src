#
#-----------------------------------------------------------------------------
#
# $Id: Makefile,v 1.4 2000/08/21 11:08:46 hurdler Exp $
#
# Copyright (C) 2000 by DooM Legacy Team.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
#
# $Log: Makefile,v $
# Revision 1.4  2000/08/21 11:08:46  hurdler
# Improve the stats
#
# Revision 1.3  2000/08/20 14:12:47  hurdler
# Adding statistics
#
# Revision 1.2  2000/08/19 11:31:03  hurdler
# Adding password and log file access tru server
#
# Revision 1.1.1.1  2000/08/18 10:32:09  hurdler
# Initial import of the Master Server code into CVS
#
#
# DESCRIPTION:
#   Makefile for DooM Legacy Master Server
#   Created on 06/23/2000 by Thierry Van Elsuwe
#   e-mail: hurdler@newdoom.com
#
#-----------------------------------------------------------------------------

PORT = 28900
#PORT = 7896

#*******************************************************************#

CCFILES      = common.cpp ipcs.cpp srvlist.cpp crypt.cpp stats.cpp
Main_CCFILES = server.cpp client.cpp

#******************************************************************#

CC      = g++
CFLAGS  = -Wall -O3
LDFLAGS = -O3
LIBS    = -lm
ifdef MINGW
LIBS   += -lwsock32
endif

ifdef DEBUG
CFLAGS := $(CFLAGS) -g -D__DEBUG__
endif

SRCS        = $(CCFILES) $(Main_CCFILES)
OFILES      = $(CCFILES:.cpp=.o)
Main_OFILES = $(Main_CCFILES:.cpp=.o)
EXEFILE     = $(Main_CCFILES:.cpp=)

#####################################################################

.SUFFIXES:	.cpp .h

default:	init $(EXEFILE) end

init:
		@echo
		@echo "Compiling options:"
		@echo "  CC      : $(CC)"
		@echo "  LDFLAGS : $(LDFLAGS)"
		@echo "  LIBS    : $(LIBS)"
		@echo "  CFLAGS  : $(CFLAGS)"
		@echo 
		@echo "Files:"
		@echo "  SRCS    : $(SRCS)"
		@echo "  OFILES  : $(OFILES) $(Main_OFILES)"
		@echo "  EXEFILE : $(EXEFILE)"
		@echo

end:
		@echo
		@echo "*** Makefile ended successfully  ***"
		@echo

#####################################################################

$(EXEFILE):	$(OFILES) $(Main_OFILES)
		@echo Linking ...
		@$(CC) $(LDFLAGS) $(OFILES) $@.o -o $@ $(LIBS)
ifndef CYGWIN
ifndef MINGW
		@chmod 755 $@
endif
endif
		@echo File $@ has been created

#####################################################################

.cpp.o:	 
		@echo Compiling $< -\> $@
		@$(CC) $(CFLAGS) $(INCS) -c $< -o $@

#####################################################################

debug:		debug_
debug_:
		@make "CFLAGS = $(CFLAGS) -D__DEBUG__"
		@echo

clean:		clean_
clean_:
		@echo
		@echo Removing obejcts files, executable files and eventually core file...
		@rm -f $(OFILES) $(Main_OFILES) $(EXEFILE) core
		@echo

realclean:	clean
		@echo Removing backup files...
		@rm -f *~ *.bak
		@echo

depend:		dep
dep:
		@echo
		@echo Make dependencies...
		@makedepend  -- $(CFLAGS) -- $(Main_CCFILES) $(CCFILES) 2> /dev/null
		@rm -f Makefile.bak
		@echo

BAKFILE = ../SRB2MasterServer
backup:		bak
bak:		clean
		@echo Copy files to $(BAKFILE).tgz...
		@tar cvf $(BAKFILE).tar * > /dev/null
		@gzip $(BAKFILE).tar
		@mv $(BAKFILE).tar.gz $(BAKFILE).tgz
		@echo Removing backup files...
		@rm -f *~ *.bak
		@echo

#mrproper:      dep clean default
mrproper:       clean default

install:	mrproper
		@echo "Launching the server, port $(PORT)..."
		@./server $(PORT)
		@echo
