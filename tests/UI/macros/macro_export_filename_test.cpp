#include <glib.h>

#include "macro_export_filename.h"

static void test_appends_txt_extension_when_missing()
{
    gchar* completed = macro_export_filename_complete("/tmp/macros", ".txt");
    g_assert_cmpstr(completed, ==, "/tmp/macros.txt");
    g_free(completed);
}

static void test_appends_yaml_extension_when_missing()
{
    gchar* completed = macro_export_filename_complete("macro-backup", ".yaml");
    g_assert_cmpstr(completed, ==, "macro-backup.yaml");
    g_free(completed);
}

static void test_preserves_matching_extension()
{
    gchar* completed = macro_export_filename_complete("macro.plist", ".plist");
    g_assert_cmpstr(completed, ==, "macro.plist");
    g_free(completed);
}

static void test_appends_selected_extension_on_conflict()
{
    gchar* completed = macro_export_filename_complete("macro.plist", ".yaml");
    g_assert_cmpstr(completed, ==, "macro.plist.yaml");
    g_free(completed);
}

static void test_detects_extension_conflict()
{
    gboolean has_conflict = macro_export_filename_has_extension_conflict("macro.plist", ".yaml");
    g_assert_true(has_conflict);
}

static void test_ignores_matching_extension_conflict_check()
{
    gboolean has_conflict = macro_export_filename_has_extension_conflict("macro.yaml", ".yaml");
    g_assert_false(has_conflict);
}

static void test_hidden_dotfile_is_treated_as_extensionless()
{
    gchar* completed = macro_export_filename_complete(".macro-export", ".yaml");
    g_assert_cmpstr(completed, ==, ".macro-export.yaml");
    g_free(completed);
}

static void test_missing_preferred_extension_keeps_name()
{
    gchar* completed = macro_export_filename_complete("macro-export", NULL);
    g_assert_cmpstr(completed, ==, "macro-export");
    g_free(completed);
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/ui/macros/export-filename/appends-txt", test_appends_txt_extension_when_missing);
    g_test_add_func("/ui/macros/export-filename/appends-yaml", test_appends_yaml_extension_when_missing);
    g_test_add_func("/ui/macros/export-filename/preserves-matching-extension", test_preserves_matching_extension);
    g_test_add_func("/ui/macros/export-filename/appends-on-conflict", test_appends_selected_extension_on_conflict);
    g_test_add_func("/ui/macros/export-filename/detects-conflict", test_detects_extension_conflict);
    g_test_add_func("/ui/macros/export-filename/ignores-matching-conflict", test_ignores_matching_extension_conflict_check);
    g_test_add_func("/ui/macros/export-filename/dotfile-is-extensionless", test_hidden_dotfile_is_treated_as_extensionless);
    g_test_add_func("/ui/macros/export-filename/missing-extension-keeps-name", test_missing_preferred_extension_keeps_name);

    return g_test_run();
}