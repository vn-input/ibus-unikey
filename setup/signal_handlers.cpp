#include <cstring>
#include <libintl.h>
#include <gdk/gdkkeysyms.h>

#include "mactab.h"
#include "unikey_config.h"

#include "macro_utils.h"
#include "signal_handlers.h"

#define _(str) gettext(str)

extern GtkWidget* dlgMacro;
extern GtkTreeView* tree_macro;
extern GtkBuilder* builder;

// Forward declarations
static void update_macro_count_label(void);
static void sort_list_store(int column, GtkSortType order);
static void update_column_headers(void);

// Sort state for columns
static GtkSortType sort_key_order = GTK_SORT_ASCENDING;
static GtkSortType sort_value_order = GTK_SORT_ASCENDING;
static int current_sort_column = COL_KEY;

#define MACRO_DEFAULT_VALUE _("(replace text)")

gboolean on_main_window_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    if (event->keyval == GDK_KEY_Escape) {
        gtk_main_quit();
        return true;
    }
    return false;
}

void on_main_window_destroy(GtkWidget* btn, gpointer user_data)
{
    gtk_main_quit();
}

void on_btn_close_clicked(GtkButton* btn, gpointer user_data)
{
    gtk_main_quit();
}

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

void on_input_method_changed(GtkComboBox* cbb, gpointer user_data)
{
    cbb_config_update(cbb, CONFIG_INPUTMETHOD);
}

void on_output_charset_changed(GtkComboBox* cbb, gpointer user_data)
{
    cbb_config_update(cbb, CONFIG_OUTPUTCHARSET);
}

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

void on_input_method_realize(GtkComboBox* cbb, gpointer user_data)
{
    cbb_config_set_active(cbb, CONFIG_INPUTMETHOD);
}

void on_output_charset_realize(GtkComboBox* cbb, gpointer user_data)
{
    cbb_config_set_active(cbb, CONFIG_OUTPUTCHARSET);
}

void on_setting_toggled(GtkToggleButton* btn, gpointer user_data)
{
    const gchar* key = gtk_widget_get_name(GTK_WIDGET(btn));
    key = key + 4; // skip "cfg_"

    gboolean b = gtk_toggle_button_get_active(btn);
    ibus_unikey_config_set_boolean(key, b);
}

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

gboolean on_macro_dialog_delete(GtkWidget* wid, GdkEvent* ev, gpointer user_data)
{
    gtk_widget_hide(dlgMacro);
    return true;
}

void macro_dialog_hide(GtkButton* btn, gpointer user_data)
{
    gtk_widget_hide(dlgMacro);
}

void on_cell_key_edited(GtkCellRendererText *celltext,
                    const gchar *string_path,
                    const gchar *newkey,
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

void on_cell_value_edited(GtkCellRendererText *celltext,
                     const gchar *string_path,
                     const gchar *newvalue,
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

void on_btn_macro_del_clicked(GtkButton *button, gpointer user_data)
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

void on_btn_macro_clear_clicked(GtkButton *button, gpointer data)
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

void on_btn_macro_import_clicked(GtkButton *button, gpointer data)
{
    auto file = gtk_file_chooser_dialog_new(_("Import macro"),
                                       GTK_WINDOW(dlgMacro),
                                       GTK_FILE_CHOOSER_ACTION_OPEN,
                                       dgettext("gtk30", "_Cancel"), GTK_RESPONSE_CANCEL,
                                       dgettext("gtk30", "_Open"), GTK_RESPONSE_OK,
                                       NULL);

    if (gtk_dialog_run(GTK_DIALOG(file)) == GTK_RESPONSE_OK)
    {

        auto fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file));
        CMacroTable macro;
        macro.init();
        macro.loadFromFile(fn);
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

void on_btn_macro_export_clicked(GtkButton *button, gpointer data)
{
    auto file = gtk_file_chooser_dialog_new(_("Export macro"),
                                       GTK_WINDOW(dlgMacro),
                                       GTK_FILE_CHOOSER_ACTION_SAVE,
                                       dgettext("gtk30", "_Cancel"), GTK_RESPONSE_CANCEL,
                                       dgettext("gtk30", "_Save"), GTK_RESPONSE_OK,
                                       NULL);

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

        auto fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(file));
        macro.writeToFile(fn);
        g_free(fn);
    }

    gtk_widget_destroy(file);
}

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

