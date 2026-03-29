find_package(PkgConfig)
pkg_check_modules(GTK3_TEST REQUIRED gtk+-3.0 gio-2.0)

add_executable(test-macro-dialog-state
    macro_dialog_state_test.cpp
    ../../../src/setup/macro_dialog_state.cpp
)

target_include_directories(test-macro-dialog-state PRIVATE
    ${GTK3_TEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include/setup
    ${PROJECT_SOURCE_DIR}/include/config
    ${PROJECT_SOURCE_DIR}/include/ukengine
)

target_link_libraries(test-macro-dialog-state
    unikey-config
    ${GTK3_TEST_LIBRARIES}
)

add_test(
    NAME macro-dialog-state-test
    COMMAND test-macro-dialog-state ${PROJECT_BINARY_DIR}/gsettings
)

add_executable(test-macro-export-filename
    macro_export_filename_test.cpp
    ../../../src/setup/macro_export_filename.cpp
)

target_include_directories(test-macro-export-filename PRIVATE
    ${GTK3_TEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include/setup
)

target_link_libraries(test-macro-export-filename
    ${GTK3_TEST_LIBRARIES}
)

add_test(
    NAME macro-export-filename-test
    COMMAND test-macro-export-filename
)

add_executable(test-macro-file-io
    macro_file_io_test.cpp
    ../../../src/setup/macro_file_io.cpp
)

target_include_directories(test-macro-file-io PRIVATE
    ${GTK3_TEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include/setup
    ${PROJECT_SOURCE_DIR}/include/ukengine
)

target_link_libraries(test-macro-file-io
    ukengine
    ${GTK3_TEST_LIBRARIES}
)

add_test(
    NAME macro-file-io-test
    COMMAND test-macro-file-io ${PROJECT_SOURCE_DIR}/tests/UI/macros
)
