#pragma once
#include "frontend/musk_parser.hpp"

void generate_parser(const std::string& filepath); // generate the parser file from internal table structure
void generate_headers(musk_ptr); // generate related header files to expose the API
void define_macros(musk_ptr); // define internal macros
void define_symbols(musk_ptr); // define symbol types as integral values

void generate_table(musk_ptr); // actually take an input parser and generate the table
