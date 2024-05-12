#pragma once
#include <string>
#include <vector>

#include "frontend/StateTransition.hpp"

const std::vector<std::string> alloc_funcs = {
    "malloc",        "calloc",           "kzalloc",
    "kmalloc",       "zalloc",           "vmalloc",
    "kcalloc",       "vzalloc",          "kzalloc_node",
    "kmalloc_array", "kmem_cache_alloc", "kmem_cache_alloc_node",
    /* "memdup",        "kmemdup",          "kstrdup" */
};

const std::vector<std::string> free_funcs = {
    "free",   "kfree",           "kzfree",         "vfree",
    "kvfree", "kmem_cache_free", "kfree_sensitive"};

const std::vector<framework::FunctionArgTransitionRule::FunctionArg>
    store_related = {
        framework::FunctionArgTransitionRule::FunctionArg(
            "__drm_atomic_helper_crtc_reset", 1, true),
        framework::FunctionArgTransitionRule::FunctionArg(
            "__drm_atomic_helper_connector_reset", 1, true),
        framework::FunctionArgTransitionRule::FunctionArg("list_add_tail", 0,
                                                          true),
        framework::FunctionArgTransitionRule::FunctionArg("list_add", 0, true),
};
