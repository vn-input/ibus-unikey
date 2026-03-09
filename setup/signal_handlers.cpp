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

#include "mactab.h"
#include "macro_dialog_state.h"
#include "macro_file_io.h"
#include "unikey_config.h"

#include "macro_utils.h"
#include "signal_handlers.h"

#define _(str) gettext(str)

/**
 * @file signal_handlers.cpp
 * @brief GTK signal callbacks and helper routines for the setup UI.
 *
 * This file implements the handlers attached to widgets defined in
 * `ibus-unikey.ui`.  It also contains a few static utility functions used by
 * multiple callbacks (sorting, count updates, scrolling).  The macro editor
 * dialog and table are manipulated here.
 */

/**
 * @brief Global pointer to the macro editor dialog created by the builder.
 */
extern GtkWidget* dlgMacro;

/**
 * @brief Tree view widget that displays the list of macros.
 */
extern GtkTreeView* tree_macro;

/**
 * @brief Shared GTK builder instance used by handlers to locate widgets.
 */
extern GtkBuilder* builder;

// Forward declarations for internal helpers.
/**
 * @brief Refresh the macro-count label shown in the dialog.
 *
 * Counts real macro rows in the tree model, excluding the trailing sentinel
 * placeholder row, and updates the label text with correct pluralization.
 */
static void update_macro_count_label(void);

/**
 * @brief Sort the macro list by the requested column and order.
 *
 * Installs the custom compare function so the sentinel row always remains at
 * the bottom, then applies the requested GTK sort order.
 *
 * @param column Column index to sort (`COL_KEY` or `COL_VALUE`).
 * @param order Desired sort order (`GTK_SORT_ASCENDING` or `GTK_SORT_DESCENDING`).
 */
static void sort_list_store(int column, GtkSortType order);

/**
 * @brief Update sort indicators on the tree view column headers.
 *
 * Ensures only the active sort column displays an indicator and that the arrow
 * direction matches the currently stored sort order.
 */
static void update_column_headers(void);

/**
 * @brief Add file chooser filters for supported import/export macro formats.
 *
 * Native UniKey files do not have a standard extension, so an all-files filter
 * is kept available alongside explicit YAML and plist filters.
 *
 * @param chooser GTK file chooser to configure.
 */
static void add_macro_file_filters(GtkFileChooser* chooser);

/**
 * @brief Display a descriptive error dialog for macro file operations.
 *
 * @param title Short operation-specific title.
 * @param error Detailed error object to display and free.
 */
static void show_macro_file_error_dialog(const gchar* title, GError* error);

/**
 * @brief Open the shared macro editor dialog and return the saved values.
 *
 * @param initial_key Initial key to prefill, or `NULL` for add mode.
 * @param initial_value Initial replacement text to prefill, or `NULL`.
 * @param out_key Returned newly allocated key when saved.
 * @param out_value Returned newly allocated value when saved.
 * @return `TRUE` when the user confirmed the dialog, otherwise `FALSE`.
 */
static gboolean run_macro_editor_dialog(const gchar* initial_key,
                                        const gchar* initial_value,
                                        gchar** out_key,
                                        gchar** out_value);

/**
 * @brief Insert or replace a macro row, then reselect and center it.
 *
 * @param original_key Existing row key being edited, or `NULL` for add mode.
 * @param new_key Final key to store.
 * @param new_value Final value to store.
 */
static void save_macro_entry(const gchar* original_key,
                             const gchar* new_key,
                             const gchar* new_value);

/**
 * @brief Remove the trailing sentinel row if it is present.
 *
 * @param store Macro list store.
 */
static void remove_null_placeholder_row(GtkListStore* store);

/**
 * @brief Locate a macro row by key using case-insensitive comparison.
 *
 * @param model Tree model to search.
 * @param key Key to match.
 * @param out_iter Destination iterator on success.
 * @return `TRUE` when a non-sentinel row matches, otherwise `FALSE`.
 */
static gboolean find_macro_iter_by_key(GtkTreeModel* model,
                                       const gchar* key,
                                       GtkTreeIter* out_iter);

/**
 * @brief Select and center the row whose key matches @p key.
 *
 * @param key Macro key to reveal.
 */
static void select_and_scroll_to_macro_key(const gchar* key);

/**
 * @brief Copy text into a bounded NUL-terminated string.
 *
 * @param text Source text.
 * @param max_len Maximum destination length including terminator.
 * @return Newly allocated bounded string.
 */
