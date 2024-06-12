#pragma once
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
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "PropagationConstraint.hpp"
#include "StateTransition.hpp"
#include "core/Utils.hpp"
#include "core/Value.hpp"

namespace framework {
enum StateType { INIT, NORMAL, BUG };
enum StateMergeMethod { STRICT, FLEX, CUSTOM };
enum BugNotificationTiming { IMMEDIATE, FUNCTION_END, END_OF_LIFE, MODULE_END };

enum TriggerConstraint { NONE = 0, NON_RETURN = 1, NON_ARG = 2 };

struct StateArgs {
  StateArgs(std::string name, StateType type = NORMAL,
            BugNotificationTiming timing = IMMEDIATE,
            bool early_notification = true, TriggerConstraint constraint = NONE,
            StateMergeMethod method = STRICT)
      : name_(name),
        type_(type),
        method_(method),
        early_notification_(early_notification),
        trigger_constraint_(constraint),
        timing_(timing){};
  std::string name_;
  StateType type_;
  StateMergeMethod method_;
  BugNotificationTiming timing_;
  bool early_notification_;
  TriggerConstraint trigger_constraint_;
};

class State {
 public:
  constexpr static int kStateMaxNum = 100;
  constexpr static int kBugStateIDBase = INT_MAX - kStateMaxNum;

  State() = default;
  State(int ID, std::string name, StateType type = NORMAL,
        StateMergeMethod method = STRICT, bool early_notification = true,
        TriggerConstraint = NONE, BugNotificationTiming timing = IMMEDIATE);
  State(const State& state);

  State(int ID, StateArgs& args);

  bool operator<(const State& state) const;
  bool operator<(const StateArgs& args) const;
  bool operator==(const State& state) const;
  bool operator!=(const State& state) const;
  bool operator==(const std::string& name) const;
  bool operator==(const StateArgs& args) const;
  State& operator=(const State& state);

  std::string Name() const { return name_; };

  bool isInitState() const { return type_ == StateType::INIT; }
  bool isBugState() const { return type_ == StateType::BUG; }
  bool EarlyNotification() const { return early_notification_; }

  TriggerConstraint getTriggerConstraint() const { return trigger_constraint_; }
  bool isTriggerConstraintSet(TriggerConstraint constraint);

  const framework::StateMergeMethod MergeMethod() { return method_; };

  const framework::BugNotificationTiming NotificationTiming() {
    return timing_;
  }

  void addTransition(State& state, TransitionRule& rule);

  const std::vector<std::pair<State, TransitionRule>>& Transitions() {
    return transitions_;
  }

 private:
  int ID_;
  std::string name_;
  StateType type_;
  StateMergeMethod method_;
  BugNotificationTiming timing_;
  TriggerConstraint trigger_constraint_;
  bool early_notification_;

  std::vector<std::pair<State, TransitionRule>> transitions_;
};

class NullState : public State {
 public:
  static NullState& GetInstance() {
    // Singleton
    static NullState* state = new NullState();
    return *state;
  }

 private:
  NullState() : State(INT_MIN, "Null"){};
};

class Transition {
 public:
  Transition(const State source, const State target);
  Transition(const Transition& transition);

  void addTransitionRule(std::weak_ptr<TransitionRule> arg);
  const State& Source() const { return source_; };
  const State& Target() const { return target_; };

  Transition& operator=(const Transition& transition);
  bool operator<(const Transition& transition) const;
  bool operator==(const Transition& transition) const;

  friend llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
                                       const framework::Transition& transition);

 private:
  /* int id_; */
  /* State& source_; */
  /* State& target_; */
  State source_;
  State target_;

  std::vector<std::weak_ptr<TransitionRule>> transition_args_;
};

class TransitionLogs {
 public:
  TransitionLogs();
  TransitionLogs(const TransitionLogs& logs);
  TransitionLogs(Transition transition,
                 std::shared_ptr<framework::Instruction> instruction);

  const State& CurrentState() const;
  const std::shared_ptr<framework::Instruction> CurrentInstruction() const {
    return transition_logs_.back().instruction;
  }

  void addTransition(framework::Transition& transition,
                     std::shared_ptr<framework::Instruction> instruction);

  void setWarned();

  bool operator==(const TransitionLogs& logs) const;
  TransitionLogs& operator=(const TransitionLogs& logs);

