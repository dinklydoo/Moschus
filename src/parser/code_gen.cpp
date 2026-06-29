#include "code_gen.hpp"
#include "../errors/except_handler.hpp"
#include "preprocess.hpp"
#include "table_gen.hpp"

#include <fstream>
#include <unordered_set>
/*
    generate parser code in seperate .cpp and .hpp file
    expose main API's and make sure reduce actions are easily modifiable,
    actions as functions should be labelled by their base production and an ID

    eg.) For productions of an arbitrary nonterminal 'A'
        A : a B b {
            ... codeblock 1 ...
        }
        | a B c {
            ... codeblock 2 ...

        };

    In the generated code we should have:
        reduce_A_1(){
            // defined aliases for subterms
            Token& $1 = ...;
            auto& $2 = ...;
            Token& $3 = ...;

            [... codeblock 1 ...]
        }

        reduce_A_2(){
            // defined aliases for subterms
            Token& $1 = ...;
            auto& $2 = ...;
            Token& $3 = ...;

            [... codeblock 2 ...]
        }

    In addition each state must have a defined error handler
        error_state_1(Token& input) {
            // user defined error with token input that caused it
        }

        error_state_2(Token& input) {
            // user defined error with token input that caused it
        }
    
    These functions should be called automatically when reduction actions
    occur and when error actions occur with users being able to easily modify
    their bodies
*/

namespace CodeGenerator {
  namespace {
    std::string _OUTPUT_DIR;

    // two space tabs
    #define TAB std::string(2, ' ')

    /**====================================================================================================================================
        .HPP FILE GENERATION BEGIN
    ====================================================================================================================================**/

    #define HPP_PARAMS const musk_ptr& ast, std::ofstream& HPP_FILE

    void write_hpp_includes(HPP_PARAMS){
      std::string STATIC_INCLUDES =
        "#include <variant>\n"
        "#include <stack>\n"
        "#include <concepts>\n";
      HPP_FILE << STATIC_INCLUDES;

      HPP_FILE << "\n/* .MUSK @INCLUDES BEGIN */\n";
      HPP_FILE << ast->musk_header.head_includes;
      HPP_FILE << "\n/* .MUSK @INCLUDES END */\n\n";
    }

    /*
     * Generate NT_VARIANT alias:
     * using NT_VARIANT = std::variant<
     *   nt_type1,
     *   nt_type2,
     *   nt_type3
     * >;
     * <- FILE pointer at newline after
     */
    void build_nonterm_variant(HPP_PARAMS){
      std::unordered_set<std::string> non_term_types;

      for (const auto& nt_decl : ast->nt_decls){
        const auto& declaration = nt_decl.second;
        if (declaration.nt_type == "void") continue;
        non_term_types.insert(declaration.nt_type);
      }

      HPP_FILE << "\n/* NON-TERMINAL VARIANT WITH ALL USER DEFINED NON-TERMINAL TYPES */\n";
      // all void productions
      if (non_term_types.empty()){
        // still define but we will never use it (makes struct defn easier to assume NT_VARIANT is at least defined)
        HPP_FILE << "using NT_VARIANT = void;\n";
        return;
      }

      std::string NT_VARIANT = "using NT_VARIANT = std::variant<";
      for (auto it = non_term_types.begin(); it != non_term_types.end(); it++){
        const std::string& type = *it;
        if (it != non_term_types.begin()) NT_VARIANT += ',';
        NT_VARIANT += "\n"+TAB+type;
      }
      NT_VARIANT+="\n>;\n";
      HPP_FILE << NT_VARIANT;
    }

    /*
     * Using concepts to validate that token type actually implements
     * required member functions (interfaces) and throw with non-cryptic concept
     *
     *
     */
    void validate_token_interface(HPP_PARAMS){
      const std::string& token_obj = ast->tok_prop.token_object;
      const std::string& token_type = ast->tok_prop.token_type;

      HPP_FILE << "\n/* TOKEN INTERFACE CHECK FOR METHOD get_token(void) */\n";

      // FIXME : MAYBE ANON NAMESPACE THIS -> KEEP HIDDEN
      std::string TOKEN_CONCEPT =
        "template<typename T>\n"
        "concept Tokenizable = requires(T a){\n"
        "  { a.get_token() } -> std::same_as<"+token_type+">;\n"
        "};\n"
        "template<typename T>\n"
        "struct validate_token_interface {\n"
        "  static_assert(Tokenizable<T>, \""+
        MoschusString(Color::red, "Token object does not implement required interface method get_token(void)").to_string()+"\");\n"
        "};\n"
        "inline validate_token_interface<"+token_obj+"> token_check;\n";

      HPP_FILE << TOKEN_CONCEPT;
    }

