// 15-745 S18 Assignment 2: available.cpp
// Group:
////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

#include "dataflow.h"
#include "available-support.h"

using namespace llvm;
using namespace std;

namespace {

  void available_transfer(BasicBlock* b, std::map<BasicBlock*, BBInfo*> &m){
    errs()<<*b<<"\n";
  }

  void available_meet(BasicBlock* lb, std::map<BasicBlock*, BBInfo*> &m){
    if(pred_begin(lb) == pred_end(lb)) //if entry block
    {
      errs()<<"\nInvoked Meet on entry block";
      return;
    }


    
    BasicBlock* pred = *pred_begin(lb);
    int bit_size = m[pred]->output->size();

    

    BitVector meeting(bit_size,true);

    int numPreds = 0;
    for(auto it = pred_begin(lb), et = pred_end(lb); it != et;it++){
      BasicBlock* curr = *it;
      meeting &= *(m[curr]->output); //Take intersection of predecessors
      numPreds++;
    }
    m[lb]->input = new BitVector(meeting);

    errs()<<"# Predeccesors = "<<numPreds<<"\n";

    for(unsigned i =0;i < m[lb]->input->size();i++){
      if((*(m[lb]->input))[i])
        errs()<<"1 ";
      else
        errs()<<"0 ";
    }
    errs()<<"\n";

    return;
  }


  class Available : public Flow {
    private:
      vector<Expression> expressions;
      int numExpressions;

    public:
      Available(std::vector<Expression> vec, Function& p) : expressions(vec), numExpressions(vec.size()),Flow(true,p,available_transfer,available_meet,vec.size()){}




  }; 


  class AvailableExpressions : public FunctionPass {

    public:
      static char ID;

      AvailableExpressions() : FunctionPass(ID) { }


      virtual bool runOnFunction(Function& F) {

        // Here's some code to familarize you with the Expression
        // class and pretty printing code we've provided:

        vector<Expression> expressions;
        for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
          BasicBlock* block = &*FI;
          for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
            Instruction* I = &*i;
            // We only care about available expressions for BinaryOperators
            if (BinaryOperator *BI = dyn_cast<BinaryOperator>(I)) {
              // Create a new Expression to capture the RHS of the BinaryOperator
              expressions.push_back(Expression(BI));
            }
          }
        }

        // Print out the expressions used in the function
        outs() << "Expressions used by this function:\n";
        printSet(&expressions);

        //Flow df(true,F,available_transfer,available_meet);
        //df.analyze();
        //
        Available df(expressions,F);
        df.analyze();



        // Did not modify the incoming Function.
        return false;
      }

      virtual void getAnalysisUsage(AnalysisUsage& AU) const {
        AU.setPreservesAll();
      }

    private:
  };

  char AvailableExpressions::ID = 0;
  RegisterPass<AvailableExpressions> X("available",
      "15745 Available Expressions");
}
