//------------------------------------------------------------------------------
// UniKey Vietnamese Input Method
// Copyright (C) 2000-2005 Pham Kim Long
// Copyright (C) 2026 Hoang Duy Tran & AI Agent
// Contact:
//   unikey@gmail.com
//   hoangduytran1960@googlemail.com
//   UniKey project: http://unikey.org
//------------------------------------------------------------------------------

#include <cstdio>
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

static gchar* get_saved_string_or_empty(const gchar* key)
{
    gchar* saved_value = NULL;
    const gboolean has_saved_value = ibus_unikey_config_has_key(key)
        && ibus_unikey_config_get_string(key, &saved_value)
        && saved_value != NULL;
    if (has_saved_value) {
        return saved_value;
    }

    g_free(saved_value);
    return g_strdup("");
}

static void remember_simple_string(const gchar* key, const gchar* value)
{
    const bool has_value = (value != NULL);
    ibus_unikey_config_set_string(key, has_value ? value : "");
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

void macro_dialog_remember_search_text(const gchar* text)
{
    remember_simple_string(CONFIG_MACRO_SEARCH_TEXT, text);
}

gchar* macro_dialog_get_search_text(void)
{
    return get_saved_string_or_empty(CONFIG_MACRO_SEARCH_TEXT);
}

void macro_dialog_remember_selected_key(const gchar* key)
{
    remember_simple_string(CONFIG_MACRO_SELECTED_KEY, key);
}

gchar* macro_dialog_get_selected_key(void)
{
    return get_saved_string_or_empty(CONFIG_MACRO_SELECTED_KEY);
}

void macro_dialog_remember_export_format(const gchar* extension)
{
    remember_simple_string(CONFIG_MACRO_EXPORT_FORMAT, extension);
}

gchar* macro_dialog_get_export_format(void)
{
    return get_saved_string_or_empty(CONFIG_MACRO_EXPORT_FORMAT);
}

void macro_dialog_remember_window_geometry(GtkWindow* window)
{
    if (window == NULL) {
        return;
    }

    int width = 0;
    int height = 0;
    gtk_window_get_size(window, &width, &height);
    gchar* size_text = g_strdup_printf("%dx%d", width, height);
    remember_simple_string(CONFIG_MACRO_DIALOG_SIZE, size_text);
    g_free(size_text);

    int x = 0;
    int y = 0;
    gtk_window_get_position(window, &x, &y);
    gchar* position_text = g_strdup_printf("%d,%d", x, y);
    remember_simple_string(CONFIG_MACRO_DIALOG_POSITION, position_text);
    g_free(position_text);
}

void macro_dialog_apply_window_geometry(GtkWindow* window)
{
    if (window == NULL) {
        return;
    }

    gchar* size_text = get_saved_string_or_empty(CONFIG_MACRO_DIALOG_SIZE);
    int width = 0;
    int height = 0;
    if (sscanf(size_text, "%dx%d", &width, &height) == 2 && width > 0 && height > 0) {
        gtk_window_set_default_size(window, width, height);
    }
    g_free(size_text);

    gchar* position_text = get_saved_string_or_empty(CONFIG_MACRO_DIALOG_POSITION);
    int x = 0;
    int y = 0;
    if (sscanf(position_text, "%d,%d", &x, &y) == 2) {
        gtk_window_move(window, x, y);
    }
    g_free(position_text);
}
