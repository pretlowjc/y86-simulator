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

	// call aluA method.
	uint64_t e_aluA = aluA(icode, valA, valC);

	// call aluB method.
	uint64_t e_aluB = aluB(icode, valB);

	// call alufun method.
	uint64_t e_alufun = alufun(icode, ifun);

	Stage::e_valE = alu(e_alufun, e_aluA, e_aluB, set_cc(ereg, wreg));
	Stage::e_Cnd = cond(icode, ifun);
	Stage::e_dstE = e_dstE(icode, dstE); // Pass with 27 out of 44. Cycle 15, dstE is f, should be 5.
	// below I call the new method to set M_bubble
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

	else mreg -> normal();
}

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

uint64_t ExecuteStage::aluA(uint64_t e_icode, uint64_t valA, uint64_t valC)
{
	if (e_icode == Instruction::IRRMOVQ || e_icode == Instruction::IOPQ)
	{
		return valA;
	}
	if (e_icode == Instruction::IIRMOVQ || e_icode == Instruction::IRMMOVQ || e_icode == Instruction::IMRMOVQ)
	{
		return valC;
	}
	if (e_icode == Instruction::ICALL || e_icode == Instruction::IPUSHQ)
	{
		return -8;
	}
	if (e_icode == Instruction::IRET || e_icode == Instruction::IPOPQ)
	{
		return 8;
	}
	return 0;
}

uint64_t ExecuteStage::aluB(uint64_t e_icode, uint64_t valB)
{
	if (e_icode == Instruction::IRMMOVQ || e_icode == Instruction::IMRMOVQ || e_icode == Instruction::IOPQ || e_icode == Instruction::ICALL || e_icode == Instruction::IPUSHQ || e_icode == Instruction::IRET || e_icode == Instruction::IPOPQ)
	{
		return valB;
	}
	if (e_icode == Instruction::IRRMOVQ || e_icode == Instruction::IIRMOVQ)
	{
		return 0;
	}
	return 0;
}

uint64_t ExecuteStage::alufun(uint64_t e_icode, uint64_t ifun)
{
	if (e_icode == Instruction::IOPQ)
	{
		return ifun;
	}
	return Instruction::ADDQ;
}

// set_CC main method changed, changed the params, to follow HCL control logic
bool ExecuteStage::set_cc(PipeReg *ereg, PipeReg *wreg)
{
	uint64_t W_stat = wreg->get(W_STAT);
	uint64_t E_icode = ereg->get(E_ICODE);

	return (E_icode == Instruction::IOPQ) && 
	(Stage::m_stat != Status::SADR && Stage::m_stat != Status::SINS && Stage::m_stat != Status::SHLT) && 
	(W_stat != Status::SADR && W_stat != Status::SHLT && W_stat != Status::SINS);
}

uint64_t ExecuteStage::e_dstE(uint64_t e_icode, uint64_t dstE)
{
	if (e_icode == Instruction::IRRMOVQ && !Stage::e_Cnd)
	{
		return RegisterFile::RNONE;
	}
	return dstE;
}

// control signals method used previously.
bool ExecuteStage::calculateControlSignals(PipeReg *wreg)
{

	uint64_t W_stat = wreg->get(W_STAT);

	return m_stat == Status::SADR || m_stat == Status::SINS ||
		   m_stat == Status::SHLT || W_stat == Status::SADR ||
		   W_stat == Status::SINS || W_stat == Status::SHLT;
}

void ExecuteStage::cc(bool zeroflag, bool signflag, bool overflow)
{
	bool error;
	ConditionCodes *condcodes = ConditionCodes::getInstance();
	condcodes->setConditionCode(zeroflag, ConditionCodes::ZF, error);
	condcodes->setConditionCode(signflag, ConditionCodes::SF, error);
	condcodes->setConditionCode(overflow, ConditionCodes::OF, error);
}

uint64_t ExecuteStage::alu(uint64_t alufun, uint64_t aluA, uint64_t aluB, bool set_cc)
{
	/*
		The ALU will be used to perform add, sub, xor, or and depending
		upon the value retured from the ALU fun. control unit.

		See the ExecuteStage diagram to see how all of these fit together.

		You'll need to modify doClockLow to call these functions.

		NOTE: I have already added the other function calls. All that needs
		to be done is cc and alu methods and add their function calls...I think...
	*/
	bool overflow = false;
	uint64_t value;
	// if add we add aluA and aluB
	if (alufun == Instruction::ADDQ)
	{
		overflow = Tools::addOverflow(aluA, aluB);
		value = aluA + aluB;
	}
	// if and we and aluA and aluB
	if (alufun == Instruction::ANDQ)
	{
		value = aluA & aluB;
	}
	// if xor we ^ aluA and aluB
	if (alufun == Instruction::XORQ)
	{
		value = aluA ^ aluB;
	}
	// if sub aluA - aluB
	if (alufun == Instruction::SUBQ)
	{
		overflow = Tools::subOverflow(aluA, aluB);
		value = aluB - aluA;
	}
	// if set_cc component returns true then use cc component to set condition codes.
	if (set_cc)
	{
		bool zeroflag = (value == 0);
		bool signflag = Tools::sign(value);
		cc(zeroflag, signflag, overflow);
	}
	return value;
}

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
