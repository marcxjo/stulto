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

#include "stulto-session-manager.h"
#include "stulto-terminal.h"

struct _StultoSessionManager {
    GtkNotebook parent_instance;
};

G_DEFINE_FINAL_TYPE(StultoSessionManager, stulto_session_manager, GTK_TYPE_NOTEBOOK)

static void stulto_session_manager_dispose(GObject *object);
static void stulto_session_manager_finalize(GObject *object);
static void stulto_session_manager_init(StultoSessionManager *session_manager);
static void stulto_session_manager_class_init(StultoSessionManagerClass *klass);

// region Signal Callbacks

static void page_added_cb(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    StultoTerminal *terminal = STULTO_TERMINAL(child);
    const char *term_title = stulto_terminal_get_title(terminal);

    gchar *tab_label_txt = g_strdup_printf("%d: %s", page_num, term_title);
    stulto_terminal_set_title(terminal, tab_label_txt);

    GtkWidget *tab_label = gtk_label_new(tab_label_txt);

    g_free(tab_label_txt);

    gtk_notebook_set_tab_label(notebook, child, tab_label);

    gtk_notebook_set_current_page(notebook, (gint) page_num);
}

static void switch_page_cb(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    gtk_widget_show_all(GTK_WIDGET(child));
}

// endregion

static void stulto_session_manager_dispose(GObject *object) {
    G_OBJECT_CLASS(stulto_session_manager_parent_class)->dispose(object);
}

static void stulto_session_manager_finalize(GObject *object) {
    G_OBJECT_CLASS(stulto_session_manager_parent_class)->finalize(object);
}

static void stulto_session_manager_init(StultoSessionManager *session_manager) {
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(session_manager), FALSE);
    g_signal_connect(session_manager, "page-added", G_CALLBACK(page_added_cb), NULL);
    g_signal_connect(session_manager, "switch-page", G_CALLBACK(switch_page_cb), NULL);
}

static void stulto_session_manager_class_init(StultoSessionManagerClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = stulto_session_manager_dispose;
    object_class->finalize = stulto_session_manager_finalize;
}

StultoSessionManager *stulto_session_manager_new() {
    return STULTO_SESSION_MANAGER(g_object_new(STULTO_TYPE_SESSION_MANAGER, NULL));
}

void stulto_session_manager_add_terminal(StultoSessionManager *session_manager, StultoTerminal *terminal) {
    GtkNotebook *notebook = GTK_NOTEBOOK(session_manager);

    gint num_pages = gtk_notebook_get_n_pages(notebook);
    gchar *label_txt = g_strdup_printf("%d", num_pages);
    GtkWidget *new_tab_label = gtk_label_new(label_txt);
    g_free(label_txt);

    gtk_notebook_append_page(notebook, GTK_WIDGET(terminal), new_tab_label);
}

void stulto_session_manager_prev_session(StultoSessionManager *session_manager) {
    GtkNotebook *notebook = GTK_NOTEBOOK(session_manager);

    gint num_pages = gtk_notebook_get_n_pages(notebook);
    gint last_page = num_pages - 1;

    gint cur_page_num = gtk_notebook_get_current_page(notebook);

    if (cur_page_num == 0) {
        gtk_notebook_set_current_page(notebook, last_page);
        return;
    }

    gtk_notebook_prev_page(notebook);
}

void stulto_session_manager_next_session(StultoSessionManager *session_manager) {
    GtkNotebook *notebook = GTK_NOTEBOOK(session_manager);

    gint num_pages = gtk_notebook_get_n_pages(notebook);
    gint last_page = num_pages - 1;

    gint cur_page_num = gtk_notebook_get_current_page(notebook);

    if (cur_page_num == last_page) {
        gtk_notebook_set_current_page(notebook, 0);
        return;
    }

    gtk_notebook_next_page(notebook);
}
