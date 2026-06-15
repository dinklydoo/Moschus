#include "preprocess.hpp"
#include "../errors/moschus_error.hpp"

namespace ProductionProcesser {
  ProductionStore store_;
  ProductionAlias alias_;
}

// Method definitions for preprocesser objects

void ProductionAlias::new_alias(const std::string& label, bool terminal){
  std::unordered_map<std::string, ProductionItem>& _map =
    (terminal)? _terminal_alias : _nonterm_alias;

  if (_map.count(label)) return;

  _map.emplace(label, _registered_alias);
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

void ProductionObject::FIRST_insert(ProductionItem item){
  FIRST.insert(item);
}

// TODO : remember to take lfp of Union, topological is not enough for circular dependency (A -> B | some tokens) (B -> A | some tokens)
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
