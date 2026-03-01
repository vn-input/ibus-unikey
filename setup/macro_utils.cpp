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
#include <unordered_set>
#include <string>

#include "keycons.h"
#include "macro_utils.h"

#define _(str) gettext(str)

/**
 * @file macro_utils.cpp
 * @brief Utility routines that bridge macro data between `CMacroTable` and GTK list models.
 *
 * This file contains helper functors and conversion/transfer routines used by the
 * setup UI to:
 * - Keep macro keys unique in a case-insensitive manner.
 * - Convert macro text between internal Vietnamese encoding and UTF-8 for display.
 * - Synchronize macro entries between `GtkListStore` and `CMacroTable`.
 */

/**
 * @brief Case-insensitive hash functor for `std::string` keys.
 *
 * This hash normalizes case during hash accumulation so keys that differ only by
 * letter case map to the same hash bucket.
 */
struct CaseInsensitiveHash {
    /**
     * @brief Computes a case-insensitive hash value for a string.
     * @param s Input string to hash.
     * @return Hash value derived from lowercase characters in @p s.
     */
    size_t operator()(const std::string& s) const {
        size_t h = 0;
        for (char c : s) h = h * 31 + tolower((unsigned char)c);
        return h;
    }
};

/**
 * @brief Case-insensitive equality functor for `std::string` keys.
 *
 * Used together with `CaseInsensitiveHash` inside unordered containers to ensure
 * duplicate detection ignores letter case.
 */
struct CaseInsensitiveEqual {
    /**
     * @brief Compares two strings without case sensitivity.
     * @param a First string.
     * @param b Second string.
     * @return `true` if @p a and @p b are equal ignoring case, otherwise `false`.
     */
    bool operator()(const std::string& a, const std::string& b) const {
/**
 * @brief strcasecmp.
 *
 * @param c_str Parameter.
 * @param c_str Parameter.
 * @return Return value.
 */
        return strcasecmp(a.c_str(), b.c_str()) == 0;
    }
};

/**
 * @brief Checks whether a key already exists in a GTK list store.
 *
 * @param store List model containing macro rows.
 * @param check_key Key to search for (case-insensitive).
 * @return `true` when a matching key exists, otherwise `false`.
 */
gboolean list_store_check_exists(GtkListStore* store, gchar* check_key)
{
    GtkTreeIter iter;
    gchar* key;
    auto model = GTK_TREE_MODEL(store);
    auto b = gtk_tree_model_get_iter_first(model, &iter);
    while (b)
    {
/**
 * @brief gtk_tree_model_get.
 *
 * @param model Parameter.
 * @param iter Parameter.
 * @param COL_KEY Parameter.
 * @param key Parameter.
 * @return Return value.
 */
        gtk_tree_model_get(model, &iter, COL_KEY, &key, -1);
        if (strcasecmp(key, check_key) == 0)
        {
/**
 * @brief g_free.
 *
 * @param key Parameter.
 * @return Return value.
 */
            g_free(key);
            return true;
        }
/**
 * @brief g_free.
 *
 * @param key Parameter.
 * @return Return value.
 */
        g_free(key);

        b = gtk_tree_model_iter_next(model, &iter);
    }

    return false;
}

/**
 * @brief Appends all macro rows to a list store without duplicate checking.
 *
 * This routine is intended for fresh loads where @p list is empty and the input
 * `CMacroTable` is already sorted and deduplicated.
 *
 * @param list Destination GTK list store.
 * @param macro Source macro table in internal encoding.
 */
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
/**
 * @brief gtk_list_store_append.
 *
 * @param list Parameter.
 * @param iter Parameter.
 * @return Return value.
 */
        gtk_list_store_append(list, &iter);
/**
 * @brief gtk_list_store_set.
 *
 * @param list Parameter.
 * @param iter Parameter.
 * @param COL_KEY Parameter.
 * @param key Parameter.
 * @param COL_VALUE Parameter.
 * @param value Parameter.
 * @return Return value.
 */
        gtk_list_store_set(list, &iter, COL_KEY, key, COL_VALUE, value, -1);
    }
}

/**
 * @brief Appends macro rows to a list store while skipping duplicate keys.
 *
 * Existing and incoming keys are compared case-insensitively using a hash set,
 * making duplicate checks efficient for large imports.
 *
 * @param list Destination GTK list store, possibly with existing rows.
 * @param macro Source macro table whose rows are merged into @p list.
 */
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
/**
 * @brief gtk_tree_model_get.
 *
 * @param model Parameter.
 * @param iter Parameter.
 * @param COL_KEY Parameter.
 * @param existing_key Parameter.
 * @return Return value.
 */
            gtk_tree_model_get(model, &iter, COL_KEY, &existing_key, -1);
            if (existing_key) {
                seen.insert(existing_key);
/**
 * @brief g_free.
 *
 * @param existing_key Parameter.
 * @return Return value.
 */
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
/**
 * @brief gtk_list_store_append.
 *
 * @param list Parameter.
 * @param iter Parameter.
 * @return Return value.
 */
        gtk_list_store_append(list, &iter);
/**
 * @brief gtk_list_store_set.
 *
 * @param list Parameter.
 * @param iter Parameter.
 * @param COL_KEY Parameter.
 * @param key Parameter.
 * @param COL_VALUE Parameter.
 * @param value Parameter.
 * @return Return value.
 */
        gtk_list_store_set(list, &iter, COL_KEY, key, COL_VALUE, value, -1);
    }
}

