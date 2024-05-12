#include "frontend/Framework.hpp"
#include "frontend/State.hpp"
#include "include/DF_Detector.hpp"

namespace {
class DoubleFreeDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    // Create States
    auto init_args = framework::StateArgs("init", framework::StateType::INIT);
    framework::State &init = manager.createState(init_args);

    auto free_args = framework::StateArgs("free");
    framework::State &free = manager.createState(free_args);

    auto df_args =
        framework::StateArgs("double free", framework::StateType::BUG);
    framework::State &double_free = manager.createState(df_args);

    // Create rule for Function Arg Transition (i.e. when the value is passed
    // as the argument of specified functions)
    auto free_func_rule = framework::FunctionArgTransitionRule(free_funcs);

    manager.addTransition(init, free, free_func_rule);
    manager.addTransition(free, double_free, free_func_rule);

    // Create rule for NULL Store Transition (i.e. when the value is nulled)
    auto store_any_rule = framework::StoreValueTransitionRule(
        framework::StoreValueTransitionRule::ANY);

    manager.addTransition(free, init, store_any_rule);
    addStateManager(manager);
  }
};

class UseAfterFreeDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    // Create States
    auto init_args = framework::StateArgs("init", framework::StateType::INIT);
    framework::State &init = manager.createState(init_args);

    auto store_args = framework::StateArgs("free");
    framework::State &free = manager.createState(store_args);

    auto free_func_rule = framework::FunctionArgTransitionRule(free_funcs);
    manager.addTransition(init, free, free_func_rule);

    auto ubi_args = framework::StateArgs("Used", framework::StateType::BUG);
    framework::State &uaf = manager.createState(ubi_args);

    auto use_rule = framework::UseValueTransitionRule();
    manager.addTransition(free, uaf, use_rule);

    auto store_any_rule = framework::StoreValueTransitionRule(
        framework::StoreValueTransitionRule::ANY);

    manager.addTransition(free, init, store_any_rule);
    addStateManager(manager);
  }
};

}  // namespace

std::vector<framework::FrameworkPass *> framework::FrameworkPass::passes = {
    new DoubleFreeDetector(), new UseAfterFreeDetector()};
