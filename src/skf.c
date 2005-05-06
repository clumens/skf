/* $Id: skf.c,v 1.1 2005/05/06 01:28:56 chris Exp $ */

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

   screen = SDL_SetVideoMode (640, 480, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);

   if (screen == NULL)
   {
      fprintf (stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
      exit(1);
   }

   do {
      SDL_WaitEvent (&evt);
   } while (evt.type != SDL_QUIT);

   return 0;
}
