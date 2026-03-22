/**
 * @file inputproc_wordbreak_test.cpp
 * @brief Tests for word-break symbol detection in input processor.
 *
 * These tests verify that WordBreakSyms array and UkcMap correctly mark
 * word-breaking characters (delimiters) as ukcWordBreak.
 */

#include <glib.h>
#include "inputproc.h"

// External declarations from inputproc.cpp
extern UkCharType UkcMap[256];
extern unsigned char WordBreakSyms[];
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
 * @brief Test that common punctuation is classified as word-break.
 */
static void test_punctuation_is_wordbreak()
{
    ensure_tables_initialized();
    
    g_assert_cmpint(UkcMap[(unsigned char)','], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)';'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)':'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'.'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'!'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'?'], ==, ukcWordBreak);
}

/**
 * @brief Test that space is classified as word-break.
 */
static void test_space_is_wordbreak()
{
    ensure_tables_initialized();
    
    g_assert_cmpint(UkcMap[(unsigned char)' '], ==, ukcWordBreak);
}

/**
 * @brief Test that quotes are classified as word-break.
 */
static void test_quotes_are_wordbreak()
{
    ensure_tables_initialized();
    
    g_assert_cmpint(UkcMap[(unsigned char)'"'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'\''], ==, ukcWordBreak);
}

/**
 * @brief Test that brackets and braces are classified as word-break.
 */
static void test_brackets_are_wordbreak()
{
    ensure_tables_initialized();
    
    g_assert_cmpint(UkcMap[(unsigned char)'('], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)')'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'{'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'}'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'['], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)']'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'<'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'>'], ==, ukcWordBreak);
}

/**
 * @brief Test that math/comparison operators are classified as word-break.
 */
static void test_math_operators_are_wordbreak()
{
    ensure_tables_initialized();
    
    g_assert_cmpint(UkcMap[(unsigned char)'+'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'-'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'*'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'/'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'='], ==, ukcWordBreak);
}

/**
 * @brief Test that special symbols are classified as word-break.
 */
static void test_special_symbols_are_wordbreak()
{
    ensure_tables_initialized();
    
    g_assert_cmpint(UkcMap[(unsigned char)'@'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'#'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'$'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'%'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'&'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'_'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'\\'], ==, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'|'], ==, ukcWordBreak);
}

/**
 * @brief Test that tilde/caret/backtick are NOT word-break.
 *
 * These are intentionally excluded from WordBreakSyms to avoid
 * interfering with Vietnamese composition (tone marks).
 */
static void test_composition_chars_not_wordbreak()
{
    ensure_tables_initialized();
    
    // These should NOT be word-break because they're used in Vietnamese input
    g_assert_cmpint(UkcMap[(unsigned char)'~'], !=, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'^'], !=, ukcWordBreak);
    g_assert_cmpint(UkcMap[(unsigned char)'`'], !=, ukcWordBreak);
}

/**
 * @brief Test that all WordBreakSyms entries are marked as ukcWordBreak.
 */
static void test_wordbreaksyms_array_consistency()
{
    ensure_tables_initialized();
    
    // All characters in WordBreakSyms should be marked as ukcWordBreak
    unsigned char expected_breaks[] = {
        ',', ';', ':', '.', '"', '\'', '!', '?', ' ',
        '<', '>', '=', '+', '-', '*', '/', '\\',
        '_', '@', '#', '$', '%', '&', '(', ')', '{', '}', '[', ']', '|'
    };
    
    for (size_t i = 0; i < sizeof(expected_breaks); i++) {
        unsigned char c = expected_breaks[i];
        g_assert_cmpint(UkcMap[c], ==, ukcWordBreak);
    }
}

int main(int argc, char** argv)
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/ukengine/wordbreak/punctuation", test_punctuation_is_wordbreak);
    g_test_add_func("/ukengine/wordbreak/space", test_space_is_wordbreak);
    g_test_add_func("/ukengine/wordbreak/quotes", test_quotes_are_wordbreak);
    g_test_add_func("/ukengine/wordbreak/brackets", test_brackets_are_wordbreak);
    g_test_add_func("/ukengine/wordbreak/math-operators", test_math_operators_are_wordbreak);
    g_test_add_func("/ukengine/wordbreak/special-symbols", test_special_symbols_are_wordbreak);
    g_test_add_func("/ukengine/wordbreak/composition-chars-excluded", test_composition_chars_not_wordbreak);
    g_test_add_func("/ukengine/wordbreak/array-consistency", test_wordbreaksyms_array_consistency);

    return g_test_run();
}
