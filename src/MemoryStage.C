#include "PipeRegArray.h"
#include "MemoryStage.h"
#include "W.h"
#include "M.h"
#include "E.h"

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register
					  (F, D, E, M, W instances)
 */
bool MemoryStage::doClockLow(PipeRegArray *pipeRegs)
{
	// Some of this is possibly wrong. - Justin
	
	PipeReg *mreg = pipeRegs->getExecuteReg();
	uint64_t stat = mreg->get(M_STAT);
	uint64_t icode = mreg->get(M_ICODE);
	uint64_t dstM = mreg->get(M_DSTM);
	uint64_t dstE = mreg->get(M_DSTE);
	uint64_t valC = mreg->get(E_VALC);
	uint64_t valA = mreg->get(M_VALA);
	// uint64_t srcB = mreg -> get(E_SRCB);
	// uint64_t srcA = mreg -> get(E_SRCA);
	// // uint64_t numFields = mreg -> get(E_NUMFIELDS);
	setWInput(mreg, stat, icode, valC, valA, dstE, dstM);

	return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeRegArray *pipeRegs)
{
	PipeReg *mreg = pipeRegs -> getMemoryReg();
	PipeReg *wreg = pipeRegs -> getWritebackReg();
	mreg -> normal();
	wreg -> normal();
}

void setWInput(PipeReg *reg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, uint64_t dstE, uint64_t dstM)
{
	// probably some things missing but these are my initial thoughts... -Justin

	reg->set(W_STAT, stat);
	reg->set(W_ICODE, icode);
	reg->set(W_VALE, valE);
	reg->set(W_VALM, valM);
	reg->set(W_DSTE, dstE);
	reg->set(W_DSTM, dstM);
}
