#include "parser.hpp"
#include "tokens.hpp"

#include <iostream>

int main(int argc, char** argv) {
  std::cout << "ENTER A SEQUENCE OF NUMBERS AND OPERATIONS FOR ME TO CALCULATE (token by token because im too lazy to lex)!" << std::endl;
  MoschusParser parser{};

  std::string input;
  while (std::cin >> input){
    token::Token current_token(0);
    if (input == "+"){
      current_token = token::Token(token::TokenKind::ADD);
    } else if (input == "-"){
      current_token = token::Token(token::TokenKind::SUB);
    } else if (input == "*") {
      current_token = token::Token(token::TokenKind::MUL);
    } else if (input == "/") {
      current_token = token::Token(token::TokenKind::DIV);
    } else if (input == "(") {
      current_token = token::Token(token::TokenKind::LPAREN);
    } else if (input == ")"){
      current_token = token::Token(token::TokenKind::RPAREN);
    } else {
      current_token = token::Token(std::stoi(input));
    }
    parser.parse(current_token);
  }
  int result = parser.parse_final();
  std::cout << "RESULT: " << result << std::endl;
  return 0;
}
