#include <gtk/gtk.h>
#include <string.h>

#include <libintl.h>
#include "keycons.h"
#include "mactab.h"
#include "dlg_macro_table.h"

#define _(str) gettext(str)


#define STR_NULL_ITEM "..."
#define MACRO_DEFAULT_VALUE _("(replace text)")

enum {COL_KEY = 0, COL_VALUE, NUM_COLS};

void key_edited_cb (GtkCellRendererText *celltext,
                    const gchar *string_path,
                    const gchar *newkey,
                    gpointer user_data);
void value_edited_cb(GtkCellRendererText *celltext,
                     const gchar *string_path,
                     const gchar *newvalue,
                     gpointer data);
void remove_macro_clicked_cb(GtkButton *button, gpointer user_data);
void removeall_macro_clicked_cb(GtkButton *button, gpointer user_data);
void check_last_macro_in_list(GtkListStore* list);


GtkWidget* unikey_macro_dialog_new()
{
    GtkBuilder* builder = gtk_builder_new();

    gtk_builder_add_from_file(builder, GLADE_DATA_DIR "/setup-macro.glade", NULL);

    GtkWidget* dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dlg_macro_table"));

    // init macro list
    GtkListStore* list = GTK_LIST_STORE(gtk_builder_get_object(builder, "list_macro"));
    check_last_macro_in_list(list);

    GtkTreeView* tree = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tree_macro"));
    g_object_set_data(G_OBJECT(dialog), "tree_macro", tree);

    GtkTreeViewColumn* col;
    GtkCellRenderer* render;

// create key column
    render = gtk_cell_renderer_text_new();
    g_object_set(render,
                 "editable", TRUE,
                 "width-chars", MAX_MACRO_KEY_LEN+8,
                 NULL);
    g_signal_connect(render, "edited", G_CALLBACK(key_edited_cb), tree);
    col = gtk_tree_view_column_new_with_attributes(_("Word"),
                                                   render,
                                                   "text",
                                                   COL_KEY,
                                                   NULL);
    gtk_tree_view_column_set_resizable(col, TRUE);
    gtk_tree_view_append_column(tree, col);

// create value column
    render = gtk_cell_renderer_text_new();
    g_object_set(render,
                 "editable", TRUE,
                 NULL);
    g_signal_connect(render, "edited", G_CALLBACK(value_edited_cb), tree);
    col = gtk_tree_view_column_new_with_attributes(_("Replace with"),
                                                   render,
                                                   "text",
                                                   COL_VALUE,
                                                   NULL);
    gtk_tree_view_column_set_resizable(col, TRUE);
    gtk_tree_view_append_column(tree, col);

    GtkWidget* btn;

    // connect signal
    btn = GTK_WIDGET(gtk_builder_get_object(builder, "btn_remove"));
    g_signal_connect(btn, "clicked", G_CALLBACK(remove_macro_clicked_cb), tree);

    // connect signal
    btn = GTK_WIDGET(gtk_builder_get_object(builder, "btn_removeall"));
    g_signal_connect(btn, "clicked", G_CALLBACK(removeall_macro_clicked_cb), tree);

    g_object_unref(builder);

    return dialog;
}

void unikey_macro_dialog_load_macro(GtkDialog* dialog, CMacroTable macro)
{
    GtkTreeView* tree;
    GtkListStore* list;
    GtkTreeIter iter;
    gchar key[MAX_MACRO_KEY_LEN*3];
    gchar value[MAX_MACRO_TEXT_LEN*3];
    UKBYTE* p;
    int inLen, maxOutLen;
    int i, ret;

    tree = GTK_TREE_VIEW(g_object_get_data(G_OBJECT(dialog), "tree_macro"));
    list = GTK_LIST_STORE(gtk_tree_view_get_model(tree));
    
    gtk_list_store_clear(list);

    for (i = 0 ; i < macro.getCount(); i++)
    {
        // get key and convert to XUTF charset
        p = (UKBYTE*)macro.getKey(i);
        inLen = -1;
        maxOutLen = sizeof(key);
        ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_XUTF8,
                        p, (UKBYTE*)key,
                        &inLen, &maxOutLen);
        if (ret != 0)
            continue;

        // get value and convert to XUTF charset
        p = (UKBYTE*)macro.getText(i);
        inLen = -1;
        maxOutLen = sizeof(value);
        ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_XUTF8,
                        p, (UKBYTE*)value,
                        &inLen, &maxOutLen);
        if (ret != 0)
            continue;

        // append to liststore
        gtk_list_store_append(list, &iter);
        gtk_list_store_set(list, &iter, COL_KEY, key, COL_VALUE, value, -1);
    }

    check_last_macro_in_list(list);

    // select first iter
    GtkTreeSelection* select = gtk_tree_view_get_selection(tree);
    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list), &iter);
    gtk_tree_selection_select_iter(select, &iter);
}

