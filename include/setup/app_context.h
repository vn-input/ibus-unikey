// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2000-2005 Pham Kim Long
 * Copyright (C) 2026 Hoang Duy Tran & AI Agent
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef __SETUP_APP_CONTEXT_H
#define __SETUP_APP_CONTEXT_H

/**
 * @file app_context.h
 * @brief Centralized context for setup UI dependencies.
 *
 * This module provides a narrowly-scoped container for the previously global
 * setup state (widgets, builder, etc.). Extracted as part of the legacy
 * cleanup effort (Phase 5) to reduce scattered global access.
 *
 * Migration path:
 * 1. Existing globals remain accessible via app_context_get_*() functions.
 * 2. New code should prefer explicit context passing where practical.
 * 3. Full global elimination can happen in a later cleanup cycle.
 */

#include <gtk/gtk.h>

/**
 * @struct SetupAppContext
 * @brief Container holding references to core setup widgets and builder.
 */
typedef struct {
    GtkWidget* main_window;     /**< Main setup window */
    GtkWidget* macro_dialog;    /**< Macro editor dialog */
    GtkTreeView* tree_macro;    /**< Tree view for macro list */
    GtkBuilder* builder;        /**< GTK builder for widget lookups */
    gboolean is_initialized;    /**< Whether context has been set up */
} SetupAppContext;

/**
 * @brief Get the singleton application context.
 *
 * @return Pointer to the global context structure.
 */
SetupAppContext* app_context_get(void);

/**
 * @brief Initialize the application context with the builder and widgets.
 *
 * @param builder The GTK builder instance.
 * @param main_window The main setup window widget.
 * @param macro_dialog The macro editor dialog widget.
 * @param tree_macro The macro tree view widget.
 */
void app_context_init(GtkBuilder* builder,
                      GtkWidget* main_window,
                      GtkWidget* macro_dialog, 
                      GtkTreeView* tree_macro);

/**
 * @brief Get the GTK builder from the context.
 *
 * Convenience function for compatibility with code that previously used
 * the global `builder` variable directly.
 *
 * @return The GTK builder instance, or NULL if not initialized.
 */
GtkBuilder* app_context_get_builder(void);

/**
 * @brief Get the main window from the context.
 *
 * @return The main window widget, or NULL if not initialized.
 */
GtkWidget* app_context_get_main_window(void);

/**
 * @brief Get the macro dialog from the context.
 *
 * @return The macro dialog widget, or NULL if not initialized.
 */
GtkWidget* app_context_get_macro_dialog(void);

/**
 * @brief Get the macro tree view from the context.
 *
 * @return The tree view widget, or NULL if not initialized.
 */
GtkTreeView* app_context_get_tree_macro(void);

#endif // __SETUP_APP_CONTEXT_H
