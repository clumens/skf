/* $Id: skf.c,v 1.43 2005/06/15 02:37:01 chris Exp $ */

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

#include "blocks.h"
#include "clock.h"
#include "colors.h"
#include "draw.h"
#include "skf.h"

/* Types for user-defined events. */
#define EVT_DROP           0     /* timer for dropping blocks */
#define EVT_LAND           1     /* a block landed */
#define EVT_REMOVED        2     /* we removed lines */
#define EVT_CLOCK          3     /* update the clock */
#define EVT_SLIDE_TIMEOUT  4     /* time out for sliding the block */
#define EVT_OPEN           5     /* open a gap in the middle of the field */

#define DISABLE_DROP_TIMER(state) \
   if ((state)->drop_timer_id != NULL) \
      SDL_RemoveTimer ((state)->drop_timer_id);

#define ENABLE_DROP_TIMER(state) \
   (state)->drop_timer_id = SDL_AddTimer((state)->drop_timer_int, \
                                         drop_timer_cb, NULL);

#define DISABLE_CLOCK_TIMER(state) \
   if ((state)->clock_timer_id != NULL) \
      SDL_RemoveTimer ((state)->clock_timer_id);

#define ENABLE_CLOCK_TIMER(state) \
   (state)->clock_timer_id = SDL_AddTimer(1000, clock_cb, NULL);

/* Why is the game ending? */
typedef enum { SKF_LOSE, SKF_QUIT } end_state_t;

/* Forward declarations are easier than making sure everything's in the right
 * order.
 */
static Uint32 clock_cb (Uint32 interval, void *params);
static void do_game_over (state_t *state);
static void do_update_block (state_t *state, block_t *block);
static void drop_block (state_t *state, block_t *block);
static void drop_field (state_t *state, int lowest_y);
static Uint32 drop_timer_cb (Uint32 interval, void *params);
static void flush_evt_queue ();
static void game_over (state_t *state, end_state_t reason);
static void handle_keypress (state_t *state, block_t *block, SDLKey sym);
static void init_field (state_t *state);
static void init_surfaces (state_t *state);
static unsigned int __inline__ line_empty (field_t *field, int line);
static unsigned int __inline__ line_full (field_t *field, int line);
static void mark_full_lines (state_t *state, unsigned int min_y);
static void open_field_gap (state_t *state, block_t *block);
static unsigned int random_block ();
static Uint32 random_timer ();
static void reap_full_lines (state_t *state, block_t *block,
                             unsigned int clear_anyway);
static void shift_field_region (state_t *state, int start_x, int end_x,
                                int direction);
static void shift_field_left (state_t *state, block_t *block);
static void shift_field_right (state_t *state, block_t *block);
static int slide_filter (const SDL_Event *evt);
static unsigned int try_position_block (state_t *state, block_t *block);
static void update_block (state_t *state, block_t *block);

/* Pointers to the various block initialization functions. */
static void ((*block_inits[5])(block_t *block)) = { init_4block, init_lblock,
   init_plusblock, init_sblock, init_ublock
};

/* +================================================================+
 * | CALLBACKS                                                      |
 * +================================================================+
 */

static Uint32 clock_cb (Uint32 interval, void *params)
{
   SDL_Event evt;

   evt.type = SDL_USEREVENT;
   evt.user.code = EVT_CLOCK;
   evt.user.data1 = NULL;
   evt.user.data2 = NULL;
   SDL_PushEvent (&evt);

   return interval;
}

/* Callback function for timer - just put an event into the queue and later,
 * it will be processed to do the drawing.  We're not supposed to call functions
 * from within the callback (stupid interrupt handler model).
 */
static Uint32 drop_timer_cb (Uint32 interval, void *params)
{
   SDL_Event evt;

   evt.type = SDL_USEREVENT;
   evt.user.code = EVT_DROP;
   evt.user.data1 = NULL;
   evt.user.data2 = NULL;
   SDL_PushEvent (&evt);

   /* Return the interval so we know to reschedule the timer. */
   return interval;
}

