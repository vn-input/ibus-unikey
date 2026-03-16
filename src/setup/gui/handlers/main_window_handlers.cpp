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
 * @file main_window_handlers.cpp
 * @brief Signal handlers owned by the setup main window.
 */

#include <cstring>

#include "unikey_config.h"
#include "signal_handlers.h"

void on_main_window_destroy(GtkWidget* btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;
    gtk_main_quit();
}

void on_btn_close_clicked(GtkButton* btn, gpointer user_data)
{
    (void)btn;
    (void)user_data;
    gtk_main_quit();
}

static void cbb_config_update(GtkComboBox* cbb, const gchar* key)
{
    GValue val = {0};
    GtkTreeIter iter;
    GtkTreeModel* model = gtk_combo_box_get_model(cbb);
    gtk_combo_box_get_active_iter(cbb, &iter);
    gtk_tree_model_get_value(model, &iter, 0, &val);

    ibus_unikey_config_set_string(key, g_value_get_string(&val));
    g_value_unset(&val);
}

void on_input_method_changed(GtkComboBox* cbb, gpointer user_data)
{
    (void)user_data;
    cbb_config_update(cbb, CONFIG_INPUTMETHOD);
}

void on_output_charset_changed(GtkComboBox* cbb, gpointer user_data)
{
    (void)user_data;
    cbb_config_update(cbb, CONFIG_OUTPUTCHARSET);
}

static void cbb_config_set_active(GtkComboBox* cbb, const gchar* key)
{
    GValue val = {0};
    GtkTreeIter iter;
    GtkTreeModel* model = NULL;

    gchar* configured_value = NULL;
    const gboolean has_configured_value = ibus_unikey_config_get_string(key, &configured_value);
    if (!has_configured_value) {
        return;
    }

    model = gtk_combo_box_get_model(cbb);
    gtk_tree_model_get_iter_first(model, &iter);
    do {
        gtk_tree_model_get_value(model, &iter, 0, &val);
        const gboolean is_matching_value = (strcmp(configured_value, g_value_get_string(&val)) == 0);
        if (is_matching_value) {
            gtk_combo_box_set_active_iter(cbb, &iter);
            g_value_unset(&val);
            break;
        }
        g_value_unset(&val);
    } while (gtk_tree_model_iter_next(model, &iter));

    g_free(configured_value);
}

void on_input_method_realize(GtkComboBox* cbb, gpointer user_data)
{
    (void)user_data;
    cbb_config_set_active(cbb, CONFIG_INPUTMETHOD);
}

void on_output_charset_realize(GtkComboBox* cbb, gpointer user_data)
{
    (void)user_data;
    cbb_config_set_active(cbb, CONFIG_OUTPUTCHARSET);
}
