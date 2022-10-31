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

#ifndef STULTO_EXEC_DATA_H
#define STULTO_EXEC_DATA_H

#include <glib.h>

typedef struct _StultoExecData {
    gchar **command_argv;
} StultoExecData;

StultoExecData *stulto_exec_data_default();

StultoExecData *stulto_exec_data_create(gchar **cmd_argv);

#endif //STULTO_EXEC_DATA_H
