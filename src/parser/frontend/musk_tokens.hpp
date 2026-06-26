#pragma once
#include <string>
#include <vector>

#define mtt MuskTokenType

struct MuskToken;
enum class MuskTokenType;

using MuskTokenStream = std::vector<MuskToken>;
using MuskTTypeStream = std::vector<MuskTokenType>;

struct Location {
  int line = 1;
  int col = 1;
};

enum class MuskTokenType {
SECTION_INCLUDES, // @includes
SECTION_UTILS, // @utilities
SECTION_TOK_OBJ, // @token_obj
SECTION_TOK_TYPE, // @token_type
SECTION_DECL, // @declarations
SECTION_START, // @start
SECTION_PROD, // @productions

L_CRB, R_CRB, L_SQB, R_SQB, // tokens for .musk blocks

MUSK_BLOCK,
CODE_BLOCK,

TOK_DECL, // token
TYPE_DECL, // loose match of words
TERM_DECL, // stricter form of type decl

PROD_SUM, // production sum |
PROD_SEP, // production seperator :
PROD_END, // production end ;

MUSK_EOF // literal end of input token
};

struct MuskToken {
  MuskTokenType type;
  std::string internal;

  Location start_loc;
  Location end_loc;

  MuskToken() = default;
  MuskToken(mtt type, Location sloc, Location eloc) : type(type), start_loc(sloc), end_loc(eloc) {}
  MuskToken(mtt type, std::string strbuf, Location sloc, Location eloc) : type(type), internal(strbuf), start_loc(sloc), end_loc(eloc) {}
};