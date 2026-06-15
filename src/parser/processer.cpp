#include "preprocess.hpp"
#include "../errors/moschus_warning.hpp"
#include "../errors/moschus_error.hpp"

#include <unordered_set>

// TODO : MAYBE RENAME THIS TO FIRST_FOLLOW.CPP BCUZ TBH THATS ALL THIS IS DOING

namespace ProductionProcesser {

  namespace {

    void populate_terminals(const musk_ptr& ast){
      for (auto& term : ast->tok_decls){
        alias_.new_alias(term.token_identifier, true);
      }
    }

    void populate_nonterminals(const musk_ptr& ast){
      for (auto& non_term : ast->nt_decls){
        alias_.new_alias(non_term.nt_identifier, false);

        // create the alias item
        ProductionItem item = alias_.get_alias(non_term.nt_identifier, false);
        store_.new_object(item);
      }
    }

    // Validate if a non-terminal has been defined and defined non-terminal
    void validate_nonterm_exists(const std::string& non_term){
      static std::unordered_set<std::string> checked_;
      if (checked_.contains(non_term)) return; // this symbol has already been checked

      bool ok = false;
      alias_.try_alias(non_term, false, ok);
      if (!ok){
        alias_.try_alias(non_term, true, ok);
        if (ok){
           // TODO : error for t definition instead of nt defn
        } else {
          // TODO : error for undefined in both nt and t
        }
      }
      checked_.insert(non_term);
    }

    // check if a symbol is defined
    // is_nt: set non-terminal flag true if label is nonterminal otherwise set as false
    void validate_symbol_exists(const std::string& label, bool& is_nt){
      static std::unordered_map<std::string, bool> checked_;
      if (checked_.contains(label)){
        is_nt = checked_.at(label); // set nt flag based on prior check
        return;
      }

      bool ok = false;
      ProductionItem _root_alias = alias_.try_alias(label, false, ok);
      if (!ok){
        alias_.try_alias(label, true, ok);
        if (!ok){
           // TODO : error for undefined
        }
      }
      checked_.emplace(label, is_nt);
    }

    // validate a singular production rule then return a vector containing all non-terminals
    // referenced inside of the production
    std::vector<std::string> validate_production_rule(ProductionRule& rule){
      const std::string& non_term = rule.nt_base;
      validate_nonterm_exists(non_term);

      std::vector<std::string> referenced_nt;
      for (const std::string& consequent : rule.nt_prods){
        bool is_nt;

        validate_symbol_exists(consequent, is_nt);
        if (is_nt){
          referenced_nt.push_back(consequent);
        }
      }
      return referenced_nt;
    }

    /*
    ** Validate all production rules, sanitize the tree before we run the preprocesser
    ** Sanitization includes
    ** - Ensuring root is defined and a non-terminal
    ** - Ensuring all non-terminals are reachable and their productions
    ** - Ensure all tokens in production rules are defined
     */
    void validate_productions(const musk_ptr& ast){
      const std::string& root = ast->start_nt;

      // non-fatal if the non-terminal is not defined
      validate_nonterm_exists(root);

      std::unordered_set<std::string> validated; // non-terms already validated

      // BFS to find and verify all reachable productions from start token
      std::queue<std::string> traverse;
      traverse.push(root);
      validated.insert(root);
      while (!traverse.empty()){
        const std::string base = traverse.front();
        traverse.pop();

        std::vector<ProductionRule>& productions = ast->prod_rules.at(base);
        for (ProductionRule& prod : productions) {
          std::vector<std::string> referenced_nt = validate_production_rule(prod);
          for (const std::string& non_term : referenced_nt){
            if (validated.contains(non_term)) continue;

            traverse.push(non_term);
            validated.insert(non_term);
          }
        }
      }

      // mark and warn on unreachable/unused non-terminals
      for (NonTerminalDeclaration& nt_decl : ast->nt_decls){
        const std::string& nt_label = nt_decl.nt_identifier;
        if (validated.contains(nt_label)) continue;

        // TODO : warning messages here
      }
      // TODO : throw here before the preprocesser runs
    }

    ProductionObject& get_nonterminal_object(const std::string& label){
      ProductionItem _item = alias_.get_alias(label, false);
      return store_.get_object(_item);
    }

