# ============================================================================
# Example: How to integrate pre-wgsl into your C++ CMake project
# ============================================================================

cmake_minimum_required(VERSION 3.17)
project(pre-wgsl-example CXX)


# ----------------------------------------------------------------------------
# Method 1: add_subdirectory
# ----------------------------------------------------------------------------
add_subdirectory(../.. build)

# ----------------------------------------------------------------------------
# Method 2: FetchContent
# ----------------------------------------------------------------------------
# include(FetchContent)
#
# FetchContent_Declare(
#     pre-wgsl
#     GIT_REPOSITORY https://github.com/reeselevine/pre-wgsl.git
#     GIT_TAG main
# )
#
# FetchContent_MakeAvailable(pre-wgsl)

# ----------------------------------------------------------------------------
# Method 3: find_package
# ----------------------------------------------------------------------------
# find_package(pre-wgsl REQUIRED)

add_executable(pre-wgsl-cli main.cpp)
target_link_libraries(pre-wgsl-cli PRIVATE pre-wgsl)
target_compile_features(pre-wgsl-cli PRIVATE cxx_std_17)