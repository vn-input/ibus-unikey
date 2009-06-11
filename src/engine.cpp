#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <string>
#include <ibus.h>

#include "engine_private.h"
#include "utils.h"
#include "unikey.h"
#include "vnconv.h"

#define _(string) (string)

static IBusEngineClass* parent_class = NULL;
static IBusConfig*      config       = NULL;

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
#ifdef DEBUG
    ibus_warning("class_init()");
#endif

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
#ifdef DEBUG
    ibus_warning("init()");
#endif

    unikey->im = UkTelex;
    unikey->oc = CONV_CHARSET_XUTF8;

    ibus_unikey_engine_create_property_list(unikey);
}

static GObject* ibus_unikey_engine_constructor(GType type,
                                               guint n_construct_params,
                                               GObjectConstructParam* construct_params)
{
#ifdef DEBUG
    ibus_warning("constructor()");
#endif

    IBusUnikeyEngine* unikey;
    const gchar* engine_name;

    unikey = (IBusUnikeyEngine*)
        G_OBJECT_CLASS(parent_class)->constructor(type,
                                                  n_construct_params,
                                                  construct_params);

    engine_name = ibus_engine_get_name((IBusEngine*)unikey);

    return (GObject*)unikey;
}

static void ibus_unikey_engine_destroy(IBusUnikeyEngine* unikey)
{
}

static gboolean ibus_unikey_engine_process_key_event(IBusEngine* engine,
                                                     guint keyval,
                                                     guint modifiers)
{

    return ibus_unikey_engine_process_key_event_preedit(engine, keyval, modifiers);
}

static void ibus_unikey_engine_focus_in(IBusEngine* engine)
{
#ifdef DEBUG
    ibus_warning("focus_in()");
#endif

    IBusUnikeyEngine* unikey = (IBusUnikeyEngine*)engine;

    UnikeySetInputMethod(unikey->im);
    UnikeySetOutputCharset(unikey->oc);

    ibus_engine_register_properties(engine, unikey->prop_list);

    parent_class->focus_in(engine);
}

static void ibus_unikey_engine_focus_out(IBusEngine* engine)
{
#ifdef DEBUG
    ibus_warning("focus_out()");
#endif

    ibus_unikey_engine_reset(engine);

    parent_class->focus_out(engine);
}

static void ibus_unikey_engine_reset(IBusEngine* engine)
{
#ifdef DEBUG
    ibus_warning("reset()");
#endif

    IBusUnikeyEngine *unikey = (IBusUnikeyEngine*)engine;

    UnikeyResetBuf();

    parent_class->reset(engine);
}

static void ibus_unikey_engine_enable(IBusEngine* engine)
{
#ifdef DEBUG
    ibus_warning("enable()");
#endif

    parent_class->enable(engine);
}

static void ibus_unikey_engine_disable(IBusEngine* engine)
{
#ifdef DEBUG
    ibus_warning("disable()");
#endif

    parent_class->disable(engine);
}

static void ibus_unikey_engine_property_activate(IBusEngine* engine,
                                                 const gchar* prop_name,
                                                 guint prop_state)
{
    IBusUnikeyEngine* unikey;
    IBusProperty* prop;
    int i;

    unikey = (IBusUnikeyEngine*)engine;

    // if input method active
    if (strncmp(prop_name, "InputMethod", strlen("InputMethod")) == 0)
    {
        for (i=0; i<NUM_INPUTMETHOD; i++)
        {
            if (strncmp(prop_name+strlen("InputMethod")+1,
                        Unikey_IMNames[i],
                        strlen(Unikey_IMNames[i])) == 0)
            {
                unikey->im = Unikey_IM[i];

                // update property state
                for (int j=0; j<unikey->menu_im->properties->len; j++)
                {
                    prop = ibus_prop_list_get(unikey->menu_im, j);
                    if (prop==NULL)
                        return;
                    else if (strcmp(prop->key, prop_name)==0)
                        prop->state = PROP_STATE_CHECKED;
                    else
                        prop->state = PROP_STATE_UNCHECKED;
                } // end update property state

                break;
            }
        }
    } // end input method active

    // if output charset active
    else if (strncmp(prop_name, "OutputCharset", strlen("OutputCharset")) == 0)
    {
        for (i=0; i<NUM_OUTPUTCHARSET; i++)
        {
            if (strncmp(prop_name+strlen("OutputCharset")+1,
                        Unikey_OCNames[i],
                        strlen(Unikey_OCNames[i])) == 0)
            {
                unikey->oc = Unikey_OC[i];

                // update property state
                for (int j=0; j<unikey->menu_oc->properties->len; j++)
                {
                    prop = ibus_prop_list_get(unikey->menu_oc, j);
                    if (prop==NULL)
                        return;
                    else if (strcmp(prop->key, prop_name)==0)
                        prop->state = PROP_STATE_CHECKED;
                    else
                        prop->state = PROP_STATE_UNCHECKED;
                } // end update property state

                break;
            }
        }
    } // end output charset active

    ibus_unikey_engine_focus_out(engine);
    ibus_unikey_engine_focus_in(engine);
}

