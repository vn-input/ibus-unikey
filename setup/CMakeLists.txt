find_package(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_executable(ibus-setup-unikey
    ../src/setup/main.cpp
    ../src/setup/app_context.cpp
    ../src/setup/macro_export_filename.cpp
    ../src/setup/macro_dialog_state.cpp
    ../src/setup/signal_handlers_settings.cpp
    ../src/setup/signal_handlers_window.cpp
    ../src/setup/signal_handlers_macros.cpp
    ../src/setup/macro_utils.cpp
    ../src/setup/macro_file_io.cpp
)
target_link_libraries(ibus-setup-unikey
    ukengine
    unikey-config
    ${GTK3_LIBRARIES}
)
target_link_options(ibus-setup-unikey PRIVATE -rdynamic)
target_include_directories(ibus-setup-unikey
    PUBLIC
        ${GTK3_INCLUDE_DIRS}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include/setup
)

install(TARGETS ibus-setup-unikey DESTINATION ${LIBEXECDIR})
install(FILES ibus-unikey.ui DESTINATION ${PKGDATADIR}/ui)

