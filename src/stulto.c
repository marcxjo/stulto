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
#include <vte/vte.h>

#include "terminal-config.h"

#ifdef VTE_TYPE_REGEX

#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>

#endif



typedef struct _WindowState {
    GtkWindow *window;
    GtkNotebook *notebook;
    TermConfig *conf;
} WindowState;

static int exit_status = EXIT_FAILURE;

// Declare the terminal creator early so we don't have to shuffle around code just to keep the new-tab keybinding from
// breaking
// TODO - the real fix is to not host every.single.function in one file - this project is big enough to justify a more
// granular architecture
static GtkWidget *create_terminal(WindowState *window_state);

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

static void window_title_changed(GtkWidget *widget, gpointer data) {
    GtkWindow *window = data;

    gtk_window_set_title(window, vte_terminal_get_window_title(VTE_TERMINAL(widget)));
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

static void destroy_and_quit(GtkWidget *window) {
    gtk_widget_destroy(window);

    gtk_main_quit();
}

static void delete_event(GtkWidget *window, GdkEvent *event, gpointer data) {
    exit_status = EXIT_SUCCESS;

    destroy_and_quit(window);
}

static void child_exited(VteTerminal *widget, int status, gpointer data) {
    WindowState *window_state = data;

    gint numPages = gtk_notebook_get_n_pages(window_state->notebook);
    gint currentPage = gtk_notebook_get_current_page(window_state->notebook);

    if (numPages > 1) {
        gtk_notebook_remove_page(window_state->notebook, currentPage);
        return;
    }

    exit_status = status;
    destroy_and_quit(GTK_WIDGET(window_state->window));
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
    GtkWindow *window = data;

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

    gtk_window_get_size(window, &owidth, &oheight);

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

static void page_added(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    TermConfig *conf = data;

    gchar *tab_label = g_strdup_printf("%d: %s", page_num, conf->command_argv[0]);
    gtk_notebook_set_tab_label(notebook, child, gtk_label_new(tab_label));
    g_free(tab_label);

    gtk_notebook_set_current_page(notebook, (gint) page_num);
}

static void switch_page(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    gtk_widget_show(child);
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
    WindowState *window_state = data;
    GtkWidget *window = GTK_WIDGET(window_state->window);

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
                gtk_notebook_append_page(window_state->notebook, create_terminal(data), gtk_label_new("New Tab"));
                return TRUE;
        }
    }

    if ((event->key.state & modifiers) == (GDK_CONTROL_MASK)) {
        switch (gdk_keyval_to_lower(event->key.keyval)) {
            case GDK_KEY_Page_Up:
                gtk_notebook_prev_page(GTK_NOTEBOOK(window_state->notebook));
                return TRUE;
            case GDK_KEY_Page_Down:
                gtk_notebook_next_page(GTK_NOTEBOOK(window_state->notebook));
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

static void parse_colors(GKeyFile *file, const gchar *filename, TermConfig *conf) {
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

static void parse_urlmatch(GKeyFile *file, const gchar *filename, TermConfig *conf) {
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

static void parse_file(TermConfig *conf, GOptionEntry *options) {
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

static void connect_terminal_signals(VteTerminal *terminal, WindowState *window_state) {
    GtkWidget *widget = GTK_WIDGET(terminal);
    GtkWidget *window = GTK_WIDGET(window_state->window);
    TermConfig *conf = window_state->conf;

    /* Connect to the "window_title_changed" signal to set the main window's title */
    g_signal_connect(widget, "window-title-changed", G_CALLBACK(window_title_changed), window);

    /* Connect to the "button-press" event. */
    if (conf->program)
        g_signal_connect(widget, "button-press-event", G_CALLBACK(button_press_event), conf->program);

    /* Connect to application request signals. */
    g_signal_connect(widget, "resize-window", G_CALLBACK(resize_window), window);

    /* Connect to font tweakage */
    g_signal_connect(widget, "increase-font-size", G_CALLBACK(increase_font_size), window);
    g_signal_connect(widget, "decrease-font-size", G_CALLBACK(decrease_font_size), window);
    g_signal_connect(widget, "key-press-event", G_CALLBACK(key_press_event), window_state);

    /* Connect to bell signal */
    if (conf->urgent_on_bell) {
        g_signal_connect(widget, "bell", G_CALLBACK(handle_bell), window);
        g_signal_connect(widget, "focus-in-event", G_CALLBACK(handle_focus_in), window);
    }

    /* Sync clipboard */
    if (conf->sync_clipboard)
        g_signal_connect(widget, "selection-changed", G_CALLBACK(selection_changed), NULL);
}

static void configure_terminal(VteTerminal *terminal, TermConfig *conf) {
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

static void get_shell_and_title(VteTerminal *terminal, TermConfig *conf, GtkWidget *window) {
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
    } else {
        gchar *title = g_strjoinv(" ", conf->command_argv);

        gtk_window_set_title(GTK_WINDOW(window), title);
        g_free(title);
    }
}

static void spawn_callback(VteTerminal *terminal, GPid pid, GError *error, gpointer data) {
    WindowState *window_state = data;
    GtkWidget *widget = GTK_WIDGET(terminal);
    GtkWidget *window = GTK_WIDGET(window_state->window);

    if (pid < 0) {
        g_printerr("%s\n", error->message);
        g_error_free(error);
        destroy_and_quit(window);

        return;
    }

    g_signal_connect(widget, "child-exited", G_CALLBACK(child_exited), window_state);

    gtk_widget_realize(widget);
}

static GtkWidget *create_terminal(WindowState *window_state) {
    GtkWidget *window_widget = GTK_WIDGET(window_state->window);

    GtkWidget *terminal_widget = vte_terminal_new();
    VteTerminal *terminal = VTE_TERMINAL(terminal_widget);

    connect_terminal_signals(terminal, window_state);
    configure_terminal(terminal, window_state->conf);
    get_shell_and_title(terminal, window_state->conf, window_widget);

    vte_terminal_spawn_async(
            terminal,
            VTE_PTY_DEFAULT,
            NULL,
            window_state->conf->command_argv,
            NULL,
            G_SPAWN_SEARCH_PATH,
            NULL,
            NULL,
            NULL,
            -1,
            NULL,
            &spawn_callback,
            (gpointer) window_state);

    return terminal_widget;
}

static gboolean setup(int argc, char *argv[]) {
    TermConfig *conf = g_malloc(sizeof(TermConfig));
    GOptionEntry options[] = {
            {
                    .long_name = "_TermConfig",
                    .short_name = 'c',
                    .arg = G_OPTION_ARG_STRING,
                    .arg_data = &conf->config_file,
                    .description = "Specify alternative _TermConfig file",
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
    gtk_container_add(GTK_CONTAINER(window), notebook);

    WindowState *window_state = g_malloc(sizeof(WindowState));
    window_state->window = GTK_WINDOW(window);
    window_state->notebook = GTK_NOTEBOOK(notebook);
    window_state->conf = conf;

    g_signal_connect(notebook, "page-added", G_CALLBACK(page_added), conf);
    g_signal_connect(notebook, "switch-page", G_CALLBACK(switch_page), NULL);

    // For whatever odd reason, the first terminal created, doesn't capture focus automatically
    GtkWidget *term = create_terminal(window_state);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), term, NULL);

    gtk_widget_grab_focus(term);

    gtk_widget_show_all(window);

    return TRUE;
}

int main(int argc, char *argv[]) {
    if (setup(argc, argv)) {
        gtk_main();
    }

    return exit_status;
}
