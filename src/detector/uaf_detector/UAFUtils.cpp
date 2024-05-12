#include "UAF_Detector.hpp"
#include "frontend/State.hpp"

namespace UseAfterFree {
class OneshotCallConstraint : public framework::StatefulConstraint {
 public:
  bool shouldPropagateOnCallInst(
      std::shared_ptr<framework::CallInst> inst) override {
    std::shared_ptr<framework::Function> called_func = inst->CalledFunction();
    std::shared_ptr<framework::Function> parent =
        inst->Parent().lock()->Parent().lock();
    if (!called_func || !parent) return true;

    // If "put"  is in the function name, it is most-likely reference counted
    // function. Hence, do not consider them
    if (called_func->Name().find("put") != std::string::npos) {
      return false;
    }
    return true;
  }
};

void defineStates(framework::StateManager& manager) {
  // Create States
  framework::State& init = manager.getInitState();

  auto store_args = framework::StateArgs("free");
  framework::State& free = manager.createState(store_args);

  auto free_func_rule = framework::FunctionArgTransitionRule(free_funcs);
  manager.addTransition(init, free, free_func_rule);

  auto ubi_args =
      framework::StateArgs("Used", framework::StateType::BUG,
                           framework::BugNotificationTiming::IMMEDIATE, false);
  framework::State& uaf = manager.createState(ubi_args);

  auto use_rule = framework::UseValueTransitionRule();
  manager.addTransition(free, uaf, use_rule);

  auto store_any_rule = framework::StoreValueTransitionRule(
      framework::StoreValueTransitionRule::ANY);

  manager.addTransition(free, init, store_any_rule);
  manager.enableStatefulConstraint(std::make_shared<OneshotCallConstraint>());
}
}  // namespace UseAfterFree
