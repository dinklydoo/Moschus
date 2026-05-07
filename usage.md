# Moschus Usage
Moschous follows very similar syntax to other parser generators like yacc and ANTLR however some of the syntax is unique.

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
    include "tokens.hpp"
    include "ast.hpp"
>

@utilities<
    ast* tree;
    TypeSystem ts = TypeSystem::instance();
>

@token<token::Token>
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

TODO : in the future add a native LEXER "fuscous"