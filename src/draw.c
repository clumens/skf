/* $Id: draw.c,v 1.10 2005/05/09 02:47:26 chris Exp $ */

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

#include "colors.h"
#include "draw.h"
#include "skf.h"

/* +=====================================================================+
 * | PRIVATE FUNCTIONS                                                   |
 * +=====================================================================+
 */

/* Draw one pixel to the video framebuffer.  screen must be locked before
 * calling this function.
 */
static void __inline__ draw_pixel (SDL_Surface *screen, Uint8 R, Uint8 G,
                                   Uint8 B, Uint32 x, Uint32 y)
{
   Uint32 *bufp = (Uint32 *) screen->pixels + y*screen->pitch/4 + x;
   *bufp = SDL_MapRGB(screen->format, R, G, B);
}

static Uint32 __inline__ get_pixel (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + y * screen->pitch + x * 4;
   return *(Uint32 *)p;
}

/* +=====================================================================+
 * | PUBLIC FUNCTIONS                                                    |
 * +=====================================================================+
 */
void copy_block (SDL_Surface *screen, Uint32 src_x, Uint32 src_y,
                 Uint32 dest_x, Uint32 dest_y)
{
   int x, y;

   /* Make sure to lock before drawing. */
   if (SDL_MUSTLOCK(screen))
   {
      if (SDL_LockSurface(screen) < 0)
         return;
   }

   /* Copy all the pixels. */
   for (y = 0; y < BLOCK_SIZE; y++)
   {
      for (x = 0; x < BLOCK_SIZE; x++)
      {
         Uint32 color = get_pixel (screen, src_x+x, src_y+y);

         draw_pixel (screen, RVAL(color), GVAL(color), BVAL(color),
                     dest_x+x, dest_y+y);
      }
   }

   /* Give up the lock. */
   if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);

   /* Update the rectangular region we just drew. */
   SDL_UpdateRect (screen, dest_x, dest_y, BLOCK_SIZE, BLOCK_SIZE);
}

void draw_block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y,
                 Uint32 color)
{
   Uint32 x, y;

   Uint8 R = RVAL(color);
   Uint8 G = GVAL(color);
   Uint8 B = BVAL(color);

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
         draw_pixel (screen, R, G, B, base_x+x, base_y+y);
   }

   /* A little decoration would be nice. */
   draw_line (screen, base_x+3, base_y+3, base_x+BLOCK_SIZE-4, base_y+3, BLACK);
   draw_line (screen, base_x+3, base_y+3, base_x+3, base_y+BLOCK_SIZE-4, BLACK);
   draw_line (screen, base_x+3, base_y+BLOCK_SIZE-4, base_x+BLOCK_SIZE-4,
              base_y+BLOCK_SIZE-4, BLACK);
   draw_line (screen, base_x+BLOCK_SIZE-4, base_y+3, base_x+BLOCK_SIZE-4,
              base_y+BLOCK_SIZE-4, BLACK);

   /* Give up the lock. */
   if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);

   /* Update the rectangular region we just drew. */
   SDL_UpdateRect (screen, base_x, base_y, BLOCK_SIZE, BLOCK_SIZE);
}

void draw_4block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y,
                  Uint32 color)
{
   draw_block (screen, base_x, base_y, color);
   draw_block (screen, base_x+BLOCK_SIZE, base_y, color);
   draw_block (screen, base_x, base_y+BLOCK_SIZE, color);
   draw_block (screen, base_x+BLOCK_SIZE, base_y+BLOCK_SIZE, color);
}

void draw_line (SDL_Surface *screen, Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2,
                Uint32 color)
{
   Uint8 R = RVAL(color);
   Uint8 G = GVAL(color);
   Uint8 B = BVAL(color);

   Uint32 i;

   if (x1 == x2)
   {
      /* vertical line */
      for (i = y1; i <= y2; i++)
         draw_pixel (screen, R, G, B, x1, i);
   }
   else if (y1 == y2)
   {
      /* horizontal line */
      for (i = x1; i <= x2; i++)
         draw_pixel (screen, R, G, B, i, y1);
   }
   else
      return;
}

void erase_block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y)
{
   SDL_Rect r = { base_x, base_y, BLOCK_SIZE, BLOCK_SIZE };

   SDL_FillRect (screen, &r, GREY);
   SDL_UpdateRect (screen, base_x, base_y, BLOCK_SIZE, BLOCK_SIZE);
}

void erase_4block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y)
{
   erase_block (screen, base_x, base_y);
   erase_block (screen, base_x+BLOCK_SIZE, base_y);
   erase_block (screen, base_x, base_y+BLOCK_SIZE);
   erase_block (screen, base_x+BLOCK_SIZE, base_y+BLOCK_SIZE);
}

void init_screen (SDL_Surface *screen)
{
   /* Make sure to lock before drawing. */
   if (SDL_MUSTLOCK(screen))
   {
      if (SDL_LockSurface(screen) < 0)
         return;
   }

   SDL_FillRect (screen, NULL, GREY);
   SDL_Flip(screen);

   /* Give up the lock. */
   if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);
}

void x_out_block (SDL_Surface *screen, Uint32 base_x, Uint32 base_y)
{
   Uint32 i;

   Uint8 R = RVAL(WHITE);
   Uint8 G = GVAL(WHITE);
   Uint8 B = BVAL(WHITE);

   /* Make sure to lock before drawing. */
   if (SDL_MUSTLOCK(screen))
   {
      if (SDL_LockSurface(screen) < 0)
         return;
   }

   for (i = 0; i < BLOCK_SIZE; i++)
      draw_pixel (screen, R, G, B, base_x+i, base_y+i);

   for (i = 0; i < BLOCK_SIZE; i++)
      draw_pixel (screen, R, G, B, base_x+i, base_y+BLOCK_SIZE-i);

   /* Give up the lock. */
   if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);

   /* Update the rectangular region we just drew. */
   SDL_UpdateRect (screen, base_x, base_y, BLOCK_SIZE, BLOCK_SIZE);
}
