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

#ifndef __SETUP_MACRO_EXPORT_FILENAME_H__
#define __SETUP_MACRO_EXPORT_FILENAME_H__

#include <glib.h>

/**
 * @file macro_export_filename.h
 * @brief Helpers for completing export filenames from the selected file type.
 */

/**
 * @brief Check whether the typed filename extension conflicts with the chosen export type.
 *
 * Hidden dotfiles such as `.env` are treated as extensionless names, not as
 * conflicting extensions.
 *
 * @param filename User-provided export filename.
 * @param extension Preferred extension including the leading dot.
 * @return `TRUE` when the filename already has a different extension.
 */
gboolean macro_export_filename_has_extension_conflict(const gchar* filename,
                                                      const gchar* extension);

/**
 * @brief Complete the export filename using the chosen file type.
 *
 * If the filename has no extension, the chosen one is appended. If the
 * filename already has a different extension, the chosen extension is appended
 * on top so no existing suffix is discarded.
 *
 * @param filename User-provided export filename.
 * @param extension Preferred extension including the leading dot.
 * @return Newly allocated final filename.
 */
gchar* macro_export_filename_complete(const gchar* filename,
                                      const gchar* extension);

#endif