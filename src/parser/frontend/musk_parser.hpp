#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "musk_tokens.hpp"

struct MuskAST;
using musk_ptr = std::unique_ptr<MuskAST>;

/* global definition of tokens, specifies toktype and objtype */
struct TokenProperties {
  std::string token_type;
  std::string token_object;
};

/* container for header code, include paths + user defined code utilities */
struct MuskHeader {
  std::string head_includes;
  std::string head_utils;

  bool header_valid() const{
    return !head_includes.empty() || !head_utils.empty();
  }
};

/* token declarations, just a token identifier (type is inferred from TokenProperties)*/
struct TokenDeclaration {
  std::string token_identifier;
};

/* non-term declarations, pair of non-terminal type and identifier */
struct NonTerminalDeclaration {
  std::string nt_type;
  std::string nt_identifier;
};

using RuleIdentifier = long long;

/* a single production rule of form A -> a..A, specifies lhs and rhs + action when match*/
struct ProductionRule {
  private:
    inline static RuleIdentifier _defined_rules = 0;
  public:
    std::string nt_base;
    std::string prod_action;

    std::vector<std::string> nt_prods;
    RuleIdentifier rule_identifier;

    ProductionRule(const std::string& _base) : nt_base(_base), rule_identifier(_defined_rules++) {};
};

/* tree representation of our .musk file */
struct MuskAST {
    TokenProperties tok_prop;
    MuskHeader musk_header;

    std::string start_nt;

    std::vector<TokenDeclaration> tok_decls;
    std::vector<NonTerminalDeclaration> nt_decls;

    std::unordered_map<std::string, std::vector<ProductionRule>> prod_rules;
    std::unordered_map<RuleIdentifier, ProductionRule> rules_by_id;
};

using tok_it = std::vector<MuskToken>::const_iterator;
using decl_pair = std::pair<std::vector<TokenDeclaration>, std::vector<NonTerminalDeclaration>>;
#define p_args tok_it&, tok_it

musk_ptr parse_musk(const MuskTokenStream&);

std::string parse_includes(p_args);
std::string parse_utilities(p_args);
TokenProperties parse_token_p(p_args);
decl_pair parse_decl(p_args);
std::string parse_start(p_args);
std::unordered_map<std::string, std::vector<ProductionRule>> parse_prod(p_args);
