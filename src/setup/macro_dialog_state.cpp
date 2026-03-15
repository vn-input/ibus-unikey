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
 * @brief Return the shared remembered chooser directory or seed the default one.
 * @return Newly allocated absolute directory path.
 */
static gchar* get_saved_dir_or_default(void)
{
    bool has_settings_key = ibus_unikey_config_has_key(CONFIG_MACRO_LASTWORKINGDIR);

    gchar* saved_dir = NULL;
    gboolean has_saved_dir = has_settings_key
        && ibus_unikey_config_get_string(CONFIG_MACRO_LASTWORKINGDIR, &saved_dir);
    bool is_missing_dir = (has_saved_dir == FALSE || saved_dir == NULL || saved_dir[0] == '\0');
    bool is_existing_dir = (!is_missing_dir) && g_file_test(saved_dir, G_FILE_TEST_IS_DIR);
    if (is_existing_dir) {
        return saved_dir;
    }

    g_free(saved_dir);
    gchar* default_dir = get_default_import_export_dir();
    if (has_settings_key) {
        ibus_unikey_config_set_string(CONFIG_MACRO_LASTWORKINGDIR, default_dir);
    }
    return default_dir;
}

/**
 * @brief Store the parent directory of a selected file.
 *
 * @param filename Absolute selected filename.
 */
static void remember_file_dir(const gchar* filename)
{
    bool has_filename = (filename != NULL && filename[0] != '\0');
    if (!has_filename) {
        return;
    }

    gchar* dir_name = g_path_get_dirname(filename);
    bool has_valid_dir = (dir_name != NULL && strcmp(dir_name, ".") != 0 && dir_name[0] != '\0');
    if (has_valid_dir) {
        ibus_unikey_config_set_string(CONFIG_MACRO_LASTWORKINGDIR, dir_name);
    }
    g_free(dir_name);
}

/**
 * @brief Initialize chooser folder from the shared saved directory.
 *
 * @param chooser File chooser widget.
 */
static void apply_saved_folder(GtkFileChooser* chooser)
{
    gchar* saved_dir = get_saved_dir_or_default();

    bool is_existing_dir = g_file_test(saved_dir, G_FILE_TEST_IS_DIR);
    if (is_existing_dir) {
        gtk_file_chooser_set_current_folder(chooser, saved_dir);
    }

    g_free(saved_dir);
}

void macro_dialog_remember_import_file(const gchar* filename)
{
    remember_file_dir(filename);
}

void macro_dialog_remember_export_file(const gchar* filename)
{
    remember_file_dir(filename);
}

gchar* macro_dialog_get_last_import_dir(void)
{
    return get_saved_dir_or_default();
}

gchar* macro_dialog_get_last_export_dir(void)
{
    return get_saved_dir_or_default();
}

void macro_dialog_apply_import_folder(GtkFileChooser* chooser)
{
    apply_saved_folder(chooser);
}

void macro_dialog_apply_export_folder(GtkFileChooser* chooser)
{
    apply_saved_folder(chooser);
}
