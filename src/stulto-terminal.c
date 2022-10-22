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

static void window_title_changed(GtkWidget *widget, gpointer data) {
    gboolean *enable_headerbar = data;
    GtkWidget *window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
    GtkWidget *notebook = gtk_widget_get_ancestor(widget, GTK_TYPE_NOTEBOOK);

    gint num_tabs = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    gint cur_tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));

    gchar *new_title;

    if (*enable_headerbar)
    {
        new_title = g_strdup_printf("Stulto: %s", vte_terminal_get_window_title(VTE_TERMINAL(widget)));

        // Not currently used, but we'll eventually plug it in to update the headerbar's session indicator
        // gchar *new_session_state = g_strdup_printf("%d/%d", cur_tab + 1, num_tabs);
        // TODO - update session state indicator
        // g_free(new_session_state);
    } else {
        new_title = g_strdup_printf(
                "[%d/%d] %s",
                cur_tab + 1,
                num_tabs,
                vte_terminal_get_window_title(VTE_TERMINAL(widget)));
    }

    gtk_window_set_title(GTK_WINDOW(window), new_title);

    g_free(new_title);
}

static void handle_bell(GtkWidget *widget, gpointer data) {
    GtkWindow *window = data;

    gtk_window_set_urgency_hint(window, TRUE);
}

static int handle_focus_in(GtkWidget *widget, GdkEvent *event, gpointer data) {
    GtkWindow *window = data;

    gtk_window_set_urgency_hint(window, FALSE);

    return FALSE;
}

static void child_exited(VteTerminal *widget, int status, gpointer data) {
    GtkWidget *window = gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW);
    GtkWidget *notebook = gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_NOTEBOOK);

    gint numPages = gtk_notebook_get_n_pages(GTK_NOTEBOOK(notebook));
    gint currentPage = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));

    if (numPages > 1) {
        gtk_notebook_remove_page(GTK_NOTEBOOK(notebook), currentPage);
        return;
    }

    stulto_set_exit_status(status);
    stulto_destroy_and_quit(window);
}

static gboolean button_press_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
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

