#include "parse_gen.hpp"
#include "frontend/musk_tokens.hpp"
#include "frontend/musk_lexer.hpp"

#include "preprocess.hpp"
#include "table_gen.hpp"
#include "code_gen.hpp"

MuskTokenStream lex_tokens(const char* path){
  MuskTokenStream tok_stream;

  char fpath[256];
  snprintf(fpath, 256, "%s", path);

  FILE* f = fopen(fpath, "r");

  yyin = f;

  MuskToken token;
  while ((token = yylex()).type != mtt::MUSK_EOF) {
    tok_stream.push_back(token);
  }
  tok_stream.push_back(token);

  fclose(f);
  yylex_destroy();

  return tok_stream;
}

void generate_parser(const std::string& musk_path, const std::string& output_dir) {
  const MuskTokenStream tok_stream = lex_tokens((const char*)musk_path.data());
  musk_ptr musk_ast = parse_musk(musk_path, tok_stream);

  ProductionProcesser::process_musk_ast(musk_ast);
  ParseTable::generate_parse_table(musk_ast);
  CodeGenerator::generate_parser(output_dir, musk_ast);
}
