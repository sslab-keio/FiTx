#pragma once

#include "lock.hpp"
#include "frontend/State.hpp"

namespace DoubleLock {
  void define_states(framework::StateManager& manager);
}
