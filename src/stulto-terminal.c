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

#include "stulto-terminal.h"

#include "exit-status.h"

static void window_title_changed_cb(GtkWidget *widget, gpointer data) {
    /* TODO - we were previously passing in a pointer to the window to update its title
     * What we instead want to do is pick up the notify signal for the terminal's `window-title` property
     * At that point, we should also remove this now vestigial comeback
     */
}

static void bell_cb(GtkWidget *widget, gpointer data) {
    GtkWindow *window = data;

    gtk_window_set_urgency_hint(window, TRUE);
}

static int focus_in_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data) {
    GtkWindow *window = data;

    gtk_window_set_urgency_hint(window, FALSE);

    return FALSE;
}

static void child_exited_cb(VteTerminal *widget, int status, gpointer data) {
    GtkWidget *notebook = gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_NOTEBOOK);
    GtkWidget *window = gtk_widget_get_ancestor(GTK_WIDGET(notebook), GTK_TYPE_WINDOW);

    gint numPages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));

    if (numPages > 1) {
        gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), currentPage);
        return;
    }

    stulto_set_exit_status(status);
    stulto_destroy_and_quit(window);
}

static gboolean button_press_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data) {
    gchar *program = data;

    char *match;
    int tag;

    if (event->button.button != 3) {
        return FALSE;
    }

    match = vte_terminal_match_check_event(VTE_TERMINAL(widget), event, &tag);
    if (match != NULL) {
        GError *error = NULL;
        gchar *argv[3] = {program, match, NULL};

        if (!g_spawn_async(NULL, argv, NULL,
                           G_SPAWN_SEARCH_PATH | G_SPAWN_STDOUT_TO_DEV_NULL,
                           NULL, NULL, NULL, &error)) {
            g_printerr("%s\n", error->message);
            g_error_free(error);
        }
        g_free(match);
    }

    return FALSE;
}

static void resize_window_cb(GtkWidget *widget, guint width, guint height, gpointer data) {
    GtkWidget *window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
    VteTerminal *terminal = VTE_TERMINAL(widget);

    glong row_count = vte_terminal_get_row_count(terminal);
    glong column_count = vte_terminal_get_column_count(terminal);
    glong char_width = vte_terminal_get_char_width(terminal);
    glong char_height = vte_terminal_get_char_height(terminal);
    gint owidth;
    gint oheight;
    GtkBorder padding;

    if (width < 2) {
        width = 2;
    }
    if (height < 2) {
        height = 2;
    }

    gtk_window_get_size(GTK_WINDOW(window), &owidth, &oheight);

    /* Take into account border overhead. */
    gtk_style_context_get_padding(
            gtk_widget_get_style_context(widget),
            gtk_widget_get_state_flags(widget),
            &padding);

    owidth -= char_width * column_count + padding.left + padding.right;
    oheight -= char_height * row_count + padding.top + padding.bottom;
    gtk_window_resize(GTK_WINDOW(data),
                      width + owidth, height + oheight);
}

static void increase_font_size(GtkWidget *widget) {
    gdouble scale = vte_terminal_get_font_scale(VTE_TERMINAL(widget));
    vte_terminal_set_font_scale(VTE_TERMINAL(widget), scale * 1.125);
}

static void decrease_font_size(GtkWidget *widget) {
    gdouble scale = vte_terminal_get_font_scale(VTE_TERMINAL(widget));
    vte_terminal_set_font_scale(VTE_TERMINAL(widget), scale / 1.125);
}

static gboolean key_press_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data) {
    GtkWidget *notebook = gtk_widget_get_ancestor(widget, GTK_TYPE_NOTEBOOK);

    GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask();

    g_assert(event->type == GDK_KEY_PRESS);

    if ((event->key.state & modifiers) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
        switch (event->key.hardware_keycode) {
            case 21: /* + on US keyboards */
                increase_font_size(widget);
                return TRUE;
            case 20: /* - on US keyboards */
                decrease_font_size(widget);
                return TRUE;
        }
        switch (gdk_keyval_to_lower(event->key.keyval)) {
            case GDK_KEY_c:
                vte_terminal_copy_clipboard_format(
                        (VteTerminal *) widget,
                        VTE_FORMAT_TEXT);
                return TRUE;
            case GDK_KEY_v:
                vte_terminal_paste_clipboard((VteTerminal *) widget);
                return TRUE;
            case GDK_KEY_t:
                gtk_notebook_append_page(GTK_NOTEBOOK(notebook), stulto_terminal_create(data), gtk_label_new("New Tab"));
                return TRUE;
        }
    }

    if ((event->key.state & modifiers) == (GDK_CONTROL_MASK)) {
        switch (gdk_keyval_to_lower(event->key.keyval)) {
            case GDK_KEY_Page_Up:
                gtk_notebook_prev_page(GTK_NOTEBOOK(notebook));
                return TRUE;
            case GDK_KEY_Page_Down:
                gtk_notebook_next_page(GTK_NOTEBOOK(notebook));
                return TRUE;
        }
    }

    return FALSE;
}

static gboolean selection_changed_cb(VteTerminal *terminal, gpointer data) {
    if (vte_terminal_get_has_selection(terminal)) {
        vte_terminal_copy_clipboard_format(terminal, VTE_FORMAT_TEXT);
    }

    return TRUE;
}

