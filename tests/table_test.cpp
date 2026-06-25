#include <gtest/gtest.h>
#include "test_builder.hpp"
#include "../src/parser/table_gen.hpp"
#include "../src/errors/moschus_error.hpp"

struct LabelledTransition {
  std::string label;
  StateAction action;
  StateIdentifier next_state;
};

//TODO : check rule aliasing, bit annoying rn because rules are still static and we need to hash them

// Check the table topology,
// Topology equivalence implies full table equivalence in LR(1)
// Topology must include SHIFT/REDUCE/GOTO actions
void check_topology(const std::map<StateIdentifier, std::vector<LabelledTransition>>& expected){
  const auto& table = ParseTable::_table;
  ASSERT_EQ(expected.size(), table.size());

  std::map<StateIdentifier, StateIdentifier> table_defined = {{0,0}}; // start state maps to start
  for (const auto& state : expected){
    StateIdentifier table_state = table_defined.at(state.first);

    const std::vector<LabelledTransition>& transitions = state.second;

    const auto& p_state = table.at(table_state);

    for (const LabelledTransition transition : transitions){
      ProductionItem transition_item;
      bool term_item = (transition.action != StateAction::GOTO);

      transition_item = ProductionProcesser::alias_.get_alias(transition.label, term_item); // non-terminal for GOTO
      const StateTransition& table_transition =  p_state.at(transition_item);

      ASSERT_EQ(table_transition.action, transition.action);

      if (transition.action != StateAction::REDUCE){
        if (table_defined.contains(transition.next_state)){
          ASSERT_EQ(table_defined.at(transition.next_state), table_transition.action_id);
        } else {
          table_defined.emplace(transition.next_state, table_transition.action_id);
        }
      }
    }
  }
}



TEST(TABLE, EXAMPLE){
  musk_ptr ast = TestBuilder::build_ast("example.musk");
  ProductionProcesser::process_musk_ast(ast);
  ParseTable::generate_parse_table(ast);

  // TODO : maybe serialize this but i cbf rn
  std::map<StateIdentifier, std::vector<LabelledTransition>> topo_example = {
    {0, 
      {
        {"ADD_EXP", StateAction::GOTO, 1},
        {"SUB_EXP", StateAction::GOTO, 2},
        {"MUL_EXP", StateAction::GOTO, 3},
        {"DIV_EXP", StateAction::GOTO, 4},
        {"INT", StateAction::SHIFT, 5}
      }
    },
    {1, 
      {
        {"__[EOF]__", StateAction::SHIFT, 6}
      }
    },
    {2,
      {
        {"ADD", StateAction::SHIFT, 7},
        {"__[EOF]__", StateAction::REDUCE, 0},
      }
    },
    {3,
      {
        {"SUB", StateAction::SHIFT, 8},
        {"__[EOF]__", StateAction::REDUCE, 0},
        {"ADD", StateAction::REDUCE, 0}
      }
    },
    {4,
      {
        {"MUL", StateAction::SHIFT, 9},
        {"__[EOF]__", StateAction::REDUCE, 0},
        {"ADD", StateAction::REDUCE, 0},
        {"SUB", StateAction::REDUCE, 0}
      }
    },
    {5,
      {
        {"DIV", StateAction::SHIFT, 10},
        {"__[EOF]__", StateAction::REDUCE, 0},
        {"ADD", StateAction::REDUCE, 0},
        {"SUB", StateAction::REDUCE, 0},
        {"MUL", StateAction::REDUCE, 0}
      }
    },
    {6,
      {
        // TODO : start state lookahead should be all or none? --> decide later
      }
    },
    {7,
      {
        {"ADD_EXP", StateAction::GOTO, 11},
        {"SUB_EXP", StateAction::GOTO, 2},
        {"MUL_EXP", StateAction::GOTO, 3},
        {"DIV_EXP", StateAction::GOTO, 4},
        {"INT", StateAction::SHIFT, 5}
      }
    },
    {8,
      {
        {"SUB_EXP", StateAction::GOTO, 12},
        {"MUL_EXP", StateAction::GOTO, 3},
        {"DIV_EXP", StateAction::GOTO, 4},
        {"INT", StateAction::SHIFT, 5}
      }
    },
    {9,
        {
          {"MUL_EXP", StateAction::GOTO, 13},
          {"DIV_EXP", StateAction::GOTO, 4},
          {"INT", StateAction::SHIFT, 5}
        }
    },
    {10,
        {
          {"DIV_EXP", StateAction::GOTO, 14},
          {"INT", StateAction::SHIFT, 5}
        }
    },
    {11,
        {
          {"__[EOF]__", StateAction::REDUCE, 0}
        }
    },
    {12,
        {
          {"__[EOF]__", StateAction::REDUCE, 0},
          {"ADD", StateAction::REDUCE, 0}
        }
    },
    {13,
        {
          {"__[EOF]__", StateAction::REDUCE, 0},
          {"ADD", StateAction::REDUCE, 0},
          {"SUB", StateAction::REDUCE, 0}
        }
    },
    {14,
        {
          {"__[EOF]__", StateAction::REDUCE, 0},
          {"ADD", StateAction::REDUCE, 0},
          {"SUB", StateAction::REDUCE, 0},
          {"MUL", StateAction::REDUCE, 0}
        }
    }
  };

  check_topology(topo_example);
  EXPECT_NO_THROW();

  TestBuilder::reset();
}

