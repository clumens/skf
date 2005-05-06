/* $Id: draw.c,v 1.1 2005/05/06 02:38:06 chris Exp $ */

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
#include <SDL/SDL.h>

#include "skf.h"

/* Draw one pixel to the video framebuffer. */
static void __inline__ draw_pixel (SDL_Surface *screen, Uint8 R, Uint8 G,
                                   Uint8 B, unsigned int x, unsigned int y)
{
   Uint32 *bufp = (Uint32 *) screen->pixels + y*screen->pitch/4 + x;
   *bufp = SDL_MapRGB(screen->format, R, G, B);
}

/* Draw one block. */
void draw_block (SDL_Surface *screen, unsigned int base_x, unsigned int base_y)
{
   unsigned int x, y;

   /* Make sure to lock before drawing. */
   if (SDL_MUSTLOCK(screen))
   {
      if (SDL_LockSurface(screen) < 0)
         return;
   }

   /* Draw all the pixels. */
   for (y = 0; y < BLOCK_SIZE; y++)
   {
      for (x = 0; x < BLOCK_SIZE; x++)
         draw_pixel (screen, 0xff, 0x00, 0x00, base_x+x, base_y+y);
   }

   /* Give up the lock. */
   if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);

   /* Update the rectangular region we just drew. */
   SDL_UpdateRect (screen, base_x, base_y, BLOCK_SIZE, BLOCK_SIZE);
}
