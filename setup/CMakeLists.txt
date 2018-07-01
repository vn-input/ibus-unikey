find_package(PkgConfig)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

add_executable(ibus-setup-unikey
    main.cpp
    signal_handlers.cpp
    macro_utils.cpp
)
target_link_libraries(ibus-setup-unikey
    ukengine
    unikey-config
    ${GTK3_LIBRARIES}
)
target_include_directories(ibus-setup-unikey PUBLIC ${GTK3_INCLUDE_DIRS})

install(TARGETS ibus-setup-unikey DESTINATION ${LIBEXECDIR})
install(FILES ibus-unikey.ui DESTINATION ${PKGDATADIR}/ui)

