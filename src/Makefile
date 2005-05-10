# $Id: Makefile,v 1.6 2005/05/10 03:42:17 chris Exp $

CC = gcc
CFLAGS = -O2 -Wall `sdl-config --cflags`
LDFLAGS = `sdl-config --libs`

OBJS = 4block.o colors.o draw.o sblock.o skf.o

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

colors.o: colors.c \
	colors.h

draw.o: draw.c \
	colors.h draw.h skf.h

sblock.o: sblock.c \
	blocks.h colors.h draw.h skf.h

skf.o: skf.c \
	blocks.h colors.h skf.h
