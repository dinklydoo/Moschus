#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include "../src/parser/frontend/musk_lexer.hpp"
#include "../src/parser/frontend/musk_parser.hpp"

static MuskTokenStream tok_stream;

void build_tok_s(const char* path){
    tok_stream.clear();

    char src[] = "tests/assets/";
    
    char fpath[128];
    snprintf(fpath, 128, "%s%s", src, path);

    yyin = fopen(fpath , "r");
    ASSERT_NE(yyin, nullptr);
    MuskToken token;

    reset_loc();
    while ((token = yylex()).type != mtt::MUSK_EOF){
        tok_stream.push_back(token);
    }
    tok_stream.push_back(token);
}

TEST(PARSER, MUSK_FILE){
    build_tok_s("example.musk");

    musk_ptr ast = parse_musk(tok_stream);

    EXPECT_NE(ast, nullptr);
}

TEST(PARSER, TOKEN){
    build_tok_s("missing_token.musk");

    EXPECT_DEATH(parse_musk(tok_stream), ".*");
}

TEST(PARSER, INCLUDE_OK){
    build_tok_s("include_ok.musk");

    musk_ptr ast = parse_musk(tok_stream);

    EXPECT_NE(ast, nullptr);
}

TEST(PARSER, UTILS_OK){
    build_tok_s("utils_ok.musk");

    musk_ptr ast = parse_musk(tok_stream);

    EXPECT_NE(ast, nullptr);
}

TEST(PARSER, HEADER){
    build_tok_s("missing_header.musk");

    EXPECT_DEATH(parse_musk(tok_stream), ".*");
}