    /*
     * Generate the production term wrapper for term stack
     * struct PRODUCTION_TERM {
     *   private:
     *     std::variant<TokenType, NT_VARIANT> value;
     *   public:
     *     unsigned long long TERM_ID;
     *
     *     NT_VARIANT get_nonterminal(){
     *       return std::get<NT_VARIANT>(value);
     *     }
     *     TokenType get_terminal(){
     *       return std::get<TokenType>(value);
     *     }
     * }
     */
    void build_pterm_wrapper(HPP_PARAMS){
      const std::string& token_obj = ast->tok_prop.token_object;

      HPP_FILE << "\n/* PRODUCTION TERM VARIANT WRAPPER */\n";
      std::string PRODUCTION_TERM =
        "struct PROD_TERM {\n"
        "  private:\n"
        "    std::variant<"+token_obj+", NT_VARIANT> value;\n"
        "  public:\n"
        "    PROD_TERM(token::Token token_) : value(token_) {}\n"
        "    PROD_TERM(NT_VARIANT var_) : value(var_) {}\n"
        "    \n"
        "    NT_VARIANT get_nonterminal(){\n"
        "      return std::get<NT_VARIANT>(value);\n"
        "    }\n"
        "    "+token_obj+" get_terminal(){\n"
        "      return std::get<"+token_obj+">(value);\n"
        "    }\n"
        "};\n";

      HPP_FILE << PRODUCTION_TERM;
    }

    /*
    * Build ParserError, simply a wrapper around rte but can be caught explicitly
    */
    void build_parser_error(HPP_PARAMS){
      std::string ERROR_DEFN =
      "struct ParserError : public std::runtime_error {\n"
      "  public:\n"
      "    explicit ParserError(const std::string& msg)\n"
      "      : runtime_error(msg) {}\n"
      "};\n";
      HPP_FILE << "\n/* ParserError DEFINITION WRAPS RUNTIME ERROR */\n";
      HPP_FILE << ERROR_DEFN;
    }

    /*
    * Build all Parser structs,
    * -- ParserAction (enum for actions)
    * -- ParserTransition (container for action + action id (state or rule id) )
    * -- MoschusParser (actual parser struct)
    */
    void build_parser_structs(HPP_PARAMS){
      const std::string& ret_type = ast->nt_decls.at(ast->start_nt.label).nt_type;
      const std::string& token_obj = ast->tok_prop.token_object;
      const std::string& token_type = ast->tok_prop.token_type;

      // define the struct for actions on the parse table
      HPP_FILE << "\n/* STRUCTS FOR PARSER TRANSITION ACTIONS */\n";
      std::string TRANSITION_STRUCT =
        "enum struct ParserAction{\n"
        "  SHIFT,\n"
        "  REDUCE,\n"
        "  GOTO,\n"
        "  ERROR\n"
        "};\n"
        "struct ParserTransition {\n"
        "  ParserAction action;\n"
        "  unsigned long long identifier;\n"
        "};\n";

      HPP_FILE << TRANSITION_STRUCT;

      size_t production_items =
        ProductionProcesser::alias_.get_terminals().size()+
        ProductionProcesser::alias_.get_nonterminals().size();

      // define the actual parser struct
      HPP_FILE << "\n/* MoschusParser OBJECT DEFINITION */\n";
      std::string PARSE_STRUCT =
        "using PARSER_STATE = unsigned long long;\n"
        "struct MoschusParser {\n"
        "  private:\n"
        "    std::stack<PARSER_STATE> _state_stack;\n"
        "    std::stack<PROD_TERM> _term_stack;\n"
        "    \n"
        "    friend PROD_TERM pop_term(MoschusParser&);\n"
        "    friend void push_term(MoschusParser&, const PROD_TERM&);\n"
        "  public:\n"
        "    MoschusParser() {_state_stack.push(0); }\n"
        "    \n"
        "    static const ParserTransition PARSER_TABLE["+std::to_string(ParseTable::_table.size())+"]["+std::to_string(production_items)+"];\n"
        "    static const std::unordered_map<"+token_type+", unsigned long long> TOKEN_MAPPING;\n"
        "    void parse("+token_obj+"); /* throws ParserError */\n"
        "    "+ret_type+" parse_final(void); /* throws ParserError */\n"
        "    void reset(void);\n"
        "};\n";

      HPP_FILE << PARSE_STRUCT;
    }

