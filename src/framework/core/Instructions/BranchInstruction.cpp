#include "core/Instructions/BranchInstruction.hpp"

#include <llvm/IR/Instruction.h>

#include "core/BasicBlock.hpp"
#include "core/Casting.hpp"
#include "core/Function.hpp"
#include "core/Instruction.hpp"
#include "core/Instructions.hpp"
#include "core/Utils.hpp"
#include "core/Value.hpp"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"

namespace framework {
std::shared_ptr<CompareInst> CompareInst::Create(
    llvm::ICmpInst* icmp_inst, std::vector<Value::Fields> fields,
    long array_element_num) {
  auto created =
      Converter::GetInstance().createManagedInst<framework::CompareInst>(
          icmp_inst, array_element_num, fields,
          [icmp_inst](std::shared_ptr<CompareInst> created) {
            std::vector<std::shared_ptr<framework::Value>> operands;
            for (auto& operand : icmp_inst->operands()) {
              auto created_operand =
                  framework::Value::CreateFromDefinition(operand);
              created_operand->addUser(created);
              operands.push_back(created_operand);
            }
            created->setOperands(operands);
            return created;
          });
  return created;
}

void CompareInst::setOperands(
    std::vector<std::shared_ptr<framework::Value>> operand) {
  operands_ = operand;
}

void CompareInst::replaceOperand(
    std::shared_ptr<framework::Value> operand,
    std::shared_ptr<framework::Value> new_operand) {
  std::replace(operands_.begin(), operands_.end(), operand, new_operand);
  replaced_.push_back(operand);
}

std::shared_ptr<framework::Value> CompareInst::find_if(
    std::function<bool(std::shared_ptr<framework::Value>)> lambda) {
  auto found = std::find_if(operands_.begin(), operands_.end(), lambda);
  if (found != operands_.end()) return *found;
  return nullptr;
}

bool CompareInst::operandExists(std::shared_ptr<framework::Value> value) {
  return std::find(operands_.begin(), operands_.end(), value) !=
         operands_.end();
}

bool CompareInst::operandExists(
    std::shared_ptr<framework::CallInst> call_inst) {
  return std::find_if(operands_.begin(), operands_.end(),
                      [call_inst](std::shared_ptr<framework::Value> operand) {
                        if (auto call_operand =
                                shared_dyn_cast<framework::CallInst>(operand)) {
                          return call_inst->LLVMInstruction() ==
                                 call_operand->LLVMInstruction();
                        }
                        return call_inst == operand;
                      }) != operands_.end();
}

bool CompareInst::returnValueOperandExists() {
  return std::find_if(operands_.begin(), operands_.end(),
                      [](auto& operand) { return operand->isReturnValue(); }) !=
             operands_.end() ||
         std::find_if(replaced_.begin(), replaced_.end(), [](auto& operand) {
           return operand->isReturnValue();
         }) != replaced_.end();
}

CompareInst::CompareInst(llvm::ICmpInst* icmp_inst, std::vector<Fields> fields,
                         long array_element_num)
    : Instruction(icmp_inst, fields, array_element_num) {
  predicate_ = icmp_inst->getPredicate();
}

std::shared_ptr<BranchInst> BranchInst::Create(
    llvm::SwitchInst* switch_inst, std::vector<Value::Fields> fields,
    long array_element_num) {
  auto created =
      Converter::GetInstance().createManagedInst<framework::BranchInst>(
          switch_inst, array_element_num, fields, [switch_inst](auto created) {
            auto condition = switch_inst->getCondition();
            if (auto icmp_inst = llvm::dyn_cast<llvm::ICmpInst>(condition))
              created->setCondition(framework::CompareInst::Create(icmp_inst));
            else if (auto call_inst = llvm::dyn_cast<llvm::CallInst>(condition))
              created->setCondition(framework::CallInst::Create(call_inst));

            auto function = framework::Function::createManagedFunction(
                switch_inst->getFunction());
            for (auto branch_case : switch_inst->cases()) {
              created->setTransitionNode(
                  branch_case.getCaseValue()->getSExtValue(),
                  function->getBasicBlock(branch_case.getCaseSuccessor()));
            }
            created->setTransitionNode(
                BranchInst::kTrueTransition,
                function->getBasicBlock(switch_inst->getDefaultDest()));

            return created;
          });
  return created;
}

std::shared_ptr<BranchInst> BranchInst::Create(
    llvm::BranchInst* branch_inst, std::vector<Value::Fields> fields,
    long array_element_num) {
  auto created =
      Converter::GetInstance().createManagedInst<framework::BranchInst>(
          branch_inst, array_element_num, fields, [branch_inst](auto created) {
            if (branch_inst->isUnconditional()) return created;

            auto condition = branch_inst->getCondition();
            if (auto icmp_inst = llvm::dyn_cast<llvm::ICmpInst>(condition)) {
              std::shared_ptr<framework::CompareInst> compare_inst =
                  framework::CompareInst::Create(icmp_inst);

              /* std::shared_ptr<framework::Value> expected_calls = */
              /*     compare_inst->find_if( */
              /*         [](const std::shared_ptr<framework::Value>& value) { */
              /*           auto call_inst = */
              /*               shared_dyn_cast<framework::CallInst>(value); */
              /*           return call_inst && call_inst->CalledFunction() && */
              /*                  framework::Function::IsExpectFunction( */
              /*                      call_inst->CalledFunction()); */
              /*         }); */

              llvm::Value* val = nullptr;
              for (auto operand : compare_inst->Operands()) {
                auto call_inst = shared_dyn_cast<framework::CallInst>(operand);
                if (call_inst && call_inst->CalledFunction() &&
                    framework::Function::IsExpectFunction(
                        call_inst->CalledFunction())) {
                  val = &call_inst->Arguments().front()->getLLVMValue_();
                  break;
                }
              }

              while (val && !llvm::isa<llvm::ICmpInst>(val)) {
                if (auto new_val = llvm::dyn_cast<llvm::UnaryInstruction>(val))
                  val = new_val->getOperand(0);
                else if (auto new_val =
                             llvm::dyn_cast<llvm::BinaryOperator>(val))
                  val = new_val->getOperand(0);
                else
                  val = nullptr;
              }

              if (val)
                compare_inst = framework::CompareInst::Create(
                    llvm::cast<llvm::ICmpInst>(val));

              created->setCondition(compare_inst);
            } else if (auto call_inst =
                           llvm::dyn_cast<llvm::CallInst>(condition))
              created->setCondition(framework::CallInst::Create(call_inst));

            auto function = framework::Function::createManagedFunction(
                branch_inst->getFunction());
            created->setTransitionNode(
                BranchInst::kTrueTransition,
                function->getBasicBlock(branch_inst->getSuccessor(0)));

            created->setTransitionNode(
                BranchInst::kFalseTransition,
                function->getBasicBlock(branch_inst->getSuccessor(1)));

            return created;
          });
  return created;
}

BranchInst::BranchInst(llvm::BranchInst* branch_inst,
                       std::vector<Fields> fields, long array_element_num)
    : Instruction(branch_inst, fields, array_element_num),
      condition_instruction_(std::shared_ptr<framework::Instruction>()) {}

BranchInst::BranchInst(llvm::SwitchInst* switch_inst,
                       std::vector<Fields> fields, long array_element_num)
    : Instruction(switch_inst, fields, array_element_num),
      condition_instruction_(std::shared_ptr<framework::Instruction>()) {}

void BranchInst::setTransitionNode(
    int64_t code, std::shared_ptr<framework::BasicBlock> block) {
  if (nodes_.find(code) == nodes_.end()) nodes_[code] = TransitionNodes();
  nodes_[code].push_back(block);
}

bool BranchInst::isInOperand(std::shared_ptr<framework::Value> value) {
  if (!condition_instruction_) return false;

  if (auto inst = framework::shared_dyn_cast<framework::CompareInst>(
          condition_instruction_))
    return inst->operandExists(value);

  return value == condition_instruction_;
}

bool BranchInst::returnValueOperandExists() {
  if (!condition_instruction_) return false;

  if (auto inst = framework::shared_dyn_cast<framework::CompareInst>(
          condition_instruction_))
    return inst->returnValueOperandExists();

  return condition_instruction_->isReturnValue();
}
}  // namespace framework
