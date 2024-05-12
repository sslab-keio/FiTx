#include "frontend/Framework.hpp"
#include "frontend/State.hpp"

#include "UnrefDetector.hpp"

namespace {
class UnrefCountDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    UnreferenceCounter::defineStates(manager);
    addStateManager(manager);
  }
};

}  // namespace

std::vector<framework::FrameworkPass *> framework::FrameworkPass::passes = {
    new UnrefCountDetector()};