    /**====================================================================================================================================
        .CPP FILE GENERATION BEGIN
    ====================================================================================================================================**/

    #define CPP_PARAMS const musk_ptr& ast, std::ofstream& CPP_FILE

    void write_cpp_includes(CPP_PARAMS){
      std::string INCLUDES =
        "#include \"parser.hpp\"\n";
      CPP_FILE << INCLUDES;
    }

    void write_utilities(CPP_PARAMS){
      std::string UTILS = 
        "\n/* BEGIN .MUSK @utilities */\n"
        +ast->musk_header.head_utils+
        "\n/* END .MUSK @utilities */\n";

      CPP_FILE << UTILS;
    }

    void build_parse_table(CPP_PARAMS) {
      size_t production_items =
        ProductionProcesser::alias_.get_terminals().size()+
        ProductionProcesser::alias_.get_nonterminals().size();

      std::string STATIC_TABLE =
        "const ParserTransition MoschusParser::PARSER_TABLE"
        "["+std::to_string(ParseTable::_table.size())+"]["+std::to_string(production_items)+"]= {\n";
      for (const auto& table_row : ParseTable::_table){
        STATIC_TABLE += TAB+"{ ";
        for (auto it = table_row.begin(); it != table_row.end(); it++){
          if (it != table_row.begin()) STATIC_TABLE+=", ";
          STATIC_TABLE+="{ParserAction::";

          const StateTransition& action = *it;
          // TODO : maybe add to string method to StateAction
          switch (action.action){
            case (StateAction::SHIFT) :
              STATIC_TABLE+="SHIFT, "; break;
            case (StateAction::REDUCE) :
              STATIC_TABLE+="REDUCE, "; break;
            case (StateAction::GOTO) :
              STATIC_TABLE+="GOTO, "; break;
            default :
              STATIC_TABLE+="ERROR, "; break;
          }
          STATIC_TABLE+=std::to_string(action.action_id);

          STATIC_TABLE+="}";
        }
        STATIC_TABLE += " },\n";
      }
      STATIC_TABLE += "};\n";

      CPP_FILE << STATIC_TABLE;
    }

    void build_token_mapping(CPP_PARAMS){
      const std::string& token_type = ast->tok_prop.token_type;
      std::string STATIC_MAP =
        "const std::unordered_map<"+token_type+", unsigned long long> MoschusParser::TOKEN_MAPPING = {\n";
      for (auto const& tok_decl : ast->tok_decls){
        ProductionItem tok_alias = ProductionProcesser::alias_.get_alias(tok_decl.token_identifier, true);
        STATIC_MAP+=TAB+"{ "+token_type+"::"+tok_decl.token_identifier+", "+std::to_string(tok_alias)+" },\n";
      }
      STATIC_MAP += "};\n";

      CPP_FILE << STATIC_MAP;
    }
    
    void build_friend_helpers(CPP_PARAMS){
      const std::string& token_obj = ast->tok_prop.token_object;
      std::string FRIEND_HELPERS = 
        "\n/* FRIEND HELPERS TO ACCESS PARSER STACK */\n"
        "PROD_TERM pop_term(MoschusParser& parser){\n"
        "  PROD_TERM term = parser._term_stack.top();\n"
        "  parser._term_stack.pop();\n"
        "  parser._state_stack.pop();\n"
        "  return term;\n"
        "}\n"
        "void push_term(MoschusParser& parser, const PROD_TERM& term){\n"
        "  parser._term_stack.push(term);\n"
        "}\n";

      CPP_FILE << FRIEND_HELPERS;
    }

