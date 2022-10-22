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

#include "stulto-headerbar.h"

GtkWidget *stulto_headerbar_create() {
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header_bar), FALSE);
//    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar), "Stulto");

    GtkWidget *nav_btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // TODO - this should not be a button since it doesn't do anything
    // Likely we'll want to do some custom drawing here instead
    GtkWidget *session_indicator = gtk_button_new();
    // TODO - set this to current_session_num/session_count
    gtk_button_set_label(GTK_BUTTON(session_indicator), "1/1");

    // TODO - add callback to enable adding a session
    GIcon *add_tab_icon = g_themed_icon_new("add");
    GtkWidget *add_tab_btn = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(add_tab_btn), gtk_image_new_from_gicon(add_tab_icon, GTK_ICON_SIZE_BUTTON));

    gtk_container_add(GTK_CONTAINER(nav_btn_box), session_indicator);
    gtk_container_add(GTK_CONTAINER(nav_btn_box), add_tab_btn);

    GtkWidget *tab_ctrl_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    // TODO - add callbacks to the next two buttons to enable moving between sessions
    GIcon *prev_icon = g_themed_icon_new("go-previous");
    GtkWidget *prev_btn = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(prev_btn), gtk_image_new_from_gicon(prev_icon, GTK_ICON_SIZE_BUTTON));

    GIcon * next_icon = g_themed_icon_new("go-next");
    GtkWidget *next_btn = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(next_btn), gtk_image_new_from_gicon(next_icon, GTK_ICON_SIZE_BUTTON));

    gtk_container_add(GTK_CONTAINER(tab_ctrl_box), prev_btn);
    gtk_container_add(GTK_CONTAINER(tab_ctrl_box), next_btn);

    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), nav_btn_box);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), tab_ctrl_box);

    return header_bar;
}