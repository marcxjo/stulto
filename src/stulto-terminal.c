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

#define STULTO_TERMINAL_TITLEBAR_STYLE_CLASS "stulto-terminal-titlebar"

enum {
    PROP_0,
    // TODO - profile is currently not actually implemented as a property - probably want to do this w/GSettings
    PROP_PROFILE,
    PROP_TITLE,
};

struct _StultoTerminal {
    GtkBin parent_instance;

    StultoTerminalProfile *profile;
    StultoExecData *exec_data;

    GtkLabel *title;
    VteTerminal *vte;
};

G_DEFINE_FINAL_TYPE(StultoTerminal, stulto_terminal, GTK_TYPE_BIN)

static void stulto_terminal_dispose(GObject *object);
static void stulto_terminal_finalize(GObject *object);

static void stulto_terminal_realize(GtkWidget *widget);

static void stulto_terminal_init(StultoTerminal *terminal);
static void stulto_terminal_class_init(StultoTerminalClass *klass);

const char *stulto_terminal_get_title(StultoTerminal *terminal);
void stulto_terminal_set_title(StultoTerminal *terminal, const char *title);

// region Signal Callbacks

static void window_title_changed_cb(VteTerminal *vte, gpointer data) {
    // TODO - hoist this so that we can add session index and update the app window title
    GtkWidget *parent = gtk_widget_get_ancestor(GTK_WIDGET(vte), STULTO_TYPE_TERMINAL);

    StultoTerminal *terminal = STULTO_TERMINAL(parent);

    stulto_terminal_set_title(terminal, vte_terminal_get_window_title(vte));
}

static void bell_cb(GtkWidget *widget, gpointer data) {
    GtkWidget *window = gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW);

    gtk_window_set_urgency_hint(GTK_WINDOW(window), TRUE);
}

static int focus_in_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data) {
    GtkWidget *window = gtk_widget_get_ancestor(GTK_WIDGET(widget), GTK_TYPE_WINDOW);

    gtk_window_set_urgency_hint(GTK_WINDOW(window), FALSE);

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
    gtk_window_resize(GTK_WINDOW(window), width + owidth, height + oheight);
}

static gboolean selection_changed_cb(VteTerminal *terminal, gpointer data) {
    if (vte_terminal_get_has_selection(terminal)) {
        vte_terminal_copy_clipboard_format(terminal, VTE_FORMAT_TEXT);
    }

    return TRUE;
}

// endregion

