# Moschus Usage
Moschous follows very similar syntax to other parser generators like yacc and ANTLR however some of the syntax is unique.

Unlike YACC and ANTLR however Moschus supports LR(1) grammars as opposed to LALR(1) in Yacc and LR(k-ish?) in ANTLR.

To define a Moschous Parser File you can either create one from scratch with any name suffixed with `.musk`:
~~~
<parser_name>.musk
~~~
Or create a templated `.musk` file with the command:
~~~
> muschous -t <filename>
OR
> muschous --template <filename>
~~~
This will give the following template file `<filename>.musk`:
~~~
@includes<
    // write any include headers here
>

@utilities<
    // any user defined global utility code can go here
>

@token_obj< 
    // token obj type 
>
@token_type<
    // token id enum
>

@declarations<
    // add terminal and non-terminal symbol declarations
>

@start<
    // forward declare start non-terminal symbol
>

@productions<
    // non-terminal expansions/production rules
>
~~~

A small example of a small arithmetic `.musk` file looks like this
~~~
@includes<
    include "tokens.hpp"
    include "ast.hpp"
>

@utilities<
    ast* tree;
    TypeSystem ts = TypeSystem::instance();
>

@token_obj<token::Token>
@token_type<token::TokenKind>

@declarations<
    token INT
    token ADD
    token SUB
    token MUL
    token DIV

    ast*        CALC_EXP
    arith_expr* ADD_EXP
    arith_expr* SUB_EXP
    arith_expr* MUL_EXP
    arith_expr* DIV_EXP

>

@start<CALC_EXP>
@productions<
    CALC_EXP : ADD_EXP $$ {
        return new CalcExp($1);
    };

    ADD_EXP 
    : SUB_EXP ADD SUB_EXP {
        return new ArithExp($2.op, $1, $3);
    }
    | SUB_EXP {
        return $1;
    };
    ;

    etc...
>
~~~
Tokens are unique in that user defined tokens are required, as of now there is no compatible lexer 
(however there are plans to implement one). 
The user (YOU) must define a token object/struct which can be provided to the parser with
`@token_obj<token struct>`
Note: the full namespace path should be provided UNLESS if a using macro is defined in user code.

This token struct should then implement the interface method `token_type get_token(void)` where 
`token_type` is a user defined enum class that represents the actual identifiers of tokens 
which is provided using `@token_type<token enum>`.

Note that the token ID's in this enum should match the token declarations provided in `@declarations`, 
it is okay to not declare some members of the enum class if they are not used, 
as the declared tokens must be only a subset of this enum class.

When accessing a token in the codeblocks of productions the reference `$position` WILL give you the token object NOT the token type. The token identifier is only used for generating the state tables and identifying terminals. So reading values and other
metadata defined in the token object is completely fine in inlined codeblocks.

ex.
~~~
// a simple token for a simple calculator

namespace tok {
    struct Token {
    private:
        TokenKind kind;
        std::variant<int, float> value;

    public:
        // note : the enum class is not required to be nested
        // as long as it is visible to the parser and the correct scope/namespace is provided
        enum class TokenKind {
            INT, FLOAT, ADD, SUB
        };

        TokenKind get_token(void) {
            return kind;
        }
    };
}

// we would then provide the token types in .musk as

@token_obj<tok::Token>
@token_type<tok::Token::TokenKind>

~~~

TODO : need a way to identify token types, either provide an identifier function on the type that will return an ENUM with the token names

TODO : in the future add a native LEXER "fuscous"