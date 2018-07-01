find_package(PkgConfig)
pkg_check_modules(IBUS REQUIRED ibus-1.0)

add_subdirectory(config)

add_executable(ibus-engine-unikey
    main.cpp
    engine.cpp
)
target_link_libraries(ibus-engine-unikey
    ukengine
    unikey-config
    ${IBUS_LIBRARIES}
)
target_include_directories(ibus-engine-unikey PUBLIC ${IBUS_INCLUDE_DIRS})

configure_file(unikey.xml.in unikey.xml)

install(TARGETS ibus-engine-unikey DESTINATION ${LIBEXECDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/unikey.xml DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/ibus/component")
