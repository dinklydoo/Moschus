#include <gtest/gtest.h>
#include "test_builder.hpp"

#include "../src/errors/moschus_error.hpp"
#include "../src/parser/frontend/musk_parser.hpp"

TEST(PARSER, MUSK_FILE){
    musk_ptr ast = TestBuilder::build_ast("example.musk");
    EXPECT_NE(ast, nullptr);
}

TEST(PARSER, INCLUDE_OK){
    musk_ptr ast = TestBuilder::build_ast("include_ok.musk");
    EXPECT_NE(ast, nullptr);
}

TEST(PARSER, UTILS_OK){
    musk_ptr ast = TestBuilder::build_ast("utils_ok.musk");
    EXPECT_NE(ast, nullptr);
}

TEST(PARSER, MISSING_TOKEN){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("missing_token.musk");
    EXPECT_THROW(parse_musk("../assets/missing_token.musk", tok_stream), MoschusError);
}

TEST(PARSER, MISSING_HEADER){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("missing_header.musk");
    EXPECT_THROW(parse_musk("../assets/missing_header.musk", tok_stream), MoschusError);
}