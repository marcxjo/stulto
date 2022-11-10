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
#include "stulto-session.h"

enum {
    PROP_0,
    PROP_ACTIVE_SESSION,
    N_PROPS
};

struct _StultoSessionManager {
    GtkNotebook parent_instance;
};

G_DEFINE_FINAL_TYPE(StultoSessionManager, stulto_session_manager, GTK_TYPE_NOTEBOOK)

static GParamSpec *pspecs[N_PROPS] = {NULL };

// region Declarations

/* Vfunc implementations */
static void stulto_session_manager_dispose(GObject *object);
static void stulto_session_manager_finalize(GObject *object);

static void stulto_session_manager_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void stulto_session_manager_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void stulto_session_manager_init(StultoSessionManager *session_manager);
static void stulto_session_manager_class_init(StultoSessionManagerClass *klass);

/* Getters & setters */
StultoSession *stulto_session_manager_get_active_session(StultoSessionManager *session_manager);
void stulto_session_manager_set_active_session(StultoSessionManager *session_manager, StultoSession *session);

gint stulto_session_manager_get_active_session_id(StultoSessionManager *session_manager);
void stulto_session_manager_set_active_session_id(StultoSessionManager *session_manager, gint session_id);

gint stulto_session_manager_get_n_sessions(StultoSessionManager *session_manager);

// endregion

// region Callbacks

static void page_added_cb(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    StultoSessionManager *session_manager = STULTO_SESSION_MANAGER(notebook);
    StultoSession *session = STULTO_SESSION(child);
    StultoTerminal *terminal = stulto_session_get_active_terminal(session);
    const char *term_title = stulto_terminal_get_title(terminal);

    gchar *new_title = g_strdup(
            term_title == NULL || term_title[0] == '\0'
            ? "Stulto"
            : term_title);

    stulto_terminal_set_title(terminal, new_title);

    g_free(new_title);

    stulto_session_manager_set_active_session(session_manager, session);
}

static void switch_page_cb(GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer data) {
    gtk_widget_show_all(child);

    g_object_notify(G_OBJECT(notebook), "active-session");
}

// endregion

// region GObject/GtkWidget lifecycle

static void stulto_session_manager_dispose(GObject *object) {
    G_OBJECT_CLASS(stulto_session_manager_parent_class)->dispose(object);
}

static void stulto_session_manager_finalize(GObject *object) {
    G_OBJECT_CLASS(stulto_session_manager_parent_class)->finalize(object);
}

