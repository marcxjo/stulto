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

#ifndef STULTO_SESSION_MANAGER_H
#define STULTO_SESSION_MANAGER_H

#include <gtk/gtk.h>

#include "stulto-terminal.h"
#include "stulto-session.h"

G_BEGIN_DECLS

#define STULTO_TYPE_SESSION_MANAGER stulto_session_manager_get_type()
G_DECLARE_FINAL_TYPE(StultoSessionManager, stulto_session_manager, STULTO, SESSION_MANAGER, GtkNotebook)

StultoSessionManager *stulto_session_manager_new();

StultoSession *stulto_session_manager_get_active_session(StultoSessionManager *session_manager);
void stulto_session_manager_set_active_session(StultoSessionManager *session_manager, StultoSession *session);

gint stulto_session_manager_get_active_session_id(StultoSessionManager *session_manager);
void stulto_session_manager_set_active_session_id(StultoSessionManager *session_manager, gint session_id);

gint stulto_session_manager_get_n_sessions(StultoSessionManager *session_manager);

void stulto_session_manager_add_session(StultoSessionManager *session_manager, StultoTerminal *first_terminal);

void stulto_session_manager_prev_session(StultoSessionManager *session_manager);
void stulto_session_manager_next_session(StultoSessionManager *session_manager);

G_END_DECLS

#endif //STULTO_SESSION_MANAGER_H
