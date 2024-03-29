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

#ifndef STULTO_MAIN_WINDOW_H
#define STULTO_MAIN_WINDOW_H

#include <gtk/gtk.h>

#include "stulto-terminal.h"
#include "stulto-app-config.h"

/*
 * The window containing the terminal sessions for this instance of Stulto
 * This will eventually be migrated to GtkApplicationWindow
 */

G_BEGIN_DECLS

#define STULTO_TYPE_MAIN_WINDOW stulto_main_window_get_type()
G_DECLARE_FINAL_TYPE(StultoMainWindow, stulto_main_window, STULTO, MAIN_WINDOW, GtkWindow)

StultoMainWindow *stulto_main_window_new(StultoTerminal *terminal, StultoAppConfig *config);
void *stulto_main_window_add_terminal();

G_END_DECLS

#endif //STULTO_MAIN_WINDOW_H
