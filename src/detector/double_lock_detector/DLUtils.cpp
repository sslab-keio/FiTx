#include "DL_Detector.hpp"
#include "frontend/State.hpp"
#include "lock.hpp"

class TryLockConstraint : public framework::StatefulConstraint {
 public:
  bool shouldPropagateOnCallInst(
      std::shared_ptr<framework::CallInst> inst) override {
    std::shared_ptr<framework::Function> called_func = inst->CalledFunction();
    std::shared_ptr<framework::Function> parent =
        inst->Parent().lock()->Parent().lock();
    if (!called_func || !parent) return true;

    return called_func->Name().find("_trylock") == std::string::npos;
  }
};

namespace DoubleLock {
void define_states(framework::StateManager& manager) {
  // Create States
  framework::State& init = manager.getInitState();

  auto lock_args = framework::StateArgs("locked");
  framework::State& lock = manager.createState(lock_args);

  auto dl_args =
      framework::StateArgs("double locked", framework::StateType::BUG);
  framework::State& double_lock = manager.createState(dl_args);

  auto lock_rule = framework::FunctionArgTransitionRule(lock_funcs_w_try);
  auto lock_rule_wo_try = framework::FunctionArgTransitionRule(lock_funcs);
  manager.addTransition(init, lock, lock_rule_wo_try);
  manager.addTransition(lock, double_lock, lock_rule_wo_try);

  auto unlock_rule = framework::FunctionArgTransitionRule(unlock_funcs);
  auto store_any_rule = framework::StoreValueTransitionRule(
      framework::StoreValueTransitionRule::ANY);

  manager.addTransition(lock, init, unlock_rule);
  manager.addTransition(lock, init, store_any_rule);

  manager.enableStatefulConstraint(std::make_shared<TryLockConstraint>());
}
}  // namespace DoubleLock
