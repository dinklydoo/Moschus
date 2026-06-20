#include <gtest/gtest.h>
#include "test_builder.hpp"
#include "../src/parser/table_gen.hpp"

TEST(TABLE, EXAMPLE){
    ParseTable::_table.clear();

    musk_ptr ast = TestBuilder::build_ast("example.musk");
    ProductionProcesser::process_musk_ast(ast);
    ParseTable::generate_parse_table(ast);
    EXPECT_NO_THROW();
}

TEST(TABLE, NULLABLE){
    ParseTable::_table.clear();

    musk_ptr ast = TestBuilder::build_ast("nullable.musk");
    ProductionProcesser::process_musk_ast(ast);
    ParseTable::generate_parse_table(ast);
    EXPECT_NO_THROW();
}