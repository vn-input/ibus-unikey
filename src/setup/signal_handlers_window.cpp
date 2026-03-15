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

#include <cstring>
#include <libintl.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>

#include "mactab.h"
#include "macro_export_filename.h"
#include "macro_dialog_state.h"
#include "macro_file_io.h"
#include "unikey_config.h"

#include "macro_utils.h"
#include "signal_handlers.h"

#define _(str) gettext(str)


void on_main_window_destroy(GtkWidget* btn, gpointer user_data)
{
    gtk_main_quit();
}

/**
 * @brief Handler for the "Close" button in the main dialog.
 *
 * Terminates the application by quitting the main loop.
 *
 * @param btn        The Close button widget (unused).
 * @param user_data  Unused pointer supplied by GTK builder.
 */
void on_btn_close_clicked(GtkButton* btn, gpointer user_data)
{
    gtk_main_quit();
}

/**
 * @brief Helper to update a string configuration value from a combo box.
 *
 * Reads the currently selected row's first column as a string and writes the
 * result into the configuration under @p key.
 *
 * @param cbb GTK combo box containing string options.
 * @param key Configuration key to update.
 */
static void cbb_config_update(GtkComboBox* cbb, const gchar* key)
{
    GValue val = {0};
    GtkTreeIter iter;
    GtkTreeModel* model;

    model = gtk_combo_box_get_model(cbb);
    gtk_combo_box_get_active_iter(cbb, &iter);
    gtk_tree_model_get_value(model, &iter, 0, &val);

    ibus_unikey_config_set_string(key, g_value_get_string(&val));

    g_value_unset(&val);
}

/**
 * @brief Callback when the input method combo selection changes.
 *
 * Updates the `CONFIG_INPUTMETHOD` string in configuration storage.
 *
 * @param cbb        The combo box whose active row changed.
 * @param user_data  Unused pointer provided by GTK.
 */
void on_input_method_changed(GtkComboBox* cbb, gpointer user_data)
{
    cbb_config_update(cbb, CONFIG_INPUTMETHOD);
}

/**
 * @brief Callback when the output charset combo selection changes.
 *
 * Updates the `CONFIG_OUTPUTCHARSET` string in configuration storage.
 *
 * @param cbb        The combo box whose active row changed.
 * @param user_data  Unused pointer provided by GTK.
 */
void on_output_charset_changed(GtkComboBox* cbb, gpointer user_data)
{
    cbb_config_update(cbb, CONFIG_OUTPUTCHARSET);
}

/**
 * @brief Set the active row of a combo box based on a config string.
 *
 * Reads the saved value under @p key and iterates through the model looking for
 * a matching string.  If found, makes that row active.
 *
 * @param cbb GTK combo box to modify.
 * @param key Configuration key holding the value to match.
 */
void cbb_config_set_active(GtkComboBox* cbb, const gchar* key)
{
    GValue val = {0};
    GtkTreeIter iter;
    GtkTreeModel* model;

    gchar *im;
    if (!ibus_unikey_config_get_string(key, &im))
    {
        return;
    }

    model = gtk_combo_box_get_model(cbb);
    gtk_tree_model_get_iter_first(model, &iter);
    do
    {
        gtk_tree_model_get_value(model, &iter, 0, &val);
        if (strcmp(im, g_value_get_string(&val)) == 0)
        {
            gtk_combo_box_set_active_iter(cbb, &iter);
            g_value_unset(&val);
            break;
        }
        g_value_unset(&val);
    } while (gtk_tree_model_iter_next(model, &iter));

    g_free(im);
}

/**
 * @brief Handler for the "realize" event of the input-method combo box.
 *
 * Called when the widget is instantiated; sets its active row from config.
 *
 * @param cbb        The combo box being realized.
 * @param user_data  Unused pointer provided by GTK.
 */
void on_input_method_realize(GtkComboBox* cbb, gpointer user_data)
{
    cbb_config_set_active(cbb, CONFIG_INPUTMETHOD);
}

/**
 * @brief "realize" event handler for the output-charset combo box.
 *
 * Populates the selection according to stored configuration.
 *
 * @param cbb        The combo box being realized.
 * @param user_data  Unused pointer provided by GTK.
 */
void on_output_charset_realize(GtkComboBox* cbb, gpointer user_data)
{
    cbb_config_set_active(cbb, CONFIG_OUTPUTCHARSET);
}

/**
 * @brief Generic handler for toggling boolean configuration checkboxes.
 *
 * The widget's name is expected to be "cfg_<key>"; the function strips the
 * prefix and writes the boolean state to configuration.
 *
 * @param btn Toggle button whose state changed.
 * @param user_data Unused.
 */
