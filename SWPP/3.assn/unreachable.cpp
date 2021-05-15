#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include<string>
#include<vector>
using namespace llvm;

namespace {
class MyUnreachablePass : public PassInfoMixin<MyUnreachablePass> {
public:
  /* unreachable blocks */
  std::vector<std::string> ur;
  /* reachable blocks   */
  std::vector<std::string> r;
  /* manipulate vectors for visited block */
  void check(BasicBlock* pred) {
    int n = pred -> getTerminator() -> getNumSuccessors();
    for(int i = 0; i < n; i++) /* n is number of successors */
    {
      BasicBlock* cur = pred -> getTerminator() -> getSuccessor(i);
      std::string name = cur -> getName().str();
      for(int j = 0; j < r.size(); j++) /* if visited, return */
      {
        if(r[j] == name)
        {
          return;
        }
      }
      for(int j = 0; j < ur.size(); j++) /* if marked unvisited, erase */
        {
          if(ur[j] == name)
          {
            ur[j] = "";
            break;
          }
        }
      r.push_back(name);
      /* recursive call until last block */
      check(cur);
    }
    /* this is last block with no successor */
    return;
  }
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
    BasicBlock* entry;
    for (auto &BB : F)
    {
      std::string name = BB.getName().str();
      if(name == "entry")
      {
        entry = &BB;
      }
      else
      {
        ur.push_back(name);
      }
    }
    check(entry);
    std::sort(ur.begin(), ur.end());
    for(int i = 0; i < ur.size(); i++)
    {
      if(ur[i] != "")
      {
        outs() << ur[i] << "\n";
      }
    }
    return PreservedAnalyses::all();
  }
};
}

extern "C" ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "MyUnreachablePass", "v0.1",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
           ArrayRef<PassBuilder::PipelineElement>) {
          if (Name == "my-unreachable") {
            FPM.addPass(MyUnreachablePass());
            return true;
          }
          return false;
        }
      );
    }
  };
}
