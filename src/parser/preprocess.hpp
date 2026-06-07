#pragma once
#include <string>
#include <unordered_map>
#include <set>
#include <memory>

#include "frontend/musk_parser.hpp"

struct ProductionAlias;
struct ProductionStore;
struct ProductionObject;

// alias productions into Production Items
using ProductionItem = unsigned long long;

struct ProductionAlias {
  private:
    // global count of number of productionItems
    static unsigned long long registered_alias;
    std::unordered_map<std::string, ProductionItem> terminal_alias;
    std::unordered_map<std::string, ProductionItem> nonterm_alias;

  public:
    ProductionAlias(){
      // reserve 0 for EOF token ($$) or in parser ast representation "__[EOF]__"
      registered_alias=1;
      terminal_alias.emplace("__[EOF]__", 0);
    }
    void new_alias(const std::string& label, bool terminal);

    ProductionItem try_alias(const std::string& label, bool terminal, bool& ok) const noexcept;
    ProductionItem get_alias(const std::string& label, bool terminal) const;
};

struct ProductionStore {
  private:
    std::unordered_map<ProductionItem, std::unique_ptr<ProductionObject>> _store;
  public:
    void new_object(ProductionItem);

    ProductionObject* try_object(ProductionItem alias) const noexcept;
    ProductionObject& get_object(ProductionItem alias) const;
};

struct ProductionObject {
  private:
    ProductionItem production_root; // non-terminal lhs of production
    bool processed; // has been processed recursively
    std::set<ProductionItem> FIRST; // first set
    std::set<ProductionItem> FOLLOW; // follow set
  public:
    ProductionObject(ProductionItem root) : production_root(root), processed(false) {}
    ProductionItem get_root() const;
    void FIRST_union(const ProductionObject& other); // join FIRST sets
    void FIRST_insert(ProductionItem item);
    void FOLLOW_insert(ProductionItem item); // add a new FOLLOW token

    const std::set<ProductionItem>& get_FIRST() const;
    const std::set<ProductionItem>& get_FOLLOW() const;
};

namespace ProductionProcesser {
  static ProductionStore _store;
  static ProductionAlias _alias;

  void process_musk_ast(const musk_ptr& ast);

}
