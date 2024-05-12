#include "core/AnalysisHelper.hpp"

#include "core/Utils.hpp"
#include "core/Value.hpp"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/ValueSymbolTable.h"

// include STL
#include <vector>

namespace framework {
std::vector<framework::Value::Fields> decodeGetElementPtrInst(
    llvm::GetElementPtrInst *get_element_ptr_inst) {
  llvm::Type *Ty = get_element_ptr_inst->getPointerOperandType();
  /* llvm::Type *Ty = get_element_ptr_inst->getSourceElementType(); */
  std::vector<long> indice = getValueIndices(get_element_ptr_inst);
  std::vector<framework::Value::Fields> decoded;

  for (long ind : indice) {
    long index = ind;
    if (Ty && index != Value::kNonFieldVariable) {
      if (Ty->isIntegerTy()) {
        if (Ty && getRootElementType(Ty)->isStructTy()) {
          index = getMemberIndiceFromByte(
              get_element_ptr_inst,
              llvm::cast<llvm::StructType>(getRootElementType(Ty)), index);
        }
      }
      decoded.push_back(framework::Value::Fields(Ty, index));
      if (auto StTy = llvm::dyn_cast<llvm::StructType>(getRootElementType(Ty)))
        Ty = StTy->getElementType(index);
      /* if (auto StTy = llvm::dyn_cast<llvm::StructType>(Ty)) */
      /*   Ty = StTy->getElementType(index); */
    } else {
      // decoded.push_back(std::pair<llvm::Type *, long>(NULL, ROOT_INDEX));
    }
  }

  return decoded;
}

long arrayElementNum(llvm::GetElementPtrInst *get_element_ptr_inst) {
  if (get_element_ptr_inst->getSourceElementType()->isArrayTy()) {
    if (auto const_int =
            llvm::dyn_cast<llvm::ConstantInt>(get_element_ptr_inst->idx_end()))
      return const_int->getSExtValue();
    return framework::Value::kArbitaryArrayElement;
  }

  if (auto const_int = llvm::dyn_cast<llvm::ConstantInt>(
          get_element_ptr_inst->idx_begin())) {
    if (const_int->isZero()) return framework::Value::kNonArrayElement;
    return const_int->getSExtValue();
  }
  return framework::Value::kArbitaryArrayElement;
}

std::vector<long> getValueIndices(llvm::GetElementPtrInst *inst) {
  std::vector<long> indices;

  llvm::Type *Ty = inst->getSourceElementType();
  auto idx_itr = inst->idx_begin();
  if (llvm::isa<llvm::ConstantInt>(idx_itr->get())) {
    if (!Ty->isIntegerTy()) idx_itr++;
  }

  for (; idx_itr != inst->idx_end(); idx_itr++) {
    long indice = Value::kNonFieldVariable;
    if (llvm::ConstantInt *cint =
            llvm::dyn_cast<llvm::ConstantInt>(idx_itr->get()))
      indice = cint->getSExtValue();
    indices.push_back(indice);
  }

  return indices;
}

long getMemberIndiceFromByte(llvm::Instruction *instruction,
                             llvm::StructType *STy, uint64_t byte) {
  const llvm::StructLayout *sl = getStructLayout(instruction, STy);
  if (sl != NULL) return sl->getElementContainingOffset(byte);
  return Value::kNonFieldVariable;
}

const llvm::StructLayout *getStructLayout(llvm::Instruction *instruction,
                                          llvm::StructType *STy) {
  auto llvm_function = instruction->getFunction();
  if (!llvm_function) return nullptr;

  auto llvm_module = llvm_function->getParent();
  if (!llvm_module) return nullptr;

  return llvm_module->getDataLayout().getStructLayout(STy);
}

bool isInPredecessor(std::shared_ptr<framework::BasicBlock> target,
                     std::shared_ptr<framework::BasicBlock> block, int depth) {
  if (depth < 0) return false;

  for (auto pred_ref : block->Predecessors()) {
    auto pred = pred_ref.lock();
    if (!pred)
      continue;

    if (pred == target || isInPredecessor(target, pred, depth - 1))
      return true;
  }

  return false;
}

}  // namespace framework
