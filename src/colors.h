/* $Id: colors.h,v 1.5 2005/06/08 23:58:41 chris Exp $
 *
 * Functions and definitions for manipulating colors.
 */

/* skf - shit keeps falling
 * Copyright (C) 2005 Chris Lumens
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _COLORS_H
#define _COLORS_H 1
#include <SDL/SDL.h>

/* Extract individual RGB values from a single Uint32. */
#define RVAL(c)   ((c) & 0x00ff0000) >> 16
#define GVAL(c)   ((c) & 0x0000ff00) >> 8
#define BVAL(c)    (c) & 0x000000ff

/* Special/background colors */
#define BLACK     0x00000000
#define GREY      0x00bebebe
#define WHITE     0x00ffffff

/* Block colors */
typedef enum { BLUE, GREEN, ORANGE, RED } colors_t;

/* The number of available block colors. */
#define NCOLORS   4

/* Select a random block color from one of the choices listed above. */
colors_t rand_color();

#endif