  void generateLog(llvm::raw_ostream& stream) const;
  void logicalTerminate(std::shared_ptr<framework::Instruction> instruction);
  void logicalTerminate();

  const State& LeastSignificantSource() const {
    return *least_significant_source_;
  };
  const State& MostSignificantTarget() const {
    return *most_significant_target_;
  };
  Transition ReducedTransition() const;
  
  bool containsNegativeLogs() const;

  bool isDummy() const { return transition_logs_.empty(); };

 private:
  struct Log {
    framework::Transition transition;
    std::shared_ptr<framework::Instruction> instruction;

    bool operator==(const framework::TransitionLogs::Log& log) const {
      return transition == log.transition && instruction == log.instruction;
    }
  };

  std::vector<Log> transition_logs_;
  std::optional<State> least_significant_source_;
  std::optional<State> most_significant_target_;
  bool warned_;
};

class StateTransitionManager {
 public:
  StateTransitionManager() = default;

  Transition createTransition(State& source, State& target);
  Transition createTransition(State& source, State& target,
                              TransitionRule rule);

  void addTransitionRule(Transition& transition, TransitionRule& rule);

  /* Function Arg Transition Rule*/
  bool existsInFunctionArgTransition(
      const FunctionArgTransitionRule::FunctionArg& arg);
  const std::pair<FunctionArgTransitionRule::FunctionArg,
                  std::vector<framework::Transition>>
  getFunctionArgTransitions(const std::string& name, unsigned int arg);

  /* Store Inst Transition Rule*/
  std::vector<framework::Transition> getStoreArgTransitions(
      framework::StoreValueTransitionRule::StoreValueType type,
      const std::string& name = std::string());

  std::vector<framework::Transition> getUseValueTransitions();

  std::vector<framework::Transition> getAliasTransitions();

 private:
  /* Register Function Arg Transition Rule*/
  void registerFunctionArgTransition(
      const FunctionArgTransitionRule::FunctionArg& arg,
      framework::Transition transition);
  void registerFunctionArgTransition(
      const std::vector<FunctionArgTransitionRule::FunctionArg>& arg,
      framework::Transition transition);

  void registerStoreTransition(std::shared_ptr<StoreValueTransitionRule> rule,
                               framework::Transition transition);

  void registerUseTransition(std::shared_ptr<UseValueTransitionRule> rule,
                             framework::Transition transition);

  void registerAliasTransition(std::shared_ptr<AliasValueTransitionRule> rule,
                               framework::Transition transition);

  std::map<FunctionArgTransitionRule::FunctionArg,
           std::vector<framework::Transition>>
      function_transitions_;

  /* Register Store Inst */
  std::map<std::string, std::vector<framework::Transition>>
      call_store_transitions_;

  std::map<framework::StoreValueTransitionRule::StoreValueType,
           std::vector<framework::Transition>>
      store_transitions_;

  std::vector<framework::Transition> use_transitions_;
  std::vector<framework::Transition> alias_transitions_;

  std::set<Transition> transitions_;

  std::map<framework::TransitionTrigger,
           std::vector<std::shared_ptr<TransitionRule>>>
      rules_;
};

class StateManager {
 public:
  StateManager();
  StateManager(std::set<State> states);

  // Factory
  static StateManager createWithInitStates(std::vector<StateArgs>& args);

  State& createState(StateArgs& args);
  std::set<State> createStates(std::vector<StateArgs>& states);

  void addTransition(State& source, State& target, TransitionRule& rule);

  State& getByName(const std::string& name);
  State getByID(int id);

  void set(State state);
  void set(State state, std::set<framework::Value>);

  void setEarlyStateTransition(bool early_transition);

  State& getInitState();
  const std::set<State>& getStates();
  const std::set<State> getBugStates();

  std::shared_ptr<framework::StateTransitionManager> TransitionManager();

  void enableStatefulConstraint(std::shared_ptr<StatefulConstraint> constraint);

  const std::shared_ptr<StatefulConstraint> getStatefulConstraint() {
    return propagation_constraint_;
  }

 private:
  std::set<State> states_;
  std::set<State> bug_states_;
  std::shared_ptr<framework::StateTransitionManager> transition_manager_;
  std::shared_ptr<framework::StatefulConstraint> propagation_constraint_;

  State* init_state_;

  bool early_state_transition_;
};

};  // namespace framework
