/* $Id: skf.c,v 1.5 2005/05/07 16:51:18 chris Exp $ */

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

/* Types for user-defined events. */
#define USER_EVT_DROP   0

unsigned int field[Y_BLOCKS];

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

/* Draw one line of boxes on the specified row. */
void test_draw (SDL_Surface *screen, unsigned int *on_row)
{
   unsigned int x;

   /* If the entire screen is now full, the game is over. */
   if (field[0] == 1)
   {
      fprintf (stderr, "game over\n");
      exit(0);
   }

   /* Draw the new line of blocks. */
   for (x = 0; x < X_BLOCKS; x++)
      draw_block (screen, x*BLOCK_SIZE, (*on_row)*BLOCK_SIZE, 0x00, 0xff, 0xff);

   /* Erase the space where the line used to be. */
   if (*on_row > 0)
   {
      for (x = 0; x < X_BLOCKS; x++)
         draw_block (screen, x*BLOCK_SIZE, (*on_row-1)*BLOCK_SIZE, 0, 0, 0);
   }

   /* If this is the last free row, reset to start dropping from the top. */
   if (*on_row == Y_BLOCKS-1 || field[*on_row+1] == 1)
   {
      field[*on_row] = 1;
      *on_row = 0;
      return;
   }

   (*on_row)++;

   if (*on_row >= Y_BLOCKS)
      *on_row = 0;
}

void init_field ()
{
   unsigned int y;

   for (y = 0; y < Y_BLOCKS; y++)
      field[y] = 0;
}

/* Callback function for timer - just put an event into the queue and later,
 * it will be processed to do the drawing.  We're not supposed to call functions
 * from within the callback (stupid interrupt handler model).
 */
Uint32 drop_timer_cb (Uint32 interval, void *params)
{
   SDL_Event evt;

   evt.type = SDL_USEREVENT;
   evt.user.code = USER_EVT_DROP;
   evt.user.data1 = NULL;
   evt.user.data2 = NULL;
   SDL_PushEvent (&evt);

   /* Return the interval so we know to reschedule the timer. */
   return interval;
}

int main (int argc, char **argv)
{
   SDL_Surface *screen;
   SDL_Event evt;

   unsigned int on_row = 0;

   if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
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
      SDL_WM_SetCaption("shit keeps falling - v.20050505", "skf");

   init_field();

   /* Set up a callback to add to the playing field every so often. */
   if (SDL_AddTimer (500, drop_timer_cb, NULL) == NULL)
   {
      fprintf (stderr, "Unable to set up timer callback: %s\n", SDL_GetError());
      exit(1);
   }

   do {
      SDL_WaitEvent (&evt);

      switch (evt.type) {
         case SDL_USEREVENT:
            if (evt.user.code == USER_EVT_DROP)
               test_draw (screen, &on_row);
            else
               fprintf (stderr, "got unknown user event type\n");
            break;

         default:
            break;
      }
   } while (evt.type != SDL_QUIT);

   return 0;
}
