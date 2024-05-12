#include "core/Instructions/StoreInstruction.hpp"

#include "core/Instruction.hpp"
#include "core/SFG/Converter.hpp"
#include "core/Value.hpp"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"

namespace framework {
std::shared_ptr<StoreInst> StoreInst::Create(llvm::StoreInst* store_inst,
                                             std::vector<Value::Fields> fields,
                                             long array_element_num) {
  auto created =
      Converter::GetInstance().createManagedInst<framework::StoreInst>(
          store_inst, array_element_num, fields,
          [store_inst](std::shared_ptr<StoreInst> created) {
            auto value_operand =
                Value::CreateFromDefinition(store_inst->getValueOperand());
            value_operand->addUser(created);
            created->setValue(value_operand);

            auto pointer_operand =
                Value::CreateFromDefinition(store_inst->getPointerOperand());
            pointer_operand->addUser(created);
            created->setPointer(pointer_operand);

            return created;
          });
  return created;
}

StoreInst::StoreInst(llvm::StoreInst* store_inst, std::vector<Fields> fields,
                     long array_element_num)
    : Instruction(store_inst, fields, array_element_num) {}

StoreInst::StoreInst(llvm::StoreInst* store_inst) : Instruction(store_inst) {
  value_ = Value::CreateFromDefinition(store_inst->getValueOperand());
  pointer_ = Value::CreateFromDefinition(store_inst->getPointerOperand());
}

void StoreInst::setValue(std::shared_ptr<framework::Value> value) {
  value_ = value;
}

void StoreInst::setPointer(std::shared_ptr<framework::Value> pointer) {
  pointer_ = pointer;
}
}  // namespace framework