static gchar* duplicate_bounded_text(const gchar* text, gsize max_len);

// Sort state for columns
static GtkSortType sort_key_order = GTK_SORT_ASCENDING;   /**< current sort order for key column */
static GtkSortType sort_value_order = GTK_SORT_ASCENDING; /**< current sort order for value column */
static int current_sort_column = COL_KEY;                 /**< which column is currently sorted */

/**
 * @brief Default placeholder value inserted when a new key is added.
 */
#define MACRO_DEFAULT_VALUE _("(replace text)")

static gboolean run_macro_editor_dialog(const gchar* initial_key,
                                        const gchar* initial_value,
                                        gchar** out_key,
                                        gchar** out_value)
{
    GtkDialog* edit_dialog = GTK_DIALOG(gtk_builder_get_object(builder, "add_macro_dialog"));
    GtkEntry* entry_key = GTK_ENTRY(gtk_builder_get_object(builder, "entry_macro_key"));
    GtkTextView* value_view = GTK_TEXT_VIEW(gtk_builder_get_object(builder, "textview_macro_value"));

    if (edit_dialog == NULL || entry_key == NULL || value_view == NULL) {
        g_warning("Macro editor dialog widgets not found");
        return FALSE;
    }

    GtkTextBuffer* value_buffer = gtk_text_view_get_buffer(value_view);
    bool is_edit_mode = (initial_key != NULL && g_strcmp0(initial_key, STR_NULL_ITEM) != 0);

    gtk_window_set_title(GTK_WINDOW(edit_dialog), is_edit_mode ? _("Edit Macro") : _("Add Macro"));
    gtk_window_set_transient_for(GTK_WINDOW(edit_dialog), GTK_WINDOW(dlgMacro));

    gtk_entry_set_text(entry_key, initial_key != NULL ? initial_key : "");
    gtk_text_buffer_set_text(value_buffer, initial_value != NULL ? initial_value : "", -1);

    gtk_widget_show_all(GTK_WIDGET(edit_dialog));
    gtk_widget_grab_focus(GTK_WIDGET(entry_key));

    int response = gtk_dialog_run(edit_dialog);
    gtk_widget_hide(GTK_WIDGET(edit_dialog));
    if (response != GTK_RESPONSE_OK) {
        return FALSE;
    }

    const gchar* raw_key = gtk_entry_get_text(entry_key);
    if (raw_key == NULL || strlen(raw_key) == 0) {
        return FALSE;
    }

    GtkTextIter start_iter;
    GtkTextIter end_iter;
    gtk_text_buffer_get_bounds(value_buffer, &start_iter, &end_iter);
    gchar* raw_value = gtk_text_buffer_get_text(value_buffer, &start_iter, &end_iter, FALSE);

    *out_key = duplicate_bounded_text(raw_key, MAX_MACRO_KEY_LEN);
    *out_value = duplicate_bounded_text(raw_value, MAX_MACRO_TEXT_LEN);
    g_free(raw_value);
    return TRUE;
}

static gchar* duplicate_bounded_text(const gchar* text, gsize max_len)
{
    if (text == NULL) {
        return g_strdup("");
    }

    gchar* copy = static_cast<gchar*>(g_malloc(max_len));
    strncpy(copy, text, max_len - 1);
    copy[max_len - 1] = '\0';
    return copy;
}

static void remove_null_placeholder_row(GtkListStore* store)
{
    GtkTreeIter iter;
    int child_count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);
    if (child_count <= 0) {
        return;
    }

    gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, child_count - 1);
    gchar* last_key = NULL;
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, COL_KEY, &last_key, -1);

    bool is_null_placeholder = (g_strcmp0(last_key, STR_NULL_ITEM) == 0);
    if (is_null_placeholder) {
        gtk_list_store_remove(store, &iter);
    }

    g_free(last_key);
}

static gboolean find_macro_iter_by_key(GtkTreeModel* model,
                                       const gchar* key,
                                       GtkTreeIter* out_iter)
{
    if (model == NULL || key == NULL) {
        return FALSE;
    }

    GtkTreeIter iter;
    gboolean has_row = gtk_tree_model_get_iter_first(model, &iter);
    while (has_row) {
        gchar* iter_key = NULL;
        gtk_tree_model_get(model, &iter, COL_KEY, &iter_key, -1);

        bool is_real_row = (iter_key != NULL && g_strcmp0(iter_key, STR_NULL_ITEM) != 0);
        bool is_match = is_real_row && strcasecmp(iter_key, key) == 0;
        g_free(iter_key);

        if (is_match) {
            *out_iter = iter;
            return TRUE;
        }

        has_row = gtk_tree_model_iter_next(model, &iter);
    }

    return FALSE;
}

