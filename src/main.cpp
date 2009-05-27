#include <ibus.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include "engine.h"
#include "unikey.h"

static IBusBus *bus = NULL;
static IBusFactory *factory = NULL;

/* option */
static gboolean xml = FALSE;
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry entries[] =
{
	{ "xml", 'x', 0, G_OPTION_ARG_NONE, &xml, "generate xml for engines", NULL},
	{ "ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL },
	{ "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
	{ NULL },
};

static void
ibus_disconnected_cb(IBusBus *bus, gpointer  user_data)
{
	g_debug("bus disconnected");
	ibus_quit();
}

static void start_component(void)
{
	GList *engines, *p;
	IBusComponent *component;

	ibus_init();
	UnikeySetup();

	bus = ibus_bus_new();
	g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnected_cb), NULL);

	ibus_unikey_init(bus);

	component = ibus_unikey_get_component();

	factory = ibus_factory_new(ibus_bus_get_connection(bus));

	engines = ibus_component_get_engines(component);
	for (p = engines; p != NULL; p = p->next)
	{
		IBusEngineDesc *engine = (IBusEngineDesc*)p->data;
		ibus_factory_add_engine(factory, engine->name, IBUS_TYPE_UNIKEY_ENGINE);
	}

	if (ibus)
		ibus_bus_request_name(bus, "org.freedesktop.IBus.Unikey", 0);
	else
		ibus_bus_register_component(bus, component);

	g_object_unref(component);

	ibus_main();
	ibus_unikey_exit();
}

static void print_engines_xml(void)
{
	IBusComponent *component;
	GString *output;

	ibus_init();

	component = ibus_unikey_get_component();
	output = g_string_new("");

	ibus_component_output_engines(component, output, 0);

	fprintf(stdout, "%s", output->str);

	g_string_free(output, TRUE);
}

int main(gint argc, gchar **argv)
{
	GError *error = NULL;
	GOptionContext *context;

	setlocale(LC_ALL, "");

	context = g_option_context_new("- ibus unikey engine component");

	g_option_context_add_main_entries(context, entries, "ibus-unikey");

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("Option parsing failed: %s\n", error->message);
		exit(-1);
	}

	if (xml)
	{
		print_engines_xml();
		exit(0);
	}

	start_component();

	return 0;
}
