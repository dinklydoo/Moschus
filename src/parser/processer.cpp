#include "preprocess.hpp"
#include "../errors/moschus_error.hpp"

namespace ProductionProcesser {

  namespace {
    void populate_terminals(const musk_ptr& ast){
      for (auto& _term : ast->tok_decls){
        _alias.new_alias(_term.token_identifier, true);
      }
    }

    void populate_nonterminals(const musk_ptr& ast){
      for (auto& _nonterm : ast->nt_decls){
        _alias.new_alias(_nonterm.nt_identifier, false);
      }
    }

    // Initialize the FIRST sets of the NT productions with the first tokens IF they are non-terminal
    // this step DOES NOT find the lfp of the FIRST sets
    void init_FIRST(const musk_ptr& ast){
      for (auto& prod_rules : ast->prod_rules){
        const std::string& _root = prod_rules.first;
        ProductionItem root_item = _alias.get_alias(_root, false);
        ProductionObject* prod_obj = _store.try_object(root_item);
        if (prod_obj == nullptr){
          _store.new_object(root_item);
          prod_obj = _store.try_object(root_item);
        }

        for (auto& _prod : prod_rules.second){
          // can assert that nt_prods is non-empty as the parser prevents nullity
          const std::string& first_item = _prod.nt_prods[0];
  
          bool ok;
          ProductionItem first_alias = _alias.try_alias(first_item, true, ok); // only check for terminal prod
          if (ok) { // is actually a non-terminal
            prod_obj->FIRST_insert(first_alias);
          }
        }
      }
    }

    void generate_FIRST(const musk_ptr& ast){
      bool ok;
      ProductionItem start_alias = _alias.try_alias(ast->start_nt, false, ok);
      if (!ok){
        throw MoschusError("Start Non-Terminal is Terminal: "+ast->start_nt, MoschusErrorType::MuskStartError);
      }

      bool lfp = false;

      while (!lfp){
        ProductionObject& prod_obj = _store.get_object(start_alias);

      }

    }


  }

  void process_musk_ast(const musk_ptr& ast){
    populate_terminals(ast);
    populate_nonterminals(ast);

    init_FIRST(ast);
    generate_FIRST(ast);

  }




}
