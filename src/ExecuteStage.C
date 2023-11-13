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
	PipeReg *mreg = pipeRegs->getMemoryReg();
	uint64_t stat = ereg->get(E_STAT);
	uint64_t icode = ereg->get(E_ICODE);
	// uint64_t ifun = ereg -> get(E_IFUN);
	uint64_t dstE = ereg->get(E_DSTE);
	uint64_t dstM = ereg->get(E_DSTM);
	uint64_t valE = ereg->get(e_valE);
	uint64_t valA = ereg->get(E_VALA);
	uint64_t e_cnd = 0;
	// uint64_t numFields = ereg -> get(E_NUMFIELDS);
	setMInput(mreg, stat, icode, e_cnd, valE, valA, dstE, dstM);
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
	PipeReg *mreg = pipeRegs->getMemoryReg();
	mreg->normal();
}

void ExecuteStage::setMInput(PipeReg *reg, uint64_t stat, uint64_t icode,
							 uint64_t e_cnd, uint64_t valE,
							 uint64_t valA, uint64_t dstE, uint64_t dstM)
{
	reg->set(M_STAT, stat);
	reg->set(M_ICODE, icode);

	// Add a statement to set e_valE to E_valC. The value of e_valE is then stored in M_valE.
	reg->set(e_valE, E_VALC);
	reg->set(M_VALE, e_valE);

	reg->set(M_VALA, 0);
	reg->set(M_CND, e_cnd);
	reg->set(M_DSTE, dstE);
	reg->set(M_DSTM, dstM);
}