static void select_and_scroll_to_macro_key(const gchar* key)
{
    auto model = gtk_tree_view_get_model(tree_macro);
    if (model == NULL || key == NULL) {
        return;
    }

    GtkTreeIter iter;
    gboolean found = find_macro_iter_by_key(model, key, &iter);
    if (!found) {
        return;
    }

    GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
    gtk_tree_view_set_cursor(tree_macro, path, NULL, FALSE);
    gtk_tree_view_scroll_to_cell(tree_macro, path, NULL, TRUE, 0.5, 0.0);
    gtk_widget_grab_focus(GTK_WIDGET(tree_macro));
    gtk_tree_path_free(path);
}

static void save_macro_entry(const gchar* original_key,
                             const gchar* new_key,
                             const gchar* new_value)
{
    auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));
    auto model = GTK_TREE_MODEL(store);

    remove_null_placeholder_row(store);

    if (original_key != NULL) {
        GtkTreeIter original_iter;
        bool has_original_row = find_macro_iter_by_key(model, original_key, &original_iter);
        if (has_original_row) {
            gtk_list_store_remove(store, &original_iter);
        }
    }

    GtkTreeIter conflicting_iter;
    bool has_conflicting_row = find_macro_iter_by_key(model, new_key, &conflicting_iter);
    if (has_conflicting_row) {
        gtk_list_store_remove(store, &conflicting_iter);
    }

    GtkTreeIter new_iter;
    gtk_list_store_append(store, &new_iter);
    gtk_list_store_set(store, &new_iter, COL_KEY, new_key, COL_VALUE, new_value, -1);

    list_store_add_null_item(store);
    sort_list_store(current_sort_column,
        current_sort_column == COL_KEY ? sort_key_order : sort_value_order);

    while (gtk_events_pending()) {
        gtk_main_iteration();
    }

    select_and_scroll_to_macro_key(new_key);
    update_macro_count_label();
}

/**
 * @brief Install file chooser filters for native text, YAML, plist, and generic files.
 *
 * @param chooser GTK file chooser to receive the filters.
 */
static void add_macro_file_filters(GtkFileChooser* chooser)
{
    GtkFileFilter* filter_supported = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_supported, _("Supported macro files"));
    gtk_file_filter_add_pattern(filter_supported, "*.txt");
    gtk_file_filter_add_pattern(filter_supported, "*.yml");
    gtk_file_filter_add_pattern(filter_supported, "*.yaml");
    gtk_file_filter_add_pattern(filter_supported, "*.plist");
    gtk_file_chooser_add_filter(chooser, filter_supported);

    GtkFileFilter* filter_native = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_native, _("UniKey text macro files"));
    gtk_file_filter_add_pattern(filter_native, "*.txt");
    gtk_file_chooser_add_filter(chooser, filter_native);

    GtkFileFilter* filter_yaml = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_yaml, _("YAML files"));
    gtk_file_filter_add_pattern(filter_yaml, "*.yml");
    gtk_file_filter_add_pattern(filter_yaml, "*.yaml");
    gtk_file_chooser_add_filter(chooser, filter_yaml);

    GtkFileFilter* filter_plist = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_plist, _("macOS plist files"));
    gtk_file_filter_add_pattern(filter_plist, "*.plist");
    gtk_file_chooser_add_filter(chooser, filter_plist);

    GtkFileFilter* filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all, dgettext("gtk30", "All Files"));
    gtk_file_filter_add_pattern(filter_all, "*");
    gtk_file_chooser_add_filter(chooser, filter_all);
}

/**
 * @brief Show a message dialog for import/export failures.
 *
 * @param title Dialog primary text.
 * @param error Error containing the detailed message. Ownership is consumed.
 */
static void show_macro_file_error_dialog(const gchar* title, GError* error)
{
    const gchar* detail = (error != NULL && error->message != NULL)
        ? error->message
        : _("Unknown error.");

    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(dlgMacro),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_ERROR,
                                               GTK_BUTTONS_OK,
                                               "%s",
                                               title);
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", detail);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (error != NULL) {
        g_error_free(error);
    }
}

