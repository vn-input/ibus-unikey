#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <libintl.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <string.h>
#include <ibus.h>

#include "engine_const.h"
#include "engine_private.h"
#include "utils.h"
#include "unikey.h"
#include "vnconv.h"

#if !IBUS_CHECK_VERSION(1,2,99) // ibus below version 1.2.99 have problem with PROP_TYPE_NORMAL, use RADIO instead
#define PROP_TYPE_NORMAL PROP_TYPE_RADIO
#endif

#define _(string) gettext(string)

#define CONVERT_BUF_SIZE 1024

static unsigned char WordBreakSyms[] =
{
    ',', ';', ':', '.', '\"', '\'', '!', '?', ' ',
    '<', '>', '=', '+', '-', '*', '/', '\\',
    '_', '~', '`', '@', '#', '$', '%', '^', '&', '(', ')', '{', '}', '[', ']',
    '|'
};

static unsigned char WordAutoCommit[] =
{
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'b', 'c', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'n',
    'p', 'q', 'r', 's', 't', 'v', 'x', 'z',
    'B', 'C', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N',
    'P', 'Q', 'R', 'S', 'T', 'V', 'X', 'Z'
};

static IBusEngineClass* parent_class = NULL;
static IBusConfig*      config       = NULL;

static IBusUnikeyEngine* unikey; // current (focus) unikey engine

GType ibus_unikey_engine_get_type(void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof(IBusUnikeyEngineClass),
        (GBaseInitFunc)NULL,
        (GBaseFinalizeFunc)NULL,
        (GClassInitFunc)ibus_unikey_engine_class_init,
        NULL,
        NULL,
        sizeof(IBusUnikeyEngine),
        0,
        (GInstanceInitFunc)ibus_unikey_engine_init,
    };

    if (type == 0)
    {
        type = g_type_register_static(IBUS_TYPE_ENGINE,
                                      "IBusUnikeyEngine",
                                      &type_info,
                                      (GTypeFlags)0);
    }

    return type;
}

void ibus_unikey_init(IBusBus* bus)
{
    UnikeySetup();
    config = ibus_bus_get_config(bus);
}

void ibus_unikey_exit()
{
    UnikeyCleanup();
}

static void ibus_unikey_engine_class_init(IBusUnikeyEngineClass* klass)
{
    GObjectClass* object_class         = G_OBJECT_CLASS(klass);
    IBusObjectClass* ibus_object_class = IBUS_OBJECT_CLASS(klass);
    IBusEngineClass* engine_class      = IBUS_ENGINE_CLASS(klass);

    parent_class = (IBusEngineClass* )g_type_class_peek_parent(klass);

    object_class->constructor = ibus_unikey_engine_constructor;
    ibus_object_class->destroy = (IBusObjectDestroyFunc)ibus_unikey_engine_destroy;

    engine_class->process_key_event = ibus_unikey_engine_process_key_event;
    engine_class->reset             = ibus_unikey_engine_reset;
    engine_class->enable            = ibus_unikey_engine_enable;
    engine_class->disable           = ibus_unikey_engine_disable;
    engine_class->focus_in          = ibus_unikey_engine_focus_in;
    engine_class->focus_out         = ibus_unikey_engine_focus_out;
    engine_class->property_activate = ibus_unikey_engine_property_activate;
}

static void ibus_unikey_engine_init(IBusUnikeyEngine* unikey)
{
    ibus_unikey_engine_load_config(unikey);

    unikey->preeditstr = new std::string();
    ibus_unikey_engine_create_property_list(unikey);
}

