#include "RefDetector.hpp"
#include "frontend/State.hpp"
#include "frontend/StateTransition.hpp"

namespace ReferenceCounter {
void defineStates(framework::StateManager& manager) {
  // Create States
  framework::State& init = manager.getInitState();

  /* auto init_rule = framework::FunctionArgTransitionRule(init_funcs); */
  auto inc_rule = framework::FunctionArgTransitionRule(inc_funcs);
  auto dec_rule = framework::FunctionArgTransitionRule(dec_funcs);

  framework::State* prev = &init;
  for (int i = 0; i < 10; i++) {
    framework::StateType type =
        i == 1 ? framework::StateType::BUG : framework::StateType::NORMAL;
    auto counted_args = framework::StateArgs("counted " + std::to_string(i),
                                             type, framework::MODULE_END);

    framework::State& counted = manager.createState(counted_args);
    manager.addTransition(*prev, counted, inc_rule);
    manager.addTransition(counted, *prev, dec_rule);
    prev = &counted;
  }
}
}  // namespace ReferenceCounter
