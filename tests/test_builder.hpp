#pragma once

#include "../src/parser/frontend/musk_lexer.hpp"

struct TestBuilder {
  static MuskTokenStream build_tok_s(const char* path){
    MuskTokenStream tok_stream;

    char src[] = "tests/assets/";
    char fpath[256];
    snprintf(fpath, 256, "%s%s", src, path);

    FILE* f = fopen(fpath, "r");
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
};