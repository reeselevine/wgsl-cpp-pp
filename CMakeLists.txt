cmake_minimum_required(VERSION 3.17)
project("wgslpp" CXX)

add_library(wgslpp INTERFACE)
target_include_directories(wgslpp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(wgslpp INTERFACE cxx_std_17)

# Only enable tests/cli when this is the top-level project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    option(WGSLPP_BUILD_TESTS "Build wgslpp tests" ON)
    if (WGSLPP_BUILD_TESTS)
        add_subdirectory(tests)
        enable_testing()
    endif()

    option(WGSLPP_BUILD_CLI "Build wgslpp CLI tool" ON)
    if (WGSLPP_BUILD_CLI)
      add_subdirectory(cli)
    endif()
endif()