#include <cstring>
#include <libintl.h>

#include "keycons.h"
#include "macro_utils.h"

#define _(str) gettext(str)

gboolean list_store_check_exists(GtkListStore* store, gchar* check_key)
{
    GtkTreeIter iter;
    gchar* key;
    auto model = GTK_TREE_MODEL(store);
    auto b = gtk_tree_model_get_iter_first(model, &iter);
    while (b)
    {
        gtk_tree_model_get(model, &iter, COL_KEY, &key, -1);
        if (strcasecmp(key, check_key) == 0)
        {
            g_free(key);
            return true;
        }
        g_free(key);

        b = gtk_tree_model_iter_next(model, &iter);
    }

    return false;
}

void list_store_append(GtkListStore* list, CMacroTable* macro)
{
    gchar key[MAX_MACRO_KEY_LEN*3];
    gchar value[MAX_MACRO_TEXT_LEN*3];
    UKBYTE* p;

    for (int i = 0 ; i < macro->getCount(); i++)
    {
        // get key and convert to XUTF charset
        p = (UKBYTE*)macro->getKey(i);
        int inLen = -1;
        int maxOutLen = sizeof(key);
        int ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_XUTF8,
                        p, (UKBYTE*)key,
                        &inLen, &maxOutLen);
        if (ret != 0)
            continue;

        if (list_store_check_exists(list, key))
            continue;

        // get value and convert to XUTF charset
        p = (UKBYTE*)macro->getText(i);
        inLen = -1;
        maxOutLen = sizeof(value);
        ret = VnConvert(CONV_CHARSET_VNSTANDARD, CONV_CHARSET_XUTF8,
                        p, (UKBYTE*)value,
                        &inLen, &maxOutLen);
        if (ret != 0)
            continue;

        // append to liststore
        GtkTreeIter iter;
        gtk_list_store_append(list, &iter);
        gtk_list_store_set(list, &iter, COL_KEY, key, COL_VALUE, value, -1);
    }
}

void unikey_macro_to_store(CMacroTable* macro, GtkListStore* store)
{
    gtk_list_store_clear(store);

    list_store_append(store, macro);
    list_store_add_null_item(store);

    // select first iter
    //GtkTreeSelection* select = gtk_tree_view_get_selection(tree_macro);
    //gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list), &iter);
    //gtk_tree_selection_select_iter(select, &iter);
}

void unikey_store_to_macro(GtkListStore* store, CMacroTable* macro)
{
    auto model = GTK_TREE_MODEL(store);

    GtkTreeIter iter;
    auto b = gtk_tree_model_get_iter_first(model, &iter);
    macro->resetContent();
    while (b == TRUE)
    {
        gchar *key, *value;
        gtk_tree_model_get(model, &iter, COL_KEY, &key, COL_VALUE, &value, -1);

        if (strcasecmp(key, STR_NULL_ITEM) != 0)
        {
            macro->addItem(key, value, CONV_CHARSET_XUTF8);
        }
        g_free(key);
        g_free(value);

        b = gtk_tree_model_iter_next(model, &iter);
    }
}

void list_store_add_null_item(GtkListStore* list)
{
    GtkTreeIter iter;

    auto n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list), NULL);
    if (n > 0)
    {
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(list), &iter, NULL, n-1);
        gchar *key;
        gtk_tree_model_get(GTK_TREE_MODEL(list), &iter, COL_KEY, &key, -1);
        if (strcmp(key, STR_NULL_ITEM) == 0)
        {
            g_free(key);
            return;
        }
        g_free(key);
    }

    // if last item is valid item or no item in list, add new NULL item
    gtk_list_store_append(list, &iter);
    gtk_list_store_set(list, &iter,
                       COL_KEY, STR_NULL_ITEM,
                       COL_VALUE, STR_NULL_ITEM,
                       -1);
}
