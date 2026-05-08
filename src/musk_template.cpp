#include "musk_template.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <format>

void generate_musk_template(const std::string& path){
    std::string musk_path = std::format("./{:s}.musk", path);

    std::ifstream musk_temp("../assets/template.musk", std::ios::binary | std::ios::in);

    if (std::filesystem::exists(musk_path)){
        std::string overwrite_msg = std::format(
            "File '{:s}' already exists,\n"
            "\n\n"
            "Overwrite?\n"
            "\t[Y] -- overwrite\n"
            "\t[N] -- abort", musk_path
        );

        std::cout << overwrite_msg << std::endl;

        char input;
        std::cin >> input;

        if (input == 'N'){
            static std::string unkown_cmd = "Template generate aborted";
            std::cout << unkown_cmd << std::endl;

            musk_temp.close();
            return;
        }
        else if (input != 'Y') {
            std::string unkown_cmd = std::format("Unkown command, '{:c}' template generate aborted", input);
            std::cout << unkown_cmd << std::endl;

            musk_temp.close();
            return;
        }
    }
    std::ofstream musk_file(musk_path, std::ios::binary | std::ios::trunc);
    musk_file << musk_temp.rdbuf();
    
    musk_file.close();
    musk_temp.close();
}

