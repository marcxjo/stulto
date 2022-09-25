//
// Created by marcxjo on 9/25/22.
//

#ifndef TERMINAL_CONFIG_H
#define TERMINAL_CONFIG_H

#include <vte-2.91/vte/vte.h>

typedef struct _TermConfig {
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
} TermConfig;

#endif //TERMINAL_CONFIG_H
