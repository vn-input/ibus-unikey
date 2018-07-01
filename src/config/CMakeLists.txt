FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(GIO REQUIRED gio-2.0)

add_library(unikey-config STATIC unikey_config.cpp)
TARGET_INCLUDE_DIRECTORIES(unikey-config PUBLIC ${GIO_INCLUDE_DIRS})

include(GSettings)
add_schemas(unikey-config ${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_INSTALL_PREFIX}")

