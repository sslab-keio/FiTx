#include "UAF_Detector.hpp"

#include "frontend/Framework.hpp"
#include "frontend/State.hpp"

namespace {
class UseAfterFreeDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    UseAfterFree::defineStates(manager);
    addStateManager(manager);
  }
};

}  // namespace

std::vector<framework::FrameworkPass *> framework::FrameworkPass::passes = {
    new UseAfterFreeDetector()};
