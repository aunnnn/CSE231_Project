#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
struct CountDynamicInstructionsPass : public FunctionPass {
  static char ID;
  CountDynamicInstructionsPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {

    std::map<std::string, int> instructions_counts;

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {  
        std::string Iname = I.getOpcodeName();
        if (instructions_counts.count(Iname) == 0) {
          instructions_counts[Iname] = 1;
        } else {
          instructions_counts[Iname]++;
        }
      }
    }

    std::map<std::string, int>::iterator it;
    for (it = instructions_counts.begin(); it != instructions_counts.end(); it++) {
      errs() << it->first << "\t" << it->second << "\n";
    }
    return false;
  }
}; // end  of struct Coun
}  // end of anonymous namespace

char CountDynamicInstructionsPass::ID = 0;
static RegisterPass<CountDynamicInstructionsPass> X("cse231-cdi", "Developed for cse 231 part1 submission.",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
