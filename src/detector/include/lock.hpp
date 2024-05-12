#pragma once
#include <string>
#include <vector>

#include "frontend/StateTransition.hpp"

const std::vector<framework::FunctionArgTransitionRule::FunctionArg>
    lock_funcs = {
        framework::FunctionArgTransitionRule::FunctionArg("spin_lock"),
        framework::FunctionArgTransitionRule::FunctionArg("spin_lock_irq"),
        framework::FunctionArgTransitionRule::FunctionArg("spin_lock_irqsave"),
        framework::FunctionArgTransitionRule::FunctionArg("mutex_lock"),
        framework::FunctionArgTransitionRule::FunctionArg("mutex_lock_nested"),
        framework::FunctionArgTransitionRule::FunctionArg(
            "mutex_lock_interruptable"),
        framework::FunctionArgTransitionRule::FunctionArg(
            "mutex_lock_interruptable_nested"),
        framework::FunctionArgTransitionRule::FunctionArg(
            "mutex_lock_killable"),
        framework::FunctionArgTransitionRule::FunctionArg(
            "refcount_dec_and_mutex_lock", 1),
        framework::FunctionArgTransitionRule::FunctionArg("atomic_dec_and_lock",
                                                          1),
        framework::FunctionArgTransitionRule::FunctionArg(
            "_atomic_dec_and_lock", 1),
};

const std::vector<framework::FunctionArgTransitionRule::FunctionArg>
    try_lock_funcs = {
        framework::FunctionArgTransitionRule::FunctionArg("spin_trylock"),
        framework::FunctionArgTransitionRule::FunctionArg("mutex_trylock"),
};

const std::vector<framework::FunctionArgTransitionRule::FunctionArg>
    lock_funcs_w_try = {
        framework::FunctionArgTransitionRule::FunctionArg("spin_lock"),
        framework::FunctionArgTransitionRule::FunctionArg("spin_trylock"),
        framework::FunctionArgTransitionRule::FunctionArg("spin_lock_irq"),
        framework::FunctionArgTransitionRule::FunctionArg("spin_lock_irqsave"),
        framework::FunctionArgTransitionRule::FunctionArg("mutex_lock"),
        framework::FunctionArgTransitionRule::FunctionArg("mutex_lock_nested"),
        framework::FunctionArgTransitionRule::FunctionArg("mutex_trylock"),
        framework::FunctionArgTransitionRule::FunctionArg(
            "mutex_lock_interruptable"),
        framework::FunctionArgTransitionRule::FunctionArg(
            "mutex_lock_interruptable_nested"),
        framework::FunctionArgTransitionRule::FunctionArg(
            "mutex_lock_killable"),
        framework::FunctionArgTransitionRule::FunctionArg(
            "refcount_dec_and_mutex_lock", 1),
        framework::FunctionArgTransitionRule::FunctionArg("atomic_dec_and_lock",
                                                          1),
        framework::FunctionArgTransitionRule::FunctionArg(
            "_atomic_dec_and_lock", 1),
};

const std::vector<std::string> unlock_funcs = {
    "spin_unlock", "spin_unlock_irq", "spin_unlock_irqsave", "mutex_unlock"};
