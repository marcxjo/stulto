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

#include <vte/vte.h>

#include "stulto-application.h"
#include "exit-status.h"

int main(int argc, char *argv[]) {
    if (stulto_application_init(argc, argv)) {
        gtk_main();
    }

    return stulto_get_exit_status();
}
