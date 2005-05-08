/* $Id: colors.h,v 1.3 2005/05/08 01:29:19 chris Exp $ */

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

/* Special/background colors */
#define BLACK     0x00000000
#define GREY      0x00bebebe
#define WHITE     0x00ffffff

/* Block colors */
#define BLUE      0x000000ff
#define GREEN     0x00006400
#define ORANGE    0x00ffa500
#define RED       0x00cd0000

#define NCOLORS   4

Uint32 rand_color();

#endif
