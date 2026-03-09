#include <glib.h>

#include "macro_dialog_state.h"
#include "unikey_config.h"

static void reset_saved_dirs()
{
    ibus_unikey_config_set_string(CONFIG_MACRO_LASTIMPORTDIR, "");
    ibus_unikey_config_set_string(CONFIG_MACRO_LASTEXPORTDIR, "");
}

static void test_import_dir_initially_unset()
{
    reset_saved_dirs();

    gchar* saved_dir = macro_dialog_get_last_import_dir();
    g_assert_null(saved_dir);
}

static void test_export_dir_initially_unset()
{
    reset_saved_dirs();

    gchar* saved_dir = macro_dialog_get_last_export_dir();
    g_assert_null(saved_dir);
}

static void test_remember_import_file_stores_parent_dir()
{
    reset_saved_dirs();

    macro_dialog_remember_import_file("/tmp/unikey-tests/imports/sample.txt");

    gchar* saved_dir = macro_dialog_get_last_import_dir();
    g_assert_nonnull(saved_dir);
    g_assert_cmpstr(saved_dir, ==, "/tmp/unikey-tests/imports");
    g_free(saved_dir);
}

static void test_remember_export_file_stores_parent_dir()
{
    reset_saved_dirs();

    macro_dialog_remember_export_file("/tmp/unikey-tests/exports/sample.yaml");

    gchar* saved_dir = macro_dialog_get_last_export_dir();
    g_assert_nonnull(saved_dir);
    g_assert_cmpstr(saved_dir, ==, "/tmp/unikey-tests/exports");
    g_free(saved_dir);
}

static void test_last_value_overwrites_previous_dir()
{
    reset_saved_dirs();

    macro_dialog_remember_import_file("/tmp/unikey-tests/first/a.txt");
    macro_dialog_remember_import_file("/tmp/unikey-tests/second/b.txt");

    gchar* saved_dir = macro_dialog_get_last_import_dir();
    g_assert_nonnull(saved_dir);
    g_assert_cmpstr(saved_dir, ==, "/tmp/unikey-tests/second");
    g_free(saved_dir);
}

static void test_import_and_export_dirs_are_separate()
{
    reset_saved_dirs();

    macro_dialog_remember_import_file("/tmp/unikey-tests/in/macro.txt");
    macro_dialog_remember_export_file("/tmp/unikey-tests/out/macro.plist");

    gchar* import_dir = macro_dialog_get_last_import_dir();
    gchar* export_dir = macro_dialog_get_last_export_dir();

    g_assert_nonnull(import_dir);
    g_assert_nonnull(export_dir);
    g_assert_cmpstr(import_dir, ==, "/tmp/unikey-tests/in");
    g_assert_cmpstr(export_dir, ==, "/tmp/unikey-tests/out");

    g_free(import_dir);
    g_free(export_dir);
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv, NULL);

    if (argc >= 2) {
        g_setenv("GSETTINGS_SCHEMA_DIR", argv[1], TRUE);
    }
    g_setenv("GSETTINGS_BACKEND", "memory", TRUE);

    ibus_unikey_config_init();

    g_test_add_func("/ui/macros/import-dir-initially-unset", test_import_dir_initially_unset);
    g_test_add_func("/ui/macros/export-dir-initially-unset", test_export_dir_initially_unset);
    g_test_add_func("/ui/macros/remember-import-parent", test_remember_import_file_stores_parent_dir);
    g_test_add_func("/ui/macros/remember-export-parent", test_remember_export_file_stores_parent_dir);
    g_test_add_func("/ui/macros/import-overwrites-previous", test_last_value_overwrites_previous_dir);
    g_test_add_func("/ui/macros/import-export-separate", test_import_and_export_dirs_are_separate);

    return g_test_run();
}