TEST(TABLE, NULLABLE){
  musk_ptr ast = TestBuilder::build_ast("nullable.musk");
  ProductionProcesser::process_musk_ast(ast);
  ParseTable::generate_parse_table(ast);

  std::map<StateIdentifier, std::vector<LabelledTransition>> topo_nullable = {
    {0,
      {
        {"A", StateAction::GOTO, 1},
        {"a", StateAction::SHIFT, 2},
        {"b", StateAction::REDUCE, 0},
        {"c", StateAction::REDUCE, 0},
        {"d", StateAction::REDUCE, 0},
        {"__[EOF]__", StateAction::REDUCE, 0}
      }
    },
    {1,
      {
        {"B", StateAction::GOTO, 3},
        {"b", StateAction::SHIFT, 4},
        {"c", StateAction::REDUCE, 0},
        {"d", StateAction::REDUCE, 0},
        {"__[EOF]__", StateAction::REDUCE, 0}
      }
    },
    {2,
      {
        {"b", StateAction::REDUCE, 0},
        {"c", StateAction::REDUCE, 0},
        {"d", StateAction::REDUCE, 0},
        {"__[EOF]__", StateAction::REDUCE, 0}
      }
    },
    {3,
      {
        {"C", StateAction::GOTO, 5},
        {"c", StateAction::SHIFT, 6},
        {"d", StateAction::REDUCE, 0},
        {"__[EOF]__", StateAction::REDUCE, 0}
      }
    },
    {4,
      {
        {"c", StateAction::REDUCE, 0},
        {"d", StateAction::REDUCE, 0},
        {"__[EOF]__", StateAction::REDUCE, 0}
      }
    },
    {5,
      {
        {"D", StateAction::GOTO, 7},
        {"d", StateAction::SHIFT, 8},
        {"__[EOF]__", StateAction::REDUCE, 0}
      }
    },
    {6,
      {
        {"d", StateAction::REDUCE, 0},
        {"__[EOF]__", StateAction::REDUCE, 0}
      }
    },
    {7,
      {
        {"__[EOF]__", StateAction::SHIFT, 9}
      }
    },
    {8,
      {
        {"__[EOF]__", StateAction::REDUCE, 0}
      }
    },
    {9,
      {
        // TODO -> lookahead on start
      }
    }
  };

  check_topology(topo_nullable);
  EXPECT_NO_THROW();

  TestBuilder::reset();
}

TEST(TABLE, CONFLICT){
  musk_ptr ast = TestBuilder::build_ast("conflict.musk");
  ProductionProcesser::process_musk_ast(ast);

  EXPECT_THROW(ParseTable::generate_parse_table(ast), MoschusError);

  TestBuilder::reset();
}