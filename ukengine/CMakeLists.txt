set(SOURCES
    ../src/ukengine/byteio.cpp
    ../src/ukengine/charset.cpp
    ../src/ukengine/convert.cpp
    ../src/ukengine/data.cpp
    ../src/ukengine/error.cpp
    ../src/ukengine/inputproc.cpp
    ../src/ukengine/mactab.cpp
    ../src/ukengine/pattern.cpp
    ../src/ukengine/ukengine_tables.cpp
    ../src/ukengine/ukengine_roof.cpp
    ../src/ukengine/ukengine_mapchar.cpp
    ../src/ukengine/ukengine_process.cpp
    ../src/ukengine/usrkeymap.cpp
    ../src/ukengine/unikey.cpp
)

add_library(ukengine STATIC ${SOURCES})
set_target_properties(ukengine PROPERTIES COMPILE_FLAGS "-funsigned-char -Wno-invalid-source-encoding")

target_include_directories(ukengine
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include/ukengine
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src/ukengine
)

