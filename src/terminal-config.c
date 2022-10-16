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

#include "terminal-config.h"

static gboolean parse_color(GKeyFile *file, const gchar *filename, const gchar *key, gboolean required, GdkRGBA *out) {
    GError *error = NULL;
    gchar *color = g_key_file_get_string(file, "colors", key, &error);
    gboolean ret;

    if (error) {
        if (error->code != G_KEY_FILE_ERROR_KEY_NOT_FOUND) {
            g_printerr(
                    "Error parsing '%s': %s\n",
                    filename, error->message);
        } else if (required) {
            g_printerr(
                    "Error parsing '%s': "
                    "section [colors] must specify %s\n",
                    filename, key);
        }
        g_error_free(error);
        return FALSE;
    }
    ret = gdk_rgba_parse(out, color);
    if (!ret) {
        g_printerr(
                "Error parsing '%s': invalid color '%s'\n",
                filename, color);
    }
    g_free(color);

    return ret;
}

static void parse_colors(GKeyFile *file, const gchar *filename, StultoTerminalConfig *conf) {
    gchar name[8];
    unsigned int i;

    if (!parse_color(file, filename, "background", TRUE, &conf->background)) {
        return;
    }
    if (!parse_color(file, filename, "foreground", TRUE, &conf->foreground)) {
        return;
    }
    conf->palette_size = 2;

    parse_color(file, filename, "highlight", FALSE, &conf->highlight);
    parse_color(file, filename, "highlight-foreground", FALSE, &conf->highlight_fg);

    for (i = 0; i < 16; i++) {
        g_snprintf(name, 8, "color%u", i);
        if (!parse_color(file, filename, name, FALSE, &conf->palette[i])) {
            break;
        }
        conf->palette_size++;
    }
}

static void parse_urlmatch(GKeyFile *file, const gchar *filename, StultoTerminalConfig *conf) {
    GError *error = NULL;
    gchar *regex;

    conf->program = g_key_file_get_string(file, "urlmatch", "program", &error);
    if (error) {
        if (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {
            g_printerr(
                    "Error parsing '%s': "
                    "section [urlmatch] must specify program\n",
                    filename);
        } else {
            g_printerr(
                    "Error parsing '%s': %s\n",
                    filename, error->message);
        }
        g_error_free(error);

        return;
    }

    regex = g_key_file_get_value(file, "urlmatch", "regex", &error);
    if (error) {
        if (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {
            g_printerr(
                    "Error parsing '%s': "
                    "section [urlmatch] must specify regex\n",
                    filename);
        } else {
            g_printerr(
                    "Error parsing '%s': %s\n",
                    filename, error->message);
        }
        g_error_free(error);
        g_free(conf->program);
        conf->program = NULL;

        return;
    }

#ifdef VTE_TYPE_REGEX
    conf->regex = vte_regex_new_for_match(regex, -1, PCRE2_MULTILINE, &error);
#else
    conf->regex = g_regex_new(regex, G_REGEX_MULTILINE, 0, &error);
#endif
    if (error) {
        g_printerr(
                "Error compiling regex '%s': %s\n",
                regex, error->message);
        g_error_free(error);
        g_free(conf->program);
        conf->program = NULL;
    }
    g_free(regex);
}

static void parse_file(StultoTerminalConfig *conf, GKeyFile *file, gchar *filename) {
    if (g_key_file_has_group(file, "colors")) {
        parse_colors(file, filename, conf);
    }
    if (g_key_file_has_group(file, "urlmatch")) {
        parse_urlmatch(file, filename, conf);
    }
}

void stulto_terminal_config_parse(StultoTerminalConfig *conf, GKeyFile *file, gchar *filename) {
    parse_file(conf, file, filename);
}