static void ibus_unikey_engine_load_config(IBusUnikeyEngine* unikey)
{
    gchar* str;
    gboolean b;
    guint i;

//set default options
    unikey->im = Unikey_IM[0];
    unikey->oc = Unikey_OC[0];
    unikey->ukopt.spellCheckEnabled     = DEFAULT_CONF_SPELLCHECK;
    unikey->ukopt.autoNonVnRestore      = DEFAULT_CONF_AUTONONVNRESTORE;
    unikey->ukopt.modernStyle           = DEFAULT_CONF_MODERNSTYLE;
    unikey->ukopt.freeMarking           = DEFAULT_CONF_FREEMARKING;
    unikey->ukopt.macroEnabled          = DEFAULT_CONF_MACROENABLED;
    unikey->process_w_at_begin          = DEFAULT_CONF_PROCESSWATBEGIN;

    if (ibus_unikey_config_get_string(config, CONFIG_SECTION, CONFIG_INPUTMETHOD, &str))
    {
        for (i = 0; i < NUM_INPUTMETHOD; i++)
        {
            if (strcasecmp(str, Unikey_IMNames[i]) == 0)
            {
                unikey->im = Unikey_IM[i];
                break;
            }
        }
    }

    if (ibus_unikey_config_get_string(config, CONFIG_SECTION, CONFIG_OUTPUTCHARSET, &str))
    {
        for (i = 0; i < NUM_OUTPUTCHARSET; i++)
        {
            if (strcasecmp(str, Unikey_OCNames[i]) == 0)
            {
                unikey->oc = Unikey_OC[i];
                break;
            }
        }
    }

    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_FREEMARKING, &b))
        unikey->ukopt.freeMarking = b;

    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_MODERNSTYLE, &b))
        unikey->ukopt.modernStyle = b;

    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_MACROENABLED, &b))
        unikey->ukopt.macroEnabled = b;

    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_SPELLCHECK, &b))
        unikey->ukopt.spellCheckEnabled = b;

    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_AUTORESTORENONVN, &b))
        unikey->ukopt.autoNonVnRestore = b;

    if (ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_PROCESSWATBEGIN, &b))
        unikey->process_w_at_begin = b;

    // load macro
    gchar* fn = get_macro_file();
    UnikeyLoadMacroTable(fn);
    g_free(fn);
}

static GObject* ibus_unikey_engine_constructor(GType type,
                                               guint n_construct_params,
                                               GObjectConstructParam* construct_params)
{
    IBusUnikeyEngine* unikey;

    unikey = (IBusUnikeyEngine*)
        G_OBJECT_CLASS(parent_class)->constructor(type,
                                                  n_construct_params,
                                                  construct_params);

    return (GObject*)unikey;
}

static void ibus_unikey_engine_destroy(IBusUnikeyEngine* unikey)
{
    delete unikey->preeditstr;
    g_object_unref(unikey->prop_list);

    IBUS_OBJECT_CLASS(parent_class)->destroy((IBusObject*)unikey);
}

static void ibus_unikey_engine_focus_in(IBusEngine* engine)
{
    unikey = (IBusUnikeyEngine*)engine;

    UnikeySetInputMethod(unikey->im);
    UnikeySetOutputCharset(unikey->oc);

    UnikeySetOptions(&unikey->ukopt);
    ibus_engine_register_properties(engine, unikey->prop_list);

    parent_class->focus_in(engine);
}

static void ibus_unikey_engine_clean_buffer(IBusEngine* engine) {
    UnikeyResetBuf();
    unikey->preeditstr->clear();
    ibus_engine_hide_preedit_text(engine);    
}

static void ibus_unikey_engine_commit(IBusEngine* engine) {
    unikey = (IBusUnikeyEngine*)engine;

    if (unikey->preeditstr->length() > 0)
    {
        IBusText *text;

        text = ibus_text_new_from_static_string(unikey->preeditstr->c_str());
        ibus_engine_commit_text(engine, text);        
    }
    
    ibus_unikey_engine_clean_buffer(engine);        
}

static void ibus_unikey_engine_focus_out(IBusEngine* engine)
{
    // remove buffer since we lost focus
    // cannot commit here because the text is out of focus
    ibus_unikey_engine_clean_buffer(engine);

    parent_class->focus_out(engine);
}

