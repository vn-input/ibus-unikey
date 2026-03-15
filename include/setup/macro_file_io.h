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

#ifndef __SETUP_MACRO_FILE_IO_H__
#define __SETUP_MACRO_FILE_IO_H__

#include <glib.h>

#include "mactab.h"

/**
 * @file macro_file_io.h
 * @brief Import/export helpers for UniKey macros in multiple external formats.
 */

/**
 * @brief Load macros from a UniKey native file, Espanso-style YAML, or a macOS
 *        text substitutions plist.
 *
 * The target table is reset before new entries are added.
 *
 * @param filename Source file path.
 * @param macro Destination macro table.
 * @param error Receives a descriptive error on failure.
 * @return `true` on success, otherwise `false`.
 */
bool macro_table_load_any_format(const char* filename, CMacroTable* macro, GError** error);

/**
 * @brief Save macros to a UniKey native file, Espanso-style YAML, or a macOS
 *        text substitutions plist.
 *
 * The file format is selected from the filename extension:
 * - `.yml` / `.yaml` -> YAML
 * - `.plist` -> Apple plist
 * - anything else -> UniKey native format
 *
 * @param filename Destination file path.
 * @param macro Source macro table.
 * @param error Receives a descriptive error on failure.
 * @return `true` on success, otherwise `false`.
 */
bool macro_table_write_any_format(const char* filename, CMacroTable* macro, GError** error);

#endif