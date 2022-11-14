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

#include "stulto-main-window.h"

#include "stulto-terminal.h"
#include "stulto-session-manager.h"
#include "exit-status.h"
#include "stulto-app-config.h"

struct _StultoMainWindow {
    GtkWindow parent_instance;

    StultoAppConfig *config;
    StultoSessionManager *session_manager;
    StultoTerminal *active_terminal;
};

G_DEFINE_FINAL_TYPE(StultoMainWindow, stulto_main_window, GTK_TYPE_WINDOW)

static void stulto_main_window_dispose(GObject *object);
static void stulto_main_window_finalize(GObject *object);

static void stulto_main_window_realize(GtkWidget *widget);

static void stulto_main_window_init(StultoMainWindow *main_window);
static void stulto_main_window_class_init(StultoMainWindowClass *klass);

// region Signal Callbacks

static void screen_changed_cb(GtkWidget *widget, GdkScreen *old_screen, gpointer data) {
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

static void delete_event_cb(GtkWidget *window, GdkEvent *event, gpointer data) {
    stulto_set_exit_status(EXIT_SUCCESS);

    stulto_destroy_and_quit(window);
}

static gboolean key_press_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data) {
    StultoMainWindow *main_widow = STULTO_MAIN_WINDOW(widget);
    StultoTerminal *active_terminal = main_widow->active_terminal;
    StultoSessionManager *session_manager = main_widow->session_manager;

    GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask();

    g_assert(event->type == GDK_KEY_PRESS);

    if ((event->key.state & modifiers) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
        switch (event->key.hardware_keycode) {
            case 21: /* + on US keyboards */
                stulto_terminal_increase_font_size(active_terminal);
                return TRUE;
            case 20: /* - on US keyboards */
                stulto_terminal_decrease_font_size(active_terminal);
                return TRUE;
        }
        switch (gdk_keyval_to_lower(event->key.keyval)) {
            case GDK_KEY_c:
                stulto_terminal_copy_clipboard_format(active_terminal, VTE_FORMAT_TEXT);
                return TRUE;
            case GDK_KEY_v:
                stulto_terminal_paste_clipboard(active_terminal);
                return TRUE;
            case GDK_KEY_t:
                stulto_session_manager_add_session(
                        session_manager,
                        stulto_terminal_new(main_widow->config->initial_profile, stulto_exec_data_default()));
                return TRUE;
        }
    }

    if ((event->key.state & modifiers) == (GDK_CONTROL_MASK)) {
        switch (gdk_keyval_to_lower(event->key.keyval)) {
            case GDK_KEY_Page_Up:
                stulto_session_manager_prev_session(session_manager);
                return TRUE;
            case GDK_KEY_Page_Down:
                stulto_session_manager_next_session(session_manager);
                return TRUE;
        }
    }

    return FALSE;
}

static void stulto_main_window_session_manager_notify_active_session_cb(GObject *object, GParamSpec *pspec, gpointer data) {
    GtkWindow *window = data;

    StultoSessionManager *session_manager = STULTO_SESSION_MANAGER(object);

    gint terminal_id = stulto_session_manager_get_active_session_id(session_manager);
    gint num_sessions = stulto_session_manager_get_n_sessions(session_manager);

    // GtkNotebook uses zero-based page numbering, hence we add 1 for user-friendly output
    gchar *new_title = g_strdup_printf("[%d/%d] Stulto", terminal_id + 1, num_sessions);

    gtk_window_set_title(window, new_title);

    g_free(new_title);
}

// endregion

static void stulto_main_window_dispose(GObject *object) {
    G_OBJECT_CLASS(stulto_main_window_parent_class)->dispose(object);
}

static void stulto_main_window_finalize(GObject *object) {
    G_OBJECT_CLASS(stulto_main_window_parent_class)->finalize(object);
}

static void stulto_main_window_realize(GtkWidget *widget) {
    StultoMainWindow *main_window = STULTO_MAIN_WINDOW(widget);
    StultoAppConfig *config = main_window->config;

    if (config->role) {
        gtk_window_set_role(GTK_WINDOW(main_window), config->role);
        g_free(config->role);
    }

    GTK_WIDGET_CLASS(stulto_main_window_parent_class)->realize(widget);
}

static void stulto_main_window_init(StultoMainWindow *main_window) {
    GtkWidget *window_widget = GTK_WIDGET(main_window);

    /* Fix transparency in GNOME/Mutter */
    gtk_widget_set_app_paintable(window_widget, TRUE);

    /* Use rgba colour map if possible */
    screen_changed_cb(window_widget, NULL, NULL);
    g_signal_connect(window_widget, "screen-changed", G_CALLBACK(screen_changed_cb), NULL);
    g_signal_connect(window_widget, "delete-event", G_CALLBACK(delete_event_cb), NULL);

    StultoSessionManager *session_manager = stulto_session_manager_new();

    gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(session_manager));
    main_window->session_manager = session_manager;

    g_signal_connect(window_widget, "key-press-event", G_CALLBACK(key_press_event_cb), NULL);

    g_signal_connect(main_window->session_manager,
                     "notify::active-session",
                     G_CALLBACK(stulto_main_window_session_manager_notify_active_session_cb),
                     main_window);
}

static void stulto_main_window_class_init(StultoMainWindowClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = stulto_main_window_dispose;
    object_class->finalize = stulto_main_window_finalize;

    widget_class->realize = stulto_main_window_realize;
}

StultoMainWindow *stulto_main_window_new(StultoTerminal *terminal, StultoAppConfig *config) {
    StultoMainWindow *main_window = STULTO_MAIN_WINDOW(g_object_new(STULTO_TYPE_MAIN_WINDOW, NULL));

    stulto_session_manager_add_session(main_window->session_manager, terminal);
    main_window->active_terminal = terminal;
    main_window->config = config;

    return main_window;
}
