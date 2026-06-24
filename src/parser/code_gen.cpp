#include "code_gen.hpp"
#include "../errors/moschus_string.hpp"

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

    #define HPP_PARAMS const musk_ptr& ast, std::ofstream& HPP_FILE

    void write_includes(HPP_PARAMS){
      std::string STATIC_INCLUDES =
        "#include <variant>\n";
      HPP_FILE << STATIC_INCLUDES;
      HPP_FILE << ast->musk_header.head_includes;
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
        "inline validate_token_interface<"+token_obj+"> token_check\n";

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

      // FIXME : MAYBE ANON NAMESPACE THIS -> KEEP HIDDEN
      std::string PRODUCTION_TERM =
        "struct PROD_TERM {\n"
        "  private:\n"
        "    std::variant<"+token_obj+", NT_VARIANT> value;\n"
        "  public:\n"
        "    unsigned long long TERM_ID;\n"
        "    \n"
        "    NT_VARIANT get_nonterminal(){\n"
        "      return std::get<NT_VARIANT>(value);\n"
        "    }\n"
        "    "+token_obj+" get_terminal(){\n"
        "      return std::get<"+token_obj+">(value);\n"
        "    }\n"
        "}\n";

      HPP_FILE << PRODUCTION_TERM;
    }

    void build_parser_struct(HPP_PARAMS){
      const std::string& ret_type = ast->nt_decls.at(ast->start_nt.label).nt_type;
      const std::string& token_obj = ast->tok_prop.token_object;

      std::string PARSE_STRUCT =
        "struct MoschusParser {\n"
        "  private:\n"
        "    std::stack<PARSER_STATE> _state_stack;\n"
        "    std::stack<PROD_TERM> _term_stack;\n"
        "  public:\n"
        "    "+ret_type+" parse(std::span<"+token_obj+">) throws ParserError;\n"
        "    void parse("+token_obj+") throws ParserError;\n"
        "    "+ret_type+" parse_final(void) throws ParserError;\n"
        "    void reset(void);\n"
        "};\n";

      HPP_FILE << PARSE_STRUCT;
    }






  }


  // generate the header portion
  // define publically exposed APIs
  // - Parser struct which holds an object local definition of the parser and inline of the table
  //   PUBLICALLY DEFINED MEMBER CLASSES:
  //
  //   [start type] MoschusParser::parse(std::span<const Token> token_stream) throws ParserError; -- parse all tokens in a stream
  //
  //   void MoschusParser::parse(Token token) throws ParserError;
  //     - parse a single token (re-entrant on each instance)
  //
  //   [start type] MoschusParser::parse_final() throws ParserError;
  //     - Finish the re-entrant parser, if successfully reduces to a start_nonterm production return the result of it
  //
  //   void MoschusParser::reset() noexcept;
  void generate_hpp(const musk_ptr& ast){

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
  void generate_cpp(const musk_ptr& ast){


  }

  void generate_parser(const std::string& OUTPUT_DIR, const musk_ptr& ast){
    _OUTPUT_DIR = OUTPUT_DIR;

    generate_hpp(ast);
    generate_cpp(ast);
  }

}
