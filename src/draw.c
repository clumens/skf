/* $Id: draw.c,v 1.21 2005/06/15 03:29:00 chris Exp $ */

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
#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "colors.h"
#include "draw.h"
#include "skf.h"

#define LOCK(surface) \
   do { \
      if (SDL_MUSTLOCK(surface)) \
      { \
         if (SDL_LockSurface(surface) < 0) \
            return; \
      } \
   } while (0)

#define UNLOCK(surface) \
   do { \
      if (SDL_MUSTLOCK(surface)) \
         SDL_UnlockSurface(surface); \
   } while (0)

/* Function pointers to color depth-specific drawing functions. */
static void (*draw_pixel) (SDL_Surface *screen, Uint32 color, Uint32 x,
                           Uint32 y);
static Uint32 (*get_pixel) (SDL_Surface *screen, Uint32 x, Uint32 y);

/* Images for each color of block we support. */
SDL_Surface *blue_img, *green_img, *orange_img, *red_img;
SDL_Surface *cracks_img;

/* +=====================================================================+
 * | PRIVATE FUNCTIONS                                                   |
 * +=====================================================================+
 */

/* I'm betting it's faster to have one function per depth and dereference
 * through a function pointer than to have the switch run through for every
 * pixel.
 */
static void __inline__ draw_pixel1 (SDL_Surface *screen, Uint32 color,
                                    Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + x;
   *p = color;
}

static void __inline__ draw_pixel2 (SDL_Surface *screen, Uint32 color,
                                    Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*2);
   *(Uint16 *) p = color;
}

static void __inline__ draw_pixel3l (SDL_Surface *screen, Uint32 color,
                                     Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*3);
   p[0] = BVAL(color);
   p[1] = GVAL(color);
   p[2] = RVAL(color);
}

static void __inline__ draw_pixel3b (SDL_Surface *screen, Uint32 color,
                                     Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*3);
   p[0] = RVAL(color);
   p[1] = GVAL(color);
   p[2] = BVAL(color);
}

static void __inline__ draw_pixel4 (SDL_Surface *screen, Uint32 color,
                                    Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*4);
   *(Uint32 *) p = color;
}

static Uint32 __inline__ get_pixel1 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + x;
   return *p;
}

static Uint32 __inline__ get_pixel2 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*2);
   return *(Uint16 *) p;
}

static Uint32 __inline__ get_pixel3b (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*3);
   return p[0] << 16 | p[1] << 8 | p[2];
}

static Uint32 __inline__ get_pixel3l (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*3);
   return p[0] | p[1] << 8 | p[2] << 16;
}

static Uint32 __inline__ get_pixel4 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   Uint8 *p = (Uint8 *) screen->pixels + (y*screen->pitch) + (x*4);
   return *(Uint32 *) p;
}

/* +=====================================================================+
 * | PUBLIC FUNCTIONS                                                    |
 * +=====================================================================+
 */
void copy_block (SDL_Surface *screen, int src_x, int src_y, int dest_x,
                 int dest_y)
{
   int x, y;

   Uint32 src_px_x = B2P(src_x);
   Uint32 src_px_y = B2P(src_y);
   Uint32 dest_px_x = B2P(dest_x);
   Uint32 dest_px_y = B2P(dest_y);

   LOCK(screen);

   /* Copy all the pixels. */
   for (y = 0; y < BLOCK_SIZE; y++)
   {
      for (x = 0; x < BLOCK_SIZE; x++)
      {
         Uint32 pixel = get_pixel (screen, FIELD_X(src_px_x+x),
                                   FIELD_Y(src_px_y+y));
         draw_pixel (screen, pixel, FIELD_X(dest_px_x+x), FIELD_Y(dest_px_y+y));
      }
   }

   UNLOCK(screen);

   /* Update the rectangular region we just drew. */
   SDL_UpdateRect (screen, FIELD_X(dest_px_x), FIELD_Y(dest_px_y),
                   BLOCK_SIZE, BLOCK_SIZE);
}

void draw_block (SDL_Surface *screen, int base_x, int base_y, colors_t color)
{
   Uint32 px_x = B2P(base_x);
   Uint32 px_y = B2P(base_y);

   SDL_Rect r = { FIELD_X(px_x), FIELD_Y(px_y), BLOCK_SIZE, BLOCK_SIZE };
   SDL_Surface *src;

   switch (color) {
      case BLUE:
         src = blue_img;
         break;

      case GREEN:
         src = green_img;
         break;

      case ORANGE:
         src = orange_img;
         break;

      case RED:
      default:
         src = red_img;
         break;
   }

   SDL_BlitSurface (src, NULL, screen, &r);
   SDL_UpdateRect (screen, FIELD_X(px_x), FIELD_Y(px_y), BLOCK_SIZE,
                   BLOCK_SIZE);
}

void draw_line (SDL_Surface *screen, Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2,
                Uint32 color)
{
   Uint32 i;

   if (x1 == x2)
   {
      /* vertical line */
      for (i = y1; i <= y2; i++)
         draw_pixel (screen, color, x1, i);
   }
   else if (y1 == y2)
   {
      /* horizontal line */
      for (i = x1; i <= x2; i++)
         draw_pixel (screen, color, i, y1);
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
   SDL_Rect r;
   SDL_Surface *border_img;

   switch (best_color_depth()/8) {
      case 1:
         draw_pixel = draw_pixel1;
         get_pixel = get_pixel1;
         break;

      case 2:
         draw_pixel = draw_pixel2;
         get_pixel = get_pixel2;
         break;

      case 3:
         if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
         {
            draw_pixel = draw_pixel3b;
            get_pixel = get_pixel3b;
         }
         else
         {
            draw_pixel = draw_pixel3l;
            get_pixel = get_pixel3l;
         }
         break;

      case 4:
         draw_pixel = draw_pixel4;
         get_pixel = get_pixel4;
         break;
   }

   LOCK(screen);

   r.x = CLOCK_XOFFSET;
   r.y = CLOCK_YOFFSET;
   r.w = CLOCK_XRES;
   r.h = CLOCK_YRES;

   SDL_FillRect (screen, &r, GREY);

   /* Playing field */
   r.x = FIELD_XOFFSET;
   r.y = FIELD_YOFFSET;
   r.w = FIELD_XRES;
   r.h = FIELD_YRES;

   SDL_FillRect (screen, &r, GREY);

   border_img = load_img ("border.png");
   SDL_BlitSurface (border_img, NULL, screen, NULL);
   SDL_Flip(screen);

   UNLOCK(screen);

   blue_img = load_img ("blue_block.png");
   green_img = load_img ("green_block.png");
   orange_img = load_img ("orange_block.png");
   red_img = load_img ("red_block.png");

   cracks_img = load_img ("cracks.png");
}

SDL_Surface *load_img (const char *img)
{
   SDL_Surface *retval;
   char buf[255];

   snprintf (buf, 254, "%s/%s", SKF_IMAGE_DIR, img);

   if ((retval = IMG_Load (buf)) == NULL)
   {
      fprintf (stderr, "Unable to load image: %s\n", IMG_GetError());
      exit(1);
   }

   return retval;
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

void x_out_block (SDL_Surface *screen, int x, int y)
{
   SDL_Rect r = { FIELD_X(B2P(x)), FIELD_Y(B2P(y)), BLOCK_SIZE, BLOCK_SIZE };

   LOCK(screen);
   SDL_BlitSurface (cracks_img, NULL, screen, &r);
   UNLOCK(screen);
   SDL_UpdateRect (screen, r.x, r.y, BLOCK_SIZE, BLOCK_SIZE);
}
