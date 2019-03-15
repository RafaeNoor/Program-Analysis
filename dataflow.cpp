// 15-745 S18 Assignment 2: aataflow.cpp
// Group:
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"

namespace llvm {
  Flow::Flow(bool f, Function& p, bool (*tran)(BasicBlock*, std::map<BasicBlock*, BBInfo* >&), bool (*m)(BasicBlock*,std::map<BasicBlock*,BBInfo* >&)){
    isFwd = f;
    Func = &p;
    transfer = tran;
    meet = m;

    for(Function::iterator b = Func->begin(), e = Func->end(); b!=e;++b){
      BasicBlock* bb = &*b;
      info[bb] = new BBInfo;
    }
  }

  Flow::Flow(bool f, Function& p, bool (*tran)(BasicBlock*, std::map<BasicBlock*, BBInfo* > &),bool (*m)(BasicBlock*,std::map<BasicBlock*, BBInfo*> &),int n){
    isFwd = f;
    Func = &p;
    transfer = tran;
    meet = m;

    for(Function::iterator b = Func->begin(), e = Func->end(); b!=e;++b){
      BasicBlock* bb = &*b;
      info[bb] = new BBInfo(n);
    }
  }  

  void Flow::analyze()
  {

    if(isFwd){
      BasicBlock* entry = &Func->getEntryBlock();
      runAnalysis(entry);
    } else {


    }

  }

  void Flow::runAnalysis(BasicBlock* b)
  {
    if(!b){
      return;
    }

    std::queue<BasicBlock* > q;
    q.push(b);
    info[b]->inserted = true;

    bool converge = false;

    int numIter = 1;

    while(!converge){

      errs()<<"\nIteration Number: "<<numIter<<" \n";
      numIter++;

      converge = true;
      while(q.size() != 0){
        BasicBlock* curr = q.front();
        q.pop();


        bool inputChanged = meet(curr,info);

        bool outputChanged = transfer(curr,info);

        if(inputChanged || outputChanged)
          converge = false;

        if(isFwd){
          TerminatorInst* ti = curr -> getTerminator();
          for(unsigned i=0, nsucc = ti -> getNumSuccessors(); i<nsucc;++i){
            BasicBlock* succ = &*ti->getSuccessor(i);
            if(!info[succ]->inserted){
              q.push(succ);
              info[succ]->inserted = true;
            }
          }
        } else {
          for (auto it = pred_begin(curr), et = pred_end(curr); it != et; ++it){
            BasicBlock* pred = *it;
            if(!info[pred]->inserted){
              q.push(pred);
              info[pred]->inserted = true;
            }
          }
        }


      }
      errs()<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
      for(Function::iterator bb = Func->begin(), ee = Func->end(); bb!=ee; bb++)
      {
        BasicBlock* refresh = &*bb;
        info[refresh]->inserted = false;
      }

      info[b]->inserted = true;
      q.push(b);
    }

  }

  // Add code for your dataflow abstraction here.
}
