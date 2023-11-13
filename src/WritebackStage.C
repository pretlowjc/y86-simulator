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
   // Use the RegisterFile::writeRegister method to write the value of W_valE to R[W_dstE].
   bool error;
   PipeReg *wreg = pipeRegs->getWritebackReg();
   uint64_t value = wreg->get(W_VALE);
   RegisterFile::writeRegister(value, W_DSTE, error);
}
