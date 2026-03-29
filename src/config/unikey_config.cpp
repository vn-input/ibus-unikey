// -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2000-2005 Pham Kim Long
 * Contact:
 *   unikey@gmail.com
 *   UniKey project: http://unikey.org
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * @file unikey_config.cpp
 * @brief Thin wrapper around GSettings for storing ibus-unikey preferences.
 */

#include "unikey_config.h"

/**
 * @brief Singleton GSettings instance used by the configuration API.
 */
static GSettings* settings;

/**
 * @brief Look up the ibus-unikey schema from the current schema source.
 *
 * @return Referenced schema on success, otherwise `NULL`.
 */
static GSettingsSchema* lookup_unikey_schema_from_source(GSettingsSchemaSource* source,
                                                         gboolean recursive)
{
    if (source == NULL) {
        return NULL;
    }

    return g_settings_schema_source_lookup(source, UNIKEY_GSCHEMA_ID, recursive);
}

/**
 * @brief Load the ibus-unikey schema from an explicit compiled-schema directory.
 *
 * @param schema_dir Directory containing `gschemas.compiled`.
 * @return Referenced schema on success, otherwise `NULL`.
 */
static GSettingsSchema* lookup_unikey_schema_from_directory(const gchar* schema_dir)
{
    const gboolean has_schema_dir = (schema_dir != NULL && schema_dir[0] != '\0');
    if (!has_schema_dir) {
        return NULL;
    }

    GError* error = NULL;
    GSettingsSchemaSource* source = g_settings_schema_source_new_from_directory(
        schema_dir,
        NULL,
        FALSE,
        &error);
    if (source == NULL) {
        if (error != NULL) {
            g_error_free(error);
        }
        return NULL;
    }

    GSettingsSchema* schema = lookup_unikey_schema_from_source(source, FALSE);
    g_settings_schema_source_unref(source);
    return schema;
}

/**
 * @brief Choose the single schema directory ibus-unikey should use.
 *
 * Tests may override this with `GSETTINGS_SCHEMA_DIR`. Installed builds fall
 * back to the schema directory computed from the install prefix.
 *
 * @return Absolute schema directory path.
 */
static const gchar* get_preferred_schema_dir()
{
    const gchar* explicit_schema_dir = g_getenv("GSETTINGS_SCHEMA_DIR");
    const gboolean has_explicit_schema_dir =
        (explicit_schema_dir != NULL && explicit_schema_dir[0] != '\0');
    if (has_explicit_schema_dir) {
        return explicit_schema_dir;
    }

    return UNIKEY_GSETTINGS_DIR;
}

/**
 * @brief Load the ibus-unikey schema from the one configured schema directory.
 *
 * @return Referenced best-match schema on success, otherwise `NULL`.
 */
static GSettingsSchema* lookup_unikey_schema()
{
    return lookup_unikey_schema_from_directory(get_preferred_schema_dir());
}

/**
 * @brief Initialize the configuration subsystem.
 *
 * This allocates a new GSettings object using the predefined schema id.
 */
void ibus_unikey_config_init()
{
    GSettingsSchema* schema = lookup_unikey_schema();
    if (schema != NULL) {
        settings = g_settings_new_full(schema, NULL, NULL);
        g_settings_schema_unref(schema);
        return;
    }

    settings = g_settings_new(UNIKEY_GSCHEMA_ID);
}

/**
 * @struct changed_data
 * @brief Internal wrapper storing callback and user data for GSettings signals.
 */
struct changed_data
{
    void(*cb)(gchar *, gpointer); /**< user callback invoked on change */
    gpointer user_data;          /**< opaque pointer passed to callback */
};

/**
 * @brief Internal signal handler invoked by GSettings on key change.
 * @param settings GSettings instance (unused).
 * @param name Changed key name.
 * @param user_data Pointer to a @c changed_data struct.
 */
static void settings_changed_wrap(GSettings* settings, gchar *name, gpointer user_data)
{
    auto data = (changed_data*)user_data;
    data->cb(name, data->user_data);
}

/**
 * @brief Register a callback for when any setting changes.
 *
 * The callback receives the key name and the user_data pointer provided.
 *
 * @param cb Function invoked on change.
 * @param user_data Opaque pointer forwarded to @p cb.
 */
void ibus_unikey_config_on_changed(void(*cb)(gchar *, gpointer), gpointer user_data)
{
    // TODO should provide a way to cleanup
    auto data = new changed_data{cb, user_data};
    g_signal_connect(settings, "changed", G_CALLBACK(settings_changed_wrap), data);
}

gboolean ibus_unikey_config_has_key(const gchar* name)
{
    GSettingsSchema* schema = lookup_unikey_schema();
    if (schema == NULL) {
        return FALSE;
    }

    gboolean has_key = g_settings_schema_has_key(schema, name);
    g_settings_schema_unref(schema);
    return has_key;
}

/**
 * @brief Read a string value from the settings.
 *
 * Caller receives newly-allocated string which must be freed with g_free().
 *
 * @param name Name of the GSettings key.
 * @param[out] result Receives allocated UTF-8 string on success.
 * @return TRUE if the key existed and returned a string, FALSE otherwise.
 */
gboolean ibus_unikey_config_get_string(const gchar* name, gchar** result)
{
    if (!ibus_unikey_config_has_key(name)) {
        return false;
    }

    GVariant *value = NULL;
    value = g_settings_get_value(settings, name);
    if (value)
    {
        *result = g_variant_dup_string(value, NULL);
        g_variant_unref(value);
        return true;
    }
    return false;
}

/**
 * @brief Store a UTF-8 string into the settings.
 *
 * Logs a warning if the write operation fails (e.g., due to backend error
 * or permission issues).
 *
 * @param name Key name.
 * @param value UTF-8 string value.
 * @return TRUE if write succeeded, FALSE otherwise.
 */
gboolean ibus_unikey_config_set_string(const gchar* name, const gchar* value)
{
    if (!ibus_unikey_config_has_key(name)) {
        g_warning("ibus-unikey config: attempted to set unknown key '%s'", name);
        return FALSE;
    }

    gboolean success = g_settings_set_value(settings, name, g_variant_new_string(value));
    if (!success) {
        g_warning("ibus-unikey config: failed to write string key '%s'", name);
    }
    return success;
}

/**
 * @brief Read a boolean setting value.
 * @param name Key name.
 * @param[out] result Receives boolean result on success.
 * @return TRUE if read succeeded, FALSE if key missing/invalid.
 */
gboolean ibus_unikey_config_get_boolean(const gchar* name, gboolean* result)
{
    if (!ibus_unikey_config_has_key(name)) {
        return false;
    }

    GVariant *value = NULL;
    value = g_settings_get_value(settings, name);
    if (value)
    {
        *result = g_variant_get_boolean(value);
        g_variant_unref(value);
        return true;
    }
    return false;
}

/**
 * @brief Write a boolean value to the settings.
 *
 * Logs a warning if the write operation fails (e.g., due to backend error
 * or permission issues).
 *
 * @param name Key name.
 * @param value Boolean value to store.
 * @return TRUE if write succeeded, FALSE otherwise.
 */
gboolean ibus_unikey_config_set_boolean(const gchar* name, gboolean value)
{
    if (!ibus_unikey_config_has_key(name)) {
        g_warning("ibus-unikey config: attempted to set unknown key '%s'", name);
        return FALSE;
    }

    gboolean success = g_settings_set_value(settings, name, g_variant_new_boolean(value));
    if (!success) {
        g_warning("ibus-unikey config: failed to write boolean key '%s'", name);
    }
    return success;
}

