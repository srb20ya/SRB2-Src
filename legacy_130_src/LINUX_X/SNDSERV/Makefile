##########################################################
#
# $Id: Makefile,v 1.4 2000/04/30 19:50:37 metzgermeister Exp $
#
# $Log: Makefile,v $
# Revision 1.4  2000/04/30 19:50:37  metzgermeister
# no message
#
# Revision 1.3  2000/03/12 22:34:07  linuxcub
# Fixed makefile (leading spaces converted to tabs)
#
# Revision 1.2  2000/02/27 00:42:12  hurdler
# fix CR+LF problem
#
# Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
# Initial import into CVS (v1.29 pr3)
#
#
#

CC=gcc
CFLAGS=-O2 -m486 -Wall -DNORMALUNIX -DLINUX
LDFLAGS=
LIBS=-lm

O=linux

all:     $(O)/llsndserv

clean:
	rm -f $(O)/*

# Target
$(O)/llsndserv: \
	$(O)/soundsrv.o \
	$(O)/sounds.o \
	$(O)/linux.o
	$(CC) $(CFLAGS) $(LDFLAGS) \
	$(O)/soundsrv.o \
	$(O)/sounds.o \
	$(O)/linux.o -o $(O)/llsndserv $(LIBS)
	echo make complete.

# Rule
$(O)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
