/**
 * @file inputproc_classification_test.cpp
 * @brief Tests for input processor character classification (UkcMap).
 *
 * These tests verify that the character classification table correctly
 * identifies Vietnamese characters, word-break symbols, and non-Vietnamese
 * characters.
 */

#include <glib.h>
#include "inputproc.h"

// External declarations from inputproc.cpp
extern UkCharType UkcMap[256];
extern bool is_classifier_table_initialized;
extern void SetupInputClassifierTable();

/**
 * @brief Ensure classification tables are initialized before tests.
 */
static void ensure_tables_initialized()
{
    if (!is_classifier_table_initialized) {
        SetupInputClassifierTable();
        is_classifier_table_initialized = true;
    }
}

/**
 * @brief Test that lowercase Vietnamese vowels are classified as ukcVn.
 */
static void test_lowercase_vowels_are_vn()
{
    ensure_tables_initialized();
    
    g_assert_cmpint(UkcMap[(unsigned char)'a'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'e'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'i'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'o'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'u'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'y'], ==, ukcVn);
}

/**
 * @brief Test that uppercase Vietnamese vowels are classified as ukcVn.
 */
static void test_uppercase_vowels_are_vn()
{
    ensure_tables_initialized();
    
    g_assert_cmpint(UkcMap[(unsigned char)'A'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'E'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'I'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'O'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'U'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'Y'], ==, ukcVn);
}

/**
 * @brief Test that Vietnamese consonants are classified as ukcVn.
 */
static void test_consonants_are_vn()
{
    ensure_tables_initialized();
    
    // Lowercase consonants that ARE Vietnamese
    g_assert_cmpint(UkcMap[(unsigned char)'b'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'c'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'d'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'g'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'h'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'k'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'l'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'m'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'n'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'p'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'q'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'r'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'s'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'t'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'v'], ==, ukcVn);
    g_assert_cmpint(UkcMap[(unsigned char)'x'], ==, ukcVn);
}

/**
 * @brief Test that J, F, W are classified as non-Vietnamese.
 *
 * These letters do not appear in native Vietnamese words.
 */
static void test_non_vn_letters()
{
    ensure_tables_initialized();
    
    g_assert_cmpint(UkcMap[(unsigned char)'j'], ==, ukcNonVn);
    g_assert_cmpint(UkcMap[(unsigned char)'J'], ==, ukcNonVn);
    g_assert_cmpint(UkcMap[(unsigned char)'f'], ==, ukcNonVn);
    g_assert_cmpint(UkcMap[(unsigned char)'F'], ==, ukcNonVn);
    g_assert_cmpint(UkcMap[(unsigned char)'w'], ==, ukcNonVn);
    g_assert_cmpint(UkcMap[(unsigned char)'W'], ==, ukcNonVn);
}

/**
 * @brief Test that digits are classified as non-Vietnamese.
 */
static void test_digits_are_non_vn()
{
    ensure_tables_initialized();
    
    for (unsigned char c = '0'; c <= '9'; c++) {
        g_assert_cmpint(UkcMap[c], ==, ukcNonVn);
    }
}

/**
 * @brief Test that control characters are classified as reset.
 */
static void test_control_chars_are_reset()
{
    ensure_tables_initialized();
    
    // Control characters 0-32 should be ukcReset
    for (unsigned char c = 0; c < 32; c++) {
        g_assert_cmpint(UkcMap[c], ==, ukcReset);
    }
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/ukengine/inputproc/lowercase-vowels-are-vn", test_lowercase_vowels_are_vn);
    g_test_add_func("/ukengine/inputproc/uppercase-vowels-are-vn", test_uppercase_vowels_are_vn);
    g_test_add_func("/ukengine/inputproc/consonants-are-vn", test_consonants_are_vn);
    g_test_add_func("/ukengine/inputproc/non-vn-letters", test_non_vn_letters);
    g_test_add_func("/ukengine/inputproc/digits-are-non-vn", test_digits_are_non_vn);
    g_test_add_func("/ukengine/inputproc/control-chars-are-reset", test_control_chars_are_reset);

    return g_test_run();
}
