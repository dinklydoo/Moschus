#include "musk_parser.hpp"
#include "musk_error.hpp"
#include "musk_tokens.hpp"

#include <unordered_set>

/*
    Recursive Descent Parser to parse a .musk file
    into an AST

    As the structure is mostly linear with the only
    caveat being Productions which can contain
    an internal linear set of production rules,

    we must also manually extract production sum types
    as an individual production

*/
static musk_errors errors;
static musk_warnings warnings;

#define tok_eof(it, end, ret)\
    if (it == end){\
        eof_error(errors);\
        return ret;\
    }\

#define tok_it_incr(it, end, ret)\
    it++;\
    tok_eof(it, end, ret)

// stabilise token to a predictable state for error recovery
void stabilise(tok_it& it, tok_it end){
    const static std::unordered_set<mtt> stable_tok {
        mtt::SECTION_START,
        mtt::SECTION_DECL,
        mtt::SECTION_INCLUDES,
        mtt::SECTION_PROD,
        mtt::SECTION_TOK_OBJ,
        mtt::SECTION_TOK_TYPE,
        mtt::SECTION_UTILS
    };

    while (it != end && !stable_tok.contains(it->type)){
        it++;
    }
}

// define our error macro to automatically stabilise after detecting an error
#define push_error(it, end, msg)\
    errors.push_back(\
        MuskError(msg, it->start_loc, it->end_loc)\
    );\
    stabilise(it, end)

#define push_warning(it, end, msg)\
    warnings.push_back(\
        MuskWarning(msg, it->start_loc, it->end_loc)\
    )

/*
    Main API to the musk parser,
    recursive descent with some "decent" error handling
    and recovery
*/
musk_ptr parse_musk(const MuskTokenStream& toks){
    errors.clear();
    warnings.clear();

    // with const vector we can safely pass the iterator as we consume tokens
    tok_it it = toks.begin();

    musk_ptr musk_ast = std::make_unique<MuskAST>();

    musk_ast->musk_header.head_includes = parse_includes(it, toks.end());
    musk_ast->musk_header.head_utils = parse_utilities(it, toks.end());

    if (!musk_ast->musk_header.header_valid()){
        // throw -> no way for token to be defined
        push_error(it, toks.end(), 
            "No valid definition of token type, token must be provided "
            "via an include path in @includes or defined as a structure in @utilities"
        );
    }

    musk_ast->tok_prop = parse_token_p(it, toks.end());

    const auto& declarations = parse_decl(it, toks.end());
    musk_ast->tok_decls = declarations.first;
    musk_ast->nt_decls = declarations.second;
    
    musk_ast->start_nt = parse_start(it, toks.end());
    musk_ast->prod_rules = parse_prod(it, toks.end());

    log_warnings(warnings);
    log_errors(errors);

    return musk_ast;
}

std::string parse_includes(tok_it& it, tok_it end){
    tok_eof(it, end, "");
    
    // include can be optional if token is defined in utils
    if (it->type != mtt::SECTION_INCLUDES){
        return "";
    }
    tok_it_incr(it, end, "");
    if (it->type == mtt::MUSK_BLOCK){
        const std::string& inc = it->internal;
        tok_it_incr(it, end, "");
        return inc;
    }
    return ""; // empty definition @includes[ ]
}

std::string parse_utilities(tok_it& it, tok_it end){
    tok_eof(it, end, "");

    // utils is optional
    if (it->type != mtt::SECTION_UTILS){
        return "";
    }
    tok_it_incr(it, end, "");
    if (it->type == mtt::MUSK_BLOCK){
        const std::string& inc = it->internal;
        tok_it_incr(it, end, "");
        return inc;
    }
    return ""; // empty definition @utilities[ ]
}

TokenProperties parse_token_p(tok_it& it, tok_it end){
    tok_eof(it, end, TokenProperties());

    TokenProperties token_p;
    for (int i = 0; i < 2; i++){
        if (it->type == mtt::SECTION_TOK_OBJ){
            if (!token_p.token_object.empty()){
                push_error(it, end, "Token object type is defined twice");
                return token_p;
            }
            token_p.token_object = it->internal;
            if (token_p.token_object.empty()){
                push_error(it, end, "Token object type is not defined, ensure @token_obj is defined");
            }
            tok_it_incr(it, end, token_p);
        }
        else if (it->type == mtt::SECTION_TOK_TYPE){
            if (!token_p.token_type.empty()){
                push_error(it, end, "Token identifier type is defined twice");
                return token_p;
            }
            token_p.token_type = it->internal;
            if (token_p.token_type.empty()){
                push_error(it, end, "Token identifier type is not defined, ensure @token_type is defined");
            }
            tok_it_incr(it, end, token_p);
        }
        else break;
    }
    return token_p;
}

TokenDeclaration parse_tok_decl(tok_it& it, tok_it end){
    tok_it_incr(it, end, TokenDeclaration());
    if (it->type == mtt::TERM_DECL){
        const std::string& tok = it->internal;
        tok_it_incr(it, end, TokenDeclaration());
        return TokenDeclaration(tok);
    }
    push_error(it, end, "Invalid token declaration, tokens should follow enumerable naming");
    return TokenDeclaration();
}

