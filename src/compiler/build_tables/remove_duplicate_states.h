#ifndef COMPILER_BUILD_TABLES_REMOVE_DUPLICATE_STATES_H_
#define COMPILER_BUILD_TABLES_REMOVE_DUPLICATE_STATES_H_

#include <map>
#include <vector>

#include "/Users/max/github/node-tree-sitter/vendor/tree-sitter/spec/helpers/stream_methods.h"

namespace tree_sitter {
namespace build_tables {

template <typename TableType, typename ActionType>
void remove_duplicate_states(TableType *table) {
  std::map<size_t, size_t> replacements;

  while (true) {
    bool did_add_new_replacement = false;
    for (size_t i = 0, size = table->states.size(); i < size; i++) {
      if (replacements.count(i))
        continue;
      for (size_t j = 0; j < i; j++) {
        if (replacements.count(i))
          continue;
        if (table->merge_state(j, i)) {
          did_add_new_replacement = true;
          replacements.insert({ i, j });
          break;
        }
      }
    }

    if (!did_add_new_replacement)
      break;

    for (auto &state : table->states)
      state.each_advance_action([&replacements](ActionType *action) {
        auto replacement = replacements.find(action->state_index);
        if (replacement != replacements.end())
          action->state_index = replacement->second;
      });
  }

  std::cout << replacements << "\n\n";

  std::vector<size_t> removed_state_indices;
  for (auto begin = table->states.begin(), i = begin; i != table->states.end();) {
    size_t state_index = i - begin;
    size_t original_state_index = state_index + removed_state_indices.size();
    // printf("State_index: %lu, original state index:%lu, size:%lu\n", state_index, original_state_index, table->states.size());

    auto replacement_entry = replacements.find(original_state_index);
    if (replacement_entry != replacements.end()) {
      // printf("  erasing\n");

      table->states.erase(i);
      removed_state_indices.push_back(original_state_index);
    } else {
      i->each_advance_action([&replacements](ActionType *action) {
        size_t number_of_prior_states_removed = 0;
        for (auto &replacement : replacements) {
          if (replacement.first >= action->state_index)
            break;
          number_of_prior_states_removed++;
        }

        // if (action->state_index != previous_action_state_index)
        //   printf("  adjusting action state: %lu -> %lu\n", previous_action_state_index, action->state_index);

        action->state_index -= number_of_prior_states_removed;
      });
      ++i;
    }
  }
}

}  // namespace build_tables
}  // namespace tree_sitter

#endif  // COMPILER_BUILD_TABLES_REMOVE_DUPLICATE_STATES_H_
