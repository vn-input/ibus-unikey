set (SOURCES byteio.cpp
    charset.cpp
    convert.cpp
    data.cpp
    error.cpp
    inputproc.cpp
    mactab.cpp
    pattern.cpp
    ukengine.cpp
    usrkeymap.cpp
    unikey.cpp
)

add_library(ukengine STATIC ${SOURCES})
set_target_properties(ukengine PROPERTIES COMPILE_FLAGS "-funsigned-char -Wno-invalid-source-encoding")

