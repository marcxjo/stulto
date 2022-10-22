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

#ifndef TERMINAL_CONFIG_H
#define TERMINAL_CONFIG_H

#include <vte-2.91/vte/vte.h>

#ifdef VTE_TYPE_REGEX
#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>

#endif

typedef struct _StultoTerminalConfig {
    gchar *config_file;
    gchar *font;
    gint lines;
    gchar *role;
    gboolean nodecorations;
    gboolean scroll_on_output;
    gboolean scroll_on_keystroke;
    gboolean mouse_autohide;
    gboolean sync_clipboard;
    gboolean urgent_on_bell;
    gboolean enable_headerbar;
    gchar **command_argv;
#ifdef VTE_TYPE_REGEX
    VteRegex *regex;
#else
    GRegex *regex;
#endif
    gchar *program;
    GdkRGBA background;
    GdkRGBA foreground;
    GdkRGBA highlight;
    GdkRGBA highlight_fg;
    GdkRGBA palette[16];
    gsize palette_size;
} StultoTerminalConfig;

void stulto_terminal_config_parse(StultoTerminalConfig *conf, GKeyFile *file, gchar *filename);

#endif //TERMINAL_CONFIG_H
