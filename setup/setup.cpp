#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libintl.h>
#include <locale.h>
#include <gtk/gtk.h>

#include "dlg_main_setup.h"
#include "config_utils.h"

#define _(string) gettext(string)

static gboolean engine = FALSE;
static gboolean version = FALSE;

static const GOptionEntry entries[] =
{
    { "engine", 'x', 0, G_OPTION_ARG_NONE, &engine, "indicate setup run from ibus-engine-unikey", NULL },
    { "version", 'V', 0, G_OPTION_ARG_NONE, &version, "print ibus-unikey version", NULL },
    { NULL },
};

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);

    GError* error = NULL;
    GOptionContext* context;

    context = g_option_context_new("- ibus unikey setup component");
    g_option_context_add_main_entries(context, entries, "ibus-unikey");
    g_option_context_parse(context, &argc, &argv, &error);
    
    if (version)
    {
        g_print(PACKAGE_STRING " (setup component)"
            "\n  Copyright (C) 2009, 2010 Ubuntu-VN <http://www.ubuntu-vn.org>"
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

    UnikeyMainSetupOptions opt; // create option
    set_default_config(&opt); // create default option

    read_config(&opt); // read config

    unikey_main_setup_set_values(GTK_DIALOG(main_dlg), &opt); // set config for dialog

    int ret = gtk_dialog_run(GTK_DIALOG(main_dlg));

    if (ret == GTK_RESPONSE_OK) // if pressed OK
    {
        unikey_main_setup_get_values(GTK_DIALOG(main_dlg), &opt); // get config from dialog
        write_config(&opt);
        if (!engine)
            force_engine_to_reload_config();
        return 0;
    }

    return 1;
}

