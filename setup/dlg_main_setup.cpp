#include <gtk/gtk.h>

#include "dlg_macro_table.h"
#include "dlg_main_setup.h"

enum {COL_IM_NAME = 0};
enum {COL_OC_NAME = 0};

void macro_enable_toggle_cb(GtkToggleButton* btn, gpointer user_data);
void macro_edit_button_cb(GtkButton* btn, gpointer user_data);


GtkWidget* unikey_main_setup_dialog_new()
{
    GtkBuilder* builder = gtk_builder_new();

    gtk_builder_add_from_file(builder, GLADE_DATA_DIR "/setup-main.glade", NULL);

    GtkDialog* dlg = GTK_DIALOG(gtk_builder_get_object(builder, "dlg_main_setup"));


    // set data for input method combobox
    GtkComboBox* cbb_im = GTK_COMBO_BOX(gtk_builder_get_object(builder, "cbb_input_method"));
    GtkTreeModel* list_im = GTK_TREE_MODEL(gtk_builder_get_object(builder, "list_input_method"));

    gtk_combo_box_set_model(cbb_im, list_im); // set model

    GtkCellRenderer* render = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cbb_im), render, TRUE);

    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cbb_im),
                                   render,
                                   "text", COL_IM_NAME,
                                   NULL);  // set Column Input method name
    // END set data for input method combobox


    // set data for output charset combobox
    GtkComboBox* cbb_oc = GTK_COMBO_BOX(gtk_builder_get_object(builder, "cbb_output_charset"));
    GtkTreeModel* list_oc = GTK_TREE_MODEL(gtk_builder_get_object(builder, "list_output_charset"));

    gtk_combo_box_set_model(cbb_oc, list_oc); // set model

    render = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(cbb_oc), render, TRUE);

    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(cbb_oc),
                                   render,
                                   "text", COL_OC_NAME,
                                   NULL);  // set Column output charset nam
    // END set data for output charset combobox

    // set callback
    GtkWidget* btn = GTK_WIDGET(gtk_builder_get_object(builder, "btn_macroedit")); // macro_edit button
    g_signal_connect(btn, "clicked", G_CALLBACK(macro_edit_button_cb), dlg);

    GtkWidget* wid = GTK_WIDGET(gtk_builder_get_object(builder, "check_macroenable")); // enable macro checkbox
    g_signal_connect(wid, "toggled", G_CALLBACK(macro_enable_toggle_cb), btn);
    // END set callback

    // save object pointer for future use
    g_object_set_data(G_OBJECT(dlg), "cbb_input_method", cbb_im);
    g_object_set_data(G_OBJECT(dlg), "cbb_output_charset", cbb_oc);
    g_object_set_data(G_OBJECT(dlg), "check_macroenable", wid);
    g_object_set_data(G_OBJECT(dlg), "btn_macroedit", btn);
    g_object_set_data(G_OBJECT(dlg),
                      "check_spellcheck",
                      gtk_builder_get_object(builder, "check_spellcheck"));
    g_object_set_data(G_OBJECT(dlg),
                      "check_autorestorenonvn",
                      gtk_builder_get_object(builder, "check_autorestorenonvn"));
    g_object_set_data(G_OBJECT(dlg),
                      "check_modernstyle",
                      gtk_builder_get_object(builder, "check_modernstyle"));
    g_object_set_data(G_OBJECT(dlg),
                      "check_freemarking",
                      gtk_builder_get_object(builder, "check_freemarking"));
    g_object_set_data(G_OBJECT(dlg),
                      "check_processwatbegin",
                      gtk_builder_get_object(builder, "check_processwatbegin"));
    // END save object pointer

    g_object_unref(builder);

    return GTK_WIDGET(dlg);
}

void unikey_main_setup_set_values(const GtkDialog* dlg, const UnikeyMainSetupOptions *opt)
{
    GtkWidget* wid;

// set input method
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "cbb_input_method"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(wid), opt->input_method);

// set output charset
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "cbb_output_charset"));
    gtk_combo_box_set_active(GTK_COMBO_BOX(wid), opt->output_charset);

// set spellcheck?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_spellcheck"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), opt->enableSpellcheck);

// set autorestorenonvn?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_autorestorenonvn"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), opt->autoRestoreNonVn);

// set modernstyle?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_modernstyle"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), opt->modernStyle);

// set freemarking?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_freemarking"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), opt->freeMarking);

// set macroenable?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_macroenable"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), opt->enableMacro);

// if disable macro, disable btn_macroedit
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "btn_macroedit"));
    gtk_widget_set_sensitive(wid, opt->enableMacro);

// set processwatbegin?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_processwatbegin"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), opt->processwatbegin);

// set macro file name data
    g_object_set_data(G_OBJECT(dlg), "macrofile", opt->macrofile);
}

void unikey_main_setup_get_values(const GtkDialog* dlg, UnikeyMainSetupOptions *opt)
{
    GtkWidget* wid;

// get input method
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "cbb_input_method"));
    opt->input_method = gtk_combo_box_get_active(GTK_COMBO_BOX(wid));

// get output charset
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "cbb_output_charset"));
    opt->output_charset = gtk_combo_box_get_active(GTK_COMBO_BOX(wid));

// get spellcheck?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_spellcheck"));
    opt->enableSpellcheck = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid));

// get autorestorenonvn?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_autorestorenonvn"));
    opt->autoRestoreNonVn = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid));

// get modernstyle?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_modernstyle"));
    opt->modernStyle = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid));

// get freemarking?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_freemarking"));
    opt->freeMarking = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid));

// get macroenable?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_macroenable"));
    opt->enableMacro = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid));

// get processwatbegin?
    wid = GTK_WIDGET(g_object_get_data(G_OBJECT(dlg), "check_processwatbegin"));
    opt->processwatbegin = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wid));
}

void macro_enable_toggle_cb(GtkToggleButton* btn, gpointer user_data)
{
    gboolean b = gtk_toggle_button_get_active(btn);
    gtk_widget_set_sensitive(GTK_WIDGET(user_data), b);
}

void macro_edit_button_cb(GtkButton* btn, gpointer user_data)
{
    GtkWidget* parent_dlg = GTK_WIDGET(user_data);

    gtk_widget_set_sensitive(parent_dlg, FALSE);

    GtkWidget* dlg = unikey_macro_dialog_new();

    gchar* macrofile = (gchar*)(g_object_get_data(G_OBJECT(parent_dlg), "macrofile"));

    CMacroTable macro;
    macro.init();
    macro.loadFromFile(macrofile);

    unikey_macro_dialog_load_macro(GTK_DIALOG(dlg), macro);

    int ret = gtk_dialog_run(GTK_DIALOG(dlg));

    if (ret == GTK_RESPONSE_OK)
    {
        unikey_macro_dialog_save_macro(GTK_DIALOG(dlg), &macro);

        GFile* f = g_file_get_parent(g_file_new_for_path(macrofile));
        if (g_file_query_exists(f, NULL) == FALSE)
        {
            g_file_make_directory_with_parents(f, NULL, NULL);
        }
        g_object_unref(f);

        macro.writeToFile(macrofile);
    }

    gtk_widget_destroy(dlg);

    gtk_widget_set_sensitive(parent_dlg, TRUE);
}

