/* $Id: skf.c,v 1.3 2005/05/06 02:57:01 chris Exp $ */

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

#include "draw.h"
#include "skf.h"

/* Get the best color depth we have available. */
Uint32 __inline__ best_color_depth()
{
   const SDL_VideoInfo *info = SDL_GetVideoInfo();

   if (info != NULL)
      return info->vfmt->BitsPerPixel;
   else
      return 8;
}

/* Is there a window manager? */
unsigned int __inline__ have_wm()
{
   const SDL_VideoInfo *info = SDL_GetVideoInfo();

   if (info != NULL)
      return info->wm_available;
   else
      return 0;
}

void test_draw (SDL_Surface *screen)
{
   unsigned int x, y;
   Uint8 R = 0x00, G = 0x00, B = 0x00;

   for (y = 0; y < Y_BLOCKS; y++)
   {
      B = (y % 2 == 0) ? 0x00 : 0xff;

      for (x = 0; x < X_BLOCKS; x++)
      {
         R = (x % 2 == 0) ? 0x00 : 0xff;
         G = (x % 2 == 1) ? 0x00 : 0xff;
         draw_block (screen, x*BLOCK_SIZE, y*BLOCK_SIZE, R, G, B);
      }
   }
}

int main (int argc, char **argv)
{
   SDL_Surface *screen;
   SDL_Event evt;

   if (SDL_Init (SDL_INIT_VIDEO) < 0)
   {
      fprintf (stderr, "unable to init SDL:  %s\n", SDL_GetError());
      exit(1);
   }

   atexit (SDL_Quit);

   screen = SDL_SetVideoMode (SKF_XRES, SKF_YRES, best_color_depth(),
                              SDL_HWSURFACE|SDL_DOUBLEBUF);

   if (screen == NULL)
   {
      fprintf (stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
      exit(1);
   }

   if (have_wm())
      SDL_WM_SetCaption("shit keeps falling", "skf");

   test_draw(screen);

   do {
      SDL_WaitEvent (&evt);
   } while (evt.type != SDL_QUIT);

   return 0;
}
