/* $Id: draw.c,v 1.15 2005/05/17 00:54:01 chris Exp $ */

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
#include <stdlib.h>
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
static void __inline__ draw_pixel (SDL_Surface *screen, unsigned int bpp,
                                   Uint32 color, Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*bpp);

   switch (bpp) {
      case 1:
         *p = color;
         break;

      case 2:
         *(Uint16 *) p = color;
         break;

      case 3:
         if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
         {
            p[0] = RVAL(color);
            p[1] = GVAL(color);
            p[2] = BVAL(color);
         }
         else
         {
            p[0] = BVAL(color);
            p[1] = GVAL(color);
            p[2] = RVAL(color);
         }
         break;

      case 4:
         *(Uint32 *) p = color;
         break;
   }
}

static Uint32 __inline__ get_pixel (SDL_Surface *screen, unsigned int bpp,
                                    Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*bpp);

   switch (bpp) {
      case 1:
         return *p;

      case 2:
         return *(Uint16 *) p;

      case 3:
         if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
         else
            return p[0] | p[1] << 8 | p[2] << 16;

      case 4:
         return *(Uint32 *) p;

      default:
         return 0;
   }
}

/* +=====================================================================+
 * | PUBLIC FUNCTIONS                                                    |
 * +=====================================================================+
 */
void copy_block (SDL_Surface *screen, int src_x, int src_y, int dest_x,
                 int dest_y)
{
   int x, y;
   unsigned int bpp = best_color_depth()/8;

   Uint32 src_px_x = B2P(src_x);
   Uint32 src_px_y = B2P(src_y);
   Uint32 dest_px_x = B2P(dest_x);
   Uint32 dest_px_y = B2P(dest_y);

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
         Uint32 pixel = get_pixel (screen, bpp, FIELD_X(src_px_x+x),
                                   FIELD_Y(src_px_y+y));
         draw_pixel (screen, bpp, pixel, FIELD_X(dest_px_x+x),
                     FIELD_Y(dest_px_y+y));
      }
   }

   /* Give up the lock. */
   if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);

   /* Update the rectangular region we just drew. */
   SDL_UpdateRect (screen, FIELD_X(dest_px_x), FIELD_Y(dest_px_y),
                   BLOCK_SIZE, BLOCK_SIZE);
}

void draw_block (SDL_Surface *screen, int base_x, int base_y, Uint32 color)
{
   int x, y;
   unsigned int bpp = best_color_depth()/8;

   Uint32 px_x = B2P(base_x);
   Uint32 px_y = B2P(base_y);

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
         draw_pixel (screen, bpp, color, FIELD_X(px_x+x), FIELD_Y(px_y+y));
   }

   /* A little decoration would be nice. */
   draw_line (screen, FIELD_X(px_x+3), FIELD_Y(px_y+3),
              FIELD_X(px_x+BLOCK_SIZE-4), FIELD_Y(px_y+3), BLACK);
   draw_line (screen, FIELD_X(px_x+3), FIELD_Y(px_y+3), FIELD_X(px_x+3),
              FIELD_Y(px_y+BLOCK_SIZE-4), BLACK);
   draw_line (screen, FIELD_X(px_x+3),FIELD_Y(px_y+BLOCK_SIZE-4),
              FIELD_X(px_x+BLOCK_SIZE-4), FIELD_Y(px_y+BLOCK_SIZE-4), BLACK);
   draw_line (screen, FIELD_X(px_x+BLOCK_SIZE-4), FIELD_Y(px_y+3),
              FIELD_X(px_x+BLOCK_SIZE-4), FIELD_Y(px_y+BLOCK_SIZE-4), BLACK);

   /* Give up the lock. */
   if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);

   /* Update the rectangular region we just drew. */
   SDL_UpdateRect (screen, FIELD_X(px_x), FIELD_Y(px_y), BLOCK_SIZE,
                   BLOCK_SIZE);
}

