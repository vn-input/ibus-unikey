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
static GtkEntry* get_macro_search_entry(void);
static gboolean macro_row_matches_search(const gchar* key,
                                         const gchar* value,
                                         const gchar* search_text);
static gboolean find_first_macro_iter_by_search(GtkTreeModel* model,
                                                const gchar* search_text,
                                                GtkTreeIter* out_iter);
static void apply_macro_search_selection(void);

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
 * @brief Get the preferred export extension from the selected file filter.
 *
 * @param chooser GTK file chooser being used for export.
 * @return Extension including the leading dot, or `NULL` when none is defined.
 */
static const gchar* get_selected_macro_export_extension(GtkFileChooser* chooser);

/**
 * @brief Select the `.txt` export filter by default.
 *
 * @param chooser GTK file chooser used for export.
 */
static void set_default_macro_export_filter(GtkFileChooser* chooser);
static void set_default_macro_export_name(GtkFileChooser* chooser);

/**
 * @brief Ask whether a mismatched filename extension should keep the chosen export type.
 *
 * @param typed_filename Filename entered by the user before completion.
 * @param export_filename Final filename after appending the selected extension.
 * @param selected_extension Chosen extension including the leading dot.
 * @return `TRUE` when export should proceed, otherwise `FALSE`.
 */
static gboolean confirm_macro_export_extension_conflict(const gchar* typed_filename,
                                                        const gchar* export_filename,
                                                        const gchar* selected_extension);

/**
 * @brief Ask whether an auto-completed export path may overwrite an existing file.
 *
 * @param filename Final export filename after appending the selected extension.
 * @return `TRUE` when overwrite should proceed, otherwise `FALSE`.
 */
static gboolean confirm_macro_export_overwrite(const gchar* filename);

/**
 * @brief Display a descriptive error dialog for macro file operations.
 *
 * @param title Short operation-specific title.
 * @param error Detailed error object to display and free.
 */
static void show_macro_file_error_dialog(const gchar* title, GError* error);
static void show_macro_export_success_dialog(const gchar* filename, int exported_count);

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
#define MACRO_FILE_FILTER_EXTENSION_KEY "unikey-macro-file-extension"
#define MACRO_TEXT_FILE_EXTENSION ".txt"
#define MACRO_YAML_FILE_EXTENSION ".yaml"
#define MACRO_PLIST_FILE_EXTENSION ".plist"

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

static GtkEntry* get_macro_search_entry(void)
{
    return GTK_ENTRY(gtk_builder_get_object(builder, "entry_macro_search"));
}

static gboolean macro_row_matches_search(const gchar* key,
                                         const gchar* value,
                                         const gchar* search_text)
{
    bool has_search_text = (search_text != NULL && search_text[0] != '\0');
    if (!has_search_text) {
        return TRUE;
    }

    bool is_placeholder_row = (g_strcmp0(key, STR_NULL_ITEM) == 0);
    if (is_placeholder_row) {
        return FALSE;
    }

    gchar* lowered_search = g_utf8_strdown(search_text, -1);
    gchar* lowered_key = g_utf8_strdown(key != NULL ? key : "", -1);
    gchar* lowered_value = g_utf8_strdown(value != NULL ? value : "", -1);
    gboolean is_match = (g_strstr_len(lowered_key, -1, lowered_search) != NULL)
        || (g_strstr_len(lowered_value, -1, lowered_search) != NULL);
    g_free(lowered_search);
    g_free(lowered_key);
    g_free(lowered_value);
    return is_match;
}

static gboolean find_first_macro_iter_by_search(GtkTreeModel* model,
                                                const gchar* search_text,
                                                GtkTreeIter* out_iter)
{
    if (model == NULL) {
        return FALSE;
    }

    GtkTreeIter iter;
    gboolean has_row = gtk_tree_model_get_iter_first(model, &iter);
    while (has_row) {
        gchar* key = NULL;
        gchar* value = NULL;
        gtk_tree_model_get(model, &iter, COL_KEY, &key, COL_VALUE, &value, -1);
        gboolean is_match = macro_row_matches_search(key, value, search_text);
        g_free(key);
        g_free(value);

        if (is_match) {
            *out_iter = iter;
            return TRUE;
        }

        has_row = gtk_tree_model_iter_next(model, &iter);
    }

    return FALSE;
}

