#pragma once
#include "core/Casting.hpp"
#include "core/Value.hpp"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

namespace framework {
// TODO: Remove this if possible
class BasicBlock;
class Instruction : public Value {
 public:
  static std::shared_ptr<Instruction> Create(std::shared_ptr<Instruction> inst,
                                             std::vector<Fields> field,
                                             long array_element_num);

  Instruction(llvm::Instruction* instruction);
  Instruction();
  Instruction(unsigned int opcode, llvm::Instruction* instruction);

  Instruction(llvm::Instruction* instruction, std::vector<Fields> fields,
              long array_element_num);

  Instruction(std::shared_ptr<Instruction> instruction,
              std::vector<Fields> fields, long array_element_num);

  bool operator<=(const framework::Instruction& instruction) const;

  bool operator<(llvm::Instruction* instruction) const;
  bool operator<(const framework::Instruction& instruction) const;

  bool operator==(llvm::Instruction* instruction) const;
  bool operator==(const framework::Instruction& instruction) const;
  friend llvm::raw_ostream& operator<<(
      llvm::raw_ostream& ostream, const framework::Instruction& instruction);

  bool isInSameLine(framework::Instruction inst);
  bool emptyInstruction();

  const unsigned int Line() { return line_; };
  const unsigned int Column() { return column_; };
  const unsigned int Opcode() const { return opcode_; };
  const std::string OpcodeName() const {
    return llvm::Instruction::getOpcodeName(opcode_);
  }
  const llvm::Module* Module() const { return module_; }
  const std::weak_ptr<framework::BasicBlock> Parent() const { return parent_; }

  const llvm::DebugLoc& getDebugLoc() { return debug_loc_; };
  llvm::Instruction* LLVMInstruction() { return llvm_instruction_; };

  static bool classof(const framework::Value* value) {
    // Methods for support type inquiry through isa, cast, and dyn_cast:
    return value->getValueID() >= llvm::Value::InstructionVal;
  }

 private:
  llvm::Instruction* llvm_instruction_;
  llvm::Module* module_;
  std::weak_ptr<framework::BasicBlock> parent_;

  llvm::DebugLoc debug_loc_;

  unsigned int opcode_;
  unsigned int column_;
  unsigned int line_;
};
}  // namespace framework
