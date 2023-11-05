#include "PipeRegArray.h"
#include "Stage.h"

#ifndef MEMORYSTAGE_H
#define MEMORYSTAGE_H
class MemoryStage: public Stage
{
   private:
      //TODO: provide declarations for new methods
	  void setWInput(PipeReg *wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, uint64_t dstE, uint32_t dstM);
   public:
      //These are the only methods called outside of the class
      bool doClockLow(PipeRegArray * pipeRegs);
      void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
