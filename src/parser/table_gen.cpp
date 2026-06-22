#include "frontend/musk_parser.hpp"
#include "preprocess.hpp"
#include "table_gen.hpp"
#include "../errors/except_handler.hpp"

#include <format>
#include <queue>
#include <unordered_set>

namespace ParseTable {
  TransitionTable _table;

  namespace {
    std::unordered_map<StateIdentifier, ParseState> _states;
    
    bool contains_state(const std::unordered_set<CanonicalItem>& items){
      for (const auto& state : _states){
        if (state.second.equivalent(items)) return true;
      }
      return false;
    }

    /**
     * Find a state based on its items,
     * @assert -- state already exists (by contains state)
     **/
    StateIdentifier find_state(const std::unordered_set<CanonicalItem>& items){
      for (const auto& state : _states){
        if (state.second.equivalent(items)) return state.second.get_identifier();
      }
      return 0;
    }

    ParseState& create_state(const std::unordered_set<CanonicalItem>& items){
      StateIdentifier state_id = _states.size();
      _states.emplace(state_id, ParseState(state_id));
      ParseState& new_state = _states.at(state_id);

      for (auto& item : items){
        new_state.add_item(item);
      }
      return new_state;
    }


    /**
     * Build sequential FIRST sets from rest of item rule production
     * @param -- start : start iterator from position
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
      std::unordered_set<CanonicalItem> last_CLOSURE = CLOSURE;

      bool fixed_point = false;
      while (!fixed_point){
        for (const CanonicalItem& c_item : CLOSURE){
          RuleIdentifier rule_id = c_item.get_rule_id();
          Position position = c_item.get_position();

          const std::vector<SymbolAlias>& rule_symbols = ProductionProcesser::rules_.get_prod_symbols(rule_id);
          if (rule_symbols.empty()) continue; // NULL reduction

          const SymbolAlias& head_symbol = rule_symbols[position];
          if (head_symbol.terminal) continue; // terminal symbols don't contribute to the closure

          std::set<ProductionItem> seq_FIRST = sequence_FIRST(
            rule_symbols.begin() + position,
            rule_symbols.end(),
            c_item.get_lookaheads()
          );

          if (seq_FIRST.empty()) continue; // reduce sequence does not produce new items

          ProductionItem nt_alias = head_symbol.symbol;
          for (RuleIdentifier nt_prod_rule : ProductionProcesser::rules_.get_productions(nt_alias)){
            CanonicalItem new_item{nt_prod_rule, 0, seq_FIRST};
            if (CLOSURE.contains(new_item)){
              const CanonicalItem& c_item = *CLOSURE.find(new_item);
              c_item.include_lookahead(seq_FIRST);
            }
            else {
              CLOSURE.emplace(nt_prod_rule, 0, seq_FIRST);
            }
          }
        }
        fixed_point = (CLOSURE == last_CLOSURE);
        last_CLOSURE = CLOSURE;
      }
      return CLOSURE;
    }

    std::set<StateIdentifier> construct_state(StateIdentifier curr_state){
      std::set<StateIdentifier> new_states;

      ParseState& state = _states.at(curr_state);
      const std::unordered_set<CanonicalItem>& curr_closure = _states.at(curr_state).get_items();

      std::unordered_map<SymbolAlias, std::unordered_set<CanonicalItem>> transition_items;

      for (const CanonicalItem& item : curr_closure){
        RuleIdentifier rule_id = item.get_rule_id();
        const auto& rule = ProductionProcesser::rules_.get_prod_symbols(rule_id);

        Position pos = item.get_position();

        // end of a production -> REDUCE sequence (no next state, only reduce and pop state)
        if (pos == rule.size()){
          StateTransition state_transition = StateTransition(StateAction::REDUCE, rule_id);
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

    // get all ITEMS that participate in S-R or R-R conflicts for a given
    // input on state
    std::vector<std::vector<CanonicalItem>> get_conflict_items(const ParseState& state, ProductionItem input){
      std::vector<CanonicalItem> shift_items;
      std::vector<CanonicalItem> reduce_items;
      for (const auto& item : state.get_items()){
        RuleIdentifier rule = item.get_rule_id();
        Position pos = item.get_position();
        const std::vector<SymbolAlias>& rule_prods = ProductionProcesser::rules_.get_prod_symbols(rule);

        if (pos == rule_prods.size()){ // this is a REDUCE
          if (item.get_lookaheads().contains(input)){
            reduce_items.push_back(item);
          }
        }

        if (rule_prods[pos+1].symbol == input){ // this is a SHIFT
          shift_items.push_back(item);
        }
      }
      return {shift_items, reduce_items};
    }

    // format an item that can shift on an input,
    // A -> b B • c
    std::string format_shift_item(const CanonicalItem& item){
      RuleIdentifier rule_id = item.get_rule_id();
      const std::vector<SymbolAlias>& rule_prods = ProductionProcesser::rules_.get_prod_symbols(rule_id);
      ProductionItem prod_base = ProductionProcesser::rules_.get_base(rule_id);

      std::string res = ProductionProcesser::alias_.get_label(prod_base);
      res += " -> ";
      for (size_t i = 0; i < rule_prods.size(); i++){
        if (i == item.get_position()) res += "•";

        const SymbolAlias& symbol = rule_prods[i];
        res += ProductionProcesser::alias_.get_label(symbol.symbol);
      }
      res += '\n';
      return res;
    }

    // format an item that reduces on lookahead input,
    // A -> b B •  [ c ]
    std::string format_reduce_item(const CanonicalItem& item){
      RuleIdentifier rule_id = item.get_rule_id();
      const std::vector<SymbolAlias>& rule_prods = ProductionProcesser::rules_.get_prod_symbols(rule_id);
      ProductionItem prod_base = ProductionProcesser::rules_.get_base(rule_id);

      std::string res = ProductionProcesser::alias_.get_label(prod_base);
      res += " -> ";
      for (size_t i = 0; i < rule_prods.size(); i++){
        const SymbolAlias& symbol = rule_prods[i];
        res += ProductionProcesser::alias_.get_label(symbol.symbol);
      }
      res += "•\t";

      // format lookaheads
      res += "[ ";
      const auto& lookaheads = item.get_lookaheads();

      for (auto it = lookaheads.begin(); it != lookaheads.end(); it++){
        if (it != lookaheads.begin()) res += ", ";
        res += ProductionProcesser::alias_.get_label(*it);
      }
      res += " ]\n";
      return res;
    }

    // Format the conflict actions for each state
    // Group conflicts by double of {STATE, INPUT}
    // Push a formatted error for each SHIFT/REDUCE action
    void format_conflict_err(const ParseState& state, ProductionItem input, const std::unordered_set<StateTransition>& actions){
      std::string msg =
        MoschusString(
          Color::red,
          std::format(
            "State:{} has conflicting actions on Token:\"{}\" :\n",
            state.get_identifier(),
            ProductionProcesser::alias_.get_label(input)
          ).data()
        ).to_string();
      msg += "Contributing Items:\n";

      const auto CONFLICT_ITEMS = get_conflict_items(state, input);
      const std::vector<CanonicalItem>& SHIFT_ITEMS = CONFLICT_ITEMS[0];
      const std::vector<CanonicalItem>& REDUCE_ITEMS = CONFLICT_ITEMS[1];

      if (!SHIFT_ITEMS.empty()){
        msg += MoschusString(Color::yellow, "SHIFT occurs on items:\n").to_string();
      }
      for (const CanonicalItem& S_ITEM : SHIFT_ITEMS){
        msg+="\t"+format_shift_item(S_ITEM); 
      }

      if (!REDUCE_ITEMS.empty()){
        msg += MoschusString(Color::yellow, "REDUCE occurs on items:\n").to_string();
      }
      for (const CanonicalItem& R_ITEM : REDUCE_ITEMS){
        msg+="\t"+format_reduce_item(R_ITEM);
      }

      // push the formatted exception to errors
      MoschusExceptHandler::push_error(MoschusError(msg, MoschusErrorType::MoschusGeneric));
    }

    /**
     * Build final complete table, define actions for all terminals and non-terminals
     * with parser errors, also handle s-r, r-r conflict error logging here
     *
     * TODO : build a conflict example constructor with backwards traversal to root
     * store the non-terminals taken to root and reverse replacing with token names
     **/
    void build_table(){
      for (const auto& state_pair : _states){
        const ParseState& state = state_pair.second;
        std::unordered_map<ProductionItem, StateTransition> state_actions;

        std::unordered_set<StateTransition> actions;

        size_t N_CONFLICTS = 0;

        auto populate_table = [&](const auto& alias_symbols){
          for (ProductionItem input : alias_symbols){
            actions = state.get_action(input);
            if (actions.empty()){ // error: no such step to production
              state_actions.emplace(input, StateTransition(StateAction::ERROR, 0));
            }
            else if (actions.size() > 1){ // fatal generator error: r-r or s-r conflict
              state_actions.emplace(input, StateTransition(StateAction::CONFLICT, 0));
              size_t conflict_actions = actions.size();
              N_CONFLICTS += (conflict_actions * (conflict_actions - 1))/2;

              format_conflict_err(state, input, actions);
            }
            else { // just copy over action
              state_actions.emplace(input, *actions.begin());
            }
          }
        };
        populate_table(ProductionProcesser::alias_.get_terminals());
        populate_table(ProductionProcesser::alias_.get_nonterminals());

        // if (N_CONFLICTS > 0){
        //   // TODO : check verbosity flags --> if set rerun a bfs with tracking of path and generate all conflicts

        //   MoschusExceptHandler::push_error(
        //     MoschusError(
        //       MoschusString(Color::red, std::format("Provided grammar has {} SHIFT-REDUCE or REDUCE-REDUCE conflict(s).\n", N_CONFLICTS).data()).to_string()+
        //       MoschusString(Color::red, Modifier::italic, "[for more information use flag '-v', '--verbose' to generate up to 5 traces]").to_string(),
        //       MoschusErrorType::MoschusGeneric
        //     )
        //   );
        // }
        MoschusExceptHandler::log_warnings();
        MoschusExceptHandler::log_errors();

        _table.emplace(state.get_identifier(), state_actions);
      }
    }


