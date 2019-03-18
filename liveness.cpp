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

      if(PHINode* PHI = dyn_cast<PHINode>(I)){
      
         for(unsigned t = 0; t<PHI->getNumIncomingValues(); ++t){
            errs()<<"["<<PHI->getIncomingBlock(t)->getName()<<" : "<< *(PHI->getIncomingValue(t))<<"]";
            Value* phi_val = PHI->getIncomingValue(t);
            std::string phi_name = getShortValueName(phi_val);
            errs()<<"PHI_NAME : "<<phi_name;
         }
	 errs()<<"\n";
}else{
    BranchInst* br = dyn_cast<BranchInst>(I);
    if(!br){
	   // errs()<<*br<<"\n";
	  //  break;
     



      for(User::op_iterator OI = I->op_begin(); OI!= I->op_end(); OI++){
        Value* v = *OI;
	if(isa<Instruction>(v) || isa<Argument>(v)){
	  for(unsigned i=0; i<variables.size();i++){
	    if(getShortValueName(v) == variables[i]){
	      use[i] = true;
	   //   errs()<<"adding "<<variables[i]<<" to use["<<b->getName()<<"]"<<"\n";
	    }
	  }
	}
      }

      std::string var = getShortValueName(I);
      for(unsigned i =0; i<variables.size(); i++){
        if(var == variables[i]){
	  def[i] = true;
	 // errs()<<"adding "<<variables[i]<<" to def["<<b->getName()<<"]"<<"\n";
	}
      }


    BitVector DiffTerm(In);
    DiffTerm &= (def.flip());
    BitVector UnionTerm(use);
    UnionTerm |= DiffTerm; 

    In = UnionTerm;
    }
    errs()<<*I<<"\n"<<"{";
    for(unsigned i =0; i<variables.size();i++){
      if(In[i]){
        errs() <<variables[i]<< " ";
      }
    }
    errs()<<"}"<<"\n";
}
    }
    
    m[b]->input = new BitVector(In);


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
    BasicBlock* s = &*ti->getSuccessor(0);
    int bit_size = m[s]->input->size();
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
                     
         //             errs()<<"Removing "<<variables[cl]<<" from succ Input\n";
                  
                }
              }
            }
          }
        } else {
          continue; // As Phi-Node instructions occur at the top of a basic block, if we encounter a Non-Phi Instruction, we can skip
}
       BitVector SucIn(*(m[succ]->input));
      meet |= SucIn;

      } 
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


       Flow liveness(false, F,*transfer_live, *meet_live, variables.size() );

       liveness.analyze();

       for (Function::iterator FI = F.begin(), FE = F.end(); FI != FE; ++FI) {
          BasicBlock* block = &*FI;
          BitVector out(*liveness.info[block]->output);
	  for(unsigned i =0; i<variables.size(); ++i){
	    if(out[i]){
	      errs() << variables[i] <<" ";
	    }
	  }
	    errs() << "\n";
      }
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
