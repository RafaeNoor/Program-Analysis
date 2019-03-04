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

namespace llvm {

// Add definitions (and code, depending on your strategy) for your dataflow
// abstraction here.
//
//
//

  class BBInfo{
    private:
      bool i;
    public:
      BBInfo():i(false){}
  };


  class Flow {
    protected:
      bool isFwd;
      Function* Func;
      void (*transfer)(BasicBlock*);
      void (*meet)(BasicBlock**, int); 
      std::map<BasicBlock*,BBInfo* > info;
     public:
      Flow(bool f, Function& p, void (*tran)(BasicBlock*),void (*m)(BasicBlock**,int));  
      void analyze();
      void runAnalysis(BasicBlock* b);
  };

}

#endif