    /***********************************************************************************
    ************************************************************************************
    **               VERBOSE TRACE HANDLING LOGIC
    **               ___________________________
    **
    *************************************************************************************
    ************************************************************************************/

    // build at most 5 verboce traces
    #define VERBOSE_TRACES size_t{5}

    // store the shortest terminating traces for non-terminal items
    std::unordered_map<ProductionItem, ConflictTrace> _shortest_trace;

    void build_shortest_trace(ProductionItem nt_base){
      if (_shortest_trace.contains(nt_base)) return; // already computed

      ConflictTrace shortest_trace(false, nt_base);
      size_t trace_length = SIZE_T_MAX;

      const auto& prod_rules = ProductionProcesser::rules_.get_productions(nt_base);
      for (RuleIdentifier rule : prod_rules){
        ConflictTrace candidate_trace(false, nt_base);

        const auto& prod_symbols = ProductionProcesser::rules_.get_prod_symbols(rule);
        for (const SymbolAlias& symbol : prod_symbols){
          if (symbol.terminal){
            candidate_trace.push_subtrace(
              ConflictTrace(true, symbol.symbol)
            );
          } else {
            build_shortest_trace(symbol.symbol); // recursively build shortest subtraces
            candidate_trace.push_subtrace(_shortest_trace.at(symbol.symbol));
          }
        }
        size_t candidate_length = candidate_trace.trace_length();

        // new shortest trace found
        if (candidate_length < trace_length){
          shortest_trace = candidate_trace;
          trace_length = candidate_length;
        }
      }
      _shortest_trace.emplace(nt_base, shortest_trace);
    }

    /**
    * Build conflict traces for ALL conflicts
    *
    * this is a very expensive operation so if there are a lot of conflicts it will take a LOT of resources
    * as such we actually only will generate the first MIN(N_CONFLICTS, 5) traces
    **/
    void verbose_conflicts(size_t N_CONFLICTS){
      size_t N_TRACES = std::min(VERBOSE_TRACES, N_CONFLICTS);
      // TODO : build ts

    }

  } // end of namespace (helpers)

  void reset() {
    _states.clear();
    _table.clear();
  }

  void generate_parse_table(const musk_ptr& ast){
    ParseState& start_state = get_start_state(ast);

    generate_states(start_state);

    build_table();
  }
};
