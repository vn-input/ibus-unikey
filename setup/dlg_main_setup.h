#include <gtk/gtk.h>

typedef struct
{
    int input_method;
    int output_charset;
    
    gboolean enableSpellcheck;
    gboolean autoRestoreNonVn;
    gboolean modernStyle;
    gboolean freeMarking;
    gboolean enableMacro;
    gboolean processwatbegin;

    gchar* macrofile;
} UnikeyMainSetupOptions;

GtkWidget* unikey_main_setup_dialog_new();

void unikey_main_setup_set_values(const GtkDialog* dlg, const UnikeyMainSetupOptions *opt);
void unikey_main_setup_get_values(const GtkDialog* dlg, UnikeyMainSetupOptions *opt);

