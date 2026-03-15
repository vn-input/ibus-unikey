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
#include <gdk/gdkkeysyms.h>
#include <glib/gstdio.h>

#include "mactab.h"
#include "macro_export_filename.h"
#include "macro_dialog_state.h"
#include "macro_file_io.h"
#include "unikey_config.h"

#include "macro_utils.h"
#include "signal_handlers.h"

#define _(str) gettext(str)


void on_setting_toggled(GtkToggleButton* btn, gpointer user_data)
{
    const gchar* key = gtk_widget_get_name(GTK_WIDGET(btn));
    key = key + 4; // skip "cfg_"

    gboolean b = gtk_toggle_button_get_active(btn);
    ibus_unikey_config_set_boolean(key, b);
}

/**
 * @brief Initialize a toggle button's state from configuration when realized.
 *
 * Like `on_setting_toggled`, this assumes the widget name starts with
 * "cfg_".  If the key exists in config its value is used to set the button's
 * active state.
 *
 * @param btn        The toggle button being initialized.
 * @param user_data  Unused pointer provided by GTK.
 */
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

/**
 * @brief Display the macro editor dialog and handle load/save operations.
 *
 * This callback is invoked when the "Edit" button is clicked.  It reads the
 * current macro file into a `CMacroTable`, populates the tree view, allows the
 * user to modify entries, and writes the table back if the dialog is closed with
 * the OK response.  The tree's model is temporarily detached during bulk
 * updates for performance.
 *
 * @param btn Button widget that opened the editor (unused).
 * @param user_data Unused.
 */
