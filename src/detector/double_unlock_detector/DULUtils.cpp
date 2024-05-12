#include "DUL_Detector.hpp"
#include "frontend/PropagationConstraint.hpp"

class LockInUnlockConstraint : public framework::StatefulConstraint {
 public:
  bool shouldPropagateOnCallInst(
      std::shared_ptr<framework::CallInst> inst) override {
    std::shared_ptr<framework::Function> called_func = inst->CalledFunction();
    std::shared_ptr<framework::Function> parent =
        inst->Parent().lock()->Parent().lock();
    if (!called_func || !parent) return true;

    if (called_func->Name().find("_unlock") == std::string::npos) return true;

    if (parent->Name().find("_lock") != std::string::npos) return false;
    return true;
  }
};

namespace DoubleUnlock {
void defineStates(framework::StateManager &manager) {
  // Create States
  framework::State &init = manager.getInitState();

  auto lock_args = framework::StateArgs("locked");
  framework::State &lock = manager.createState(lock_args);

  auto unlock_args = framework::StateArgs("unlocked");
  framework::State &unlock = manager.createState(unlock_args);

  auto dl_args =
      framework::StateArgs("double unlocked", framework::StateType::BUG);
  framework::State &double_unlock = manager.createState(dl_args);

  auto lock_rule = framework::FunctionArgTransitionRule(lock_funcs_w_try);
  auto store_any_rule = framework::StoreValueTransitionRule(
      framework::StoreValueTransitionRule::ANY);

  manager.addTransition(init, lock, lock_rule);

  auto unlock_rule = framework::FunctionArgTransitionRule(unlock_funcs);
  manager.addTransition(lock, unlock, unlock_rule);
  manager.addTransition(unlock, double_unlock, unlock_rule);

  manager.addTransition(unlock, lock, lock_rule);
  manager.addTransition(unlock, init, store_any_rule);

  manager.enableStatefulConstraint(std::make_shared<LockInUnlockConstraint>());
}
}  // namespace DoubleUnlock
