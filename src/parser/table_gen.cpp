#include "frontend/musk_parser.hpp"
#include "preprocess.hpp"
#include "table_gen.hpp"

#include <queue>

namespace ParseTable {
  TransitionTable _table;

  namespace {
    std::vector<ParseState> _states;
    
    bool contains_state(const std::unordered_set<CanonicalItem>& items){
      for (const auto& state : _states){
        if (state.equivalent(items)) return true;
      }
      return false;
    }

    /**
     * Find a state based on its items,
     * @assert -- state already exists (by contains state)
     **/
    StateIdentifier find_state(const std::unordered_set<CanonicalItem>& items){
      for (const auto& state : _states){
        if (state.equivalent(items)) return state.get_identifier();
      }
      return 0;
    }

    ParseState& create_state(const std::unordered_set<CanonicalItem>& items){
      _states.push_back(ParseState(_states.size()));
      ParseState& new_state = *_states.rbegin();

      for (auto& item : items){
        new_state.add_item(item);
      }
      return new_state;
    }


    /**
     * Build sequential FIRST sets from rest of item rule production
     * @param -- start : start iterator from position+1
     * @param -- end : end iterator for comparison
     * @param -- lookahead : lookahead set if the rest of the sequence is NULLABLE
     * */
    std::set<ProductionItem> sequence_FIRST(auto begin, auto end, const std::set<ProductionItem>& lookahead){
      std::set<ProductionItem> FIRST;
      if (begin == end) return FIRST; // this is a reduce sequence [A -> Bβ•, LA]

      begin++;
      for (; begin != end; begin++){
        const SymbolAlias& symbol_al = *begin;
        if (symbol_al.terminal){
          FIRST.insert(symbol_al.symbol);
          break;
        }
        else {
          ProductionObject& nt_obj = ProductionProcesser::store_.get_object(symbol_al.symbol);
          const std::set<ProductionItem>& nt_FIRST = nt_obj.get_FIRST();
          FIRST.insert(nt_FIRST.begin(), nt_FIRST.end());

          if (!nt_obj.is_NULLABLE()) break;
        }
      }
      if (begin == end){
        FIRST.insert(lookahead.begin(), lookahead.end());
      }
      return FIRST;
    }

    // generate the state closure based on the initial items
    std::unordered_set<CanonicalItem> closure(const std::unordered_set<CanonicalItem>& start_set){
      std::unordered_set<CanonicalItem> CLOSURE = start_set; // copy cons into

      for (const CanonicalItem& c_item : start_set){
        RuleIdentifier rule_id = c_item.get_rule_id();
        Position position = c_item.get_position();

        const std::vector<SymbolAlias>& rule_symbols = ProductionProcesser::rules_.get_prod_symbols(rule_id);

        const SymbolAlias& head_symbol = rule_symbols[position];
        if (head_symbol.terminal) continue; // terminal symbols don't contribute to the closure

        std::set<ProductionItem> seq_FIRST = sequence_FIRST(
          rule_symbols.begin() + position,
          rule_symbols.end(),
          c_item.get_lookaheads()
        );

        ProductionItem nt_alias = head_symbol.symbol;
        for (RuleIdentifier nt_prod_rule : ProductionProcesser::rules_.get_productions(nt_alias)){
          CanonicalItem new_item{rule_id, 0, seq_FIRST};
          if (CLOSURE.contains(new_item)){
            const CanonicalItem& c_item = *CLOSURE.find(new_item);
            c_item.include_lookahead(seq_FIRST);
          }
          else {
            CLOSURE.emplace(rule_id, 0, seq_FIRST);
          }
        }
      }
      return CLOSURE;
    }

