cmake_minimum_required(VERSION 3.17)

include(FetchContent)

# Fetch Catch2 (v3)
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(catch2)

add_executable(pre_wgsl_tests
    test_preprocessor.cpp
)

# Pass absolute path of tests/shaders into the test executable
target_compile_definitions(pre_wgsl_tests PRIVATE
    TEST_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders/"
)

target_link_libraries(pre_wgsl_tests
    PRIVATE
        pre-wgsl
        Catch2::Catch2WithMain
)

target_compile_features(pre_wgsl_tests PRIVATE cxx_std_17)

include(CTest)
include(Catch)
catch_discover_tests(pre_wgsl_tests)
