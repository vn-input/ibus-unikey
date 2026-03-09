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
 * engine. Widgets are defined in `ibus-unikey.ui` and accessed by signal
 * handlers via a global builder pointer.
 */

#include <libintl.h>
#include <locale.h>
#include <gtk/gtk.h>

#include "unikey_config.h"

#define _(string) gettext(string)

GtkWidget* mwin;      /**< Main window widget */
GtkWidget* dlgMacro;  /**< Macro editor dialog widget */
GtkTreeView* tree_macro; /**< Tree view showing macros */
GtkBuilder* builder;  /**< GTK builder instance used for lookups */

/**
 * @brief Create a GTK builder and fetch commonly used widgets.
 *
 * The builder is stored in the global `builder` variable so that signal
 * handlers (defined elsewhere) can query for widgets when needed. This
 * function also establishes the macro dialog as transient for the main
 * window and initializes the `tree_macro` pointer.
 */
void init_gtk_builder()
{
    builder = gtk_builder_new();
    const gchar *override_dir_2 = g_getenv("IBUS_UNIKEY_PKGDATADIR");
    const gchar *data_dir2 = override_dir_2 ? override_dir_2 : PKGDATADIR;
    gtk_builder_add_from_file(builder, g_strconcat(data_dir2, "/ui/ibus-unikey.ui", NULL), NULL);
    gtk_builder_connect_signals(builder, NULL);

    mwin = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    dlgMacro = GTK_WIDGET(gtk_builder_get_object(builder, "macro_dialog"));
    gtk_window_set_transient_for(GTK_WINDOW(dlgMacro), GTK_WINDOW(mwin));

    tree_macro = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tree_macro"));
    gtk_tree_view_set_activate_on_single_click(tree_macro, FALSE);

    GtkCellRenderer* key_renderer = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cellrenderertext_key"));
    GtkCellRenderer* value_renderer = GTK_CELL_RENDERER(gtk_builder_get_object(builder, "cellrenderertext_value"));
    if (key_renderer != NULL) {
        g_object_set(key_renderer,
                     "editable", FALSE,
                     "mode", GTK_CELL_RENDERER_MODE_INERT,
                     NULL);
    }
    if (value_renderer != NULL) {
        g_object_set(value_renderer,
                     "editable", FALSE,
                     "mode", GTK_CELL_RENDERER_MODE_INERT,
                     NULL);
    }

    // Note: builder is kept global so signal handlers can access widgets
}

/**
 * @brief Entry point for the setup utility.
 *
 * The program initializes localization and GTK, then constructs the
 * configuration dialog defined in `ibus-unikey.ui`.
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
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);

    ibus_unikey_config_init();

    gtk_init(&argc, &argv);
    const gchar *override_dir = g_getenv("IBUS_UNIKEY_PKGDATADIR");
    const gchar *data_dir = override_dir ? override_dir : PKGDATADIR;

    gtk_window_set_default_icon_from_file(g_strconcat(data_dir, "/icons/ibus-unikey.svg", NULL), NULL);

    /* builder will use the same path variable */
    if (override_dir) {
        /* pass override into init_gtk_builder via a global or static? simplest is to
           set a temporary variable that init_gtk_builder will read */
    }

    init_gtk_builder();

    gtk_widget_show_all(mwin);
    gtk_main();

    return 0;
}

