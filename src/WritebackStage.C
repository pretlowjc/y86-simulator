#include "PipeRegArray.h"
#include "WritebackStage.h"
#include "W.h"
#include "Status.h"
#include "RegisterFile.h"

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register
                      (F, D, E, M, W instances)
 */
bool WritebackStage::doClockLow(PipeRegArray *pipeRegs)
{
   // if the stat value out of the W register is NOT equal to Status::SAOK
   // then return true.
   PipeReg *wreg = pipeRegs->getWritebackReg();
   uint64_t stat = wreg->get(W_STAT);

   if (stat != Status::SAOK)
   {
      return true;
   }
   // Otherwise, return false.
   return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
 */
void WritebackStage::doClockHigh(PipeRegArray *pipeRegs)
{
   bool error = false;
   PipeReg *wreg = pipeRegs->getWritebackReg();

   uint64_t value = wreg->get(W_VALE);
   uint64_t dst = wreg->get(W_DSTE);
   rf->writeRegister(value, dst, error);

   uint64_t valM = wreg->get(W_VALM);
   uint64_t dstM = wreg->get(W_DSTM);
   rf->writeRegister(valM, dstM, error);
}
