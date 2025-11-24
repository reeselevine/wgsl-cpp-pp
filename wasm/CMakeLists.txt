cmake_minimum_required(VERSION 3.17)
project(pre-wgsl-wasm)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Create the WASM module
add_executable(pre-wgsl
    bindings.cpp
)

set_target_properties(pre-wgsl PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

set(EMSCRIPTEN_LINK_FLAGS
    -lembind
    -sEXPORT_ES6=1
    -sMODULARIZE=1
    -sEXPORT_NAME=createPreWGSLModule
    -sALLOW_MEMORY_GROWTH=1
    -sENVIRONMENT=web,node
    -sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']
    -sNO_FILESYSTEM=1
    -O3
)

string(REPLACE ";" " " EMSCRIPTEN_LINK_FLAGS_STR "${EMSCRIPTEN_LINK_FLAGS}")
set_target_properties(pre-wgsl PROPERTIES
    LINK_FLAGS "${EMSCRIPTEN_LINK_FLAGS_STR}"
    SUFFIX ".mjs"
)

set_target_properties(pre-wgsl PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist
)