static void ibus_unikey_engine_reset(IBusEngine* engine)
{
    ibus_unikey_engine_commit(engine);

    parent_class->reset(engine);
}

static void ibus_unikey_engine_enable(IBusEngine* engine)
{
    parent_class->enable(engine);
}

static void ibus_unikey_engine_disable(IBusEngine* engine)
{
    parent_class->disable(engine);
}

static void ibus_unikey_engine_property_activate(IBusEngine* engine,
                                                 const gchar* prop_name,
                                                 guint prop_state)
{
    IBusProperty* prop;
    IBusText* label;
    guint i, j;

    unikey = (IBusUnikeyEngine*)engine;

    // input method active
    if (strncmp(prop_name, CONFIG_INPUTMETHOD, strlen(CONFIG_INPUTMETHOD)) == 0)
    {
        for (i=0; i<NUM_INPUTMETHOD; i++)
        {
            if (strcmp(prop_name + strlen(CONFIG_INPUTMETHOD)+1,
                       Unikey_IMNames[i]) == 0)
            {
                unikey->im = Unikey_IM[i];

                // update label
                for (j=0; j<unikey->prop_list->properties->len; j++)
                {
                    prop = ibus_prop_list_get(unikey->prop_list, j);
                    if (prop==NULL)
                        return;
                    else if (strcmp(ibus_property_get_key(prop), CONFIG_INPUTMETHOD) == 0)
                    {
                        label = ibus_text_new_from_static_string(Unikey_IMNames[i]);
                        ibus_property_set_label(prop, label);
                        break;
                    }
                } // end update label

                // update property state
                for (j=0; j<unikey->menu_im->properties->len; j++)
                {
                    prop = ibus_prop_list_get(unikey->menu_im, j);
                    if (prop==NULL)
                        return;
                    else if (strcmp(ibus_property_get_key(prop), prop_name)==0)
                        ibus_property_set_state(prop, PROP_STATE_CHECKED);
                    else
                        ibus_property_set_state(prop, PROP_STATE_UNCHECKED);
                } // end update property state

                break;
            }
        }
    } // end input method active

    // output charset active
    else if (strncmp(prop_name, CONFIG_OUTPUTCHARSET, strlen(CONFIG_OUTPUTCHARSET)) == 0)
    {
        for (i=0; i<NUM_OUTPUTCHARSET; i++)
        {
            if (strcmp(prop_name+strlen(CONFIG_OUTPUTCHARSET)+1,
                       Unikey_OCNames[i]) == 0)
            {
                unikey->oc = Unikey_OC[i];

                // update label
                for (j=0; j<unikey->prop_list->properties->len; j++)
                {
                    prop = ibus_prop_list_get(unikey->prop_list, j);
                    if (prop==NULL)
                        return;
                    else if (strcmp(ibus_property_get_key(prop), CONFIG_OUTPUTCHARSET)==0)
                    {
                        label = ibus_text_new_from_static_string(Unikey_OCNames[i]);
                        ibus_property_set_label(prop, label);
                        break;
                    }
                } // end update label

                // update property state
                for (j=0; j<unikey->menu_oc->properties->len; j++)
                {
                    prop = ibus_prop_list_get(unikey->menu_oc, j);
                    if (prop==NULL)
                        return;
                    else if (strcmp(ibus_property_get_key(prop), prop_name) == 0)
                        ibus_property_set_state(prop, PROP_STATE_CHECKED);
                    else
                        ibus_property_set_state(prop, PROP_STATE_UNCHECKED);
                } // end update property state

                break;
            }
        }
    } // end output charset active

    // spellcheck active
    else if (strcmp(prop_name, CONFIG_SPELLCHECK) == 0)
    {
        unikey->ukopt.spellCheckEnabled = !unikey->ukopt.spellCheckEnabled;

        // update state
        for (j = 0; j < unikey->menu_opt->properties->len ; j++)
        {
            prop = ibus_prop_list_get(unikey->menu_opt, j);
            if (prop == NULL)
                return;

            else if (strcmp(ibus_property_get_key(prop), CONFIG_SPELLCHECK) == 0)
            {
                ibus_property_set_state(prop, (unikey->ukopt.spellCheckEnabled == 1)?
                    PROP_STATE_CHECKED:PROP_STATE_UNCHECKED);
                break;
            }
        } // end update state
    } // end spellcheck active

    // MacroEnabled active
    else if (strcmp(prop_name, CONFIG_MACROENABLED) == 0)
    {
        unikey->ukopt.macroEnabled = !unikey->ukopt.macroEnabled;

        // update state
        for (j = 0; j < unikey->menu_opt->properties->len ; j++)
        {
            prop = ibus_prop_list_get(unikey->menu_opt, j);
            if (prop == NULL)
                return;

            else if (strcmp(ibus_property_get_key(prop), CONFIG_MACROENABLED) == 0)
            {
                ibus_property_set_state(prop, (unikey->ukopt.macroEnabled == 1)?
                    PROP_STATE_CHECKED:PROP_STATE_UNCHECKED);
                break;
            }
        } // end update state
    } // end MacroEnabled active
    // if Run setup
    else if (strcmp(prop_name, "RunSetupGUI") == 0)
    {
        system(LIBEXECDIR "/ibus-setup-unikey &");
    } // END Run setup

    UnikeySetInputMethod(unikey->im);
    UnikeySetOutputCharset(unikey->oc);
    UnikeySetOptions(&unikey->ukopt);
}

