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

#ifndef __SETUP_MACRO_UTIL_H__
#define __SETUP_MACRO_UTIL_H__

/**
 * @file macro_utils.h
 * @brief Declarations for helper routines that synchronize macro data between
 *        the `CMacroTable` engine and GtkListStore used by the setup UI.
 *
 * These utilities are used by the macro editor and import/export logic to
 * guarantee case-insensitive uniqueness, perform charset conversions, and
 * maintain a trailing sentinel row.
 */

#include <gtk/gtk.h>
#include "mactab.h"

/**
 * @brief Columns used in the GtkListStore for macros.
 *
 * COL_KEY   – string key for the macro entry
 * COL_VALUE – macro body text
 */
enum {COL_KEY = 0, COL_VALUE};

/**
 * @brief Sentinel value placed in the last row to indicate an empty/new entry.
 */
#define STR_NULL_ITEM "..."

/**
 * @brief Check if a given key already exists in the list store.
 *
 * @param store Target GtkListStore.
 * @param check_key NUL-terminated key (UTF-8) to search for.
 * @return `TRUE` if a matching case-insensitive key is present, `FALSE` otherwise.
 */
gboolean list_store_check_exists(GtkListStore* store, gchar* check_key);

/**
 * @brief Ensure the list store has a trailing null-item row.
 *
 * Used by the editor UI so users can click on a blank row to add a new macro.
 * @param list GtkListStore to modify.
 */
void list_store_add_null_item(GtkListStore* list);

/**
 * @brief Append macros from a CMacroTable to an empty list store.
 *
 * No duplicate checking is performed; suitable when the destination is cleared
 * and the source table is already sorted and deduplicated.
 * @param list Destination GtkListStore.
 * @param macro Source CMacroTable.
 */
void list_store_append_fast(GtkListStore* list, CMacroTable* macro);

/**
 * @brief Append macros into a potentially non-empty list store, skipping keys
 *        that already appear.
 *
 * Duplicate detection is case-insensitive and uses an unordered_set for speed.
 * @param list Destination GtkListStore (may contain entries already).
 * @param macro Source CMacroTable.
 */
void list_store_append(GtkListStore* list, CMacroTable* macro);

/**
 * @brief Copy all entries from a CMacroTable into a GtkListStore.
 *
 * The store is cleared and repopulated; a null sentinel row is added afterward.
 * @param macro Source CMacroTable.
 * @param store Destination GtkListStore.
 */
void unikey_macro_to_store(CMacroTable* macro, GtkListStore* store);

/**
 * @brief Read all user-visible macro entries from a GtkListStore back into a
 *        CMacroTable.
 *
 * The macro table is reset before insertion; the sentinel row is skipped.
 * @param store Source GtkListStore.
 * @param macro Destination CMacroTable.
 */
void unikey_store_to_macro(GtkListStore* store, CMacroTable* macro);

#endif
