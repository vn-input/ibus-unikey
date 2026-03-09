find_package(PkgConfig)
pkg_check_modules(GTK3_TEST REQUIRED gtk+-3.0 gio-2.0)

add_executable(test-macro-dialog-state
    macro_dialog_state_test.cpp
    ../../../setup/macro_dialog_state.cpp
)

target_include_directories(test-macro-dialog-state PRIVATE
    ${GTK3_TEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/setup
    ${PROJECT_SOURCE_DIR}/src/config
    ${PROJECT_SOURCE_DIR}/ukengine
)

target_link_libraries(test-macro-dialog-state
    unikey-config
    ${GTK3_TEST_LIBRARIES}
)

add_test(
    NAME macro-dialog-state-test
    COMMAND test-macro-dialog-state ${PROJECT_BINARY_DIR}/gsettings
)
