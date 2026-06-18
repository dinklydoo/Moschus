#pragma once

#include "frontend/musk_parser.hpp"
#include "preprocess.hpp"

#include <unordered_set>

struct ParseState;
struct CanonicalItem;
struct StateTransition;

enum struct StateAction {
SHIFT,
REDUCE,
GOTO,
ERROR,
CONFLICT // placeholder conflict signal, tables will never escape generation with CONFLICT existing. (Fatal errors)
};

using Position = unsigned long long;
using StateIdentifier = unsigned long long;

struct StateTransition {
  StateAction action;
  StateIdentifier next_state;

  bool operator==(const StateTransition& _other) const = default;
};

template<>
struct std::hash<StateTransition>{
  std::size_t operator()(const StateTransition& st) const {
    std::size_t h1 = std::hash<StateAction>{}(st.action);
    std::size_t h2 = std::hash<StateIdentifier>{}(st.next_state);

    return h1 ^ (h2 << 1);
  }
};

struct CanonicalItem {
  private:
    RuleIdentifier _rule;
    const Position _position;
    mutable std::set<ProductionItem> _lookahead;
  public:
    CanonicalItem(RuleIdentifier rule, Position pos) : _rule(rule), _position(pos) {}
    CanonicalItem(RuleIdentifier rule, Position pos, const std::set<ProductionItem>& lookahead) : CanonicalItem(rule, pos){ _lookahead = lookahead; }

    void include_lookahead(const std::set<ProductionItem>& items) const{
      _lookahead.insert(items.begin(), items.end());
    }

    RuleIdentifier get_rule_id() const { return _rule; }
    Position get_position() const { return _position; }
    const std::set<ProductionItem>& get_lookaheads() const { return _lookahead; }
    CanonicalItem next_item() const { return CanonicalItem(_rule, _position+1, _lookahead); }

    bool operator==(const CanonicalItem& _other) const {
      if (this == &_other) return true;
      return _rule == _other._rule && _position == _other._position;
    }
};

template<>
struct std::hash<CanonicalItem>{
  std::size_t operator()(const CanonicalItem& item) const {
    std::size_t h1 = std::hash<RuleIdentifier>{}(item.get_rule_id());
    std::size_t h2 = std::hash<Position>{}(item.get_position());

    return h1 ^ (h2 << 1);
  }
};

struct ParseState {
  private:
    const StateIdentifier _state;
    std::unordered_set<CanonicalItem> _items;
    std::unordered_map<ProductionItem, std::unordered_set<StateTransition>> _actions; // for shift/reduce, on token input
  public:
    ParseState(StateIdentifier state) : _state(state) {}

    bool equivalent(std::unordered_set<CanonicalItem> items) const { return items == _items; }
    void add_item(const CanonicalItem& item){ _items.insert(item); }

    void add_reduce(const std::set<ProductionItem>& lookaheads, const StateTransition& transition){
      for (ProductionItem lookahead : lookaheads){
        _actions[lookahead].insert(transition);
      }
    }
    void add_shift_goto(ProductionItem item, const StateTransition& transition){
      _actions[item].insert(transition);
    }
    std::unordered_set<StateTransition> get_action(ProductionItem input) const {
      if (_actions.contains(input)) return _actions.at(input);
      return std::unordered_set<StateTransition>{};
    }

    const std::unordered_set<CanonicalItem>& get_items() const { return _items; }
    StateIdentifier get_identifier() const { return _state; }
};

namespace ParseTable {
  using TransitionTable = std::unordered_map<StateIdentifier, std::unordered_map<ProductionItem, StateTransition>>;
  extern TransitionTable _table;

  void clear(); // testing API

  void generate_parse_table(const musk_ptr& ast);
}