    std::string build_reduction_rule(const musk_ptr& ast, const ProductionRule& rule){
      const std::string& token_obj = ast->tok_prop.token_type;
      const std::string& base_type = ast->nt_decls.at(rule.nt_base.label).nt_type;
      ProductionItem base_alias = ProductionProcesser::alias_.get_alias(rule.nt_base.label, false);
      const auto& prod_symbols = ProductionProcesser::rules_.get_prod_symbols(rule.rule_identifier);

      // doc-string recalls the actual rule by its defined production (unaliased) as rule identifiers
      // can and most likely will redefine the production rule ordering
      std::string RULE_FN = "/* REDUCTION RULE FOR RULE "+std::to_string(rule.rule_identifier)+" */\n";
      RULE_FN+="/* RULE FOR PRODUCTION:  "+rule.nt_base.label+" -> ";
      for (const SymbolAlias& symbol : prod_symbols){
        RULE_FN+=ProductionProcesser::alias_.get_label(symbol.symbol)+' ';
      }
      RULE_FN+=" */\n";

      RULE_FN+="unsigned long long reduce_rule_"+std::to_string(rule.rule_identifier)+"(MoschusParser& parser){\n";

      // write '$X' variables as locals and removing/retrieving from the term stack
      for (ssize_t i = prod_symbols.size()-1; i >= 0; i--){
        const SymbolAlias& symbol = prod_symbols[i];
        if (symbol.terminal){
          RULE_FN+="  auto $"+std::to_string(i+1)+" = pop_term(parser).get_terminal();\n";
        } else {
          const std::string& nt_label = ProductionProcesser::alias_.get_label(symbol.symbol);
          const std::string nt_type = ast->nt_decls.at(nt_label).nt_type;
          if (nt_type == "void") continue;
          RULE_FN+=
            TAB+"auto $"+std::to_string(i+1)+" = "
            "std::get<"+nt_type+">(pop_term(parser).get_nonterminal());\n";
        }
      }

      // if type is non-void abstract the construction of self into a lambda then push onto the term stack
      // the self value is referenced potentially by parent productions.
      if (base_type != "void"){
        RULE_FN+=
          "  auto self = [&](){\n"
          "  /* BEGIN .MUSK RULE "+std::to_string(rule.rule_identifier)+" CODE DEFINTION */\n"
          +rule.prod_action+
          "\n  /* END .MUSK RULE "+std::to_string(rule.rule_identifier)+" CODE DEFINTION */"
          "\n  };\n"
          "  PROD_TERM term((NT_VARIANT)self());\n";
      } 
      // If type is void we don't need any lambda wrapper around self construction as the value
      // of self is never used, only side-effects.
      else {
        RULE_FN+=
          rule.prod_action+
          "  PROD_TERM term("+std::to_string(base_alias)+");\n";
      }
      RULE_FN+=
        "  push_term(parser, term);\n"
        "  return "+std::to_string(base_alias)+";\n";
      RULE_FN+="}\n";

      return RULE_FN;
    }

    void build_reduction_fn(CPP_PARAMS) {
      size_t N_RULES = ast->rules_by_id.size();

      std::string REDUCTION_FNS;
      for (size_t i = 0; i < N_RULES; i++){
        REDUCTION_FNS+="\n"+build_reduction_rule(ast, ast->rules_by_id.at(i));
      }

      CPP_FILE << REDUCTION_FNS;

      std::string NS_DECL = "\nnamespace {\n";

      for (size_t i = 0; i < N_RULES; i++){
        NS_DECL+=TAB+"unsigned long long reduce_rule_"+std::to_string(i)+"(MoschusParser&);\n";
      }
      NS_DECL+="};\n\n";

      CPP_FILE << NS_DECL;

      std::string REDUCTION_MAP = 
        "using ReduceFn = unsigned long long(*)(MoschusParser&);\n"
        "constexpr ReduceFn reduce_map[] = {\n";
      for (size_t i = 0; i < N_RULES; i++){
        REDUCTION_MAP+=TAB+"&::reduce_rule_"+std::to_string(i)+",\n";
      }
      REDUCTION_MAP+="};\n";

      CPP_FILE << REDUCTION_MAP;
    }

