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

  std::vector<Expression> expressions;

  bool available_transfer(BasicBlock* b, std::map<BasicBlock*, BBInfo*> &m){
   	BitVector gen(expressions.size(), false);
  	BitVector kill(expressions.size(), false);
	BitVector out(expressions.size(), false);
	BitVector in(*(m[b]->input));

        for (BasicBlock::iterator i = b->begin(), e = b->end(); i!=e; ++i) {
          Instruction* I = &*i;
	  errs()<<*I<<"\n";
          if (BinaryOperator *BI = dyn_cast<BinaryOperator>(I)) {
	  Value* v = I;
	  std::string ins = getShortValueName(v);
            for(unsigned k=0; k<expressions.size(); k++){
	    	if(expressions[k]==Expression(BI)){
			gen[k] = true;
                        for(unsigned j =0;j<k;j++){
			   if(ins == (expressions[j].v1)->getName().str() || ins == (expressions[j].v2)->getName().str()){
		  	       gen[j] = false;
		           }

			}
		}
                

	  
	  }

	    for(unsigned k=0;k<expressions.size();k++){
	      if(ins == (expressions[k].v1)->getName().str() || ins == (expressions[k].v2)->getName().str()){
                               kill[k] = true;
                           }

	    
	    }

//	out = kill;
	kill = kill.flip();
	in &= kill;
	gen |= in;
	out = gen;
	in = out;
        errs()<<"{";
	for(unsigned j = 0;j<expressions.size();j++){
	  if(out[j]){
	    errs()<< expressions[j].toString()<<", ";
	  }
	  //if(kill[j]){
	    //errs()<< expressions[j].toString()<<", ";
	 // }
	
	}
	errs()<<"}"<<"\n";
	gen.reset();
        kill.reset();	
          }

        }
        BitVector old(*(m[b]->output));
	m[b]->output = new BitVector(out);
	return old == out;

  }

  bool available_meet(BasicBlock* lb, std::map<BasicBlock*, BBInfo*> &m){
    if(pred_begin(lb) == pred_end(lb)) //if entry block
    {
      return true;
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

    BitVector old(*(m[lb]->input));
    m[lb]->input = new BitVector(meeting);



    return old==meeting;
  }





  class AvailableExpressions : public FunctionPass {

    public:
      static char ID;

      AvailableExpressions() : FunctionPass(ID) { }


      virtual bool runOnFunction(Function& F) {

        // Here's some code to familarize you with the Expression
        // class and pretty printing code we've provided:

       for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
          BasicBlock* block = &*FI;
          for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
            Instruction* I = &*i;
            // We only care about available expressions for BinaryOperators
            if (BinaryOperator *BI = dyn_cast<BinaryOperator>(I)) {
              // Create a new Expression to capture the RHS of the BinaryOperator
	      if(std::find(expressions.begin(), expressions.end(), Expression(BI)) == expressions.end()){
	        expressions.push_back(Expression(BI));
	      }
            }
          }
        }

        // Print out the expressions used in the function
        outs() << "Expressions used by this function:\n";
        printSet(&expressions);

        Flow df(true,F,available_transfer,available_meet,expressions.size());
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