static void stulto_session_manager_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    StultoSessionManager *session_manager = STULTO_SESSION_MANAGER(object);

    switch (prop_id) {
        case PROP_ACTIVE_SESSION:
            g_value_set_object(value, STULTO_SESSION(
                    stulto_session_manager_get_active_session(
                            session_manager
                    )));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void stulto_session_manager_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    StultoSessionManager *session_manager = STULTO_SESSION_MANAGER(object);

    switch (prop_id) {
        case PROP_ACTIVE_SESSION:
            stulto_session_manager_set_active_session(session_manager, STULTO_SESSION(g_value_get_object(value)));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void stulto_session_manager_class_init(StultoSessionManagerClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = stulto_session_manager_dispose;
    object_class->finalize = stulto_session_manager_finalize;
    object_class->get_property = stulto_session_manager_get_property;
    object_class->set_property = stulto_session_manager_set_property;

    pspecs[PROP_ACTIVE_SESSION] = g_param_spec_object(
            "active-session",
            "active-session",
            "The currently selected terminal session",
            STULTO_TYPE_SESSION,
            G_PARAM_READWRITE);

    g_object_class_install_properties(object_class, N_PROPS, pspecs);
}

static void stulto_session_manager_init(StultoSessionManager *session_manager) {
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(session_manager), FALSE);

    g_signal_connect(session_manager, "page-added", G_CALLBACK(page_added_cb), NULL);
    g_signal_connect_after(session_manager, "switch-page", G_CALLBACK(switch_page_cb), NULL);
}

StultoSessionManager *stulto_session_manager_new() {
    return STULTO_SESSION_MANAGER(g_object_new(STULTO_TYPE_SESSION_MANAGER, NULL));
}

// endregion

// region Properties

StultoSession *stulto_session_manager_get_active_session(StultoSessionManager *session_manager) {
    g_return_val_if_fail(STULTO_IS_SESSION_MANAGER(session_manager), NULL);

    GtkNotebook *notebook = GTK_NOTEBOOK(session_manager);

    gint current_page = gtk_notebook_get_current_page(notebook);

    GtkWidget *widget = gtk_notebook_get_nth_page(notebook, current_page);

    return STULTO_SESSION(widget);
}

void stulto_session_manager_set_active_session(StultoSessionManager *session_manager, StultoSession *session) {
    g_return_if_fail(STULTO_IS_SESSION_MANAGER(session_manager));

    GtkNotebook *notebook = GTK_NOTEBOOK(session_manager);

    gint page_num = gtk_notebook_page_num(notebook, GTK_WIDGET(session));

    g_return_if_fail(page_num >= 0);

    gtk_notebook_set_current_page(notebook, page_num);
}

gint stulto_session_manager_get_active_session_id(StultoSessionManager *session_manager) {
    g_return_val_if_fail(STULTO_IS_SESSION_MANAGER(session_manager), -1);

    return gtk_notebook_get_current_page(GTK_NOTEBOOK(session_manager));
}

void stulto_session_manager_set_active_session_id(StultoSessionManager *session_manager, gint session_id) {
    g_return_if_fail(STULTO_IS_SESSION_MANAGER(session_manager));

    gtk_notebook_set_current_page(GTK_NOTEBOOK(session_manager), session_id);
}

gint stulto_session_manager_get_n_sessions(StultoSessionManager *session_manager) {
    g_return_val_if_fail(STULTO_IS_SESSION_MANAGER(session_manager), -1);

    GtkNotebook *notebook = GTK_NOTEBOOK(session_manager);

    return gtk_notebook_get_n_pages(notebook);
}

void stulto_session_manager_add_session(StultoSessionManager *session_manager, StultoTerminal *first_terminal) {
    g_return_if_fail(STULTO_IS_SESSION_MANAGER(session_manager));

    GtkNotebook *notebook = GTK_NOTEBOOK(session_manager);

    gint next_id = gtk_notebook_get_n_pages(notebook);
    gchar *label_txt = g_strdup_printf("%d", next_id);
    GtkWidget *new_tab_label = gtk_label_new(label_txt);
    g_free(label_txt);

    StultoSession *session = stulto_session_new(first_terminal);
    gtk_notebook_append_page(notebook, GTK_WIDGET(session), new_tab_label);

    stulto_session_manager_set_active_session(session_manager, session);
}

void stulto_session_manager_prev_session(StultoSessionManager *session_manager) {
    g_return_if_fail(STULTO_IS_SESSION_MANAGER(session_manager));

    GtkNotebook *notebook = GTK_NOTEBOOK(session_manager);

    gint num_pages = gtk_notebook_get_n_pages(notebook);
    gint last_page = num_pages - 1;

    gint cur_page_num = gtk_notebook_get_current_page(notebook);

    g_return_if_fail(cur_page_num >= 0);

    gint new_page_num = cur_page_num == 0 ? last_page : cur_page_num - 1;

    GtkWidget *new_page_widget = gtk_notebook_get_nth_page(notebook, new_page_num);

    StultoSession *new_active_session = STULTO_SESSION(new_page_widget);

    stulto_session_manager_set_active_session(session_manager, new_active_session);
}

void stulto_session_manager_next_session(StultoSessionManager *session_manager) {
    g_return_if_fail(STULTO_IS_SESSION_MANAGER(session_manager));

    GtkNotebook *notebook = GTK_NOTEBOOK(session_manager);

    gint num_pages = gtk_notebook_get_n_pages(notebook);
    gint last_page = num_pages - 1;

    gint cur_page_num = gtk_notebook_get_current_page(notebook);

    g_return_if_fail(cur_page_num >= 0);

    gint new_page_num = cur_page_num == last_page ? 0 : cur_page_num + 1;

    GtkWidget *new_page_widget = gtk_notebook_get_nth_page(notebook, new_page_num);

    StultoSession *new_active_session = STULTO_SESSION(new_page_widget);

    stulto_session_manager_set_active_session(session_manager, new_active_session);
}

// endregion
