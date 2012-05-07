#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libintl.h>

#include <ibus.h>
#include "ukengine.h"
#include "utils.h"
#include "engine_const.h"

#define _(string) gettext(string)

#define IU_DESC _("Vietnamese Input Method Engine for IBus using Unikey Engine\n\
Usage:\n\
  - Choose input method, output charset, options in language bar.\n\
  - There are 4 input methods: Telex, Vni, STelex (simple telex) \
and STelex2 (which same as STelex, the difference is it use w as ư).\n\
  - And 7 output charsets: Unicode (UTF-8), TCVN3, VNI Win, VIQR, CString, NCR Decimal and NCR Hex.\n\
  - Use <Shift>+<Space> or <Shift>+<Shift> to restore keystrokes.\n\
  - Use <Control> to commit a word.\
")

const gchar*          Unikey_IMNames[]    = {"Telex", "Vni", "STelex", "STelex2"};
const UkInputMethod   Unikey_IM[]         = {UkTelex, UkVni, UkSimpleTelex, UkSimpleTelex2};
const unsigned int    NUM_INPUTMETHOD     = sizeof(Unikey_IM)/sizeof(Unikey_IM[0]);

const gchar*          Unikey_OCNames[]    = {"Unicode",
                                             "TCVN3",
                                             "VNI Win",
                                             "VIQR",
                                             "BK HCM 2",
                                             "CString",
                                             "NCR Decimal",
                                             "NCR Hex"};
const unsigned int    Unikey_OC[]         = {CONV_CHARSET_XUTF8,
                                             CONV_CHARSET_TCVN3,
                                             CONV_CHARSET_VNIWIN,
                                             CONV_CHARSET_VIQR,
                                             CONV_CHARSET_BKHCM2,
                                             CONV_CHARSET_UNI_CSTRING,
                                             CONV_CHARSET_UNIREF,
                                             CONV_CHARSET_UNIREF_HEX};
const unsigned int    NUM_OUTPUTCHARSET   = sizeof(Unikey_OC)/sizeof(Unikey_OC[0]);

IBusComponent* ibus_unikey_get_component()
{
    IBusComponent* component;
    IBusEngineDesc* engine;

    component = ibus_component_new("org.freedesktop.IBus.Unikey",
                                   "Unikey",
                                   PACKAGE_VERSION,
                                   "GPLv3",
                                   "Lê Quốc Tuấn <mr.lequoctuan@gmail.com>",
                                   PACKAGE_BUGREPORT,
                                   "",
                                   PACKAGE_NAME);
    
#if IBUS_CHECK_VERSION(1,3,99)
    engine = ibus_engine_desc_new_varargs ("name",        "Unikey",
                                           "longname",    "Unikey",
                                           "description", IU_DESC,
                                           "language",    "vi",
                                           "license",     "GPLv3",
                                           "author",      "Lê Quốc Tuấn <mr.lequoctuan@gmail.com>",
                                           "icon",        PKGDATADIR"/icons/ibus-unikey.png",
                                           "layout",      "us",
                                           "rank",        99,
                                           "setup",       LIBEXECDIR "/ibus-setup-unikey",
                                           NULL);
#else
    engine = ibus_engine_desc_new  ("Unikey",
                                    "Unikey",
                                    IU_DESC,
                                    "vi",
                                    "GPLv3",
                                    "Lê Quốc Tuấn <mr.lequoctuan@gmail.com>",
                                    PKGDATADIR"/icons/ibus-unikey.png",
                                    "us");
    engine->rank = 99;
#endif

    ibus_component_add_engine(component, engine);

    return component;
}

// code from x-unikey, for convert charset that not is XUtf-8
int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize)
{
    int i;
    int outLeft;
    unsigned char ch;

    outLeft = *pOutSize;

    for (i=0; i<inSize; i++)
    {
        ch = *src++;
        if (ch < 0x80)
        {
            outLeft -= 1;
            if (outLeft >= 0)
                *dst++ = ch;
        }
        else
        {
            outLeft -= 2;
            if (outLeft >= 0)
            {
                *dst++ = (0xC0 | ch >> 6);
                *dst++ = (0x80 | (ch & 0x3F));
            }
        }
    }

    *pOutSize = outLeft;
    return (outLeft >= 0);
}

gboolean ibus_unikey_config_get_string(IBusConfig* config,
                                    const gchar* section,
                                    const gchar* name,
                                    gchar** result)
{
#if IBUS_CHECK_VERSION(1,3,99)
    GVariant *value = NULL;
    value = ibus_config_get_value(config, section, name);
    if (value)
    {
        *result = g_strdup((gchar*)g_variant_get_string(value, NULL));
        g_variant_unref(value);
        return true;
    }
    return false;
#else
    GValue value = {0};
    if (ibus_config_get_value(config, section, name, &value))
    {
        *result = g_strdup((gchar*)g_value_get_string(&value));
        g_value_unset(&value);
        return true;
    }
    return false;
#endif
}

void ibus_unikey_config_set_string(IBusConfig* config,
                                    const gchar* section,
                                    const gchar* name,
                                    const gchar* value)
{
#if IBUS_CHECK_VERSION(1,3,99)
    ibus_config_set_value(config, section, name, g_variant_new_string(value));
#else
    GValue v = {0};
    g_value_init(&v, G_TYPE_STRING);
    g_value_set_string(&v, value);
    ibus_config_set_value(config, section, name, &v);
#endif
}

gboolean ibus_unikey_config_get_boolean(IBusConfig* config,
                                        const gchar* section,
                                        const gchar* name,
                                        gboolean* result)
{
#if IBUS_CHECK_VERSION(1,3,99)
    GVariant *value = NULL;
    value = ibus_config_get_value(config, section, name);
    if (value)
    {
        *result = g_variant_get_boolean(value);
        g_variant_unref(value);
        return true;
    }
    return false;
#else
    GValue value = {0};
    if (ibus_config_get_value(config, section, name, &value))
    {
        *result = g_value_get_boolean(&value);
        g_value_unset(&value);
        return true;
    }
    return false;
#endif
}

void ibus_unikey_config_set_boolean(IBusConfig* config,
                                    const gchar* section,
                                    const gchar* name,
                                    gboolean value)
{
#if IBUS_CHECK_VERSION(1,3,99)
    ibus_config_set_value(config, section, name, g_variant_new_boolean(value));
#else
    GValue v = {0};
    g_value_init(&v, G_TYPE_BOOLEAN);
    g_value_set_boolean(&v, value);
    ibus_config_set_value(config, section, name, &v);
#endif
}

#if !IBUS_CHECK_VERSION(1,3,99)
char* ibus_property_get_key(IBusProperty *prop)
{
    return prop->key;
}
#endif

