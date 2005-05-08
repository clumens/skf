# $Id: Makefile,v 1.4 2005/05/08 00:26:43 chris Exp $

CC = gcc
CFLAGS = -O2 -Wall `sdl-config --cflags`
LDFLAGS = `sdl-config --libs`

OBJS = draw.o skf.o

all: skf

skf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	-rm core* skf *.o

#####################################################################
# Dependancies                                                      #
#####################################################################
draw.o: draw.c \
	colors.h draw.h skf.h

skf.o: skf.c \
	colors.h skf.h
