#include <stdlib.h>
#include <string.h>
#include <gconf/gconf.h>

#include "dlg_main_setup.h"
#include "engine_const.h"

const gchar* Unikey_IMNames[] = {"Telex", "Vni", "STelex", "STelex2"};
const unsigned int NUM_INPUTMETHOD = sizeof(Unikey_IMNames)/sizeof(Unikey_IMNames[0]);
const gchar* Unikey_OCNames[] = {"Unicode", "TCVN3", "VNI Win", "VIQR", "BK HCM 2", "CString", "NCR Decimal", "NCR Hex"};
const unsigned int NUM_OUTPUTCHARSET = sizeof(Unikey_OCNames)/sizeof(Unikey_OCNames[0]);

#define get_macro_file() (g_build_filename(g_getenv("HOME"), UNIKEY_MACRO_FILE, NULL))

void set_default_config(UnikeyMainSetupOptions* opt)
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

void read_config(UnikeyMainSetupOptions* opt)
{
    GConfEngine* e;
    gchar* s;
    GConfValue* v;
    guint i;

    e = gconf_engine_get_default();

    // get Input method
    s = gconf_engine_get_string(e, GCONF_PREFIX CONFIG_INPUTMETHOD, NULL);
    if (s != NULL)
    {
        for (i = 0; i < NUM_INPUTMETHOD; i++)
        {
            if (strcasecmp(s, Unikey_IMNames[i]) == 0)
            {
                opt->input_method = i;
                break;
            }
        }
    }
    g_free(s);

    // get Output charset
    s = gconf_engine_get_string(e, GCONF_PREFIX CONFIG_OUTPUTCHARSET, NULL);
    if (s != NULL)
    {
        for (i = 0; i < NUM_OUTPUTCHARSET; i++)
        {
            if (strcasecmp(s, Unikey_OCNames[i]) == 0)
            {
                opt->output_charset = i;
                break;
            }
        }
    }
    g_free(s);

    // get Spellcheck
    v = gconf_engine_get(e, GCONF_PREFIX CONFIG_SPELLCHECK, NULL);
    if (v!=NULL)
    {
        opt->enableSpellcheck = gconf_value_get_bool(v);
        gconf_value_free(v);
    }

    // get autoRestoreNonVn
    v = gconf_engine_get(e, GCONF_PREFIX CONFIG_AUTORESTORENONVN, NULL);
    if (v!=NULL)
    {
        opt->autoRestoreNonVn = gconf_value_get_bool(v);
        gconf_value_free(v);
    }

    // get modernStyle
    v = gconf_engine_get(e, GCONF_PREFIX CONFIG_MODERNSTYLE, NULL);
    if (v!=NULL)
    {
        opt->modernStyle = gconf_value_get_bool(v);
        gconf_value_free(v);
    }

    // get freeMarking
    v = gconf_engine_get(e, GCONF_PREFIX CONFIG_FREEMARKING, NULL);
    if (v!=NULL)
    {
        opt->freeMarking = gconf_value_get_bool(v);
        gconf_value_free(v);
    }

    // get enableMacro
    v = gconf_engine_get(e, GCONF_PREFIX CONFIG_MACROENABLED, NULL);
    if (v!=NULL)
    {
        opt->enableMacro = gconf_value_get_bool(v);
        gconf_value_free(v);
    }

    // get ProcessWAtBegin
    v = gconf_engine_get(e, GCONF_PREFIX CONFIG_PROCESSWATBEGIN, NULL);
    if (v!=NULL)
    {
        opt->processwatbegin = gconf_value_get_bool(v);
        gconf_value_free(v);
    }

    // get MouseCapture
    v = gconf_engine_get(e, GCONF_PREFIX CONFIG_MOUSECAPTURE, NULL);
    if (v!=NULL)
    {
        opt->mousecapture = gconf_value_get_bool(v);
        gconf_value_free(v);
    }

    gconf_engine_unref(e);
}

void write_config(UnikeyMainSetupOptions* opt)
{
    GConfEngine* e;
    e = gconf_engine_get_default();

    gconf_engine_set_string(e, GCONF_PREFIX CONFIG_INPUTMETHOD,
                            Unikey_IMNames[opt->input_method], NULL);
    gconf_engine_set_string(e, GCONF_PREFIX CONFIG_OUTPUTCHARSET,
                            Unikey_OCNames[opt->output_charset], NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX CONFIG_SPELLCHECK,
                          opt->enableSpellcheck, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX CONFIG_AUTORESTORENONVN,
                          opt->autoRestoreNonVn, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX CONFIG_MODERNSTYLE,
                          opt->modernStyle, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX CONFIG_FREEMARKING,
                          opt->freeMarking, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX CONFIG_MACROENABLED,
                          opt->enableMacro, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX CONFIG_PROCESSWATBEGIN,
                          opt->processwatbegin, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX CONFIG_MOUSECAPTURE,
                          opt->mousecapture, NULL);

    gconf_engine_unref(e);
}

int force_engine_to_reload_config()
{
    return system("ibus-daemon -xrd");
}

