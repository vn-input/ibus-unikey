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
     * @brief Called when the main window is destroyed.
     *
     * Typically invoked when the user closes the window; this handler
     * quits the GTK main loop.
     *
     * @param w The main window widget being destroyed.
     * @param user_data Unused pointer provided by GTK builder.
     */
    G_MODULE_EXPORT void on_main_window_destroy(GtkWidget* w, gpointer user_data);

    /**
     * @brief Handle key press events on the main window.
     *
     * Used to implement global shortcuts (e.g. Esc to close).
     *
     * @param widget The window receiving the event.
     * @param event Event structure describing key details.
     * @param data Unused user data pointer.
     * @return `TRUE` if the event was consumed, `FALSE` to propagate.
     */
    G_MODULE_EXPORT gboolean on_main_window_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data);

    /**
     * @brief Respond to the Close button being clicked.
     *
     * Typically hides or destroys the setup window.
     *
     * @param btn The `GtkButton` that was clicked (Close button).
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_btn_close_clicked(GtkButton* btn, gpointer user_data);
    
    /**
     * @brief A configuration toggle button was changed by the user.
     *
     * The handler updates the internal settings to match the new state.
     *
     * @param btn The `GtkToggleButton` that emitted the signal.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_setting_toggled(GtkToggleButton* btn, gpointer user_data);

    /**
     * @brief Set the initial state of a toggle button when it is realized.
     *
     * Reads stored configuration and updates the widget accordingly.
     *
     * @param btn The toggle button being realized.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_setting_realize(GtkToggleButton* btn, gpointer user_data);

    /**
     * @brief Show or refresh the macro editor dialog.
     *
     * Invoked when the "Edit macros" button is clicked in the main window.
     *
     * @param btn The button that was clicked.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_btn_macroedit_clicked(GtkButton* btn, gpointer user_data);

    /**
     * @brief Invoked when the input method dropdown selection changes.
     *
     * @param cbb The `GtkComboBox` representing input methods.
     * @param user_data User data pointer (unused).
     */
    G_MODULE_EXPORT void on_input_method_changed(GtkComboBox* cbb, gpointer user_data);

    /**
     * @brief Handler for the input-method combo box `realize` event.
     *
     * Typically used to populate or adjust the widget once it has been
     * created and added to the window.
     *
     * @param cbb The `GtkComboBox` being realized.
     * @param user_data User data pointer (unused).
     */
    G_MODULE_EXPORT void on_input_method_realize(GtkComboBox* cbb, gpointer user_data);

    /**
     * @brief Called when the output charset selection is changed by the user.
     *
     * @param cbb The `GtkComboBox` widget whose active item was changed.
     * @param user_data Pointer to user data (unused).
     */
    G_MODULE_EXPORT void on_output_charset_changed(GtkComboBox* cbb, gpointer user_data);

    /**
     * @brief Handler for the output-charset combo box `realize` event.
     *
     * Used to configure the widget after it has been constructed by GTK.
     *
     * @param cbb The `GtkComboBox` being realized.
     * @param user_data User data pointer (unused).
     */
    G_MODULE_EXPORT void on_output_charset_realize(GtkComboBox* cbb, gpointer user_data);

    /**
     * @brief Handle the delete-event (window close) for the macro dialog.
     *
     * Returning `TRUE` prevents the dialog from being destroyed so it can be
     * hidden instead.
     *
     * @param wid The macro dialog widget.
     * @param ev The delete event structure.
     * @param user_data Unused pointer.
     * @return gboolean `TRUE` to stop destruction, `FALSE` to proceed.
     */
    G_MODULE_EXPORT gboolean on_macro_dialog_delete(GtkWidget* wid, GdkEvent* ev, gpointer user_data);

    G_MODULE_EXPORT gboolean on_macro_dialog_configure_event(GtkWidget* wid, GdkEvent* event, gpointer user_data);

    G_MODULE_EXPORT void on_macro_search_changed(GtkEditable* editable, gpointer user_data);

    G_MODULE_EXPORT void on_macro_selection_changed(GtkTreeSelection* selection, gpointer user_data);

    /**
     * @brief User clicked the Close/Cancel button in the macro dialog.
     *
     * Hides the dialog without applying any changes.
     *
     * @param btn The button clicked.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void macro_dialog_hide(GtkButton* btn, gpointer user_data);

    /**
     * @brief Add button in macro dialog was clicked.
     *
     * Prompts user for a new macro and appends it to the list.
     *
     * @param button The add button.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_btn_macro_add_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief Open the shared macro editor dialog for the activated row.
     *
     * Double-clicking or pressing Enter on a macro row opens the same dialog
     * used for adding entries, prefilled with the current key/value.
     * Activating the sentinel row opens the dialog in add mode.
     *
     * @param tree_view The macro tree view.
     * @param path Tree path of the activated row.
     * @param column Activated column (unused).
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_macro_row_activated(GtkTreeView *tree_view,
                     GtkTreePath *path,
                     GtkTreeViewColumn *column,
                     gpointer user_data);

    /**
     * @brief Delete button clicked in macro dialog.
     *
     * Removes the currently selected macro from the table.
     *
     * @param button The delete button.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_btn_macro_del_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief Clear-all button clicked in macro dialog.
     *
     * Removes every macro entry after asking for confirmation.
     *
     * @param button The clear-all button.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_btn_macro_clear_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief Import button clicked in macro dialog.
     *
     * Opens file chooser and loads macros from selected file.
     *
     * @param button The import button.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_btn_macro_import_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief Export button clicked in macro dialog.
     *
     * Saves current macro list to a user-specified file.
     *
     * @param button The export button.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_btn_macro_export_clicked(GtkButton *button, gpointer user_data);

    /**
     * @brief User clicked the key column header in macro table.
     *
     * Toggles sorting order by macro key.
     *
     * @param column The `GtkTreeViewColumn` for keys.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_column_key_clicked(GtkTreeViewColumn *column, gpointer user_data);

    /**
     * @brief User clicked the value column header in macro table.
     *
     * Toggles sorting order by macro value.
     *
     * @param column The `GtkTreeViewColumn` for values.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_column_value_clicked(GtkTreeViewColumn *column, gpointer user_data);

    /**
     * @brief Callback invoked when a macro key cell is edited inline.
     *
     * @param celltext The renderer for the edited cell.
     * @param string_path String identifying the row in the model.
     * @param newkey The new key string entered by the user.
     * @param user_data Unused pointer.
     */
    G_MODULE_EXPORT void on_cell_key_edited(GtkCellRendererText *celltext,
                    const gchar *string_path,
                    const gchar *newkey,
                    gpointer user_data);

    /**
     * @brief Callback invoked when a macro value cell is edited inline.
     *
     * @param celltext The renderer for the edited cell.
     * @param string_path String identifying the row in the model.
     * @param newvalue The new value string entered by the user.
     * @param data Unused pointer.
     */
    G_MODULE_EXPORT void on_cell_value_edited(GtkCellRendererText *celltext,
                     const gchar *string_path,
                     const gchar *newvalue,
                     gpointer data);
}

#endif // __SETUP_SIGNAL_HANDLERS_H__
