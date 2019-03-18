// 15-745 S18 Assignment 2: liveness.cpp
// Group:
////////////////////////////////////////////////////////////////////////////////

#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/User.h"

#include "dataflow.h"
#include "available-support.h"

#include<string>

using namespace llvm;

namespace {

  std::vector<std::string> variables;


  bool transfer_live(BasicBlock* b, std::map<BasicBlock*, BBInfo*> &m){
    BitVector old(*(m[b]->input));
    BitVector use(variables.size(),false);
    BitVector def(variables.size(), false);
    BitVector In(*(m[b]->output));

    for(BasicBlock::reverse_iterator bi = b->rbegin(),be = b->rend(); bi!=be;bi++){
      Instruction* I = &*bi;
      
      
      for(User::op_iterator OI = I->op_begin(); OI!= I->op_end(); OI++){
        Value* v = *OI;
        if(isa<Instruction>(v) || isa<Argument>(v)){
          for(unsigned i=0; i<variables.size();i++){
            if(getShortValueName(v) == variables[i]){
              use[i] = true;
            }
          }
        }
      }


      Value* val = I;
      for(unsigned i =0; i<variables.size(); i++){
        if(getShortValueName(val)==variables[i]){
	  def[i] = true;
	}
      }

      def = def.flip();
      In &= def;
      use |= In;
      In = use;

      errs()<<*I<<"\n";
      if(!(isa<PHINode>(I))){
	 errs()<<"{";
      for(unsigned k =0; k<variables.size();k++){
        if(In[k]){
	  errs()<< variables[k]<<" ";
	}
      }
      errs()<<"}"<<"\n";
      }

      use.reset();
      def.reset();

    }

   m[b]->input = new BitVector(In);
   errs()<<"!!!!!!!!!!!!!!!!!!!!!!!!!"<<"\n";
   return (*(m[b]->input) != old);
 
  
  }
  
  bool  meet_live(BasicBlock* b, std::map<BasicBlock*, BBInfo*> &m){
    bool change = false;
    BitVector old(*(m[b]->output));
    
    for(BasicBlock::iterator bi = b->begin(), be = b->end(); bi!= be; ++bi){
      Instruction* I = &*bi;
      if (ReturnInst *RI = dyn_cast<ReturnInst>(I)){
        return true;
      }
    }
   



    TerminatorInst* ti = b->getTerminator();
    int bit_size = variables.size();
    BitVector meet(bit_size, false);

    for(unsigned i=0, nsucc = ti->getNumSuccessors(); i<nsucc;++i){
       BasicBlock* succ = &*ti->getSuccessor(i);
       BitVector succIn(*(m[succ]->input));
       for(BasicBlock::iterator bi = succ->begin(), ei = succ->end(); bi != ei; ++bi){
          Instruction* ins = &(*bi);
          if(PHINode* PHI = dyn_cast<PHINode>(ins)){
             for(unsigned t = 0; t<PHI->getNumIncomingValues(); ++t){
               if(PHI->getIncomingBlock(t)->getName() != b->getName()){
                  for(unsigned cl = 0;cl < variables.size(); cl++){
                    if(variables[cl] == getShortValueName((PHI->getIncomingValue(t)))){
                      succIn[cl] = false;
                     
                  
                }
              }
            }
          }
        } else {
          continue; 
	}

      }

      meet |= succIn;
    }

    m[b]->output = new BitVector(meet);

    if(*m[b]->output != old){
      change = true; 
    }


    return change;

  
  
  }	
	
  
  class Liveness : public FunctionPass {
  public:
    static char ID;

    Liveness() : FunctionPass(ID) { }

    virtual bool runOnFunction(Function& F) {
       for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
          BasicBlock* block = &*FI;
          for (BasicBlock::iterator i = block->begin(), e = block->end(); i!=e; ++i) {
            Instruction* I = &*i;
	    for (User::op_iterator oi = I->op_begin(); oi != I->op_end(); ++oi){
	      Value* v = *oi;
	      if(isa<Instruction>(v) || isa<Argument>(v)){
		   bool isin = false;
		   for(int i =0; i<variables.size();i++){
		     if(variables[i] == getShortValueName(v)){ isin = true;}
		   }
		   if(!isin){
		   
	             variables.push_back(getShortValueName(v));
		   }
		   
		   
	      }
	    
	    }

          }
        }

       errs() << "Variables used by this function:\n";

       for(unsigned i=0; i<variables.size(); i++){
       
          
         errs() << variables[i] <<"\n";
       }
       errs()<<"=================================================="<<"\n";


       Flow liveness(false, F,*transfer_live, *meet_live, variables.size() );

       liveness.analyze();

      // Did not modify the incoming Function.
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
