/* $Id: clock.c,v 1.3 2005/06/08 23:58:41 chris Exp $ */

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

#define POS0   CLOCK_XOFFSET+5      /* hr */
#define POS1   POS0+36              /* colon */
#define POS2   POS1+17              /* min */
#define POS3   POS2+36              /* min */
#define POS4   POS3+36              /* colon */
#define POS5   POS4+17              /* sec */
#define POS6   POS5+36              /* sec */

static void hline (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   draw_line (screen, x+7, y,   x+25, y,   0x00cd0000);
   draw_line (screen, x+6, y+1, x+26, y+1, 0x00cd0000);
   draw_line (screen, x+5, y+2, x+27, y+2, 0x00cd0000);
   draw_line (screen, x+4, y+3, x+28, y+3, 0x00cd0000);
   draw_line (screen, x+5, y+4, x+27, y+4, 0x00cd0000);
   draw_line (screen, x+6, y+5, x+26, y+5, 0x00cd0000);
   draw_line (screen, x+7, y+6, x+25, y+6, 0x00cd0000);
}

static void vline (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   draw_line (screen, x,   y+4, x,   y+24, 0x00cd0000);
   draw_line (screen, x+1, y+3, x+1, y+25, 0x00cd0000);
   draw_line (screen, x+2, y+2, x+2, y+26, 0x00cd0000);
   draw_line (screen, x+3, y+1, x+3, y+27, 0x00cd0000);
   draw_line (screen, x+4, y+2, x+4, y+26, 0x00cd0000);
   draw_line (screen, x+5, y+3, x+5, y+25, 0x00cd0000);
   draw_line (screen, x+6, y+4, x+6, y+24, 0x00cd0000);
}

static void draw_colon (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   SDL_Rect r1 = { x+4, y+20, 7, 7 };
   SDL_Rect r2 = { x+4, y+40, 7, 7 };

   SDL_FillRect (screen, &r1, 0x00cd0000);
   SDL_FillRect (screen, &r2, 0x00cd0000);
}

static void draw_0 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   hline (screen, x, y+5);
   vline (screen, x, y+9);
   vline (screen, x+26, y+9);
   vline (screen, x, y+34);
   vline (screen, x+27, y+35);
   hline (screen, x, y+60);
}

static void draw_1 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   vline (screen, x+26, y+9);
   vline (screen, x+27, y+35);
}

static void draw_2 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   hline (screen, x, y+5);
   vline (screen, x+27, y+9);
   hline (screen, x, y+34);
   vline (screen, x, y+34);
   hline (screen, x, y+60);
}

static void draw_3 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   hline (screen, x, y+5);
   vline (screen, x+27, y+9);
   hline (screen, x, y+34);
   vline (screen, x+27, y+35);
   hline (screen, x, y+60);
}

static void draw_4 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   vline (screen, x, y+9);
   vline (screen, x+27, y+9);
   hline (screen, x, y+34);
   vline (screen, x+27, y+35);
}

static void draw_5 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   hline (screen, x, y+5);
   vline (screen, x, y+9);
   hline (screen, x, y+34);
   vline (screen, x+27, y+35);
   hline (screen, x, y+60);
}

static void draw_6 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   hline (screen, x, y+5);
   vline (screen, x, y+9);
   hline (screen, x, y+34);
   vline (screen, x, y+34);
   vline (screen, x+27, y+35);
   hline (screen, x, y+60);
}

static void draw_7 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   hline (screen, x, y+5);
   vline (screen, x+27, y+9);
   vline (screen, x+27, y+35);
}

static void draw_8 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   hline (screen, x, y+5);
   vline (screen, x, y+9);
   vline (screen, x+27, y+9);
   hline (screen, x, y+34);
   vline (screen, x, y+34);
   vline (screen, x+27, y+35);
   hline (screen, x, y+60);
}

static void draw_9 (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   hline (screen, x, y+5);
   vline (screen, x, y+9);
   vline (screen, x+27, y+9);
   hline (screen, x, y+34);
   vline (screen, x+27, y+35);
   hline (screen, x, y+60);
}

static void erase (SDL_Surface *screen, Uint32 x, Uint32 y)
{
   SDL_Rect r = { x, y, 34, CLOCK_YRES };

   SDL_FillRect (screen, &r, GREY);
}

/* +=====================================================================+
 * | PUBLIC FUNCTIONS                                                    |
 * +=====================================================================+
 */

void init_clock (state_t *state)
{
   state->hr = state->min = state->sec = 0;

   draw_0 (state->back, POS0, CLOCK_YOFFSET);
   draw_colon (state->back, POS1, CLOCK_YOFFSET);
   draw_0 (state->back, POS2, CLOCK_YOFFSET);
   draw_0 (state->back, POS3, CLOCK_YOFFSET);
   draw_colon (state->back, POS4, CLOCK_YOFFSET);
   draw_0 (state->back, POS5, CLOCK_YOFFSET);
   draw_0 (state->back, POS6, CLOCK_YOFFSET);
}

void update_clock (state_t *state)
{
   void ((*digits[10])(SDL_Surface *screen, Uint32 x, Uint32 y)) = {
      draw_0, draw_1, draw_2, draw_3, draw_4, draw_5, draw_6, draw_7,
      draw_8, draw_9
   };

   /* First, update the time itself. */
   state->sec++;
   if (state->sec == 60)
   {
      state->sec = 0;
      state->min++;
   }

   if (state->min == 60)
   {
      state->min = 0;
      state->hr++;
   }

   /* Now, draw new digits only for the parts that have changed. */
   if (state->min == 0)
   {
      erase (state->back, POS0, CLOCK_YOFFSET);
      digits[state->hr](state->back, POS0, CLOCK_YOFFSET);
   }

   if (state->sec == 0)
   {
      erase (state->back, POS2, CLOCK_YOFFSET);
      erase (state->back, POS3, CLOCK_YOFFSET);
      digits[state->min/10](state->back, POS2, CLOCK_YOFFSET);
      digits[state->min%10](state->back, POS3, CLOCK_YOFFSET);
   }

   erase (state->back, POS5, CLOCK_YOFFSET);
   erase (state->back, POS6, CLOCK_YOFFSET);
   digits[state->sec/10](state->back, POS5, CLOCK_YOFFSET);
   digits[state->sec%10](state->back, POS6, CLOCK_YOFFSET);

   flip_screen (state->back, state->front);
}
