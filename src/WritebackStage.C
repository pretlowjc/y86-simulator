#include "PipeRegArray.h"
#include "WritebackStage.h"
#include "W.h"
#include "Status.h"

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register 
                      (F, D, E, M, W instances)
 */
bool WritebackStage::doClockLow(PipeRegArray * pipeRegs)
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
void WritebackStage::doClockHigh(PipeRegArray * pipeRegs)
{

}



