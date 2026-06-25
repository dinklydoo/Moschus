#include <gtest/gtest.h>
#include "test_builder.hpp"

#include "../src/parser/code_gen.hpp"

TEST(CODEGEN, EXAMPLE){
  musk_ptr ast = TestBuilder::build_ast("example.musk");
  ProductionProcesser::process_musk_ast(ast);
  ParseTable::generate_parse_table(ast);

  EXPECT_NO_THROW(CodeGenerator::generate_parser("tests/out", ast));

  TestBuilder::reset();
}
