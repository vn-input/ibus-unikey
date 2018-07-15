#include <gtk/gtk.h>

extern "C"
{
    G_MODULE_EXPORT void on_main_window_destroy(GtkWidget* w, gpointer user_data);
    G_MODULE_EXPORT gboolean on_main_window_key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer data);
    G_MODULE_EXPORT void on_btn_close_clicked(GtkButton* btn, gpointer user_data);
    
    G_MODULE_EXPORT void on_setting_toggled(GtkToggleButton* btn, gpointer user_data);
    G_MODULE_EXPORT void on_setting_realize(GtkToggleButton* btn, gpointer user_data);

    G_MODULE_EXPORT void on_btn_macroedit_clicked(GtkButton* btn, gpointer user_data);

    G_MODULE_EXPORT void on_input_method_changed(GtkComboBox* cbb, gpointer user_data);
    G_MODULE_EXPORT void on_input_method_realize(GtkComboBox* cbb, gpointer user_data);
    G_MODULE_EXPORT void on_output_charset_changed(GtkComboBox* cbb, gpointer user_data);
    G_MODULE_EXPORT void on_output_charset_realize(GtkComboBox* cbb, gpointer user_data);

    G_MODULE_EXPORT gboolean on_macro_dialog_delete(GtkWidget* wid, GdkEvent* ev, gpointer user_data);
    G_MODULE_EXPORT void macro_dialog_hide(GtkButton* btn, gpointer user_data);

    G_MODULE_EXPORT void on_btn_macro_del_clicked(GtkButton *button, gpointer user_data);
    G_MODULE_EXPORT void on_btn_macro_clear_clicked(GtkButton *button, gpointer user_data);
    G_MODULE_EXPORT void on_btn_macro_import_clicked(GtkButton *button, gpointer user_data);
    G_MODULE_EXPORT void on_btn_macro_export_clicked(GtkButton *button, gpointer user_data);

    G_MODULE_EXPORT void on_cell_key_edited(GtkCellRendererText *celltext,
                    const gchar *string_path,
                    const gchar *newkey,
                    gpointer user_data);
    G_MODULE_EXPORT void on_cell_value_edited(GtkCellRendererText *celltext,
                     const gchar *string_path,
                     const gchar *newvalue,
                     gpointer data);
}