    std::string build_error_state(const musk_ptr& ast, size_t STATE){
      const std::string& token_obj = ast->tok_prop.token_object;
      std::string ERR_FN = 
        "void error_state_"+std::to_string(STATE)+"("+token_obj+"){\n"
        "  /* REDEFINE EXCEPT HANDLER FOR STATE "+std::to_string(STATE)+" */ \n"
        "  throw ParserError(\n"
        "    \"Syntax Error in State "+std::to_string(STATE)+":\\n\"\n"
        "    \"Expected only tokens: ";
      
      bool seen_first = false;
      for (size_t i = 0; i < ParseTable::_table[STATE].size(); i++){
        const StateTransition& transition = ParseTable::_table[STATE][i];
        if (transition.action != StateAction::REDUCE && transition.action != StateAction::SHIFT) continue;
        if (seen_first) ERR_FN += ", ";
        else seen_first = true;
        
        ERR_FN += ProductionProcesser::alias_.get_label(i);
      }
      ERR_FN +=  
        "\"\n"
        "  );\n"
        "}\n";

      return ERR_FN;
    }

    void build_error_fn(CPP_PARAMS){
      size_t N_STATES = ParseTable::_table.size();

      std::string ERROR_FNS;
      for (size_t i = 0; i < N_STATES; i++){
        ERROR_FNS+="\n"+build_error_state(ast, i);
      }

      CPP_FILE << ERROR_FNS;

      std::string NS_DECL = "\nnamespace {\n";

      const std::string& token_obj = ast->tok_prop.token_object;
      for (size_t i = 0; i < N_STATES; i++){
        NS_DECL+=TAB+"void error_state_"+std::to_string(i)+"("+token_obj+");\n";
      }
      NS_DECL+="};\n\n";

      CPP_FILE << NS_DECL;

      std::string REDUCTION_MAP = 
        "using ErrorFn = void(*)("+token_obj+");\n"
        "constexpr ErrorFn error_map[] = {\n";
      for (size_t i = 0; i < N_STATES; i++){
        REDUCTION_MAP+=TAB+"&::error_state_"+std::to_string(i)+",\n";
      }
      REDUCTION_MAP+="};\n";

      CPP_FILE << REDUCTION_MAP;
    }

    void build_parse_fn(CPP_PARAMS){
      const std::string& token_obj = ast->tok_prop.token_object;
      std::string PARSE_RE =
        "void MoschusParser::parse("+token_obj+" token){\n"
        "  unsigned long long idx = TOKEN_MAPPING.at(token.get_token());\n"
        "  for (;;) {\n"
        "    PARSER_STATE current_state = _state_stack.top();\n"
        "    const ParserTransition& transition = PARSER_TABLE[current_state][idx];\n"
        "    \n"
        "    ParserAction action = transition.action;\n"
        "    unsigned long long identifier = transition.identifier;\n"
        "    \n"
        "    switch (action) {\n"
        "      case (ParserAction::SHIFT) : {\n"
        "        _term_stack.emplace(token);\n"
        "        _state_stack.push(identifier);\n"
        "        return;\n"
        "      }\n"
        "      case (ParserAction::REDUCE) : {\n"
        "        unsigned long long non_terminal_idx = reduce_map[identifier](*this);\n"
        "        current_state = _state_stack.top();\n"
        "        \n"
        "        const ParserTransition& goto_transition = PARSER_TABLE[current_state][non_terminal_idx];\n"
        "        _state_stack.push(goto_transition.identifier);\n"
        "        continue;\n"
        "      }\n"
        "      default : {\n"
        "        error_map[current_state](token);\n"
        "        return;\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n";
      
      CPP_FILE << PARSE_RE;

      const std::string& start_type = ast->nt_decls.at(ast->start_nt.label).nt_type;
      std::string PARSE_FIN =
        start_type+" MoschusParser::parse_final(){\n"
        "  for (;;) {\n"
        "    PARSER_STATE current_state = _state_stack.top();\n"
        "    const ParserTransition& transition = PARSER_TABLE[current_state][0];\n"
        "    \n"
        "    ParserAction action = transition.action;\n"
        "    unsigned long long identifier = transition.identifier;\n"
        "    \n"
        "    switch (action) {\n"
        "      case (ParserAction::REDUCE) : {\n"
        "        unsigned long long non_terminal_idx = reduce_map[identifier](*this);\n"
        "        current_state = _state_stack.top();\n"
        "        \n"
        "        const ParserTransition& goto_transition = PARSER_TABLE[current_state][non_terminal_idx];\n"
        "        _state_stack.push(goto_transition.identifier);\n"
        "        continue;\n"
        "      }\n"
        "      default : break;\n"
        "    }\n"
        "    break;\n"
        "  }\n"
        "  if (_state_stack.size() != 1 || _state_stack.top() != 0){\n"
        "    // throw here\n"
        "  }\n"
        "  if (_term_stack.size() != 1){\n"
        "    // throw here\n"
        "  }\n"
        +TAB+start_type+" result = std::get<"+start_type+">(_term_stack.top().get_nonterminal());\n"
        "  _term_stack.pop();\n"
        "  return result;\n"
        "}\n";

      CPP_FILE << PARSE_FIN;
    }

  }

