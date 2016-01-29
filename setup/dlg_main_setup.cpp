#include <gtk/gtk.h>
#include <ibus.h>
#include <string.h>

#include "dlg_macro_table.h"
#include "dlg_main_setup.h"
#include "engine_const.h"
#include "utils.h"

static void init_dialog_controls(GtkBuilder*);
static void input_method_combo_box_changed_cb(GtkComboBox*, gpointer);
static void output_charset_combo_box_changed_cb(GtkComboBox*, gpointer);
static void update_config_toggle_cb(GtkToggleButton*, gpointer);
static void macro_edit_button_cb(GtkButton*, gpointer);

static IBusConfig* config;

GtkWidget* unikey_main_setup_dialog_new()
{
    GtkBuilder* builder;
    IBusBus* bus;
    GtkDialog* dlgMain;

    ibus_init();
    bus = ibus_bus_new();
    g_signal_connect(bus, "disconnected", G_CALLBACK(gtk_main_quit), NULL);
    config = ibus_bus_get_config(bus);
    
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, UI_DATA_DIR "/setup-main.ui", NULL);

    dlgMain = GTK_DIALOG(gtk_builder_get_object(builder, "dlg_main_setup"));

    init_dialog_controls(builder);

    g_object_unref(builder);

    return GTK_WIDGET(dlgMain);
}

void init_dialog_controls(GtkBuilder* builder)
{
    GtkWidget* wid;
    guint i;
    gchar* str;
    gboolean b;
    GtkListStore* ls;
    GtkTreeIter iter;

    // fill inputmethod
    ls = GTK_LIST_STORE(gtk_builder_get_object(builder, "list_input_method"));
    gtk_list_store_clear(ls);
    for (i = 0; i < NUM_INPUTMETHOD; i++)
    {
        gtk_list_store_append(ls, &iter);
        gtk_list_store_set(ls, &iter, 0, Unikey_IMNames[i], -1);
    }

    // fill outputcharset
    ls = GTK_LIST_STORE(gtk_builder_get_object(builder, "list_output_charset"));
    gtk_list_store_clear(ls);
    for (i = 0; i < NUM_OUTPUTCHARSET; i++)
    {
        gtk_list_store_append(ls, &iter);
        gtk_list_store_set(ls, &iter, 0, Unikey_OCNames[i], -1);
    }

    wid = GTK_WIDGET(gtk_builder_get_object(builder, "cbb_input_method"));
    g_signal_connect(wid, "changed", G_CALLBACK(input_method_combo_box_changed_cb), NULL);
    i = 0;
    if (ibus_unikey_config_get_string(config, CONFIG_SECTION, CONFIG_INPUTMETHOD, &str))
    {   
        for (; i < NUM_INPUTMETHOD; i++)
        {   
            if (strcasecmp(str, Unikey_IMNames[i]) == 0) break;
        }   
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(wid), i);

    wid = GTK_WIDGET(gtk_builder_get_object(builder, "cbb_output_charset"));
    g_signal_connect(wid, "changed", G_CALLBACK(output_charset_combo_box_changed_cb), NULL);
    i = 0;
    if (ibus_unikey_config_get_string(config, CONFIG_SECTION, CONFIG_OUTPUTCHARSET, &str))
    {   
        for (; i < NUM_OUTPUTCHARSET; i++)
        {   
            if (strcasecmp(str, Unikey_OCNames[i]) == 0) break;
        }   
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(wid), i);

    wid = GTK_WIDGET(gtk_builder_get_object(builder, "check_spellcheck"));
    g_signal_connect(wid, "toggled", G_CALLBACK(update_config_toggle_cb), (void*)CONFIG_SPELLCHECK);
    if (!ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_SPELLCHECK, &b))
        b = DEFAULT_CONF_SPELLCHECK;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), b);

    wid = GTK_WIDGET(gtk_builder_get_object(builder, "check_autorestorenonvn"));
    g_signal_connect(wid, "toggled", G_CALLBACK(update_config_toggle_cb), (void*)CONFIG_AUTORESTORENONVN);
    if (!ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_AUTORESTORENONVN, &b))
        b = DEFAULT_CONF_AUTONONVNRESTORE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), b);

    wid = GTK_WIDGET(gtk_builder_get_object(builder, "check_modernstyle"));
    g_signal_connect(wid, "toggled", G_CALLBACK(update_config_toggle_cb), (void*)CONFIG_MODERNSTYLE);
    if (!ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_MODERNSTYLE, &b))
        b = DEFAULT_CONF_MODERNSTYLE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), b);

    wid = GTK_WIDGET(gtk_builder_get_object(builder, "check_freemarking"));
    g_signal_connect(wid, "toggled", G_CALLBACK(update_config_toggle_cb), (void*)CONFIG_FREEMARKING);
    if (!ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_FREEMARKING, &b))
        b = DEFAULT_CONF_FREEMARKING;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), b);

    wid = GTK_WIDGET(gtk_builder_get_object(builder, "check_macroenable"));
    g_signal_connect(wid, "toggled", G_CALLBACK(update_config_toggle_cb), (void*)CONFIG_MACROENABLED);
    if (!ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_MACROENABLED, &b))
        b = DEFAULT_CONF_MACROENABLED;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), b);

    wid = GTK_WIDGET(gtk_builder_get_object(builder, "check_processwatbegin"));
    g_signal_connect(wid, "toggled", G_CALLBACK(update_config_toggle_cb), (void*)CONFIG_PROCESSWATBEGIN);
    if (!ibus_unikey_config_get_boolean(config, CONFIG_SECTION, CONFIG_PROCESSWATBEGIN, &b))
        b = DEFAULT_CONF_PROCESSWATBEGIN;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(wid), b);

    wid = GTK_WIDGET(gtk_builder_get_object(builder, "btn_macroedit"));
    g_signal_connect(wid, "clicked", G_CALLBACK(macro_edit_button_cb), gtk_builder_get_object(builder, "dlg_main_setup"));
}

void input_method_combo_box_changed_cb(GtkComboBox* cbb, gpointer user_data)
{
    gint s = gtk_combo_box_get_active(cbb);
    ibus_unikey_config_set_string(config, CONFIG_SECTION, CONFIG_INPUTMETHOD, Unikey_IMNames[s]);
}

void output_charset_combo_box_changed_cb(GtkComboBox* cbb, gpointer user_data)
{
    gint s = gtk_combo_box_get_active(cbb);
    ibus_unikey_config_set_string(config, CONFIG_SECTION, CONFIG_OUTPUTCHARSET, Unikey_OCNames[s]);
}

void update_config_toggle_cb(GtkToggleButton* btn, gpointer user_data)
{
    gboolean b = gtk_toggle_button_get_active(btn);
    ibus_unikey_config_set_boolean(config, CONFIG_SECTION, (gchar*)user_data, b);
}

void macro_edit_button_cb(GtkButton* btn, gpointer user_data)
{
    GtkWidget* parent_dlg = GTK_WIDGET(user_data);

    gtk_widget_set_sensitive(parent_dlg, FALSE);

    GtkWidget* dlg = unikey_macro_dialog_new();

    gchar* macrofile = get_macro_file();

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

    g_free(macrofile);

    gtk_widget_destroy(dlg);

    gtk_widget_set_sensitive(parent_dlg, TRUE);
}