static void connect_terminal_signals(VteTerminal *terminal, StultoTerminalProfile *profile) {
    // TODO - we're passing a window reference into callbacks before we even have an ancestor window
    // We should either store a reference to the window, handle these signals _in_ the window object,
    // or bubble them up via g_object_notify
    GtkWidget *widget = GTK_WIDGET(terminal);
    GtkWidget *window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);

    /* Connect to the "window-title-changed" signal to set the main window's title */
    g_signal_connect(widget, "window-title-changed", G_CALLBACK(window_title_changed_cb), NULL);

    /* Connect to the "button-press" event. */
    if (profile->program)
        g_signal_connect(widget, "button-press-event", G_CALLBACK(button_press_event_cb), profile->program);

    /* Connect to application request signals. */
    g_signal_connect(widget, "resize-window", G_CALLBACK(resize_window_cb), window);

    g_signal_connect(widget, "key-press-event", G_CALLBACK(key_press_event_cb), profile);

    /* Connect to bell signal */
    if (profile->urgent_on_bell) {
        g_signal_connect(widget, "bell", G_CALLBACK(bell_cb), window);
        g_signal_connect(widget, "focus-in-event", G_CALLBACK(focus_in_event_cb), window);
    }

    /* Sync clipboard */
    if (profile->sync_clipboard)
        g_signal_connect(widget, "selection-changed", G_CALLBACK(selection_changed_cb), NULL);
}

static void configure_terminal(VteTerminal *terminal, StultoTerminalProfile *profile) {
    /* Set some defaults. */
    vte_terminal_set_scroll_on_output(terminal, profile->scroll_on_output);
    vte_terminal_set_scroll_on_keystroke(terminal, profile->scroll_on_keystroke);
    vte_terminal_set_mouse_autohide(terminal, profile->mouse_autohide);
    vte_terminal_set_cursor_blink_mode(terminal, VTE_CURSOR_BLINK_OFF);
    vte_terminal_set_cursor_shape(terminal, VTE_CURSOR_SHAPE_BLOCK);
    vte_terminal_set_bold_is_bright(terminal, profile->bold_is_bright);
    if (profile->lines) {
        vte_terminal_set_scrollback_lines(terminal, profile->lines);
    }
    if (profile->palette_size) {
        vte_terminal_set_colors(terminal, &profile->foreground, &profile->background, profile->palette, profile->palette_size - 2);
    }
    if (profile->highlight.alpha) {
        vte_terminal_set_color_highlight(terminal, &profile->highlight);
    }
    if (profile->highlight_fg.alpha) {
        vte_terminal_set_color_highlight_foreground(terminal, &profile->highlight_fg);
    }
    if (profile->font) {
        PangoFontDescription *desc = pango_font_description_from_string(profile->font);

        vte_terminal_set_font(terminal, desc);
        pango_font_description_free(desc);
    }
    if (profile->regex) {
#ifdef VTE_TYPE_REGEX
        int id = vte_terminal_match_add_regex(terminal, profile->regex, 0);
#else
        int id = vte_terminal_match_add_gregex(terminal, profile->regex, 0);
        g_regex_unref(profile->regex);
#endif
        vte_terminal_match_set_cursor_name(terminal, id, "pointer");
    }
}

static void get_shell_and_title(VteTerminal *terminal, StultoTerminalProfile *profile) {
    // TODO - eventually we want to split these out and configure the ability to customize the window title
    if (profile->command_argv == NULL || profile->command_argv[0] == NULL) {
        g_strfreev(profile->command_argv);
        profile->command_argv = g_malloc(2 * sizeof(gchar *));
        profile->command_argv[0] = vte_get_user_shell();
        profile->command_argv[1] = NULL;

        if (profile->command_argv[0] == NULL || profile->command_argv[0][0] == '\0') {
            const gchar *shell = g_getenv("SHELL");

            if (shell == NULL || shell[0] == '\0') {
                shell = "/bin/sh";
            }

            g_free(profile->command_argv[0]);
            profile->command_argv[0] = g_strdup(shell);
        }
    }
}

static void spawn_callback(VteTerminal *terminal, GPid pid, GError *error, gpointer data) {
    GtkWidget *widget = GTK_WIDGET(terminal);
    GtkWidget *window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);

    if (pid < 0) {
        g_printerr("%s\n", error->message);
        g_error_free(error);
        stulto_destroy_and_quit(window);

        return;
    }

    g_signal_connect(widget, "child-exited", G_CALLBACK(child_exited_cb), NULL);

    gtk_widget_realize(widget);
}

GtkWidget *stulto_terminal_create(StultoTerminalProfile *profile) {
    GtkWidget *terminal_widget = vte_terminal_new();

    VteTerminal *terminal = VTE_TERMINAL(terminal_widget);

    connect_terminal_signals(terminal, profile);
    configure_terminal(terminal, profile);
    get_shell_and_title(terminal, profile);

    vte_terminal_spawn_async(
            terminal,
            VTE_PTY_DEFAULT,
            NULL,
            profile->command_argv,
            NULL,
            G_SPAWN_SEARCH_PATH,
            NULL,
            NULL,
            NULL,
            -1,
            NULL,
            &spawn_callback,
            NULL);

    return terminal_widget;
}