static void ibus_unikey_engine_create_property_list(IBusUnikeyEngine* unikey)
{
    IBusProperty* prop;
    IBusText* label,* tooltip;
    gchar name[32];
    guint i;

    if (unikey->prop_list == NULL)
    {
        unikey->prop_list = ibus_prop_list_new();
        unikey->menu_im   = ibus_prop_list_new();
        unikey->menu_oc   = ibus_prop_list_new();
        unikey->menu_opt  = ibus_prop_list_new();

        g_object_ref_sink(unikey->prop_list);
    }

// create input method menu
    // add item
    for (i = 0; i < NUM_INPUTMETHOD; i++)
    {
        label = ibus_text_new_from_static_string(Unikey_IMNames[i]);
        tooltip = ibus_text_new_from_static_string(""); // ?
        sprintf(name, CONFIG_INPUTMETHOD"_%s", Unikey_IMNames[i]);
        prop = ibus_property_new(name,
                                 PROP_TYPE_RADIO,
                                 label,
                                 "",
                                 tooltip,
                                 TRUE,
                                 TRUE,
                                 Unikey_IM[i]==unikey->im?PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,
                                 NULL);

        if (ibus_prop_list_update_property(unikey->menu_im, prop) == false)
            ibus_prop_list_append(unikey->menu_im, prop);
    }
// END create input method menu

// create output charset menu
    // add item
    for (i = 0; i < NUM_OUTPUTCHARSET; i++)
    {
        label = ibus_text_new_from_static_string(Unikey_OCNames[i]);
        tooltip = ibus_text_new_from_static_string(""); // ?
        sprintf(name, CONFIG_OUTPUTCHARSET"_%s", Unikey_OCNames[i]);
        prop = ibus_property_new(name,
                                 PROP_TYPE_RADIO,
                                 label,
                                 "",
                                 tooltip,
                                 TRUE,
                                 TRUE,
                                 Unikey_OC[i]==unikey->oc?PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,
                                 NULL);

        if (ibus_prop_list_update_property(unikey->menu_oc, prop) == false)
            ibus_prop_list_append(unikey->menu_oc, prop);
    }
// END create output charset menu

// create option menu (for configure unikey)
    // add option property

    // --create and add spellcheck property
    label = ibus_text_new_from_static_string(_("Enable spell check"));
    tooltip = ibus_text_new_from_static_string(_("If enable, you can decrease mistake when typing"));
    prop = ibus_property_new(CONFIG_SPELLCHECK,
                             PROP_TYPE_TOGGLE,
                             label,
                             "",
                             tooltip,
                             TRUE,
                             TRUE,
                             (unikey->ukopt.spellCheckEnabled==1)?
                             PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,
                             NULL);

    if (ibus_prop_list_update_property(unikey->menu_opt, prop) == false)
        ibus_prop_list_append(unikey->menu_opt, prop);

    // --create and add macroEnabled property
    label = ibus_text_new_from_static_string(_("Enable Macro"));
    tooltip = ibus_text_new_from_static_string("");
    prop = ibus_property_new(CONFIG_MACROENABLED,
                             PROP_TYPE_TOGGLE,
                             label,
                             "",
                             tooltip,
                             TRUE,
                             TRUE,
                             (unikey->ukopt.macroEnabled==1)?
                             PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,
                             NULL);

    if (ibus_prop_list_update_property(unikey->menu_opt, prop) == false)
        ibus_prop_list_append(unikey->menu_opt, prop);

    // --separator
    prop = ibus_property_new("", PROP_TYPE_SEPARATOR,
                             NULL, "", NULL, TRUE, TRUE,
                             PROP_STATE_UNCHECKED, NULL);
    if (ibus_prop_list_update_property(unikey->menu_opt, prop) == false)
        ibus_prop_list_append(unikey->menu_opt, prop);

    // --create and add Launch Setup GUI property
    label = ibus_text_new_from_static_string(_("Full setup..."));
    tooltip = ibus_text_new_from_static_string(_("Full setup utility for IBus-Unikey"));
    prop = ibus_property_new("RunSetupGUI",
                             PROP_TYPE_NORMAL,
                             label,
                             "",
                             tooltip,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             NULL);

    if (ibus_prop_list_update_property(unikey->menu_opt, prop) == false)
        ibus_prop_list_append(unikey->menu_opt, prop);
// END create option menu

// create top menu
    // add item
    // -- add input method menu
    for (i = 0; i < NUM_INPUTMETHOD; i++)
    {
        if (Unikey_IM[i] == unikey->im)
            break;
    }
    label = ibus_text_new_from_static_string(Unikey_IMNames[i]);
    tooltip = ibus_text_new_from_static_string(_("Choose input method"));
    prop = ibus_property_new(CONFIG_INPUTMETHOD,
                             PROP_TYPE_MENU,
                             label,
                             "",
                             tooltip,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             unikey->menu_im);

    if (ibus_prop_list_update_property(unikey->prop_list, prop) == false)
        ibus_prop_list_append(unikey->prop_list, prop);

    // -- add output charset menu
    for (i = 0; i < NUM_OUTPUTCHARSET; i++)
    {
        if (Unikey_OC[i] == unikey->oc)
            break;
    }
    label = ibus_text_new_from_static_string(Unikey_OCNames[i]);
    tooltip = ibus_text_new_from_static_string(_("Choose output charset"));
    prop = ibus_property_new(CONFIG_OUTPUTCHARSET,
                             PROP_TYPE_MENU,
                             label,
                             "",
                             tooltip,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             unikey->menu_oc);

    if (ibus_prop_list_update_property(unikey->prop_list, prop) == false)
        ibus_prop_list_append(unikey->prop_list, prop);

    // -- add option menu
    label = ibus_text_new_from_static_string(_("Options"));
    tooltip = ibus_text_new_from_static_string(_("Options for Unikey"));
    prop = ibus_property_new("Options",
                             PROP_TYPE_MENU,
                             label,
                             "gtk-preferences",
                             tooltip,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             unikey->menu_opt);

    if (ibus_prop_list_update_property(unikey->prop_list, prop) == false)
        ibus_prop_list_append(unikey->prop_list, prop);
// end top menu
}

