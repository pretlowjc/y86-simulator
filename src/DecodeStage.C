#include "PipeRegArray.h"
#include "DecodeStage.h"
#include "D.h"
#include "F.h"
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
bool DecodeStage::doClockLow(PipeRegArray *pipeRegs)
{
	PipeReg *dreg = pipeRegs->getDecodeReg();
	PipeReg *ereg = pipeRegs->getExecuteReg();

	uint64_t stat = dreg->get(D_STAT);
	uint64_t icode = dreg->get(D_ICODE);
	uint64_t ifun = dreg->get(D_IFUN);
	uint64_t rA = dreg->get(D_RA);
	uint64_t rB = dreg->get(D_RB);
	uint64_t valC = dreg->get(D_VALC);
	uint64_t valA = 0;
	uint64_t valB = 0;
	uint64_t dstE = RegisterFile::RNONE;
	uint64_t dstM = RegisterFile::RNONE;
	uint64_t srcA = RegisterFile::RNONE;
	uint64_t srcB = RegisterFile::RNONE;

	// During compilation, numFields is an unused variable.
	// I don't we need it... -Justin
	// uint64_t numFields = dreg -> get(D_NUMFIELDS);

	// maybe pass numfields.
	setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB);

	return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeRegArray *pipeRegs)
{
	PipeReg *ereg = pipeRegs -> getExecuteReg();

	ereg -> normal();

}

void DecodeStage::setEInput(PipeReg *ereg, uint64_t stat, uint64_t icode,
							uint64_t ifun, uint64_t valC, uint64_t valA,
							uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB)
{
	ereg->set(E_STAT, stat);
	ereg->set(E_ICODE, icode);
	ereg->set(E_IFUN, ifun);
	ereg->set(E_VALC, valC);
	ereg->set(E_VALA, valA );
	ereg->set(E_VALB, valB);
	ereg->set(E_DSTE, dstE);
	ereg->set(E_DSTM, dstM);
	ereg->set(E_SRCA, srcA);
	ereg->set(E_SRCB, srcB);
}
