#include "core/Instructions/CallInstruction.hpp"

#include "core/Function.hpp"
#include "core/Instruction.hpp"
#include "core/SFG/Converter.hpp"
#include "core/Value.hpp"
#include "llvm/IR/InstrTypes.h"

namespace framework {
std::shared_ptr<CallInst> CallInst::Create(llvm::CallInst* call_inst,
                                           std::vector<Fields> fields,
                                           long array_element_num) {
  auto created =
      Converter::GetInstance().createManagedInst<framework::CallInst>(
          call_inst, array_element_num, fields,
          [call_inst](std::shared_ptr<CallInst> created) {
            std::vector<std::shared_ptr<framework::Value>> arguments;
            for (auto& arg : call_inst->args())
              arguments.push_back(Value::CreateFromDefinition(arg));
            created->setArguments(arguments);
            if (created->CalledFunction())
              created->CalledFunction()->addCallerFunction(
                  framework::Function::createManagedFunction(
                      call_inst->getFunction()));
            return created;
          });
  return created;
}

std::shared_ptr<CallInst> CallInst::Create(std::shared_ptr<CallInst> call_inst,
                                           std::vector<Fields> fields,
                                           long array_element_num) {
  auto created =
      Converter::GetInstance().createManagedInst<framework::CallInst>(
          call_inst, array_element_num, fields,
          [call_inst](std::shared_ptr<CallInst> created) {
            created->setArguments(call_inst->Arguments());
            return created;
          });
  return created;
}

CallInst::CallInst(llvm::CallInst* call_inst)
    : Instruction(call_inst),
      called_function_(std::shared_ptr<framework::Function>()) {
  if (call_inst->getCalledFunction())
    called_function_ = framework::Function::createManagedFunction(
        call_inst->getCalledFunction());
}

CallInst::CallInst(llvm::CallInst* call_inst, std::vector<Fields> fields,
                   long array_element_num)
    : Instruction(call_inst, fields, array_element_num),
      called_function_(std::shared_ptr<framework::Function>()) {
  if (call_inst->getCalledFunction())
    called_function_ = framework::Function::createManagedFunction(
        call_inst->getCalledFunction());
}

CallInst::CallInst(std::shared_ptr<CallInst> instruction,
                   std::vector<Fields> fields, long array_element_num)
    : Instruction(instruction, fields, array_element_num),
      called_function_(instruction->CalledFunction()),
      arguments_(instruction->Arguments()) {}

void CallInst::setArguments(
    std::vector<std::shared_ptr<framework::Value>> arguments) {
  arguments_ = arguments;
}
}  // namespace framework
