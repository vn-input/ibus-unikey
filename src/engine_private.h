#ifndef __ENGINE_PRIVATE_H__
#define __ENGINE_PRIVATE_H__

#include <ibus.h>
#include "unikey.h"
#include "vnconv.h"

const gchar*        Unikey_IMNames[] = {"Telex", "Vni"};
const UkInputMethod Unikey_IM[]      = {UkTelex, UkVni};
const unsigned int  NUM_INPUTMETHOD  = sizeof(Unikey_IMNames)/sizeof(Unikey_IMNames[0]);

const gchar*       Unikey_OCNames[]  = {"Unicode",  "TCVN3","VNI Win",  "VIQR"};
const unsigned int Unikey_OC[]       = {CONV_CHARSET_XUTF8,  CONV_CHARSET_TCVN3, CONV_CHARSET_VNIWIN, CONV_CHARSET_VIQR};
const unsigned int NUM_OUTPUTCHARSET = sizeof(Unikey_OCNames)/sizeof(Unikey_OCNames[0]);

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

typedef struct _IBusUnikeyEngine       IBusUnikeyEngine;
typedef struct _IBusUnikeyEngineClass  IBusUnikeyEngineClass;

struct _IBusUnikeyEngine
{
    IBusEngine parent;

/* members */
    IBusPropList* prop_list;
    IBusPropList* menu_im;
    IBusPropList* menu_oc;
    UkInputMethod im; // input method
    unsigned int  oc; // output charset

    gboolean can_commit;
    std::string preeditstr;
};

struct _IBusUnikeyEngineClass
{
    IBusEngineClass parent;
};

// prototype
static void ibus_unikey_engine_class_init(IBusUnikeyEngineClass* kclass);
static void ibus_unikey_engine_init(IBusUnikeyEngine* unikey);

static GObject* ibus_unikey_engine_constructor(GType type,
                                               guint n_construct_params,
                                               GObjectConstructParam* construct_params);

static void ibus_unikey_engine_destroy(IBusUnikeyEngine* unikey);
static gboolean ibus_unikey_engine_process_key_event(IBusEngine* engine,
                                                     guint keyval,
                                                     guint modifiers);

static void ibus_unikey_engine_focus_in(IBusEngine* engine);
static void ibus_unikey_engine_focus_out(IBusEngine* engine);
static void ibus_unikey_engine_reset(IBusEngine* engine);
static void ibus_unikey_engine_enable(IBusEngine* engine);
static void ibus_unikey_engine_disable(IBusEngine* engine);
static void ibus_unikey_engine_property_activate(IBusEngine* engine,
                                                 const gchar* prop_name,
                                                 guint prop_state);

static gboolean ibus_unikey_engine_process_key_event_preedit(IBusEngine* engine,
                                                             guint keyval,
                                                             guint modifiers);

static void ibus_unikey_engine_create_property_list(IBusUnikeyEngine* unikey);

static void ibus_unikey_engine_commit_string(IBusEngine *engine, const gchar *string);
static void ibus_unikey_engine_update_preedit_string(IBusEngine *engine, const gchar *string, gboolean visible);

#endif

