%option header-file="src/parser/frontend/musk_lexer.hpp"
%option noyywrap
%option stack

%top{
    #include "musk_tokens.hpp"
    #define YY_DECL MuskToken yylex()

    extern MuskToken yylex (void);
    extern void reset_loc(void);
}

%{
    #include <stack>

    static Location loc;
    static Location pin_loc;

    // reset for testing
    void reset_loc(){
        loc = {1,1};
        pin_loc = {1,1};
    }

    void update_loc(const std::string& yytext){
        for (int i = 0; i < yytext.length(); i++){
            if (yytext[i] == '\n'){
                loc.col = 1;
                loc.line++;
            }
            else loc.col++;
        }
    }

    Location post_update(const std::string& yytext){
        Location old_loc = loc;
        update_loc(yytext);
        return old_loc;
    }

    std::stack<int> bracket_count;
    std::string strbuf;

    void bracket_nest(){
        bracket_count.push(0);
    }

    int bracket_incr(){
        if (bracket_count.empty()){
            bracket_count.push(0);
        }
        bracket_count.top()++;
        return bracket_count.top();
    }

    int bracket_decr(){
        if (bracket_count.empty()){
            return -1;
        }
        bracket_count.top()--;

        if (bracket_count.top() == 0){
            bracket_count.pop();
            return 0;
        }
        return bracket_count.top();
    }

    void error(const std::string &msg, const std::string& text_snippet, int yylineno) {
        throw std::runtime_error(
            "Lexical error at line " + std::to_string(yylineno) +
            ": " + msg + " (near \"" + text_snippet + "\")"
        );
    }
%}

MuskToken yylex();

%x MUSK_BLOCK_S 
%x CODE_BLOCK_S 
%x DECL_BLOCK_S 
%x TOK_O_BLOCK_S
%x TOK_T_BLOCK_S
%x START_BLOCK_S
%x PROD_BLOCK_S

%%
<MUSK_BLOCK_S>{
    "[" {
        pin_loc = loc;
        update_loc(yytext);
        bracket_incr(); 
    }
    "]" {
        Location l = loc;

        int bc = bracket_decr();
        update_loc(yytext);

        if (bc == 0){
            yy_pop_state();
            auto tok = MuskToken(mtt::MUSK_BLOCK, strbuf, pin_loc, l);
            strbuf.clear();
            return tok;
        }
        else if (bc < 0){
            error("illegal close of missing block declaration", yytext, yylineno);
        }
    }
    (.|\n) {
        strbuf.append(yytext);
        update_loc(yytext);
    }
}

<CODE_BLOCK_S>{
    "{" {
        pin_loc = loc;
        update_loc(yytext);
        bracket_incr(); 
    }
    "}" {
        Location l = loc;

        update_loc(yytext); 
        int bc = bracket_decr();

        if (bc == 0){
            yy_pop_state();
            auto tok = MuskToken(mtt::CODE_BLOCK, strbuf, pin_loc, l);
            strbuf.clear();
            return tok;
        }
        if (bc < 0){
            error("illegal close of missing block declaration", yytext, yylineno);
        }
    }
    (.|\n) { 
        strbuf.append(yytext);
        update_loc(yytext);
    }
}

<TOK_O_BLOCK_S>{
    "[" {
        update_loc(yytext);
        int bc = bracket_incr();
        if (bc > 1) {
            error("illegal nested block declarations in @tok_obj/@tok_type", yytext, yylineno);
        }
    }
    "]" {
        Location l = loc;
        update_loc(yytext);
        int bc = bracket_decr();

        if (bc == 0) {
            yy_pop_state();
            auto tok = MuskToken(mtt::SECTION_TOK_OBJ, strbuf, pin_loc, l);
            strbuf.clear();
            return tok;
        }
        if (bc < 0){
            error("illegal close of missing tok declaration", yytext, yylineno);
        }
    }
    (.|\n) { 
        strbuf.append(yytext);
        update_loc(yytext);
    }
}

<TOK_T_BLOCK_S>{
    "[" {
        update_loc(yytext);
        int bc = bracket_incr();
        if (bc > 1) {
            error("illegal nested block declarations in @tok_obj/@tok_type", yytext, yylineno);
        }
    }
    "]" {
        Location l = loc;
        update_loc(yytext);
        int bc = bracket_decr();

        if (bc == 0) {
            yy_pop_state();
            auto tok = MuskToken(mtt::SECTION_TOK_TYPE, strbuf, pin_loc, l);
            strbuf.clear();
            return tok;
        }
        if (bc < 0){
            error("illegal close of missing tok declaration", yytext, yylineno);
        }
    }
    (.|\n) { 
        strbuf.append(yytext);
        update_loc(yytext);
    }
}

