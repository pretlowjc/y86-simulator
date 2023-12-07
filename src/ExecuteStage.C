#include "PipeRegArray.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Tools.h"
#include "Instruction.h"
#include "ConditionCodes.h"
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
bool ExecuteStage::doClockLow(PipeRegArray *pipeRegs)
{
	PipeReg *ereg = pipeRegs->getExecuteReg();
	PipeReg *mreg = pipeRegs->getMemoryReg();
	PipeReg *wreg = pipeRegs->getWritebackReg();
	uint64_t stat = ereg->get(E_STAT);
	uint64_t icode = ereg->get(E_ICODE);
	uint64_t dstE = ereg->get(E_DSTE);
	uint64_t dstM = ereg->get(E_DSTM);
	uint64_t valA = ereg->get(E_VALA);
	uint64_t valB = ereg->get(E_VALB);
	uint64_t ifun = ereg->get(E_IFUN);
	uint64_t valC = ereg->get(E_VALC);

	uint64_t e_aluA = aluA(icode, valA, valC);
	uint64_t e_aluB = aluB(icode, valB);
	uint64_t e_alufun = alufun(icode, ifun);

	Stage::e_valE = alu(e_alufun, e_aluA, e_aluB, set_cc(ereg, wreg));
	Stage::e_Cnd = cond(icode, ifun);
	Stage::e_dstE = e_dstE(icode, dstE);

	M_bubble = calculateControlSignals(wreg);

	setMInput(mreg, stat, icode, Stage::e_Cnd, Stage::e_valE, valA, Stage::e_dstE, dstM);

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
	// doclockhigh modifcation
	if (M_bubble)
		((M *)mreg)->bubble();

	else
		mreg->normal();
}

/** setMInput
 * provides the input to potentially be stored in the M register
 * during doClockHigh
 *
 * @param reg - pointer to the register instance
 * @param stat - value to be stored in the stat pipeline register within M
 * @param icode - value to be stored in the icode pipeline register within M
 * @param e_cnd - value to be stored in the cnd pipeline register within M
 * @param valE - value to be stored in valE pipeline register within M
 * @param valA - value to be stored in valA pipeline register within M
 * @param dstE - value to be stored in dstE pipeline register within M
 * @param dstM - value to be stored in dstM pipeline register within M
 */
void ExecuteStage::setMInput(PipeReg *reg, uint64_t stat, uint64_t icode,
							 uint64_t e_cnd, uint64_t valE,
							 uint64_t valA, uint64_t dstE, uint64_t dstM)
{
	reg->set(M_STAT, stat);
	reg->set(M_ICODE, icode);
	reg->set(M_VALE, valE);
	reg->set(M_VALA, valA);
	reg->set(M_CND, e_cnd);
	reg->set(M_DSTE, dstE);
	reg->set(M_DSTM, dstM);
}

/**
 * aluA
 * preparing value to be passed to alu for alu operation.
 * @param e_icode - value from e stage's icode
 * @param valA - value of valA to be passed depending on instruction.
 * @param valC - value of valC to be passed depending on instruction.
 * @return Returns the value to be passed to the alu for operation depending upon what
 * intruction icode is.
 */
uint64_t ExecuteStage::aluA(uint64_t e_icode, uint64_t valA, uint64_t valC)
{
	switch (e_icode)
	{
	case Instruction::IRRMOVQ:
	case Instruction::IOPQ:
		return valA;
	case Instruction::IIRMOVQ:
	case Instruction::IRMMOVQ:
	case Instruction::IMRMOVQ:
		return valC;
	case Instruction::ICALL:
	case Instruction::IPUSHQ:
		return -8;
	case Instruction::IRET:
	case Instruction::IPOPQ:
		return 8;
	default:
		return 0;
	}
}

/**
 * aluB
 * preparing value to be passed to alu for alu operation.
 * @param e_icode - value from e stage's icode
 * @param valB - value of valB to be passed depending on instruction.
 * @return Returns the value to be passed to the alu for operation depending upon what
 * intruction icode is.
 */
uint64_t ExecuteStage::aluB(uint64_t e_icode, uint64_t valB)
{
	switch (e_icode)
	{
	case Instruction::IRMMOVQ:
	case Instruction::IMRMOVQ:
	case Instruction::IOPQ:
	case Instruction::ICALL:
	case Instruction::IPUSHQ:
	case Instruction::IRET:
	case Instruction::IPOPQ:
		return valB;
	case Instruction::IRRMOVQ:
	case Instruction::IIRMOVQ:
		return 0;
	default:
		return 0;
	}
}

/**
 * alufun
 * takes in icode and ifun and sends the correct fun to the alu for operation.
 * @param e_icode - value from e stage's icode
 * @param ifun - value from e stage's ifun
 * @return - returns the correct fun to be passed to the alu operation.
 */
uint64_t ExecuteStage::alufun(uint64_t e_icode, uint64_t ifun)
{
	switch (e_icode)
	{
	case Instruction::IOPQ:
		return ifun;
	default:
		return Instruction::ADDQ;
	}
}