static void apply_macro_search_selection(void)
{
    GtkEntry* search_entry = get_macro_search_entry();
    GtkTreeModel* model = gtk_tree_view_get_model(tree_macro);
    if (search_entry == NULL || model == NULL) {
        return;
    }

    const gchar* search_text = gtk_entry_get_text(search_entry);
    bool has_search_text = (search_text != NULL && search_text[0] != '\0');
    if (!has_search_text) {
        gchar* selected_key = macro_dialog_get_selected_key();
        bool has_selected_key = (selected_key != NULL && selected_key[0] != '\0');
        if (has_selected_key) {
            select_and_scroll_to_macro_key(selected_key);
        }
        g_free(selected_key);
        return;
    }

    GtkTreeIter iter;
    gboolean found = find_first_macro_iter_by_search(model, search_text, &iter);
    if (!found) {
        return;
    }

    GtkTreePath* path = gtk_tree_model_get_path(model, &iter);
    gtk_tree_view_set_cursor(tree_macro, path, NULL, FALSE);
    gtk_tree_view_scroll_to_cell(tree_macro, path, NULL, TRUE, 0.5, 0.0);
    gtk_widget_grab_focus(GTK_WIDGET(tree_macro));
    gtk_tree_path_free(path);
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
    g_object_set_data(G_OBJECT(filter_native),
                      MACRO_FILE_FILTER_EXTENSION_KEY,
                      const_cast<gchar*>(MACRO_TEXT_FILE_EXTENSION));
    gtk_file_chooser_add_filter(chooser, filter_native);

    GtkFileFilter* filter_yaml = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_yaml, _("YAML files"));
    gtk_file_filter_add_pattern(filter_yaml, "*.yml");
    gtk_file_filter_add_pattern(filter_yaml, "*.yaml");
    g_object_set_data(G_OBJECT(filter_yaml),
                      MACRO_FILE_FILTER_EXTENSION_KEY,
                      const_cast<gchar*>(MACRO_YAML_FILE_EXTENSION));
    gtk_file_chooser_add_filter(chooser, filter_yaml);

    GtkFileFilter* filter_plist = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_plist, _("macOS plist files"));
    gtk_file_filter_add_pattern(filter_plist, "*.plist");
    g_object_set_data(G_OBJECT(filter_plist),
                      MACRO_FILE_FILTER_EXTENSION_KEY,
                      const_cast<gchar*>(MACRO_PLIST_FILE_EXTENSION));
    gtk_file_chooser_add_filter(chooser, filter_plist);

    GtkFileFilter* filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all, dgettext("gtk30", "All Files"));
    gtk_file_filter_add_pattern(filter_all, "*");
    gtk_file_chooser_add_filter(chooser, filter_all);
}

static const gchar* get_selected_macro_export_extension(GtkFileChooser* chooser)
{
    GtkFileFilter* filter = gtk_file_chooser_get_filter(chooser);
    if (filter == NULL) {
        return NULL;
    }

    return static_cast<const gchar*>(
        g_object_get_data(G_OBJECT(filter), MACRO_FILE_FILTER_EXTENSION_KEY));
}

static void set_default_macro_export_filter(GtkFileChooser* chooser)
{
    gchar* saved_extension = macro_dialog_get_export_format();
    const gchar* preferred_extension =
        (saved_extension != NULL && saved_extension[0] != '\0')
            ? saved_extension
            : MACRO_TEXT_FILE_EXTENSION;

    GSList* filters = gtk_file_chooser_list_filters(chooser);
    for (GSList* item = filters; item != NULL; item = item->next) {
        GtkFileFilter* filter = GTK_FILE_FILTER(item->data);
        const gchar* extension = static_cast<const gchar*>(
            g_object_get_data(G_OBJECT(filter), MACRO_FILE_FILTER_EXTENSION_KEY));
        const gboolean is_preferred_filter = (g_strcmp0(extension, preferred_extension) == 0);
        if (is_preferred_filter) {
            gtk_file_chooser_set_filter(chooser, filter);
            break;
        }
    }
    g_slist_free(filters);
    g_free(saved_extension);
}

