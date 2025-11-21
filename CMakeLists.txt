cmake_minimum_required(VERSION 3.17)
project("pre-wgsl" CXX)

add_library(pre-wgsl INTERFACE)
target_include_directories(pre-wgsl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(pre-wgsl INTERFACE cxx_std_17)

# Only enable tests when this is the top-level project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(PRE_WGSL_BUILD_TESTS "Build pre-wgsl tests" ON)
    if (PRE_WGSL_BUILD_TESTS)
        add_subdirectory(tests)
        enable_testing()
    endif()
endif()