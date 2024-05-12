#include "frontend/Framework.hpp"
#include "frontend/State.hpp"

#include "DUL_Detector.hpp"

namespace {
class DoubleUnlockDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    DoubleUnlock::defineStates(manager);
    addStateManager(manager);
  }
};
}  // namespace

std::vector<framework::FrameworkPass *> framework::FrameworkPass::passes = {
    new DoubleUnlockDetector()};
