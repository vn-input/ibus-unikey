#include <stdlib.h>
#include <string.h>
#include <ibus.h>

#include "dlg_main_setup.h"
#include "engine_const.h"

const gchar* Unikey_IMNames[] = {"Telex", "Vni", "STelex", "STelex2"};
const unsigned int NUM_INPUTMETHOD = sizeof(Unikey_IMNames)/sizeof(Unikey_IMNames[0]);
const gchar* Unikey_OCNames[] = {"Unicode", "TCVN3", "VNI Win", "VIQR", "BK HCM 2", "CString", "NCR Decimal", "NCR Hex"};
const unsigned int NUM_OUTPUTCHARSET = sizeof(Unikey_OCNames)/sizeof(Unikey_OCNames[0]);

#define get_macro_file() (g_build_filename(g_getenv("HOME"), UNIKEY_MACRO_FILE, NULL))

static void set_default_config(UnikeyMainSetupOptions* opt)
{
    opt->input_method = 0;
    opt->output_charset = 0;
    opt->enableSpellcheck   = DEFAULT_CONF_SPELLCHECK;
    opt->autoRestoreNonVn   = DEFAULT_CONF_AUTONONVNRESTORE;
    opt->modernStyle        = DEFAULT_CONF_MODERNSTYLE;
    opt->freeMarking        = DEFAULT_CONF_FREEMARKING;
    opt->enableMacro        = DEFAULT_CONF_MACROENABLED;
    opt->processwatbegin    = DEFAULT_CONF_PROCESSWATBEGIN;
    opt->mousecapture       = DEFAULT_CONF_MOUSECAPTURE;

    opt->macrofile = get_macro_file();
}

void read_config(void *data, UnikeyMainSetupOptions* opt)
{
    IBusConfig* config = (IBusConfig*)data;
    GVariant* value = {0};
    gchar* str;
    guint i;

    set_default_config(opt);

    // get Input method
    if ((value = ibus_config_get_value(config, CONFIG_SECTION, CONFIG_INPUTMETHOD)))
    {   
        str = (gchar *) g_variant_get_string(value, NULL);
        for (i = 0; i < NUM_INPUTMETHOD; i++)
        {   
            if (strcasecmp(str, Unikey_IMNames[i]) == 0)
            {   
                opt->input_method = i; 
                break;
            }
        }
        g_variant_unref(value);
    }

    // get Output charset
    if ((value = ibus_config_get_value(config, CONFIG_SECTION, CONFIG_OUTPUTCHARSET)))
    {  
        str = (gchar *) g_variant_get_string(value, NULL);
        for (i = 0; i < NUM_OUTPUTCHARSET; i++)
        {  
            if (strcasecmp(str, Unikey_OCNames[i]) == 0)
            {  
                opt->output_charset = i;
                break;
            }
        }
        g_variant_unref(value);
    }

    // get Spellcheck
    if ((value = ibus_config_get_value(config, CONFIG_SECTION, CONFIG_SPELLCHECK)))
    {   
        opt->enableSpellcheck = g_variant_get_boolean(value);
        g_variant_unref(value);
    }

    // get autoRestoreNonVn
    if ((value = ibus_config_get_value(config, CONFIG_SECTION, CONFIG_AUTORESTORENONVN)))
    {   
        opt->autoRestoreNonVn = g_variant_get_boolean(value);
        g_variant_unref(value);
    }

    // get modernStyle
    if ((value = ibus_config_get_value(config, CONFIG_SECTION, CONFIG_MODERNSTYLE)))
    {   
        opt->modernStyle = g_variant_get_boolean(value);
        g_variant_unref(value);
    }

    // get freeMarking
    if ((value = ibus_config_get_value(config, CONFIG_SECTION, CONFIG_FREEMARKING)))
    {   
        opt->freeMarking = g_variant_get_boolean(value);
        g_variant_unref(value);
    }

    // get enableMacro
    if ((value = ibus_config_get_value(config, CONFIG_SECTION, CONFIG_MACROENABLED)))
    {   
        opt->enableMacro = g_variant_get_boolean(value);
        g_variant_unref(value);
    }

    // get ProcessWAtBegin
    if ((value = ibus_config_get_value(config, CONFIG_SECTION, CONFIG_PROCESSWATBEGIN)))
    {   
        opt->processwatbegin = g_variant_get_boolean(value);
        g_variant_unref(value);
    }

    // get MouseCapture
    if ((value = ibus_config_get_value(config, CONFIG_SECTION, CONFIG_MOUSECAPTURE)))
    {   
        opt->mousecapture = g_variant_get_boolean(value);
        g_variant_unref(value);
    }
}

void write_config(void* data, UnikeyMainSetupOptions* opt)
{
    IBusConfig* config = (IBusConfig*)data;

    ibus_config_set_value(config, CONFIG_SECTION, CONFIG_INPUTMETHOD,
                          g_variant_new_string(Unikey_IMNames[opt->input_method]));
    ibus_config_set_value(config, CONFIG_SECTION, CONFIG_OUTPUTCHARSET,
                          g_variant_new_string(Unikey_OCNames[opt->output_charset]));
    ibus_config_set_value(config, CONFIG_SECTION, CONFIG_SPELLCHECK,
                          g_variant_new_boolean(opt->enableSpellcheck));
    ibus_config_set_value(config, CONFIG_SECTION, CONFIG_AUTORESTORENONVN,
                          g_variant_new_boolean(opt->autoRestoreNonVn));
    ibus_config_set_value(config, CONFIG_SECTION, CONFIG_MODERNSTYLE,
                          g_variant_new_boolean(opt->modernStyle));
    ibus_config_set_value(config, CONFIG_SECTION, CONFIG_FREEMARKING,
                          g_variant_new_boolean(opt->freeMarking));
    ibus_config_set_value(config, CONFIG_SECTION, CONFIG_MACROENABLED,
                          g_variant_new_boolean(opt->enableMacro));
    ibus_config_set_value(config, CONFIG_SECTION, CONFIG_PROCESSWATBEGIN,
                          g_variant_new_boolean(opt->processwatbegin));
    ibus_config_set_value(config, CONFIG_SECTION, CONFIG_MOUSECAPTURE,
                          g_variant_new_boolean(opt->mousecapture));
}

int force_engine_to_reload_config()
{
    return system("ibus-daemon -xrd");
}

