#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <libintl.h>
#include <locale.h>
#include <gtk/gtk.h>

#include "dlg_main_setup.h"

#define _(string) gettext(string)

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    textdomain(GETTEXT_PACKAGE);

    gtk_init(&argc, &argv);

    gtk_window_set_default_icon_from_file(PKGDATADIR "/icons/ibus-unikey.svg", NULL);

    GtkWidget* main_dlg = unikey_main_setup_dialog_new(); // create main dlg
    g_signal_connect(main_dlg, "destroy", gtk_main_quit, NULL); // connect with signal

    gtk_dialog_run(GTK_DIALOG(main_dlg));

    return 0;
}

