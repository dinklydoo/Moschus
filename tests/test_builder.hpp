#pragma once

#include "../src/parser/frontend/musk_lexer.hpp"
#include "../src/parser/frontend/musk_parser.hpp"

struct TestBuilder {

  static std::string musk_test_path(const std::string& path){
    return "tests/assets/"+path;
  }

  static MuskTokenStream build_tok_s(const char* path){
    MuskTokenStream tok_stream;

    std::string fpath = musk_test_path(path);

    FILE* f = fopen(fpath.data(), "r");
    yyin = f;
    reset_loc();

    MuskToken token;
    while ((token = yylex()).type != mtt::MUSK_EOF) {
        tok_stream.push_back(token);
    }
    tok_stream.push_back(token);

    fclose(f);
    yylex_destroy();

    return tok_stream;
  }

  static musk_ptr build_ast(const char* path){
    MuskTokenStream tok_s = build_tok_s(path);
    return parse_musk(musk_test_path(path), tok_s);
  }
};