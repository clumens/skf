# $Id: Makefile,v 1.5 2005/05/08 01:07:34 chris Exp $

CC = gcc
CFLAGS = -O2 -Wall `sdl-config --cflags`
LDFLAGS = `sdl-config --libs`

OBJS = colors.o draw.o skf.o

all: skf

skf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-rm core* skf *.o

#####################################################################
# Dependancies                                                      #
#####################################################################
colors.o: colors.c \
	colors.h

draw.o: draw.c \
	colors.h draw.h skf.h

skf.o: skf.c \
	colors.h skf.h
