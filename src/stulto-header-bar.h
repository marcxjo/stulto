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

#ifndef STULTO_HEADER_BAR_H
#define STULTO_HEADER_BAR_H

#include <gtk/gtk.h>

/*
 * A Gtk CSD widget for Stulto
 * This implementation is currently WIP
 */

G_BEGIN_DECLS

#define STULTO_TYPE_HEADER_BAR stulto_header_bar_get_type()
G_DECLARE_FINAL_TYPE(StultoHeaderBar, stulto_header_bar, STULTO, HEADER_BAR, GtkHeaderBar)

StultoHeaderBar *stulto_header_bar_new();

void stulto_header_bar_set_session_indicator_label(StultoHeaderBar *header_bar, gint current_session, gint num_sessions);

G_END_DECLS

#endif //STULTO_HEADER_BAR_H
