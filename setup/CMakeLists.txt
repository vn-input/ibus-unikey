find_package(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_executable(ibus-setup-unikey
    main.cpp
    app_context.cpp
    macro_export_filename.cpp
    macro_dialog_state.cpp
    signal_handlers.cpp
    macro_utils.cpp
    macro_file_io.cpp
)
target_link_libraries(ibus-setup-unikey
    ukengine
    unikey-config
    ${GTK3_LIBRARIES}
)
target_link_options(ibus-setup-unikey PRIVATE -rdynamic)
target_include_directories(ibus-setup-unikey PUBLIC ${GTK3_INCLUDE_DIRS})

install(TARGETS ibus-setup-unikey DESTINATION ${LIBEXECDIR})
install(FILES ibus-unikey.ui DESTINATION ${PKGDATADIR}/ui)

