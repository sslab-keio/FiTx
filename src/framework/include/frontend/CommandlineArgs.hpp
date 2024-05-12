#include "llvm/Support/CommandLine.h"

namespace framework {
  namespace CommandLineArgs {
    llvm::cl::opt<bool> Flex("flex", llvm::cl::desc("Print all possible errors"));
  }
}
