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

  std::vector<std::string> names;
  void printBitVector(BitVector bv){
    for(unsigned i =0 ; i < bv.size() ;i++){
      if(bv[i])
        errs()<<"1 ";
      else
        errs()<<"0 ";
    }
    errs()<<"\n";
  }


  bool liveness_transfer(BasicBlock* b, std::map<BasicBlock*, BBInfo*> &m){
    if(!b)
      return false;

    errs()<<"\nBasicBlock:"<<*b<<"\n\n";
    //errs()<<"=====================================\n";

    int bv_size = m[b]->input->size(); 


    bool Verbose = true;

    BitVector Use_B(bv_size, false);// Use[B]
    BitVector Def_B(bv_size, false); // Def[B]
    BitVector Out_B(*(m[b]->output)); // Out[B]
    BitVector In_B(*(m[b]->output)); // In[B]


    for(BasicBlock::reverse_iterator BB = b->rbegin(), BE = b->rend(); BB!=BE;BB++){ // Iterate over instructions in reverse
      Instruction* ins = &*BB;
      Value* ins_val = &(*ins);

      //errs()<<*ins<<"\n";


      for(unsigned i =0; i < names.size(); ++i){
        if(names[i] == getShortValueName(ins_val)){
          Def_B[i] = true;
          if(Verbose)
            errs()<<"Adding "<<names[i]<<" to Def[B]\n";
        }
      }

      for(User::op_iterator OI = ins ->op_begin(); OI!= ins->op_end();++OI){
        Value* op_val = *OI;
        if(isa<Instruction>(op_val) || isa<Argument>(op_val)){
          std::string ins_user = getShortValueName(op_val);
          for(unsigned i =0; i < names.size(); ++i){
            if(names[i] == ins_user){
              Use_B[i] = true;
              if(Verbose)
                errs()<<"Adding "<<names[i]<<" to Use[B]\n";
            }
          }
        }
      }



      if(Verbose){
        errs()<<"Use[B] ";
        printBitVector(Use_B);

        errs()<<"Def[B] ";
        printBitVector(Def_B);

        errs()<<"In[B] ";
        printBitVector(In_B);
      }

      //In[B] = Use[B] U (Out[B] - Def[B])

      BitVector DiffTerm(In_B);
      DiffTerm &= (Def_B.flip());

      BitVector UnionTerm(Use_B);
      UnionTerm |= DiffTerm; 

      In_B = UnionTerm;

      errs()<<"{";
      for(unsigned k = 0; k<In_B.size(); k++){
        if(In_B[k] == true){
          errs()<<names[k]<<" ";
        }
      }
      errs()<<"}\n";

      errs()<<*ins<<"\n";

      Def_B.reset();
      Use_B.reset();

    }
    m[b]->input = new BitVector(In_B);
    

    errs()<<"=====================================\n";

    return false;
  }


  bool liveness_meet(BasicBlock* lb, std::map<BasicBlock*, BBInfo*> &m){
    if(!lb)
      return false;

     
    BitVector Old_Output(*(m[lb]->output));
  

    TerminatorInst* ti = lb->getTerminator();
    if(ti->getNumSuccessors() == 0){
      errs()<<"Meet invoked on exit block\n";
      errs()<<"# successors = 0\n==========================================";
      return false;
    }


    BitVector meeting(m[lb]->input->size(),false);
    for(unsigned i=0, nsucc = ti ->getNumSuccessors();i < nsucc; ++i){
      BasicBlock* succ = &*ti->getSuccessor(i);
      meeting |= *(m[succ]->input);
    }
    errs()<<"# Successors = "<<ti->getNumSuccessors()<<"\n";

    m[lb]->output = new BitVector(meeting);

    bool isChanged = *(m[lb]->output) != Old_Output;

    return isChanged;
  }

  class LivenessAnalysis : public Flow {
    private:
      std::vector<std::string> use_names;

    public:
      LivenessAnalysis(std::vector<std::string> vec, Function& p) : use_names(vec),Flow(false,p,liveness_transfer,liveness_meet,vec.size()){}
  }; 



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
            Value* ins_val =&(*ins);

            for(User::op_iterator OI = ins ->op_begin();OI != ins->op_end();++OI){
              Value* val = *OI;

              if(Instruction* IO = dyn_cast<Instruction>(val)){
                errs()<<"{INS}["<<getShortValueName(val)<<"] is used by ["<<*ins<<"] where LHS: "<<getShortValueName(ins_val)<<"\n";

                unique_names.insert(getShortValueName(val));
              }
              if(Argument* ARG = dyn_cast<Argument>(val)){
                errs()<<"{ARG}["<<getShortValueName(val)<<"] is used by ["<<*ins<<"] where LHS: "<<getShortValueName(ins_val)<<"\n";
                unique_names.insert(getShortValueName(val));
              }

            }
            errs()<<"\n";
          }
        }


        std::vector<std::string> temp;

        for(auto start = unique_names.begin(), end = unique_names.end(); start!=end; ++start){
          errs()<<*start<<"\n";
          temp.push_back(*start);
        }

        names = temp;// get names in global vector

        LivenessAnalysis lv(temp,F);
        lv.analyze();

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
