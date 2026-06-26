#include <variant>
#include <stack>
#include <concepts>

/* .MUSK @INCLUDES BEGIN */

    #include "tokens.hpp"

/* .MUSK @INCLUDES END */


/* TOKEN INTERFACE CHECK FOR METHOD get_token(void) */
template<typename T>
concept Tokenizable = requires(T a){
  { a.get_token() } -> std::same_as<token::TokenKind>;
};
template<typename T>
struct validate_token_interface {
  static_assert(Tokenizable<T>, "[31mToken object does not implement required interface method get_token(void)[0m");
};
inline validate_token_interface<token::Token> token_check;

/* NON-TERMINAL VARIANT WITH ALL USER DEFINED NON-TERMINAL TYPES */
using NT_VARIANT = std::variant<
  int
>;

/* PRODUCTION TERM VARIANT WRAPPER */
struct PROD_TERM {
  private:
    std::variant<token::Token, NT_VARIANT> value;
  public:
    PROD_TERM(token::Token token_) : value(token_) {}
    PROD_TERM(NT_VARIANT var_) : value(var_) {}
    
    NT_VARIANT get_nonterminal(){
      return std::get<NT_VARIANT>(value);
    }
    token::Token get_terminal(){
      return std::get<token::Token>(value);
    }
};

/* ParserError DEFINITION WRAPS RUNTIME ERROR */
struct ParserError : public std::runtime_error {
  public:
    explicit ParserError(const std::string& msg)
      : runtime_error(msg) {}
};

/* STRUCTS FOR PARSER TRANSITION ACTIONS */
enum struct ParserAction{
  SHIFT,
  REDUCE,
  GOTO,
  ERROR
};
struct ParserTransition {
  ParserAction action;
  unsigned long long identifier;
};

/* MoschusParser OBJECT DEFINITION */
using PARSER_STATE = unsigned long long;
struct MoschusParser {
  private:
    std::stack<PARSER_STATE> _state_stack;
    std::stack<PROD_TERM> _term_stack;
    
    friend PROD_TERM pop_term(MoschusParser&);
    friend void push_term(MoschusParser&, const PROD_TERM&);
  public:
    MoschusParser() {_state_stack.push(0); }
    
    static const ParserTransition PARSER_TABLE[30][12];
    static const std::unordered_map<token::TokenKind, unsigned long long> TOKEN_MAPPING;
    void parse(token::Token); /* throws ParserError */
    int parse_final(void); /* throws ParserError */
    void reset(void);
};
