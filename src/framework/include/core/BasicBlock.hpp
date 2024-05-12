#pragma once
#include <memory>
#include <set>
#include <vector>

#include "core/Instruction.hpp"
#include "llvm/IR/BasicBlock.h"

namespace framework {
// TODO: remove this prototype from here
class Function;
class BranchInst;
class BasicBlock {
 public:
  constexpr static long kNoId = -1;
  BasicBlock(llvm::BasicBlock* basic_block);

  bool operator==(llvm::BasicBlock* basic_block);
  bool operator==(const framework::BasicBlock& basic_block);

  bool operator<(const framework::BasicBlock& basic_block);

  friend bool operator<(
      const std::weak_ptr<framework::BasicBlock> basic_block,
      const std::weak_ptr<framework::BasicBlock> target_block);

  friend bool operator==(
      const std::shared_ptr<framework::BasicBlock> basic_block,
      const llvm::BasicBlock* llvm_block);

  void addSuccessor(std::shared_ptr<framework::BasicBlock> block);
  void addPredecessor(std::shared_ptr<framework::BasicBlock> block);
  bool isInPredecessor(std::shared_ptr<framework::BasicBlock> block);

  const std::set<std::weak_ptr<framework::BasicBlock>>& Predecessors() {
    return predecessors_;
  }

  const std::set<std::weak_ptr<framework::BasicBlock>>& Successors() {
    return successors_;
  }

  void addInstruction(std::shared_ptr<framework::Instruction> inst);
  const std::vector<std::shared_ptr<framework::Instruction>> Instructions() {
    return instructions_;
  }

  llvm::BasicBlock* LLVMBasicBlock() { return llvm_basic_block_; }
  const std::string& Name() { return name_; }

  std::weak_ptr<framework::Function> Parent() { return parent_; };

  bool isCleanupBlock() { return !pass_through_.empty(); };
  bool collectPassthroughBlock();
  const std::vector<std::weak_ptr<framework::BasicBlock>> getPassthroughBlock(
      std::weak_ptr<framework::BasicBlock> succ_block);

  void addDeadValue(std::shared_ptr<framework::Value> value);
  const std::set<std::shared_ptr<framework::Value>> DeadValues() const;

  void setBranchInst(std::shared_ptr<framework::BranchInst> branch_inst) {
    branch_inst_ = branch_inst;
  }

  std::shared_ptr<framework::BranchInst> getBranchInst() {
    return branch_inst_;
  }

  unsigned int Line() {return line_; }

  void setId(long id) { id_ = id; }
  long Id() { return id_; }

 private:
  // BasicBlockInfo
  long id_;
  llvm::BasicBlock* llvm_basic_block_;
  std::string name_;
  unsigned int line_;

  // Contained Instructions
  std::vector<std::shared_ptr<framework::Instruction>> instructions_;
  std::weak_ptr<framework::Function> parent_;

  std::shared_ptr<framework::BranchInst> branch_inst_;

  bool is_cleanup_block_;

  std::map<std::weak_ptr<framework::BasicBlock>,
           std::vector<std::weak_ptr<framework::BasicBlock>>, std::owner_less<>>
      pass_through_;

  // Values to be dead by the end of this BB
  std::set<std::shared_ptr<framework::Value>> dead_values_;

  // Interactions
  std::set<std::weak_ptr<framework::BasicBlock>> predecessors_;
  std::set<std::weak_ptr<framework::BasicBlock>> successors_;
};
}  // namespace framework
