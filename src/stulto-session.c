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

#include "stulto-session.h"

struct _StultoSession {
    GtkBin parent_instance;

    StultoTerminal *active_terminal;
};

G_DEFINE_FINAL_TYPE(StultoSession, stulto_session, GTK_TYPE_BIN)

enum {
    PROP_0,
    PROP_ACTIVE_TERMINAL,
    N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES];

// region Declarations

/* Vfunc implementations */
static void stulto_session_dispose(GObject *object);
static void stulto_session_finalize(GObject *object);

static void stulto_session_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void stulto_session_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

StultoSession *stulto_session_new(StultoTerminal *terminal);

/* Getters & setters */
StultoTerminal *stulto_session_get_active_terminal(StultoSession *session);
void stulto_session_set_active_terminal(StultoSession *session, StultoTerminal *terminal);

// endregion

// region GObject/GtkWidget lifecycle

static void stulto_session_dispose(GObject *object) {
    G_OBJECT_CLASS(stulto_session_parent_class)->dispose(object);
}

static void stulto_session_finalize(GObject *object) {
    G_OBJECT_CLASS(stulto_session_parent_class)->finalize(object);
}

static void stulto_session_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {
    StultoSession *session = STULTO_SESSION(object);

    switch (prop_id)
    {
        case PROP_ACTIVE_TERMINAL:
            g_value_set_object(value, STULTO_TERMINAL(stulto_session_get_active_terminal(session)));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void stulto_session_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) {
    StultoSession *session = STULTO_SESSION(object);

    switch (prop_id)
    {
        case PROP_ACTIVE_TERMINAL:
            stulto_session_set_active_terminal(session, g_value_get_object(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
            break;
    }
}

static void stulto_session_class_init(StultoSessionClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = stulto_session_dispose;
    object_class->finalize = stulto_session_finalize;
    object_class->get_property = stulto_session_get_property;
    object_class->set_property = stulto_session_set_property;

    obj_properties[PROP_ACTIVE_TERMINAL] = g_param_spec_object(
            "active-terminal",
            "active-terminal",
            "The session's active terminal widget",
            STULTO_TYPE_TERMINAL,
            G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties(object_class, N_PROPERTIES, obj_properties);
}

static void stulto_session_init(StultoSession *session) {
    // Nothing to do here
}

StultoSession *stulto_session_new(StultoTerminal *terminal)
{
    StultoSession *session = STULTO_SESSION(g_object_new(
            STULTO_TYPE_SESSION,
            "active-terminal", terminal,
            NULL
    ));

    session->active_terminal = terminal;

    return session;
}

// endregion

// region Properties

StultoTerminal *stulto_session_get_active_terminal(StultoSession *session) {
    return session->active_terminal;
}

void stulto_session_set_active_terminal(StultoSession *session, StultoTerminal *terminal) {
    session->active_terminal = terminal;

    gtk_container_add(GTK_CONTAINER(session), GTK_WIDGET(terminal));
}

// endregion
