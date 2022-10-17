/*
 * This file is part of Stulto.
 * Copyright (C) 2022 Marĉjo Givens
 *
 * This is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Exit status as an app state variable is a concept inherited from the stupidterm codebase
 * We'll remove this module once we refactor Stulto to depend on GtkApplication's lifecycle management
 */

#ifndef EXIT_STATUS_H
#define EXIT_STATUS_H

#include <gtk/gtk.h>

int stulto_get_exit_status();

void stulto_set_exit_status(int status);

/*
 * This isn't exactly the right place for this function, but as it's currently used in two modules and isn't worth
 * adding another, we'll live with the loose fit until _this_ module is refactored away
 */
void stulto_destroy_and_quit(GtkWidget *window);

#endif //EXIT_STATUS_H
