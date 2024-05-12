#pragma once
#include "core/Instruction.hpp"
#include "core/Value.hpp"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

namespace framework {
class StoreInst : public Instruction {
 public:
  static std::shared_ptr<StoreInst> Create(
      llvm::StoreInst* load_inst,
      std::vector<Fields> field = std::vector<Fields>(),
      long array_element_num = Value::kNonArrayElement);

  StoreInst(llvm::StoreInst* store_inst, std::vector<Fields> fields,
           long array_element_num);
  StoreInst(llvm::StoreInst* store_inst);

  std::shared_ptr<framework::Value> ValueOperand() const {return value_; };
  std::shared_ptr<framework::Value> PointerOperand() const {return pointer_; };

  void setValue(std::shared_ptr<framework::Value> value);
  void setPointer(std::shared_ptr<framework::Value> pointer);

  /// Methods for support type inquiry through isa, cast, and dyn_cast:
  static bool classof(const framework::Instruction* I) {
    return I->Opcode() == llvm::Instruction::Store;
  }

  static bool classof(const framework::Value* V) {
    return llvm::isa<framework::Instruction>(V) &&
           classof(llvm::cast<framework::Instruction>(V));
  }

 private:
  std::shared_ptr<framework::Value> value_;
  std::shared_ptr<framework::Value> pointer_;
};

}  // namespace framework
