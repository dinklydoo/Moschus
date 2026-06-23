#include "frontend/musk_parser.hpp"
#include "preprocess.hpp"
#include "../errors/moschus_error.hpp"

namespace ProductionProcesser {
  ProductionStore store_;
  ProductionAlias alias_;
  ProductionRuleStore rules_;
}

// Method definitions for preprocesser objects

void ProductionAlias::new_alias(const std::string& label, bool terminal){
  std::unordered_map<std::string, ProductionItem>& _map =
    (terminal)? _terminal_alias : _nonterm_alias;

  if (_map.count(label)) return;

  _map.emplace(label, _registered_alias);
  _reverse_alias.emplace(_registered_alias, label);
  _registered_alias++;
}

ProductionItem ProductionAlias::try_alias(const std::string& label, bool terminal, bool& ok) const noexcept {
  const std::unordered_map<std::string, ProductionItem>& _map =
    (terminal)? _terminal_alias : _nonterm_alias;

  if (_map.count(label)){
    ok = true;
    return _map.at(label);
  }
  ok = false;
  return 0;
}

ProductionItem ProductionAlias::get_alias(const std::string& label, bool terminal) const {
  const std::unordered_map<std::string, ProductionItem>& _map =
    (terminal)? _terminal_alias : _nonterm_alias;
  try {
    return _map.at(label);
  } catch (std::out_of_range){
    std::string err_string = std::string("No alias found for ")+((terminal)?"terminal":"non-terminal")+" named: "+label;
    throw MoschusError(err_string, MoschusErrorType::BadAccess);
  }
}

std::string ProductionAlias::get_label(ProductionItem alias) const {
  return _reverse_alias.at(alias);
}

std::set<ProductionItem> ProductionAlias::get_terminals() const {
  std::set<ProductionItem> terminals;
  for (const auto& t_alias : _terminal_alias){
    terminals.insert(t_alias.second);
  }
  return terminals;
}

std::set<ProductionItem> ProductionAlias::get_nonterminals() const {
  std::set<ProductionItem> nonterminals;
  for (const auto& nt_alias : _nonterm_alias){
    nonterminals.insert(nt_alias.second);
  }
  return nonterminals;
}

void ProductionStore::new_object(ProductionItem item) {
  if (_store.contains(item)) return;

  _store.emplace(
    item,
    std::make_unique<ProductionObject>(item)
  );
}

ProductionObject* ProductionStore::try_object(ProductionItem item) const noexcept {
  if (_store.contains(item)) return _store.at(item).get();
  return nullptr;
}

ProductionObject& ProductionStore::get_object(ProductionItem item) const {
  try {
    const std::unique_ptr<ProductionObject>& _ptr = _store.at(item);
    if (_ptr == nullptr){
      throw MoschusError("Null object at production alias: "+std::to_string(item), MoschusErrorType::NullDeref);
    }
    return *_ptr;
  } catch (std::out_of_range){
    throw MoschusError("No object found for production alias: "+std::to_string(item), MoschusErrorType::BadAccess);
  }
}

void ProductionRuleStore::add_rule(RuleIdentifier rule_id, ProductionItem base, const std::vector<SymbolAlias>& consequent){
  _rules.emplace(rule_id, std::make_pair(base, consequent));
}

void ProductionRuleStore::add_productions(ProductionItem nt_alias, const std::vector<ProductionRule>& nt_prods){
  for (const auto& rule : nt_prods){
    _produces[nt_alias].push_back(rule.rule_identifier);
  }
}

const std::vector<SymbolAlias>& ProductionRuleStore::get_prod_symbols(RuleIdentifier rule_id) const {
  return _rules.at(rule_id).second;
}

SymbolAlias ProductionRuleStore::get_symbol_at(RuleIdentifier rule_id, unsigned long long pos) const{
  return get_prod_symbols(rule_id)[pos];
}

const std::vector<RuleIdentifier>& ProductionRuleStore::get_productions(ProductionItem item) const {
  return _produces.at(item);
}

ProductionItem ProductionRuleStore::get_base(RuleIdentifier rule_id) const {
  return _rules.at(rule_id).first;
}

void ProductionObject::FIRST_insert(ProductionItem item){
  FIRST.insert(item);
}

void ProductionObject::FIRST_union(const ProductionObject& other){
  if (&other == this) return; // same instance / direct left recursion

  const std::set<ProductionItem>& _oFIRST = other.get_FIRST();
  FIRST.insert(_oFIRST.begin(), _oFIRST.end());
}

void ProductionObject::FOLLOW_insert(ProductionItem item){
  FOLLOW.insert(item);
}

void ProductionObject::FOLLOW_union_first(const ProductionObject& other){
  // this time the same instance CAN modify the FOLLOW set in left-recursive productions
  const std::set<ProductionItem>& _oFIRST = other.get_FIRST();
  FOLLOW.insert(_oFIRST.begin(), _oFIRST.end());
}

void ProductionObject::FOLLOW_union_follow(const ProductionObject& other){
  if (&other == this) return;

  const std::set<ProductionItem>& _oFOLLOW = other.get_FOLLOW();
  FOLLOW.insert(_oFOLLOW.begin(), _oFOLLOW.end());
}

const std::set<ProductionItem>& ProductionObject::get_FIRST() const {
  return FIRST;
}

const std::set<ProductionItem>& ProductionObject::get_FOLLOW() const {
  return FOLLOW;
}

// testing APIs for clearing static stores
void ProductionAlias::reset() {
  _nonterm_alias.clear();
  _terminal_alias.clear();
  _reverse_alias.clear();
  
  _terminal_alias.emplace("__[EOF]__", 0);
  _reverse_alias.emplace(0, "$$");
  _registered_alias = 1;
}

void ProductionStore::reset() {
  _store.clear();
}

void ProductionRuleStore::reset(){
  _rules.clear();
  _produces.clear();
}