//------------------------------------------------------------------------------
// UniKey Vietnamese Input Method
// Copyright (C) 2000-2005 Pham Kim Long
// Copyright (C) 2026 Hoang Duy Tran & AI Agent
// Contact:
//   unikey@gmail.com
//   hoangduytran1960@googlemail.com
//   UniKey project: http://unikey.org
//------------------------------------------------------------------------------

#include <cstring>

#include "macro_dialog_state.h"
#include "unikey_config.h"

/**
 * @brief Default folder name used when no remembered chooser folder exists.
 */
#define DEFAULT_IMPORT_EXPORT_DIR "Documents"

/**
 * @brief Build the default chooser directory under the current HOME.
 *
 * The directory is created on demand so import/export can always start from a
 * valid folder even on a fresh profile.
 *
 * @return Newly allocated absolute path.
 */
static gchar* get_default_import_export_dir()
{
    gchar* default_dir = g_build_filename(g_get_home_dir(), DEFAULT_IMPORT_EXPORT_DIR, NULL);
    g_mkdir_with_parents(default_dir, 0700);
    return default_dir;
}

/**
 * @brief Return a remembered chooser directory or seed the default one.
 *
 * @param key GSettings key to read/write.
 * @return Newly allocated absolute directory path.
 */
static gchar* get_saved_dir_or_default(const gchar* key)
{
    bool has_settings_key = ibus_unikey_config_has_key(key);

    gchar* saved_dir = NULL;
    gboolean has_saved_dir = has_settings_key && ibus_unikey_config_get_string(key, &saved_dir);
    bool is_missing_dir = (has_saved_dir == FALSE || saved_dir == NULL || saved_dir[0] == '\0');
    bool is_existing_dir = (!is_missing_dir) && g_file_test(saved_dir, G_FILE_TEST_IS_DIR);
    if (is_existing_dir) {
        return saved_dir;
    }

    g_free(saved_dir);
    gchar* default_dir = get_default_import_export_dir();
    if (has_settings_key) {
        ibus_unikey_config_set_string(key, default_dir);
    }
    return default_dir;
}

/**
 * @brief Read a saved chooser directory and treat empty strings as unset.
 *
 * @param key GSettings key to read.
 * @param out_dir Receives a newly allocated directory string on success.
 * @return `TRUE` if a non-empty directory was found, otherwise `FALSE`.
 */
static gboolean get_saved_dir(const gchar* key, gchar** out_dir)
{
    if (!ibus_unikey_config_has_key(key)) {
        return FALSE;
    }

    gchar* saved_dir = NULL;
    gboolean has_saved_dir = ibus_unikey_config_get_string(key, &saved_dir);
    bool is_missing_dir = (has_saved_dir == FALSE || saved_dir == NULL || saved_dir[0] == '\0');
    if (is_missing_dir) {
        g_free(saved_dir);
        return FALSE;
    }

    *out_dir = saved_dir;
    return TRUE;
}

/**
 * @brief Store the parent directory of a selected file.
 *
 * @param key GSettings key to update.
 * @param filename Absolute selected filename.
 */
static void remember_file_dir(const gchar* key, const gchar* filename)
{
    bool has_filename = (filename != NULL && filename[0] != '\0');
    if (!has_filename) {
        return;
    }

    gchar* dir_name = g_path_get_dirname(filename);
    bool has_valid_dir = (dir_name != NULL && strcmp(dir_name, ".") != 0 && dir_name[0] != '\0');
    if (has_valid_dir) {
        ibus_unikey_config_set_string(key, dir_name);
    }
    g_free(dir_name);
}

/**
 * @brief Add a valid saved directory as a chooser shortcut.
 *
 * @param chooser File chooser that should expose the shortcut.
 * @param dir_path Saved directory candidate.
 */
static void add_shortcut_if_valid(GtkFileChooser* chooser, const gchar* dir_path)
{
    bool has_dir_path = (dir_path != NULL && dir_path[0] != '\0');
    bool is_existing_dir = has_dir_path && g_file_test(dir_path, G_FILE_TEST_IS_DIR);
    if (!is_existing_dir) {
        return;
    }

    GError* error = NULL;
    gtk_file_chooser_add_shortcut_folder(chooser, dir_path, &error);
    if (error != NULL) {
        g_error_free(error);
    }
}

/**
 * @brief Add both saved import/export locations as chooser shortcuts.
 *
 * @param chooser File chooser widget.
 */
static void add_saved_shortcuts(GtkFileChooser* chooser)
{
    gchar* import_dir = macro_dialog_get_last_import_dir();
    gchar* export_dir = macro_dialog_get_last_export_dir();

    add_shortcut_if_valid(chooser, import_dir);

    bool has_distinct_export_dir = (export_dir != NULL && import_dir != NULL)
        ? (g_strcmp0(import_dir, export_dir) != 0)
        : (export_dir != NULL);
    if (has_distinct_export_dir) {
        add_shortcut_if_valid(chooser, export_dir);
    }

    g_free(import_dir);
    g_free(export_dir);
}

/**
 * @brief Initialize chooser folder from a saved key and add shortcuts.
 *
 * @param chooser File chooser widget.
 * @param key Saved directory setting name.
 */
static void apply_saved_folder(GtkFileChooser* chooser, const gchar* key)
{
    add_saved_shortcuts(chooser);

    gchar* saved_dir = get_saved_dir_or_default(key);

    bool is_existing_dir = g_file_test(saved_dir, G_FILE_TEST_IS_DIR);
    if (is_existing_dir) {
        gtk_file_chooser_set_current_folder(chooser, saved_dir);
    }

    g_free(saved_dir);
}

void macro_dialog_remember_import_file(const gchar* filename)
{
    remember_file_dir(CONFIG_MACRO_LASTIMPORTDIR, filename);
}

void macro_dialog_remember_export_file(const gchar* filename)
{
    remember_file_dir(CONFIG_MACRO_LASTEXPORTDIR, filename);
}

gchar* macro_dialog_get_last_import_dir(void)
{
    return get_saved_dir_or_default(CONFIG_MACRO_LASTIMPORTDIR);
}

gchar* macro_dialog_get_last_export_dir(void)
{
    return get_saved_dir_or_default(CONFIG_MACRO_LASTEXPORTDIR);
}

void macro_dialog_apply_import_folder(GtkFileChooser* chooser)
{
    apply_saved_folder(chooser, CONFIG_MACRO_LASTIMPORTDIR);
}

void macro_dialog_apply_export_folder(GtkFileChooser* chooser)
{
    apply_saved_folder(chooser, CONFIG_MACRO_LASTEXPORTDIR);
}
