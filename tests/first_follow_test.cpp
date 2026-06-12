#include <gtest/gtest.h>
#include <unordered_map>
#include "test_builder.hpp"

#include "../src/errors/moschus_error.hpp"
#include "../src/parser/frontend/musk_parser.hpp"
#include "../src/parser/preprocess.hpp"


TEST(PROCESSER, FF_EXAMPLE){
    MuskTokenStream tok_stream = TestBuilder::build_tok_s("example.musk");
    musk_ptr ast = parse_musk(tok_stream);

    ProductionProcesser::process_musk_ast(ast);
    
    std::string GLOBAL_FIRST = "INT";
    std::unordered_map<std::string, std::string> GLOBAL_FOLLOW = {
        {"CALC_EXP", "__[EOF]__"}, // this might be wrong
        {"ADD_EXP", "__[EOF]__"},
        {"SUB_EXP", "ADD"},
        {"MUL_EXP", "SUB"},
        {"DIV_EXP", "MUL"},
    };

    auto ALIAS_FIRST = ProductionProcesser::alias_.get_alias(GLOBAL_FIRST, true);
    std::unordered_map<ProductionItem, ProductionItem> ALIAS_FOLLOW;
    
    for (auto& follow_p : GLOBAL_FOLLOW){
        ALIAS_FOLLOW.emplace(
            ProductionProcesser::alias_.get_alias(follow_p.first, false),
            ProductionProcesser::alias_.get_alias(follow_p.second, true)
        );
    }

    for (auto& alias_p : ALIAS_FOLLOW){
        ProductionObject& object = ProductionProcesser::store_.get_object(alias_p.first);
        
        const auto& FIRST = object.get_FIRST();
        const auto& FOLLOW = object.get_FOLLOW();

        ASSERT_EQ(FIRST.size(), 1); // single first token
        ASSERT_EQ(FOLLOW.size(), 1); // single follow token

        ASSERT_EQ(*FIRST.begin(), ALIAS_FIRST);
        ASSERT_EQ(*FOLLOW.begin(), alias_p.second);
    }
}