/* This is pretty annoyingly convoluted. */
static Uint32 slide_timer_cb (Uint32 interval, void *params)
{
   SDL_Event evt;

   evt.type = SDL_USEREVENT;
   evt.user.code = EVT_SLIDE_TIMEOUT;
   evt.user.data1 = NULL;
   evt.user.data2 = NULL;
   SDL_PushEvent (&evt);

   return 0;
}

/* Filter out events we don't want to handle when the player is allowed to
 * slide the block back and forth.
 */
static int slide_filter (const SDL_Event *evt)
{
   if (evt->type == SDL_KEYDOWN)
   {
      if (evt->key.keysym.sym == SDLK_LEFT || evt->key.keysym.sym == SDLK_RIGHT)
         return 1;
      else
         return 0;
   }
   else if (evt->type == SDL_USEREVENT)
   {
      if (evt->user.code == EVT_CLOCK || evt->user.code == EVT_SLIDE_TIMEOUT)
         return 1;
      else
         return 0;
   }
   else
      return 1;
}

/* +================================================================+
 * | VIDEO INFORMATION FUNCTIONS                                    |
 * +================================================================+
 */

/* Get the best color depth we have available. */
Uint32 best_color_depth()
{
   const SDL_VideoInfo *info = SDL_GetVideoInfo();

   if (info != NULL)
      return info->vfmt->BitsPerPixel;
   else
      return 8;
}

/* Is there a window manager? */
unsigned int have_wm()
{
   const SDL_VideoInfo *info = SDL_GetVideoInfo();

   if (info != NULL)
      return info->wm_available;
   else
      return 0;
}

/* +================================================================+
 * | BLOCK MANIPULATION FUNCTIONS                                   |
 * +================================================================+
 */

/* Do all the hard work of block movement. */
static void do_update_block (state_t *state, block_t *block)
{
   /* Make sure the requested move makes sense before doing all the junk
    * below.
    */
   if (!block->may_move_sideways (block) || block->collides (block, state))
      return;

   /* Erase the previous position of the block. */
   block->erase (block, state->back);

   if (state->slide_filter == NULL)
      block->y += block->dy;

   block->x += block->dx;

   /* Draw the block in its new position. */
   block->draw (block, state->back);
   flip_screen (state->back, state->front);
}

/* When the space bar is pressed, drop the current block all the way until it
 * lands.  This way, you don't have to sit there waiting for it to happen.
 * Keyboard events are disabled during this process because we don't want any
 * other movement to be allowed.
 */
static void drop_block (state_t *state, block_t *block)
{
   SDL_Event evt;

   while (!block->landed (block, state))
   {
      do_update_block (state, block);
      SDL_Delay (50);
   }

   evt.type = SDL_USEREVENT;
   evt.user.code = EVT_LAND;
   evt.user.data1 = NULL;
   evt.user.data2 = NULL;

   if (state->slide_filter == NULL || slide_filter(&evt))
      SDL_PushEvent (&evt);
}

static unsigned int random_block ()
{
   unsigned int block_probs[5] = { 25, 25, 15, 20, 15 };
   unsigned int n, i;

   do {
      i = rnd(10) % 5;
      n = rnd(100);
   } while (n <= block_probs[i]);

   return i;
}

/* Given a newly created block, try to position it on the field.  Return 0 if
 * positioning the block fails, or 1 if it works.
 */
static unsigned int try_position_block (state_t *state, block_t *block)
{
   unsigned int i;

   /* If the new block collides with what's already there, we
    * may have reached the end of the game.
    */
   if (block->collides (block, state))
   {
      /* Try removing all the currently full lines.  Does that
       * give us room for the new block?
       */
      reap_full_lines (state, block, 1);
      if (!block->collides (block, state))
         return 1;

      /* No, still no room.  Start from the left edge of
       * the field and try every position until we find one
       * where the block fits.  If no good position, then
       * it's game over.
       */
      for (i = 0; i < X_BLOCKS-block->width; i++)
      {
         block->x = i;
         if (!block->collides (block, state))
            return 1;
      }

      return 0;
   }
   else
      return 1;
}

