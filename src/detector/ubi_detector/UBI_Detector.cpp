#include "include/UBI_Detector.hpp"

#include "frontend/Framework.hpp"
#include "frontend/State.hpp"

namespace {
class UseBeforeInitializationDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    // Create States
    auto init_args = framework::StateArgs("init", framework::StateType::INIT);
    framework::State &init = manager.createState(init_args);

    auto store_args = framework::StateArgs("store");
    framework::State &stored = manager.createState(store_args);

    auto ubi_args = framework::StateArgs(
        "UBI", framework::StateType::BUG,
        framework::BugNotificationTiming::IMMEDIATE, false);
    framework::State &ubi = manager.createState(ubi_args);

    auto use_rule = framework::UseValueTransitionRule();
    manager.addTransition(init, ubi, use_rule);

    auto store_any_rule = framework::StoreValueTransitionRule(
        framework::StoreValueTransitionRule::ANY);

    manager.addTransition(init, stored, store_any_rule);
    addStateManager(manager);
  }
};

}  // namespace

std::vector<framework::FrameworkPass *> framework::FrameworkPass::passes = {
    new UseBeforeInitializationDetector()};
