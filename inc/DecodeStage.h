#include "PipeRegArray.h"
#include "Stage.h"

#ifndef DECODESTAGE_H
#define DECODESTAGE_H
class DecodeStage : public Stage
{
private:
   // TODO: provide declarations for new methods
   void setEInput(PipeReg *ereg, uint64_t stat, uint64_t icode, uint64_t ifun,
                  uint64_t valC, uint64_t valA,
                  uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB);
   uint64_t setSrcA(uint64_t D_icode, uint64_t D_rA);
   uint64_t setSrcB(uint64_t D_icode, uint64_t D_rB);
   uint64_t setDstE(uint64_t D_icode, uint64_t D_rB);
   uint64_t setDstM(uint64_t D_icode, uint64_t D_rA);
   uint64_t SelFwdA(PipeReg *wreg, PipeReg *mreg,uint64_t d_srcA);
   uint64_t FwdB(PipeReg *wreg, PipeReg *mreg, uint64_t d_srcB);

public:
   // These are the only methods called outside of the class
   bool doClockLow(PipeRegArray *pipeRegs);
   void doClockHigh(PipeRegArray *pipeRegs);
};
#endif
