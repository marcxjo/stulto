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

#include "stulto-application.h"

#include "exit-status.h"
#include "stulto-terminal-profile.h"
#include "stulto-terminal.h"
#include "stulto-headerbar.h"
#include "stulto-app-config.h"
#include "stulto-exec-data.h"
#include "stulto-main-window.h"

gboolean stulto_application_create(int argc, char *argv[]) {
    StultoAppConfig *config = g_malloc0(sizeof(StultoAppConfig));
    gchar **cmd_argv = NULL;

    GOptionEntry options[] = {
            {
                    .long_name = "config",
                    .short_name = 'c',
                    .arg = G_OPTION_ARG_STRING,
                    .arg_data = &config->initial_profile_path,
                    .description = "Specify alternative config file",
                    .arg_description = "FILE",
            },
            {
                    .long_name = "role",
                    .short_name = 'r',
                    .arg = G_OPTION_ARG_STRING,
                    .arg_data = &config->role,
                    .description = "Set window role",
                    .arg_description = "ROLE",
            },
            {
                    // TODO - this will become opt-out once the headerbar design is fully implemented
                    // (Assuming there hasn't yet been user uptake AND users don't voice strong opposition)
                    .long_name = "enable-headerbar",
                    .arg = G_OPTION_ARG_NONE,
                    .arg_data = &config->enable_headerbar,
                    .description = "Enable CSD-style headerbar",
            },
            {
                    .long_name = G_OPTION_REMAINING,
                    .arg = G_OPTION_ARG_STRING_ARRAY,
                    .arg_data = cmd_argv,
            },
            {} /* terminator */
    };

    GError *error = NULL;

    if (!gtk_init_with_args(
            &argc, &argv,
            "[-- COMMAND] - A Terminal for Fools",
            options, NULL, &error)) {
        g_printerr("%s\n", error->message);
        g_error_free(error);

        return FALSE;
    }

    StultoExecData *exec_data = stulto_exec_data_create(cmd_argv);
    StultoTerminalProfile *profile = stulto_terminal_profile_parse(config->initial_profile_path);
    config->initial_profile = profile;

    gchar *filename = profile->config_file
            ? profile->config_file
            : g_build_filename(
            g_get_user_config_dir(),
            "stulto",
            "stulto.ini",
            NULL);

    if (error) {
        switch (error->code) {
            case G_FILE_ERROR_NOENT:
            case G_KEY_FILE_ERROR_NOT_FOUND:
                break;
            default:
                g_printerr("Error opening '%s': %s\n", filename, error->message);
        }
        g_error_free(error);
        g_free(filename);

        return FALSE;
    }

    StultoTerminal *terminal = stulto_terminal_new(profile, exec_data);
    StultoMainWindow *window = stulto_main_window_new(terminal, config);

    gtk_widget_show_all(GTK_WIDGET(window));

    return TRUE;
}
