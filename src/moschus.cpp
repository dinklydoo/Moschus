#include "musk_template.hpp"
#include <argparse/argparse.hpp>
#include "errors/moschus_error.hpp"
#include "parser/parse_gen.hpp"

int main(int argc, char* argv[]) {
    argparse::ArgumentParser program("moschus");
    program.add_argument("-t", "--template")
    .help("generate a template .musk file")
    .nargs(1);

    program.add_argument("-g", "--generate")
    .help("generate the parser from the specified .musk file")
    .nargs(1);

    program.add_argument("-o", "--output")
    .help("specify moschus output filename and path")
    .nargs(1);

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    // Check if only generating a .musk template file
    if (program.is_used("--template")) {
      const std::string& input = program.get<std::string>("template");
      generate_musk_template(input);
      return 0;
    }

    // Validate that output dir is called with parser generation
    if (program.is_used("--output")) {
      if (!program.is_used("--generate")) {
        std::cerr << "--output flag must be used in conjunction with Moschus generate mode" << std::endl;
        return 1;
      }
    }

    // Call the generator
    if (program.is_used("--generate")) {
      const std::string &musk_file = program.get<std::string>("generate");
      if (!musk_file.ends_with(".musk")) {
        std::cerr << "source file is not a .musk file" << std::endl;
        return 1;
      }

      std::filesystem::path musk_path = musk_file;
      std::string output_dir = musk_path.parent_path();
      if (program.is_used("--output")) {
        output_dir = program.get<std::string>("output");
      }

      try {
        generate_parser(musk_file ,output_dir);
      } catch (const MoschusError &e) {
        // TODO log error or smthing -> based on type
        return -1;
      }

      return 0;
    }

    std::cerr << "moschus called with no flags? exiting now" << std::endl;
    return 1;
}
