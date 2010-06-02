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

    opt->enableSpellcheck = UNIKEY_OPT_SPELLCHECKENABLED;
    opt->autoRestoreNonVn = UNIKEY_OPT_AUTONONVNRESTORE;
    opt->modernStyle = UNIKEY_OPT_MODERNSTYLE;
    opt->freeMarking = UNIKEY_OPT_FREEMARKING;
    opt->enableMacro = UNIKEY_OPT_MACROENABLED;

    opt->processwatbegin = UNIKEY_OPT_PROCESSWATBEGIN;

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
    s = gconf_engine_get_string(e, GCONF_PREFIX UNIKEY_INPUTMETHOD, NULL);
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
    // END get Input method

    // get Output charset
    s = gconf_engine_get_string(e, GCONF_PREFIX UNIKEY_OUTPUTCHARSET, NULL);
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
    // END get Output charset

    // get Spellcheck
    v = gconf_engine_get(e, GCONF_PREFIX UNIKEY_OPTION_SPELLCHECK, NULL);
    if (v!=NULL)
    {
        opt->enableSpellcheck = gconf_value_get_bool(v);
        gconf_value_free(v);
    }
    // END get Spellcheck

    // get autoRestoreNonVn
    v = gconf_engine_get(e, GCONF_PREFIX UNIKEY_OPTION_AUTONONVNRESTORE, NULL);
    if (v!=NULL)
    {
        opt->autoRestoreNonVn = gconf_value_get_bool(v);
        gconf_value_free(v);
    }
    // END get autoRestoreNonVn

    // get modernStyle
    v = gconf_engine_get(e, GCONF_PREFIX UNIKEY_OPTION_MODERNSTYLE, NULL);
    if (v!=NULL)
    {
        opt->modernStyle = gconf_value_get_bool(v);
        gconf_value_free(v);
    }
    // END get modernStyle

    // get freeMarking
    v = gconf_engine_get(e, GCONF_PREFIX UNIKEY_OPTION_FREEMARKING, NULL);
    if (v!=NULL)
    {
        opt->freeMarking = gconf_value_get_bool(v);
        gconf_value_free(v);
    }
    // END get freeMarking

    // get enableMacro
    v = gconf_engine_get(e, GCONF_PREFIX UNIKEY_OPTION_MACROENABLED, NULL);
    if (v!=NULL)
    {
        opt->enableMacro = gconf_value_get_bool(v);
        gconf_value_free(v);
    }
    // END get enableMacro

    // get ProcessWAtBegin
    v = gconf_engine_get(e, GCONF_PREFIX UNIKEY_OPTION_PROCESSWATBEGIN, NULL);
    if (v!=NULL)
    {
        opt->processwatbegin = gconf_value_get_bool(v);
        gconf_value_free(v);
    }
    // END get ProcessWAtBegin

    gconf_engine_unref(e);
}

void write_config(UnikeyMainSetupOptions* opt)
{
    GConfEngine* e;
    e = gconf_engine_get_default();

    gconf_engine_set_string(e, GCONF_PREFIX UNIKEY_INPUTMETHOD,
                            Unikey_IMNames[opt->input_method], NULL);
    gconf_engine_set_string(e, GCONF_PREFIX UNIKEY_OUTPUTCHARSET,
                            Unikey_OCNames[opt->output_charset], NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX UNIKEY_OPTION_SPELLCHECK,
                          opt->enableSpellcheck, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX UNIKEY_OPTION_AUTONONVNRESTORE,
                          opt->autoRestoreNonVn, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX UNIKEY_OPTION_MODERNSTYLE,
                          opt->modernStyle, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX UNIKEY_OPTION_FREEMARKING,
                          opt->freeMarking, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX UNIKEY_OPTION_MACROENABLED,
                          opt->enableMacro, NULL);
    gconf_engine_set_bool(e, GCONF_PREFIX UNIKEY_OPTION_PROCESSWATBEGIN,
                          opt->processwatbegin, NULL);

    gconf_engine_unref(e);
}

int force_engine_to_reload_config()
{
    return system("ibus-daemon --replace --daemonize");
}

