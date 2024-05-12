#pragma once
#include "Instruction.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Pass.h"
#include "llvm/PassAnalysisSupport.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

// include STL
#include <algorithm>
#include <ctime>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "State.hpp"
#include "Value.hpp"
#include "core/BasicBlock.hpp"
#include "core/Function.hpp"
#include "core/Instructions.hpp"

namespace framework {

class ArgTransitions {
 public:
  ArgTransitions();
  ArgTransitions(std::set<framework::State> states);
  ArgTransitions(const ArgTransitions& arg_transitions);

  bool operator==(const ArgTransitions& arg_transitions) const;

  void addArgTransitions(const ArgTransitions& arg_transitions);
  bool addTransition(std::vector<Transition>& transitions,
                     std::shared_ptr<framework::Instruction> inst);
  TransitionLogs getTransitionLog(State state);
  std::map<framework::State, TransitionLogs>& TransitionPerState() {
    return transition_per_state_;
  }

 private:
  std::map<framework::State, TransitionLogs> transition_per_state_;
};

class ArgValueStates {
 public:
  ArgValueStates();
  /* ArgValueStates(uint64_t arg_num); */
  ArgValueStates(uint64_t arg_num, const std::set<State>& states);
  ArgValueStates(const ArgValueStates& arg_value_states);

  bool operator==(const ArgValueStates& states);
  ArgValueStates& operator=(const ArgValueStates& arg_value_states);

  bool transitionState(std::vector<Transition>& transitions,
                       std::shared_ptr<framework::Value> value,
                       std::shared_ptr<framework::Instruction> instruction);

  void addArgValueState(const ArgValueStates& states);
  const uint64_t Size() const { return value_states_.size(); }
  bool ValueExistsInArg(uint64_t arg, std::shared_ptr<Value>);

  const std::map<std::shared_ptr<framework::Value>, std::vector<Transition>>
  getValueStateForArg(int64_t index) const;

  /* const std::map<std::shared_ptr<framework::Value>,
   * std::vector<TransitionLogs>> */
  /* getValueTransitionLogsForArg(int64_t index) const; */

  const std::map<std::shared_ptr<framework::Value>, ArgTransitions>
  getArgTransitions(int64_t index) const;

  std::vector<std::pair<std::shared_ptr<framework::Value>, TransitionLogs*>>
  getValueTransitionStates(const framework::State& state);

  void print();

 private:
  /* std::vector< */
  /*     std::map<std::shared_ptr<framework::Value>,
   * std::vector<TransitionLogs>>> */
  /*     value_states_; */

  std::vector<std::map<std::shared_ptr<framework::Value>, ArgTransitions>>
      value_states_;

  const std::set<State> states_;
};

class BasicBlockValueStates {
 public:
  BasicBlockValueStates() = default;
  BasicBlockValueStates(const BasicBlockValueStates& states);
  bool operator==(const BasicBlockValueStates& states);

  bool valueExists(std::shared_ptr<framework::Value> value);
  void updateReturnValue(std::shared_ptr<framework::BasicBlock> block);
  bool transitionState(std::vector<Transition>& transitions,
                       std::shared_ptr<framework::Value> value,
                       std::shared_ptr<framework::Instruction> instruction);

  void setValueState(std::shared_ptr<framework::Value> value,
                     framework::Transition& states,
                     std::shared_ptr<framework::Instruction> instruction);

  void setValueState(std::shared_ptr<framework::Value> value,
                     framework::TransitionLogs& logs);

  TransitionLogs& getTransitionLog(std::shared_ptr<framework::Value> value);

  std::vector<std::shared_ptr<framework::Value>> getStateValues(
      const framework::State& state);

  std::vector<std::pair<std::shared_ptr<framework::Value>, TransitionLogs*>>
  getValueTransitionStates(const framework::State& state);

  const State& getState(std::shared_ptr<framework::Value> value) {
    return value_states_[value].CurrentState();
  };

  const std::map<std::shared_ptr<framework::Value>, TransitionLogs>
  ValueStates() {
    return value_states_;
  };

  void print();

 private:
  std::map<std::shared_ptr<framework::Value>, TransitionLogs> value_states_;
};

class BasicBlockInformation {
 public:
  constexpr static int kMaxTimeToLive = 5;

  enum BlockStatus {
    NONE,
    ERROR,
    SUCCESS,
    NUTRAL
  };

