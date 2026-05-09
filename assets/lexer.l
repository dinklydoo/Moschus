%option noyywrap
%option stack

/*

    LITTLE FUN FACT !!!!
    IF YOU WANT A LEXER THAT COMPILES IN CPP USING FLEX
    ALTHOUGH THERE IS OPTION C++ YOU SHOULD NEVER USE IT HAHAHAHA
    SO WHY THE FUCK DOES IT EXIST

    WORKING WITH FLEX AGAIN MAKES ME REALISE HOW FUCKED THIS SOFTWARE IS
    NOW IM MORE MOTIVATED TO ACTUALLY WRITE A LEXER THAT IS SOMEWHAT FUCKING USABLE
    
*/

%{
    #include "musk_tokens.hpp"
    #include <stack>

    #define YY_DECL MuskToken yylex()
    #define mtt MuskTokenType

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

/* MUSK_BLOCK, anything inside of a non-prod muskblock will be user defined code, just return this as a string in a block token */ 

/* PROD_BLOCK, prod blocks may contain a production which will be followed by user defined code 
    return this as a token indicator of PROD_BLOCK and follow with prod tokens or code blocks */ 

/* CODE_BLOCK, when in a prod block we can enter a code block which is indicated by an occurence of '{' token */ 

%x MUSK_BLOCK_S CODE_BLOCK_S DECL_BLOCK_S START_BLOCK_S PROD_BLOCK_S

%%
<MUSK_BLOCK_S>{
    "[" { bracket_incr(); }
    "]" { 
        int bc = bracket_decr();

        if (bc == 0){
            yy_pop_state();
            auto tok = MuskToken(mtt::MUSK_BLOCK, strbuf);
            strbuf.clear();
            return tok;
        }
    }
    (.|\n) { strbuf.append(yytext); }
}

<CODE_BLOCK_S>{
    "{" { bracket_incr(); }
    "}" { 
        int bc = bracket_decr();

        if (bc == 0){
            yy_pop_state();
            auto tok = MuskToken(mtt::CODE_BLOCK, strbuf);
            strbuf.clear();
            return tok;
        }
        if (bc < 0){
            error("illegal close of missing block declaration", yytext, yylineno);
        }
    }
    (.|\n) { strbuf.append(yytext); }
}

<DECL_BLOCK_S>{
    "[" {
        int bc = bracket_incr();
        if (bc > 1) {
            error("illegal nested block declarations in @declarations", yytext, yylineno);
        }
    }
    "]" {
        int bc = bracket_decr();

        if (bc == 0) yy_pop_state();
        if (bc < 0){
            error("illegal close of missing block declaration", yytext, yylineno);
        }
    }

    "token"                     { return MuskToken(mtt::TOK_DECL); }
    [a-zA-Z0-9_][a-zA-Z0-9_]*   { return MuskToken(mtt::TERM_DECL, yytext); }
    [^[:space:]]+               { return MuskToken(mtt::TYPE_DECL, yytext); }
    [ \t\n\r]+                  { /* skip whitespace */ }
}

<START_BLOCK_S>{
    "[" {
        int bc = bracket_incr();
        if (bc > 1) {
            error("illegal nested block declarations in @start", yytext, yylineno);
        }
    }
    "]" {
        int bc = bracket_decr();

        if (bc == 0) yy_pop_state();
        if (bc < 0){
            error("illegal close of missing block declaration", yytext, yylineno);
        }
    }
    [a-zA-Z0-9_][a-zA-Z0-9_]*   { return MuskToken(mtt::TERM_DECL, yytext); }
    [ \t\n\r]+                  { /* skip whitespace */ }
}

<PROD_BLOCK_S>{
    "[" {
        int bc = bracket_incr();
        if (bc > 1) {
            error("illegal nested block declarations in @productions", yytext, yylineno);
        }
    }
    "]" {
        int bc = bracket_decr();

        if (bc == 0) yy_pop_state();
        if (bc < 0){
            error("illegal close of missing block declaration", yytext, yylineno);
        }
    }

    "{" {
        bracket_nest();
        bracket_incr();
        yy_push_state(CODE_BLOCK_S);
    }

    "$$"                        { return MuskToken(mtt::PROD_EOF); }
    "|"                         { return MuskToken(mtt::PROD_SUM); }
    ":"                         { return MuskToken(mtt::PROD_SEP); }
    ";"                         { return MuskToken(mtt::PROD_END); }
    [a-zA-Z0-9_][a-zA-Z0-9_]*   { return MuskToken(mtt::TERM_DECL, yytext); }
    [ \t\n\r]+                  { /* skip whitespace */ }
}

"@includes"         { yy_push_state(MUSK_BLOCK_S); return MuskToken(mtt::SECTION_INCLUDES); }
"@utilities"        { yy_push_state(MUSK_BLOCK_S); return MuskToken(mtt::SECTION_UTILS); }
"@token_obj"        { yy_push_state(MUSK_BLOCK_S); return MuskToken(mtt::SECTION_TOK_OBJ); }
"@token_type"       { yy_push_state(MUSK_BLOCK_S); return MuskToken(mtt::SECTION_TOK_TYPE); }
"@declarations"     { yy_push_state(DECL_BLOCK_S); return MuskToken(mtt::SECTION_DECL); }
"@start"            { yy_push_state(START_BLOCK_S); return MuskToken(mtt::SECTION_START); }
"@productions"       { yy_push_state(PROD_BLOCK_S); return MuskToken(mtt::SECTION_PROD); }
[ \t\n\r]+          { /* skip whitespace */ }

<MUSK_BLOCK_S><<EOF>>   { error("illegal unclosed musk block", yytext, yylineno); }
<CODE_BLOCK_S><<EOF>>   { error("illegal unclosed code block", yytext, yylineno); }
<DECL_BLOCK_S><<EOF>>   { error("illegal unclosed declaration block", yytext, yylineno); }
<START_BLOCK_S><<EOF>>  { error("illegal unclosed start block", yytext, yylineno); }
<PROD_BLOCK_S><<EOF>>   { error("illegal unclosed production block", yytext, yylineno); }
<<EOF>>                 { return MuskToken(mtt::MUSK_EOF); }
%%