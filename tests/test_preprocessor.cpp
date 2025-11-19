#include <algorithm>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "wgslpp.hpp"

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
    wgslpp::Preprocessor pp;

    const std::string src = R"(fn main() {
    var x : f32 = 1.0;
})";

    std::string out = pp.preprocess(src);
    out = normalize_newlines(out);

    REQUIRE(out == src + "\n");
}

TEST_CASE("basic_include") {
    wgslpp::Options opts;
    opts.include_path = test_shader_dir;
    wgslpp::Preprocessor pp(opts);

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
    wgslpp::Options opts;
    opts.include_path = "tests/shaders";
    wgslpp::Preprocessor pp(opts);

    REQUIRE_THROWS_AS(pp.preprocess_file("recursive_a.wgsl"), std::runtime_error);
}

TEST_CASE("ifdef_defined") {
    wgslpp::Preprocessor pp;

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
    wgslpp::Preprocessor pp;

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
    wgslpp::Preprocessor pp;

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

TEST_CASE("if_undefined") {
    wgslpp::Preprocessor pp;

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
    wgslpp::Preprocessor pp;

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
    wgslpp::Preprocessor pp;

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
    wgslpp::Preprocessor pp;

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
    wgslpp::Preprocessor pp;

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
    wgslpp::Preprocessor pp;

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
    wgslpp::Preprocessor pp;

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

// TODO: tests for define with no value, missing #endif