/* Update the position of the currently dropping block on the playing field. */
static void update_block (state_t *state, block_t *block)
{
   /* Do all the hard work. */
   do_update_block (state, block);

   /* If the block landed, first check to see if any lines are now full
    * and then reset for dropping a new block.
    */
   if (block->landed (block, state))
   {
      SDL_Event evt;
      int y;

      /* Give newly filled lines a 20% chance of disappearing on their first
       * time into the reaper.
       */
      for (y = block->y; y < Y_BLOCKS; y++)
      {
         if (line_full (&state->field, y))
            state->fills[y] = 20;
      }

      evt.type = SDL_USEREVENT;
      evt.user.code = EVT_LAND;
      evt.user.data1 = NULL;
      evt.user.data2 = NULL;

      if (state->slide_filter == NULL || slide_filter (&evt))
         SDL_PushEvent (&evt);
   }
}

/* +================================================================+
 * | FIELD MANIPULATION FUNCTIONS                                   |
 * +================================================================+
 */

/* Lower the whole playing field at or above lowest_y by one position.  Used
 * when a line is deleted and we need to drop the contents down.
 */
static void drop_field (state_t *state, int lowest_y)
{
   int x, y;

   for (y = lowest_y; y > 0; y--)
   {
      state->fills[y] = state->fills[y-1];

      for (x = 0; x < X_BLOCKS; x++)
      {
         state->field[x][y] = state->field[x][y-1];
         copy_block (state->back, x, y-1, x, y);
      }
   }

   /* Now we need to make the top-most line clear. */
   state->fills[0] = 0;

   for (x = 0; x < X_BLOCKS; x++)
   {
      state->field[x][0] = 0;
      erase_block (state->back, x, 0);
   }

   flip_screen (state->back, state->front);
}

/* Initialize the playing field array by clearing out all positions where a
 * block could be.
 */
static void init_field (state_t *state)
{
   int x, y;

   for (y = 0; y < Y_BLOCKS; y++)
   {
      state->fills[y] = 0;

      for (x = 0; x < X_BLOCKS; x++)
         state->field[x][y] = 0;
   }
}

static void open_field_gap (state_t *state, block_t *block)
{
   unsigned int y;
   unsigned int middle = (X_BLOCKS-1)/2;

   block->erase (block, state->back);
   shift_field_region (state, 1, middle, -1);
   shift_field_region (state, middle+1, X_BLOCKS-2, 1);

   for (y = 0; y < Y_BLOCKS; y++)
   {
      erase_block (state->back, middle, y);
      erase_block (state->back, middle+1, y);
      state->field[middle][y] = 0;
      state->field[middle+1][y] = 0;
   }

   block->draw (block, state->back);
   flip_screen (state->back, state->front);
}

/* Shift the portion of the screen given by [start_x, end_x] by the given
 * number of positions in direction.  Left shifts are negative, right shifts
 * are positive.
 */
static void shift_field_region (state_t *state, int start_x, int end_x,
                                int direction)
{
   SDL_Surface *save_surface;
   SDL_Rect rect;
   int x, y;

   /* First just copy the locked field information one column at a time. */
   if (direction < 0)
   {
      for (x = start_x; x <= end_x; x++)
      {
         for (y = 0; y < Y_BLOCKS; y++)
            state->field[x+direction][y] = state->field[x][y];
      }
   }
   else
   {
      for (x = end_x; x >= start_x; x--)
      {
         for (y = 0; y < Y_BLOCKS; y++)
            state->field[x+direction][y] = state->field[x][y];
      }
   }

   save_surface = save_region (state->back, FIELD_X(B2P(start_x)), FIELD_Y(0),
                               (end_x+1-start_x)*BLOCK_SIZE, FIELD_YRES);

   /* Now copy the graphics data over by one position. */
   rect.x = FIELD_X(B2P(start_x+direction));
   rect.y = FIELD_Y(0);

   SDL_BlitSurface (save_surface, NULL, state->back, &rect);
   SDL_FreeSurface (save_surface);
}

