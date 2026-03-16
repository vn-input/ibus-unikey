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

/**
 * @file add_macro_dialog_handlers.cpp
 * @brief GTK callbacks owned by the add/edit macro dialog.
 */

#include "signal_handlers.h"

extern "C" {

G_MODULE_EXPORT gboolean on_add_macro_dialog_delete(GtkWidget* widget,
                                                    GdkEvent* event,
                                                    gpointer user_data)
{
    (void)event;
    (void)user_data;
    gtk_dialog_response(GTK_DIALOG(widget), GTK_RESPONSE_CANCEL);
    return TRUE;
}

G_MODULE_EXPORT void on_add_macro_dialog_cancel_clicked(GtkButton* button,
                                                        gpointer user_data)
{
    GtkWidget* dialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
    (void)user_data;
    if (GTK_IS_DIALOG(dialog)) {
        gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
    }
}

G_MODULE_EXPORT void on_add_macro_dialog_ok_clicked(GtkButton* button,
                                                    gpointer user_data)
{
    GtkWidget* dialog = gtk_widget_get_toplevel(GTK_WIDGET(button));
    (void)user_data;
    if (GTK_IS_DIALOG(dialog)) {
        gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    }
}

} // extern "C"
