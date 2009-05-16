#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <ibus.h>
#include <stdio.h>
#include "engine.h"
#include "unikey.h"
#include "vnconv.h"


const gchar          *Unikey_IMNames[]    = {"Telex", "Vni"};
const UkInputMethod   Unikey_IM[]         = {UkTelex, UkVni};
const unsigned int    NUM_INPUTMETHOD     = sizeof(Unikey_IMNames)/sizeof(Unikey_IMNames[0]);

const gchar          *Unikey_OCNames[]    = {"Unicode",           "TCVN3",            "VNI Win",              "VIQR"};
const unsigned int    Unikey_OC[]         = {CONV_CHARSET_XUTF8,  CONV_CHARSET_TCVN3, CONV_CHARSET_VNIWIN,    CONV_CHARSET_VIQR};
const unsigned int    NUM_OUTPUTCHARSET   = sizeof(Unikey_OCNames)/sizeof(Unikey_OCNames[0]);

typedef struct _IBusUnikeyEngine IBusUnikeyEngine;
typedef struct _IBusUnikeyEngineClass IBusUnikeyEngineClass;

struct _IBusUnikeyEngine
{
	IBusEngine parent;

	/* members */
	//VInputContext   *context;
	IBusProperty    *status_prop;
	IBusPropList    *prop_list;
};

struct _IBusUnikeyEngineClass {
	IBusEngineClass parent;
};

// prototype
static void ibus_unikey_engine_class_init(IBusUnikeyEngineClass *kclass);
static void ibus_unikey_engine_init(IBusUnikeyEngine *unikey);

static GObject *ibus_unikey_engine_constructor(GType type,
											   guint n_construct_params,
											   GObjectConstructParam *construct_params);

static void ibus_unikey_engine_destroy(IBusUnikeyEngine *unikey);
static gboolean ibus_unikey_engine_process_key_event(IBusEngine *engine,
													 guint keyval,
													 guint modifiers);
static void ibus_unikey_engine_focus_in(IBusEngine *engine);
static void ibus_unikey_engine_focus_out(IBusEngine *engine);
static void ibus_unikey_engine_reset(IBusEngine *engine);
static void ibus_unikey_engine_enable(IBusEngine *engine);
static void ibus_unikey_engine_disable(IBusEngine *engine);



static IBusEngineClass *parent_class = NULL;

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
		type = g_type_register_static (IBUS_TYPE_ENGINE,
									   "IBusUnikeyEngine",
									   &type_info,
									   (GTypeFlags)0
			);
	}

	return type;
}

static void ibus_unikey_engine_class_init(IBusUnikeyEngineClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS(klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS(klass);

	parent_class = (IBusEngineClass *)g_type_class_peek_parent(klass);

	object_class->constructor = ibus_unikey_engine_constructor;
	ibus_object_class->destroy = (IBusObjectDestroyFunc)ibus_unikey_engine_destroy;

	engine_class->process_key_event = ibus_unikey_engine_process_key_event;

	engine_class->reset = ibus_unikey_engine_reset;
	engine_class->enable = ibus_unikey_engine_enable;
	engine_class->disable = ibus_unikey_engine_disable;

	engine_class->focus_in = ibus_unikey_engine_focus_in;
	engine_class->focus_out = ibus_unikey_engine_focus_out;
}

static void ibus_unikey_engine_init(IBusUnikeyEngine *unikey)
{
/*
// doan nay dung de test thoi
IBusProperty *prop;
IBusText *label, *tooltip;

label = ibus_text_new_from_string("Test2");
tooltip = ibus_text_new_from_string("Second test");

prop = ibus_property_new("Test",
PROP_TYPE_NORMAL,
label,
"gtk-preferences",
tooltip,
TRUE, TRUE, (IBusPropState)0, NULL);

g_object_unref(label);
g_object_unref(tooltip);

ibus_prop_list_append(unikey->prop_list, prop);
*/
}

static GObject *ibus_unikey_engine_constructor(GType type,
											   guint n_construct_params,
											   GObjectConstructParam *construct_params)
{
	IBusUnikeyEngine *unikey;
	const gchar *engine_name;

	unikey = (IBusUnikeyEngine*)G_OBJECT_CLASS(parent_class)->constructor(type,
																		  n_construct_params,
																		  construct_params);

	engine_name = ibus_engine_get_name((IBusEngine*)unikey);
	g_assert(engine_name);


	return (GObject*)unikey;
}

static void ibus_unikey_engine_destroy(IBusUnikeyEngine *unikey)
{
}

static gboolean ibus_unikey_engine_process_key_event(IBusEngine *engine,
													 guint keyval,
													 guint modifiers)
{
	return FALSE;
}

static void ibus_unikey_engine_focus_in(IBusEngine *engine)
{
}

static void ibus_unikey_engine_focus_out(IBusEngine *engine)
{
}

static void ibus_unikey_engine_reset(IBusEngine *engine)
{
}

static void ibus_unikey_engine_enable(IBusEngine *engine)
{
}

static void ibus_unikey_engine_disable(IBusEngine *engine)
{
}





// utils


GList *ibus_unikey_list_engines()
{
	GList *engines = NULL;
	int i, j;
	gchar name[128];

	for (i=0; i < NUM_INPUTMETHOD; i++)
	{
		for (j=0; j < NUM_OUTPUTCHARSET; j++)
		{
			sprintf(name, "%s - %s", Unikey_IMNames[i], Unikey_OCNames[j]);
			IBusEngineDesc *desc = ibus_engine_desc_new(name,
														name,
														name,
														"vi",
														"GPL",
														"Le Quoc Tuan <mr.lequoctuan@gmail.com>",
														PKGDATADIR"/icons/ibus-unikey.png",
														"us");

			engines = g_list_append(engines, desc);
									
		}
	}

	return engines;
}

IBusComponent *ibus_unikey_get_component()
{
	IBusComponent *component;
	GList *engines, *p;


	component = ibus_component_new("org.freedesktop.IBus.Unikey",
								   "Unikey",
								   PACKAGE_VERSION,
								   "GPL",
								   "Le Quoc Tuan <mr.lequoctuan@gmail.com>",
								   PACKAGE_BUGREPORT,
								   "",
								   PACKAGE_NAME);

	engines = ibus_unikey_list_engines();

	for (p = engines; p != NULL; p = p->next)
		ibus_component_add_engine(component, (IBusEngineDesc*)p->data);

	g_list_free(engines);
	
	return component;
}
