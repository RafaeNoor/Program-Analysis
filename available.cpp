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


  void printBitVector(BitVector bv){
    for(unsigned i =0 ; i < bv.size() ;i++){
      if(bv[i])
        errs()<<"1 ";
      else
        errs()<<"0 ";
    }
    errs()<<"\n";
  }

  std::vector<Expression> globalVec;

  void available_transfer(BasicBlock* b, std::map<BasicBlock*, BBInfo*> &m){
    //errs()<<*b<<"\n";

    /*
    for(unsigned i =0; i<globalVec.size();i++){ // globalVec set according to runOnFunction. Hence available in our global scope
      errs()<<globalVec[i].toString()<<"\n";
    }
    */

    // Transfer Function:
    // Out[b] = gen[b] U (In[B] - Kill[B])
    // 
    
    BitVector In_B(*(m[b]->input)); // In[B]
    BitVector Gen_B(m[b]->input->size(),false);//Gen[B]
    BitVector Kill_B(m[b]->input->size(),false);//Kill[B]
    BitVector Out_B(In_B); //Out[B]


    std::vector<std::string> Kill_Names; // Kill_name

    
    for(BasicBlock::iterator BI = b->begin(), e = b->end(); BI!=e;++BI){
      Instruction* I = &*BI;
      std::vector<Expression> alive_rn;
      errs()<<"\n========\n"<<*I<<"\n";
      Value* val = &(*I);

      if(BinaryOperator * BO = dyn_cast<BinaryOperator>(I)){// If it's an Expression
        Expression expr(I);
        errs()<<"Expression: "<<expr.toString()<<"\n";
      
        
        std::string toKill = getShortValueName(val);
        // Gen_Set processing
        for(int k = 0; k<globalVec.size(); k++){ //TODO: Use find operator to one-line this function
          if(globalVec[k] == expr){
            Gen_B[k] = true; // expression is generated at this point
            errs()<<expr.toString()<<" added to GenSet"<<"\n";
            //errs()<<"LHS: "<<getShortValueName(val)<<"\n"; // need to remove any expression from GenSet which used LHS

            for(int j=0;j<globalVec.size();j++){
              if(j==k)
                continue;

              if(getShortValueName(globalVec[j].v1) == toKill){
                Gen_B[j] = 0;
                errs()<<"Removing from Gen definition: "<<globalVec[j].toString()<<" as "<<toKill<<" == "<<getShortValueName(globalVec[j].v1)<<"\n";
              }


              if(getShortValueName(globalVec[j].v2) == toKill){
                Gen_B[j] = 0;
                errs()<<"Removing from Gen definition: "<<globalVec[j].toString()<<" as "<<toKill<<" == "<<getShortValueName(globalVec[j].v2)<<"\n";
              }
              
            }
          }
        }


        //Kill_Set processing
        for(int k =0;k<globalVec.size();k++){
          if(getShortValueName(globalVec[k].v1) == toKill){
            Kill_B[k] = 1;
            errs()<<"Killing definition: "<<globalVec[k].toString()<<"\n";
          }

          if(getShortValueName(globalVec[k].v2) == toKill){
            Kill_B[k] = 1;
            errs()<<"Killing definition: "<<globalVec[k].toString()<<"\n";

          }
        }
      }
      //Update Out_B after every instructions
      //Out_B = Gen_B | (Out_B & Kill_B.flip()) ; 

      errs()<<"\nGen_B: ";
      printBitVector(Gen_B);

      errs()<<"Kill_B: ";
      printBitVector(Kill_B);

      errs()<<"In_B: ";
      printBitVector(Out_B);


      BitVector Gen_Kill(Out_B); 
      Gen_Kill &= (Kill_B.flip());
      
      BitVector OR_Gen(Gen_B);
      OR_Gen |= Gen_Kill;

      Out_B = OR_Gen;

      Kill_B.reset();
      Gen_B.reset();

      for(unsigned p = 0; p< Out_B.size(); p++){
        if(Out_B[p] == true){
          alive_rn.push_back(globalVec[p]);
          errs()<<"1 ";
        } else {
          errs()<<"0 ";
        }
      }

      printSet(&alive_rn);
      
    }

    //Dummy updating of out[B]
    m[b]->output =  new BitVector(Out_B);

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

    errs()<<"=================================================================\n# Predeccesors = "<<numPreds<<"\n";

    
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

        globalVec = expressions;

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