/* Shift the playing field to the left by a random number of positions.  The
 * items on the leftmost column will be wrapped around to the right side.  This
 * is used when a line is deleted and we want to make things more interesting.
 */
static void shift_field_left (state_t *state, block_t *block)
{
   unsigned int n = rnd(3);

   while (n > 0)
   {
      SDL_Surface *col_surface;
      SDL_Rect rect;
      int tmp[Y_BLOCKS];
      unsigned int y;

      /* Erase the block from the back surface so it doesn't get copied over. */
      block->erase (block, state->back);

      /* Make a copy of the leftmost column. */
      col_surface = save_region (state->back, FIELD_XOFFSET, FIELD_YOFFSET,
                                 BLOCK_SIZE, FIELD_YRES);
      for (y = 0; y < Y_BLOCKS; y++)
         tmp[y] = state->field[0][y];

      shift_field_region (state, 1, X_BLOCKS-1, -1);

      /* Paste the saved column into the rightmost position. */
      for (y = 0; y < Y_BLOCKS; y++)
         state->field[X_BLOCKS-1][y] = tmp[y];

      rect.x = FIELD_X(FIELD_XRES-BLOCK_SIZE);
      rect.y = FIELD_YOFFSET;

      SDL_BlitSurface (col_surface, NULL, state->back, &rect);
      SDL_FreeSurface (col_surface);

      n--;

      /* Pause briefly again before refreshing to show what we've done. */
      block->draw (block, state->back);
      flip_screen (state->back, state->front);
      SDL_Delay (200);
   }
}

static void shift_field_right (state_t *state, block_t *block)
{
   unsigned int n = rnd(3);

   while (n > 0)
   {
      SDL_Surface *col_surface;
      SDL_Rect rect;
      int tmp[Y_BLOCKS];
      unsigned int y;

      /* Erase the block from the back surface so it doesn't get copied over. */
      block->erase (block, state->back);

      /* Make a copy of the rightmost column. */
      col_surface = save_region (state->back, FIELD_X(B2P(X_BLOCKS-1)),
                                 FIELD_YOFFSET, BLOCK_SIZE, FIELD_YRES);
      for (y = 0; y < Y_BLOCKS; y++)
         tmp[y] = state->field[X_BLOCKS-1][y];

      shift_field_region (state, 0, X_BLOCKS-2, 1);

      /* Paste the saved column into the leftmost position. */
      for (y = 0; y < Y_BLOCKS; y++)
         state->field[0][y] = tmp[y];

      rect.x = FIELD_XOFFSET;
      rect.y = FIELD_YOFFSET;

      SDL_BlitSurface (col_surface, NULL, state->back, &rect);
      SDL_FreeSurface (col_surface);

      n--;

      /* Pause briefly again before refreshing to show what we've done. */
      block->draw (block, state->back);
      flip_screen (state->back, state->front);
      SDL_Delay (200);
   }
}

/* +================================================================+
 * | ROW MANIPULATION FUNCTIONS                                     |
 * +================================================================+
 */

static unsigned int __inline__ line_empty (field_t *field, int line)
{
   int i;

   for (i = 0; i < X_BLOCKS; i++)
   {
      if ((*field)[i][line] == 1)
         return 0;
   }

   return 1;
}

static unsigned int __inline__ line_full (field_t *field, int line)
{
   int i;

   for (i = 0; i < X_BLOCKS; i++)
   {
      if ((*field)[i][line] == 0)
         return 0;
   }

   return 1;
}

static void mark_full_lines (state_t *state, unsigned int min_y)
{
   unsigned int y;

   /* Give newly filled lines a 20% chance of disappearing on their first time
    * into the reaper.
    */
   for (y = min_y; y < Y_BLOCKS; y++)
   {
      if (line_full (&state->field, y))
         state->fills[y] = 20;
   }
}

