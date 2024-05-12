#include "frontend/Framework.hpp"
#include "frontend/State.hpp"
#include "RefDetector.hpp"

namespace {
class RefCountDetector : public framework::FrameworkPass {
  virtual void defineStates() override {
    framework::StateManager manager;
    ReferenceCounter::defineStates(manager);
    addStateManager(manager);
  }
};

}  // namespace

std::vector<framework::FrameworkPass *> framework::FrameworkPass::passes = {
    new RefCountDetector()};
