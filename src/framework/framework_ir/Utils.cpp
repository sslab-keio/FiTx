#include "framework_ir/Utils.hpp"

#include "core/Instructions.hpp"
#include "llvm/IR/Instructions.h"

namespace ir_generator {
int getPointerDereferenceNum(llvm::Type* type) {
  int dereference_num = 0;

  while(type->isPointerTy()) {
    type = type->getPointerElementType();
    dereference_num++;
  }

  return dereference_num;
}
}  // namespace ir_generator
