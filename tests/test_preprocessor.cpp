#include <algorithm>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "pre_wgsl.hpp"

static const std::string test_shader_dir = TEST_SHADER_DIR;

static std::string normalize_newlines(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c != '\r') out.push_back(c);
    }
    return out;
}

TEST_CASE("passthrough") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(fn main() {
    var x : f32 = 1.0;
})";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out == src + "\n");
}

TEST_CASE("basic_include") {
    pre_wgsl::Options opts;
    opts.include_path = test_shader_dir;
    pre_wgsl::Preprocessor pp(opts);

    INFO("Running preprocessor on main_include.wgsl");

    std::string out = pp.preprocess_file(test_shader_dir + "main_include.wgsl");
    out = normalize_newlines(out);
    INFO("Preprocessor output:\n" + out);

    // Should contain content of both main_include.wgsl and included file
    REQUIRE(out.find("// main_include.wgsl") != std::string::npos);
    REQUIRE(out.find("// from include_a.wgsl") != std::string::npos);
    REQUIRE(out.find("let a_from_include") != std::string::npos);
}

TEST_CASE("recursive_include") {
    pre_wgsl::Options opts;
    opts.include_path = "tests/shaders";
    pre_wgsl::Preprocessor pp(opts);

    REQUIRE_THROWS_AS(pp.preprocess_file("recursive_a.wgsl"), std::runtime_error);
}

TEST_CASE("ifdef_defined") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#define ENABLE_FOO 1
#ifdef ENABLE_FOO
var foo_enabled : i32 = 1;
#else
var foo_enabled : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var foo_enabled : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var foo_enabled : i32 = 0;") == std::string::npos);
}

TEST_CASE("ifndef_undefined") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#ifndef DISABLE_BAR
var bar_disabled : i32 = 0;
#else
var bar_disabled : i32 = 1;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var bar_disabled : i32 = 0;") != std::string::npos);
    REQUIRE(out.find("var bar_disabled : i32 = 1;") == std::string::npos);
}

TEST_CASE("if_defined") {
    pre_wgsl::Preprocessor pp;

    // Extra spaces to test trimming
    const std::string src = R"(#define HAS_ALPHA 1
#if  defined( HAS_ALPHA)
var has_alpha : i32 = 1;
#else
var has_alpha : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var has_alpha : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var has_alpha : i32 = 0;") == std::string::npos);
}

TEST_CASE("if_defined_empty_value") {
    pre_wgsl::Preprocessor pp;

    // Extra spaces to test trimming
    const std::string src = R"(#define HAS_ALPHA
#if defined(HAS_ALPHA)
var has_alpha : i32 = 1;
#else
var has_alpha : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var has_alpha : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var has_alpha : i32 = 0;") == std::string::npos);
}

TEST_CASE("if_undefined") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#if defined HAS_BETA
var has_beta  : i32 = 1;
#else
var has_beta  : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var has_beta  : i32 = 0;") != std::string::npos);
    REQUIRE(out.find("var has_beta  : i32 = 1;") == std::string::npos);
}

TEST_CASE("if_not_defined") {
    pre_wgsl::Preprocessor pp;

    // Extra spaces to test trimming
    const std::string src = R"(#if ! defined(HAS_BETA)
var has_beta  : i32 = 1;
#else
var has_beta  : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var has_beta  : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var has_beta  : i32 = 0;") == std::string::npos);
}
TEST_CASE("if_arithmetic_equality") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#define NUM_THREADS 64
#define BLOCKS 4

#if (NUM_THREADS * BLOCKS) == 256
var product_256 : i32 = 1;
#else
var product_256 : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var product_256 : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var product_256 : i32 = 0;") == std::string::npos);
}

TEST_CASE("if_arithmetic_inequality") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#define NUM_THREADS 64
#define BLOCKS 4
#if (NUM_THREADS * BLOCKS) != 256
var product_not_256 : i32 = 1;
#else
var product_not_256 : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var product_not_256 : i32 = 0;") != std::string::npos);
    REQUIRE(out.find("var product_not_256 : i32 = 1;") == std::string::npos);
}

TEST_CASE("if_logical_and") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#define NUM_THREADS 64
#define BLOCKS 4
#define HIGH_QUALITY 1

#if (NUM_THREADS == 64) && (BLOCKS == 4) && HIGH_QUALITY
var combo_true : i32 = 1;
#else
var combo_true : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var combo_true : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var combo_true : i32 = 0;") == std::string::npos);
}

TEST_CASE("if_logical_or") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#define NUM_THREADS 64
#define BLOCKS 4

#if (NUM_THREADS == 32) || (BLOCKS == 1)
var combo_false : i32 = 1;
#else
var combo_false : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var combo_false : i32 = 0;") != std::string::npos);
    REQUIRE(out.find("var combo_false : i32 = 1;") == std::string::npos);
}

