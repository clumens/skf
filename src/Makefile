# $Id: Makefile,v 1.11 2005/06/06 03:44:35 chris Exp $

CC = gcc
CFLAGS = -O2 -Wall `sdl-config --cflags`
LDFLAGS = `sdl-config --libs`

OBJS = 4block.o clock.o colors.o draw.o lblock.o plusblock.o sblock.o skf.o \
		 ublock.o

all: skf

skf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-rm core* skf *.o

#####################################################################
# Dependancies                                                      #
#####################################################################
4block.o: 4block.c \
	blocks.h colors.h draw.h skf.h

clock.o: clock.c \
	colors.h draw.h skf.h

colors.o: colors.c \
	colors.h skf.h

draw.o: draw.c \
	colors.h draw.h skf.h

lblock.o: lblock.c \
	blocks.h colors.h draw.h skf.h

plusblock.o: plusblock.c \
	blocks.h colors.h draw.h skf.h

sblock.o: sblock.c \
	blocks.h colors.h draw.h skf.h

skf.o: skf.c \
	blocks.h clock.h colors.h skf.h

ublock.o: ublock.c \
	blocks.h colors.h draw.h skf.h
