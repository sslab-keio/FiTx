#include "DF_Detector.hpp"

#include "frontend/Framework.hpp"
#include "frontend/State.hpp"

namespace {
class DoubleFreeDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    DoubleFree::define_states(manager);
    addStateManager(manager);
  }
};

}  // namespace

std::vector<framework::FrameworkPass *> framework::FrameworkPass::passes = {
    new DoubleFreeDetector()};