static void reap_full_lines (state_t *state, block_t *block,
                             unsigned int clear_anyway)
{
   int x, y = Y_BLOCKS-1;
   unsigned int removed_lines = 0;
   unsigned int do_open = 0;

   while (y >= 0)
   {
      /* If this line is not full, check the next one up. */
      if (state->fills[y] == 0)
      {
         y--;
         continue;
      }

      /* If the line is full, check the probability that we can remove it.  We
       * can also just remove all full lines if we're in the game over situation
       * and we want to prolong things a little bit.
       */
      if (clear_anyway || rnd(100) <= state->fills[y])
      {
         removed_lines = 1;
         state->lines_cleared++;

         if (state->lines_cleared % 11 == 0)
            do_open = 1;

         for (x = 0; x < X_BLOCKS; x++)
            x_out_block (state->back, x, y);

         flip_region (state->back, state->front, FIELD_X(0), FIELD_Y(B2P(y)),
                      FIELD_XRES, BLOCK_SIZE);

         /* Drop down the lines above this one. */
         SDL_Delay (200);
         drop_field (state, y);
      }
      else
      {
         /* Increase probability for this line to disappear next time and
          * loop for the next row.
          */
         state->fills[y] += 10;
         y--;
      }
   }

   /* Now that we've removed lines, possibly shift all the blocks on the screen
    * by up to 3 positions.
    */
   if (removed_lines)
   {
      SDL_Event evt;

      /* Check if the field is completely clear.  If so, dole out some
       * punishment.  If not, shift things around.
       */
      for (y = 0; y < Y_BLOCKS; y++)
      {
         if (!line_empty (&state->field, y))
         {
            evt.type = SDL_USEREVENT;
            evt.user.code = EVT_REMOVED;
            evt.user.data1 = NULL;
            evt.user.data2 = NULL;

            if (state->slide_filter == NULL || slide_filter (&evt))
               SDL_PushEvent (&evt);

            state->drop_timer_id = NULL;
            break;
         }
      }

      /* If we've cleared out enough lines then open up a gap in the middle
       * of the playing field.  This appears to help the player, but also
       * screws up whatever plans they may have had.  This makes things more
       * fun.
       */
      if (do_open)
      {
         evt.type = SDL_USEREVENT;
         evt.user.code = EVT_OPEN;
         evt.user.data1 = NULL;
         evt.user.data2 = NULL;

         if (state->slide_filter == NULL || slide_filter (&evt))
            SDL_PushEvent (&evt);
      }
   }
}

/* +================================================================+
 * | MISCELLANEOUS FUNCTIONS                                        |
 * +================================================================+
 */

static void do_game_over (state_t *state)
{
   fprintf (stderr, "Game over.\n");
   fprintf (stderr, "Elapsed time: %02d:%02d:%02d\n", state->hr, state->min,
            state->sec);
   exit (0);
}

/* Handle the event loop.  We only leave this function if the user decides they
 * want to start a new game after losing.  All other game over situations go
 * into functions that do not return.
 */
