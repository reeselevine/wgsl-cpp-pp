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

TEST_CASE("Simple passthrough WGSL with no directives") {
    wgslpp::Preprocessor pp;

    const std::string src = R"(fn main() {
    var x : f32 = 1.0;
})";

    std::string out = pp.run_from_string(src);
    out = normalize_newlines(out);

    REQUIRE(out == src + "\n");
}

TEST_CASE("Include basic file") {
    wgslpp::Options opts;
    opts.include_path = test_shader_dir;
    wgslpp::Preprocessor pp(opts);

    INFO("Running preprocessor on main_include.wgsl");

    std::string out = pp.run(test_shader_dir + "main_include.wgsl");
    out = normalize_newlines(out);
    INFO("Preprocessor output:\n" + out);

    // Should contain content of both main_include.wgsl and included file
    REQUIRE(out.find("// main_include.wgsl") != std::string::npos);
    REQUIRE(out.find("// from include_a.wgsl") != std::string::npos);
    REQUIRE(out.find("let a_from_include") != std::string::npos);
}

TEST_CASE("Recursive include is rejected") {
    wgslpp::Options opts;
    opts.include_path = "tests/shaders";
    wgslpp::Preprocessor pp(opts);

    REQUIRE_THROWS_AS(pp.run("recursive_a.wgsl"), std::runtime_error);
}

TEST_CASE("#define and #ifdef / #ifndef") {
    wgslpp::Options opts;
    opts.include_path = test_shader_dir;
    wgslpp::Preprocessor pp(opts);

    std::string out = pp.run(test_shader_dir + "defined_ifdef.wgsl");
    out = normalize_newlines(out);

    // ENABLE_FOO is defined in the file
    REQUIRE(out.find("var foo_enabled : i32 = 1;") != std::string::npos);
    // DISABLE_BAR is *not* defined
    REQUIRE(out.find("var bar_disabled : i32 = 0;") != std::string::npos);
}

TEST_CASE("#if defined(NAME) and !defined(NAME)") {
    wgslpp::Options opts;
    opts.include_path = test_shader_dir;
    wgslpp::Preprocessor pp(opts);

    std::string out = pp.run(test_shader_dir + "defined_expr.wgsl");
    out = normalize_newlines(out);

    REQUIRE(out.find("var has_alpha : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var has_beta  : i32 = 0;") != std::string::npos);
}

TEST_CASE("Arithmetic and logical conditionals in #if") {
    wgslpp::Options opts;
    opts.include_path = test_shader_dir;
    wgslpp::Preprocessor pp(opts);

    std::string out = pp.run(test_shader_dir + "arith_if.wgsl");
    out = normalize_newlines(out);

    // NUM_THREADS is 64, BLOCKS is 4 → 64 * 4 = 256
    // Only the branch with 256 should be present
    REQUIRE(out.find("var product_256 : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var product_not_256 : i32 = 0;") != std::string::npos);

    // Combined logical conditions
    REQUIRE(out.find("var combo_true : i32 = 1;") != std::string::npos);
    REQUIRE(out.find("var combo_false : i32 = 0;") != std::string::npos);
}

TEST_CASE("Nested conditionals and #elif / #else") {
    wgslpp::Options opts;
    opts.include_path = test_shader_dir;
    wgslpp::Preprocessor pp(opts);

    std::string out = pp.run(test_shader_dir + "nested_if.wgsl");
    out = normalize_newlines(out);

    // Outer: MODE == 2, inner: HIGH_QUALITY == 1
    REQUIRE(out.find("var selected_mode : i32 = 2;") != std::string::npos);
    REQUIRE(out.find("var quality_level : i32 = 2;") != std::string::npos);

    // Ensure that unmatched branches are not present
    REQUIRE(out.find("var selected_mode : i32 = 1;") == std::string::npos);
    REQUIRE(out.find("var selected_mode : i32 = 3;") == std::string::npos);
}

// TODO: tests for define with no value, missing #endif