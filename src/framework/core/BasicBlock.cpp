#include "core/BasicBlock.hpp"

#include "core/Function.hpp"
#include "core/Instruction.hpp"
#include "core/Instructions.hpp"
#include "core/Utils.hpp"

namespace framework {
BasicBlock::BasicBlock(llvm::BasicBlock* basic_block)
    : llvm_basic_block_(basic_block), line_(0), id_(kNoId) {
  name_ = basic_block->getName();
  line_ = getLine(basic_block->getFirstNonPHIOrDbgOrLifetime());
  parent_ =
      framework::Function::createManagedFunction(basic_block->getParent());
}

bool BasicBlock::operator==(llvm::BasicBlock* basic_block) {
  return basic_block == llvm_basic_block_;
}

bool BasicBlock::operator==(const framework::BasicBlock& basic_block) {
  return basic_block.llvm_basic_block_ == llvm_basic_block_;
}

bool BasicBlock::operator<(const framework::BasicBlock& basic_block) {
  return llvm_basic_block_ < basic_block.llvm_basic_block_;
}

bool operator<(const std::weak_ptr<framework::BasicBlock> source_block,
               const std::weak_ptr<framework::BasicBlock> target_block) {
  auto source = source_block.lock();
  auto target = target_block.lock();

  return source->llvm_basic_block_ < target->llvm_basic_block_;
}

bool operator==(const std::shared_ptr<framework::BasicBlock> basic_block,
                const llvm::BasicBlock* llvm_block) {
  return basic_block->llvm_basic_block_ == llvm_block;
}

void BasicBlock::addSuccessor(std::shared_ptr<framework::BasicBlock> block) {
  successors_.insert(block);
}

void BasicBlock::addPredecessor(std::shared_ptr<framework::BasicBlock> block) {
  predecessors_.insert(block);
}

bool BasicBlock::isInPredecessor(std::shared_ptr<framework::BasicBlock> block) {
  return std::find_if(predecessors_.begin(), predecessors_.end(),
                      [block](std::weak_ptr<framework::BasicBlock> pred) {
                        return pred.lock() == block;
                      }) != predecessors_.end();
}

void BasicBlock::addInstruction(std::shared_ptr<framework::Instruction> inst) {
  instructions_.push_back(inst);
}

bool BasicBlock::collectPassthroughBlock() {
  llvm::Instruction* transition_inst = llvm_basic_block_->getTerminator();
  if (!transition_inst) return false;

  std::map<std::weak_ptr<framework::BasicBlock>,
           std::vector<std::weak_ptr<framework::BasicBlock>>, std::owner_less<>>
      succ_to_preds;

  if (auto branch_inst = llvm::dyn_cast<llvm::BranchInst>(transition_inst)) {
    if (!branch_inst->isConditional()) return false;

    if (auto icmp_inst =
            llvm::dyn_cast<llvm::ICmpInst>(branch_inst->getCondition())) {
      long match_block_index = 1;
      if (icmp_inst->getSignedPredicate() ==
          llvm::CmpInst::Predicate::ICMP_EQ) {
        match_block_index = 0;
      }

      auto compared_value =
          llvm::dyn_cast<llvm::ConstantInt>(icmp_inst->getOperand(1));
      if (!compared_value) return false;

      if (auto phi_node =
              llvm::dyn_cast<llvm::PHINode>(icmp_inst->getOperand(0))) {
        for (size_t i = 0; i < phi_node->getNumIncomingValues(); i++) {
          auto const_int =
              llvm::dyn_cast<llvm::ConstantInt>(phi_node->getIncomingValue(i));
          if (!const_int) return false;

          long block_idx = match_block_index;
          if (const_int->getSExtValue() != compared_value->getSExtValue())
            block_idx = branch_inst->getNumSuccessors() - match_block_index - 1;

          if (auto function = parent_.lock()) {
            auto pred_block =
                function->getBasicBlock(phi_node->getIncomingBlock(i));
            auto succ_block =
                function->getBasicBlock(branch_inst->getSuccessor(block_idx));

            if (succ_to_preds.find(succ_block) == succ_to_preds.end())
              succ_to_preds[succ_block] =
                  std::vector<std::weak_ptr<framework::BasicBlock>>();
            succ_to_preds[succ_block].push_back(pred_block);
          }
        }
        pass_through_ = succ_to_preds;
        return true;
      }
    }
  } else if (auto switch_inst =
                 llvm::dyn_cast<llvm::SwitchInst>(transition_inst)) {
    if (auto phi_node =
            llvm::dyn_cast<llvm::PHINode>(switch_inst->getCondition())) {
      for (size_t i = 0; i < phi_node->getNumIncomingValues(); i++) {
        auto const_int =
            llvm::dyn_cast<llvm::ConstantInt>(phi_node->getIncomingValue(i));
        if (!const_int) return false;

        auto case_value = switch_inst->findCaseValue(const_int);
        if (auto function = parent_.lock()) {
          auto succ_block =
              function->getBasicBlock(case_value->getCaseSuccessor());
          auto pred_block =
              function->getBasicBlock(phi_node->getIncomingBlock(i));
          if (succ_to_preds.find(succ_block) == succ_to_preds.end())
            succ_to_preds[succ_block] =
                std::vector<std::weak_ptr<framework::BasicBlock>>();

          succ_to_preds[succ_block].push_back(pred_block);
        }
      }
      pass_through_ = succ_to_preds;
      return true;
    } else if (auto load_inst = llvm::dyn_cast<llvm::LoadInst>(
                   switch_inst->getCondition())) {
      for (auto def : load_inst->getPointerOperand()->users()) {
        if (auto store_inst = llvm::dyn_cast<llvm::StoreInst>(def)) {
          if (store_inst->getPointerOperand() != load_inst->getPointerOperand())
            continue;

          if (std::find(llvm::pred_begin(llvm_basic_block_),
                        llvm::pred_end(llvm_basic_block_),
                        store_inst->getParent()) ==
              llvm::pred_end(llvm_basic_block_))
            continue;

          auto const_int =
              llvm::dyn_cast<llvm::ConstantInt>(store_inst->getValueOperand());
          if (!const_int) return false;

          auto case_value = switch_inst->findCaseValue(const_int);
          if (auto function = parent_.lock()) {
            auto succ_block =
                function->getBasicBlock(case_value->getCaseSuccessor());
            auto pred_block = function->getBasicBlock(store_inst->getParent());
            if (succ_to_preds.find(succ_block) == succ_to_preds.end())
              succ_to_preds[succ_block] =
                  std::vector<std::weak_ptr<framework::BasicBlock>>();

            succ_to_preds[succ_block].push_back(pred_block);
          }
        }
      }
      pass_through_ = succ_to_preds;
      return true;
    }
  }
  return false;
}

const std::vector<std::weak_ptr<framework::BasicBlock>>
BasicBlock::getPassthroughBlock(
    std::weak_ptr<framework::BasicBlock> succ_block) {
  auto prev_block = pass_through_.find(succ_block);
  if (prev_block != pass_through_.end()) return prev_block->second;
  return std::vector<std::weak_ptr<framework::BasicBlock>>();
}

void BasicBlock::addDeadValue(std::shared_ptr<framework::Value> value) {
  dead_values_.insert(value);
}

const std::set<std::shared_ptr<framework::Value>> BasicBlock::DeadValues()
    const {
  return dead_values_;
}
}  // namespace framework
