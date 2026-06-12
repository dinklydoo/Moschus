#include <gtest/gtest.h>
#include "test_builder.hpp"

#include "../src/errors/moschus_error.hpp"
#include "../src/parser/frontend/musk_parser.hpp"

TEST(PARSER, MUSK_FILE){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("example.musk");

    musk_ptr ast = parse_musk(tok_stream);

    EXPECT_NE(ast, nullptr);
}

TEST(PARSER, INCLUDE_OK){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("include_ok.musk");

    musk_ptr ast = parse_musk(tok_stream);

    EXPECT_NE(ast, nullptr);
}

TEST(PARSER, UTILS_OK){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("utils_ok.musk");

    musk_ptr ast = parse_musk(tok_stream);

    EXPECT_NE(ast, nullptr);
}


TEST(PARSER, MISSING_TOKEN){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("missing_token.musk");

    EXPECT_THROW(parse_musk(tok_stream), MoschusError);
}

TEST(PARSER, MISSING_HEADER){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("missing_header.musk");

    EXPECT_THROW(parse_musk(tok_stream), MoschusError);
}