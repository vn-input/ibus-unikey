cmake_minimum_required (VERSION 3.0)
project (ibus-unikey)

cmake_policy(SET CMP0054 NEW)

add_compile_options(-std=c++11 -Werror)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(GNUInstallDirs)

set(LIBEXECDIR "${CMAKE_INSTALL_FULL_LIBEXECDIR}" CACHE string "where to install ibus-{engine,setup}-unikey")
set(PKGDATADIR "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}")
set(LOCALEDIR "${CMAKE_INSTALL_FULL_LOCALEDIR}")

set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -L ${LIBEXECDIR}")

add_definitions(
    -DPACKAGE_NAME="${PROJECT_NAME}"
    -DGETTEXT_PACKAGE="${PROJECT_NAME}"
    -DPACKAGE_VERSION="0.7.0"
    -DPACKAGE_BUGREPORT="https://github.com/vn-input/ibus-unikey/issues"
    -DLOCALEDIR="${LOCALEDIR}"
    -DPKGDATADIR="${PKGDATADIR}"
    -DLIBEXECDIR="${LIBEXECDIR}"
)

include_directories(
    ${PROJECT_SOURCE_DIR}/ukengine
    ${PROJECT_SOURCE_DIR}/src/config
)

add_subdirectory(ukengine)
add_subdirectory(src)
add_subdirectory(setup)
add_subdirectory(data)
add_subdirectory(po)