void draw_line (SDL_Surface *screen, Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2,
                Uint32 color)
{
   unsigned int bpp = best_color_depth()/8;
   Uint32 i;

   if (x1 == x2)
   {
      /* vertical line */
      for (i = y1; i <= y2; i++)
         draw_pixel (screen, bpp, color, x1, i);
   }
   else if (y1 == y2)
   {
      /* horizontal line */
      for (i = x1; i <= x2; i++)
         draw_pixel (screen, bpp, color, i, y1);
   }
   else
      return;
}

void erase_block (SDL_Surface *screen, int base_x, int base_y)
{
   SDL_Rect r = { FIELD_X(B2P(base_x)), FIELD_Y(B2P(base_y)), BLOCK_SIZE,
                  BLOCK_SIZE };

   SDL_FillRect (screen, &r, GREY);
   SDL_UpdateRect (screen, r.x, r.y, BLOCK_SIZE, BLOCK_SIZE);
}

void flip_region (SDL_Surface *src, SDL_Surface *dest, Uint32 x, Uint32 y,
                  Uint32 width, Uint32 height)
{
   SDL_Rect *src_rect, *dest_rect;

   src_rect = malloc (sizeof(SDL_Rect));
   dest_rect = malloc (sizeof(SDL_Rect));

   src_rect->x = dest_rect->x = x;
   src_rect->y = dest_rect->y = y;
   src_rect->w = width;
   src_rect->h = height;

   SDL_BlitSurface (src, src_rect, dest, dest_rect);

   free(src_rect);
   free(dest_rect);

   SDL_UpdateRect (dest, x, y, width, height);
}

void flip_screen (SDL_Surface *src, SDL_Surface *dest)
{
   SDL_BlitSurface (src, NULL, dest, NULL);
   SDL_UpdateRect (dest, 0, 0, 0, 0);
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

SDL_Surface *save_region (SDL_Surface *src, Uint32 base_x, Uint32 base_y,
                          Uint32 width, Uint32 height)
{
   SDL_Surface *retval;
   SDL_Rect *src_rect;

   /* Create the new surface with the same parameters as the source. */
   retval = SDL_CreateRGBSurface (SDL_SWSURFACE, width, height,
                                  src->format->BitsPerPixel, src->format->Rmask,
                                  src->format->Gmask, src->format->Bmask,
                                  src->format->Amask);

   src_rect = malloc (sizeof(SDL_Rect));
   src_rect->x = base_x;
   src_rect->y = base_y;
   src_rect->w = width;
   src_rect->h = height;

   if (SDL_BlitSurface (src, src_rect, retval, NULL) == -1)
   {
      SDL_FreeSurface (retval);
      free(src_rect);
      return NULL;
   }
   else
   {
      free(src_rect);
      return retval;
   }
}

void x_out_block (SDL_Surface *screen, int base_x, int base_y)
{
   Uint32 i;
   unsigned int bpp = best_color_depth()/8;

   Uint32 px_x = B2P(base_x);
   Uint32 px_y = B2P(base_y);

   /* Make sure to lock before drawing. */
   if (SDL_MUSTLOCK(screen))
   {
      if (SDL_LockSurface(screen) < 0)
         return;
   }

   for (i = 0; i < BLOCK_SIZE; i++)
      draw_pixel (screen, bpp, WHITE, FIELD_X(px_x+i), FIELD_Y(px_y+i));

   for (i = 0; i < BLOCK_SIZE; i++)
      draw_pixel (screen, bpp, WHITE, FIELD_X(px_x+i),
                  FIELD_Y(px_y+BLOCK_SIZE-i));

   /* Give up the lock. */
   if (SDL_MUSTLOCK(screen))
      SDL_UnlockSurface(screen);

   /* Update the rectangular region we just drew. */
   SDL_UpdateRect (screen, FIELD_X(px_x), FIELD_Y(px_y), BLOCK_SIZE,
                   BLOCK_SIZE);
}
