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

/**
 * @file app_context.cpp
 * @brief Implementation of centralized setup context.
 *
 * This provides a singleton-style context that holds references to the
 * key UI elements, replacing direct global variable access with accessor
 * functions that can eventually be refactored to explicit parameter passing.
 */

#include "app_context.h"

/** @brief Singleton application context instance. */
static SetupAppContext g_app_context = {
    NULL,   // main_window
    NULL,   // macro_dialog
    NULL,   // tree_macro
    NULL,   // builder
    FALSE   // is_initialized
};

SetupAppContext* app_context_get(void)
{
    return &g_app_context;
}

void app_context_init(GtkBuilder* builder,
                      GtkWidget* main_window,
                      GtkWidget* macro_dialog,
                      GtkTreeView* tree_macro)
{
    g_app_context.builder = builder;
    g_app_context.main_window = main_window;
    g_app_context.macro_dialog = macro_dialog;
    g_app_context.tree_macro = tree_macro;
    g_app_context.is_initialized = TRUE;
}

GtkBuilder* app_context_get_builder(void)
{
    return g_app_context.builder;
}

GtkWidget* app_context_get_main_window(void)
{
    return g_app_context.main_window;
}

GtkWidget* app_context_get_macro_dialog(void)
{
    return g_app_context.macro_dialog;
}

GtkTreeView* app_context_get_tree_macro(void)
{
    return g_app_context.tree_macro;
}
