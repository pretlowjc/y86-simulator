#include "PipeRegArray.h"
#include "Stage.h"

#ifndef EXECUTESTAGE_H
#define EXECUTESTAGE_H
class ExecuteStage: public Stage
{
   private:
      //TODO: provide declarations for new methods
	  void setMInput(PipeReg * reg, uint64_t stat, uint64_t icode,
                     uint64_t dstM, uint64_t dstE,
                     uint64_t valC, uint64_t valA, uint64_t srcB, uint64_t srcA, uint64_t e_cnd);
   public:
      //These are the only methods called outside of the class
      bool doClockLow(PipeRegArray * pipeRegs);
      void doClockHigh(PipeRegArray * pipeRegs);
	  
};
#endif
