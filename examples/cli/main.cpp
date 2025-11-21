#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "pre_wgsl.hpp"

void print_usage() {
    std::cout << "Usage: pre-wgsl-cli <input.wgsl> [-I include_path] [-D MACRO[=value]] [-o output.wgsl]\n";
    std::cout << "Options:\n";
    std::cout << "  -I <path>      Set include path for #include directives\n";
    std::cout << "  -D <macro>     Define a macro (e.g., -D FOO or -D BAR=1)\n";
    std::cout << "  -o <output>    Write output to file instead of stdout\n";
}

int main(int argc, char** argv) {
    if (argc < 2 || std::string(argv[1]) == "-h") {
        print_usage();
        return 0;
    }

    std::string input = argv[1];
    std::string output;

    pre_wgsl::Options opts;
    opts.include_path = ".";

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-o" && i + 1 < argc) {
            output = argv[++i];
        } else if (arg == "-I" && i + 1 < argc) {
            opts.include_path = argv[++i];
        } else if (arg == "-D" && i + 1 < argc) {
            opts.macros.push_back(argv[++i]);
        } else if (arg == "-h") {
            print_usage();
            return 0;
        }
    }

    try {
        pre_wgsl::Preprocessor pp(opts);
        std::string result = pp.preprocess_file(input);

        if (!output.empty()) {
            std::ofstream f(output);
            f << result;
        } else {
            std::cout << result;
        }
    } catch (const std::exception& e) {
        std::cerr << "pre-wgsl error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
