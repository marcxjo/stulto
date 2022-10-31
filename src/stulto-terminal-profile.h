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

#ifndef STULTO_TERMINAL_PROFILE_H
#define STULTO_TERMINAL_PROFILE_H

#include <vte-2.91/vte/vte.h>

#ifdef VTE_TYPE_REGEX
#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>

#endif

typedef struct _StultoTerminalProfile {
    gchar *config_file;
    gchar *font;
    gint lines;
    gboolean bold_is_bright;
    gboolean scroll_on_output;
    gboolean scroll_on_keystroke;
    gboolean mouse_autohide;
    gboolean sync_clipboard;
    gboolean urgent_on_bell;
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
} StultoTerminalProfile;

StultoTerminalProfile *stulto_terminal_profile_parse(gchar *filename);

#endif //STULTO_TERMINAL_PROFILE_H
