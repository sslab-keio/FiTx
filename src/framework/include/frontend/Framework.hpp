#pragma once
#include <string>
#include <vector>

#include "State.hpp"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace framework {
class FrameworkPass : public llvm::ModulePass {
 public:
  static char ID;
  static std::vector<framework::FrameworkPass*> passes;

  FrameworkPass();
  virtual void getAnalysisUsage(llvm::AnalysisUsage& AU) const override;

  /*** Main Modular ***/
  bool runOnModule(llvm::Module& M) override;

  virtual void defineStates(){};
  void createTransitions(framework::StateManager& manager);

  void addStateManager(framework::StateManager manager) {
    manager_.push_back(manager);
  }

 private:
  std::vector<framework::StateManager> manager_;
};  // end of struct
}  // namespace framework
