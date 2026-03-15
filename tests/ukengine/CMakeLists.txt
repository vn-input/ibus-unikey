# Tests for ukengine core functionality
# Phase 1 of legacy cleanup: regression tests for engine behavior

find_package(PkgConfig)
pkg_check_modules(GLIB_TEST REQUIRED glib-2.0)

# Test: Input processor character classification
add_executable(test-inputproc-classification
    inputproc_classification_test.cpp
)

target_include_directories(test-inputproc-classification PRIVATE
    ${GLIB_TEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/ukengine
)

target_link_libraries(test-inputproc-classification
    ukengine
    ${GLIB_TEST_LIBRARIES}
)

add_test(
    NAME inputproc-classification-test
    COMMAND test-inputproc-classification
)

# Test: Word break detection
add_executable(test-inputproc-wordbreak
    inputproc_wordbreak_test.cpp
)

target_include_directories(test-inputproc-wordbreak PRIVATE
    ${GLIB_TEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/ukengine
)

target_link_libraries(test-inputproc-wordbreak
    ukengine
    ${GLIB_TEST_LIBRARIES}
)

add_test(
    NAME inputproc-wordbreak-test
    COMMAND test-inputproc-wordbreak
)