static void event_loop (state_t *state, block_t *block)
{
   SDL_Event evt;

   while (1)
   {
      SDL_WaitEvent (&evt);

      switch (evt.type) {
         case SDL_KEYDOWN:
            handle_keypress (state, block, evt.key.keysym.sym);
            break;

         case SDL_QUIT:
            game_over (state, SKF_QUIT);

         case SDL_USEREVENT:
            switch (evt.user.code) {
               case EVT_DROP:
                  block->dx = 0;
                  block->dy = 1;
                  update_block (state, block);
                  break;

               /* When a block lands, the player has a short amount of time to
                * move the block from side to side before it is locked in
                * position.  Mask off all the events we don't want to deal with
                * during this interval.
                */
               case EVT_LAND:
                  SDL_SetEventFilter (slide_filter);
                  state->slide_filter = SDL_GetEventFilter();
                  SDL_AddTimer (300, slide_timer_cb, NULL);
                  break;

               /* Okay, the player's out of time for sliding the block around.
                * Re-enable all events and lock the block into position.
                */
               case EVT_SLIDE_TIMEOUT:
                  state->slide_filter = NULL;
                  SDL_SetEventFilter (NULL);
                  DISABLE_DROP_TIMER (state);

                  /* It's possible that sliding the block around put it into
                   * a position where it can drop some more, so check for
                   * that.
                   */
                  if (! block->landed (block, state))
                  {
                     ENABLE_DROP_TIMER (state);
                     break;
                  }

                  /* Make sure that chunk of the field is in use. */
                  block->lock (block, &state->field);
                  mark_full_lines (state, block->y);
                  reap_full_lines (state, block, 0);

                  /* Create a new block. */
                  block_inits[random_block()](block);

                  if (!try_position_block (state, block))
                  {
                     game_over (state, SKF_LOSE);

                     /* If we came back from game_over, time to start a new
                      * game.  This is such a mess.
                      */
                     return;
                  }

                  update_block (state, block);
                  state->drop_timer_int = random_timer();
                  ENABLE_DROP_TIMER (state);
                  break;

               case EVT_REMOVED:
                  DISABLE_DROP_TIMER (state);
                  if (rnd(10) % 2 == 0)
                     shift_field_left (state, block);
                  else
                     shift_field_right (state, block);
                  ENABLE_DROP_TIMER (state);
                  break;

               case EVT_CLOCK:
                  update_clock (state);
                  break;

               case EVT_OPEN:
                  DISABLE_DROP_TIMER (state);
                  open_field_gap (state, block);
                  ENABLE_DROP_TIMER (state);
                  break;
            }

            break;

         default:
            break;
      }
   }
}

static void flush_evt_queue ()
{
   SDL_Event junk;

   while (SDL_PollEvent (&junk))
      ;
}

/* Handle the ultimate game over situation. */
static void game_over (state_t *state, end_state_t reason)
{
   SDL_Surface *img_surface;
   SDL_Rect rect;
   SDL_Event evt;
   int x, y;

   if (reason == SKF_LOSE)
   {
      for (y = Y_BLOCKS-1; y >= 0; y--)
      {
         for (x = 0; x < X_BLOCKS; x++)
         {
            if (state->field[x][y] != 0)
               x_out_block (state->back, x, y);
         }

         flip_region (state->back, state->front, FIELD_X(0), FIELD_Y(B2P(y)),
                      FIELD_XRES, BLOCK_SIZE);
         SDL_Delay (100);
      }

      /* Draw an image to the screen so they know they lost. */
      img_surface = load_img ("game_over.png");

      rect.x = FIELD_X(B2P(3));
      rect.y = FIELD_Y(B2P(8));

      SDL_BlitSurface (img_surface, NULL, state->back, &rect);
      SDL_FreeSurface (img_surface);
      flip_screen (state->back, state->front);

      /* Purge any existing events so the player has to do something here. */
      flush_evt_queue();

      /* Quit or play again? */
      while (1)
      {
         SDL_WaitEvent (&evt);

         switch (evt.type) {
            case SDL_KEYDOWN:
               if (evt.key.keysym.sym == SDLK_ESCAPE ||
                   evt.key.keysym.sym == SDLK_q)
                  do_game_over (state);
               return;

            case SDL_QUIT:
               do_game_over (state);
               return;

            /* If they don't want to quit, we'll skip printing the message and
             * set up for playing again.  Breaking out here means we need to
             * jump out of several other functions.  Since non-local jumps suck,
             * we'll just throw return values all over.
             */
            default:
               break;
         }
      }
   }
   else
      do_game_over (state);
}

/* Handle the various key presses that can happen in our event loop - no
 * real reason to break this out other than that the event loop function is
 * getting a little too large.
 */
