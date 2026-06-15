#include <gtest/gtest.h>
#include <unordered_map>
#include "test_builder.hpp"

#include "../src/errors/moschus_error.hpp"
#include "../src/parser/frontend/musk_parser.hpp"
#include "../src/parser/preprocess.hpp"

using exp_sets = std::unordered_map<std::string, std::set<std::string>>;
using al_exp_sets = std::unordered_map<ProductionItem, std::set<ProductionItem>>;

void test_FF_sets(const exp_sets& exp_FIRST, const exp_sets& exp_FOLLOW){
    al_exp_sets ALIAS_FIRST;
    al_exp_sets ALIAS_FOLLOW;

    auto CONSTRUCT_ALIAS_SET = [](al_exp_sets& ALIAS_SET, const exp_sets& SRC_SET){
        for (auto& non_term : SRC_SET){
            ProductionItem nt_alias = ProductionProcesser::alias_.get_alias(non_term.first, false);
            for (auto& term : non_term.second){
                ProductionItem t_alias = ProductionProcesser::alias_.get_alias(term, true);
                ALIAS_SET[nt_alias].insert(t_alias);
            }
        }
    };
    CONSTRUCT_ALIAS_SET(ALIAS_FIRST, exp_FIRST);
    CONSTRUCT_ALIAS_SET(ALIAS_FOLLOW, exp_FOLLOW);

    auto TEST_EXP_SET = [](al_exp_sets& ALIAS_SET, bool FIRST){
        for (auto& non_term : ALIAS_SET){
            ProductionItem nt_alias = non_term.first;
            ProductionObject& nt_object = ProductionProcesser::store_.get_object(nt_alias);

            const std::set<ProductionItem>& ACTUAL_SET = (FIRST)? nt_object.get_FIRST() : nt_object.get_FOLLOW();
            EXPECT_EQ(ACTUAL_SET.size(), ALIAS_SET[nt_alias].size()); // ensure ACTUAL set is the same size as EXP set
            for (ProductionItem exp_e : non_term.second){
                EXPECT_TRUE(ACTUAL_SET.contains(exp_e)); // ensure ACTUAL set holds all EXP items
            }
        }
    };
    TEST_EXP_SET(ALIAS_FIRST, true);
    TEST_EXP_SET(ALIAS_FOLLOW, false);
}

TEST(PROCESSER, FF_EXAMPLE){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("example.musk");
    musk_ptr ast = parse_musk(tok_stream);

    ProductionProcesser::process_musk_ast(ast);
    
    exp_sets EXP_FIRST = {
        {"CALC_EXP", {"INT"}},
        {"ADD_EXP", {"INT"}},
        {"SUB_EXP", {"INT"}},
        {"MUL_EXP", {"INT"}},
        {"DIV_EXP", {"INT"}}
    };
    exp_sets EXP_FOLLOW = {
        {"CALC_EXP", {}},
        {"ADD_EXP", {"__[EOF]__"}},
        {"SUB_EXP", {"ADD", "__[EOF]__"}},
        {"MUL_EXP", {"ADD", "SUB", "__[EOF]__"}},
        {"DIV_EXP", {"ADD", "SUB", "MUL", "__[EOF]__"}},
    };
    test_FF_sets(EXP_FIRST, EXP_FOLLOW);
}