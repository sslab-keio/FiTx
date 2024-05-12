#include "core/ValueTypeAlias.hpp"

#include <memory>
#include <vector>

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"

namespace framework {
Operands::Operands(std::vector<std::shared_ptr<framework::Value>> values) {
  std::transform(
      values.begin(), values.end(), std::back_inserter(values_),
      [](std::shared_ptr<framework::Value> value) {
        return std::make_shared<std::shared_ptr<framework::Value>>(value);
      });
}

Operands::Operands(
    std::vector<std::shared_ptr<std::shared_ptr<framework::Value>>> values)
    : values_(values) {}

void Operands::add(std::shared_ptr<std::shared_ptr<framework::Value>> value) {
  values_.push_back(value);
}

void Operands::add(std::shared_ptr<framework::Value> value) {
  values_.push_back(std::make_shared<std::shared_ptr<framework::Value>>(value));
}

std::shared_ptr<std::shared_ptr<framework::Value>> Operands::operator[](
    const int index) {
  return values_[index];
}

size_t Operands::size() const { return values_.size(); }

ValueTypeAlias::ValueTypeAlias()
    : aliased_value_(
          std::make_shared<std::map<framework::Instruction, Operands>>()) {}

ValueTypeAlias::ValueTypeAlias(const ValueTypeAlias& values) {
  aliased_value_ = values.aliased_value_;
  store_alias_ = values.store_alias_;
}

ValueTypeAlias& ValueTypeAlias::operator=(const ValueTypeAlias& values) {
  aliased_value_ = values.aliased_value_;
  store_alias_ = values.store_alias_;
  return *this;
}

void ValueTypeAlias::setValues(llvm::Instruction* instruction,
                               Operands framework_value) {
  auto framework_inst = framework::Instruction(instruction);
  setValues(framework_inst, framework_value);
}

Operands ValueTypeAlias::getValues(llvm::Instruction* instruction) {
  auto framework_inst = framework::Instruction(instruction);
  return getValues(framework_inst);
}

bool ValueTypeAlias::exists(llvm::Instruction* instruction) {
  auto framework_inst = framework::Instruction(instruction);
  return exists(framework_inst);
}

void ValueTypeAlias::setValues(framework::Instruction instruction,
                               Operands framework_value) {
  if (!exists(instruction)) {
    (*aliased_value_)[instruction] = framework_value;
  }
}

Operands ValueTypeAlias::getValues(framework::Instruction instruction) {
  return (*aliased_value_)[instruction];
}

bool ValueTypeAlias::exists(framework::Instruction instruction) {
  return aliased_value_->find(instruction) != aliased_value_->end();
}

Operands ValueTypeAlias::getAliasedValues(framework::Instruction instruction) {
  if (!InstructionAliasExists(instruction)) return Operands();
  auto aliased_inst = getAliasedStore(instruction);

  if (!exists(aliased_inst)) return Operands();

  return getValues(aliased_inst);
}

void ValueTypeAlias::setStoreAlias(llvm::StoreInst* store_inst,
                                   llvm::CallInst* call_inst) {
  auto framework_store_inst = framework::Instruction(store_inst);
  auto framework_call_inst = framework::Instruction(call_inst);
  store_alias_[framework_call_inst] = framework_store_inst;
}

framework::Instruction ValueTypeAlias::getAliasedStore(
    llvm::CallInst* call_inst) {
  auto framework_call_inst = framework::Instruction(call_inst);
  return store_alias_[framework_call_inst];
}

framework::Instruction ValueTypeAlias::getAliasedStore(
    framework::Instruction instruction) {
  return store_alias_[instruction];
}

bool ValueTypeAlias::InstructionAliasExists(llvm::CallInst* call_inst) {
  return store_alias_.find(call_inst) != store_alias_.end();
}

bool ValueTypeAlias::InstructionAliasExists(
    framework::Instruction instruction) {
  return store_alias_.find(instruction) != store_alias_.end();
}

}  // namespace framework
