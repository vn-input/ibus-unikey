
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>

#include "dlg_main_setup.h"
#include "config_utils.h"

int main(int argc, char** argv)
{
    gtk_init(&argc, &argv);

    GtkWidget* main_dlg = unikey_main_setup_dialog_new(); // create main dlg

    gtk_signal_connect(GTK_OBJECT(main_dlg), "destroy", gtk_main_quit, NULL); // connect with signal

    UnikeyMainSetupOptions opt; // create option
    set_default_config(&opt); // create default option

    read_config(&opt); // read config

    unikey_main_setup_set_values(GTK_DIALOG(main_dlg), &opt);

    int ret = gtk_dialog_run(GTK_DIALOG(main_dlg));

    if (ret == GTK_RESPONSE_OK) // if pressed OK
    {
        unikey_main_setup_get_values(GTK_DIALOG(main_dlg), &opt);

        write_config(&opt);

        force_engine_to_reload_config();
    }

    return 0;
}