void unikey_macro_dialog_save_macro(GtkDialog* dialog, CMacroTable* macro)
{
    GtkTreeView* tree;
    GtkTreeModel* model;
    GtkTreeIter iter;
    gboolean b;
    gchar *key, *value;

    macro->resetContent();

    tree = GTK_TREE_VIEW(g_object_get_data(G_OBJECT(dialog), "tree_macro"));
    model = GTK_TREE_MODEL(gtk_tree_view_get_model(tree));

    b = gtk_tree_model_get_iter_first(model, &iter);
    while (b == TRUE)
    {
        gtk_tree_model_get(model, &iter, COL_KEY, &key, COL_VALUE, &value, -1);

        if (strcasecmp(key, STR_NULL_ITEM) != 0)
        {
            macro->addItem(key, value, CONV_CHARSET_XUTF8);
        }

        b = gtk_tree_model_iter_next(model, &iter);
    }
}

void key_edited_cb (GtkCellRendererText *celltext,
                    const gchar *string_path,
                    const gchar *newkey,
                    gpointer data)
{
    GtkTreeView *tree;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *key, *oldkey, *oldvalue;
    gchar nkey[MAX_MACRO_KEY_LEN];
    gboolean b;

    tree = GTK_TREE_VIEW(data);
    model = gtk_tree_view_get_model(tree);

    strncpy(nkey, newkey, MAX_MACRO_KEY_LEN-1);
    nkey[MAX_MACRO_KEY_LEN-1] = '\0';

    if (strcmp(nkey, STR_NULL_ITEM) == 0
        || (strlen(STR_NULL_ITEM) != 0 && strlen(nkey) == 0))
    {
        return;
    }

    // check if any key same as newkey
    b = gtk_tree_model_get_iter_first(model, &iter);
    while (b)
    {
        gtk_tree_model_get(model, &iter, COL_KEY, &key, -1);
        if (strcasecmp(key, nkey) == 0)
        {
            return;
        }

        b = gtk_tree_model_iter_next(model, &iter);
    }
    // end check

    // get iter of newkey
    gtk_tree_model_get_iter_from_string(model, &iter, string_path);
    // get old value of that iter
    gtk_tree_model_get(model, &iter, COL_KEY, &oldkey, COL_VALUE, &oldvalue, -1);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_KEY, nkey, -1);

    if (strcmp(oldkey, STR_NULL_ITEM) == 0)
    {
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_VALUE, MACRO_DEFAULT_VALUE);
    }

    check_last_macro_in_list(GTK_LIST_STORE(model));
}

void value_edited_cb(GtkCellRendererText *celltext,
                     const gchar *string_path,
                     const gchar *newvalue,
                     gpointer data)
{
    GtkTreeView *tree;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *key;
    gchar value[MAX_MACRO_TEXT_LEN];

    tree = GTK_TREE_VIEW(data);
    model = gtk_tree_view_get_model(tree);

    gtk_tree_model_get_iter_from_string(model, &iter, string_path);

    gtk_tree_model_get(model, &iter, COL_KEY, &key, -1);

    strncpy(value, newvalue, MAX_MACRO_TEXT_LEN-1);
    value[MAX_MACRO_TEXT_LEN-1] = '\0';

    if (strcmp(key, STR_NULL_ITEM) != 0)
    {
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, COL_VALUE, value, -1);
    }
}

void remove_macro_clicked_cb(GtkButton *button, gpointer user_data)
{
    GtkTreeView     *treeview;
    GtkListStore    *list;
    GtkTreeSelection*select;
    GtkTreeIter     iter;
    gchar           *key;

    treeview = GTK_TREE_VIEW(user_data);

    select = gtk_tree_view_get_selection(treeview);

    list = GTK_LIST_STORE(gtk_tree_view_get_model(treeview));

    if (gtk_tree_selection_get_selected(select, NULL, &iter) == TRUE)
    {
        gtk_tree_model_get(GTK_TREE_MODEL(list), &iter, COL_KEY, &key, -1);

        if (strcmp(key, STR_NULL_ITEM) != 0)
        {
            gtk_list_store_remove(list, &iter);
        }

        gtk_tree_selection_select_iter(select, &iter); // select current index
    }
}

void removeall_macro_clicked_cb(GtkButton *button, gpointer data)
{
    GtkTreeView* tree;
    GtkListStore* list;
    GtkTreeIter iter;
    GtkTreeSelection* select;

    tree = GTK_TREE_VIEW(data);
    list = GTK_LIST_STORE(gtk_tree_view_get_model(tree));

    gtk_list_store_clear(list);

    check_last_macro_in_list(list);

    select = gtk_tree_view_get_selection(tree);

    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list), &iter);

    gtk_tree_selection_select_iter(select, &iter);
}

void check_last_macro_in_list(GtkListStore* list)
{
    GtkTreeIter iter;
    gchar *key;
    gint n;

    // get number item in list
    n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list), NULL);

    if (n > 0)
    {
        // get last item
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(list), &iter, NULL, n-1);

        // get key of item
        gtk_tree_model_get(GTK_TREE_MODEL(list), &iter, COL_KEY, &key, -1);
    }

    // if key is value used for NULL item
    if (strcmp(key, STR_NULL_ITEM) == 0)
    {
        return;
    }

    // if last item is valid item or no item in list, add new NULL item
    gtk_list_store_append(list, &iter);
    gtk_list_store_set(list, &iter,
                       COL_KEY, STR_NULL_ITEM,
                       COL_VALUE, STR_NULL_ITEM,
                       -1);
}