    std::set<StateIdentifier> construct_state(StateIdentifier curr_state){
      std::set<StateIdentifier> new_states;

      const std::unordered_set<CanonicalItem>& curr_closure = _states[curr_state].get_items();

      ParseState& state = create_state(curr_closure);
      std::unordered_map<SymbolAlias, std::unordered_set<CanonicalItem>> transition_items;

      for (const CanonicalItem& item : curr_closure){
        RuleIdentifier rule_id = item.get_rule_id();
        const auto& rule = ProductionProcesser::rules_.get_prod_symbols(rule_id);

        Position pos = item.get_position();

        // end of a production -> REDUCE sequence (no next state, only reduce and pop state)
        if (pos == rule.size()){
          StateTransition state_transition = StateTransition(StateAction::REDUCE, 0);
          state.add_reduce(item.get_lookaheads(), state_transition);
          continue;
        }

        // middle of a production -> SHIFT/GOTO sequence
        const SymbolAlias& current_symbol = rule[pos];
        CanonicalItem next_item = item.next_item();
        transition_items[current_symbol].insert(next_item);
      }

      for (auto& transition : transition_items){
        const SymbolAlias& current_symbol = transition.first;
        ProductionItem current_input = current_symbol.symbol;

        const std::unordered_set<CanonicalItem>& items = transition.second;

        StateIdentifier shift_state;

        // get the next states closure and the state identifier
        std::unordered_set<CanonicalItem> next_closure = closure(items);
        if (contains_state(next_closure)){
          shift_state = find_state(next_closure);
        } else {
          ParseState& temp = create_state(next_closure);
          shift_state = temp.get_identifier();
          new_states.insert(shift_state);
        }
        StateTransition state_transition = StateTransition(
          (current_symbol.terminal)? StateAction::SHIFT : StateAction::GOTO,
          shift_state
        );
        state.add_shift_goto(current_input, state_transition);
      }
      return new_states;
    }

    /**
     * Build the start state from start state productions
     * and items
     **/
    ParseState& get_start_state(const musk_ptr& ast){
      const std::string& start = ast->start_nt.label;
      ProductionItem start_alias = ProductionProcesser::alias_.get_alias(start, false);

      std::unordered_set<CanonicalItem> start_items;
      for (RuleIdentifier start_rule : ProductionProcesser::rules_.get_productions(start_alias)){
        start_items.emplace(start_rule, 0, std::set<ProductionItem>{});
      }
      start_items = closure(start_items);
      ParseState& start_state = create_state(start_items);
      return start_state;
    }

    /**
     * Generate all states and transitions
     * i.e: generate the parse table in graph topology
     **/
    void generate_states(const ParseState& start_state){
      std::queue<StateIdentifier> traverse_states;
      traverse_states.push(start_state.get_identifier());
      while (!traverse_states.empty()){
        StateIdentifier current_state = traverse_states.front(); traverse_states.pop();
        std::set<StateIdentifier> new_states = construct_state(current_state);

        for (StateIdentifier next_state : new_states) traverse_states.push(next_state);
      }
    }

    /**
     * Build final complete table, define actions for all terminals and non-terminals
     * with parser errors, also handle s-r, r-r conflict error logging here
     *
     * TODO : build a conflict example constructor with backwards traversal to root
     * store the non-terminals taken to root and reverse replacing with token names
     **/
    void build_table(){
      for (const ParseState& state : _states){
        std::unordered_map<ProductionItem, StateTransition> state_actions;

        std::unordered_set<StateTransition> actions;

        auto populate_table = [&](const auto& alias_symbols){
          for (ProductionItem input : alias_symbols){
            actions = state.get_action(input);
            if (actions.empty()){ // error: no such step to production
              state_actions.emplace(input, StateTransition(StateAction::ERROR, 0));
            }
            else if (actions.size() > 1){ // fatal generator error: r-r or s-r conflict
              state_actions.emplace(input, StateTransition(StateAction::CONFLICT, 0));
              // TODO : log error message here
            }
            else { // just copy over action
              state_actions.emplace(input, *actions.begin());
            }
          }
        };
        populate_table(ProductionProcesser::alias_.get_terminals());
        populate_table(ProductionProcesser::alias_.get_nonterminals());

        _table.emplace(state.get_identifier(), state_actions);
      }
    }

  } // end of namespace (helpers)

  void clear(){
    _table.clear();
    _states.clear();
  }

  void generate_parse_table(const musk_ptr& ast){
    ParseState& start_state = get_start_state(ast);

    generate_states(start_state);

    build_table();
  }
};