/**
 * @brief Handle key presses on the main window.
 *
 * *Escape* is treated as a request to quit the application.
 *
 * @param widget The main window receiving the key event.
 * @param event Key event structure supplied by GTK.
 * @param data Unused callback data.
 * @return `TRUE` if the event was consumed, otherwise `FALSE`.
 */
gboolean on_main_window_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    if (event->keyval == GDK_KEY_Escape) {
        gtk_main_quit();
        return true;
    }
    return false;
}

/**
 * @brief Quit the GTK main loop when the main window is destroyed.
 *
 * This is connected to the `destroy` signal of the top-level window.
 *
 * @param btn Unused widget pointer (main window).
 * @param user_data Unused.
 */
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
void on_setting_toggled(GtkToggleButton* btn, gpointer user_data)
{
    const gchar* key = gtk_widget_get_name(GTK_WIDGET(btn));
    key = key + 4; // skip "cfg_"

    gboolean b = gtk_toggle_button_get_active(btn);
    ibus_unikey_config_set_boolean(key, b);
}

/**
 * @brief Initialize a toggle button's state from configuration when realized.
 *
 * Like `on_setting_toggled`, this assumes the widget name starts with
 * "cfg_".  If the key exists in config its value is used to set the button's
 * active state.
 *
 * @param btn        The toggle button being initialized.
 * @param user_data  Unused pointer provided by GTK.
 */
void on_setting_realize(GtkToggleButton* btn, gpointer user_data)
{
    const gchar* key = gtk_widget_get_name(GTK_WIDGET(btn));
    key = key + 4; // skip "cfg_"

    gboolean b;
    if (ibus_unikey_config_get_boolean(key, &b))
    {
        gtk_toggle_button_set_active(btn, b);
    }
}

/**
 * @brief Display the macro editor dialog and handle load/save operations.
 *
 * This callback is invoked when the "Edit" button is clicked.  It reads the
 * current macro file into a `CMacroTable`, populates the tree view, allows the
 * user to modify entries, and writes the table back if the dialog is closed with
 * the OK response.  The tree's model is temporarily detached during bulk
 * updates for performance.
 *
 * @param btn Button widget that opened the editor (unused).
 * @param user_data Unused.
 */
void on_btn_macroedit_clicked(GtkButton* btn, gpointer user_data)
{
    gchar* macrofile = get_macro_file();

    CMacroTable macro;
    macro.init();
    macro.loadFromFile(macrofile);

    auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));
    
    // Detach model from tree view to avoid per-row UI updates (massive speedup)
    g_object_ref(store);
    gtk_tree_view_set_model(tree_macro, NULL);
    
    unikey_macro_to_store(&macro, store);
    
    // Re-attach model
    gtk_tree_view_set_model(tree_macro, GTK_TREE_MODEL(store));
    g_object_unref(store);

    // Initial sort and count update
    sort_list_store(COL_KEY, GTK_SORT_ASCENDING);
    sort_key_order = GTK_SORT_ASCENDING;
    current_sort_column = COL_KEY;
    update_column_headers();
    update_macro_count_label();
    
    int ret = gtk_dialog_run(GTK_DIALOG(dlgMacro));
    if (ret == GTK_RESPONSE_OK)
    {
        unikey_store_to_macro(store, &macro);

        GFile* f = g_file_get_parent(g_file_new_for_path(macrofile));
        if (g_file_query_exists(f, NULL) == FALSE)
        {
            g_file_make_directory_with_parents(f, NULL, NULL);
        }
        g_object_unref(f);

        macro.writeToFile(macrofile);
    }

    g_free(macrofile);
}

/**
 * @brief Handler for the delete-event on the macro dialog window.
 *
 * Hides the dialog instead of destroying it so it can be reused.
 *
 * @param wid        The dialog widget being closed.
 * @param ev         Event structure describing the delete request.
 * @param user_data  Unused pointer.
 * @return `TRUE` to stop further handlers.
 */
gboolean on_macro_dialog_delete(GtkWidget* wid, GdkEvent* ev, gpointer user_data)
{
    gtk_widget_hide(dlgMacro);
    return true;
}

/**
 * @brief Simple helper bound to the cancel/close button inside the macro dialog.
 *
 * @param btn Button that triggered the hide action (unused).
 * @param user_data Unused.
 */