static void handle_keypress (state_t *state, block_t *block, SDLKey sym)
{
   switch (sym) {
      /* Update block position as soon as a key is pressed for snappy
       * response times!  Make sure to not drop it at the same time.
       */
      case SDLK_LEFT:
         block->dx = -1;
         block->dy = 0;
         update_block (state, block);
         break;

      case SDLK_RIGHT:
         block->dx = 1;
         block->dy = 0;
         update_block (state, block);
         break;

      case SDLK_UP:
         if (block->rotate != NULL)
            block->rotate (CW, block, state);
         break;

      case SDLK_DOWN:
         if (block->rotate != NULL)
            block->rotate (CCW, block, state);
         break;

      case SDLK_SPACE:
         block->dx = 0;
         block->dy = 1;
         DISABLE_DROP_TIMER (state);
         DISABLE_CLOCK_TIMER (state);
         drop_block (state, block);
         ENABLE_DROP_TIMER (state);
         ENABLE_CLOCK_TIMER (state);
         break;

      case SDLK_ESCAPE:
      case SDLK_q:
         DISABLE_DROP_TIMER (state);
         DISABLE_CLOCK_TIMER (state);
         game_over (state, SKF_QUIT);
         break;

      default:
         break;
   }
}

static void init_surfaces (state_t *state)
{
   /* The front surface is the video buffer - anything drawn onto this surface
    * will be displayed to the screen as soon as an update is called.  We only
    * need a software surface since this is a simple 2D game.
    */
   state->front = SDL_SetVideoMode (XRES, YRES, best_color_depth(),
                                    SDL_SWSURFACE);

   if (state->front == NULL)
   {
      fprintf (stderr, "Unable to create front surface: %s\n", SDL_GetError());
      exit(1);
   }

   /* The back surface is where we make all the changes.  Once a set of
    * changes have been drawn onto this surface, a flip call will sync the
    * front buffer with this one and our changes will be put to the screen.
    * This eliminates much of the tearing problem.
    */
   state->back = SDL_CreateRGBSurface (SDL_SWSURFACE, XRES, YRES,
                                       state->front->format->BitsPerPixel,
                                       state->front->format->Rmask,
                                       state->front->format->Gmask,
                                       state->front->format->Bmask,
                                       state->front->format->Amask);

   if (state->back == NULL)
   {
      fprintf (stderr, "Unable to create back surface: %s\n", SDL_GetError());
      exit(1);
   }
}

/* Return a random interval for the drop timer such that 250ms < n <= 750ms. */
static Uint32 random_timer ()
{
   Uint32 n;

   do {
      n = rnd (75);
   } while (n < 25);

   return n*10;
}

unsigned int rnd (float max)
{
   return (unsigned int) (max*rand()/RAND_MAX);
}

int main (int argc, char **argv)
{
   state_t *state;
   block_t *block;
   int y;

   /* Seed RNG for picking random colors, among other things. */
   srand (time(NULL));

   if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
   {
      fprintf (stderr, "Unable to init SDL:  %s\n", SDL_GetError());
      exit(1);
   }

   atexit (SDL_Quit);

   if (have_wm())
      SDL_WM_SetCaption("shit keeps falling - v.20050614", "skf");

   if ((state = malloc (sizeof(state_t))) == NULL)
   {
      fprintf (stderr, "Unable to malloc for state struct\n");
      exit(1);
   }

   if ((block = malloc (sizeof(block_t))) == NULL)
   {
      fprintf (stderr, "Unable to malloc for block struct\n");
      exit(1);
   }

   init_surfaces (state);

   while (1)
   {
      state->slide_filter = NULL;
      state->lines_cleared = 0;

      block_inits[random_block()](block);
      init_field (state);
      init_screen (state->back);
      init_clock (state);
      block->draw (block, state->back);
      flip_screen (state->back, state->front);

      /* Set up a callback to update the playing field every so often. */
      state->drop_timer_int = random_timer();
      ENABLE_DROP_TIMER (state);
      ENABLE_CLOCK_TIMER (state);

      if (SDL_EnableKeyRepeat (75, 75) == -1)
      {
         fprintf (stderr, "Couldn't enable keyboard repeat.\n");
         exit(1);
      }

      event_loop (state, block);

      /* Time for a new game. */
      for (y = Y_BLOCKS-1; y >= 0; y--)
      {
         drop_field (state, Y_BLOCKS-1);
         SDL_Delay (25);
      }

      DISABLE_DROP_TIMER (state);
      DISABLE_CLOCK_TIMER (state);

      /* We need to clear out the event queue. */
      flush_evt_queue();
   }
}