TEST_CASE("elif_nested_cond") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#define MODE 2
#define HIGH_QUALITY 1
#if MODE == 1
var selected_mode : i32 = 1;
#elif MODE == 2
var selected_mode : i32 = 2;
    #if HIGH_QUALITY
    var quality_level : i32 = 2;
    #else
    var quality_level : i32 = 1;
    #endif
#else
var selected_mode : i32 = 3;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    // Outer: MODE == 2, inner: HIGH_QUALITY == 1
    REQUIRE(out.find("var selected_mode : i32 = 2;") != std::string::npos);
    REQUIRE(out.find("var quality_level : i32 = 2;") != std::string::npos);

    REQUIRE(out.find("var selected_mode : i32 = 1;") == std::string::npos);
    REQUIRE(out.find("var selected_mode : i32 = 3;") == std::string::npos);
    REQUIRE(out.find("var quality_level : i32 = 1;") == std::string::npos);
}

TEST_CASE("unmatched_endif") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(var x : i32 = 1;
#endif
var y : i32 = 2;
)";

    REQUIRE_THROWS_AS(pp.preprocess(src), std::runtime_error);
}

TEST_CASE("unmatched_if") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#define FOO 1
#if FOO == 1
var x : i32 = 1;
)";

    REQUIRE_THROWS_AS(pp.preprocess(src), std::runtime_error);
}

TEST_CASE("unknown_directive") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(var x : i32 = 1;
#pragma something
var y : i32 = 2;
)";

    REQUIRE_THROWS_AS(pp.preprocess(src), std::runtime_error);
}

TEST_CASE("define_expansion_in_code") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#define WORKGROUP_SIZE 256
#define PI 3.14159

@compute @workgroup_size(WORKGROUP_SIZE)
fn main() {
    let radius : f32 = 10.0;
    let area : f32 = PI* radius * radius;
    var threads : i32 = WORKGROUP_SIZE;
}
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    INFO("Preprocessor output:\n" + out);

    // Macros should be expanded in code outside directives
    REQUIRE(out.find("@workgroup_size(256)") != std::string::npos);
    REQUIRE(out.find("let area : f32 = 3.14159* radius * radius;") != std::string::npos);
    REQUIRE(out.find("var threads : i32 = 256;") != std::string::npos);

    // The macro names themselves should not appear in the output
    REQUIRE(out.find("WORKGROUP_SIZE") == std::string::npos);
    REQUIRE(out.find("PI") == std::string::npos);
}

TEST_CASE("options_macro_simple") {
    pre_wgsl::Options opts;
    opts.macros = {"FOO"};
    pre_wgsl::Preprocessor pp(opts);

    const std::string src = R"(#ifdef FOO
var foo_defined : i32 = 1;
#else
var foo_defined : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var foo_defined : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var foo_defined : i32 = 0;") == std::string::npos);
}

TEST_CASE("options_macro_with_value") {
    pre_wgsl::Options opts;
    opts.macros = {"BAR=42"};
    pre_wgsl::Preprocessor pp(opts);

    const std::string src = R"(#if BAR == 42
var bar_is_42 : i32 = 1;
#else
var bar_is_42 : i32 = 0;
#endif
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out.find("var bar_is_42 : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var bar_is_42 : i32 = 0;") == std::string::npos);
}

TEST_CASE("options_macro_complex_value") {
    pre_wgsl::Options opts;
    opts.macros = {"VEC_TYPE=vec4<u32>"};
    pre_wgsl::Preprocessor pp(opts);

    const std::string src = R"(var my_vector : VEC_TYPE = VEC_TYPE(1u, 2u, 3u, 4u);
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    INFO("Preprocessor output:\n" + out);

    REQUIRE(out.find("var my_vector : vec4<u32> = vec4<u32>(1u, 2u, 3u, 4u);") != std::string::npos);
    REQUIRE(out.find("VEC_TYPE") == std::string::npos);
}

TEST_CASE("options_macro_overrides_define") {
    pre_wgsl::Options opts;
    opts.macros = {"OVERRIDE=999"};
    pre_wgsl::Preprocessor pp(opts);

    const std::string src = R"(#define OVERRIDE 123
var value : i32 = OVERRIDE;
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    INFO("Preprocessor output:\n" + out);

    // Options macro should override the #define directive
    REQUIRE(out.find("var value : i32 = 999;") != std::string::npos);
    REQUIRE(out.find("var value : i32 = 123;") == std::string::npos);
}

TEST_CASE("options_multiple_macros") {
    pre_wgsl::Options opts;
    opts.macros = {"FOO", "BAR=1", "BAZ=vec4<u32>"};
    pre_wgsl::Preprocessor pp(opts);

    const std::string src = R"(#ifdef FOO
var foo_defined : i32 = BAR;
#endif
var my_vec : BAZ;
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    INFO("Preprocessor output:\n" + out);

    REQUIRE(out.find("var foo_defined : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var my_vec : vec4<u32>;") != std::string::npos);
}

