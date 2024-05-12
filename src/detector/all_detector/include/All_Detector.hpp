#pragma once
#include <functional>
#include <string>
#include <vector>

#include "DF_Detector.hpp"
#include "DL_Detector.hpp"
#include "DUL_Detector.hpp"
#include "Leak_Detector.hpp"
#include "RefDetector.hpp"
#include "UAF_Detector.hpp"
#include "UnrefDetector.hpp"
#include "frontend/State.hpp"

const auto def_funcs = {
    DoubleFree::define_states,        DoubleLock::define_states,
    DoubleUnlock::defineStates,       MemoryLeak::defineStates,
    /* UnreferenceCounter::defineStates, */
    ReferenceCounter::defineStates,
    UseAfterFree::defineStates,
};
