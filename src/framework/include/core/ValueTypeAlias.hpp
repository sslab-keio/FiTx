#pragma once
#include <vector>

#include "core/Instruction.hpp"
#include "core/Value.hpp"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"

namespace framework {

struct FunctionSigniture {
  llvm::Type* return_type;
  std::vector<llvm::Type*> argument_type;
};

class Operands {
 public:
  Operands() = default;
  Operands(std::vector<std::shared_ptr<framework::Value>> values);
  Operands(
      std::vector<std::shared_ptr<std::shared_ptr<framework::Value>>> values);

  std::shared_ptr<std::shared_ptr<framework::Value>> operator[](
      const int index);

  void add(std::shared_ptr<std::shared_ptr<framework::Value>> value);
  void add(std::shared_ptr<framework::Value> value);

  size_t size() const;

 private:
  std::vector<std::shared_ptr<std::shared_ptr<framework::Value>>> values_;
};

class ValueTypeAlias {
 public:
  ValueTypeAlias();

  ValueTypeAlias(const ValueTypeAlias&);
  ValueTypeAlias& operator=(const ValueTypeAlias&);

  void setValues(llvm::Instruction* instruction, Operands operand_values);
  Operands getValues(llvm::Instruction* value);
  bool exists(llvm::Instruction* value);

  void setValues(framework::Instruction instruction, Operands operand_values);
  Operands getValues(framework::Instruction value);
  bool exists(framework::Instruction value);

  Operands getAliasedValues(framework::Instruction value);
  void setStoreAlias(llvm::StoreInst* store_inst, llvm::CallInst* call_inst);

  framework::Instruction getAliasedStore(llvm::CallInst* call_inst);
  framework::Instruction getAliasedStore(framework::Instruction instruction);

  bool InstructionAliasExists(llvm::CallInst* call_inst);
  bool InstructionAliasExists(framework::Instruction instruction);
  std::shared_ptr<std::map<framework::Instruction, Operands>> AliasedValue() {
    return aliased_value_;
  };

 private:
  std::shared_ptr<std::map<framework::Instruction, Operands>> aliased_value_;
  std::map<framework::Instruction, framework::Instruction> store_alias_;
};
}  // namespace framework