static void set_default_macro_export_name(GtkFileChooser* chooser)
{
    gchar* saved_extension = macro_dialog_get_export_format();
    const gchar* preferred_extension =
        (saved_extension != NULL && saved_extension[0] != '\0')
            ? saved_extension
            : MACRO_TEXT_FILE_EXTENSION;
    gchar* default_name = g_strconcat("macro", preferred_extension, NULL);
    gtk_file_chooser_set_current_name(chooser, default_name);
    g_free(default_name);
    g_free(saved_extension);
}

static gboolean confirm_macro_export_extension_conflict(const gchar* typed_filename,
                                                        const gchar* export_filename,
                                                        const gchar* selected_extension)
{
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(dlgMacro),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_WARNING,
                                               GTK_BUTTONS_NONE,
                                               "%s",
                                               _("Filename extension does not match the selected file type"));
    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                           dgettext("gtk30", "_Cancel"), GTK_RESPONSE_CANCEL,
                           _("Use Selected Type"), GTK_RESPONSE_OK,
                           NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Confirm Export Type"));
    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dialog),
        _("You entered:\n%s\n\nbut selected file type:\n%s\n\nUniKey can continue with the completed filename below, or you can cancel and change it:\n%s"),
        typed_filename != NULL ? typed_filename : "",
        selected_extension != NULL ? selected_extension : "",
        export_filename != NULL ? export_filename : "");

    gboolean should_use_selected_type = (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK);
    gtk_widget_destroy(dialog);
    return should_use_selected_type;
}

static gboolean confirm_macro_export_overwrite(const gchar* filename)
{
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(dlgMacro),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_QUESTION,
                                               GTK_BUTTONS_NONE,
                                               "%s",
                                               _("Replace existing export file?"));
    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dialog),
        "%s",
        _("The selected file type completed the filename with an extension, and that file already exists. Replace it?"));
    gtk_dialog_add_buttons(GTK_DIALOG(dialog),
                           dgettext("gtk30", "_Cancel"), GTK_RESPONSE_CANCEL,
                           dgettext("gtk30", "_Replace"), GTK_RESPONSE_OK,
                           NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Confirm Export Overwrite"));

    gchar* basename = g_path_get_basename(filename);
    gtk_message_dialog_format_secondary_text(
        GTK_MESSAGE_DIALOG(dialog),
        _("The export path was completed as:\n%s\n\nReplace the existing file?"),
        basename != NULL ? basename : filename);
    g_free(basename);

    gboolean should_replace = (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK);
    gtk_widget_destroy(dialog);
    return should_replace;
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

static void show_macro_export_success_dialog(const gchar* filename, int exported_count)
{
    GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(dlgMacro),
                                               GTK_DIALOG_MODAL,
                                               GTK_MESSAGE_INFO,
                                               GTK_BUTTONS_OK,
                                               "%s",
                                               _("Macros exported successfully."));
    gtk_window_set_title(GTK_WINDOW(dialog), _("Export complete"));
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
                                             _("Exported %d macro(s) to:\n%s"),
                                             exported_count,
                                             filename != NULL ? filename : "");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
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