  // generate the header portion
  // define publically exposed APIs
  // - Parser struct which holds an object local definition of the parser and inline of the table
  //   PUBLICALLY DEFINED MEMBER CLASSES:
  //
  //   void MoschusParser::parse(Token token) throws ParserError;
  //     - parse a single token (re-entrant on each instance)
  //
  //   [start type] MoschusParser::parse_final() throws ParserError;
  //     - Finish the re-entrant parser, if successfully reduces to a start_nonterm production return the result of it
  //
  //   void MoschusParser::reset() noexcept;
  void generate_hpp(HPP_PARAMS){
    write_hpp_includes(ast, HPP_FILE);
    validate_token_interface(ast, HPP_FILE);
    build_nonterm_variant(ast, HPP_FILE);
    build_pterm_wrapper(ast, HPP_FILE);
    build_parser_error(ast, HPP_FILE);
    build_parser_structs(ast, HPP_FILE);
  }

  // Generate the cpp portion
  //
  // a lot of declarations and wrappers are required
  // to abstract away the non-user friendly portion
  // user friendly portion should be:
  // - Custom error handling bodies
  //   * On default the error handler should output:
  //     "Syntax Error: Unexpected Token/Nonterminal '[term_name]' in state [state no.], expected terms are: [All tokens/ All nonterminal edges]"
  // - Production Bodies if users don't want to write these in the .musk files
  //   OR they want to add additional code outside of the generated code
  void generate_cpp(CPP_PARAMS){
    write_cpp_includes(ast, CPP_FILE);
    write_utilities(ast, CPP_FILE);
    build_reduction_fn(ast, CPP_FILE);
    build_error_fn(ast, CPP_FILE);
    build_friend_helpers(ast, CPP_FILE);
    build_parse_fn(ast, CPP_FILE);
    build_token_mapping(ast, CPP_FILE);
    build_parse_table(ast, CPP_FILE);
  }

  void generate_parser(const std::string& OUTPUT_DIR, const musk_ptr& ast){
    _OUTPUT_DIR = OUTPUT_DIR;

    std::string HPP_PATH = _OUTPUT_DIR+"/parser.hpp";
    std::string CPP_PATH = _OUTPUT_DIR+"/parser.cpp";

    auto fd_err = [&](const std::string& path) {
      MoschusExceptHandler::push_error(
        MoschusError(
          MoschusString(
            Color::red, "File I/O error, could not open file "
          ).to_string()+
          MoschusString(
            Color::yellow, Modifier::italic, 
            std::format("\"{}\"", path).c_str()
          ).to_string()
          ,
          MoschusErrorType::FileIOError
        )
      );
    };

    std::ofstream HPP_FILE(HPP_PATH, std::ios::out);
    if (!HPP_FILE.is_open()){
      fd_err(HPP_PATH);
    }
    std::ofstream CPP_FILE(CPP_PATH, std::ios::out);
    if (!CPP_FILE.is_open()){
      fd_err(CPP_PATH);
    }
    MoschusExceptHandler::log_errors();

    generate_hpp(ast, HPP_FILE);
    generate_cpp(ast, CPP_FILE);
  }
}