/**
 * set_cc
 * this method is telling us if the condition codes need to be set based on the
 * values passed from the e and w register pipelines.
 * @param ereg - pointer to the e register
 * @param wreg - pointer to the w register
 * @return - returns a bool value of whether or not the condition codes need to be set.
 */
bool ExecuteStage::set_cc(PipeReg *ereg, PipeReg *wreg)
{
	uint64_t W_stat = wreg->get(W_STAT);
	uint64_t E_icode = ereg->get(E_ICODE);

	return (E_icode == Instruction::IOPQ) &&
		   (Stage::m_stat != Status::SADR && Stage::m_stat != Status::SINS && Stage::m_stat != Status::SHLT) &&
		   (W_stat != Status::SADR && W_stat != Status::SHLT && W_stat != Status::SINS);
}

/**
 * e_dstE
 * this method is setting the value dstE during the e stage.
 * @param e_icode - value to be passed from the e stage's icode.
 * @param dstE - value to be passed from e stage's dstE.
 * @return - returns the value to be passed to dstE for setEInput.
 */
uint64_t ExecuteStage::e_dstE(uint64_t e_icode, uint64_t dstE)
{
	if (e_icode == Instruction::IRRMOVQ && !Stage::e_Cnd)
	{
		return RegisterFile::RNONE;
	}
	return dstE;
}

/**
 * calculateControlSignals
 * this method is preventing control hazards.
 * @param wreg - pointer to the w register.
 * @return - returns a boolean value depending on certain status.
 */
bool ExecuteStage::calculateControlSignals(PipeReg *wreg)
{
	uint64_t W_stat = wreg->get(W_STAT);

	return m_stat == Status::SADR || m_stat == Status::SINS ||
		   m_stat == Status::SHLT || W_stat == Status::SADR ||
		   W_stat == Status::SINS || W_stat == Status::SHLT;
}

/**
 * cc
 * this method is setting the condition codes
 * @param zeroflag - value of what zf should be
 * @param signflag - value of what sf should be
 * @param overflow - value of what of should be
 */
void ExecuteStage::cc(bool zeroflag, bool signflag, bool overflow)
{
	bool error;
	ConditionCodes *condcodes = ConditionCodes::getInstance();
	condcodes->setConditionCode(zeroflag, ConditionCodes::ZF, error);
	condcodes->setConditionCode(signflag, ConditionCodes::SF, error);
	condcodes->setConditionCode(overflow, ConditionCodes::OF, error);
}

/**
 * alu
 * The ALU will be used to perform add, sub, xor, or and depending upon the value
 * retured from the ALU fun. control unit.
 * @param alufun - fun value to be passed for operation
 * @param aluA - value of correct srcA to be used.
 * @param aluB - value of correct srcB to be used.
 * @param set_cc - value of whether or not to set condition codes.
 * @return - returns value from operation executed and the correct flags to be set.
 */
uint64_t ExecuteStage::alu(uint64_t alufun, uint64_t aluA, uint64_t aluB, bool set_cc)
{
	bool overflow = false;
	uint64_t value;

	if (alufun == Instruction::ADDQ)
	{
		overflow = Tools::addOverflow(aluA, aluB);
		value = aluA + aluB;
	}
	if (alufun == Instruction::ANDQ)
	{
		value = aluA & aluB;
	}
	if (alufun == Instruction::XORQ)
	{
		value = aluA ^ aluB;
	}
	if (alufun == Instruction::SUBQ)
	{
		overflow = Tools::subOverflow(aluA, aluB);
		value = aluB - aluA;
	}
	if (set_cc)
	{
		bool zeroflag = (value == 0);
		bool signflag = Tools::sign(value);
		cc(zeroflag, signflag, overflow);
	}
	return value;
}

/**
 * cond
 * this tells us what condition flag is to be set?
 * @param e_icode - icode from e stage to be passed
 * @param ifun - ifun from e stage to be passed.
 * @return returns the value of e_Cnd
 */
uint64_t ExecuteStage::cond(uint64_t e_icode, uint64_t ifun)
{
	bool error = false;

	ConditionCodes *condcodes = ConditionCodes::getInstance();
	uint8_t zf = condcodes->getConditionCode(ConditionCodes::ZF, error);
	uint8_t of = condcodes->getConditionCode(ConditionCodes::OF, error);
	uint8_t sf = condcodes->getConditionCode(ConditionCodes::SF, error);

	if (e_icode == Instruction::IJXX || e_icode == Instruction::ICMOVXX)
	{
		if (ifun == Instruction::UNCOND)
		{
			return 1;
		}
		if (ifun == Instruction::LESSEQ)
		{
			return (sf ^ of) | zf;
		}
		if (ifun == Instruction::LESS)
		{
			return (sf ^ of);
		}
		if (ifun == Instruction::EQUAL)
		{
			return zf;
		}
		if (ifun == Instruction::NOTEQUAL)
		{
			return !zf;
		}
		if (ifun == Instruction::GREATER)
		{
			return !(sf ^ of) & !zf;
		}
		if (ifun == Instruction::GREATEREQ)
		{
			return !(sf ^ of);
		}
	}
	return 0;
}