void macro_dialog_hide(GtkButton* btn, gpointer user_data)
{
    gtk_widget_hide(dlgMacro);
}

/**
 * @brief Callback invoked when the key cell of a row is edited in-place.
 *
 * Performs validation, rejects duplicates or sentinel text, and ensures a
 * null placeholder row remains at the end.
 *
 * @param celltext The cell renderer that was edited.
 * @param string_path String representation of the tree path of the edited row.
 * @param newkey The new text typed by the user.
 * @param data Unused user data.
 */
void on_cell_key_edited(GtkCellRendererText* celltext,
                        const gchar* string_path,
                        const gchar* newkey,
                        gpointer data)
{
    GtkTreeIter iter;
    gchar *oldkey;
    gchar nkey[MAX_MACRO_KEY_LEN];

    auto model = gtk_tree_view_get_model(tree_macro);

    strncpy(nkey, newkey, MAX_MACRO_KEY_LEN-1);
    nkey[MAX_MACRO_KEY_LEN-1] = '\0';

    if (strcmp(nkey, STR_NULL_ITEM) == 0
        || (strlen(STR_NULL_ITEM) != 0 && strlen(nkey) == 0))
        return;

    if (list_store_check_exists(GTK_LIST_STORE(model), nkey))
        return;

    gtk_tree_model_get_iter_from_string(model, &iter, string_path);
    gtk_tree_model_get(model, &iter, COL_KEY, &oldkey, -1);
    if (strcmp(oldkey, STR_NULL_ITEM) == 0)
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_KEY, nkey, COL_VALUE, MACRO_DEFAULT_VALUE, -1);
    else
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_KEY, nkey, -1);
    g_free(oldkey);

    list_store_add_null_item(GTK_LIST_STORE(model));
}

/**
 * @brief Callback invoked when the value cell of a row is edited in-place.
 *
 * The new text is truncated to the maximum allowed length. Entries with the
 * null sentinel key are ignored.
 *
 * @param celltext    The cell renderer that fired the edit signal.
 * @param string_path String representation of the tree path for the row.
 * @param newvalue    The text entered by the user.
 * @param data        Unused user data pointer.
 */
void on_cell_value_edited(GtkCellRendererText* celltext,
                          const gchar* string_path,
                          const gchar* newvalue,
                          gpointer data)
{
    GtkTreeIter iter;
    gchar *key;
    gchar value[MAX_MACRO_TEXT_LEN];

    auto model = gtk_tree_view_get_model(tree_macro);
    gtk_tree_model_get_iter_from_string(model, &iter, string_path);
    gtk_tree_model_get(model, &iter, COL_KEY, &key, -1);

    strncpy(value, newvalue, MAX_MACRO_TEXT_LEN-1);
    value[MAX_MACRO_TEXT_LEN-1] = '\0';

    if (strcmp(key, STR_NULL_ITEM) != 0)
    {
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_VALUE, value, -1);
    }
    g_free(key);
}

/**
 * @brief Remove the currently selected macro row (if not the null placeholder).
 *
 * Updates the count label and keeps the selection on the subsequent row.
 *
 * @param button     The Delete button that was clicked.
 * @param user_data  Unused pointer.
 */
void on_btn_macro_del_clicked(GtkButton* button, gpointer user_data)
{
    GtkTreeIter     iter;
    auto select = gtk_tree_view_get_selection(tree_macro);
    if (gtk_tree_selection_get_selected(select, NULL, &iter) == TRUE)
    {
        auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));
        gchar *key;
        gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, COL_KEY, &key, -1);
        if (strcmp(key, STR_NULL_ITEM) != 0)
        {
            gtk_list_store_remove(store, &iter);
            update_macro_count_label();
        }
        gtk_tree_selection_select_iter(select, &iter); // select current index
        g_free(key);
    }
}

/**
 * @brief Clear all macros from the list, leaving only the null placeholder.
 *
 * Removes every row in the tree store except the sentinel and resets
 * selection to the first entry.
 *
 * @param button  The Clear button that was clicked (unused).
 * @param data    Unused user data pointer.
 */
void on_btn_macro_clear_clicked(GtkButton* button, gpointer data)
{
    auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));
    gtk_list_store_clear(store);
    list_store_add_null_item(store);

    auto select = gtk_tree_view_get_selection(tree_macro);
    GtkTreeIter iter;
    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    gtk_tree_selection_select_iter(select, &iter);
    update_macro_count_label();
}

