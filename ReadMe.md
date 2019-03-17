Program Analysis @ LUMS

Assignment 1:
Abdul Rafae Noor
Momina Nofal


To build, configure Makefile to appropriate LLVM Version (We used LLVM-4.0)

DataFlow.cpp/h provide a general framework to make any dataflow analysis provided we provide the 
transfer functions and meet operators. This generic framework makes use of function pointers and the 
BBInfo class which allows us to maintain arbritrary state across basic blocks. A map from BasicBlocks
to their corresponding BBInfo seperates state from the frame-work all together. 

As a result, according to the needs of the analysis the BBInfo class can be modified and the function 
pointers would be able to make use of the state.


A base class called "Flow" is implemented which takes the transfer and meet operators and contains a 
bool for direction isFwd. Then to run analysis, the analyze function checks the direction and runs the
analysis accordingly. A queue structure is used to traverse level by level. We either insert successors
or predecessors according to the analysis direction and just pop from queue until size is 0. We also
need to see if a basic block is already inserted in the queue, so we shouldn't insert it again (even if it
has been popped before re-insertion in the same queue).


Both Analysis have a global variable called Verbose to level the debug info displayed during analysis. If set to
true, it would output detailed information at each program point. But for just regular output, it can be set to false.



The analysis implemented are Available Expressions (Forward) analysis, and Liveness (Backward) Analysis. Both have their own
classes which inherit from "Flow". Both also have appropriate meet and transfer functions according to their analysis. Note:
due to structuing of the map from BasicBlock to BBInfo, any analysis implemented in the future would have to follow the syntax
for function pointers indiciated in the dataflow.h file. 