<DECL_BLOCK_S>{
    "[" {
        update_loc(yytext);
        int bc = bracket_incr();
        if (bc > 1) {
            error("illegal nested block declarations in @declarations", yytext, yylineno);
        }
    }
    "]" {
        update_loc(yytext);
        int bc = bracket_decr();

        if (bc == 0) yy_pop_state();
        if (bc < 0){
            error("illegal close of missing block declaration", yytext, yylineno);
        }
    }

    "token"                         { return MuskToken(mtt::TOK_DECL, post_update(yytext), loc); }
    [a-zA-Z0-9_][a-zA-Z0-9_]*       { return MuskToken(mtt::TERM_DECL, yytext, post_update(yytext), loc); }
    [^[:space:],\[\]]+              { return MuskToken(mtt::TYPE_DECL, yytext, post_update(yytext), loc); }
    [ \t\n\r]+                      { update_loc(yytext); }
}

<START_BLOCK_S>{
    "[" {
        update_loc(yytext);
        int bc = bracket_incr();
        if (bc > 1) {
            error("illegal nested block declarations in @start", yytext, yylineno);
        }
    }
    "]" {
        int bc = bracket_decr();

        if (bc == 0){
            yy_pop_state();
            return MuskToken(mtt::SECTION_START, strbuf, post_update(yytext), loc);
        }
        if (bc < 0){
            error("illegal close of missing block declaration", yytext, yylineno);
        }
    }
    (.|\n) { 
        strbuf.append(yytext);
        update_loc(yytext);
    }
}

<PROD_BLOCK_S>{
    "[" {
        update_loc(yytext);
        int bc = bracket_incr();
        if (bc > 1) {
            error("illegal nested block declarations in @productions", yytext, yylineno);
        }
    }
    "]" {
        update_loc(yytext);
        int bc = bracket_decr();

        if (bc == 0) yy_pop_state();
        if (bc < 0){
            error("illegal close of missing block declaration", yytext, yylineno);
        }
    }

    "{" {
        update_loc(yytext);
        bracket_nest();
        bracket_incr();
        yy_push_state(CODE_BLOCK_S);
    }

    "$$"                        { return MuskToken(mtt::PROD_EOF, post_update(yytext), loc); }
    "|"                         { return MuskToken(mtt::PROD_SUM, post_update(yytext), loc); }
    ":"                         { return MuskToken(mtt::PROD_SEP, post_update(yytext), loc); }
    ";"                         { return MuskToken(mtt::PROD_END, post_update(yytext), loc); }
    [a-zA-Z0-9_][a-zA-Z0-9_]*   { return MuskToken(mtt::TERM_DECL, yytext, post_update(yytext), loc); }
    [ \t\n\r]+                  { update_loc(yytext); }
}

"@includes"         { yy_push_state(MUSK_BLOCK_S); return MuskToken(mtt::SECTION_INCLUDES, post_update(yytext), loc); }
"@utilities"        { yy_push_state(MUSK_BLOCK_S); return MuskToken(mtt::SECTION_UTILS, post_update(yytext), loc); }
"@token_obj"        { yy_push_state(TOK_O_BLOCK_S); pin_loc = loc; update_loc(yytext); }
"@token_type"       { yy_push_state(TOK_T_BLOCK_S); pin_loc = loc; update_loc(yytext); }
"@declarations"     { yy_push_state(DECL_BLOCK_S); return MuskToken(mtt::SECTION_DECL, post_update(yytext), loc); }
"@start"            { yy_push_state(START_BLOCK_S); update_loc(yytext); }
"@productions"      { yy_push_state(PROD_BLOCK_S); return MuskToken(mtt::SECTION_PROD, post_update(yytext), loc); }
[ \t\n\r]+          { update_loc(yytext); }

<MUSK_BLOCK_S><<EOF>>   { error("illegal unclosed musk block", yytext, yylineno); }
<CODE_BLOCK_S><<EOF>>   { error("illegal unclosed code block", yytext, yylineno); }
<DECL_BLOCK_S><<EOF>>   { error("illegal unclosed declaration block", yytext, yylineno); }
<START_BLOCK_S><<EOF>>  { error("illegal unclosed start block", yytext, yylineno); }
<PROD_BLOCK_S><<EOF>>   { error("illegal unclosed production block", yytext, yylineno); }
<<EOF>>                 { return MuskToken(mtt::MUSK_EOF, loc, loc); }
%%