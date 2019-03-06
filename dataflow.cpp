// 15-745 S18 Assignment 2: aataflow.cpp
// Group:
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"

namespace llvm {
  Flow::Flow(bool f, Function& p, void (*tran)(BasicBlock*), void (*m)(BasicBlock**,int)){
    isFwd = f;
    Func = &p;
    transfer = tran;
    meet = m;
  }


  void Flow::analyze()
  {
    for(Function::iterator b = Func->begin(), e = Func->end(); b!=e;++b){
      BasicBlock* bb = &*b;
      info[bb] = new BBInfo;
    }


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


    while(q.size() != 0){
      BasicBlock* curr = q.front();
      q.pop();

      transfer(curr);

      TerminatorInst* ti = curr -> getTerminator();
      for(unsigned i=0, nsucc = ti -> getNumSuccessors(); i<nsucc;++i){
        BasicBlock* succ = &*ti->getSuccessor(i);
        q.push(succ);
      }

    }
  }
  // Add code for your dataflow abstraction here.
}
