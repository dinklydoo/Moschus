#pragma once
#include "tokens.hpp"

struct ast {

};

struct arith_expr {
    arith_expr(int op, arith_expr* l, arith_expr* r);
    arith_expr(int op, token::Token l, arith_expr* r);
};

struct int_expr : public arith_expr {
    int_expr(token::Token tok);
};


struct calc_expr : public arith_expr {
    calc_expr(arith_expr* a);
};
