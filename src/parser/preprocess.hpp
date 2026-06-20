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
    // reserve 0 for EOF token ($$) or in parser ast representation "__[EOF]__"
    static inline unsigned long long _registered_alias = 1;
    std::unordered_map<std::string, ProductionItem> _terminal_alias;
    std::unordered_map<std::string, ProductionItem> _nonterm_alias;

    std::unordered_map<ProductionItem, std::string> _reverse_alias;
  public:
    ProductionAlias(){
      _terminal_alias.emplace("__[EOF]__", 0);
    }
    void new_alias(const std::string& label, bool terminal);

    ProductionItem try_alias(const std::string& label, bool terminal, bool& ok) const noexcept;
    ProductionItem get_alias(const std::string& label, bool terminal) const;

    std::string get_label(ProductionItem alias) const;

    std::set<ProductionItem> get_terminals() const;
    std::set<ProductionItem> get_nonterminals() const;
};

struct ProductionStore {
  private:
    std::unordered_map<ProductionItem, std::unique_ptr<ProductionObject>> _store;
  public:
    void new_object(ProductionItem);

    ProductionObject* try_object(ProductionItem alias) const noexcept;
    ProductionObject& get_object(ProductionItem alias) const;
};

struct SymbolAlias {
  ProductionItem symbol;
  bool terminal;

  bool operator==(const SymbolAlias& _other) const = default;
};

template<>
struct std::hash<SymbolAlias>{
  std::size_t operator()(const SymbolAlias& st) const {
    std::size_t h1 = std::hash<ProductionItem>{}(st.symbol);
    std::size_t h2 = std::hash<bool>{}(st.terminal);

    return h1 ^ (h2 << 1);
  }
};

struct ProductionRuleStore {
  private:
    std::unordered_map<RuleIdentifier, std::pair<ProductionItem, std::vector<SymbolAlias>>> _rules;
    std::unordered_map<ProductionItem, std::vector<RuleIdentifier>> _produces;
  public:
    void add_rule(RuleIdentifier rule_id, ProductionItem base, const std::vector<SymbolAlias>& consequent);
    void add_productions(ProductionItem nt_alias, const std::vector<ProductionRule>& nt_prods);

    const std::vector<SymbolAlias>& get_prod_symbols(RuleIdentifier rule_id) const;
    SymbolAlias get_symbol_at(RuleIdentifier rule_id, unsigned long long pos) const;
    const std::vector<RuleIdentifier>& get_productions(ProductionItem item) const;
    ProductionItem get_base(RuleIdentifier rule_id) const;
};

struct ProductionObject {
  private:
    ProductionItem production_root; // non-terminal lhs of production
    bool NULLABLE;
    std::set<ProductionItem> FIRST; // first set
    std::set<ProductionItem> FOLLOW; // follow set
  public:
    ProductionObject(ProductionItem root) : production_root(root), NULLABLE(false) {}
    ProductionItem get_root() const {return production_root;}
    void set_NULLABLE() {NULLABLE = true;}
    bool is_NULLABLE() const {return NULLABLE;}
    void FIRST_union(const ProductionObject& other); // join FIRST sets
    void FIRST_insert(ProductionItem item);
    void FOLLOW_union_first(const ProductionObject& other); // join FIRST of other into FOLLOW
    void FOLLOW_union_follow(const ProductionObject& other); // join FOLLOW of other into FOLLOW
    void FOLLOW_insert(ProductionItem item); // add a new FOLLOW token

    const std::set<ProductionItem>& get_FIRST() const;
    const std::set<ProductionItem>& get_FOLLOW() const;
};

namespace ProductionProcesser {
  extern ProductionStore store_;
  extern ProductionAlias alias_;
  extern ProductionRuleStore rules_;

  void process_musk_ast(const musk_ptr& ast);
}
