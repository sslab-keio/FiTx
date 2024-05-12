#pragma once
#include <vector>
#include <string>

const std::vector<std::string> alloc_funcs = {
    "malloc",        "calloc",           "kzalloc",
    "kmalloc",       "zalloc",           "vmalloc",
    "kcalloc",       "vzalloc",          "kzalloc_node",
    "kmalloc_array", "kmem_cache_alloc", "kmem_cache_alloc_node",
    "memdup",        "kmemdup",          "kstrdup"};

const std::vector<std::string> free_funcs = {"free", "kfree", "kzfree", "vfree",
                                             "kvfree"};