  BasicBlockInformation(std::shared_ptr<framework::BasicBlock> basic_block,
                        const std::set<State>& states);
  BasicBlockInformation(const BasicBlockInformation& info);

  bool changeValueState(std::vector<Transition>& transitions,
                        std::shared_ptr<framework::Value> value,
                        std::shared_ptr<framework::Instruction> instruction);
  bool valueHasState(std::shared_ptr<framework::Value> value);
  void removeValueFromState(
      std::shared_ptr<framework::Value> value,
      std::shared_ptr<framework::Instruction> instruction);

  void resetValueState(std::shared_ptr<framework::Value> value,
                       std::shared_ptr<framework::Instruction> instruction);

  framework::BasicBlockValueStates& ValueStates() { return value_states_; };

  std::pair<framework::BasicBlockValueStates, framework::ArgValueStates>
  ValueStatesForSuccessor(std::shared_ptr<framework::BasicBlock> successor);

  std::set<std::shared_ptr<framework::Value>> ReturnCodeForSuccessor(
      std::shared_ptr<framework::BasicBlock> successor);

  framework::ArgValueStates& getArgValueStates() { return arg_value_states_; };
  void setPendingValueStates(std::weak_ptr<framework::BasicBlock>,
                             framework::ArgValueStates arg_value_state);

  std::vector<std::pair<std::shared_ptr<framework::Value>, TransitionLogs*>>
  getValueTransitionStates(const State& state);

  void setPendingReturnValues(std::weak_ptr<framework::BasicBlock>,
                              std::shared_ptr<framework::ConstValue>);

  bool operator==(const framework::BasicBlockInformation& prev_block_info);

  // TODO: TTL should be removed once worklist alogrithm is implemented
  // correctly
  void setTimeToLive(int ttl) { time_to_live_ = ttl; }
  int TimeToLive() { return time_to_live_; }

  const std::set<std::shared_ptr<framework::Value>>& ReturnValues() {
    return return_values_;
  }

  void addReturnValues(
      const std::set<std::shared_ptr<framework::Value>>& return_values);
  void addReturnValue(std::shared_ptr<framework::Value> value);

  bool ReturnValueSatisfiable(long value);

  void removeReturnvalue(int value);

  std::shared_ptr<framework::BasicBlock> BasicBlock() { return basic_block_; };

  void setPartialStates(bool partial_states);
  bool PartialStates() { return is_partial_states_; };

  void setPredecessorPartial(bool partial_states) {
    predecessor_partial_ = partial_states;
  };

  bool PredecessorPartial() { return predecessor_partial_; };

  bool IsInSamelinePredecessor(std::shared_ptr<framework::BasicBlock> block);
  void addSameLinePredecessor(std::shared_ptr<framework::BasicBlock> block);

  void addSameLinePredecessor(
      std::vector<std::weak_ptr<framework::BasicBlock>> blocks);

  std::vector<std::weak_ptr<framework::BasicBlock>> SameLinePredecessors() {
    return same_line_predecessors_;
  }

  AliasValues& getAliasValues() { return alias_info_; }

  void readCurrentStates();

  void setBlockStatus(BlockStatus status) {
    if (status_ != NUTRAL)
      status_ = status;
  }
  BlockStatus getBlockStatus() { return status_; }

 private:
  std::shared_ptr<framework::BasicBlock> basic_block_;
  struct PendingValues {
    framework::ArgValueStates arg_states;
    std::set<std::shared_ptr<framework::ConstValue>> return_values;
  };

  struct ValueStates {
    framework::ArgValueStates arg_value_states_;
    framework::BasicBlockValueStates value_states_;
  };

  // TODO: remove for quasi worklist algorithm
  int time_to_live_ = kMaxTimeToLive;
  bool is_partial_states_;
  bool predecessor_partial_;

  std::vector<std::weak_ptr<framework::BasicBlock>> same_line_predecessors_;

  std::set<std::shared_ptr<framework::Value>> return_values_;

  AliasValues alias_info_;

  std::map<std::weak_ptr<framework::BasicBlock>, struct PendingValues,
           std::owner_less<>>
      pending_values_;

  framework::ArgValueStates arg_value_states_;
  framework::BasicBlockValueStates value_states_;

  std::set<State> states_;

  struct ValueStates refcounted_states_;
  BlockStatus status_;
};

}  // namespace framework
