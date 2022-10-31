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

#ifndef STULTO_TERMINAL_H
#define STULTO_TERMINAL_H

#include <gtk/gtk.h>

#include "stulto-terminal-profile.h"
#include "stulto-exec-data.h"

/*
 * This is Stulto's terminal widget - essentially a typical VteTerminal, but configured via its own config object type
 * and rendered with a titlebar which indicates the session index and the current terminal process
 */

G_BEGIN_DECLS

#define STULTO_TYPE_TERMINAL stulto_terminal_get_type()
G_DECLARE_FINAL_TYPE(StultoTerminal, stulto_terminal, STULTO, TERMINAL, GtkBin)

StultoTerminal *stulto_terminal_new(StultoTerminalProfile *profile, StultoExecData *exec_data);

const char *stulto_terminal_get_title(StultoTerminal *terminal);
void stulto_terminal_set_title(StultoTerminal *terminal, const char *title);

void stulto_terminal_increase_font_size(StultoTerminal *terminal);
void stulto_terminal_decrease_font_size(StultoTerminal *terminal);

void stulto_terminal_copy_clipboard_format(StultoTerminal *terminal, VteFormat format);
void stulto_terminal_paste_clipboard(StultoTerminal *terminal);

G_END_DECLS

#endif //STULTO_TERMINAL_H
