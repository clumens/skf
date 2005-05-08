/* $Id: draw.h,v 1.7 2005/05/08 01:29:19 chris Exp $ */

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
#ifndef _DRAW_H
#define _DRAW_H 1

#include <SDL/SDL.h>

void draw_block (SDL_Surface *screen, unsigned int base_x, unsigned int base_y,
                 Uint32 color);
void draw_line (SDL_Surface *screen, unsigned int x1, unsigned int y1,
                unsigned int x2, unsigned int y2, Uint32 color);
void erase_block (SDL_Surface *screen, unsigned int base_x,
                  unsigned int base_y);
void init_screen (SDL_Surface *screen);

#endif
