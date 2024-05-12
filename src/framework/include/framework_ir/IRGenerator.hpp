#pragma once
#include "framework_ir/Analyzer.hpp"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

namespace ir_generator {
class IRGenerator : public llvm::FunctionPass {
 public:
  IRGenerator();

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const override;
  bool runOnFunction(llvm::Function &F) override;

  static char ID;
  static std::map<llvm::Module*, std::set<std::shared_ptr<framework::Function>>>
      framework_ir_;

 private:
  Analyzer analyzer;
};  // end of struct
}  // namespace ir_generator
