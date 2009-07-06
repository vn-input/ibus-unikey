
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libintl.h>
#include <locale.h>
#include <gtk/gtk.h>

#include "dlg_main_setup.h"
#include "config_utils.h"

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);

    gtk_init(&argc, &argv);

    gtk_window_set_default_icon_from_file(PKGDATADIR"/icons/ibus-unikey.png", NULL);

    GtkWidget* main_dlg = unikey_main_setup_dialog_new(); // create main dlg
    gtk_window_set_title(GTK_WINDOW(main_dlg), "ibus-unikey setup v" PACKAGE_VERSION);
    gtk_signal_connect(GTK_OBJECT(main_dlg), "destroy", gtk_main_quit, NULL); // connect with signal

    UnikeyMainSetupOptions opt; // create option
    set_default_config(&opt); // create default option

    read_config(&opt); // read config

    unikey_main_setup_set_values(GTK_DIALOG(main_dlg), &opt); // set config for dialog

    int ret = gtk_dialog_run(GTK_DIALOG(main_dlg));

    if (ret == GTK_RESPONSE_OK) // if pressed OK
    {
        unikey_main_setup_get_values(GTK_DIALOG(main_dlg), &opt); // get config from dialog

        write_config(&opt);

        force_engine_to_reload_config();
    }

    return 0;
}

