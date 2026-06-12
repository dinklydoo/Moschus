#include <gtest/gtest.h>
#include "test_builder.hpp"

#include "../src/parser/frontend/musk_tokens.hpp"

TEST(LEXER, MUSK_FILE){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("example.musk");

    MuskTTypeStream expected = {
        mtt::SECTION_INCLUDES,
        mtt::MUSK_BLOCK,
        mtt::SECTION_UTILS,
        mtt::MUSK_BLOCK,
        mtt::SECTION_TOK_OBJ,
        mtt::SECTION_TOK_TYPE,
        mtt::SECTION_DECL,
        mtt::TOK_DECL, // INT
        mtt::TERM_DECL,
        mtt::TOK_DECL, // ADD
        mtt::TERM_DECL,
        mtt::TOK_DECL, // SUB
        mtt::TERM_DECL,
        mtt::TOK_DECL, // MUL
        mtt::TERM_DECL,
        mtt::TOK_DECL, // DIV
        mtt::TERM_DECL,
        mtt::TYPE_DECL, // CALC_EXP
        mtt::TERM_DECL,
        mtt::TYPE_DECL, // ADD_EXP
        mtt::TERM_DECL,
        mtt::TYPE_DECL, // SUB_EXP
        mtt::TERM_DECL,
        mtt::TYPE_DECL, // MUL_EXP
        mtt::TERM_DECL,
        mtt::TYPE_DECL, // DIV_EXP
        mtt::TERM_DECL,
        mtt::SECTION_START,
        mtt::SECTION_PROD,
        mtt::TERM_DECL, // CALC_EXP prod
        mtt::PROD_SEP,
        mtt::TERM_DECL,
        mtt::PROD_EOF,
        mtt::CODE_BLOCK,
        mtt::PROD_END,
        mtt::TERM_DECL, // ADD_EXP prod
        mtt::PROD_SEP,
        mtt::TERM_DECL,
        mtt::TERM_DECL,
        mtt::TERM_DECL,
        mtt::CODE_BLOCK,
        mtt::PROD_SUM,
        mtt::TERM_DECL,
        mtt::CODE_BLOCK,
        mtt::PROD_END,
        mtt::TERM_DECL, // SUB_EXP prod
        mtt::PROD_SEP,
        mtt::TERM_DECL,
        mtt::TERM_DECL,
        mtt::TERM_DECL,
        mtt::CODE_BLOCK,
        mtt::PROD_SUM,
        mtt::TERM_DECL,
        mtt::CODE_BLOCK,
        mtt::PROD_END,
        mtt::TERM_DECL, // MUL_EXP prod
        mtt::PROD_SEP,
        mtt::TERM_DECL,
        mtt::TERM_DECL,
        mtt::TERM_DECL,
        mtt::CODE_BLOCK,
        mtt::PROD_SUM,
        mtt::TERM_DECL,
        mtt::CODE_BLOCK,
        mtt::PROD_END,
        mtt::TERM_DECL, // DIV_EXP prod
        mtt::PROD_SEP,
        mtt::TERM_DECL,
        mtt::TERM_DECL,
        mtt::TERM_DECL,
        mtt::CODE_BLOCK,
        mtt::PROD_SUM,
        mtt::TERM_DECL,
        mtt::CODE_BLOCK,
        mtt::PROD_END,
        mtt::MUSK_EOF
    };

    auto equals = [](const MuskTokenStream& out, const MuskTTypeStream& exp)->bool{
        if (out.size() != exp.size()){
            return false;
        }
        for (int i = 0; i < out.size(); i++){
            if (out[i].type != exp[i]){
                return false;
            }
        }
        return true;
    };

    EXPECT_TRUE(equals(tok_stream, expected));
}