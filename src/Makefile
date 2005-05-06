# $Id: Makefile,v 1.2 2005/05/06 02:38:06 chris Exp $

CC = gcc
CFLAGS = -O2 -Wall `sdl-config --cflags`
LDFLAGS = `sdl-config --libs`

OBJS = draw.o skf.o

all: skf

skf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-rm core skf *.o

#####################################################################
# Dependancies                                                      #
#####################################################################
draw.o: draw.c \
	draw.h skf.h

skf.o: skf.c \
	skf.h
