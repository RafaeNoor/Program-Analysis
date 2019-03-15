// 15-745 S18 Assignment 2: liveness.cpp
// Group:
////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"
#include "available-support.h"

using namespace llvm;

namespace {

  class Liveness : public FunctionPass {
    public:
      static char ID;

      Liveness() : FunctionPass(ID) { }

      virtual bool runOnFunction(Function& F) {

        // Did not modify the incoming Function.

        std::set< std::string> unique_names;

        for(Function::iterator bb = F.begin(), be = F.end(); bb!=be; bb++){
          BasicBlock* block = &*bb;
          errs()<<*block<<"\n";
          for(BasicBlock::iterator ib = block->begin(), eb = block->end(); ib!=eb;ib++){
            Instruction* ins = &*ib;

            for(User::op_iterator OI = ins ->op_begin();OI != ins->op_end();++OI){
              Value* val = *OI;
              /*if(isa<Instruction>(val) || isa<Argument>(val)){
                //val is used by ins

                errs()<<"["<<*val<<"] is used by ["<<*ins<<"]\n";

              }*/


              if(Instruction* IO = dyn_cast<Instruction>(val)){
                errs()<<"{INS}["<<getShortValueName(val)<<"] is used by ["<<*ins<<"]\n";
                unique_names.insert(getShortValueName(val));
              }
              if(Argument* ARG = dyn_cast<Argument>(val)){
                errs()<<"{ARG}["<<getShortValueName(val)<<"] is used by ["<<*ins<<"]\n";
                unique_names.insert(getShortValueName(val));
              }

            }
            errs()<<"\n";
          }
        }



        for(auto start = unique_names.begin(), end = unique_names.end(); start!=end; ++start){
          errs()<<*start<<"\n";


        }
        return false;
      }

      virtual void getAnalysisUsage(AnalysisUsage& AU) const {
        AU.setPreservesAll();
      }

    private:
  };

  char Liveness::ID = 0;
  RegisterPass<Liveness> X("liveness", "15745 Liveness");
}
