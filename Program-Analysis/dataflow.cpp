// 15-745 S18 Assignment 2: aataflow.cpp
// Group:
////////////////////////////////////////////////////////////////////////////////

#include "dataflow.h"

namespace llvm {
  Flow::Flow(bool f, Function& p, bool (*tran)(BasicBlock*, std::map<BasicBlock*, BBInfo* >&), bool(*m)(BasicBlock*,std::map<BasicBlock*,BBInfo* >&)){

    isFwd = f;
    Func = &p;
    transfer = tran;
    meet = m;


    for(Function::iterator b = Func->begin(), e = Func->end(); b!=e;++b){
      BasicBlock* bb = &*b;
      info[bb] = new BBInfo;
    }
    std::cout<<"ss"<<"\n";
  }

  Flow::Flow(bool f, Function& p, bool (*tran)(BasicBlock*, std::map<BasicBlock*, BBInfo* > &),bool (*m)(BasicBlock*,std::map<BasicBlock*, BBInfo*> &),int n){
    isFwd = f;
    Func = &p;
    transfer = tran;
    meet = m;
    std::cout<<"sxb"<<"\n";


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

   //   for(Function::iterator b = Func->begin(), e = Func->end(); b!=e; ++b){
//	BasicBlock* block = &*b;
  //      for(BasicBlock::iterator bi = b->begin(), be = b->end(); bi !=be; ++bi){
//	  Instruction* i = &*bi;
//	  if (ReturnInst *RI = dyn_cast<ReturnInst>(i)){
//	    runAnalysis(block);
//	    break;
//	   
//	  }
//	}
  //    }
       BasicBlock* exit = &Func->back();
       runAnalysis(exit);
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

    bool c = true;
    int count = 1;
    while(c){

      c = false;
    while(q.size() != 0){
      BasicBlock* curr = q.front();
      q.pop();

      bool mc = meet(curr,info);
      bool tc = transfer(curr,info);

      c = mc || tc;

      TerminatorInst* ti = curr -> getTerminator();
      if(isFwd){
        for(unsigned i=0, nsucc = ti -> getNumSuccessors(); i<nsucc;++i){
          BasicBlock* succ = &*ti->getSuccessor(i);
          if(!info[succ]->inserted){
            q.push(succ);
            info[succ]->inserted = true;
          }
         }
      }else{
	for (auto it = pred_begin(curr), et = pred_end(curr); it != et; ++it){
          BasicBlock* pred = *it;

	  if(!info[pred]->inserted){
	    q.push(pred);
	    info[pred]->inserted = true;
	  }
	}
      
      
      
      }
    }
    
      count++;
      for(Function::iterator bf = Func->begin(), ef = Func->end(); bf!=ef; ++bf){
	BasicBlock* block = &*bf;
	info[block]->inserted = false;
      }
      info[b]->inserted =true;
      q.push(b);
      errs() <<"-------------------------------------------------------------------------------------------"<<"\n";

    }
    errs() <<count<<"\n";
  }
  // Add code for your dataflow abstraction here.
}
