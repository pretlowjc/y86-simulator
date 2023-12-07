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

	uint64_t valA = 0;
	uint64_t valB = 0;

	uint64_t dstE = RegisterFile::RNONE;
	uint64_t dstM = RegisterFile::RNONE;

	uint64_t rA = dreg->get(D_RA);
	uint64_t rB = dreg->get(D_RB);

	d_srcA = setSrcA(icode, rA);
	d_srcB = setSrcB(icode, rB);

	dstM = setDstM(icode, rA);
	dstE = setDstE(icode, rB);

	valA = SelFwdA(icode, dreg, wreg, mreg, d_srcA);
	valB = FwdB(wreg, mreg, d_srcB);

	calculateControlSignals(ereg);
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
	if (E_bubble)
		((E *)ereg)->bubble();
	else
		ereg->normal();
}

/** setEInput
 * provides the input to potentially be stored in the E register
 * during doClockHigh
 *
 * @param ereg pointer to the E register instance
 * @param stat - value to be stored in the stat pipeline register within E
 * @param icode - value to be stored in the icode pipeline register within E
 * @param ifun - value to be stored in the ifun pipeline register within E
 * @param rA - value to be stored in the rA pipeline register within E
 * @param rB - value to be stored in the rB pipeline register within E
 * @param valC - value to be stored in the valC pipeline register within E
 * @param valP - value to be stored in the valP pipeline register within E
 */
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

/**
 * setSrcA
 * This method is setting what srcA should be during the D stage.
 * @param D_icode value to be passed from d stage's icode.
 * @param D_rA value from rA pipeline register.
 * @return Return the value to be set in d_srcA.
 */
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

/**
 * setSrcB
 * This method is setting what srcB should be during the D stage.
 * @param D_icode value to be passed from d stage's icode.
 * @param D_rB value from rB pipeline register.
 * @return Return the value to be set in srcB.
 */
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

/**
 * setDstE
 * this method is set dstE based on what icode is.
 * @param D_icode value passed from the d stage's icode
 * @param D_rB value from the rB pipeline register.
 * @return returns what dstE should be set to.
 */
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

/**
 * setDstE
 * this method is set dstM based on what icode is.
 * @param D_icode value passed from the d stage's icode
 * @param D_rA value from the rA pipeline register.
 * @return returns what dstM should be set to.
 */
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

/**
 * SelFwdA
 * this method is forwarding a value to srcA.
 * @param D_icode value from d stage's icode.
 * @param dreg pointer to the d register
 * @param wreg pointer to the w register
 * @param mreg pointer to the m register
 * @param d_srcA value from d stage's srcA
 * @return returns the correct value to be forwarded to srcA.
 */
uint64_t DecodeStage::SelFwdA(uint64_t D_icode, PipeReg *dreg, PipeReg *wreg, PipeReg *mreg, uint64_t d_srcA)
{
	bool hasError = false;
	uint64_t d_rvalA = 0;
	uint64_t D_valP = dreg->get(D_VALP);
	uint64_t M_valE = mreg->get(M_VALE);
	uint64_t W_valM = wreg->get(W_VALM);
	uint64_t W_valE = wreg->get(W_VALE);

	if (D_icode == Instruction::ICALL || D_icode == Instruction::IJXX)
	{
		return D_valP;
	}
	if (d_srcA == RegisterFile::RNONE)
	{
		return 0;
	}
	if (d_srcA == Stage::e_dstE)
	{
		return Stage::e_valE;
	}
	if (d_srcA == mreg->get(M_DSTM))
	{
		return Stage::m_valM;
	}
	if (d_srcA == mreg->get(M_DSTE))
	{
		return M_valE;
	}
	if (d_srcA == wreg->get(W_DSTM))
	{
		return W_valM;
	}
	if (d_srcA == wreg->get(W_DSTE))
	{
		return W_valE;
	}

	return d_rvalA = rf->readRegister(d_srcA, hasError); //-- current option.
}

/**
 * FwdB
 * this method is forwarding a value to srcB
 * @param wreg pointer to w register
 * @param mreg pointer to m register
 * @param d_srcB value from d stage's srcB
 * @return the correct value to be forwarded to srcB.
 */
uint64_t DecodeStage::FwdB(PipeReg *wreg, PipeReg *mreg, uint64_t d_srcB)
{
	bool hasError = false;
	uint64_t d_rvalB = 0;

	if (d_srcB == RegisterFile::RNONE)
	{
		return 0;
	}
	if (d_srcB == Stage::e_dstE)
	{
		return Stage::e_valE;
	}
	if (d_srcB == mreg->get(M_DSTM))
	{
		return Stage::m_valM;
	}
	if (d_srcB == mreg->get(M_DSTE))
	{
		return mreg->get(M_VALE);
	}
	if (d_srcB == wreg->get(W_DSTM))
	{
		return wreg->get(W_VALM);
	}
	if (d_srcB == wreg->get(W_DSTE))
	{
		return wreg->get(W_VALE);
	}

	return d_rvalB = rf->readRegister(d_srcB, hasError);
}

/**
 * calculateControlSignals
 * This method is to handle control hazards, telling us whether to bubble or not.
 * @param ereg pointer to the e register.
 */
void DecodeStage::calculateControlSignals(PipeReg *ereg)
{
	uint64_t E_icode = ereg->get(E_ICODE);
	uint64_t E_dstM = ereg->get(E_DSTM);
	E_bubble = ((E_icode == Instruction::IJXX && !e_Cnd) ||
				((E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) &&
				 (E_dstM == d_srcA || E_dstM == d_srcB)));
}