static void resize_window(GtkWidget *widget, guint width, guint height, gpointer data) {
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

static void adjust_font_size(GtkWidget *widget, GtkWindow *window, gdouble factor) {
    VteTerminal *terminal = VTE_TERMINAL(widget);
    glong rows = vte_terminal_get_row_count(terminal);
    glong columns = vte_terminal_get_column_count(terminal);
    glong char_width = vte_terminal_get_char_width(terminal);
    glong char_height = vte_terminal_get_char_height(terminal);
    gint owidth;
    gint oheight;
    gdouble scale;

    /* Take into account padding and border overhead. */
    gtk_window_get_size(window, &owidth, &oheight);
    owidth -= char_width * columns;
    oheight -= char_height * rows;

    scale = vte_terminal_get_font_scale(terminal);
    vte_terminal_set_font_scale(terminal, scale * factor);

    /* This above call will have changed the char size! */
    char_width = vte_terminal_get_char_width(terminal);
    char_height = vte_terminal_get_char_height(terminal);

    gtk_window_resize(
            window,
            columns * char_width + owidth,
            rows * char_height + oheight);
}

static void increase_font_size(GtkWidget *widget, gpointer data) {
    GtkWindow *window = data;

    adjust_font_size(widget, window, 1.125);
}

static void decrease_font_size(GtkWidget *widget, gpointer data) {
    GtkWindow *window = data;

    adjust_font_size(widget, window, 1. / 1.125);
}

static gboolean key_press_event(GtkWidget *widget, GdkEvent *event, gpointer data) {
    GtkWidget *window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);
    GtkWidget *notebook = gtk_widget_get_ancestor(widget, GTK_TYPE_NOTEBOOK);

    GdkModifierType modifiers = gtk_accelerator_get_default_mod_mask();

    g_assert(event->type == GDK_KEY_PRESS);

    if ((event->key.state & modifiers) == (GDK_CONTROL_MASK | GDK_SHIFT_MASK)) {
        switch (event->key.hardware_keycode) {
            case 21: /* + on US keyboards */
                increase_font_size(widget, window);
                return TRUE;
            case 20: /* - on US keyboards */
                decrease_font_size(widget, window);
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

static gboolean selection_changed(VteTerminal *terminal, gpointer data) {
    if (vte_terminal_get_has_selection(terminal)) {
        vte_terminal_copy_clipboard_format(terminal, VTE_FORMAT_TEXT);
    }

    return TRUE;
}

static void connect_terminal_signals(VteTerminal *terminal, StultoTerminalConfig *conf) {
    GtkWidget *widget = GTK_WIDGET(terminal);
    GtkWidget *window = gtk_widget_get_ancestor(widget, GTK_TYPE_WINDOW);

    /* Connect to the "window-title-changed" signal to set the main window's title */
    g_signal_connect(widget, "window-title-changed", G_CALLBACK(window_title_changed), &conf->enable_headerbar);

    /* Connect to the "button-press" event. */
    if (conf->program)
        g_signal_connect(widget, "button-press-event", G_CALLBACK(button_press_event), conf->program);

    /* Connect to application request signals. */
    g_signal_connect(widget, "resize-window", G_CALLBACK(resize_window), window);

    /* Connect to font tweakage */
    g_signal_connect(widget, "increase-font-size", G_CALLBACK(increase_font_size), window);
    g_signal_connect(widget, "decrease-font-size", G_CALLBACK(decrease_font_size), window);
    g_signal_connect(widget, "key-press-event", G_CALLBACK(key_press_event), conf);

    /* Connect to bell signal */
    if (conf->urgent_on_bell) {
        g_signal_connect(widget, "bell", G_CALLBACK(handle_bell), window);
        g_signal_connect(widget, "focus-in-event", G_CALLBACK(handle_focus_in), window);
    }

    /* Sync clipboard */
    if (conf->sync_clipboard)
        g_signal_connect(widget, "selection-changed", G_CALLBACK(selection_changed), NULL);
}

static void configure_terminal(VteTerminal *terminal, StultoTerminalConfig *conf) {
    /* Set some defaults. */
    vte_terminal_set_scroll_on_output(terminal, conf->scroll_on_output);
    vte_terminal_set_scroll_on_keystroke(terminal, conf->scroll_on_keystroke);
    vte_terminal_set_mouse_autohide(terminal, conf->mouse_autohide);
    vte_terminal_set_cursor_blink_mode(terminal, VTE_CURSOR_BLINK_OFF);
    vte_terminal_set_cursor_shape(terminal, VTE_CURSOR_SHAPE_BLOCK);
    vte_terminal_set_bold_is_bright(terminal, TRUE);
    if (conf->lines) {
        vte_terminal_set_scrollback_lines(terminal, conf->lines);
    }
    if (conf->palette_size) {
        vte_terminal_set_colors(terminal, &conf->foreground, &conf->background, conf->palette, conf->palette_size - 2);
    }
    if (conf->highlight.alpha) {
        vte_terminal_set_color_highlight(terminal, &conf->highlight);
    }
    if (conf->highlight_fg.alpha) {
        vte_terminal_set_color_highlight_foreground(terminal, &conf->highlight_fg);
    }
    if (conf->font) {
        PangoFontDescription *desc = pango_font_description_from_string(conf->font);

        vte_terminal_set_font(terminal, desc);
        pango_font_description_free(desc);
    }
    if (conf->regex) {
#ifdef VTE_TYPE_REGEX
        int id = vte_terminal_match_add_regex(terminal, conf->regex, 0);
#else
        int id = vte_terminal_match_add_gregex(terminal, conf->regex, 0);
        g_regex_unref(conf->regex);
#endif
        vte_terminal_match_set_cursor_name(terminal, id, "pointer");
    }
}

static void get_shell_and_title(VteTerminal *terminal, StultoTerminalConfig *conf) {
    // TODO - eventually we want to split these out and configure the ability to customize the window title
    if (conf->command_argv == NULL || conf->command_argv[0] == NULL) {
        g_strfreev(conf->command_argv);
        conf->command_argv = g_malloc(2 * sizeof(gchar *));
        conf->command_argv[0] = vte_get_user_shell();
        conf->command_argv[1] = NULL;

        if (conf->command_argv[0] == NULL || conf->command_argv[0][0] == '\0') {
            const gchar *shell = g_getenv("SHELL");

            if (shell == NULL || shell[0] == '\0') {
                shell = "/bin/sh";
            }

            g_free(conf->command_argv[0]);
            conf->command_argv[0] = g_strdup(shell);
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

    g_signal_connect(widget, "child-exited", G_CALLBACK(child_exited), NULL);

    gtk_widget_realize(widget);
}

GtkWidget *stulto_terminal_create(StultoTerminalConfig *conf) {
    GtkWidget *terminal_widget = vte_terminal_new();

    VteTerminal *terminal = VTE_TERMINAL(terminal_widget);

    connect_terminal_signals(terminal, conf);
    configure_terminal(terminal, conf);
    get_shell_and_title(terminal, conf);

    vte_terminal_spawn_async(
            terminal,
            VTE_PTY_DEFAULT,
            NULL,
            conf->command_argv,
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
