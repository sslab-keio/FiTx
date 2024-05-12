#pragma once
#include "core/BasicBlock.hpp"
#include "core/Instruction.hpp"
#include "core/Value.hpp"
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
void generateWarning(llvm::Instruction* Inst, std::string war);
void generateWarning(llvm::Instruction* Inst, llvm::Value* val);
void generateWarning(llvm::Instruction* Inst, llvm::Type* type);
void generateWarning(std::string warning);

void generateError(llvm::Instruction* Inst, std::string warn);

std::string getDebugInfo(llvm::Instruction* inst);
std::string getFileName(llvm::Instruction* instruction);
int getLine(llvm::Instruction* instruction);
bool findFunctionName(const std::string& function_name,
                      const std::string& target_function);

llvm::Type* getRootElementType(llvm::Type* type);

void generateWarning(framework::Instruction* Inst, std::string warn);
void generateWarning(framework::Instruction* Inst, framework::Value* val);
void generateWarning(framework::Instruction* Inst, llvm::Type* type);
void generateWarning(framework::BasicBlock* basic_block, std::string warn);

void generateError(llvm::raw_ostream& stream, framework::Instruction* Inst,
                   std::string warn);
void generateError(llvm::raw_ostream& stream, framework::Instruction* Inst,
                   framework::Value* value);
std::string getDebugInfo(framework::Instruction* inst);

void generateLog(llvm::raw_ostream& stream, framework::Instruction* Inst,
                 std::string warn);

}  // namespace framework
