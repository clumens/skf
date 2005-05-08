/* $Id: skf.c,v 1.10 2005/05/08 01:29:19 chris Exp $ */

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
#include <time.h>
#include <SDL/SDL.h>

#include "colors.h"
#include "draw.h"
#include "skf.h"

typedef struct {
   unsigned int new;
   int x, y;
   int dx, dy;
   Uint32 color;
} block_t;

/* Types for user-defined events. */
#define USER_EVT_DROP   0

/* A representation of the playing field so we can easily tell if there's
 * something in that space or not.  This is better than relying on whatever
 * we happened to draw there.
 */
unsigned int field[X_BLOCKS][Y_BLOCKS];

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

/* Is there anything underneath the block starting at (x, y)? */
unsigned int __inline__ landed (unsigned int x, unsigned int y)
{
   if (y == Y_BLOCKS-2 || field[x][y+2] == 1 || field[x+1][y+2] == 1)
      return 1;
   else
      return 0;
}

/* Update the position of the currently dropping block on the playing field. */
void update_block (SDL_Surface *screen, block_t *block)
{
   if (landed (block->x, 0))
   {
      fprintf (stderr, "game over\n");
      exit(0);
   }

   /* Only try to erase the previous position if it's not a new block.  If it's
    * new, it obviously just got here and doesn't have a previous position.
    */
   if (block->new)
      block->new = 0;
   else
   {
      /* Erase the previous position of the block. */
      erase_block (screen, block->x*BLOCK_SIZE, block->y*BLOCK_SIZE);
      erase_block (screen, (block->x+1)*BLOCK_SIZE, block->y*BLOCK_SIZE);
      erase_block (screen, block->x*BLOCK_SIZE, (block->y+1)*BLOCK_SIZE);
      erase_block (screen, (block->x+1)*BLOCK_SIZE, (block->y+1)*BLOCK_SIZE);

      block->y += block->dy;
   }

   /* Update x position, making sure the new position makes sense. */
   block->x += block->dx;

   if (block->x < 0)
      block->x = 0;
   else if (block->x > X_BLOCKS-2)
      block->x = X_BLOCKS-2;

   /* Draw the block in its new position. */
   draw_block (screen, block->x*BLOCK_SIZE, block->y*BLOCK_SIZE,
               block->color);
   draw_block (screen, (block->x+1)*BLOCK_SIZE, block->y*BLOCK_SIZE,
               block->color);
   draw_block (screen, block->x*BLOCK_SIZE, (block->y+1)*BLOCK_SIZE,
               block->color);
   draw_block (screen, (block->x+1)*BLOCK_SIZE, (block->y+1)*BLOCK_SIZE,
               block->color);

   /* If the block landed somewhere, reset for dropping the next one. */
   if (landed (block->x, block->y))
   {
      /* Make sure that chunk of the field is in use. */
      field[block->x][block->y] = 1;
      field[block->x+1][block->y] = 1;
      field[block->x][block->y+1] = 1;
      field[block->x+1][block->y+1] = 1;

      /* Create a new block. */
      block->new = 1;
      block->x = (X_BLOCKS-1) / 2;
      block->y = 0;
      block->dx = 0;
      block->dy = 0;
      block->color = rand_color();
   }
}

void init_field ()
{
   unsigned int x, y;

   for (y = 0; y < Y_BLOCKS; y++)
   {
      for (x = 0; x < X_BLOCKS; x++)
         field[x][y] = 0;
   }
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
   block_t block = { 1, (X_BLOCKS-1) / 2, 0, 0, 0, BLUE };

   /* Seed RNG for picking random colors, among other things. */
   srand (time(NULL));

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
      fprintf (stderr, "Unable to set video mode: %s\n", SDL_GetError());
      exit(1);
   }

   if (have_wm())
      SDL_WM_SetCaption("shit keeps falling - v.20050507", "skf");

   init_field();
   init_screen(screen);

   /* Set up a callback to update the playing field every so often. */
   if (SDL_AddTimer (500, drop_timer_cb, NULL) == NULL)
   {
      fprintf (stderr, "Unable to set up timer callback: %s\n", SDL_GetError());
      exit(1);
   }

   do {
      SDL_WaitEvent (&evt);

      switch (evt.type) {
         case SDL_KEYDOWN:
            switch (evt.key.keysym.sym) {
               /* Update block position as soon as a key is pressed for snappy
                * response times!  Make sure to not drop it at the same time.
                */
               case SDLK_LEFT:
                  block.dx = -1;
                  block.dy = 0;
                  update_block (screen, &block);
                  break;

               case SDLK_RIGHT:
                  block.dx = 1;
                  block.dy = 0;
                  update_block (screen, &block);
                  break;

               default:
                  fprintf (stderr, "exiting on keypress\n");
                  exit(0);
            }
            break;

         case SDL_USEREVENT:
            if (evt.user.code == USER_EVT_DROP)
            {
               block.dx = 0;
               block.dy = 1;
               update_block (screen, &block);
            }

            break;

         default:
            break;
      }
   } while (evt.type != SDL_QUIT);

   return 0;
}
