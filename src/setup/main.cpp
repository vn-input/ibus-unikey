//------------------------------------------------------------------------------
// UniKey Vietnamese Input Method
// Copyright (C) 2000-2005 Pham Kim Long
// Copyright (C) 2026 Hoang Duy Tran & AI Agent
// Contact:
//   unikey@gmail.com
//   hoangduytran1960@googlemail.com
//   UniKey project: http://unikey.org
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//------------------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * @file main.cpp
 * @brief Entry point and GTK builder initialization for the ibus-unikey setup
 *        utility.
 *
 * This executable provides a simple GTK-based configuration dialog for the
 * engine. Widgets are defined across the setup `.ui` files and accessed by
 * signal handlers via a global builder pointer.
 */

#include <libintl.h>
#include <locale.h>
#include <cstdarg>
#include <cstdio>
#include <gtk/gtk.h>

#include "unikey_config.h"
#include "app_context.h"
#include "gui/builder/gui_builder.h"

#define _(string) gettext(string)

GtkWidget* mwin;      /**< Main window widget */
GtkWidget* dlgMacro;  /**< Macro editor dialog widget */
GtkTreeView* tree_macro; /**< Tree view showing macros */
GtkBuilder* builder;  /**< GTK builder instance used for lookups */

static void append_debug_log(const char* format, ...)
{
    gchar* log_path = g_build_filename(g_get_home_dir(), "debug.log", NULL);
    FILE* log_file = fopen(log_path, "a");
    if (log_file == NULL) {
        g_free(log_path);
        return;
    }

    GDateTime* now = g_date_time_new_now_local();
    gchar* timestamp = g_date_time_format(now, "%Y-%m-%d %H:%M:%S");
    fprintf(log_file, "[%s] ", timestamp != NULL ? timestamp : "unknown-time");

    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);

    fputc('\n', log_file);
    fclose(log_file);

    g_free(timestamp);
    g_date_time_unref(now);
    g_free(log_path);
}

/**
 * @brief Entry point for the setup utility.
 *
 * The program initializes localization and GTK, then constructs the
 * configuration dialog defined in the setup `.ui` files.
 *
 * @param argc Number of command-line arguments; may be altered by
 *        `gtk_init` during initialization.
 * @param argv Array of argument strings passed from the caller.  GTK may
 *        remove or consume recognized options (e.g. `--display`) when
 *        `gtk_init` is invoked.
 * @return Standard exit status returned to the operating system (always
 *         0 in current implementation).
 */
int main(int argc, char** argv)
{
    append_debug_log("setup main: start argv0=%s", (argc > 0 && argv[0] != NULL) ? argv[0] : "(null)");
    append_debug_log("setup main: DISPLAY=%s WAYLAND_DISPLAY=%s XDG_CURRENT_DESKTOP=%s",
                     g_getenv("DISPLAY") != NULL ? g_getenv("DISPLAY") : "(null)",
                     g_getenv("WAYLAND_DISPLAY") != NULL ? g_getenv("WAYLAND_DISPLAY") : "(null)",
                     g_getenv("XDG_CURRENT_DESKTOP") != NULL ? g_getenv("XDG_CURRENT_DESKTOP") : "(null)");

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);

    ibus_unikey_config_init();
    append_debug_log("setup main: config initialized");

    gtk_init(&argc, &argv);
    append_debug_log("setup main: gtk_init completed");
    const gchar *override_dir = g_getenv("IBUS_UNIKEY_PKGDATADIR");
    const gchar *data_dir = override_dir ? override_dir : PKGDATADIR;

    gchar *icon_path = g_strconcat(data_dir, "/icons/ibus-unikey.svg", NULL);
    append_debug_log("setup main: icon path=%s", icon_path);
    gtk_window_set_default_icon_from_file(icon_path, NULL);
    g_free(icon_path);

    /* builder will use the same path variable */
    if (override_dir) {
        /* pass override into init_gtk_builder via a global or static? simplest is to
           set a temporary variable that init_gtk_builder will read */
    }

    append_debug_log("setup main: init_gtk_builder start");
    if (!init_gtk_builder()) {
        append_debug_log("setup main: init_gtk_builder failed");
        g_critical("Failed to initialize GTK builder - cannot proceed");
        return 1;
    }
    append_debug_log("setup main: init_gtk_builder succeeded");

    append_debug_log("setup main: showing main window");
    gtk_widget_show_all(mwin);
    append_debug_log("setup main: entering gtk_main");
    gtk_main();
    append_debug_log("setup main: exited gtk_main");

    return 0;
}

