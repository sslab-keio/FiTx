#include "core/Instructions/LoadInstruction.hpp"

#include "core/Instruction.hpp"
#include "core/SFG/Converter.hpp"
#include "core/Value.hpp"
#include "llvm/IR/InstrTypes.h"

namespace framework {
std::shared_ptr<LoadInst> LoadInst::Create(llvm::LoadInst* load_inst,
                                           std::vector<Value::Fields> fields,
                                           long array_element_num) {
  auto created =
      Converter::GetInstance().createManagedInst<framework::LoadInst>(
          load_inst, array_element_num, fields,
          [load_inst](std::shared_ptr<LoadInst> created) {
            auto operand =
                Value::CreateFromDefinition(load_inst->getPointerOperand());
            created->setValue(operand);
            return created;
          });

  return created;
}

void LoadInst::setValue(std::shared_ptr<framework::Value> value) {
  load_value_ = value;
}

LoadInst::LoadInst(llvm::LoadInst* load_inst)
    : Instruction(load_inst),
      load_value_(framework::Value::CreateFromDefinition(
          load_inst->getPointerOperand())) {}

LoadInst::LoadInst(llvm::LoadInst* instruction, std::vector<Fields> fields,
                   long array_element_num)
    : Instruction(instruction, fields, array_element_num),
      pointer_type_(instruction->getType()) {}
}  // namespace framework