/**
 * @brief Rebuilds a GTK list store from a `CMacroTable` snapshot.
 *
 * The target store is cleared, repopulated in bulk, and finished with a trailing
 * sentinel null row used by the editor UI.
 *
 * @param macro Source macro table.
 * @param store Destination GTK list store.
 */
void unikey_macro_to_store(CMacroTable* macro, GtkListStore* store)
{
    // Temporarily increase ref count and detach from any views
    // to avoid per-row UI updates during bulk insert
/**
 * @brief g_object_ref.
 *
 * @param store Parameter.
 * @return Return value.
 */
    g_object_ref(store);
    
/**
 * @brief gtk_list_store_clear.
 *
 * @param store Parameter.
 * @return Return value.
 */
    gtk_list_store_clear(store);

    // Use fast append - no duplicate check needed since store is empty
    // and CMacroTable already sorted + deduped
/**
 * @brief list_store_append_fast.
 *
 * @param store Parameter.
 * @param macro Parameter.
 * @return Return value.
 */
    list_store_append_fast(store, macro);
/**
 * @brief list_store_add_null_item.
 *
 * @param store Parameter.
 * @return Return value.
 */
    list_store_add_null_item(store);

/**
 * @brief g_object_unref.
 *
 * @param store Parameter.
 * @return Return value.
 */
    g_object_unref(store);
}

/**
 * @brief Copies macro rows from a GTK list store into a `CMacroTable`.
 *
 * The target macro table content is reset before rows are imported. Sentinel null
 * rows are ignored.
 *
 * @param store Source GTK list store.
 * @param macro Destination macro table.
 */
void unikey_store_to_macro(GtkListStore* store, CMacroTable* macro)
{
    auto model = GTK_TREE_MODEL(store);

    GtkTreeIter iter;
    auto b = gtk_tree_model_get_iter_first(model, &iter);
    macro->resetContent();
    while (b == TRUE)
    {
        gchar *key, *value;
/**
 * @brief gtk_tree_model_get.
 *
 * @param model Parameter.
 * @param iter Parameter.
 * @param COL_KEY Parameter.
 * @param key Parameter.
 * @param COL_VALUE Parameter.
 * @param value Parameter.
 * @return Return value.
 */
        gtk_tree_model_get(model, &iter, COL_KEY, &key, COL_VALUE, &value, -1);

        if (strcasecmp(key, STR_NULL_ITEM) != 0)
        {
            macro->addItem(key, value, CONV_CHARSET_XUTF8);
        }
/**
 * @brief g_free.
 *
 * @param key Parameter.
 * @return Return value.
 */
        g_free(key);
/**
 * @brief g_free.
 *
 * @param value Parameter.
 * @return Return value.
 */
        g_free(value);

        b = gtk_tree_model_iter_next(model, &iter);
    }
}

/**
 * @brief Ensures a trailing sentinel null row exists in the list store.
 *
 * If the last row already contains `STR_NULL_ITEM`, no change is made.
 * Otherwise a new sentinel row is appended.
 *
 * @param list GTK list store to validate/update.
 */
void list_store_add_null_item(GtkListStore* list)
{
    GtkTreeIter iter;

    auto n = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list), NULL);
    if (n > 0)
    {
/**
 * @brief gtk_tree_model_iter_nth_child.
 *
 * @param list Parameter.
 * @param iter Parameter.
 * @param NULL Parameter.
 * @param n Parameter.
 * @return Return value.
 */
        gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(list), &iter, NULL, n-1);
        gchar *key;
/**
 * @brief gtk_tree_model_get.
 *
 * @param list Parameter.
 * @param iter Parameter.
 * @param COL_KEY Parameter.
 * @param key Parameter.
 * @return Return value.
 */
        gtk_tree_model_get(GTK_TREE_MODEL(list), &iter, COL_KEY, &key, -1);
        if (strcmp(key, STR_NULL_ITEM) == 0)
        {
/**
 * @brief g_free.
 *
 * @param key Parameter.
 * @return Return value.
 */
            g_free(key);
            return;
        }
/**
 * @brief g_free.
 *
 * @param key Parameter.
 * @return Return value.
 */
        g_free(key);
    }

    // if last item is valid item or no item in list, add new NULL item
/**
 * @brief gtk_list_store_append.
 *
 * @param list Parameter.
 * @param iter Parameter.
 * @return Return value.
 */
    gtk_list_store_append(list, &iter);
/**
 * @brief gtk_list_store_set.
 *
 * @param list Parameter.
 * @param iter Parameter.
 * @param COL_KEY Parameter.
 * @param STR_NULL_ITEM Parameter.
 * @param COL_VALUE Parameter.
 * @param STR_NULL_ITEM Parameter.
 * @return Return value.
 */
    gtk_list_store_set(list, &iter,
                       COL_KEY, STR_NULL_ITEM,
                       COL_VALUE, STR_NULL_ITEM,
                       -1);
}
