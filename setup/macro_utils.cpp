#include <cstring>
#include <libintl.h>
#include <unordered_set>
#include <string>

#include "keycons.h"
#include "macro_utils.h"

#define _(str) gettext(str)

// Case-insensitive hash for std::string
struct CaseInsensitiveHash {
    size_t operator()(const std::string& s) const {
        size_t h = 0;
        for (char c : s) h = h * 31 + tolower((unsigned char)c);
        return h;
    }
};

struct CaseInsensitiveEqual {
    bool operator()(const std::string& a, const std::string& b) const {
        return strcasecmp(a.c_str(), b.c_str()) == 0;
    }
};

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

// Fast append without duplicate checking - use when store is empty (fresh load)
// CMacroTable already handles sorting and deduplication
void list_store_append_fast(GtkListStore* list, CMacroTable* macro)
{
    gchar key[MAX_MACRO_KEY_LEN*3];
    gchar value[MAX_MACRO_TEXT_LEN*3];
    UKBYTE* p;
    GtkTreeIter iter;

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
        gtk_list_store_append(list, &iter);
        gtk_list_store_set(list, &iter, COL_KEY, key, COL_VALUE, value, -1);
    }
}

// Append with duplicate checking - use when merging into existing entries (import)
void list_store_append(GtkListStore* list, CMacroTable* macro)
{
    gchar key[MAX_MACRO_KEY_LEN*3];
    gchar value[MAX_MACRO_TEXT_LEN*3];
    UKBYTE* p;

    // Build hash set of existing keys for O(1) duplicate check
    std::unordered_set<std::string, CaseInsensitiveHash, CaseInsensitiveEqual> seen;
    
    GtkTreeIter iter;
    auto model = GTK_TREE_MODEL(list);
    if (gtk_tree_model_get_iter_first(model, &iter)) {
        do {
            gchar* existing_key;
            gtk_tree_model_get(model, &iter, COL_KEY, &existing_key, -1);
            if (existing_key) {
                seen.insert(existing_key);
                g_free(existing_key);
            }
        } while (gtk_tree_model_iter_next(model, &iter));
    }

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

        // O(1) duplicate check using hash set
        if (seen.count(key))
            continue;
        seen.insert(key);

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
        gtk_list_store_append(list, &iter);
        gtk_list_store_set(list, &iter, COL_KEY, key, COL_VALUE, value, -1);
    }
}

void unikey_macro_to_store(CMacroTable* macro, GtkListStore* store)
{
    // Temporarily increase ref count and detach from any views
    // to avoid per-row UI updates during bulk insert
    g_object_ref(store);
    
    gtk_list_store_clear(store);

    // Use fast append - no duplicate check needed since store is empty
    // and CMacroTable already sorted + deduped
    list_store_append_fast(store, macro);
    list_store_add_null_item(store);

    g_object_unref(store);
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
