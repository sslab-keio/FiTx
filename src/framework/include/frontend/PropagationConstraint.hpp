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
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "StateTransition.hpp"
#include "core/Instructions.hpp"
#include "core/Utils.hpp"
#include "core/Value.hpp"

namespace framework {
// An interface class to be used to restrict the state propagtion on
// certain situations. Should be overwritten by developers if necessary.
class StatefulConstraint {
 public:
  StatefulConstraint() = default;
  virtual bool shouldPropagateOnCallInst(
      std::shared_ptr<framework::CallInst> inst) {
    return true;
  }

  virtual void propagate() {}

};
};  // namespace framework
