#ifndef __SETUP_GUI_BUILDER_H__
#define __SETUP_GUI_BUILDER_H__

#include <gtk/gtk.h>

/**
 * @file gui_builder.h
 * @brief GTK builder bootstrap for the setup application UI.
 *
 * This module centralizes loading of the setup `.ui` files so the entry point
 * does not need to know individual dialog paths or widget bootstrap details.
 */

gboolean init_gtk_builder(void);

#endif // __SETUP_GUI_BUILDER_H__
