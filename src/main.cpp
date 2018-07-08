#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libintl.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

#include <ibus.h>
#include "engine.h"
#include "unikey.h"

#define _(string) gettext(string)

static IBusBus* bus         = NULL;
static IBusFactory* factory = NULL;

/* option */
static gboolean xml     = FALSE;
static gboolean ibus    = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry entries[] =
{
    { "xml",     'x', 0, G_OPTION_ARG_NONE, &xml,     "generate xml for engines", NULL },
    { "ibus",    'i', 0, G_OPTION_ARG_NONE, &ibus,    "component is executed by ibus", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
    { NULL },
};

static IBusComponent* ibus_unikey_get_component();

static void ibus_disconnected_cb(IBusBus* bus, gpointer user_data)
{
    ibus_quit();
}

static void start_component(void)
{
    GList* engines;
    GList* p;
    IBusComponent* component;

    ibus_init();

    bus = ibus_bus_new();
    g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnected_cb), NULL);

    component = ibus_unikey_get_component();

    factory = ibus_factory_new(ibus_bus_get_connection(bus));

    engines = ibus_component_get_engines(component);
    for (p = engines; p != NULL; p = p->next)
    {
        IBusEngineDesc* engine = (IBusEngineDesc*)p->data;
        ibus_factory_add_engine(factory, ibus_engine_desc_get_name(engine), IBUS_TYPE_UNIKEY_ENGINE);
    }

    if (ibus)
        ibus_bus_request_name(bus, "org.freedesktop.IBus.Unikey", 0);
    else
        ibus_bus_register_component(bus, component);

    g_object_unref(component);

    ibus_unikey_init(bus);
    ibus_main();
    ibus_unikey_exit();
}

static void print_engines_xml(void)
{
    IBusComponent* component;
    GString* output;

    ibus_init();

    component = ibus_unikey_get_component();
    output = g_string_new("");

    ibus_component_output_engines(component, output, 0);

    fprintf(stdout, "%s", output->str);

    g_string_free(output, TRUE);
}

int main(gint argc, gchar** argv)
{
    GError* error = NULL;
    GOptionContext* context;

    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);

    context = g_option_context_new("- ibus unikey engine component");

    g_option_context_add_main_entries(context, entries, "ibus-unikey");

    if (!g_option_context_parse(context, &argc, &argv, &error)) {
        g_print("Option parsing failed: %s\n", error->message);
        exit(-1);
    }

    if (xml)
    {
        print_engines_xml();
        return 0;
    }

    start_component();

    return 0;
}

#define IU_DESC _("Vietnamese Input Method Engine for IBus using Unikey Engine\n\
Usage:\n\
  - Choose input method, output charset, options in language bar.\n\
  - There are 4 input methods: Telex, Vni, STelex (simple telex) \
and STelex2 (which same as STelex, the difference is it use w as ư).\n\
  - And 7 output charsets: Unicode (UTF-8), TCVN3, VNI Win, VIQR, CString, NCR Decimal and NCR Hex.\n\
  - Use <Shift>+<Space> or <Shift>+<Shift> to restore keystrokes.\n\
  - Use <Control> to commit a word.\
")

static IBusComponent* ibus_unikey_get_component()
{
    IBusComponent* component;
    IBusEngineDesc* engine;

    component = ibus_component_new("org.freedesktop.IBus.Unikey",
                                   "Unikey component",
                                   PACKAGE_VERSION,
                                   "GPLv3",
                                   "Vietnamese input group",
                                   PACKAGE_BUGREPORT,
                                   "",
                                   PACKAGE_NAME);

    engine = ibus_engine_desc_new_varargs ("name",        "Unikey",
                                           "longname",    "Unikey",
                                           "description", IU_DESC,
                                           "language",    "vi",
                                           "license",     "GPLv3",
                                           "author",      "Vietnamese input group",
                                           "icon",        PKGDATADIR "/icons/ibus-unikey.svg",
                                           "layout",      "*",
                                           "rank",        99,
                                           "setup",       LIBEXECDIR "/ibus-setup-unikey",
                                           NULL);

    ibus_component_add_engine(component, engine);

    return component;
}

