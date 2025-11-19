#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "wgslpp.hpp"

void print_usage() {
    std::cout << "Usage: wgslpp-cli <input.wgsl> [-I include_path] [-o output.wgsl]\n";
}

int main(int argc, char** argv) {
    if (argc < 2 || std::string(argv[1]) == "-h") {
        print_usage();
        return 0;
    }

    std::string input = argv[1];
    std::string output;

    wgslpp::Options opts;
    opts.include_path = {};

    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-o" && i + 1 < argc) {
            output = argv[++i];
        } else if (arg == "-I" && i + 1 < argc) {
            opts.include_path = argv[++i];
        } else if (arg == "-h") {
            print_usage();
            return 0;
        }
    }

    try {
        wgslpp::Preprocessor pp(opts);
        std::string result = pp.run(input);

        if (!output.empty()) {
            std::ofstream f(output);
            f << result;
        } else {
            std::cout << result;
        }
    } catch (const std::exception& e) {
        std::cerr << "wgslpp error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