void on_btn_macroedit_clicked(GtkButton* btn, gpointer user_data)
{
    macro_dialog_apply_window_geometry(GTK_WINDOW(dlgMacro));

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

    GtkEntry* search_entry = get_macro_search_entry();
    if (search_entry != NULL) {
        gchar* saved_search_text = macro_dialog_get_search_text();
        gtk_entry_set_text(search_entry, saved_search_text != NULL ? saved_search_text : "");
        g_free(saved_search_text);
    }

    // Restore sort state from previous session (default: key asc)
    // Stored as '<column>:<order>' such as 'key:asc' or 'value:desc'.
    gchar* sort_state = NULL;
    if (ibus_unikey_config_get_string(CONFIG_MACRO_LIST_SORT, &sort_state)) {
        gchar** parts = g_strsplit(sort_state, ":", 2);
        if (parts != NULL && parts[0] != NULL) {
            if (g_strcmp0(parts[0], "value") == 0) {
                current_sort_column = COL_VALUE;
            } else {
                current_sort_column = COL_KEY;
            }
            GtkSortType order = GTK_SORT_ASCENDING;
            if (parts[1] != NULL && g_strcmp0(parts[1], "desc") == 0) {
                order = GTK_SORT_DESCENDING;
            }
            if (current_sort_column == COL_KEY) {
                sort_key_order = order;
            } else {
                sort_value_order = order;
            }
            sort_list_store(current_sort_column, order);
        }
        g_strfreev(parts);
        g_free(sort_state);
    } else {
        // default to key ascending
        sort_list_store(COL_KEY, GTK_SORT_ASCENDING);
        sort_key_order = GTK_SORT_ASCENDING;
        current_sort_column = COL_KEY;
    }

    update_column_headers();
    update_macro_count_label();

    gchar* saved_selected_key = macro_dialog_get_selected_key();
    bool has_saved_selected_key = (saved_selected_key != NULL && saved_selected_key[0] != '\0');
    if (has_saved_selected_key) {
        select_and_scroll_to_macro_key(saved_selected_key);
    }
    g_free(saved_selected_key);
    apply_macro_search_selection();
    
    int ret = gtk_dialog_run(GTK_DIALOG(dlgMacro));
    macro_dialog_remember_window_geometry(GTK_WINDOW(dlgMacro));
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
    macro_dialog_remember_window_geometry(GTK_WINDOW(dlgMacro));
    gtk_widget_hide(dlgMacro);
    return true;
}

gboolean on_macro_dialog_configure_event(GtkWidget* wid, GdkEvent* event, gpointer user_data)
{
    macro_dialog_remember_window_geometry(GTK_WINDOW(wid));
    return FALSE;
}

/**
 * @brief Simple helper bound to the cancel/close button inside the macro dialog.
 *
 * @param btn Button that triggered the hide action (unused).
 * @param user_data Unused.
 */
void macro_dialog_hide(GtkButton* btn, gpointer user_data)
{
    macro_dialog_remember_window_geometry(GTK_WINDOW(dlgMacro));
    gtk_widget_hide(dlgMacro);
}

void on_macro_search_changed(GtkEditable* editable, gpointer user_data)
{
    GtkEntry* search_entry = GTK_ENTRY(editable);
    const gchar* search_text = gtk_entry_get_text(search_entry);
    macro_dialog_remember_search_text(search_text);
    apply_macro_search_selection();
    update_macro_count_label();
}

