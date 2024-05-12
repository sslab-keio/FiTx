#include "Leak_Detector.hpp"
#include "alloc.hpp"
#include "frontend/State.hpp"
#include "frontend/StateTransition.hpp"

namespace MemoryLeak {
void defineStates(framework::StateManager& manager) {
  // Create States
  framework::State& init = manager.getInitState();

  auto alloc_args =
      framework::StateArgs("allocated", framework::StateType::BUG,
                           framework::BugNotificationTiming::MODULE_END, false,
                           framework::NON_RETURN);

  framework::State& allocated = manager.createState(alloc_args);

  auto store_alloc_rule = framework::StoreValueTransitionRule(
      framework::StoreValueTransitionRule::CALL_FUNC, alloc_funcs);

  manager.addTransition(init, allocated, store_alloc_rule);

  auto store_any_rule = framework::StoreValueTransitionRule(
      framework::StoreValueTransitionRule::ANY);
  manager.addTransition(allocated, init, store_any_rule);

  auto alias_rule = framework::AliasValueTransitionRule();
  manager.addTransition(allocated, init, alias_rule);

  auto call_free_rule = framework::FunctionArgTransitionRule(free_funcs);
  manager.addTransition(allocated, init, call_free_rule);

  auto store_related_rule = framework::FunctionArgTransitionRule(store_related);
  manager.addTransition(allocated, init, store_related_rule);
}
}  // namespace MemoryLeak
