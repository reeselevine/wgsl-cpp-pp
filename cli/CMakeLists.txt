add_executable(wgslpp-cli main.cpp)
target_link_libraries(wgslpp-cli PRIVATE wgslpp)
target_compile_features(wgslpp-cli PRIVATE cxx_std_17)