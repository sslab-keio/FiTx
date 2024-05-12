#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

// include STL
#include <algorithm>
#include <ctime>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

namespace framework {
enum TransitionTrigger { FUNCTION_ARG, STORE_VALUE, USE_VALUE, ALIASED_VALUE };

class TransitionRule {
 public:
  TransitionRule(TransitionTrigger trigger);
  TransitionRule(const TransitionRule& rule);
  TransitionTrigger Trigger();

 private:
  TransitionTrigger trigger_;
};

class FunctionArgTransitionRule : public TransitionRule {
 public:
  struct FunctionArg {
    FunctionArg(std::string name, unsigned int index = 0,
                bool consider_parent = false)
        : function_name(name),
          arg_index(index),
          consider_parent(consider_parent) {}
    std::string function_name;
    unsigned int arg_index;
    bool consider_parent;

    bool operator<(const struct FunctionArg& arg) const {
      if (function_name == arg.function_name) return arg_index < arg.arg_index;
      return function_name < arg.function_name;
    }
  };

  FunctionArgTransitionRule(TransitionRule* rule);

  FunctionArgTransitionRule(std::vector<FunctionArg> names);
  FunctionArgTransitionRule(std::vector<std::string> names);
  FunctionArgTransitionRule(std::string name);
  FunctionArgTransitionRule(const FunctionArgTransitionRule& rule);

  /* const std::vector<std::string>& FunctionNames() { return function_names_;
   * }; */
  const std::vector<FunctionArg>& FunctionArgs() { return function_args_; };

 private:
  std::vector<FunctionArg> function_args_;
  /* std::vector<std::string> function_names_; */
};

class StoreValueTransitionRule : public TransitionRule {
 public:
  static constexpr int kNullBranchOffset = 4;
  enum StoreValueType {
    NULL_VAL,
    NON_NULL_VAL,
    ANY,
    CALL_FUNC,
    // The latter Values are special instances of Branch Considered Store Insts
    // Developers should not used these fields unless they really know what they
    // are doing.
    // TODO: Set this to a different place, or make a new rule out of this
    NULL_BRANCH_CONSIDERED_NULL,
    NULL_BRANCH_CONSIDERED_NON_NULL,
    NULL_BRANCH_CONSIDERED_ANY
  };

  StoreValueTransitionRule(StoreValueType type, std::vector<std::string> funcs =
                                                    std::vector<std::string>());

  void setConsiderNullBranch(bool consider) {
    consider_null_branch_ = consider;
  }

  bool ConsiderNullBranch() { return consider_null_branch_; }

  const std::vector<std::string>& FunctionNames() { return function_names_; };
  StoreValueType Type() { return type_; };

 private:
  bool consider_null_branch_;
  const std::vector<std::string> function_names_;
  StoreValueType type_;
};

class UseValueTransitionRule : public TransitionRule {
 public:
  UseValueTransitionRule();
};

class AliasValueTransitionRule : public TransitionRule {
 public:
  AliasValueTransitionRule();
};

};  // namespace framework
