#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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

