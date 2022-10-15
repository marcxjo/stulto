/*
 * This file is part of Stulto.
 * Copyright (C) 2001,2002 Red Hat, Inc.
 * Copyright (C) 2013-2015 Emil Renner Berthing
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

#include <stdlib.h>

#include "stulto-application.h"

#include "exit-status.h"
#include "terminal-config.h"
#include "stulto-terminal.h"

typedef struct _StultoApplication {
    GtkWindow *window;
    GtkNotebook *notebook;
    StultoTerminalConfig *conf;
} StultoApplication;

static void screen_changed(GtkWidget *widget, GdkScreen *old_screen, gpointer data) {
    GdkScreen *screen = gtk_widget_get_screen(widget);
    GdkVisual *visual = NULL;

    if (gdk_screen_is_composited(screen)) {
        visual = gdk_screen_get_rgba_visual(screen);
    }

    if (!visual) {
        visual = gdk_screen_get_system_visual(screen);
    }

    gtk_widget_set_visual(widget, visual);
}

static void delete_event(GtkWidget *window, GdkEvent *event, gpointer data) {
    stulto_set_exit_status(EXIT_SUCCESS);

    destroy_and_quit(window);
}

static void page_added(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    StultoTerminalConfig *conf = data;

    gchar *tab_label = g_strdup_printf("%d: %s", page_num, conf->command_argv[0]);
    gtk_notebook_set_tab_label(notebook, child, gtk_label_new(tab_label));
    g_free(tab_label);

    gtk_notebook_set_current_page(notebook, (gint) page_num);
}

static void switch_page(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    gtk_widget_show(child);

    gint num_tabs = gtk_notebook_get_n_pages(notebook);
    gint cur_tab = gtk_notebook_page_num(notebook, child);

    GtkWidget *window = gtk_widget_get_ancestor(GTK_WIDGET(notebook), GTK_TYPE_WINDOW);
    gchar *new_title = g_strdup_printf(
            "[%d/%d] %s",
            cur_tab + 1,
            num_tabs,
            vte_terminal_get_window_title(VTE_TERMINAL(child)));

    gtk_window_set_title(GTK_WINDOW(window), new_title);
    g_free(new_title);
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

static void parse_file(StultoTerminalConfig *conf, GOptionEntry *options) {
    GKeyFile *file = g_key_file_new();
    GError *error = NULL;
    GOptionEntry *entry;
    gboolean option;
    gchar *filename;

    if (conf->config_file) {
        filename = conf->config_file;
    } else {
        filename = g_build_filename(g_get_user_config_dir(), "stulto", "stulto.ini", NULL);
    }

    g_key_file_load_from_file(file, filename, G_KEY_FILE_NONE, &error);

    if (error) {
        switch (error->code) {
            case G_FILE_ERROR_NOENT:
            case G_KEY_FILE_ERROR_NOT_FOUND:
                break;
            default:
                g_printerr("Error opening '%s': %s\n", filename, error->message);
        }
        g_error_free(error);
        g_key_file_free(file);
        g_free(filename);

        return;
    }

    for (entry = options; entry->long_name; entry++) {
        switch (entry->arg) {
            case G_OPTION_ARG_NONE:
                option = g_key_file_get_boolean(
                        file, "options",
                        entry->long_name,
                        &error);
                if (*((gboolean *) entry->arg_data)) {
                    *((gboolean *) entry->arg_data) = !option;
                } else {
                    *((gboolean *) entry->arg_data) = option;
                }
                break;
            case G_OPTION_ARG_INT:
                if (*((gint *) entry->arg_data) == 0) {
                    *((gint *) entry->arg_data) = g_key_file_get_integer(
                            file, "options",
                            entry->long_name,
                            &error);
                }
                break;
            case G_OPTION_ARG_STRING:
                if (*((gchar **) entry->arg_data) == NULL) {
                    *((gchar **) entry->arg_data) = g_key_file_get_string(
                            file, "options",
                            entry->long_name,
                            &error);
                }
                break;
            default:
                continue;
        }
        if (error) {
            switch (error->code) {
                case G_KEY_FILE_ERROR_GROUP_NOT_FOUND:
                case G_KEY_FILE_ERROR_KEY_NOT_FOUND:
                    break;
                default:
                    g_printerr(
                            "Error parsing '%s': %s\n",
                            filename, error->message);
            }
            g_error_free(error);
            error = NULL;
        }
    }

    if (g_key_file_has_group(file, "colors")) {
        parse_colors(file, filename, conf);
    }
    if (g_key_file_has_group(file, "urlmatch")) {
        parse_urlmatch(file, filename, conf);
    }

    g_key_file_free(file);
    g_free(filename);
}

gboolean stulto_application_init(int argc, char *argv[]) {
    StultoTerminalConfig *conf = g_malloc(sizeof(StultoTerminalConfig));
    GOptionEntry options[] = {
            {
                    .long_name = "config",
                    .short_name = 'c',
                    .arg = G_OPTION_ARG_STRING,
                    .arg_data = &conf->config_file,
                    .description = "Specify alternative config file",
                    .arg_description = "FILE",
            },
            {
                    .long_name = "font",
                    .short_name = 'f',
                    .arg = G_OPTION_ARG_STRING,
                    .arg_data = &conf->font,
                    .description = "Specify a font to use",
                    .arg_description = "FONT",
            },
            {
                    .long_name = "lines",
                    .short_name = 'n',
                    .arg = G_OPTION_ARG_INT,
                    .arg_data = &conf->lines,
                    .description = "Specify the number of scrollback lines",
                    .arg_description = "LINES",
            },
            {
                    .long_name = "role",
                    .short_name = 'r',
                    .arg = G_OPTION_ARG_STRING,
                    .arg_data = &conf->role,
                    .description = "Set window role",
                    .arg_description = "ROLE",
            },
            {
                    .long_name = "no-decorations",
                    .arg = G_OPTION_ARG_NONE,
                    .arg_data = &conf->nodecorations,
                    .description = "Disable window decorations",
            },
            {
                    .long_name = "scroll-on-output",
                    .arg = G_OPTION_ARG_NONE,
                    .arg_data = &conf->scroll_on_output,
                    .description = "Toggle scroll on output",
            },
            {
                    .long_name = "scroll-on-keystroke",
                    .arg = G_OPTION_ARG_NONE,
                    .arg_data = &conf->scroll_on_keystroke,
                    .description = "Toggle scroll on keystroke",
            },
            {
                    .long_name = "mouse-autohide",
                    .arg = G_OPTION_ARG_NONE,
                    .arg_data = &conf->mouse_autohide,
                    .description = "Toggle autohiding the mouse cursor",
            },
            {
                    .long_name = "sync-clipboard",
                    .arg = G_OPTION_ARG_NONE,
                    .arg_data = &conf->sync_clipboard,
                    .description = "Update both primary and clipboard on selection",
            },
            {
                    .long_name = "urgent-on-bell",
                    .arg = G_OPTION_ARG_NONE,
                    .arg_data = &conf->urgent_on_bell,
                    .description = "Set window urgency hint on bell",
            },
            {
                    .long_name = G_OPTION_REMAINING,
                    .arg = G_OPTION_ARG_STRING_ARRAY,
                    .arg_data = &conf->command_argv,
            },
            {} /* terminator */
    };
    GtkWidget *window;
    GtkWidget *notebook;
    GError *error = NULL;

    if (!gtk_init_with_args(
            &argc, &argv,
            "[-- COMMAND] - A Terminal for Fools",
            options, NULL, &error)) {
        g_printerr("%s\n", error->message);
        g_error_free(error);

        return FALSE;
    }

    parse_file(conf, options);

    /* Create a window to hold the scrolling shell, and hook its
     * delete event to the quit function.. */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);

    /* Fix transparency in GNOME/Mutter */
    gtk_widget_set_app_paintable(window, TRUE);

    /* Use rgba colour map if possible */
    screen_changed(window, NULL, NULL);
    g_signal_connect(window, "screen-changed", G_CALLBACK(screen_changed), NULL);

    if (conf->role) {
        gtk_window_set_role(GTK_WINDOW(window), conf->role);
        g_free(conf->role);
    }

    if (conf->nodecorations) {
        gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    }

    /* Create the terminal widget and add it to the window */
    notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_container_add(GTK_CONTAINER(window), notebook);

    StultoApplication *app = g_malloc(sizeof(StultoApplication));
    app->window = GTK_WINDOW(window);
    app->notebook = GTK_NOTEBOOK(notebook);
    app->conf = conf;

    g_signal_connect(notebook, "page-added", G_CALLBACK(page_added), conf);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(switch_page), NULL);

    // For whatever odd reason, the first terminal created, doesn't capture focus automatically
    GtkWidget *term = create_terminal(conf);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), term, NULL);

    gtk_widget_grab_focus(term);

    gtk_widget_show_all(window);

    return TRUE;
}
