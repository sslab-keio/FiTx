#pragma once
#include "core/BasicBlock.hpp"
#include "core/Value.hpp"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"

// include STL
#include <vector>

namespace framework {
std::vector<framework::Value::Fields> decodeGetElementPtrInst(
    llvm::GetElementPtrInst *get_element_ptr_inst);
long arrayElementNum(llvm::GetElementPtrInst *get_element_ptr_inst);

static std::vector<long> getValueIndices(llvm::GetElementPtrInst *inst);
static long getMemberIndiceFromByte(llvm::Instruction *instruction,
                                    llvm::StructType *STy, uint64_t byte);
static const llvm::StructLayout *getStructLayout(llvm::Instruction *instruction,
                                                 llvm::StructType *STy);

bool isInPredecessor(std::shared_ptr<framework::BasicBlock> target,
                     std::shared_ptr<framework::BasicBlock> block, int depth);
}  // namespace framework
