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

#include <vte/vte.h>
#include "stulto-exec-data.h"

const gchar *default_shell = "/bin/bash";

StultoExecData *stulto_exec_data_default() {
    gchar **cmd_argv = g_malloc0(sizeof(gchar[2][15]));;

    const gchar *shell = g_getenv("SHELL");

    if (shell == NULL || shell[0] == '\0') {
        cmd_argv[0] = g_strdup(default_shell);
    }
    else
    {
        cmd_argv[0] = g_strdup(shell);
    }

    StultoExecData *exec_data = g_malloc0(sizeof(StultoExecData));
    exec_data->command_argv = cmd_argv;

    return exec_data;
}

StultoExecData *stulto_exec_data_create(gchar **cmd_argv) {
    if (cmd_argv == NULL || cmd_argv[0] == NULL) {
        g_strfreev(cmd_argv);
        cmd_argv = g_malloc(2 * sizeof(gchar *));
        cmd_argv[0] = vte_get_user_shell();
        cmd_argv[1] = NULL;

        if (cmd_argv[0] == NULL || cmd_argv[0][0] == '\0') {
            const gchar *shell = g_getenv("SHELL");

            if (shell == NULL || shell[0] == '\0') {
                shell = "/bin/sh";
            }

            cmd_argv[0] = g_strdup(shell);
        }
    }

    StultoExecData *exec_data = g_malloc0(sizeof(StultoExecData));
    exec_data->command_argv = cmd_argv;

    return exec_data;
}