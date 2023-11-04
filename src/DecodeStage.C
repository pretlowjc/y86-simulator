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

	uint64_t stat = dreg->get(D_STAT);
	uint64_t icode = dreg->get(D_ICODE);
	uint64_t ifun = dreg->get(D_IFUN);
	uint64_t rA = dreg->get(D_RA);
	uint64_t rB = dreg->get(D_RB);
	uint64_t valC = dreg->get(D_VALC);
	uint64_t valP = dreg->get(D_VALP);

	// During compilation, numFields is an unused variable.
	// I don't we need it... -Justin
	// uint64_t numFields = dreg -> get(D_NUMFIELDS);

	// maybe pass numfields.
	setEInput(dreg, stat, icode, ifun, rA, rB, valC, valP);

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
	PipeRegField(2).normal();
}

void DecodeStage::setEInput(PipeReg *dreg, uint64_t stat, uint64_t icode,
							uint64_t ifun, uint64_t rA, uint64_t rB,
							uint64_t valC, uint64_t valP)
{
	dreg->set(E_STAT, stat);
	dreg->set(E_ICODE, icode);
	dreg->set(E_IFUN, ifun);
	dreg->set(D_VALC, valC);
	dreg->set(D_VALP, valP);
	dreg->set(E_VALA, 0);
	dreg->set(E_VALB, 0);
	dreg->set(E_DSTE, RegisterFile::RNONE);
	dreg->set(E_DSTM, RegisterFile::RNONE);
	dreg->set(E_SRCA, rA);
	dreg->set(E_SRCB, rB);
}
