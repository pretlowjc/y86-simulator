#include "PipeRegArray.h"
#include "ExecuteStage.h"
#include "E.h"
#include "M.h"

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register
					  (F, D, E, M, W instances)
 */
bool ExecuteStage::doClockLow(PipeRegArray *pipeRegs)
{

	PipeReg *ereg = pipeRegs->getExecuteReg();
	PipeReg *mreg = pipeRegs -> getMemoryReg();
	uint64_t stat = ereg->get(E_STAT);
	uint64_t icode = ereg->get(E_ICODE);
	// uint64_t ifun = ereg -> get(E_IFUN);
	uint64_t dstE = ereg->get(E_DSTE);
	uint64_t dstM = ereg->get(E_DSTM);
	uint64_t valC = ereg->get(E_VALC);
	uint64_t valA = ereg->get(E_VALA);
	uint64_t srcB = ereg->get(E_SRCB);
	uint64_t srcA = ereg->get(E_SRCA);
	uint64_t e_cnd = 0;
	// uint64_t numFields = ereg -> get(E_NUMFIELDS);
	setMInput(mreg, stat, icode, dstM, dstE, valC, valA, srcB, srcA, e_cnd);
	return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeRegArray *pipeRegs)
{
	PipeReg *ereg = pipeRegs -> getExecuteReg();
	PipeReg *mreg = pipeRegs -> getMemoryReg();
	ereg -> normal();
	mreg -> normal();

}

void ExecuteStage::setMInput(PipeReg *reg, uint64_t stat, uint64_t icode,
							 uint64_t dstM, uint64_t dstE,
							 uint64_t valC, uint64_t valA, uint64_t srcB, uint64_t srcA, uint64_t e_cnd)
{
	reg->set(M_STAT, stat);
	reg->set(M_ICODE, icode);
	// reg->set(E_IFUN, ifun);
	reg->set(M_VALE, 0);
	reg->set(M_VALA, 0);
	 reg->set(M_CND, e_cnd);
	reg->set(E_VALB, 0);
	reg->set(M_DSTE, dstE);
	reg->set(M_DSTM, dstM);
	// reg->set(M_SRCA, srcA);
	// reg->set(M_SRCB,srcB);
}
