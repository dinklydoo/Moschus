#include <gtest/gtest.h>
#include "../src/errors/moschus_error.hpp"
#include "../src/parser/frontend/musk_lexer.hpp"
#include "../src/parser/frontend/musk_parser.hpp"

MuskTokenStream tok_stream;

void build_tok_s(const char* path){
    tok_stream.clear();

    char src[] = "tests/assets/";
    char fpath[256];
    snprintf(fpath, 256, "%s%s", src, path);

    FILE* f = fopen(fpath, "r");
    ASSERT_NE(f, nullptr);

    yyin = f;
    reset_loc();

    MuskToken token;
    while ((token = yylex()).type != mtt::MUSK_EOF) {
        tok_stream.push_back(token);
    }
    tok_stream.push_back(token);

    fclose(f);
    yylex_destroy();
}

TEST(PARSER, MUSK_FILE){
    build_tok_s("example.musk");

    musk_ptr ast = parse_musk(tok_stream);

    EXPECT_NE(ast, nullptr);
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


TEST(PARSER, MISSING_TOKEN){
    build_tok_s("missing_token.musk");

    EXPECT_THROW(parse_musk(tok_stream), MoschusError);
}

TEST(PARSER, MISSING_HEADER){
    build_tok_s("missing_header.musk");

    EXPECT_THROW(parse_musk(tok_stream), MoschusError);
}