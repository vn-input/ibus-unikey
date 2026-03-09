#include <glib.h>

#include "macro_file_io.h"

static const gchar* fixture_dir = NULL;

static gint load_macro_count(const gchar* filename)
{
    CMacroTable macro;
    GError* error = NULL;

    const gboolean is_loaded = macro_table_load_any_format(filename, &macro, &error);
    g_assert_true(is_loaded);
    g_assert_no_error(error);

    return macro.getCount();
}

static void test_native_txt_loads_from_extension()
{
    gchar* txt_path = g_build_filename(fixture_dir, "unikey_macros.txt", NULL);
    const gint count = load_macro_count(txt_path);

    g_assert_cmpint(count, >, 0);
    g_free(txt_path);
}

static void test_yaml_loads_from_extension()
{
    gchar* yaml_path = g_build_filename(fixture_dir, "unikey_macro.yaml", NULL);
    const gint count = load_macro_count(yaml_path);

    g_assert_cmpint(count, >, 0);
    g_free(yaml_path);
}

static void test_plist_loads_from_extension()
{
    gchar* plist_path = g_build_filename(fixture_dir, "unikey_macro.plist", NULL);
    const gint count = load_macro_count(plist_path);

    g_assert_cmpint(count, >, 0);
    g_free(plist_path);
}

static void test_all_formats_resolve_by_filename_extension()
{
    gchar* txt_path = g_build_filename(fixture_dir, "unikey_macros.txt", NULL);
    gchar* yaml_path = g_build_filename(fixture_dir, "unikey_macro.yaml", NULL);
    gchar* plist_path = g_build_filename(fixture_dir, "unikey_macro.plist", NULL);

    const gint txt_count = load_macro_count(txt_path);
    const gint yaml_count = load_macro_count(yaml_path);
    const gint plist_count = load_macro_count(plist_path);

    g_assert_cmpint(txt_count, >, 0);
    g_assert_cmpint(yaml_count, >, 0);
    g_assert_cmpint(plist_count, >, 0);

    g_free(txt_path);
    g_free(yaml_path);
    g_free(plist_path);
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv, NULL);
    g_assert_cmpint(argc, >=, 2);

    fixture_dir = argv[1];

    g_test_add_func("/ui/macros/file-io/load-txt-from-extension", test_native_txt_loads_from_extension);
    g_test_add_func("/ui/macros/file-io/load-yaml-from-extension", test_yaml_loads_from_extension);
    g_test_add_func("/ui/macros/file-io/load-plist-from-extension", test_plist_loads_from_extension);
    g_test_add_func("/ui/macros/file-io/all-supported-filter-still-uses-filename-extension", test_all_formats_resolve_by_filename_extension);

    return g_test_run();
}