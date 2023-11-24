#include "PipeRegArray.h"
#include "Stage.h"

#ifndef EXECUTESTAGE_H
#define EXECUTESTAGE_H
class ExecuteStage : public Stage
{
private:
   // TODO: provide declarations for new methods
   void setMInput(PipeReg *reg, uint64_t stat, uint64_t icode,
                  uint64_t e_cnd, uint64_t valE,
                  uint64_t valA, uint64_t dstE, uint64_t dstM);
   uint64_t aluA(uint64_t e_icode, uint64_t valE, uint64_t valA);
   uint64_t aluB(uint64_t e_icode, uint64_t valB);
   uint64_t alufun(uint64_t e_icode, uint64_t ifun);
   bool set_cc(uint64_t e_icode);
   uint64_t e_dstE(uint64_t e_icode, uint64_t dstE);
   uint64_t cc(uint64_t value, bool overflow, uint64_t aluA, uint64_t aluB);
   uint64_t alu(uint64_t alufun, uint64_t aluA, uint64_t aluB, bool set_cc);

public:
   // These are the only methods called outside of the class
   bool doClockLow(PipeRegArray *pipeRegs);
   void doClockHigh(PipeRegArray *pipeRegs);
};
#endif