    /**
     * Construct the FIRST sets for all non-terminals
     * @assert -- Productions and their rules have already been sanitized, get_object is safe
     *
     * Classic LFP construction for FIRST sets with NULLABILITY
     **/
    void construct_FIRST(const musk_ptr& ast){

      bool fixed_point = false;
      while (!fixed_point){
        fixed_point = true;

        for (const auto& p : ast->prod_rules){
          const std::string& label = p.first;
          const std::vector<ProductionRule>& rules = p.second;

          // production validation step prior ensures that all productions are "well defined"
          // i.e no non-terminal derives a production
          // also that all aliases have been registered correctly for productions
          ProductionObject& nt_obj = get_nonterminal_object(label);

          std::set<ProductionItem> old_FIRST = nt_obj.get_FIRST();
          bool old_NULLABLE = nt_obj.is_NULLABLE();
          for (const ProductionRule& rule : rules){
            // empty transition
            if (rule.nt_prods.empty()) {
              nt_obj.set_NULLABLE();
              continue;
            }

            bool is_nt;
            ProductionItem first_alias = alias_.try_alias(rule.nt_prods[0], false, is_nt);
            if (is_nt) { // non-terminal first
              const ProductionObject& first_obj = store_.get_object(first_alias);

              // first non-term is nullable look-ahead
              if (first_obj.is_NULLABLE()){
                int next_i = 1;
                for (; next_i < rule.nt_prods.size(); next_i++){
                  ProductionItem next_alias = alias_.try_alias(rule.nt_prods[next_i], false, is_nt);

                  // non-terminal follows a NULLABLE non-terminal
                  if (is_nt){
                    ProductionObject& next_obj = store_.get_object(next_alias);

                    // the next non-terminal is non-NULLABLE, union the FIRST sets and end lookahead
                    if (!next_obj.is_NULLABLE()){
                      nt_obj.FIRST_union(next_obj);
                      break;
                    }
                  }
                  // terminal follows a NULLABLE non-terminal, is the FIRST token of this prod rule
                  else {
                    ProductionItem next_alias = alias_.get_alias(rule.nt_prods[next_i], true);
                    nt_obj.FIRST_insert(next_alias);
                    break;
                  }
                }
                // nullable non-terms until end of rule -> this is nullable
                if (next_i == rule.nt_prods.size()){
                  nt_obj.set_NULLABLE();
                }
              }
              // first non-term is fixed join fixed set
              else {
                nt_obj.FIRST_union(first_obj);
              }
            }
            // terminal first directly in the FIRST set
            else {
              first_alias = alias_.get_alias(rule.nt_prods[0], true);
              nt_obj.FIRST_insert(first_alias);
            }

            // Check for changes to the FIRST sets and NULLABLE to set fixed point
            if (nt_obj.get_FIRST() != old_FIRST || nt_obj.is_NULLABLE() != old_NULLABLE){
              fixed_point = false;
            }
          }
        }
      }
    }

    // Get the ProductionObject& for a non-terminal in the production consequent
    // if the index is invalid OR term is a terminal then returns nullptr
    ProductionObject* get_object_at(const std::vector<std::string> consequent, int index){
      if (index < 0 || index >= consequent.size()) return nullptr;

      const std::string& label = consequent[index];
      bool is_nonterminal;
      ProductionItem item = alias_.try_alias(label, false, is_nonterminal);
      if (!is_nonterminal) return nullptr;

      return store_.try_object(item);
    }


    /**
     * Construct FOLLOW sets for all non-terminals
     * @assert -- productions have been sanitized prior, get_object is safe
     *
     * classic LFP construction of FOLLOW sets
     **/
    void construct_FOLLOW(const musk_ptr& ast){

      bool fixed_point = false;
      while (!fixed_point){
        fixed_point = true;

        for (auto& p : ast->prod_rules){
          const std::string& label = p.first;
          const std::vector<ProductionRule>& rules = p.second;

          ProductionObject& root_obj = get_nonterminal_object(label);

          for (const ProductionRule& rule : rules){
            if (rule.nt_prods.empty()) continue;

            for (int i = 0; i < rule.nt_prods.size(); i++){
              const std::string& term = rule.nt_prods[i];

              bool is_nonterm;
              ProductionItem term_alias = alias_.try_alias(term, false, is_nonterm);
              if (!is_nonterm) continue;
              ProductionObject& candidate = store_.get_object(term_alias);

              std::set<ProductionItem> old_FOLLOW = candidate.get_FOLLOW();

              // if next is terminal : A -> ...Bb... then directly add b to FOLLOW(B)
              // if next is non-terminal : A -> ... BCD ... then union with FIRST(B) check nullity
              // if nullable next then shift pointer again and repeat until next is non-null, terminal or end
              int next_i = i+1;
              while (next_i <= rule.nt_prods.size()){
                // TODO : we can maybe optimize by caching a few obtained non-terminal objects -> no refetching
                if (next_i == rule.nt_prods.size()){
                  candidate.FOLLOW_union_follow(root_obj);
                  break;
                }
                ProductionObject* next_obj = get_object_at(rule.nt_prods, next_i);
                // Not at the end of the consequent -> next term is a terminal symbol
                if (next_obj == nullptr){
                  ProductionItem terminal = alias_.get_alias(rule.nt_prods[next_i], true);
                  candidate.FOLLOW_insert(terminal);

                  break; // terminal ends the sequence
                } else {
                  candidate.FOLLOW_union_first(*next_obj);

                  // if the non-terminal is nullable we can still progress
                  if (!next_obj->is_NULLABLE()) break;
                }
                next_i++;
              }
              if (old_FOLLOW != candidate.get_FOLLOW()) fixed_point = false;
            }
          }
        }
      }
    }

  }

  void process_musk_ast(const musk_ptr& ast){
    populate_terminals(ast);
    populate_nonterminals(ast);

    validate_productions(ast);

    construct_FIRST(ast);
    construct_FOLLOW(ast);
  }
}
