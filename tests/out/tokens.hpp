#pragma once

namespace token {
    enum struct TokenKind {
        INT, ADD, SUB, MUL, DIV
    };

    struct Token {
        int op;
        TokenKind get_token(void);
    };
}