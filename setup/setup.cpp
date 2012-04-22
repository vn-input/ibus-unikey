#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libintl.h>
#include <locale.h>
#include <gtk/gtk.h>
#include <ibus.h>
#include <stdlib.h>

#include "dlg_main_setup.h"
#include "config_utils.h"

#define _(string) gettext(string)

static gboolean version = FALSE;

static const GOptionEntry entries[] =
{
    { "version", 'V', 0, G_OPTION_ARG_NONE, &version, "print ibus-unikey version", NULL },
    { NULL },
};

static void ibus_disconnected_cb(IBusBus* bus, gpointer user_data)
{   
    exit(2);
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);

    GError* error = NULL;
    GOptionContext* context;
    IBusConfig* config;

    context = g_option_context_new("- ibus unikey setup component");
    g_option_context_add_main_entries(context, entries, "ibus-unikey");
    g_option_context_parse(context, &argc, &argv, &error);
    
    if (version)
    {
        g_print(PACKAGE_STRING " (setup component)"
            "\n  Copyright (C) 2009 - 2012 Ubuntu-VN <http://www.ubuntu-vn.org>"
            "\n  Author: Lê Quốc Tuấn <mr.lequoctuan@gmail.com>"
            "\n  Homepage: <http://ibus-unikey.googlecode.com>"
            "\n  License: GNU GPL3"
            "\n");
        return 0;
    }

    gtk_init(&argc, &argv);

    gtk_window_set_default_icon_from_file(PKGDATADIR"/icons/ibus-unikey.png", NULL);

    GtkWidget* main_dlg = unikey_main_setup_dialog_new(); // create main dlg
    g_signal_connect(main_dlg, "destroy", gtk_main_quit, NULL); // connect with signal

    ibus_init();
    IBusBus* bus = ibus_bus_new();
    g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnected_cb), NULL);
    config = ibus_bus_get_config(bus);

    UnikeyMainSetupOptions opt; // create option

    read_config(config, &opt); // read config

    unikey_main_setup_set_values(GTK_DIALOG(main_dlg), &opt); // set config for dialog

    int ret = gtk_dialog_run(GTK_DIALOG(main_dlg));

    if (ret == GTK_RESPONSE_OK) // if pressed OK
    {
        unikey_main_setup_get_values(GTK_DIALOG(main_dlg), &opt); // get config from dialog
        write_config(config, &opt);
        return 0;
    }

    return 1;
}

