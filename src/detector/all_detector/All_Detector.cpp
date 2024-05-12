#include "include/All_Detector.hpp"

#include "frontend/Framework.hpp"
#include "frontend/State.hpp"

namespace {
class AllDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    for (auto& define_states : def_funcs) {
      framework::StateManager manager;
      define_states(manager);
      addStateManager(manager);
    }
  }
};
}  // namespace

std::vector<framework::FrameworkPass*> framework::FrameworkPass::passes = {
    new AllDetector()};
