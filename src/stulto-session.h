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

#ifndef STULTO_SESSION_H
#define STULTO_SESSION_H

#include <gtk/gtk.h>
#include "stulto-terminal.h"

/**
 * A container type that hosts multiple terminal sessions as tiled widgets
 * WIP - currently only hosts a single session
 */

G_BEGIN_DECLS

#define STULTO_TYPE_SESSION stulto_session_get_type()
G_DECLARE_FINAL_TYPE(StultoSession, stulto_session, STULTO, SESSION, GtkBin)

StultoSession *stulto_session_new(StultoTerminal *terminal);

StultoTerminal *stulto_session_get_active_terminal(StultoSession *session);
void stulto_session_set_active_terminal(StultoSession *session, StultoTerminal *terminal);

G_END_DECLS

#endif //STULTO_SESSION_H
