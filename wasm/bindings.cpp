#include <emscripten/bind.h>
#include "pre_wgsl.hpp"
#include <string>
#include <vector>

using namespace emscripten;
using namespace pre_wgsl;

// Embind declarations
EMSCRIPTEN_BINDINGS(pre_wgsl_module) {
    value_object<Options>("Options")
        .field("includePath", &Options::include_path)
        .field("macros", &Options::macros);

    register_vector<std::string>("VectorString");

    class_<Preprocessor>("PreWGSL")
        .constructor<>()
        .constructor<Options>()
        .function("preprocess", &Preprocessor::preprocess);
}