/**
 * @brief Import macros from a file and merge them into the current list.
 *
 * A file chooser dialog is presented; selected entries are loaded into a
 * temporary CMacroTable and appended to the tree store using duplicate checking.
 * The previous selection moves to the first row, and the count/ sort state is
 * updated.
 *
 * @param button Import button that triggered the file chooser (unused).
 * @param data Unused callback data.
 */
void on_btn_macro_import_clicked(GtkButton* button, gpointer data)
{
    auto file = gtk_file_chooser_dialog_new(_("Import macro"),
                                       GTK_WINDOW(dlgMacro),
                                       GTK_FILE_CHOOSER_ACTION_OPEN,
                                       dgettext("gtk30", "_Cancel"), GTK_RESPONSE_CANCEL,
                                       dgettext("gtk30", "_Open"), GTK_RESPONSE_OK,
                                       NULL);
    GtkFileChooser* chooser = GTK_FILE_CHOOSER(file);
    add_macro_file_filters(chooser);
    macro_dialog_apply_import_folder(chooser);

    if (gtk_dialog_run(GTK_DIALOG(file)) == GTK_RESPONSE_OK)
    {
        auto fn = gtk_file_chooser_get_filename(chooser);
        macro_dialog_remember_import_file(fn);
        CMacroTable macro;
        GError* error = NULL;
        const gboolean is_loaded = macro_table_load_any_format(fn, &macro, &error);
        if (!is_loaded)
        {
            show_macro_file_error_dialog(_("Failed to import macros."), error);
            g_free(fn);
            gtk_widget_destroy(file);
            return;
        }
        g_free(fn);

        auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));

        // Detach model from tree view to avoid per-row UI updates (massive speedup)
        g_object_ref(store);
        gtk_tree_view_set_model(tree_macro, NULL);

        GtkTreeIter iter;
        auto n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);     // get number of iter
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, n-1); // get last iter
        gtk_list_store_remove(store, &iter); // remove last iter (...)

        list_store_append(store, &macro);
        list_store_add_null_item(store); // add iter (...)

        // Re-attach model
        gtk_tree_view_set_model(tree_macro, GTK_TREE_MODEL(store));
        g_object_unref(store);

        // select first iter
        auto select = gtk_tree_view_get_selection(tree_macro);
        gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
        gtk_tree_selection_select_iter(select, &iter);
        
        // Re-sort and update count
        sort_list_store(current_sort_column,
            current_sort_column == COL_KEY ? sort_key_order : sort_value_order);
        update_macro_count_label();
    }

    gtk_widget_destroy(file);
}

/**
 * @brief Export the current macros to a file chosen by the user.
 *
 * Presents a save file chooser; all non-placeholder rows are copied into a
 * temporary `CMacroTable` which is then written in engine format.
 *
 * @param button The Export button clicked (unused).
 * @param data   Unused pointer.
 */
void on_btn_macro_export_clicked(GtkButton* button, gpointer data)
{
    auto file = gtk_file_chooser_dialog_new(_("Export macro"),
                                       GTK_WINDOW(dlgMacro),
                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                       dgettext("gtk30", "_Cancel"), GTK_RESPONSE_CANCEL,
                                       dgettext("gtk30", "_Save"), GTK_RESPONSE_OK,
                                       NULL);
    GtkFileChooser* chooser = GTK_FILE_CHOOSER(file);
    gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    add_macro_file_filters(chooser);
    macro_dialog_apply_export_folder(chooser);

    if (gtk_dialog_run(GTK_DIALOG(file)) == GTK_RESPONSE_OK)
    {
        CMacroTable macro;
        macro.init();

        auto model = GTK_TREE_MODEL(gtk_tree_view_get_model(tree_macro));

        GtkTreeIter iter;
        gchar* key, *value;
        gboolean b = gtk_tree_model_get_iter_first(model, &iter);
        while (b == TRUE)
        {
            gtk_tree_model_get(model, &iter, COL_KEY, &key, COL_VALUE, &value, -1);
            if (strcasecmp(key, STR_NULL_ITEM) != 0)
            {
                macro.addItem(key, value, CONV_CHARSET_XUTF8);
            }
            g_free(key);
            g_free(value);
            b = gtk_tree_model_iter_next(model, &iter);
        }

        auto fn = gtk_file_chooser_get_filename(chooser);
        macro_dialog_remember_export_file(fn);
        GError* error = NULL;
        if (!macro_table_write_any_format(fn, &macro, &error))
        {
            show_macro_file_error_dialog(_("Failed to export macros."), error);
            g_free(fn);
            gtk_widget_destroy(file);
            return;
        }
        g_free(fn);
    }

    gtk_widget_destroy(file);
}

