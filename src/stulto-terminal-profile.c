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

#include "stulto-terminal-profile.h"

#define STULTO_DEFAULT_PROFILE "stulto.ini"

static void parse_options(GKeyFile *file, const gchar *filename, StultoTerminalProfile *profile)
{
    GError *error = NULL;

    profile->font = g_key_file_get_string(file, "options", "font", &error);
    profile->lines = g_key_file_get_integer(file, "options", "lines", &error);
    profile->bold_is_bright = g_key_file_get_boolean(file, "options", "bold-is-bright", &error);
    profile->scroll_on_output = g_key_file_get_boolean(file, "options", "scroll-on-output", &error);
    profile->scroll_on_keystroke = g_key_file_get_boolean(file, "options", "scroll-on-keystroke", &error);
    profile->mouse_autohide = g_key_file_get_boolean(file, "options", "mouse-autohide", &error);
    profile->sync_clipboard = g_key_file_get_boolean(file, "options", "sync-clipboard", &error);
    profile->urgent_on_bell = g_key_file_get_boolean(file, "options", "urgent-on-bell", &error);

    if (error)
    {
        switch (error->code)
        {
            case G_KEY_FILE_ERROR_GROUP_NOT_FOUND:
            case G_KEY_FILE_ERROR_KEY_NOT_FOUND:
                break;
            default:
                g_printerr(
                        "Error parsing '%s': %s\n", filename, error->message
                );
        }
        g_error_free(error);
        error = NULL;
    }
}

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

static void parse_colors(GKeyFile *file, const gchar *filename, StultoTerminalProfile *profile) {
    gchar name[8];
    unsigned int i;

    if (!parse_color(file, filename, "background", TRUE, &profile->background)) {
        return;
    }
    if (!parse_color(file, filename, "foreground", TRUE, &profile->foreground)) {
        return;
    }
    profile->palette_size = 2;

    parse_color(file, filename, "highlight", FALSE, &profile->highlight);
    parse_color(file, filename, "highlight-foreground", FALSE, &profile->highlight_fg);

    for (i = 0; i < 16; i++) {
        g_snprintf(name, 8, "color%u", i);
        if (!parse_color(file, filename, name, FALSE, &profile->palette[i])) {
            break;
        }
        profile->palette_size++;
    }
}

static void parse_urlmatch(GKeyFile *file, const gchar *filename, StultoTerminalProfile *profile) {
    GError *error = NULL;
    gchar *regex;

    profile->program = g_key_file_get_string(file, "urlmatch", "program", &error);
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
        g_free(profile->program);
        profile->program = NULL;

        return;
    }

#ifdef VTE_TYPE_REGEX
    profile->regex = vte_regex_new_for_match(regex, -1, PCRE2_MULTILINE, &error);
#else
    profile->regex = g_regex_new(regex, G_REGEX_MULTILINE, 0, &error);
#endif
    if (error) {
        g_printerr(
                "Error compiling regex '%s': %s\n",
                regex, error->message);
        g_error_free(error);
        g_free(profile->program);
        profile->program = NULL;
    }
    g_free(regex);
}

static void parse_file(StultoTerminalProfile *profile, GKeyFile *file, gchar *filename) {
    if (g_key_file_has_group(file, "options")) {
        parse_options(file, filename, profile);
    }
    if (g_key_file_has_group(file, "colors")) {
        parse_colors(file, filename, profile);
    }
    if (g_key_file_has_group(file, "urlmatch")) {
        parse_urlmatch(file, filename, profile);
    }
}

StultoTerminalProfile *stulto_terminal_profile_parse(gchar *filename) {
    StultoTerminalProfile *profile = g_new0(StultoTerminalProfile, 1);

    if (filename == NULL || filename[0] == '\0')
    {
        filename = profile->config_file
                 ? profile->config_file
                 : g_build_filename(g_get_user_config_dir(), "stulto", STULTO_DEFAULT_PROFILE, NULL);
    }

    GError *error = NULL;
    GKeyFile *file = g_key_file_new();
    g_key_file_load_from_file(file, filename, G_KEY_FILE_NONE, &error);

    if (error) {
        g_printerr("Unable to load config file '%s': %s\n", filename, error->message);
        g_error_free(error);
    }

    parse_file(profile, file, filename);

    profile->config_file = filename;
    g_key_file_free(file);

    return profile;
}