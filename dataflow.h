// 15-745 S18 Assignment 2: dataflow.h
// Group:
////////////////////////////////////////////////////////////////////////////////

#ifndef __CLASSICAL_DATAFLOW_H__
#define __CLASSICAL_DATAFLOW_H__

#include <stdio.h>
#include <iostream>
#include <queue>
#include <vector>

#include "llvm/IR/Instructions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/IR/CFG.h"
#include <map>
#include <queue>

namespace llvm {

// Add definitions (and code, depending on your strategy) for your dataflow
// abstraction here.
//
//
//

  class BBInfo{
    public:
      bool inserted;
      BitVector* input;
      BitVector* output; 
      BBInfo():inserted(false){}
      BBInfo(int n){
        inserted = false;
        input = new BitVector(n,false); 
        output = new BitVector(n,false);
      }
  };


  class Flow {
    protected:
      bool isFwd;
      Function* Func;
      void (*transfer)(BasicBlock*, std::map<BasicBlock*,BBInfo*>&);
      void (*meet)(BasicBlock* , std::map<BasicBlock*,BBInfo* >&); 
      std::map<BasicBlock*,BBInfo* > info;

     public:
      Flow(bool f, Function& p, void (*tran)(BasicBlock*, std::map<BasicBlock*, BBInfo* >&),void (*m)(BasicBlock*,std::map<BasicBlock*, BBInfo*>&));  
      Flow(bool f, Function& p, void (*tran)(BasicBlock*, std::map<BasicBlock*, BBInfo*>&),void (*m)(BasicBlock*,std::map<BasicBlock*, BBInfo*>&),int n);  


      void analyze();
      void runAnalysis(BasicBlock* b);
  };

}

#endif
