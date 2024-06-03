#include "BasicBlock.hpp"
#include "Instruction.hpp"
#include "Instructions/BranchInstruction.hpp"
#include "StateTransition.hpp"
#include "Value.hpp"
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
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <ctime>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

#include "core/Casting.hpp"
#include "core/Instructions.hpp"
#include "core/Utils.hpp"
#include "framework_ir/IRGenerator.hpp"
#include "frontend/Analyzer.hpp"
#include "frontend/CommandlineArgs.hpp"
#include "frontend/Function.hpp"
#include "frontend/PropagationConstraint.hpp"

namespace framework {
Analyzer::Analyzer(llvm::Module &llvm_module,
                   framework::StateManager &state_manager,
                   framework::LoggingClient &client)
    : llvm_module_(llvm_module), state_manager_(state_manager), log_(client) {}

void Analyzer::analyze() {
  auto &framework_ir = ir_generator::IRGenerator::framework_ir_;
  if (framework_ir.find(&llvm_module_) == framework_ir.end()) return;
  for (auto function : framework_ir[&llvm_module_]) {
    analyzeFunction(function);
  }
  log_.flush();
}

void Analyzer::analyzeFunction(std::shared_ptr<framework::Function> function) {
  // Add new FunctionInformation Class
  if (!functionInformationExists(function))
    function_info_[function] =
        std::make_shared<framework::FunctionInformation>(function);

  auto func_info = function_info_[function];
  if (func_info->Stat() != FunctionInformation::UNANALYZED) return;
  func_info->setAnalysisStat(
      framework::FunctionInformation::AnalysisStat::IN_PROGRESS);

  analyzing_function_.push(function);
  /* auto target_blocks = function->BasOrderedicBlocks(); */
  auto target_blocks = function->OrderedBasicBlocks();

  std::queue<std::shared_ptr<framework::BasicBlock>> block_queue(
      std::deque(target_blocks.begin(), target_blocks.end()));

  while (!block_queue.empty()) {
    auto block = block_queue.front();
    bb_info_ =
        func_info->createBasicBlockInfo(block, state_manager_.getStates());
    func_info->setAnayzingBasicBlock(block);
    analyzePrevBlockBranch(block);
    for (auto inst : block->Instructions()) {
      switch (inst->Opcode()) {
        case llvm::Instruction::Call:
          analyzeCallInst(inst);
          break;
        case llvm::Instruction::Store:
          analyzeStoreInst(inst);
          break;
        case llvm::Instruction::Load:
          analyzeLoadInst(inst);
          break;
        default:
          break;
      }
    }
    block_queue.pop();

    if (func_info->basicBlockInfoChanged(block)) {
      block_queue.push(block);
      continue;
    }

    generateError(BugNotificationTiming::IMMEDIATE);
    generateError(BugNotificationTiming::END_OF_LIFE, block->DeadValues());
  }

  analyzeReturnValue(function);

  bb_info_ = func_info->getBasicBlockInformation(function->ReturnBlock());
  if (bb_info_) {
    generateError(BugNotificationTiming::FUNCTION_END);

    if (function->CallerFunctions().empty())
      generateError(BugNotificationTiming::MODULE_END);
  }

  analyzing_function_.pop();
  func_info->setAnalysisStat(
      framework::FunctionInformation::AnalysisStat::ANALYZED);
}

void Analyzer::analyzePrevBlockBranch(
    std::shared_ptr<framework::BasicBlock> block) {
  for (auto pred_reference : block->Predecessors()) {
    auto preds = pred_reference.lock();
    if (!preds ||
        !currentFunctionInformation()->getBasicBlockInformation(preds))
      continue;
    /* auto passthrough_blocks = */
    /*     std::vector<std::shared_ptr<framework::BasicBlock>>(); */

    /* auto weak_blocks = preds->getPassthroughBlock(block); */
    /* std::transform(weak_blocks.begin(), weak_blocks.end(), */
    /*                std::back_inserter(passthrough_blocks), */
    /*                [](const std::weak_ptr<framework::BasicBlock> block) {
     */
    /*                  return block.lock(); */
    /*                }); */
    /* if (!passthrough_blocks.size()) passthrough_blocks.push_back(preds); */

    /* for (auto passed_block : passthrough_blocks) { */
    auto branch_inst = preds->getBranchInst();
    if (!branch_inst || !branch_inst->Condition()) continue;

    auto condition_inst =
        shared_dyn_cast<framework::CompareInst>(branch_inst->Condition());
    if (!condition_inst) continue;

    bool null_exists = false;
    std::shared_ptr<framework::Value> comp_value = nullptr;
    for (auto operand : condition_inst->Operands()) {
      if (shared_isa<framework::NullValue>(operand)) {
        null_exists = true;
        continue;
      }
      if (shared_isa<framework::ConstValue>(operand)) continue;
      comp_value = operand;
    }
    if (!comp_value) continue;

    auto call_inst = shared_dyn_cast<CallInst>(comp_value);
    if (call_inst && call_inst->CalledFunction() &&
        Function::IsExpectFunction(call_inst->CalledFunction())) {
      for (auto args : call_inst->Arguments())
        generateWarning(call_inst.get(), args.get());
    }

    if (!null_exists) continue;
    BranchInst::TransitionNodes null_nodes =
        condition_inst->GetPredicate() == llvm::CmpInst::ICMP_EQ
            ? branch_inst->TruePathNodes()
            : branch_inst->FalsePathNodes();
    StoreValueTransitionRule::StoreValueType type =
        std::find_if(null_nodes.begin(), null_nodes.end(),
                     [&block](auto node) { return node.lock() == block; }) !=
                null_nodes.end()
            ? StoreValueTransitionRule::NULL_BRANCH_CONSIDERED_NULL
            : StoreValueTransitionRule::NULL_BRANCH_CONSIDERED_NON_NULL;

    // Add semantically correct transitions
    auto possible_transitions =
        state_manager_.TransitionManager()->getStoreArgTransitions(type);

    auto transitions =
        state_manager_.TransitionManager()->getStoreArgTransitions(
            framework::StoreValueTransitionRule::NULL_BRANCH_CONSIDERED_ANY);

    transitions.insert(transitions.end(), possible_transitions.begin(),
                       possible_transitions.end());

    std::set<std::shared_ptr<framework::Value>> related_values =
        currentFunctionInformation()->GetValueCollection().getRelatedValues(
            comp_value);
    related_values.insert(comp_value);

    if (auto aliased = currentFunctionInformation()
                           ->getBasicBlockInformation(preds)
                           ->getAliasValues()
                           .getAliasInfo(comp_value)) {
      related_values.insert(aliased->Values().begin(), aliased->Values().end());
    }

    generateWarning(branch_inst.get(), "Branch Inst Transition");
    for (auto value : related_values) {
      changeValueState(transitions, value, branch_inst);
    }
    generateWarning(branch_inst.get(), "Branch Inst Transition Done");

    // Additionally remove corresponding ret val for semantic correspondness
    int remove_ret_val =
        type == framework::StoreValueTransitionRule::NULL_VAL ? 0 : -1;
    bb_info_->removeReturnvalue(remove_ret_val);
    /* } */
  }
}

void Analyzer::analyzeCallInst(std::shared_ptr<framework::Instruction> I) {
  auto call_inst = std::static_pointer_cast<framework::CallInst>(I);
  /* if (state_manager_.getStatefulConstraint() && */
  /*     !state_manager_.getStatefulConstraint()->shouldPropagateOnCallInst( */
  /*         call_inst)) { */
  /*   return; */
  /* } */

  if (analyzeFunctionCall(call_inst)) return;

  auto function = call_inst->CalledFunction();
  if (!function) {
    // This is an indirect call. Should deal like its being outed
    for (auto value : call_inst->Arguments()) {
      std::set<std::shared_ptr<framework::Value>> related_values =
          currentFunctionInformation()->GetValueCollection().getRelatedValues(
              value);
      related_values.insert(value);
      for (auto &related : related_values) {
        bb_info_->removeValueFromState(related, I);
      }
    }
    return;
  }

  if (framework::Function::IsDebugDeclareFunction(function)) {
    bb_info_->resetValueState(call_inst->Arguments()[0], I);
    return;
  }

  if (function->isDebugFunction()) return;

  // Special Case where memset is called. This is semantically the same as
  // storing something to the target value, so we collect for such info..
  if (Function::IsMemSetFunction(function) && !call_inst->Arguments().empty()) {
    std::vector<framework::Transition> transitions;
    /* auto target_value = */
    /*     shared_dyn_cast<framework::ConstValue>(call_inst->Arguments()[1]); */
    /* if (target_value && target_value->getConstValue() == 0) { */
    /*   auto possible_transitions = */
    /*       state_manager_.TransitionManager()->getStoreArgTransitions( */
    /*           framework::StoreValueTransitionRule::NULL_VAL); */

    /*   transitions.insert(transitions.end(), possible_transitions.begin(), */
    /*                      possible_transitions.end()); */
    /* } */

    auto possible_transitions =
        state_manager_.TransitionManager()->getStoreArgTransitions(
            framework::StoreValueTransitionRule::ANY);

    transitions.insert(transitions.end(), possible_transitions.begin(),
                       possible_transitions.end());

    std::set<std::shared_ptr<framework::Value>> related_values =
        currentFunctionInformation()->GetValueCollection().getRelatedValues(
            call_inst->Arguments()[0]);
    related_values.insert(call_inst->Arguments()[0]);

    for (auto value : related_values) {
      changeValueState(transitions, value, I);
    }
    return;
  }

  if (function->isDeclaration()) {
    /* for (auto value : call_inst->Arguments()) { */
    /*   bb_info_->removeValueFromState(value, I); */
    /* } */
    for (auto value : call_inst->Arguments()) {
      std::set<std::shared_ptr<framework::Value>> related_values =
          currentFunctionInformation()->GetValueCollection().getRelatedValues(
              value);
      related_values.insert(value);
      for (auto &related : related_values) {
        bb_info_->removeValueFromState(related, I);
      }
    }
  } else {
    if (state_manager_.getStatefulConstraint() &&
        !state_manager_.getStatefulConstraint()->shouldPropagateOnCallInst(
            call_inst)) {
      return;
    }
    analyzeFunction(function);
    bb_info_ = function_info_[analyzing_function_.top()]
                   ->getCurrentBasicBlockInformation();
    copyFunctionValues(function, call_inst);
  }
}

void Analyzer::analyzeStoreInst(std::shared_ptr<framework::Instruction> I) {
  auto store_inst = std::static_pointer_cast<framework::StoreInst>(I);
  std::vector<framework::Transition> transitions;

  auto value_operand = store_inst->ValueOperand();
  if (framework::shared_isa<framework::NullValue>(value_operand)) {
    auto possible_transitions =
        state_manager_.TransitionManager()->getStoreArgTransitions(
            framework::StoreValueTransitionRule::NULL_VAL);

    transitions.insert(transitions.end(), possible_transitions.begin(),
                       possible_transitions.end());
  } else {
    auto possible_transitions =
        state_manager_.TransitionManager()->getStoreArgTransitions(
            framework::StoreValueTransitionRule::NON_NULL_VAL);

    transitions.insert(transitions.end(), possible_transitions.begin(),
                       possible_transitions.end());
  }

  if (auto call_inst =
          framework::shared_dyn_cast<framework::CallInst>(value_operand)) {
    auto called_func = call_inst->CalledFunction();
    if (called_func) {
      auto possible_transitions =
          state_manager_.TransitionManager()->getStoreArgTransitions(
              framework::StoreValueTransitionRule::CALL_FUNC,
              called_func->Name());

      transitions.insert(transitions.end(), possible_transitions.begin(),
                         possible_transitions.end());
    }
  }

  auto possible_transitions =
      state_manager_.TransitionManager()->getStoreArgTransitions(
          framework::StoreValueTransitionRule::ANY);
  transitions.insert(transitions.end(), possible_transitions.begin(),
                     possible_transitions.end());

  std::set<std::shared_ptr<framework::Value>> related_values =
      currentFunctionInformation()->GetValueCollection().getRelatedValues(
          store_inst->PointerOperand());
  related_values.insert(store_inst->PointerOperand());

  for (auto value : related_values) {
    changeValueState(transitions, value, I);
  }

  checkAlias(store_inst);
}

void Analyzer::checkAlias(std::shared_ptr<framework::StoreInst> store_inst) {
  auto value_operand = store_inst->ValueOperand();

  const ValueCollection *aliased = currentFunctionInformation()
                                       ->getCurrentBasicBlockInformation()
                                       ->getAliasValues()
                                       .getAliasInfo(value_operand);

  currentFunctionInformation()
      ->getCurrentBasicBlockInformation()
      ->getAliasValues()
      .addAlias(store_inst->PointerOperand(), store_inst->ValueOperand());

  // Check for additional alias information
  if (shared_isa<framework::ConstValue>(value_operand) ||
      shared_isa<framework::NullValue>(value_operand) ||
      (shared_isa<framework::CallInst>(value_operand) && !aliased))
    return;

  std::set<std::shared_ptr<framework::Value>> related_values =
      currentFunctionInformation()->GetValueCollection().getRelatedValues(
          store_inst->PointerOperand());
  related_values.insert(store_inst->PointerOperand());

  if (!shared_isa<framework::CallInst>(value_operand)) {
    auto value_related =
        currentFunctionInformation()->GetValueCollection().getRelatedValues(
            value_operand);

    related_values.insert(value_related.begin(), value_related.end());
    related_values.insert(value_operand);
  }

  if (aliased) {
    auto values = aliased->Values();
    related_values.insert(values.begin(), values.end());
  }

  auto possible_transitions =
      state_manager_.TransitionManager()->getAliasTransitions();
  for (auto value : related_values) {
    changeValueState(possible_transitions, value, store_inst);
  }
}

void Analyzer::analyzeLoadInst(std::shared_ptr<framework::Instruction> I) {
  auto load_inst = std::static_pointer_cast<framework::LoadInst>(I);
  auto transitions =
      state_manager_.TransitionManager()->getUseValueTransitions();

  changeValueState(transitions, load_inst->LoadValue(), I);
}

bool Analyzer::functionInformationExists(
    std::shared_ptr<framework::Function> function) {
  return function_info_.find(function) != function_info_.end();
}

void Analyzer::changeValueState(std::vector<Transition> transitions,
                                std::shared_ptr<Value> value,
                                std::shared_ptr<framework::Instruction> inst) {
  if (value->isGlobalVar() || transitions.empty()) return;
  if (currentFunctionInformation()
          ->getCurrentBasicBlockInformation()
          ->changeValueState(transitions, value, inst)) {
    currentFunctionInformation()->addValue(value);
    framework::generateWarning(inst.get(),
                               "[Value Change] Change Value State: ");
    framework::generateWarning(inst.get(), value.get());
  }
}

void Analyzer::generateError(
    BugNotificationTiming timing,
    const std::set<std::shared_ptr<framework::Value>> values) {
  for (auto state : state_manager_.getBugStates()) {
    if (state.NotificationTiming() != timing) continue;
    for (auto value : bb_info_->getValueTransitionStates(state)) {
      if (!values.empty() && values.find(value.first) == values.end()) continue;
      if (value.second->CurrentState() != state) continue;
      if (!value.second->LeastSignificantSource().isInitState()) continue;
      if (value.second->containsNegativeLogs()) continue;
      /* if (!value.second->ReducedTransition().Source().isInitState()) continue; */

      if (state.getTriggerConstraint() == TriggerConstraint::NON_RETURN &&
          value.first ==
              currentFunctionInformation()->Function()->getReturnValue()) {
        continue;
      }

      if (framework::CommandLineArgs::Flex ||
          !value.first->isArbitaryArrayElement()) {
        llvm::raw_string_ostream log_stream = log_.raw_stream();
        framework::generateError(log_stream,
                                 value.second->CurrentInstruction().get(),
                                 "--- [" + state.Name() + "] ---");
        framework::generateError(log_stream,
                                 value.second->CurrentInstruction().get(),
                                 value.first.get());
        value.second->generateLog(log_stream);
      }
      value.second->logicalTerminate(value.second->CurrentInstruction());
    }
  }
}

void Analyzer::copyFunctionValues(
    std::shared_ptr<framework::Function> called_func,
    std::shared_ptr<framework::CallInst> call_inst) {
  if (!functionInformationExists(called_func)) return;
  auto called_func_info = function_info_[called_func];

  if (called_func->ProtectedRefcountValue() &&
      called_func->lastRefcountInstruction() != call_inst) {
    // Simply do not consider the refcount values for now (This will obviously
    // become a problem in situations such as leak, so we need to think of
    // another option).
    return;
  }

  // Check if current block information should be copied or not
  if (addPendingFunctionValues(called_func, call_inst)) return;

  auto success_blocks = called_func_info->getSuccessBlock();
  if (success_blocks.empty()) return;

  auto operands = call_inst->Arguments();
  ArgValueStates pending_states(operands.size(), state_manager_.getStates());
  for (auto success_block_ref : success_blocks) {
    auto success_block = success_block_ref.lock();
    if (!success_block) continue;
    auto basic_block_info =
        called_func_info->getBasicBlockInformation(success_block);

    if (!basic_block_info) continue;
    pending_states.addArgValueState(basic_block_info->getArgValueStates());

    /* llvm::errs() << "=== :(\n"; */
    /* auto operands = call_inst->Arguments(); */
    /* for (int i = 0; i < operands.size(); i++) { */
    /*   auto operand = operands[i]; */
    /*   for (auto value : */
    /*        basic_block_info->getArgValueStates().getValueStateForArg(i)) { */
    /*     llvm::errs() << "=== " << i << "\n"; */
    /*     llvm::errs() << value.first; */
    /*     for (auto trans : value.second) { */
    /*       llvm::errs() << trans << " (Transitions)\n"; */
    /*     } */
    /* auto new_value = Value::CreateAppend(operand, value.first); */
    /* changeValueState(value.second, new_value, call_inst); */
    /* } */
    /* } */
    /* llvm::errs() << "=== :)\n"; */
  }

  generateWarning(call_inst.get(), "Call Inst Here");
  for (int i = 0; i < operands.size(); i++) {
    auto operand = operands[i];
    for (auto value : pending_states.getValueStateForArg(i)) {
      auto new_value = Value::CreateAppend(operand, value.first);
      if (called_func->ProtectedRefcountValue() &&
          framework::shared_isa<framework::Argument>(new_value))
        continue;
      changeValueState(value.second, new_value, call_inst);
    }
  }
}

bool Analyzer::addPendingFunctionValues(
    std::shared_ptr<framework::Function> called_func,
    std::shared_ptr<framework::CallInst> call_inst) {
  auto branch_inst =
      currentFunctionInformation()->currentBasicBlock()->getBranchInst();
  if (!branch_inst || !branch_inst->Condition()) return false;

  generateWarning(call_inst.get(), "Found BranchInst");

  int compared_value = framework::FunctionInformation::kSuccessCode;
  llvm::CmpInst::Predicate predicate = llvm::CmpInst::Predicate::ICMP_NE;
  bool is_null_value = false;

  if (auto compare_inst = framework::shared_dyn_cast<framework::CompareInst>(
          branch_inst->Condition())) {
    if (!compare_inst->operandExists(call_inst)) {
      return false;
    }
    generateWarning(call_inst.get(), "Found CompareInst");

    predicate = compare_inst->GetPredicate();

    auto operand = std::find_if(
        compare_inst->Operands().begin(), compare_inst->Operands().end(),
        [call_inst](auto operand) { return operand != call_inst; });

    if (operand == compare_inst->Operands().end()) return false;

    if (auto const_value =
            framework::shared_dyn_cast<framework::ConstValue>(*operand)) {
      compared_value = const_value->getConstValue();
    } else if (framework::shared_isa<framework::NullValue>(*operand)) {
      is_null_value = true;
    }
  } else if (framework::shared_isa<framework::CallInst>(
                 branch_inst->Condition())) {
    if (call_inst != branch_inst->Condition()) return false;
    generateWarning(call_inst.get(), "Found Call Inst instead");
  }

  if (!functionInformationExists(called_func)) return false;
  auto called_func_info = function_info_[called_func];
  generateWarning(call_inst.get(), "Found called func info");

  auto basic_block_info =
      currentFunctionInformation()->getCurrentBasicBlockInformation();
  for (auto ret : called_func_info->getReturnValueInfo()) {
    bool is_false_path = false;
    switch (predicate) {
      case llvm::CmpInst::Predicate::ICMP_EQ:
        is_false_path = ret.first == compared_value;
        break;
      case llvm::CmpInst::Predicate::ICMP_NE:
        is_false_path = ret.first != compared_value;
        if (is_null_value) is_false_path = !is_false_path;
        break;
      case llvm::CmpInst::Predicate::ICMP_SLE:
      case llvm::CmpInst::Predicate::ICMP_ULE:
        is_false_path = ret.first >= compared_value;
        break;
      case llvm::CmpInst::Predicate::ICMP_SGE:
      case llvm::CmpInst::Predicate::ICMP_UGE:
        is_false_path = ret.first <= compared_value;
        break;
      case llvm::CmpInst::Predicate::ICMP_SLT:
      case llvm::CmpInst::Predicate::ICMP_ULT:
        is_false_path = ret.first < compared_value;
        break;
      case llvm::CmpInst::Predicate::ICMP_SGT:
      case llvm::CmpInst::Predicate::ICMP_UGT:
        is_false_path = ret.first > compared_value;
        break;
      default:
        break;
        // TODO
    }
    generateWarning(call_inst.get(),
                    std::to_string(ret.first) + " Called Func");
    generateWarning(call_inst.get(),
                    is_false_path ? "True Path" : "False Path");

    BranchInst::TransitionNodes nodes = is_false_path
                                            ? branch_inst->TruePathNodes()
                                            : branch_inst->FalsePathNodes();
    for (auto successor_node : nodes) {
      auto ret_value = std::make_shared<framework::ConstValue>(ret.first);
      auto locked = successor_node.lock();
      for (auto success_block_ref : ret.second) {
        auto success_block = success_block_ref.lock();
        if (!success_block) continue;
        auto success_basic_block_info =
            called_func_info->getBasicBlockInformation(success_block);

        if (!success_block->Instructions().empty())
          generateWarning(success_block->Instructions().front().get(),
                          "Pred Blocks");

        if (!locked->Instructions().empty())
          generateWarning(locked->Instructions().front().get(),
                          "Propagating Block");

        if (!success_basic_block_info) continue;

        basic_block_info->setPendingValueStates(
            successor_node, success_basic_block_info->getArgValueStates());
        basic_block_info->setPendingReturnValues(successor_node, ret_value);
      }
    }
  }

  currentFunctionInformation()->addValues(
      called_func_info->GetValueCollection());
  return true;
}

bool Analyzer::analyzeFunctionCall(
    std::shared_ptr<framework::CallInst> call_inst) {
  auto function = call_inst->CalledFunction();
  if (!function || call_inst->Arguments().empty()) return false;

  bool changed = false;
  for (int arg = 0; arg < call_inst->Arguments().size(); arg++) {
    const std::pair<FunctionArgTransitionRule::FunctionArg,
                    std::vector<framework::Transition>>
        transitions =
            state_manager_.TransitionManager()->getFunctionArgTransitions(
                function->Name(), arg);
    changed = changed || !transitions.second.empty();

    std::set<std::shared_ptr<framework::Value>> args;
    if (transitions.first.consider_parent) {
      args = currentFunctionInformation()->GetValueCollection().getParentValues(
          call_inst->Arguments()[arg]);
    }

    args.insert(call_inst->Arguments()[arg]);

    for (auto &value : args) {
      changeValueState(transitions.second, value, call_inst);
    }
  }
  return changed;
}

void Analyzer::analyzeReturnValue(
    std::shared_ptr<framework::Function> function) {
  auto return_block = function->ReturnBlock();
  auto func_info = getFunctionInformation(function);
  if (!return_block || !func_info) return;

  if (!function->ReturnType()->isIntOrPtrTy()) {
    func_info->addReturnValueInfo(FunctionInformation::kSuccessCode,
                                  return_block);
    return;
  }

  std::set<std::weak_ptr<framework::BasicBlock>> pred_block(
      return_block->Predecessors());

  /* if (pred_block.empty() || !return_block->Instructions().empty()) { */
  /*   pred_block.insert(return_block); */
  /* } */

  if (!return_block->Instructions().empty()) {
    pred_block.clear();
  }

  if (pred_block.empty()) {
    pred_block.insert(return_block);
  }

  for (auto pred_reference : pred_block) {
    if (auto pred = pred_reference.lock()) {
      if (!pred->Instructions().empty()) {
        generateWarning(pred->Instructions().front().get(),
                        "Pred Block analysis");
      }

      auto block_info = func_info->getBasicBlockInformation(pred);
      if (!block_info) continue;
      if (block_info->ReturnValues().empty()) {
        func_info->addReturnValueInfo(FunctionInformation::kSuccessCode, pred);
        func_info->addReturnValueInfo(FunctionInformation::kErrorCode, pred);
        continue;
      }

      for (auto return_value : block_info->ReturnValues()) {
        if (auto const_int = framework::shared_dyn_cast<framework::ConstValue>(
                return_value)) {
          auto value = const_int->getConstValue();
          generateWarning(std::to_string(value));
          if (block_info->ReturnValueSatisfiable(value))
            func_info->addReturnValueInfo(value, pred);
        } else if (auto const_null =
                       framework::shared_dyn_cast<framework::NullValue>(
                           return_value)) {
          func_info->addReturnValueInfo(FunctionInformation::kErrorCode, pred);
          // Add the value to the list
        } else if (auto call_inst =
                       framework::shared_dyn_cast<framework::CallInst>(
                           return_value)) {
          auto called_function = call_inst->CalledFunction();
          if (called_function) {
            if (called_function->isErrorFunction()) {
              func_info->addReturnValueInfo(FunctionInformation::kErrorCode,
                                            pred);
              continue;
            }

            auto called_func_info = getFunctionInformation(called_function);
            if (!called_func_info) {
              /* func_info->addReturnValueInfo(FunctionInformation::kErrorCode,
               */
              /*                               pred); */
              /* func_info->addReturnValueInfo(FunctionInformation::kSuccessCode,
               */
              /*                               pred); */
              continue;
            }

            for (auto return_value : called_func_info->getReturnValueInfo()) {
              /* func_info->addReturnValueInfo(return_value.first, */
              /*                               return_value.second); */
              if (block_info->ReturnValueSatisfiable(return_value.first))
                func_info->addReturnValueInfo(return_value.first, pred);
            }
          }
          // Decode the call_inst by chaning
        } else {
          if (return_value->getValueID() > 0) {
            func_info->addReturnValueInfo(FunctionInformation::kSuccessCode,
                                          pred);
            func_info->addReturnValueInfo(FunctionInformation::kErrorCode,
                                          pred);
          }
          // is success block
        }
      }
    }
  }
}

std::shared_ptr<FunctionInformation> Analyzer::getFunctionInformation(
    std::shared_ptr<framework::Function> function) {
  auto func_info = function_info_.find(function);
  if (func_info != function_info_.end()) return func_info->second;
  return nullptr;
}

}  // namespace framework
