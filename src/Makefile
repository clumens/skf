# $Id: Makefile,v 1.12 2005/06/08 03:01:12 chris Exp $

include ../Includes.mk

CC = gcc
CFLAGS = -O2 -Wall `sdl-config --cflags`
LDFLAGS = `sdl-config --libs` -lSDL_image

OBJS = 4block.o clock.o colors.o draw.o lblock.o plusblock.o sblock.o skf.o \
		 ublock.o

all: skf

skf: CFLAGS += -DSKF_IMAGE_DIR=\"$(IMGDIR)\"
skf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: CFLAGS += -DSKF_IMAGE_DIR=\"../graphics\"
test: $(OBJS)
	$(CC) $(CFLAGS) -o skf $^ $(LDFLAGS)

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
