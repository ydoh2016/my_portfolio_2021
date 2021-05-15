/* author : Yanghoon, 2017-14342                   */
/* date   : 20210417 - 20210418                    */
/* for    : SWPP assignment4                       */

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/PatternMatch.h" // Implement
#include "llvm/IR/Function.h" // Implement

using namespace llvm;
using namespace std;
using namespace llvm::PatternMatch;
//using namespace llvm::Function;

namespace {
class PropagateIntegerEquality : public PassInfoMixin<PropagateIntegerEquality> {
  /* Replace all (old) with (eq) in BasicBlock (BB)                             */
  /* Note 1 : do not replace (old) outside (BB)                                 */
  /* Note 2 : domination is already checked before this function                */
  /*          it means that (BB) is dominated by true branch we're dealing with */
  void replaceSpecificUsesWithEquality(BasicBlock* BB, Value* eq, Value* old)
  {
    for(auto itr = old -> use_begin(), end = old -> use_end(); itr != end;)
    {
      Use &U = *itr++;
      User *Usr = U.getUser();
      Instruction *UsrI = dyn_cast<Instruction>(Usr);
      /* Is instruction / value to be replaced is in side (BB)? */
      if(BB -> getName() != UsrI -> getParent() -> getName()) continue;
      U.set(eq);
    }
  }
public:
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    /* Dominator Tree */
    DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
    for(auto &BB : F)
    {
      /* Value */
      Value* X;
      Value* Y;
      /* Predicate to find out -eq- */
      ICmpInst::Predicate Pred;
      /* CODE for replacement */
      /* 3 : II               */
      /* 2 : IA               */
      /* 1 : AI               */
      /* 0 : AA               */
      int code = -1; /* code will be -1 if there is no ICMP eq in BasicBlock */
      for(auto &I : BB)
      {
        if(match(&I, m_ICmp(Pred, m_Value(X), m_Value(Y))) && Pred == ICmpInst::ICMP_EQ)
        {
          code += 1;
          /* is X instruction? */
          auto *C1 = dyn_cast<Instruction>(X);
          if(C1 != nullptr)
          {
            code += 2;
          }
          /* is Y instruction? */
          auto *C2 = dyn_cast<Instruction>(Y);
          if(C2 != nullptr)
          {
            code += 1;
          }
        }
      }
      BranchInst *TI = dyn_cast<BranchInst>(BB.getTerminator());
      if(TI != nullptr)
      {
        /* First successor will be true branch for ICMP eq */
        /* where one can be replaced with another          */
        BasicBlock *BBNext = TI -> getSuccessor(0);
        /* BB is BasicBlock where ICMP eq we're currently dealing with lies */
        BasicBlockEdge BBE(&BB, BBNext);
        /* BBTrue is new BasicBlock which iterates through Function         */
        /* if BBTrue is dominated by true branch, replacement will be made  */
        for(auto &BBTrue : F)
        {
          if((DT.dominates(BBE, &BBTrue)) && (BBTrue.getName() != BB.getName()))
          {
            for(auto &I : BBTrue)
            {
              /* arg == arg : look for order in which arguments were passed*/
              if(code == 0)
              {
                for(Function::arg_iterator I = F.arg_begin(), E = F.arg_end(); I != E; ++I)
                {
                  /* X precedes Y */
                  if(dyn_cast<Value>(I) == X)
                  {
                    replaceSpecificUsesWithEquality(&BBTrue, X, Y);
                    break;
                  }
                  /* Y precedes X */
                  else if(dyn_cast<Value>(I) == Y)
                  {
                    replaceSpecificUsesWithEquality(&BBTrue, Y, X);
                    break;
                  }
                }
                
              }
              /* arg == inst : replace inst(Y) with arg(X) */
              else if(code == 1) 
              {
                replaceSpecificUsesWithEquality(&BBTrue, X, Y);
              }
              /* inst == arg : replace inst(X) with arg(Y) */
              else if(code == 2)
              {
                replaceSpecificUsesWithEquality(&BBTrue, Y, X);
              }
              /* inst == inst : use dominates(Value*, Instruction) */
              else if(code == 3)
              {
                /* X dominates Y */
                if(DT.dominates(X, dyn_cast<Instruction>(Y)))
                {
                  replaceSpecificUsesWithEquality(&BBTrue, X, Y);
                }
                /* Y dominates X */
                else if(DT.dominates(Y, dyn_cast<Instruction>(X)))
                {
                  replaceSpecificUsesWithEquality(&BBTrue, Y, X);
                }
              }
            }
          }
        }
      }
    }
    return PreservedAnalyses::all();
  }
};
}

extern "C" ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "PropagateIntegerEquality", "v0.1",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "prop-int-eq") {
            FPM.addPass(PropagateIntegerEquality());
            return true;
          }
          return false;
        }
      );
    }
  };
}
