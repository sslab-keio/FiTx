#include "Leak_Detector.hpp"
#include "frontend/Framework.hpp"
#include "frontend/State.hpp"

namespace {
class LeakDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    MemoryLeak::defineStates(manager);  
    addStateManager(manager);
  }
};

}  // namespace

std::vector<framework::FrameworkPass *> framework::FrameworkPass::passes = {
    new LeakDetector()};
