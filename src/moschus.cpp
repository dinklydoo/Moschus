#include "musk_template.hpp"
#include <argparse/argparse.hpp>

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("moschus");
    program.add_argument("-t", "--template")
    .help("generate a template .musk file")
    .nargs(1);

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    const std::string& input = program.get<std::string>("template");
    generate_musk_template(input);

    return 0;
}