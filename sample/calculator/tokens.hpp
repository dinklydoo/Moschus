#pragma once

namespace token {
  enum struct TokenKind {
    INT,
    ADD,
    SUB,
    MUL,
    DIV,
    LPAREN,
    RPAREN
  };

  struct Token {
    private:
      TokenKind kind;
      int value;
    public:
      Token(TokenKind kind_) : kind(kind_) {}
      Token(int value_) : kind(TokenKind::INT), value(value_) {}

      TokenKind get_token(void){
        return kind;
      }

      int get_value(void){
        return value;
      }
  };
}