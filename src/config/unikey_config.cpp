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
 * @brief Initialize the configuration subsystem.
 *
 * This allocates a new GSettings object using the predefined schema id.
 */
void ibus_unikey_config_init()
{
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
 * @param name Key name.
 * @param value UTF-8 string value.
 */
void ibus_unikey_config_set_string(const gchar* name, const gchar* value)
{
    g_settings_set_value(settings, name, g_variant_new_string(value));
}

/**
 * @brief Read a boolean setting value.
 * @param name Key name.
 * @param[out] result Receives boolean result on success.
 * @return TRUE if read succeeded, FALSE if key missing/invalid.
 */
gboolean ibus_unikey_config_get_boolean(const gchar* name, gboolean* result)
{
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
 * @param name Key name.
 * @param value Boolean value to store.
 */
void ibus_unikey_config_set_boolean(const gchar* name, gboolean value)
{
    g_settings_set_value(settings, name, g_variant_new_boolean(value));
}

