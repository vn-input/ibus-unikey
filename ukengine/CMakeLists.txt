set(SOURCES
    ../src/ukengine/io/byteio.cpp
    ../src/ukengine/io/charset.cpp
    ../src/ukengine/mapping/convert.cpp
    ../src/ukengine/mapping/error.cpp
    ../src/ukengine/io/data.cpp
    ../src/ukengine/io/inputproc.cpp
    ../src/ukengine/macro/mactab.cpp
    ../src/ukengine/pattern/pattern.cpp
    ../src/ukengine/tables/ukengine_tables.cpp
    ../src/ukengine/utils/ukengine_utils.cpp
    ../src/ukengine/roof/ukengine_roof.cpp
    ../src/ukengine/mapping/ukengine_mapchar.cpp
    ../src/ukengine/core/ukengine.cpp
    ../src/ukengine/usrkeymap/usrkeymap.cpp
    ../src/ukengine/unikey/unikey.cpp
)

add_library(ukengine STATIC ${SOURCES})
set_target_properties(ukengine PROPERTIES COMPILE_FLAGS "-funsigned-char -Wno-invalid-source-encoding")

target_include_directories(ukengine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include/ukengine
)

