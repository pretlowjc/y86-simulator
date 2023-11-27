#include "PipeRegArray.h"
#include "Stage.h"
#include "StageArray.h"
#include "DecodeStage.h"
#include "D.h"
#include "F.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Instruction.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"

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
	PipeReg *mreg = pipeRegs->getMemoryReg();
	PipeReg *wreg = pipeRegs->getWritebackReg();

	uint64_t stat = dreg->get(D_STAT);
	uint64_t icode = dreg->get(D_ICODE);
	int64_t ifun = dreg->get(D_IFUN);
	uint64_t valC = dreg->get(D_VALC);
	uint64_t valA = 0; // ??
	uint64_t valB = 0; // ??
	uint64_t dstE = RegisterFile::RNONE;
	uint64_t dstM = RegisterFile::RNONE;
	uint64_t srcA = RegisterFile::RNONE;
	uint64_t srcB = RegisterFile::RNONE;

	uint64_t rA = dreg->get(D_RA);
	uint64_t rB = dreg->get(D_RB);

	d_srcA = setSrcA(icode, rA);
	d_srcB = setSrcB(icode, rB);

	dstM = setDstM(icode, rA);
	dstE = setDstE(icode, rB);

	uint64_t d_rvalA = 0;
	uint64_t d_rvalB = 0;

	valA = SelFwdA(wreg, mreg, d_srcA);
	valB = FwdB(wreg, mreg, d_srcB);

	setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, d_srcA, d_srcB);

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
	PipeReg *ereg = pipeRegs->getExecuteReg();

	ereg->normal();
}

void DecodeStage::setEInput(PipeReg *ereg, uint64_t stat, uint64_t icode,
							uint64_t ifun, uint64_t valC, uint64_t valA,
							uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB)
{
	ereg->set(E_STAT, stat);
	ereg->set(E_ICODE, icode);
	ereg->set(E_IFUN, ifun);
	ereg->set(E_VALC, valC);
	ereg->set(E_VALA, valA);
	ereg->set(E_VALB, valB);
	ereg->set(E_DSTE, dstE);
	ereg->set(E_DSTM, dstM);
	ereg->set(E_SRCA, srcA);
	ereg->set(E_SRCB, srcB);
}

uint64_t DecodeStage::setSrcA(uint64_t D_icode, uint64_t D_rA)
{
	switch (D_icode)
	{
	case Instruction::IRRMOVQ:
	case Instruction::IRMMOVQ:
	case Instruction::IOPQ:
	case Instruction::IPUSHQ:
		return D_rA;
	case Instruction::IPOPQ:
	case Instruction::IRET:
		return RegisterFile::rsp;
	default:
		return RegisterFile::RNONE;
	}
}

uint64_t DecodeStage::setSrcB(uint64_t D_icode, uint64_t D_rB)
{
	switch (D_icode)
	{
	case Instruction::IOPQ:
	case Instruction::IRMMOVQ:
	case Instruction::IMRMOVQ:
		return D_rB;
	case Instruction::IPUSHQ:
	case Instruction::IPOPQ:
	case Instruction::ICALL:
	case Instruction::IRET:
		return RegisterFile::rsp;
	default:
		return RegisterFile::RNONE;
	}
}

uint64_t DecodeStage::setDstE(uint64_t D_icode, uint64_t D_rB)
{
	switch (D_icode)
	{
	case Instruction::IRRMOVQ:
	case Instruction::IIRMOVQ:
	case Instruction::IOPQ:
		return D_rB;
	case Instruction::IPUSHQ:
	case Instruction::IPOPQ:
	case Instruction::ICALL:
	case Instruction::IRET:
		return RegisterFile::rsp;
	default:
		return RegisterFile::RNONE;
	}
}

uint64_t DecodeStage::setDstM(uint64_t D_icode, uint64_t D_rA)
{
	switch (D_icode)
	{
	case Instruction::IMRMOVQ:
	case Instruction::IPOPQ:
		return D_rA;
	default:
		return RegisterFile::RNONE;
	}
}

uint64_t DecodeStage::SelFwdA(PipeReg *wreg, PipeReg *mreg, uint64_t d_srcA)
{
	bool hasError = false;
	uint64_t d_rvalA = 0;

	if (d_srcA == Stage::e_dstE)
	{
		return Stage::e_valE;
	}
	if (d_srcA == mreg->get(M_DSTE))
	{
		uint64_t M_valE = mreg->get(M_VALE);
		return M_valE;
	}
	if (d_srcA == wreg->get(W_DSTE))
	{
		uint64_t W_valE = wreg->get(W_VALE);
		return W_valE;
	}

	return d_rvalA = rf->readRegister(d_srcA, hasError); //-- current option.
}

uint64_t DecodeStage::FwdB(PipeReg *wreg, PipeReg *mreg, uint64_t d_srcB)
{
	bool hasError = false;
	uint64_t d_rvalB = 0;

	if (d_srcB == Stage::e_dstE)
		return Stage::e_valE;
	if (d_srcB == mreg->get(M_DSTE))
		return mreg->get(M_VALE);
	if (d_srcB == wreg->get(W_DSTE))
		return wreg->get(W_VALE);

	return d_rvalB = rf->readRegister(d_srcB, hasError);
}