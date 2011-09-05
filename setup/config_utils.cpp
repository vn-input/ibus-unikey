#include <stdlib.h>
#include <string.h>
#include <ibus.h>

#include "utils.h"
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
    gchar* str;
    gboolean b;
    guint i;

    set_default_config(opt);

    // get Input method
    if (ibus_unikey_config_get_string(config, CONFIG_SECTION, CONFIG_INPUTMETHOD, &str))
    {   
        for (i = 0; i < NUM_INPUTMETHOD; i++)
        {   
            if (strcasecmp(str, Unikey_IMNames[i]) == 0)
            {   
                opt->input_method = i; 
                break;
            }
        }
    }

    // get Output charset
    if (ibus_unikey_config_get_string(config, CONFIG_SECTION, CONFIG_OUTPUTCHARSET, &str))
    {  
        for (i = 0; i < NUM_OUTPUTCHARSET; i++)
        {  
            if (strcasecmp(str, Unikey_OCNames[i]) == 0)
            {  
                opt->output_charset = i;
                break;
            }
        }
    }

    // get Spellcheck
    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_SPELLCHECK, &b))
        opt->enableSpellcheck = b;

    // get autoRestoreNonVn
    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_AUTORESTORENONVN, &b))
        opt->autoRestoreNonVn = b;

    // get modernStyle
    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_MODERNSTYLE, &b))
        opt->modernStyle = b;

    // get freeMarking
    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_FREEMARKING, &b))
        opt->freeMarking = b;

    // get enableMacro
    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_MACROENABLED, &b))
        opt->enableMacro = b;

    // get ProcessWAtBegin
    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_PROCESSWATBEGIN, &b))
        opt->processwatbegin = b;

    // get MouseCapture
    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_MOUSECAPTURE, &b))
        opt->mousecapture = b;
}

void write_config(void* data, UnikeyMainSetupOptions* opt)
{
    IBusConfig* config = (IBusConfig*)data;

    ibus_unikey_config_set_string(config, CONFIG_SECTION, CONFIG_INPUTMETHOD,
                                    Unikey_IMNames[opt->input_method]);
    ibus_unikey_config_set_string(config, CONFIG_SECTION, CONFIG_OUTPUTCHARSET,
                                    Unikey_OCNames[opt->output_charset]);
    ibus_unikey_config_set_boolean(config, CONFIG_SECTION, CONFIG_SPELLCHECK,
                                    opt->enableSpellcheck);
    ibus_unikey_config_set_boolean(config, CONFIG_SECTION, CONFIG_AUTORESTORENONVN,
                                    opt->autoRestoreNonVn);
    ibus_unikey_config_set_boolean(config, CONFIG_SECTION, CONFIG_MODERNSTYLE,
                                    opt->modernStyle);
    ibus_unikey_config_set_boolean(config, CONFIG_SECTION, CONFIG_FREEMARKING,
                                    opt->freeMarking);
    ibus_unikey_config_set_boolean(config, CONFIG_SECTION, CONFIG_MACROENABLED,
                                    opt->enableMacro);
    ibus_unikey_config_set_boolean(config, CONFIG_SECTION, CONFIG_PROCESSWATBEGIN,
                                    opt->processwatbegin);
    ibus_unikey_config_set_boolean(config, CONFIG_SECTION, CONFIG_MOUSECAPTURE,
                                    opt->mousecapture);
}

int force_engine_to_reload_config()
{
    return system("ibus-daemon -xrd");
}