static void ibus_unikey_engine_update_preedit_string(IBusEngine *engine, const gchar *string, gboolean visible)
{
    IBusText *text;

    text = ibus_text_new_from_static_string(string);

    // underline text
    ibus_text_append_attribute(text, IBUS_ATTR_TYPE_UNDERLINE, IBUS_ATTR_UNDERLINE_SINGLE, 0, -1);

    // update and display text
    ibus_engine_update_preedit_text_with_mode(engine, text, ibus_text_get_length(text), visible, IBUS_ENGINE_PREEDIT_COMMIT);
}

static void ibus_unikey_engine_erase_chars(IBusEngine *engine, int num_chars)
{
    int i, k;
    guchar c;

    k = num_chars;

    for ( i = unikey->preeditstr->length()-1; i >= 0 && k > 0; i--)
    {
        c = unikey->preeditstr->at(i);

        // count down if byte is begin byte of utf-8 char
        if (c < (guchar)'\x80' || c >= (guchar)'\xC0')
        {
            k--;
        }
    }

    unikey->preeditstr->erase(i+1);
}

static gboolean ibus_unikey_engine_process_key_event(IBusEngine* engine,
                                                     guint keyval,
                                                     guint keycode,
                                                     guint modifiers)
{
    static gboolean tmp;

    unikey = (IBusUnikeyEngine*)engine;

    tmp = ibus_unikey_engine_process_key_event_preedit(engine, keyval, keycode, modifiers);

    // check last keyevent with shift
    if (keyval >= IBUS_space && keyval <=IBUS_asciitilde)
    {
        unikey->last_key_with_shift = modifiers & IBUS_SHIFT_MASK;
    }
    else
    {
        unikey->last_key_with_shift = false;
    } // end check last keyevent with shift

    return tmp;
}