static void connect_terminal_signals(VteTerminal *vte, StultoTerminalProfile *profile) {
    // TODO - we're passing a window reference into callbacks before we even have an ancestor window
    // We should either store a reference to the window, handle these signals _in_ the window object,
    // or bubble them up via g_object_notify
    GtkWidget *widget = GTK_WIDGET(vte);

    /* Connect to the "window-title-changed" signal to set the main window's title */
    g_signal_connect(vte, "window-title-changed", G_CALLBACK(window_title_changed_cb), NULL);

    /* Connect to the "button-press" event. */
    if (profile->program) {
        g_signal_connect(widget, "button-press-event", G_CALLBACK(button_press_event_cb), profile->program);
    }

    /* Connect to application request signals. */
    g_signal_connect(widget, "resize-window", G_CALLBACK(resize_window_cb), NULL);

    /* Connect to bell signal */
    if (profile->urgent_on_bell) {
        g_signal_connect(widget, "bell", G_CALLBACK(bell_cb), NULL);
        g_signal_connect(widget, "focus-in-event", G_CALLBACK(focus_in_event_cb), NULL);
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
}

static void stulto_terminal_dispose(GObject *object) {
    G_OBJECT_CLASS(stulto_terminal_parent_class)->dispose(object);
}

static void stulto_terminal_finalize(GObject *object) {
    G_OBJECT_CLASS(stulto_terminal_parent_class)->finalize(object);
}

static void stulto_terminal_realize(GtkWidget *widget) {
    StultoTerminal *terminal = STULTO_TERMINAL(widget);

    GTK_WIDGET_CLASS(stulto_terminal_parent_class)->realize(widget);

    vte_terminal_spawn_async(
            terminal->vte,
            VTE_PTY_DEFAULT,
            NULL,
            terminal->exec_data->command_argv, /* TODO - this should be configurable */
            NULL,
            G_SPAWN_SEARCH_PATH,
            NULL,
            NULL,
            NULL,
            -1,
            NULL,
            &spawn_callback,
            NULL);
}

static void stulto_terminal_init(StultoTerminal *terminal) {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    GtkWidget *titlebar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    // TODO - need to ship a default stylesheet to ensure that terminal titlebars aren't transparent
    // This makes the app look broken
    GtkStyleContext *box_style_context = gtk_widget_get_style_context(titlebar);
    gtk_style_context_add_class(box_style_context, STULTO_TERMINAL_TITLEBAR_STYLE_CLASS);

    GtkWidget *label = gtk_label_new("Stulto");
    gtk_container_add(GTK_CONTAINER(titlebar), label);

    gtk_box_pack_start(GTK_BOX(box), titlebar, FALSE, FALSE, 0);

    GtkWidget *vte = vte_terminal_new();
    gtk_box_pack_start(GTK_BOX(box), vte, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(terminal), box);

    terminal->vte = VTE_TERMINAL(vte);
    terminal->title = GTK_LABEL(label);
}

static void stulto_terminal_set_exec_data(StultoTerminal *terminal, StultoExecData *exec_data)
{
    terminal->exec_data = exec_data;
}

static void stulto_terminal_set_profile(StultoTerminal *terminal, StultoTerminalProfile *profile) {
    terminal->profile = profile;
    connect_terminal_signals(VTE_TERMINAL(terminal->vte), terminal->profile);
    configure_terminal(VTE_TERMINAL(terminal->vte), terminal->profile);
}

const char *stulto_terminal_get_title(StultoTerminal *terminal) {
    return vte_terminal_get_window_title(VTE_TERMINAL(terminal->vte));
}

void stulto_terminal_set_title(StultoTerminal *terminal, const char *title) {
    GtkLabel *title_widget = GTK_LABEL(terminal->title);

    gtk_label_set_text(title_widget, title);
}

static void stulto_terminal_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
    StultoTerminal *terminal = STULTO_TERMINAL(object);

    switch (prop_id)
    {
        case PROP_TITLE:
            g_value_set_string (value, stulto_terminal_get_title(terminal));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void stulto_terminal_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
    StultoTerminal *screen = STULTO_TERMINAL(object);

    switch (prop_id)
    {
        case PROP_PROFILE:
            stulto_terminal_set_profile(screen, (StultoTerminalProfile *) value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void stulto_terminal_class_init(StultoTerminalClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    object_class->dispose = stulto_terminal_dispose;
    object_class->finalize = stulto_terminal_finalize;
    object_class->get_property = stulto_terminal_get_property;
    object_class->set_property = stulto_terminal_set_property;

    widget_class->realize = stulto_terminal_realize;

    g_object_class_install_property(
            object_class,
            PROP_TITLE,
            g_param_spec_string(
                    "title",
                    "title",
                    "The terminal's title",
                    "Stulto",
                    G_PARAM_READABLE
            ));
}

StultoTerminal *stulto_terminal_new(StultoTerminalProfile *profile, StultoExecData *exec_data) {
    StultoTerminal *terminal = g_object_new(stulto_terminal_get_type(), NULL);

    stulto_terminal_set_profile(terminal, profile);
    stulto_terminal_set_exec_data(terminal, exec_data);
    stulto_terminal_set_title(terminal, "Stulto");

    return terminal;
}

void stulto_terminal_increase_font_size(StultoTerminal *terminal) {
    VteTerminal *vte = terminal->vte;

    gdouble scale = vte_terminal_get_font_scale(vte);
    vte_terminal_set_font_scale(vte, scale * 1.125);
}

void stulto_terminal_decrease_font_size(StultoTerminal *terminal) {
    VteTerminal *vte = terminal->vte;

    gdouble scale = vte_terminal_get_font_scale(vte);
    vte_terminal_set_font_scale(vte, scale / 1.125);
}

void stulto_terminal_copy_clipboard_format(StultoTerminal *terminal, VteFormat format) {
    vte_terminal_copy_clipboard_format(terminal->vte, format);
}

void stulto_terminal_paste_clipboard(StultoTerminal *terminal) {
    vte_terminal_paste_clipboard(terminal->vte);
}
