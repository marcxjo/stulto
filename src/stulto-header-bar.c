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

#include "stulto-header-bar.h"

struct _StultoHeaderBar {
    GtkHeaderBar parent_instance;
};

G_DEFINE_FINAL_TYPE(StultoHeaderBar, stulto_header_bar, GTK_TYPE_HEADER_BAR)

enum{
    NEW_SESSION,
    PREV_SESSION,
    NEXT_SESSION,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

#define HEADER_BAR_STYLE_CLASS "stulto-header-bar"
#define SESSION_INDICATOR_STYLE_CLASS "session-indicator"


static void stulto_header_bar_dispose(GObject *object);
static void stulto_header_bar_finalize(GObject *object);

static void stulto_header_bar_init(StultoHeaderBar *header_bar);
static void stulto_header_bar_class_init(StultoHeaderBarClass *klass);


static void new_session_btn_clicked_cb(GtkButton *button, gpointer data) {
    StultoHeaderBar *header_bar = data;

    g_signal_emit(G_OBJECT(header_bar), signals[NEW_SESSION], 0, NULL);
}

static void prev_session_btn_clicked_cb(GtkButton *button, gpointer data) {
    StultoHeaderBar *header_bar = data;

    g_signal_emit(G_OBJECT(header_bar), signals[PREV_SESSION], 0, NULL);
}

static void next_session_btn_clicked_cb(GtkButton *button, gpointer data) {
    StultoHeaderBar *header_bar = data;

    g_signal_emit(G_OBJECT(header_bar), signals[NEXT_SESSION], 0, NULL);
}

static void stulto_header_bar_dispose(GObject *object) {
    G_OBJECT_CLASS(stulto_header_bar_parent_class)->dispose(object);
}

static void stulto_header_bar_finalize(GObject *object) {
    G_OBJECT_CLASS(stulto_header_bar_parent_class)->finalize(object);
}


static void stulto_header_bar_init(StultoHeaderBar *header_bar) {
    GtkStyleContext *header_bar_style_context = gtk_widget_get_style_context(GTK_WIDGET(header_bar));
    gtk_style_context_add_class(header_bar_style_context, HEADER_BAR_STYLE_CLASS);

    GtkWidget *nav_btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // TODO - add callback to enable adding a session
    GIcon *new_session_icon = g_themed_icon_new("add");
    GtkWidget *new_session_btn = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(new_session_btn), gtk_image_new_from_gicon(new_session_icon, GTK_ICON_SIZE_BUTTON));

    g_signal_connect(new_session_btn, "clicked", G_CALLBACK(new_session_btn_clicked_cb), header_bar);

    gtk_container_add(GTK_CONTAINER(nav_btn_box), new_session_btn);

    GtkWidget *tab_ctrl_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    GIcon *prev_session_icon = g_themed_icon_new("go-previous");
    GtkWidget *prev_session_btn = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(prev_session_btn), gtk_image_new_from_gicon(prev_session_icon, GTK_ICON_SIZE_BUTTON));

    g_signal_connect(prev_session_btn, "clicked", G_CALLBACK(prev_session_btn_clicked_cb), header_bar);

    GIcon *next_session_icon = g_themed_icon_new("go-next");
    GtkWidget *next_session_btn = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(next_session_btn), gtk_image_new_from_gicon(next_session_icon, GTK_ICON_SIZE_BUTTON));

    g_signal_connect(next_session_btn, "clicked", G_CALLBACK(next_session_btn_clicked_cb), header_bar);

    gtk_container_add(GTK_CONTAINER(tab_ctrl_box), prev_session_btn);
    gtk_container_add(GTK_CONTAINER(tab_ctrl_box), next_session_btn);

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), nav_btn_box);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), tab_ctrl_box);
}

static void stulto_header_bar_class_init(StultoHeaderBarClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->dispose = stulto_header_bar_dispose;
    object_class->finalize = stulto_header_bar_finalize;

    signals[NEW_SESSION] = g_signal_new(
            "new-session",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_LAST,
            0, NULL, NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
            );

    signals[PREV_SESSION] = g_signal_new(
            "prev-session",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_LAST,
            0, NULL, NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
    );

    signals[NEXT_SESSION] = g_signal_new(
            "next-session",
            G_TYPE_FROM_CLASS(klass),
            G_SIGNAL_RUN_LAST,
            0, NULL, NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0
    );
}


StultoHeaderBar *stulto_header_bar_new() {
    return STULTO_HEADER_BAR(g_object_new(STULTO_TYPE_HEADER_BAR, NULL));
}