static gboolean ibus_unikey_engine_process_key_event_preedit(IBusEngine* engine,
                                                             guint keyval,
                                                             guint keycode,
                                                             guint modifiers)
{
    if (modifiers & IBUS_RELEASE_MASK)
    {
        return false;
    }
    //special key, hot key that we won't handle
    else if (modifiers & IBUS_CONTROL_MASK
             || modifiers & IBUS_MOD1_MASK // alternate mask
             || keyval == IBUS_Control_L
             || keyval == IBUS_Control_R
             || keyval == IBUS_Tab
             || keyval == IBUS_Return
             || keyval == IBUS_Delete
             || keyval == IBUS_KP_Enter
             || (keyval >= IBUS_Home && keyval <= IBUS_Insert)
             || (keyval >= IBUS_KP_Home && keyval <= IBUS_KP_Delete)
        )
    {
        ibus_unikey_engine_commit(engine);
        return false;
    }
    else if ((keyval >= IBUS_Caps_Lock && keyval <= IBUS_Hyper_R)
            || (!(modifiers & IBUS_SHIFT_MASK) && (keyval == IBUS_Shift_L || keyval == IBUS_Shift_R))  // when press one shift key
        )
    {
        return false;
    }
    else if (keyval >=IBUS_KP_Multiply && keyval <=IBUS_KP_9)
    {
        ibus_unikey_engine_commit(engine);
        return false;
    }
    // capture BackSpace
    else if (keyval == IBUS_BackSpace)
    {
        UnikeyBackspacePress();

        if (UnikeyBackspaces == 0 || unikey->preeditstr->empty())
        {
            return false;
        }
        else
        {
            if (unikey->preeditstr->length() <= (guint)UnikeyBackspaces)
            {
                unikey->preeditstr->clear();
                ibus_engine_hide_preedit_text(engine);
            }
            else
            {
                ibus_unikey_engine_erase_chars(engine, UnikeyBackspaces);
                ibus_unikey_engine_update_preedit_string(engine, unikey->preeditstr->c_str(), true);
            }

            // change tone position after press backspace
            if (UnikeyBufChars > 0)
            {
                if (unikey->oc == CONV_CHARSET_XUTF8)
                {
                    unikey->preeditstr->append((const gchar*)UnikeyBuf, UnikeyBufChars);
                }
                else
                {
                    static unsigned char buf[CONVERT_BUF_SIZE];
                    int bufSize = CONVERT_BUF_SIZE;

                    latinToUtf(buf, UnikeyBuf, UnikeyBufChars, &bufSize);
                    unikey->preeditstr->append((const gchar*)buf, CONVERT_BUF_SIZE - bufSize);
                }

                ibus_unikey_engine_update_preedit_string(engine, unikey->preeditstr->c_str(), true);
            }
        }
        return true;
    } // end capture BackSpace
    // capture ascii printable char
    else if ((keyval >= IBUS_space && keyval <= IBUS_asciitilde)
            || keyval == IBUS_Shift_L || keyval == IBUS_Shift_R) // sure this have IBUS_SHIFT_MASK
    {
        static guint i;

        UnikeySetCapsState(modifiers & IBUS_SHIFT_MASK, modifiers & IBUS_LOCK_MASK);

        // process keyval
        if ((unikey->im == UkTelex || unikey->im == UkSimpleTelex2)
            && unikey->process_w_at_begin == false
            && UnikeyAtWordBeginning()
            && (keyval == IBUS_w || keyval == IBUS_W))
        {
            UnikeyPutChar(keyval);
            if (unikey->ukopt.macroEnabled == 0)
            {
                return false;
            }
            else
            {
                unikey->preeditstr->append(keyval==IBUS_w?"w":"W");
                ibus_unikey_engine_update_preedit_string(engine, unikey->preeditstr->c_str(), true);
                return true;
            }
        }
        // shift + space, shift + shift event
        if ((unikey->last_key_with_shift == false && modifiers & IBUS_SHIFT_MASK
                    && keyval == IBUS_space && !UnikeyAtWordBeginning())
            || (keyval == IBUS_Shift_L || keyval == IBUS_Shift_R) // (&& modifiers & IBUS_SHIFT_MASK), sure this have IBUS_SHIFT_MASK
           )
        {
            UnikeyRestoreKeyStrokes();
        } // end shift + space, shift + shift event
        else
        {
            UnikeyFilter(keyval);
        }
        // end process keyval

        // process result of ukengine
        if (UnikeyBackspaces > 0)
        {
            if (unikey->preeditstr->length() <= (guint)UnikeyBackspaces)
            {
                unikey->preeditstr->clear();
            }
            else
            {
                ibus_unikey_engine_erase_chars(engine, UnikeyBackspaces);
            }
        }

        if (UnikeyBufChars > 0)
        {
            if (unikey->oc == CONV_CHARSET_XUTF8)
            {
                unikey->preeditstr->append((const gchar*)UnikeyBuf, UnikeyBufChars);
            }
            else
            {
                static unsigned char buf[CONVERT_BUF_SIZE];
                int bufSize = CONVERT_BUF_SIZE;

                latinToUtf(buf, UnikeyBuf, UnikeyBufChars, &bufSize);
                unikey->preeditstr->append((const gchar*)buf, CONVERT_BUF_SIZE - bufSize);
            }
        }
        else if (keyval != IBUS_Shift_L && keyval != IBUS_Shift_R) // if ukengine not process
        {
            static int n;
            static char s[6];

            n = g_unichar_to_utf8(keyval, s); // convert ucs4 to utf8 char
            unikey->preeditstr->append(s, n);
        }
        // end process result of ukengine

        // commit string: if need
        if (unikey->preeditstr->length() > 0)
        {
            static guint i;
            for (i = 0; i < sizeof(WordBreakSyms); i++)
            {
                if (WordBreakSyms[i] == unikey->preeditstr->at(unikey->preeditstr->length()-1)
                    && WordBreakSyms[i] == keyval)
                {
                    ibus_unikey_engine_commit(engine);
                    return true;
                }
            }
        }
        // end commit string

        ibus_unikey_engine_update_preedit_string(engine, unikey->preeditstr->c_str(), true);
        return true;
    } //end capture printable char

    // non process key
    ibus_unikey_engine_commit(engine);
    return false;
}