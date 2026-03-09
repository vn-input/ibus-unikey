#include <glib.h>
#include <glib/gstdio.h>

#include "macro_dialog_state.h"
#include "unikey_config.h"

static gchar* test_home_dir = NULL;

static gchar* get_expected_default_dir()
{
    return g_build_filename(test_home_dir, "Documents", NULL);
}

static gchar* create_existing_dir(const gchar* first, const gchar* second)
{
    gchar* dir_path = g_build_filename(test_home_dir, first, second, NULL);
    g_mkdir_with_parents(dir_path, 0700);
    return dir_path;
}

static void reset_saved_dirs()
{
    ibus_unikey_config_set_string(CONFIG_MACRO_LASTWORKINGDIR, "");
}

static void test_import_dir_defaults_to_documents()
{
    reset_saved_dirs();

    gchar* saved_dir = macro_dialog_get_last_import_dir();
    gchar* expected_dir = get_expected_default_dir();

    g_assert_nonnull(saved_dir);
    g_assert_cmpstr(saved_dir, ==, expected_dir);
    g_assert_true(g_file_test(saved_dir, G_FILE_TEST_IS_DIR));

    gchar* persisted_dir = NULL;
    g_assert_true(ibus_unikey_config_get_string(CONFIG_MACRO_LASTWORKINGDIR, &persisted_dir));
    g_assert_cmpstr(persisted_dir, ==, expected_dir);

    g_free(persisted_dir);
    g_free(expected_dir);
    g_free(saved_dir);
}

static void test_export_dir_defaults_to_documents()
{
    reset_saved_dirs();

    gchar* saved_dir = macro_dialog_get_last_export_dir();
    gchar* expected_dir = get_expected_default_dir();

    g_assert_nonnull(saved_dir);
    g_assert_cmpstr(saved_dir, ==, expected_dir);
    g_assert_true(g_file_test(saved_dir, G_FILE_TEST_IS_DIR));

    gchar* persisted_dir = NULL;
    g_assert_true(ibus_unikey_config_get_string(CONFIG_MACRO_LASTWORKINGDIR, &persisted_dir));
    g_assert_cmpstr(persisted_dir, ==, expected_dir);

    g_free(persisted_dir);
    g_free(expected_dir);
    g_free(saved_dir);
}

static void test_remember_import_file_stores_parent_dir()
{
    reset_saved_dirs();

    gchar* import_dir = create_existing_dir("tmp", "imports");
    gchar* filename = g_build_filename(import_dir, "sample.txt", NULL);

    macro_dialog_remember_import_file(filename);

    gchar* saved_dir = macro_dialog_get_last_import_dir();
    g_assert_nonnull(saved_dir);
    g_assert_cmpstr(saved_dir, ==, import_dir);
    g_free(filename);
    g_free(import_dir);
    g_free(saved_dir);
}

static void test_remember_export_file_stores_parent_dir()
{
    reset_saved_dirs();

    gchar* export_dir = create_existing_dir("tmp", "exports");
    gchar* filename = g_build_filename(export_dir, "sample.yaml", NULL);

    macro_dialog_remember_export_file(filename);

    gchar* saved_dir = macro_dialog_get_last_export_dir();
    g_assert_nonnull(saved_dir);
    g_assert_cmpstr(saved_dir, ==, export_dir);
    g_free(filename);
    g_free(export_dir);
    g_free(saved_dir);
}

static void test_last_value_overwrites_previous_dir()
{
    reset_saved_dirs();

    gchar* first_dir = create_existing_dir("tmp", "first");
    gchar* second_dir = create_existing_dir("tmp", "second");
    gchar* first_file = g_build_filename(first_dir, "a.txt", NULL);
    gchar* second_file = g_build_filename(second_dir, "b.txt", NULL);

    macro_dialog_remember_import_file(first_file);
    macro_dialog_remember_import_file(second_file);

    gchar* saved_dir = macro_dialog_get_last_import_dir();
    g_assert_nonnull(saved_dir);
    g_assert_cmpstr(saved_dir, ==, second_dir);
    g_free(first_file);
    g_free(second_file);
    g_free(first_dir);
    g_free(second_dir);
    g_free(saved_dir);
}

static void test_import_and_export_share_same_last_working_dir()
{
    reset_saved_dirs();

    gchar* import_dir_expected = create_existing_dir("tmp", "in");
    gchar* export_dir_expected = create_existing_dir("tmp", "out");
    gchar* import_file = g_build_filename(import_dir_expected, "macro.txt", NULL);
    gchar* export_file = g_build_filename(export_dir_expected, "macro.plist", NULL);

    macro_dialog_remember_import_file(import_file);
    macro_dialog_remember_export_file(export_file);

    gchar* import_dir = macro_dialog_get_last_import_dir();
    gchar* export_dir = macro_dialog_get_last_export_dir();

    g_assert_nonnull(import_dir);
    g_assert_nonnull(export_dir);
    g_assert_cmpstr(import_dir, ==, export_dir_expected);
    g_assert_cmpstr(export_dir, ==, export_dir_expected);

    g_free(import_file);
    g_free(export_file);
    g_free(export_dir_expected);
    g_free(import_dir_expected);
    g_free(import_dir);
    g_free(export_dir);
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv, NULL);

    test_home_dir = g_dir_make_tmp("unikey-macro-dialog-test-home-XXXXXX", NULL);
    g_assert_nonnull(test_home_dir);
    g_setenv("HOME", test_home_dir, TRUE);

    if (argc >= 2) {
        g_setenv("GSETTINGS_SCHEMA_DIR", argv[1], TRUE);
    }
    g_setenv("GSETTINGS_BACKEND", "memory", TRUE);

    ibus_unikey_config_init();

    g_test_add_func("/ui/macros/import-dir-defaults-to-documents", test_import_dir_defaults_to_documents);
    g_test_add_func("/ui/macros/export-dir-defaults-to-documents", test_export_dir_defaults_to_documents);
    g_test_add_func("/ui/macros/remember-import-parent", test_remember_import_file_stores_parent_dir);
    g_test_add_func("/ui/macros/remember-export-parent", test_remember_export_file_stores_parent_dir);
    g_test_add_func("/ui/macros/import-overwrites-previous", test_last_value_overwrites_previous_dir);
    g_test_add_func("/ui/macros/import-export-share-last-working-dir", test_import_and_export_share_same_last_working_dir);

    return g_test_run();
}
