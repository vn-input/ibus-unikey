// -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2000-2005 Pham Kim Long
 * Contact:
 *   unikey@gmail.com
 *   UniKey project: http://unikey.org
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
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * @file unikey_config.cpp
 * @brief Implementation for unikey_config.cpp.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/**
 * @file unikey_config.cpp
 * @brief Thin wrapper around GSettings for storing ibus-unikey preferences.
 */

#include "unikey_config.h"

static GSettings* settings;

void ibus_unikey_config_init()
{
    settings = g_settings_new(UNIKEY_GSCHEMA_ID);
}

struct changed_data
{
    void(*cb)(gchar *, gpointer);
    gpointer user_data;
};

static void settings_changed_wrap(GSettings* settings, gchar *name, gpointer user_data)
{
    auto data = (changed_data*)user_data;
    data->cb(name, data->user_data);
}

void ibus_unikey_config_on_changed(void(*cb)(gchar *, gpointer), gpointer user_data)
{
    // TODO should provide a way to cleanup
    auto data = new changed_data{cb, user_data};
    g_signal_connect(settings, "changed", G_CALLBACK(settings_changed_wrap), data);
}

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

void ibus_unikey_config_set_string(const gchar* name, const gchar* value)
{
    g_settings_set_value(settings, name, g_variant_new_string(value));
}

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

void ibus_unikey_config_set_boolean(const gchar* name, gboolean value)
{
    g_settings_set_value(settings, name, g_variant_new_boolean(value));
}

