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
struct CountDynamicInstructionsPass : public FunctionPass {
  static char ID;
  CountDynamicInstructionsPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {

    LLVMContext& Ctx = F.getContext();
    Module *M = F.getParent();

    Constant* updateIntstrInfoFunc = M->getOrInsertFunction("updateInstrInfo", 
    Type::getVoidTy(Ctx), 
    Type::getInt32Ty(Ctx), 
    Type::getInt32PtrTy(Ctx),
    Type::getInt32PtrTy(Ctx)
    );

    Constant *printOutInstrInfo = M->getOrInsertFunction("printOutInstrInfo", 
    Type::getVoidTy(Ctx)
    );

    for (BasicBlock &BB : F) {    

      // 1. COUNT INSTRUCTIONS JUST FOR THIS BLOCK
      std::map<int, int> instructions_counts;
      for (Instruction &I: BB) {
        int opcode = I.getOpcode();
        if (instructions_counts.count(opcode) == 0) {
          instructions_counts[opcode] = 1;
        } else {
          instructions_counts[opcode]++;
        }
      }

      // 2. TRANSFORM TO FORMAT THAT WE WILL PASS THROUGH OUR lib231.
      std::vector<Constant*> opcodes;
      std::vector<Constant*> icounts;

      for (auto const &ic: instructions_counts) {
        Constant * opcode = llvm::ConstantInt::get(IntegerType::get(Ctx,32), ic.first);
        opcodes.push_back(opcode);

        Constant * icount = llvm::ConstantInt::get(IntegerType::get(Ctx,32), ic.second);
        icounts.push_back(icount);
      }

      // GET ARRAY TYPE
      ArrayType* arrayType = ArrayType::get(IntegerType::get(Ctx, 32), opcodes.size());

      // CONVERT TO Constant
      Constant* opcodes_arr = ConstantArray::get(arrayType, opcodes);
      Constant* icounts_arr = ConstantArray::get(arrayType, icounts);

      // MAKE GLOBAL VARS TO PASS THROUGH DURING COMPILATION TIME
      GlobalVariable *opcodes_globalvar = new GlobalVariable(*M, arrayType, true, GlobalValue::InternalLinkage, opcodes_arr, "opcodes_arr");
      GlobalVariable *icounts_globalvar = new GlobalVariable(*M, arrayType, true, GlobalValue::InternalLinkage, icounts_arr, "icounts_arr");
      Constant *num = ConstantInt::get(IntegerType::get(Ctx,32), opcodes.size());

      llvm::IRBuilder<> builder(&BB);

      // Set the call at last instruction
      builder.SetInsertPoint(BB.getTerminator());

      Value* opcodes_value = builder.CreatePointerCast(opcodes_globalvar, Type::getInt32PtrTy(Ctx));
      Value* icounts_value = builder.CreatePointerCast(icounts_globalvar, Type::getInt32PtrTy(Ctx));

      Value* args[3] = {num, opcodes_value, icounts_value};
      builder.CreateCall(updateIntstrInfoFunc, args);
      
      // Print Analysis at each return
      for (Instruction &I: BB) {
        if ((std::string)(I.getOpcodeName()) == "ret") {
          builder.SetInsertPoint(&I);
          builder.CreateCall(printOutInstrInfo);
        }
      }
    }
    return false;
  }
}; // end  of struct Coun
}  // end of anonymous namespace

char CountDynamicInstructionsPass::ID = 0;
static RegisterPass<CountDynamicInstructionsPass> X("cse231-cdi", "Developed for cse 231 part1 submission.",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
