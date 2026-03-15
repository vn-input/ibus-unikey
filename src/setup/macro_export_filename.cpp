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
#include <glib.h>

#include "macro_export_filename.h"

namespace {

static const gchar* find_filename_extension(const gchar* filename)
{
    if (filename == NULL || filename[0] == '\0') {
        return NULL;
    }

    const gchar* last_separator = strrchr(filename, G_DIR_SEPARATOR);
    const gchar* basename = (last_separator != NULL) ? last_separator + 1 : filename;
    const gchar* last_dot = strrchr(basename, '.');

    const gboolean is_missing_dot = (last_dot == NULL);
    const gboolean is_hidden_dotfile = (last_dot == basename);
    const gboolean is_trailing_dot = (last_dot != NULL && last_dot[1] == '\0');

    if (is_missing_dot || is_hidden_dotfile || is_trailing_dot) {
        return NULL;
    }

    return last_dot;
}

} // namespace

gboolean macro_export_filename_has_extension_conflict(const gchar* filename,
                                                      const gchar* extension)
{
    const gboolean has_extension = (extension != NULL && extension[0] != '\0');
    const gchar* filename_extension = find_filename_extension(filename);

    if (!has_extension || filename_extension == NULL) {
        return FALSE;
    }

    return g_ascii_strcasecmp(filename_extension, extension) != 0;
}

gchar* macro_export_filename_complete(const gchar* filename,
                                      const gchar* extension)
{
    const gboolean has_filename = (filename != NULL && filename[0] != '\0');
    const gboolean has_extension = (extension != NULL && extension[0] != '\0');
    const gchar* filename_extension = find_filename_extension(filename);

    if (!has_filename) {
        return g_strdup("");
    }

    if (!has_extension) {
        return g_strdup(filename);
    }

    const gboolean has_existing_extension = (filename_extension != NULL);
    const gboolean has_matching_extension = has_existing_extension
        && g_ascii_strcasecmp(filename_extension, extension) == 0;

    if (has_matching_extension) {
        return g_strdup(filename);
    }

    return g_strconcat(filename, extension, NULL);
}