void on_macro_selection_changed(GtkTreeSelection* selection, gpointer user_data)
{
    GtkTreeIter iter;
    GtkTreeModel* model = NULL;
    gboolean has_selection = gtk_tree_selection_get_selected(selection, &model, &iter);
    if (!has_selection || model == NULL) {
        return;
    }

    gchar* key = NULL;
    gtk_tree_model_get(model, &iter, COL_KEY, &key, -1);
    bool is_real_row = (key != NULL && g_strcmp0(key, STR_NULL_ITEM) != 0);
    if (is_real_row) {
        macro_dialog_remember_selected_key(key);
    }
    g_free(key);
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
        apply_macro_search_selection();
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
    set_default_macro_export_filter(chooser);
    set_default_macro_export_name(chooser);
    macro_dialog_apply_export_folder(chooser);

    while (gtk_dialog_run(GTK_DIALOG(file)) == GTK_RESPONSE_OK)
    {
        gchar* selected_filename = gtk_file_chooser_get_filename(chooser);
        const gchar* selected_extension = get_selected_macro_export_extension(chooser);
        const gboolean has_extension_conflict =
            macro_export_filename_has_extension_conflict(selected_filename, selected_extension);
        gchar* export_filename = macro_export_filename_complete(selected_filename,
                                                                selected_extension);
        const gboolean has_completed_extension =
            (g_strcmp0(selected_filename, export_filename) != 0);
        const gboolean is_existing_completed_path =
            has_completed_extension && g_file_test(export_filename, G_FILE_TEST_EXISTS);

        if (has_extension_conflict
            && !confirm_macro_export_extension_conflict(selected_filename,
                                                        export_filename,
                                                        selected_extension)) {
            gchar* selected_dir = g_path_get_dirname(selected_filename);
            gchar* selected_base = g_path_get_basename(selected_filename);
            gtk_file_chooser_set_current_folder(chooser, selected_dir);
            gtk_file_chooser_set_current_name(chooser, selected_base);
            g_free(selected_dir);
            g_free(selected_base);
            g_free(selected_filename);
            g_free(export_filename);
            continue;
        }

        if (is_existing_completed_path && !confirm_macro_export_overwrite(export_filename)) {
            gchar* export_dir = g_path_get_dirname(export_filename);
            gchar* export_base = g_path_get_basename(export_filename);
            gtk_file_chooser_set_current_folder(chooser, export_dir);
            gtk_file_chooser_set_current_name(chooser, export_base);
            g_free(export_dir);
            g_free(export_base);
            g_free(selected_filename);
            g_free(export_filename);
            continue;
        }

        CMacroTable macro;
        macro.init();
        int exported_count = 0;

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
                exported_count++;
            }
            g_free(key);
            g_free(value);
            b = gtk_tree_model_iter_next(model, &iter);
        }

        macro_dialog_remember_export_file(export_filename);
    macro_dialog_remember_export_format(selected_extension != NULL ? selected_extension : MACRO_TEXT_FILE_EXTENSION);
        GError* error = NULL;
        if (!macro_table_write_any_format(export_filename, &macro, &error))
        {
            show_macro_file_error_dialog(_("Failed to export macros."), error);
            g_free(selected_filename);
            g_free(export_filename);
            gtk_widget_destroy(file);
            return;
        }

        show_macro_export_success_dialog(export_filename, exported_count);

        g_free(selected_filename);
        g_free(export_filename);
        break;
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

    int count = 0;
    int matched_count = 0;
    GtkTreeIter iter;
    GtkEntry* search_entry = get_macro_search_entry();
    const gchar* search_text = search_entry != NULL ? gtk_entry_get_text(search_entry) : "";
    bool has_search_text = (search_text != NULL && search_text[0] != '\0');

    gboolean has_row = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(model), &iter);
    while (has_row) {
        gchar* key = NULL;
        gchar* value = NULL;
        gtk_tree_model_get(model, &iter, COL_KEY, &key, COL_VALUE, &value, -1);
        bool is_real_row = (key != NULL && g_strcmp0(key, STR_NULL_ITEM) != 0);
        if (is_real_row) {
            count++;
            if (macro_row_matches_search(key, value, search_text)) {
                matched_count++;
            }
        }
        g_free(key);
        g_free(value);
        has_row = gtk_tree_model_iter_next(model, &iter);
    }

    gchar* text = NULL;
    if (has_search_text) {
        text = g_strdup_printf(ngettext("%d macro • %d match", "%d macros • %d matches", count),
                               count,
                               matched_count);
    } else {
        text = g_strdup_printf(ngettext("%d macro", "%d macros", count), count);
    }
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
    ibus_unikey_config_set_string(CONFIG_MACRO_LIST_SORT,
                                  (sort_key_order == GTK_SORT_ASCENDING) ? "key:asc" : "key:desc");
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
    ibus_unikey_config_set_string(CONFIG_MACRO_LIST_SORT,
                                  (sort_value_order == GTK_SORT_ASCENDING) ? "value:asc" : "value:desc");
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
