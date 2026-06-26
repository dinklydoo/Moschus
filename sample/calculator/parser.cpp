#include "parser.hpp"

/* BEGIN .MUSK @utilities */

/* END .MUSK @utilities */

/* REDUCTION RULE FOR RULE 0 */
/* RULE FOR PRODUCTION:  TERM -> TERM MUL FACTOR  */
unsigned long long reduce_rule_0(MoschusParser& parser){
  auto $3 = std::get<int>(pop_term(parser).get_nonterminal());
  auto $2 = pop_term(parser).get_terminal();
  auto $1 = std::get<int>(pop_term(parser).get_nonterminal());
  auto self = [&](){
  /* BEGIN .MUSK RULE 0 CODE DEFINTION */

        return $1 * $3;
    
  /* END .MUSK RULE 0 CODE DEFINTION */
  };
  PROD_TERM term((NT_VARIANT)self());
  push_term(parser, term);
  return 8;
}

/* REDUCTION RULE FOR RULE 1 */
/* RULE FOR PRODUCTION:  TERM -> TERM DIV FACTOR  */
unsigned long long reduce_rule_1(MoschusParser& parser){
  auto $3 = std::get<int>(pop_term(parser).get_nonterminal());
  auto $2 = pop_term(parser).get_terminal();
  auto $1 = std::get<int>(pop_term(parser).get_nonterminal());
  auto self = [&](){
  /* BEGIN .MUSK RULE 1 CODE DEFINTION */

        return $1 / $3;
    
  /* END .MUSK RULE 1 CODE DEFINTION */
  };
  PROD_TERM term((NT_VARIANT)self());
  push_term(parser, term);
  return 8;
}

/* REDUCTION RULE FOR RULE 2 */
/* RULE FOR PRODUCTION:  TERM -> FACTOR  */
unsigned long long reduce_rule_2(MoschusParser& parser){
  auto $1 = std::get<int>(pop_term(parser).get_nonterminal());
  auto self = [&](){
  /* BEGIN .MUSK RULE 2 CODE DEFINTION */

        return $1;
    
  /* END .MUSK RULE 2 CODE DEFINTION */
  };
  PROD_TERM term((NT_VARIANT)self());
  push_term(parser, term);
  return 8;
}

/* REDUCTION RULE FOR RULE 3 */
/* RULE FOR PRODUCTION:  FACTOR -> LPAREN EXPR RPAREN  */
unsigned long long reduce_rule_3(MoschusParser& parser){
  auto $3 = pop_term(parser).get_terminal();
  auto $2 = std::get<int>(pop_term(parser).get_nonterminal());
  auto $1 = pop_term(parser).get_terminal();
  auto self = [&](){
  /* BEGIN .MUSK RULE 3 CODE DEFINTION */

        return $2;
    
  /* END .MUSK RULE 3 CODE DEFINTION */
  };
  PROD_TERM term((NT_VARIANT)self());
  push_term(parser, term);
  return 11;
}

/* REDUCTION RULE FOR RULE 4 */
/* RULE FOR PRODUCTION:  FACTOR -> INT  */
unsigned long long reduce_rule_4(MoschusParser& parser){
  auto $1 = pop_term(parser).get_terminal();
  auto self = [&](){
  /* BEGIN .MUSK RULE 4 CODE DEFINTION */

        return $1.get_value();
    
  /* END .MUSK RULE 4 CODE DEFINTION */
  };
  PROD_TERM term((NT_VARIANT)self());
  push_term(parser, term);
  return 11;
}

/* REDUCTION RULE FOR RULE 5 */
/* RULE FOR PRODUCTION:  EXPR -> EXPR ADD TERM  */
unsigned long long reduce_rule_5(MoschusParser& parser){
  auto $3 = std::get<int>(pop_term(parser).get_nonterminal());
  auto $2 = pop_term(parser).get_terminal();
  auto $1 = std::get<int>(pop_term(parser).get_nonterminal());
  auto self = [&](){
  /* BEGIN .MUSK RULE 5 CODE DEFINTION */

        return $1 + $3;
    
  /* END .MUSK RULE 5 CODE DEFINTION */
  };
  PROD_TERM term((NT_VARIANT)self());
  push_term(parser, term);
  return 9;
}