/**
 * @brief Update the label that displays the number of macros currently shown.
 *
 * Counts the rows in the tree model excluding the null placeholder and updates
 * the "lbl_macro_count" label accordingly, using correct pluralization.
 */
static void update_macro_count_label(void)
{
    GtkLabel* lbl = GTK_LABEL(gtk_builder_get_object(builder, "lbl_macro_count"));
    if (lbl == NULL) return;

    auto model = gtk_tree_view_get_model(tree_macro);
    if (model == NULL) return;

    int count = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(model), NULL);
    // Subtract 1 for the null item placeholder
    if (count > 0) count--;

    gchar* text = g_strdup_printf(ngettext("%d macro", "%d macros", count), count);
    gtk_label_set_text(lbl, text);
    g_free(text);
}

/**
 * @brief Custom comparison function for sorting tree store rows.
 *
 * Ensures the null sentinel row always sorts to the very end, regardless of the
 * chosen column or order.  Otherwise uses `g_utf8_collate` for locale-aware
 * comparison of the text in the requested column.
 *
 * @param model     The tree model containing rows to compare.
 * @param a         Iterator pointing to the first row.
 * @param b         Iterator pointing to the second row.
 * @param user_data Integer column index (packed in a gpointer) to compare.
 * @return <0, 0, or >0 as in `g_utf8_collate`.
 */
static int compare_store_items(GtkTreeModel* model,
                               GtkTreeIter* a,
                               GtkTreeIter* b,
                               gpointer user_data)
{
    gchar *key_a, *key_b;
    int col = GPOINTER_TO_INT(user_data);
    gtk_tree_model_get(model, a, col, &key_a, -1);
    gtk_tree_model_get(model, b, col, &key_b, -1);

    // Always put null item at the end
    if (g_strcmp0(key_a, STR_NULL_ITEM) == 0) {
        g_free(key_a);
        g_free(key_b);
        return 1;
    }
    if (g_strcmp0(key_b, STR_NULL_ITEM) == 0) {
        g_free(key_a);
        g_free(key_b);
        return -1;
    }

    int result = g_utf8_collate(key_a ? key_a : "", key_b ? key_b : "");
    g_free(key_a);
    g_free(key_b);
    return result;
}

/**
 * @brief Apply sorting to the macro tree store.
 *
 * This routine installs a custom sort func (`compare_store_items`) to ensure
 * the null placeholder remains last, and then updates the sort column and
 * order.
 *
 * @param column Column index to sort (COL_KEY or COL_VALUE).
 * @param order  GTK_SORT_ASCENDING or GTK_SORT_DESCENDING.
 */
static void sort_list_store(int column, GtkSortType order)
{
    auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));
    auto sortable = GTK_TREE_SORTABLE(store);

    // Keep explicit sort function so sentinel/null row stays at bottom
    // regardless of selected sort column or order.
    gtk_tree_sortable_set_sort_func(sortable, column, compare_store_items, GINT_TO_POINTER(column), NULL);
    gtk_tree_sortable_set_sort_column_id(sortable, column, order);
    current_sort_column = column;
}

/**
 * @brief Refresh the sort indicators on the tree view headers.
 *
 * Only the currently active column shows an arrow; the direction matches the
 * stored sort order variables.
 */
static void update_column_headers(void)
{
    GtkTreeViewColumn* col_key = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "treeviewcolumn1"));
    GtkTreeViewColumn* col_value = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "treeviewcolumn2"));

    // Use GTK's native sort indicator - just toggle visibility
    gtk_tree_view_column_set_sort_indicator(col_key, current_sort_column == COL_KEY);
    gtk_tree_view_column_set_sort_indicator(col_value, current_sort_column == COL_VALUE);

    // Set sort order for the indicator arrow direction
    if (current_sort_column == COL_KEY) {
        gtk_tree_view_column_set_sort_order(col_key, sort_key_order);
    } else {
        gtk_tree_view_column_set_sort_order(col_value, sort_value_order);
    }
}

/**
 * @brief Scroll the tree view so the current selection (or first row) is centered.
 *
 * Used after sorting so that the user does not lose context.  If no row is
 * selected, the first row is chosen.
 */