static void ibus_unikey_engine_create_property_list(IBusUnikeyEngine* unikey)
{
    IBusProperty* prop;
    IBusText* label,* tooltip;
    gchar name[32];
    int i;

// create property list

// create input method menu
    unikey->menu_im = ibus_prop_list_new();

// add item
    for (i = 0; i < NUM_INPUTMETHOD; i++)
    {
        label = ibus_text_new_from_string(Unikey_IMNames[i]);
        tooltip = ibus_text_new_from_string(""); // ?
        sprintf(name, "InputMethod-%s", Unikey_IMNames[i]);
        prop = ibus_property_new(name,
                                 PROP_TYPE_RADIO,
                                 label,
                                 "",
                                 tooltip,
                                 TRUE,
                                 TRUE,
                                 Unikey_IM[i]==unikey->im?PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,
                                 NULL);
        g_object_unref(label);
        g_object_unref(tooltip);
        ibus_prop_list_append(unikey->menu_im, prop);
    }

// create output charset menu
    unikey->menu_oc = ibus_prop_list_new();

// add item
    for (i = 0; i < NUM_OUTPUTCHARSET; i++)
    {
        label = ibus_text_new_from_string(Unikey_OCNames[i]);
        tooltip = ibus_text_new_from_string(""); // ?
        sprintf(name, "OutputCharset-%s", Unikey_OCNames[i]);
        prop = ibus_property_new(name,
                                 PROP_TYPE_RADIO,
                                 label,
                                 "",
                                 tooltip,
                                 TRUE,
                                 TRUE,
                                 Unikey_OC[i]==unikey->oc?PROP_STATE_CHECKED:PROP_STATE_UNCHECKED,
                                 NULL);
        g_object_unref(label);
        g_object_unref(tooltip);
        ibus_prop_list_append(unikey->menu_oc, prop);
    }
/*
// create misc menu
menu_misc = ibus_prop_list_new();

// add item
// - Spell check
label = ibus_text_new_from_string(_("Enable spell check"));
tooltip = ibus_text_new_from_string(_("If enable, you can decrease mistake when typing"));
prop = ibus_property_new("Spellcheck",
PROP_TYPE_TOGGLE,
label,
"gtk-preferences",
tooltip,
TRUE,
TRUE,
PROP_STATE_UNCHECKED,
NULL);
g_object_unref(label);
g_object_unref(tooltip);

ibus_prop_list_append(menu_misc, prop);
*/

// create top menu
    unikey->prop_list = ibus_prop_list_new();

// add item
// - add input method menu
    label = ibus_text_new_from_string(_("Input method"));
    tooltip = ibus_text_new_from_string(_("Choose input method"));
    prop = ibus_property_new("InputMethod",
                             PROP_TYPE_MENU,
                             label,
                             "add",
                             tooltip,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             unikey->menu_im);
    g_object_unref(label);
    g_object_unref(tooltip);

    ibus_prop_list_append(unikey->prop_list, prop);

// - add output charset menu
    label = ibus_text_new_from_string(_("Output charset"));
    tooltip = ibus_text_new_from_string(_("Choose output charset"));
    prop = ibus_property_new("OutputCharset",
                             PROP_TYPE_MENU,
                             label,
                             "cancel",
                             tooltip,
                             TRUE,
                             TRUE,
                             PROP_STATE_UNCHECKED,
                             unikey->menu_oc);
    g_object_unref(label);
    g_object_unref(tooltip);

    ibus_prop_list_append(unikey->prop_list, prop);

/*
// - add misc menu
label = ibus_text_new_from_string(_("Misc options"));
tooltip = ibus_text_new_from_string(_(""));
prop = ibus_property_new("MiscOptions",
PROP_TYPE_MENU,
label,
"gtk-preferences",
tooltip,
TRUE,
TRUE,
PROP_STATE_UNCHECKED,
menu_misc);
g_object_unref(label);
g_object_unref(tooltip);

ibus_prop_list_append(unikey->prop_list, prop);
*/
}

static void ibus_unikey_engine_commit_string(IBusEngine *engine, const gchar *string)
{
    IBusText *text;
    text = ibus_text_new_from_static_string(string);
    ibus_engine_commit_text(engine, text);
    g_object_unref(text);
}

static void ibus_unikey_engine_update_preedit_string(IBusEngine *engine, const gchar *string, gboolean visible)
{
    IBusText *text;
    text = ibus_text_new_from_static_string(string);
    ibus_engine_update_preedit_text(engine, text, strlen(string), visible);
    g_object_unref(text);
}

static gboolean ibus_unikey_engine_process_key_event_preedit(IBusEngine* engine,
                                                             guint keyval,
                                                             guint modifiers)
{
    IBusUnikeyEngine* unikey;

    unikey = (IBusUnikeyEngine*)engine;

    if (modifiers & IBUS_RELEASE_MASK)
        return false;

    // non process key
    ibus_unikey_engine_reset(engine);
    return false;
}

