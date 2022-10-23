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
#include "stulto-terminal-profile.h"
#include "stulto-terminal.h"
#include "stulto-headerbar.h"
#include "stulto-app-config.h"

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

    stulto_destroy_and_quit(window);
}

static void page_added(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    StultoTerminalProfile *profile = data;

    gchar *tab_label = g_strdup_printf("%d: %s", page_num, profile->command_argv[0]);
    gtk_notebook_set_tab_label(notebook, child, gtk_label_new(tab_label));
    g_free(tab_label);

    gtk_notebook_set_current_page(notebook, (gint) page_num);
}

static void switch_page(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    gtk_widget_show(child);
    /* TODO - we were previously passing in a pointer to the window to update its title
     * What we instead want to do is pick up the notify signal for the notebook's `page` property
     */
}

static void parse_command_line_options(GOptionEntry *options, GKeyFile *file, gchar *filename, GError *error) {
    GOptionEntry *entry;
    gboolean option;

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
}

gboolean stulto_application_create(int argc, char *argv[]) {
    StultoTerminalProfile *profile = g_malloc(sizeof(StultoTerminalProfile));
    StultoAppConfig *config = g_malloc0(sizeof(StultoAppConfig));

    // TODO - when we refactor to GtkApplication, most of these options will go away
    // Those that remain should live in the application type rather than tangled up with the config model
    // (i.e., if it can live in the config file, it should _only_ live there)
    GOptionEntry options[] = {
            {
                    .long_name = "config",
                    .short_name = 'c',
                    .arg = G_OPTION_ARG_STRING,
                    .arg_data = &profile->config_file,
                    .description = "Specify alternative config file",
                    .arg_description = "FILE",
            },
            {
                    .long_name = "role",
                    .short_name = 'r',
                    .arg = G_OPTION_ARG_STRING,
                    .arg_data = &config->role,
                    .description = "Set window role",
                    .arg_description = "ROLE",
            },
            {
                    // TODO - this will become opt-out once the headerbar design is fully implemented
                    // (Assuming there hasn't yet been user uptake AND users don't voice strong opposition)
                    .long_name = "enable-headerbar",
                    .arg = G_OPTION_ARG_NONE,
                    .arg_data = &config->enable_headerbar,
                    .description = "Enable CSD-style headerbar",
            },
            {
                    .long_name = G_OPTION_REMAINING,
                    .arg = G_OPTION_ARG_STRING_ARRAY,
                    .arg_data = &profile->command_argv,
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

    gchar *filename = profile->config_file
            ? profile->config_file
            : g_build_filename(
            g_get_user_config_dir(),
            "stulto",
            "stulto.ini",
            NULL);

    GKeyFile *file = g_key_file_new();
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

        return FALSE;
    }

    stulto_terminal_profile_parse(profile, file, filename);
    parse_command_line_options(options, file, filename, error);

    /* Create a window to hold the scrolling shell, and hook its
     * delete event to the quit function.. */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);

    if (config->enable_headerbar)
    {
        GtkWidget *header_bar = stulto_headerbar_create();
        gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    }

    /* Fix transparency in GNOME/Mutter */
    gtk_widget_set_app_paintable(window, TRUE);

    /* Use rgba colour map if possible */
    screen_changed(window, NULL, NULL);
    g_signal_connect(window, "screen-changed", G_CALLBACK(screen_changed), NULL);

    if (config->role) {
        gtk_window_set_role(GTK_WINDOW(window), config->role);
        g_free(config->role);
    }

    /* Create the terminal widget and add it to the window */
    notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_container_add(GTK_CONTAINER(window), notebook);

    g_signal_connect(notebook, "page-added", G_CALLBACK(page_added), profile);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(switch_page), NULL);

    // For whatever odd reason, the first terminal created, doesn't capture focus automatically
    GtkWidget *term = stulto_terminal_create(profile);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), term, NULL);

    gtk_widget_grab_focus(term);

    gtk_widget_show_all(window);

    return TRUE;
}