static int compare_store_items(GtkTreeModel *model,
                               GtkTreeIter *a,
                               GtkTreeIter *b,
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

void on_column_key_clicked(GtkTreeViewColumn *column, gpointer user_data)
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

void on_column_value_clicked(GtkTreeViewColumn *column, gpointer user_data)
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

void on_btn_macro_add_clicked(GtkButton *button, gpointer user_data)
{
    GtkDialog* add_dialog = GTK_DIALOG(gtk_builder_get_object(builder, "add_macro_dialog"));
    GtkEntry* entry_key = GTK_ENTRY(gtk_builder_get_object(builder, "entry_macro_key"));
    GtkEntry* entry_value = GTK_ENTRY(gtk_builder_get_object(builder, "entry_macro_value"));

    if (add_dialog == NULL || entry_key == NULL || entry_value == NULL) {
        g_warning("Add macro dialog widgets not found");
        return;
    }

    // Clear entries and set parent
    gtk_entry_set_text(entry_key, "");
    gtk_entry_set_text(entry_value, "");
    gtk_window_set_transient_for(GTK_WINDOW(add_dialog), GTK_WINDOW(dlgMacro));
    gtk_widget_show_all(GTK_WIDGET(add_dialog));

    int response = gtk_dialog_run(add_dialog);
    gtk_widget_hide(GTK_WIDGET(add_dialog));

    if (response == GTK_RESPONSE_OK) {
        const gchar* new_key = gtk_entry_get_text(entry_key);
        const gchar* new_value = gtk_entry_get_text(entry_value);

        // Validate: both fields must have content
        if (strlen(new_key) == 0) {
            return;
        }

        // Truncate to max lengths
        gchar key_buf[MAX_MACRO_KEY_LEN];
        gchar value_buf[MAX_MACRO_TEXT_LEN];
        strncpy(key_buf, new_key, MAX_MACRO_KEY_LEN - 1);
        key_buf[MAX_MACRO_KEY_LEN - 1] = '\0';
        strncpy(value_buf, new_value, MAX_MACRO_TEXT_LEN - 1);
        value_buf[MAX_MACRO_TEXT_LEN - 1] = '\0';

        auto store = GTK_LIST_STORE(gtk_tree_view_get_model(tree_macro));

        // Check for duplicate
        if (list_store_check_exists(store, key_buf)) {
            GtkWidget* msg = gtk_message_dialog_new(GTK_WINDOW(dlgMacro),
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                _("Macro '%s' already exists."), key_buf);
            gtk_dialog_run(GTK_DIALOG(msg));
            gtk_widget_destroy(msg);
            return;
        }

        // Remove the null placeholder row
        GtkTreeIter iter;
        int n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL);
        if (n > 0) {
            gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store), &iter, NULL, n - 1);
            gchar* last_key;
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, COL_KEY, &last_key, -1);
            if (g_strcmp0(last_key, STR_NULL_ITEM) == 0) {
                gtk_list_store_remove(store, &iter);
            }
            g_free(last_key);
        }

        // Add new item
        GtkTreeIter new_iter;
        gtk_list_store_append(store, &new_iter);
        gtk_list_store_set(store, &new_iter, COL_KEY, key_buf, COL_VALUE, value_buf, -1);

        // Add null item back
        list_store_add_null_item(store);

        // Re-sort to put the item in the right place
        sort_list_store(current_sort_column,
            current_sort_column == COL_KEY ? sort_key_order : sort_value_order);

        // Process pending events to ensure sort is reflected in UI
        while (gtk_events_pending())
            gtk_main_iteration();

        // Find the new item and scroll to it
        GtkTreeIter find_iter;
        gboolean found = FALSE;
        if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &find_iter)) {
            do {
                gchar* iter_key;
                gtk_tree_model_get(GTK_TREE_MODEL(store), &find_iter, COL_KEY, &iter_key, -1);
                if (g_strcmp0(iter_key, key_buf) == 0) {
                    found = TRUE;
                    g_free(iter_key);
                    break;
                }
                g_free(iter_key);
            } while (gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &find_iter));
        }

        if (found) {
            GtkTreePath* path = gtk_tree_model_get_path(GTK_TREE_MODEL(store), &find_iter);

            // Use set_cursor which selects, scrolls, and focuses in one operation
            gtk_tree_view_set_cursor(tree_macro, path, NULL, FALSE);

            // Scroll to center the new item so user can see surrounding entries
            gtk_tree_view_scroll_to_cell(tree_macro, path, NULL, TRUE, 0.5, 0.0);

            // Grab focus on tree view to make selection highlight visible
            gtk_widget_grab_focus(GTK_WIDGET(tree_macro));

            gtk_tree_path_free(path);
        }

        update_macro_count_label();
    }
}
