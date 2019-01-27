#include "llvm/Pass.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

  // Modify the program to call functions
struct BranchBiasPass : public FunctionPass {
  static char ID;
  BranchBiasPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {

    LLVMContext& Ctx = F.getContext();
    Module *M = F.getParent();

    Constant* updateBranchInfo = M->getOrInsertFunction("updateBranchInfo", 
    Type::getVoidTy(Ctx), 
    Type::getInt1Ty(Ctx)
    );

    Constant *printOutBranchInfo = M->getOrInsertFunction("printOutBranchInfo", 
    Type::getVoidTy(Ctx)
    );

    for (BasicBlock &BB : F) {    
      llvm::IRBuilder<> builder(&BB);

      // Branch can be only in last instruction.
      Instruction *I = BB.getTerminator();

      // Set the call at last instruction
      builder.SetInsertPoint(I);

      BranchInst *br = dyn_cast<BranchInst>(I);

      // If it is actually branch & CONDITIONAL
      if (br != NULL && br->isConditional()) {
        Value *isTaken = br->getCondition();
        Value* args[1] = {isTaken};
        builder.CreateCall(updateBranchInfo, args);
      }
      
      // Print Analysis at each return
      for (Instruction &I: BB) {
        if ((std::string)(I.getOpcodeName()) == "ret") {
          builder.SetInsertPoint(&I);
          builder.CreateCall(printOutBranchInfo);
        }
      }

    }

    return false;
  }
}; // end  of struct Coun
}  // end of anonymous namespace

char BranchBiasPass::ID = 0;
static RegisterPass<BranchBiasPass> X("cse231-bb", "Developed for cse 231 part1 submission.",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
