#include "core/Instruction.hpp"
#include "core/Value.hpp"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

namespace framework {
class LoadInst : public Instruction {
 public:
  static std::shared_ptr<LoadInst> Create(
      llvm::LoadInst* load_inst,
      std::vector<Fields> field = std::vector<Fields>(),
      long array_element_num = Value::kNonArrayElement);

  LoadInst(llvm::LoadInst* load_inst);
  LoadInst(llvm::LoadInst* instruction, std::vector<Fields> fields,
           long array_element_num);

  std::shared_ptr<framework::Value> LoadValue() {return load_value_; }
  llvm::Type* PointerType() {return pointer_type_; }

  void setValue(std::shared_ptr<framework::Value>);

  /// Methods for support type inquiry through isa, cast, and dyn_cast:
  static bool classof(const framework::Instruction* I) {
    return I->Opcode() == llvm::Instruction::Load;
  }

  static bool classof(const framework::Value* V) {
    return llvm::isa<framework::Instruction>(V) &&
           classof(llvm::cast<framework::Instruction>(V));
  }

 private:
  std::shared_ptr<framework::Value> load_value_;
  llvm::Type* pointer_type_;
};
}  // namespace framework
