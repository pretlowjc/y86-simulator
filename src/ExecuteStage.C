#include "PipeRegArray.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "E.h"
#include "M.h"
#include "Tools.h"
#include "Instruction.h"
#include "ConditionCodes.h"

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
	uint64_t dstE = ereg->get(E_DSTE);
	uint64_t dstM = ereg->get(E_DSTM);
	uint64_t valA = ereg->get(E_VALA);
	uint64_t valB = ereg->get(E_VALB);
	uint64_t ifun = ereg->get(E_IFUN);
	uint64_t valC = ereg->get(E_VALC);

	uint64_t e_cnd = 0;

	// call aluA method.
	uint64_t e_aluA = aluA(icode, valA, valC);

	// call aluB method.
	uint64_t e_aluB = aluB(icode, valB);

	// call alufun method.
	uint64_t e_alufun = alufun(icode, ifun);

	// call ALU method.
	Stage::e_valE = alu(e_alufun, e_aluA, e_aluB, set_cc(icode));
	Stage::e_dstE = dstE;

	setMInput(mreg, stat, icode, e_cnd, Stage::e_valE, valA, dstE, dstM);
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
	reg->set(M_VALE, valE);
	reg->set(M_VALA, valA);
	reg->set(M_CND, e_cnd);
	reg->set(M_DSTE, dstE);
	reg->set(M_DSTM, dstM);
}

// would returning -8 cause issues since I am using an uint64_t??
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

bool ExecuteStage::set_cc(uint64_t e_icode)
{
	return (e_icode == Instruction::IOPQ);
}

uint64_t ExecuteStage::e_dstE(uint64_t e_icode, uint64_t dstE)
{
	if (e_icode == Instruction::IRRMOVQ && !Stage::e_Cnd)
	{
		return RegisterFile::RNONE;
	}
	return dstE;
}

uint64_t ExecuteStage::cc(bool zeroflag, bool signflag, bool overflow)
{
	/*
		If the set_cc component returns true then the CC component
		will be used to set the condition codes (in the ConditionCodes)
		class) to 0 or 1.
	*/
	bool error;

	// set condition codes to 0 or 1 using the ConditionCodes class.
	ConditionCodes *condcodes = ConditionCodes::getInstance();
	condcodes->setConditionCode(zeroflag, ConditionCodes::ZF, error);
	condcodes->setConditionCode(signflag, ConditionCodes::SF, error);
	condcodes->setConditionCode(overflow, ConditionCodes::OF, error);
	// set zero flag if zero?
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
