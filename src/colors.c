/* $Id: colors.c,v 1.3 2005/05/08 01:59:54 chris Exp $ */

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

/* Possible color choices for a block. */
static Uint32 colors[] = { BLUE, GREEN, ORANGE, RED };

/* +=====================================================================+
 * | PUBLIC FUNCTIONS                                                    |
 * +=====================================================================+
 */
Uint32 rand_color()
{
   unsigned int n = (int) ((NCOLORS+1.0)*rand()/(RAND_MAX+1.0));

   /* Sanity check. */
   if (n >= NCOLORS) n = NCOLORS-1;
   if (n < 0) n = 0;

   return colors[n];
}
