#pragma once
#include <vector>
#include <string>

#include "alloc.hpp"
#include "frontend/State.hpp"

namespace UseAfterFree {
  void defineStates(framework::StateManager& manager);
}
