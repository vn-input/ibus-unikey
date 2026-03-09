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
//------------------------------------------------------------------------------

#ifndef __SETUP_MACRO_DIALOG_STATE_H__
#define __SETUP_MACRO_DIALOG_STATE_H__

#include <gtk/gtk.h>

/**
 * @file macro_dialog_state.h
 * @brief Helpers for persisting and restoring macro import/export chooser state.
 */

/**
 * @brief Remember the parent directory of an imported file in GSettings.
 * @param filename Absolute file path selected by the user.
 */
void macro_dialog_remember_import_file(const gchar* filename);

/**
 * @brief Remember the parent directory of an exported file in GSettings.
 * @param filename Absolute file path selected by the user.
 */
void macro_dialog_remember_export_file(const gchar* filename);

/**
 * @brief Get the remembered import directory.
 * @return Newly allocated directory string, or `NULL` if unset.
 */
gchar* macro_dialog_get_last_import_dir(void);

/**
 * @brief Get the remembered export directory.
 * @return Newly allocated directory string, or `NULL` if unset.
 */
gchar* macro_dialog_get_last_export_dir(void);

/**
 * @brief Apply the remembered import directory and shortcuts to a chooser.
 * @param chooser File chooser widget to initialize.
 */
void macro_dialog_apply_import_folder(GtkFileChooser* chooser);

/**
 * @brief Apply the remembered export directory and shortcuts to a chooser.
 * @param chooser File chooser widget to initialize.
 */
void macro_dialog_apply_export_folder(GtkFileChooser* chooser);

#endif