TEST_CASE("options_macro_with_spaces") {
    pre_wgsl::Options opts;
    opts.macros = {"  SPACED  =  123  "};
    pre_wgsl::Preprocessor pp(opts);

    const std::string src = R"(var value : i32 = SPACED;
)";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    INFO("Preprocessor output:\n" + out);

    // Should handle spaces around macro name and value
    REQUIRE(out.find("var value : i32 = 123;") != std::string::npos);
}

TEST_CASE("per_call_macros_basic") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#ifdef FEATURE
var feature_enabled : i32 = 1;
#else
var feature_enabled : i32 = 0;
#endif
)";

    // First call with FEATURE defined
    std::string out1 = pp.preprocess(src, {"FEATURE"});
    out1 = normalize_newlines(out1);
    REQUIRE(out1.find("var feature_enabled : i32 = 1;") != std::string::npos);

    // Second call without FEATURE defined
    std::string out2 = pp.preprocess(src);
    out2 = normalize_newlines(out2);
    REQUIRE(out2.find("var feature_enabled : i32 = 0;") != std::string::npos);
}

TEST_CASE("per_call_macros_with_value") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(var size : i32 = SIZE;
)";

    // Call with different SIZE values
    std::string out1 = pp.preprocess(src, {"SIZE=10"});
    out1 = normalize_newlines(out1);
    REQUIRE(out1.find("var size : i32 = 10;") != std::string::npos);

    std::string out2 = pp.preprocess(src, {"SIZE=20"});
    out2 = normalize_newlines(out2);
    REQUIRE(out2.find("var size : i32 = 20;") != std::string::npos);
}

TEST_CASE("per_call_macros_override_per_file") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#define LOCAL 100
var local_value : i32 = LOCAL;
)";

    // Per-call macro should override #define in the file (per-call macros are predefined)
    std::string out = pp.preprocess(src, {"LOCAL=999"});
    out = normalize_newlines(out);

    // The per-call macro should win because it's predefined
    REQUIRE(out.find("var local_value : i32 = 999;") != std::string::npos);
}

TEST_CASE("global_and_per_call_macros") {
    pre_wgsl::Options opts;
    opts.macros = {"GLOBAL=100"};
    pre_wgsl::Preprocessor pp(opts);

    const std::string src = R"(var global_val : i32 = GLOBAL;
var local_val : i32 = LOCAL;
)";

    // Use both global and per-call macros
    std::string out = pp.preprocess(src, {"LOCAL=200"});
    out = normalize_newlines(out);

    REQUIRE(out.find("var global_val : i32 = 100;") != std::string::npos);
    REQUIRE(out.find("var local_val : i32 = 200;") != std::string::npos);
}

TEST_CASE("global_macros_should_be_overridden") {
    pre_wgsl::Options opts;
    opts.macros = {"OVERRIDEABLE=100"};
    pre_wgsl::Preprocessor pp(opts);

    const std::string src = R"(var value : i32 = OVERRIDEABLE;
)";

    // Override global macro with per-call macro
    std::string out = pp.preprocess(src, {"OVERRIDEABLE=999"});
    out = normalize_newlines(out);

    // Global macro value should be preserved
    REQUIRE(out.find("var value : i32 = 100;") == std::string::npos);
    REQUIRE(out.find("var value : i32 = 999;") != std::string::npos);
}

TEST_CASE("per_call_macros_multiple") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(#if defined(A) && defined(B)
var ab_value : i32 = A + B;
#endif
)";

    std::string out = pp.preprocess(src, {"A=10", "B=20"});
    out = normalize_newlines(out);

    REQUIRE(out.find("var ab_value : i32 = 10 + 20;") != std::string::npos);
}

TEST_CASE("per_call_macros_complex_value") {
    pre_wgsl::Preprocessor pp;

    const std::string src = R"(var my_vec : VEC_TYPE;
)";

    std::string out = pp.preprocess(src, {"VEC_TYPE=vec3<f32>"});
    out = normalize_newlines(out);

    REQUIRE(out.find("var my_vec : vec3<f32>;") != std::string::npos);
}

TEST_CASE("per_call_macros_persistent_across_calls") {
    pre_wgsl::Options opts;
    opts.macros = {"PERSISTENT=42"};
    pre_wgsl::Preprocessor pp(opts);

    const std::string src = R"(var val : i32 = PERSISTENT;
)";

    // First call with no per-call macros
    std::string out1 = pp.preprocess(src);
    out1 = normalize_newlines(out1);
    REQUIRE(out1.find("var val : i32 = 42;") != std::string::npos);

    // Second call with per-call macro - global should still work
    std::string out2 = pp.preprocess(src, {"OTHER=100"});
    out2 = normalize_newlines(out2);
    REQUIRE(out2.find("var val : i32 = 42;") != std::string::npos);

    // Third call - verify global macro still works
    std::string out3 = pp.preprocess(src);
    out3 = normalize_newlines(out3);
    REQUIRE(out3.find("var val : i32 = 42;") != std::string::npos);
}

