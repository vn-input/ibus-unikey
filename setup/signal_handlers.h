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

#ifndef __SETUP_SIGNAL_HANDLERS_H__
#define __SETUP_SIGNAL_HANDLERS_H__

#include <gtk/gtk.h>

/**
 * @file signal_handlers.h
 * @brief External declarations for GTK signal callbacks used by the setup UI.
 *
 * These functions are defined in `signal_handlers.cpp` and are exported with
 * `G_MODULE_EXPORT` so that the GTK builder can connect them by name.  Their
 * signatures match the widgets and signals defined in `ibus-unikey.ui`.
 */

extern "C"
{
    /**
     * @brief Main window destroy handler (quits main loop).
     */
    G_MODULE_EXPORT void on_main_window_destroy(GtkWidget* w, gpointer user_data);

    /**
     * @brief Key press event handler for the main window.
     */
    G_MODULE_EXPORT gboolean on_main_window_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data);

    /**
     * @brief "Close" button clicked handler.
     */
    G_MODULE_EXPORT void on_btn_close_clicked(GtkButton* btn, gpointer user_data);
    
    /**
     * @brief Checkbox toggled configuration handler.
     */
    G_MODULE_EXPORT void on_setting_toggled(GtkToggleButton* btn, gpointer user_data);

    /**
     * @brief Initialize toggle button state from configuration.
     */
    G_MODULE_EXPORT void on_setting_realize(GtkToggleButton* btn, gpointer user_data);

    /**
     * @brief Refresh macro list dialog.
     */
    G_MODULE_EXPORT void on_btn_macroedit_clicked(GtkButton* btn, gpointer user_data);

    /**
     * @brief Input method combo changed.
     */
    G_MODULE_EXPORT void on_input_method_changed(GtkComboBox* cbb, gpointer user_data);

    /**
     * @brief Input method combo "realize" event.
     */
    G_MODULE_EXPORT void on_input_method_realize(GtkComboBox* cbb, gpointer user_data);

    /**
     * @brief Output charset combo changed.
     */
    G_MODULE_EXPORT void on_output_charset_changed(GtkComboBox* cbb, gpointer user_data);

    /**
     * @brief Output charset combo "realize" event.
     */
    G_MODULE_EXPORT void on_output_charset_realize(GtkComboBox* cbb, gpointer user_data);

    /**
     * @brief Macro dialog delete-event handler.
     */
    G_MODULE_EXPORT gboolean on_macro_dialog_delete(GtkWidget* wid, GdkEvent* ev, gpointer user_data);

    /**
     * @brief Close/Cancel button handler inside macro dialog.
     */
    G_MODULE_EXPORT void macro_dialog_hide(GtkButton* btn, gpointer user_data);

    /**
     * @brief Add macro button clicked.
     */
    G_MODULE_EXPORT void on_btn_macro_add_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief Delete selected macro button clicked.
     */
    G_MODULE_EXPORT void on_btn_macro_del_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief Clear all macros button clicked.
     */
    G_MODULE_EXPORT void on_btn_macro_clear_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief Import macros from file button clicked.
     */
    G_MODULE_EXPORT void on_btn_macro_import_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief Export macros to file button clicked.
     */
    G_MODULE_EXPORT void on_btn_macro_export_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief Key column header clicked (for sorting).
     */
    G_MODULE_EXPORT void on_column_key_clicked(GtkTreeViewColumn *column, gpointer user_data);

    /**
     * @brief Value column header clicked (for sorting).
     */
    G_MODULE_EXPORT void on_column_value_clicked(GtkTreeViewColumn *column, gpointer user_data);

    /**
     * @brief In-place edit of a key cell.
     */
    G_MODULE_EXPORT void on_cell_key_edited(GtkCellRendererText *celltext,
                    const gchar *string_path,
                    const gchar *newkey,
                    gpointer user_data);

    /**
     * @brief In-place edit of a value cell.
     */
    G_MODULE_EXPORT void on_cell_value_edited(GtkCellRendererText *celltext,
                     const gchar *string_path,
                     const gchar *newvalue,
                     gpointer data);
}

#endif // __SETUP_SIGNAL_HANDLERS_H__
