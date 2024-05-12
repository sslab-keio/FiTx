#pragma once
#include <string>
#include <vector>

#include "refcount.hpp"
#include "frontend/State.hpp"

namespace UnreferenceCounter {
  void defineStates(framework::StateManager& manager);
}