/* REDUCTION RULE FOR RULE 6 */
/* RULE FOR PRODUCTION:  EXPR -> EXPR SUB TERM  */
unsigned long long reduce_rule_6(MoschusParser& parser){
  auto $3 = std::get<int>(pop_term(parser).get_nonterminal());
  auto $2 = pop_term(parser).get_terminal();
  auto $1 = std::get<int>(pop_term(parser).get_nonterminal());
  auto self = [&](){
  /* BEGIN .MUSK RULE 6 CODE DEFINTION */

        return $1 - $3;
    
  /* END .MUSK RULE 6 CODE DEFINTION */
  };
  PROD_TERM term((NT_VARIANT)self());
  push_term(parser, term);
  return 9;
}

/* REDUCTION RULE FOR RULE 7 */
/* RULE FOR PRODUCTION:  EXPR -> TERM  */
unsigned long long reduce_rule_7(MoschusParser& parser){
  auto $1 = std::get<int>(pop_term(parser).get_nonterminal());
  auto self = [&](){
  /* BEGIN .MUSK RULE 7 CODE DEFINTION */

        return $1;
    
  /* END .MUSK RULE 7 CODE DEFINTION */
  };
  PROD_TERM term((NT_VARIANT)self());
  push_term(parser, term);
  return 9;
}

/* REDUCTION RULE FOR RULE 8 */
/* RULE FOR PRODUCTION:  CALC_EXP -> EXPR  */
unsigned long long reduce_rule_8(MoschusParser& parser){
  auto $1 = std::get<int>(pop_term(parser).get_nonterminal());
  auto self = [&](){
  /* BEGIN .MUSK RULE 8 CODE DEFINTION */

        return $1;
    
  /* END .MUSK RULE 8 CODE DEFINTION */
  };
  PROD_TERM term((NT_VARIANT)self());
  push_term(parser, term);
  return 10;
}

namespace {
  unsigned long long reduce_rule_0(MoschusParser&);
  unsigned long long reduce_rule_1(MoschusParser&);
  unsigned long long reduce_rule_2(MoschusParser&);
  unsigned long long reduce_rule_3(MoschusParser&);
  unsigned long long reduce_rule_4(MoschusParser&);
  unsigned long long reduce_rule_5(MoschusParser&);
  unsigned long long reduce_rule_6(MoschusParser&);
  unsigned long long reduce_rule_7(MoschusParser&);
  unsigned long long reduce_rule_8(MoschusParser&);
};

using ReduceFn = unsigned long long(*)(MoschusParser&);
constexpr ReduceFn reduce_map[] = {
  &::reduce_rule_0,
  &::reduce_rule_1,
  &::reduce_rule_2,
  &::reduce_rule_3,
  &::reduce_rule_4,
  &::reduce_rule_5,
  &::reduce_rule_6,
  &::reduce_rule_7,
  &::reduce_rule_8,
};

