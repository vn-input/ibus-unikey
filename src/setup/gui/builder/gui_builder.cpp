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

/**
 * @file gui_builder.cpp
 * @brief Central GTK builder bootstrap for setup windows and dialogs.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gui/builder/gui_builder.h"

#include "app_context.h"

extern GtkWidget* mwin;
extern GtkWidget* dlgMacro;
extern GtkTreeView* tree_macro;
extern GtkBuilder* builder;

static gboolean load_setup_ui_file(const gchar* installed_filename)
{
    const gchar* override_dir = g_getenv("IBUS_UNIKEY_PKGDATADIR");
    const gchar* data_dir = override_dir != NULL ? override_dir : PKGDATADIR;
    GError* error = NULL;

    gchar* installed_path = g_build_filename(data_dir, "ui", installed_filename, NULL);
    gboolean is_installed_loaded = gtk_builder_add_from_file(builder, installed_path, &error);
    if (is_installed_loaded) {
        g_free(installed_path);
        return TRUE;
    }
    g_clear_error(&error);

    gchar* cwd = g_get_current_dir();
    gchar* dev_path = g_build_filename(cwd, "setup", "gui", "ui", installed_filename, NULL);
    g_free(cwd);

    gboolean is_dev_loaded = gtk_builder_add_from_file(builder, dev_path, &error);
    if (!is_dev_loaded) {
        g_warning("Failed to load UI file '%s': %s",
                  installed_filename,
                  error != NULL ? error->message : "unknown error");
        g_clear_error(&error);
    }

    g_free(dev_path);
    g_free(installed_path);
    return is_dev_loaded;
}

gboolean init_gtk_builder(void)
{
    builder = gtk_builder_new();

    const gchar* ui_filenames[] = {
        "main_window.ui",
        "macro_dialog.ui",
        "add_macro_dialog.ui",
    };
    const gsize ui_file_count = sizeof(ui_filenames) / sizeof(ui_filenames[0]);

    for (gsize index = 0; index < ui_file_count; ++index) {
        const gboolean is_loaded = load_setup_ui_file(ui_filenames[index]);
        if (!is_loaded) {
            return FALSE;
        }
    }

    gtk_builder_connect_signals(builder, NULL);

    mwin = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    dlgMacro = GTK_WIDGET(gtk_builder_get_object(builder, "macro_dialog"));
    tree_macro = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tree_macro"));

    if (mwin == NULL || dlgMacro == NULL || tree_macro == NULL) {
        g_warning("Failed to resolve required setup widgets from GTK builder");
        return FALSE;
    }

    gtk_window_set_transient_for(GTK_WINDOW(dlgMacro), GTK_WINDOW(mwin));
    gtk_tree_view_set_activate_on_single_click(tree_macro, FALSE);

    app_context_init(builder, mwin, dlgMacro, tree_macro);

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

    return TRUE;
}