static void scroll_to_selection_or_top(void)
{
    auto model = gtk_tree_view_get_model(tree_macro);
    if (model == NULL) return;

    GtkTreeIter iter;
    GtkTreePath* path = NULL;

    // Check if there's a selection
    auto select = gtk_tree_view_get_selection(tree_macro);
    if (gtk_tree_selection_get_selected(select, NULL, &iter)) {
        path = gtk_tree_model_get_path(model, &iter);
    } else if (gtk_tree_model_get_iter_first(model, &iter)) {
        // No selection, use first row
        path = gtk_tree_model_get_path(model, &iter);
    }

    if (path != NULL) {
        // UX rule: when sorting, keep user's context centered on the
        // previously selected row; if no selection exists, center first row.
        gtk_tree_view_scroll_to_cell(tree_macro, path, NULL, TRUE, 0.5, 0.0);
        gtk_tree_path_free(path);
    }
}

/**
 * @brief Handler for clicking the key column header.
 *
 * Toggles the sort order if already sorting by key; otherwise switches to key
 * sorting ascending. After sorting, headers and scroll position are updated.
 *
 * @param column    The column header that was clicked.
 * @param user_data Unused pointer provided by GTK.
 */
void on_column_key_clicked(GtkTreeViewColumn* column, gpointer user_data)
{
    if (current_sort_column == COL_KEY) {
        sort_key_order = (sort_key_order == GTK_SORT_ASCENDING) ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING;
    } else {
        sort_key_order = GTK_SORT_ASCENDING;
    }
    sort_list_store(COL_KEY, sort_key_order);
    update_column_headers();
    scroll_to_selection_or_top();
}

/**
 * @brief Handler for clicking the value column header.
 *
 * Behaves similarly to `on_column_key_clicked` but for the value column.
 *
 * @param column    The column header that was clicked.
 * @param user_data Unused pointer provided by GTK.
 */
void on_column_value_clicked(GtkTreeViewColumn* column, gpointer user_data)
{
    if (current_sort_column == COL_VALUE) {
        sort_value_order = (sort_value_order == GTK_SORT_ASCENDING) ? GTK_SORT_DESCENDING : GTK_SORT_ASCENDING;
    } else {
        sort_value_order = GTK_SORT_ASCENDING;
    }
    sort_list_store(COL_VALUE, sort_value_order);
    update_column_headers();
    scroll_to_selection_or_top();
}

/**
 * @brief Add a new macro via a secondary dialog.
 *
 * Displays a simple dialog where the user can type a key/value pair.  After
 * validation and duplicate checking the new row is inserted, the list is
 * resorted, and the new entry is scrolled into view and selected.
 *
 * @param button    The Add button that was clicked (unused).
 * @param user_data Unused pointer.
 */
void on_btn_macro_add_clicked(GtkButton* button, gpointer user_data)
{
    gchar* new_key = NULL;
    gchar* new_value = NULL;
    bool should_save = run_macro_editor_dialog(NULL, NULL, &new_key, &new_value);
    if (!should_save) {
        return;
    }

    save_macro_entry(NULL, new_key, new_value);
    g_free(new_key);
    g_free(new_value);
}

void on_macro_row_activated(GtkTreeView* tree_view,
                            GtkTreePath* path,
                            GtkTreeViewColumn* column,
                            gpointer user_data)
{
    GtkTreeIter iter;
    auto model = gtk_tree_view_get_model(tree_view);
    bool has_row = gtk_tree_model_get_iter(model, &iter, path);
    if (!has_row) {
        return;
    }

    gchar* key = NULL;
    gchar* value = NULL;
    gtk_tree_model_get(model, &iter, COL_KEY, &key, COL_VALUE, &value, -1);

    const gchar* initial_key = (g_strcmp0(key, STR_NULL_ITEM) == 0) ? NULL : key;
    const gchar* initial_value = (g_strcmp0(key, STR_NULL_ITEM) == 0) ? NULL : value;

    gchar* saved_key = NULL;
    gchar* saved_value = NULL;
    bool should_save = run_macro_editor_dialog(initial_key, initial_value, &saved_key, &saved_value);
    if (should_save) {
        save_macro_entry(initial_key, saved_key, saved_value);
    }

    g_free(saved_key);
    g_free(saved_value);
    g_free(key);
    g_free(value);
}