void error_state_0(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 0 */ 
  throw ParserError(
    "Syntax Error in State 0:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_1(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 1 */ 
  throw ParserError(
    "Syntax Error in State 1:\n"
    "Expected only tokens: $$, ADD, SUB, MUL, DIV"
  );
}

void error_state_2(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 2 */ 
  throw ParserError(
    "Syntax Error in State 2:\n"
    "Expected only tokens: $$, ADD, SUB, MUL, DIV"
  );
}

void error_state_3(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 3 */ 
  throw ParserError(
    "Syntax Error in State 3:\n"
    "Expected only tokens: $$, ADD, SUB, MUL, DIV"
  );
}

void error_state_4(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 4 */ 
  throw ParserError(
    "Syntax Error in State 4:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_5(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 5 */ 
  throw ParserError(
    "Syntax Error in State 5:\n"
    "Expected only tokens: $$, ADD, SUB"
  );
}

void error_state_6(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 6 */ 
  throw ParserError(
    "Syntax Error in State 6:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_7(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 7 */ 
  throw ParserError(
    "Syntax Error in State 7:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_8(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 8 */ 
  throw ParserError(
    "Syntax Error in State 8:\n"
    "Expected only tokens: , ADD, SUB, MUL, DIV, RPAREN"
  );
}

void error_state_9(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 9 */ 
  throw ParserError(
    "Syntax Error in State 9:\n"
    "Expected only tokens: , ADD, SUB, MUL, DIV, RPAREN"
  );
}

void error_state_10(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 10 */ 
  throw ParserError(
    "Syntax Error in State 10:\n"
    "Expected only tokens: , ADD, SUB, MUL, DIV, RPAREN"
  );
}

void error_state_11(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 11 */ 
  throw ParserError(
    "Syntax Error in State 11:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_12(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 12 */ 
  throw ParserError(
    "Syntax Error in State 12:\n"
    "Expected only tokens: , ADD, SUB, RPAREN"
  );
}

void error_state_13(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 13 */ 
  throw ParserError(
    "Syntax Error in State 13:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_14(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 14 */ 
  throw ParserError(
    "Syntax Error in State 14:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_15(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 15 */ 
  throw ParserError(
    "Syntax Error in State 15:\n"
    "Expected only tokens: $$, ADD, SUB, MUL, DIV"
  );
}

void error_state_16(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 16 */ 
  throw ParserError(
    "Syntax Error in State 16:\n"
    "Expected only tokens: $$, ADD, SUB, MUL, DIV"
  );
}

void error_state_17(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 17 */ 
  throw ParserError(
    "Syntax Error in State 17:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_18(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 18 */ 
  throw ParserError(
    "Syntax Error in State 18:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_19(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 19 */ 
  throw ParserError(
    "Syntax Error in State 19:\n"
    "Expected only tokens: , ADD, SUB, RPAREN"
  );
}

void error_state_20(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 20 */ 
  throw ParserError(
    "Syntax Error in State 20:\n"
    "Expected only tokens: $$, ADD, SUB, MUL, DIV"
  );
}

void error_state_21(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 21 */ 
  throw ParserError(
    "Syntax Error in State 21:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_22(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 22 */ 
  throw ParserError(
    "Syntax Error in State 22:\n"
    "Expected only tokens: , INT, LPAREN"
  );
}

void error_state_23(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 23 */ 
  throw ParserError(
    "Syntax Error in State 23:\n"
    "Expected only tokens: $$, ADD, SUB, MUL, DIV"
  );
}

void error_state_24(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 24 */ 
  throw ParserError(
    "Syntax Error in State 24:\n"
    "Expected only tokens: $$, ADD, SUB, MUL, DIV"
  );
}

void error_state_25(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 25 */ 
  throw ParserError(
    "Syntax Error in State 25:\n"
    "Expected only tokens: , ADD, SUB, MUL, DIV, RPAREN"
  );
}

void error_state_26(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 26 */ 
  throw ParserError(
    "Syntax Error in State 26:\n"
    "Expected only tokens: , ADD, SUB, MUL, DIV, RPAREN"
  );
}

void error_state_27(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 27 */ 
  throw ParserError(
    "Syntax Error in State 27:\n"
    "Expected only tokens: , ADD, SUB, MUL, DIV, RPAREN"
  );
}

void error_state_28(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 28 */ 
  throw ParserError(
    "Syntax Error in State 28:\n"
    "Expected only tokens: , ADD, SUB, MUL, DIV, RPAREN"
  );
}

void error_state_29(token::Token){
  /* REDEFINE EXCEPT HANDLER FOR STATE 29 */ 
  throw ParserError(
    "Syntax Error in State 29:\n"
    "Expected only tokens: , ADD, SUB, MUL, DIV, RPAREN"
  );
}

namespace {
  void error_state_0(token::Token);
  void error_state_1(token::Token);
  void error_state_2(token::Token);
  void error_state_3(token::Token);
  void error_state_4(token::Token);
  void error_state_5(token::Token);
  void error_state_6(token::Token);
  void error_state_7(token::Token);
  void error_state_8(token::Token);
  void error_state_9(token::Token);
  void error_state_10(token::Token);
  void error_state_11(token::Token);
  void error_state_12(token::Token);
  void error_state_13(token::Token);
  void error_state_14(token::Token);
  void error_state_15(token::Token);
  void error_state_16(token::Token);
  void error_state_17(token::Token);
  void error_state_18(token::Token);
  void error_state_19(token::Token);
  void error_state_20(token::Token);
  void error_state_21(token::Token);
  void error_state_22(token::Token);
  void error_state_23(token::Token);
  void error_state_24(token::Token);
  void error_state_25(token::Token);
  void error_state_26(token::Token);
  void error_state_27(token::Token);
  void error_state_28(token::Token);
  void error_state_29(token::Token);
};

using ErrorFn = void(*)(token::Token);
constexpr ErrorFn error_map[] = {
  &::error_state_0,
  &::error_state_1,
  &::error_state_2,
  &::error_state_3,
  &::error_state_4,
  &::error_state_5,
  &::error_state_6,
  &::error_state_7,
  &::error_state_8,
  &::error_state_9,
  &::error_state_10,
  &::error_state_11,
  &::error_state_12,
  &::error_state_13,
  &::error_state_14,
  &::error_state_15,
  &::error_state_16,
  &::error_state_17,
  &::error_state_18,
  &::error_state_19,
  &::error_state_20,
  &::error_state_21,
  &::error_state_22,
  &::error_state_23,
  &::error_state_24,
  &::error_state_25,
  &::error_state_26,
  &::error_state_27,
  &::error_state_28,
  &::error_state_29,
};

/* FRIEND HELPERS TO ACCESS PARSER STACK */
PROD_TERM pop_term(MoschusParser& parser){
  PROD_TERM term = parser._term_stack.top();
  parser._term_stack.pop();
  parser._state_stack.pop();
  return term;
}
void push_term(MoschusParser& parser, const PROD_TERM& term){
  parser._term_stack.push(term);
}
void MoschusParser::parse(token::Token token){
  unsigned long long idx = TOKEN_MAPPING.at(token.get_token());
  for (;;) {
    PARSER_STATE current_state = _state_stack.top();
    const ParserTransition& transition = PARSER_TABLE[current_state][idx];
    
    ParserAction action = transition.action;
    unsigned long long identifier = transition.identifier;
    
    switch (action) {
      case (ParserAction::SHIFT) : {
        _term_stack.emplace(token);
        _state_stack.push(identifier);
        return;
      }
      case (ParserAction::REDUCE) : {
        unsigned long long non_terminal_idx = reduce_map[identifier](*this);
        current_state = _state_stack.top();
        
        const ParserTransition& goto_transition = PARSER_TABLE[current_state][non_terminal_idx];
        _state_stack.push(goto_transition.identifier);
        continue;
      }
      default : {
        error_map[current_state](token);
        return;
      }
    }
  }
}
int MoschusParser::parse_final(){
  for (;;) {
    PARSER_STATE current_state = _state_stack.top();
    const ParserTransition& transition = PARSER_TABLE[current_state][0];
    
    ParserAction action = transition.action;
    unsigned long long identifier = transition.identifier;
    
    switch (action) {
      case (ParserAction::REDUCE) : {
        unsigned long long non_terminal_idx = reduce_map[identifier](*this);
        current_state = _state_stack.top();
        
        const ParserTransition& goto_transition = PARSER_TABLE[current_state][non_terminal_idx];
        _state_stack.push(goto_transition.identifier);
        continue;
      }
      default : break;
    }
    break;
  }
  if (_state_stack.size() != 1 || _state_stack.top() != 0){
    // throw here
  }
  if (_term_stack.size() != 1){
    // throw here
  }
  int result = std::get<int>(_term_stack.top().get_nonterminal());
  _term_stack.pop();
  return result;
}
const std::unordered_map<token::TokenKind, unsigned long long> MoschusParser::TOKEN_MAPPING = {
  { token::TokenKind::INT, 1 },
  { token::TokenKind::ADD, 2 },
  { token::TokenKind::SUB, 3 },
  { token::TokenKind::MUL, 4 },
  { token::TokenKind::DIV, 5 },
  { token::TokenKind::LPAREN, 6 },
  { token::TokenKind::RPAREN, 7 },
};
const ParserTransition MoschusParser::PARSER_TABLE[30][12]= {
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 2}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 4}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 3}, {ParserAction::GOTO, 5}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 1} },
  { {ParserAction::REDUCE, 2}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 2}, {ParserAction::REDUCE, 2}, {ParserAction::REDUCE, 2}, {ParserAction::REDUCE, 2}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::REDUCE, 4}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 4}, {ParserAction::REDUCE, 4}, {ParserAction::REDUCE, 4}, {ParserAction::REDUCE, 4}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::REDUCE, 7}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 7}, {ParserAction::REDUCE, 7}, {ParserAction::SHIFT, 7}, {ParserAction::SHIFT, 6}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 9}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 11}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 10}, {ParserAction::GOTO, 12}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 8} },
  { {ParserAction::REDUCE, 8}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 13}, {ParserAction::SHIFT, 14}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 2}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 4}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 15} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 2}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 4}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 16} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 2}, {ParserAction::REDUCE, 2}, {ParserAction::REDUCE, 2}, {ParserAction::REDUCE, 2}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 2}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 4}, {ParserAction::REDUCE, 4}, {ParserAction::REDUCE, 4}, {ParserAction::REDUCE, 4}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 4}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 7}, {ParserAction::REDUCE, 7}, {ParserAction::SHIFT, 18}, {ParserAction::SHIFT, 17}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 7}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 9}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 11}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 10}, {ParserAction::GOTO, 19}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 8} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 21}, {ParserAction::SHIFT, 22}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 20}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 2}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 4}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 23}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 1} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 2}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 4}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 24}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 1} },
  { {ParserAction::REDUCE, 1}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 1}, {ParserAction::REDUCE, 1}, {ParserAction::REDUCE, 1}, {ParserAction::REDUCE, 1}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::REDUCE, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 0}, {ParserAction::REDUCE, 0}, {ParserAction::REDUCE, 0}, {ParserAction::REDUCE, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 9}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 11}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 25} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 9}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 11}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 26} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 21}, {ParserAction::SHIFT, 22}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 27}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::REDUCE, 3}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 3}, {ParserAction::REDUCE, 3}, {ParserAction::REDUCE, 3}, {ParserAction::REDUCE, 3}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 9}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 11}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 28}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 8} },
  { {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 9}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::SHIFT, 11}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 29}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::GOTO, 8} },
  { {ParserAction::REDUCE, 5}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 5}, {ParserAction::REDUCE, 5}, {ParserAction::SHIFT, 7}, {ParserAction::SHIFT, 6}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::REDUCE, 6}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 6}, {ParserAction::REDUCE, 6}, {ParserAction::SHIFT, 7}, {ParserAction::SHIFT, 6}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 1}, {ParserAction::REDUCE, 1}, {ParserAction::REDUCE, 1}, {ParserAction::REDUCE, 1}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 1}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 0}, {ParserAction::REDUCE, 0}, {ParserAction::REDUCE, 0}, {ParserAction::REDUCE, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 3}, {ParserAction::REDUCE, 3}, {ParserAction::REDUCE, 3}, {ParserAction::REDUCE, 3}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 3}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 5}, {ParserAction::REDUCE, 5}, {ParserAction::SHIFT, 18}, {ParserAction::SHIFT, 17}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 5}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
  { {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 6}, {ParserAction::REDUCE, 6}, {ParserAction::SHIFT, 18}, {ParserAction::SHIFT, 17}, {ParserAction::ERROR, 0}, {ParserAction::REDUCE, 6}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0}, {ParserAction::ERROR, 0} },
};
