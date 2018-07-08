#ifndef __ENGINE_PRIVATE_H__
#define __ENGINE_PRIVATE_H__

#include <string>
#include <ibus.h>
#include "unikey.h"
#include "vnconv.h"

typedef struct _IBusUnikeyEngine       IBusUnikeyEngine;
typedef struct _IBusUnikeyEngineClass  IBusUnikeyEngineClass;

struct _IBusUnikeyEngine
{
    IBusEngine parent;

/* members */
    IBusPropList* prop_list;

    UkInputMethod im; // input method
    unsigned int  oc; // output charset
    UnikeyOptions ukopt;
    gboolean process_w_at_begin;

    gboolean last_key_with_shift;

    std::string* preeditstr;
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
                                                     guint keycode,
                                                     guint modifiers);

static void ibus_unikey_engine_focus_in(IBusEngine* engine);
static void ibus_unikey_engine_focus_out(IBusEngine* engine);
static void ibus_unikey_engine_reset(IBusEngine* engine);
static void ibus_unikey_engine_enable(IBusEngine* engine);
static void ibus_unikey_engine_disable(IBusEngine* engine);
static void ibus_unikey_engine_load_config(IBusUnikeyEngine* unikey);
static void ibus_unikey_config_value_changed(gchar* name, gpointer user_data);
static void ibus_unikey_engine_property_activate(IBusEngine* engine,
                                                 const gchar* prop_name,
                                                 guint prop_state);

static gboolean ibus_unikey_engine_process_key_event_preedit(IBusEngine* engine,
                                                             guint keyval,
                                                             guint keycode,
                                                             guint modifiers);

static void ibus_unikey_engine_create_property_list(IBusUnikeyEngine* unikey);

static void ibus_unikey_engine_update_preedit_string(IBusEngine *engine, const gchar *string, gboolean visible);
static void ibus_unikey_engine_erase_chars(IBusEngine *engine, int num_chars);

static int latinToUtf(unsigned char* dst, unsigned char* src, int inSize, int* pOutSize);

#endif