NonTerminalDeclaration parse_nt_decl(tok_it& it, tok_it end){
    const std::string& nt_type = it->internal;
    tok_it_incr(it, end, NonTerminalDeclaration());
    if (it->type == mtt::TERM_DECL){
        const std::string& nt_label = it->internal;
        tok_it_incr(it, end, NonTerminalDeclaration());
        return NonTerminalDeclaration(nt_type, nt_label);
    }
    push_error(it, end, "Invalid non-terminal declaration, non-terminal namings must follow regex [a-zA-Z0-9_]+");
    return NonTerminalDeclaration();
}

decl_pair parse_decl(tok_it& it, tok_it end){
    std::vector<TokenDeclaration> tok_decl;
    std::vector<NonTerminalDeclaration> nt_decl;

    tok_eof(it, end, std::make_pair(tok_decl, nt_decl));

    if (it->type == mtt::SECTION_DECL){ tok_it_incr(it, end, {}) }
    else {
        push_error(it, end, "Missing or misaligned declaration block @declarations");
        return std::make_pair(tok_decl, nt_decl);
    }

    bool in_declarations = true;
    while (in_declarations) {

        switch (it->type) {
            case mtt::TOK_DECL: 
                parse_tok_decl(it, end);
                break;
            case mtt::TERM_DECL: // term is a stricter binding of type, a type can be parsed as a term
            case mtt::TYPE_DECL:
                parse_nt_decl(it, end);
                break;
            case mtt::SECTION_START:
                in_declarations = false;
                break;
            default:
                push_error(it, end, "Unexpected token found in @declarations");
                return std::make_pair(tok_decl, nt_decl);
        }
    }

    // validate with warnings
    std::unordered_set<std::string> defined;
    for (const auto& tok : tok_decl){
        const std::string& id = tok.token_identifier;
        if (defined.count(id)){
            push_warning(it, end, "Duplicate definitions of token/non-terminal symbol "+id);
        }
        defined.insert(id);
    }
    for (const auto& nt : nt_decl){
        const std::string& id = nt.nt_identifier;
        if (defined.count(id)){
            push_warning(it, end, "Duplicate definitions of token/non-terminal symbol "+id);
        }
        defined.insert(id);
    }

    return {tok_decl, nt_decl};
}

std::string parse_start(tok_it& it, tok_it end){
    tok_eof(it, end, "");
    
    if (it->type != mtt::SECTION_START){
        //throw
        push_error(it, end, "Missing or misaligned start symbol block @start");
        return "";
    }

    if (it->internal.empty()){
        push_error(it, end, "Start symbol is empty/undefined in @start");
        return "";
    }

    const std::string& nt = it->internal;
    tok_it_incr(it, end, "");
    return nt;
}

std::vector<ProductionRule> parse_rule(tok_it& it, tok_it end){
    std::vector<ProductionRule> p_rules;

    tok_eof(it, end, p_rules);
    
    if (it->type != mtt::TERM_DECL){
        push_error(it, end, "Grammar production must have a single non-terminal on left-hand size");
        return p_rules;
    }
    const std::string& nt_base = it->internal;
    tok_it_incr(it, end, p_rules);
    if (it->type != mtt::PROD_SEP){
        push_error(it, end, "Grammar production missing token seperator");
        return p_rules;
    }
    tok_it_incr(it, end, p_rules);
    

    ProductionRule p_rule(nt_base);

    bool in_sequence = true;
    bool eof_consumed = false;
    while (in_sequence) {
        switch (it->type) {
            case mtt::TERM_DECL : {
                if (eof_consumed){
                    push_warning(it, end, "Productions past an EOF token $$, production is never satisfied");
                }
                p_rule.nt_prods.push_back(it->internal);
                tok_it_incr(it, end, p_rules);
                break;
            }
            case mtt::PROD_EOF : {
                p_rule.nt_prods.push_back("__[EOF]__");
                tok_it_incr(it, end, p_rules);
                eof_consumed = true;
                break;
            }
            case mtt::CODE_BLOCK : {
                p_rule.prod_action = it->internal;
                tok_it_incr(it, end, p_rules);
                in_sequence = false;
                break;
            }
            default : {
                push_error(it, end, "Unexpected token in production sequence");
                break;
            }
        }

        if (!in_sequence && it->type == mtt::PROD_SEP){
            tok_it_incr(it, end, p_rules);

            p_rules.push_back(p_rule);
            p_rule.nt_prods.clear();
            p_rule.prod_action.clear();

            in_sequence = true;
        }
    }
    return p_rules;
}

std::vector<ProductionRule> parse_prod(tok_it& it, tok_it end){
    std::vector<ProductionRule> p_rules;
    if (it == end){
        eof_error(errors);
        return p_rules;
    }
    if (it->type != mtt::SECTION_PROD){
        push_error(it, end, "Missing or misaligned production block @productions");
        return p_rules;
    }
    tok_it_incr(it, end, p_rules);
    
    std::vector<ProductionRule> p_subrules;
    while (it->type != mtt::MUSK_EOF){
        p_rules.insert(p_rules.end(), p_subrules.begin(), p_subrules.end());
        tok_it_incr(it, end, p_rules);
    }
    return p_rules;
}
