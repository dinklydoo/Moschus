#include "parse_gen.hpp"
#include "frontend/musk_tokens.hpp"
#include "../errors/moschus_error.hpp"
#include <iostream>
#include <fstream>

MuskTokenStream lex_tokens(const char* path){
  MuskTokenStream tok_stream;

  char fpath[256];
  snprintf(fpath, 256, "%s", path);

  FILE* f = fopen(fpath, "r");

  yyin = f;

  MuskToken token;
  while ((token = yylex()).type != mtt::MUSK_EOF) {
    tok_stream.push_back(token);
  }
  tok_stream.push_back(token);

  fclose(f);
  yylex_destroy();

  return tok_stream;
}

void generate_parser(const std::string& musk_path, const std::string& output_file) {
  const MuskTokenStream tok_stream = lex_tokens((const char*)musk_path.data());
  musk_ptr musk_ast = parse_musk(tok_stream);

  // output file is always non-empty, guarantee by main

  // prepare .cpp file for writing
  std::ifstream out_cpp_fd(output_file+".cpp", std::ios::trunc); // truncate
  if (!out_cpp_fd.is_open()){
    throw MoschusError("Failed to open/create moschus parser .cpp file:"+output_file+".cpp\n", MoschusErrorType::FileIOError);
  }
  out_cpp_fd.close();
  out_cpp_fd.open(output_file+".cpp", std::ios::app); // re-open for append

  // prepare .hpp file for writing
  std::ifstream out_hpp_fd(output_file+".hpp", std::ios::trunc); // truncate
  if (!out_hpp_fd.is_open()){
    throw MoschusError("Failed to open/create moschus parser header file:"+output_file+".hpp\n", MoschusErrorType::FileIOError);
  }
  out_hpp_fd.close();
  out_hpp_fd.open(output_file+".hpp", std::ios::app);


}
