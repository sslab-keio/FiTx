#include "frontend/Framework.hpp"
#include "frontend/State.hpp"

#include "DL_Detector.hpp"

namespace {
class DoubleLockDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    DoubleLock::define_states(manager);
    addStateManager(manager);
  }
};

}  // namespace

std::vector<framework::FrameworkPass *> framework::FrameworkPass::passes = {
    new DoubleLockDetector()};
