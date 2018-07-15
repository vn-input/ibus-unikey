#ifndef __SETUP_MACRO_UTIL_H__
#define __SETUP_MACRO_UTIL_H__

#include <gtk/gtk.h>
#include "mactab.h"

enum {COL_KEY = 0, COL_VALUE};
#define STR_NULL_ITEM "..."

gboolean list_store_check_exists(GtkListStore* store, gchar* check_key);
void list_store_add_null_item(GtkListStore* list);
void list_store_append(GtkListStore* list, CMacroTable* macro);
void unikey_macro_to_store(CMacroTable* macro, GtkListStore* store);
void unikey_store_to_macro(GtkListStore* store, CMacroTable* macro);

#endif
