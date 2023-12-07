// TODO add more #includes as you need them
#include <cstdint>
#include "PipeRegArray.h"
#include "PipeReg.h"
#include "Memory.h"
#include "FetchStage.h"
#include "Instruction.h"
#include "RegisterFile.h"
#include "Status.h"
#include "Stage.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h"
#include "Tools.h"
#include <iostream>

using namespace std;
/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pipeRegs - array of the pipeline register
					  (F, D, E, M, W instances)
 */
bool FetchStage::doClockLow(PipeRegArray *pipeRegs)
{
	PipeReg *freg = pipeRegs->getFetchReg();
	PipeReg *dreg = pipeRegs->getDecodeReg();
	bool mem_error = false;
	uint64_t icode = Instruction::INOP, ifun = Instruction::FNONE;
	uint64_t rA = RegisterFile::RNONE, rB = RegisterFile::RNONE;
	uint64_t valC = 0, valP = 0, stat = 0, predPC = 0;
	bool needvalC = false;
	bool needregId = false;
	PipeReg *wreg = pipeRegs->getWritebackReg();
	PipeReg *mreg = pipeRegs->getMemoryReg();
	PipeReg *ereg = pipeRegs->getExecuteReg();

	uint64_t f_pc = selectPC(freg, mreg, wreg);
	uint64_t byte = mem->getByte(f_pc, mem_error);

	icode = f_icode(mem_error, Tools::getBits(byte, 4, 7));
	stat = f_stat(icode, mem_error);
	ifun = f_ifun(mem_error, Tools::getBits(byte, 0, 3));

	if (needRegIds(icode))
	{
		getRegIds(f_pc, &rA, &rB);
	}
	if (needValC(icode))
	{
		valC = buildValC(f_pc, needRegIds(icode));
	}
	needvalC = needValC(icode);
	needregId = needRegIds(icode);

	valP = PCincrement(f_pc, needregId, needvalC);

	predPC = predictPC(icode, valC, valP);
	calculateControlSignals(ereg, dreg, mreg);
	freg->set(F_PREDPC, predPC);
	setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
	return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeRegArray *pipeRegs)
{
	PipeReg *freg = pipeRegs->getFetchReg();
	PipeReg *dreg = pipeRegs->getDecodeReg();
	if (!F_stall)
	{
		freg->normal();
	}
	if (D_bubble)
	{
		((D *)dreg)->bubble();
	}
	else if (!D_stall)
	{
		dreg->normal();
	}
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
 */
void FetchStage::setDInput(PipeReg *dreg, uint64_t stat, uint64_t icode,
						   uint64_t ifun, uint64_t rA, uint64_t rB,
						   uint64_t valC, uint64_t valP)
{
	dreg->set(D_STAT, stat);
	dreg->set(D_ICODE, icode);
	dreg->set(D_IFUN, ifun);
	dreg->set(D_RA, rA);
	dreg->set(D_RB, rB);
	dreg->set(D_VALC, valC);
	dreg->set(D_VALP, valP);
}

/**
 * selectPC
 * Chooses the value to be used as the PC to fetch the next instruction
 * out of memory.
 * @param freg Pointer to the F register instance.
 * @param mreg Pointer to the M register instance.
 * @param wreg Pointer to the W register instance.
 * @return Returns the predicted pc.
 */
uint64_t FetchStage::selectPC(PipeReg *freg, PipeReg *mreg, PipeReg *wreg)
{
	uint64_t M_icode = mreg->get(M_ICODE);
	uint64_t W_icode = wreg->get(W_ICODE);
	uint64_t M_cnd = mreg->get(M_CND);

	if ((M_icode == Instruction::IJXX) && !M_cnd)
		return mreg->get(M_VALA);

	else if (W_icode == Instruction::IRET)
		return wreg->get(W_VALM);

	return freg->get(F_PREDPC);
}

/**
 * needRegIds
 * This method tells us whether or not we need to get the register
 * id's.
 * @param f_icode takes in the icode for the fetch stage.
 * @return Returns boolean value telling us whether we need the register's id?
 */
bool FetchStage::needRegIds(uint64_t f_icode)
{
	return (f_icode == Instruction::IRRMOVQ || f_icode == Instruction::IOPQ || f_icode == Instruction::IPUSHQ || f_icode == Instruction::IPOPQ || f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ || f_icode == Instruction::IMRMOVQ);
}

/**
 * needValC
 * This method is used to help calculate the address of the next sequential
 * instruction in tandem with needRegIds.
 * @param f_icode takes in the icode from fetch stage.
 * @return Returns boolean value telling us whether we need valC or not based on
 * what f_icode is.
 */
bool FetchStage::needValC(uint64_t f_icode)
{
	return (f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ || f_icode == Instruction::IJXX || f_icode == Instruction::ICALL || f_icode == Instruction::IMRMOVQ);
}

/**
 * predictPC
 * This method is used to predict what the pc is.
 * @param f_icode takes in icode from the fetch stage.
 * @param f_valC takes in the valC from the fetch stage.
 * @param f_valP takes in the valP from the fetch stage.
 * @returns returns either valC or valP depending upon what the instruction
 * is.
 */
uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
	switch (f_icode)
	{
	case Instruction::IJXX:
	case Instruction::ICALL:
		return f_valC;
	default:
		return f_valP;
	}
}

/**
 * PCincrement
 * This method is used to increment the pc depending upon whether or not
 * the result is of reg or valC.
 * @param f_pc this is the value for the program counter.
 * @param regResult this value is coming from needRegIds?
 * @param valCResult this value is coming from needValC?
 * @return the returns the correct pc depending upon where the result
 * is coming from.
 */
uint64_t FetchStage::PCincrement(uint64_t f_pc, bool regResult, bool valCResult)
{
	if (regResult)
		f_pc += 1;
	if (valCResult)
		f_pc += 8;

	return f_pc += 1;
}

/**
 * getRegIds
 * This method is used to read the register byte and initialize rA and rB to
 * the appropriate bits in the register byte. These are then used as input
 * to the D register.
 * @param r_pc takes in the pc.
 * @param rA takes in a pointer to src register A.
 * @param rB takes in a pointer to src register B.
 */
void FetchStage::getRegIds(int32_t f_pc, uint64_t *rA, uint64_t *rB)
{
	bool hasError;
	uint64_t r_byte = mem->getByte(f_pc + 1, hasError);
	uint64_t rA_bits = Tools::getBits(r_byte, 4, 7);
	uint64_t rB_bits = Tools::getBits(r_byte, 0, 3);
	*rA = rA_bits;
	*rB = rB_bits;
}

/**
 * buildValC
 * This method is used to build valC by grabbing the byte from memory and then
 * the buildLong method from our tools class.
 * @param f_pc the pc
 * @param needRegIds takes in the bool value from needRegIds
 * @return This returns what valC is.
 */
uint64_t FetchStage::buildValC(int32_t f_pc, bool needRegIds)
{
	bool hasError;
	uint8_t valC[8];

	f_pc++;
	if (needRegIds)
	{
		f_pc += 1;
	}

	for (int i = 0; i < 8; i++)
	{
		valC[i] = mem->getByte(f_pc + i, hasError);
	}

	return Tools::buildLong(valC);
}

/**
 * instr_valid
 * This method is checking whether an instruction is valid or not.
 * @param f_icode Takes in the icode from the fetch stage.
 * @return Returns boolean value telling us whether the instruction being passed
 * is valid or not.
 */
bool FetchStage::instr_valid(uint64_t f_icode)
{

	return (f_icode == Instruction::INOP || f_icode == Instruction::IHALT ||
			f_icode == Instruction::IRRMOVQ || f_icode == Instruction::IIRMOVQ ||
			f_icode == Instruction::IRMMOVQ || f_icode == Instruction::IMRMOVQ ||
			f_icode == Instruction::IOPQ || f_icode == Instruction::IJXX ||
			f_icode == Instruction::ICALL || f_icode == Instruction::IRET ||
			f_icode == Instruction::IPUSHQ || f_icode == Instruction::IPOPQ);
}

/**
 * f_stat
 * This method will return what stat is during the Fetch stage.
 * @param f_icode takes in icode from the fetch stage
 * @param mem_error takes in the bool value from mem_error method telling us
 * whether or not there is an error with memory.
 * @return This method returns stat depending on three cases.
 */
uint64_t FetchStage::f_stat(uint64_t f_icode, bool mem_error)
{
	if (mem_error)
		return Status::SADR;
	if (!instr_valid(f_icode))
		return Status::SINS;
	if (f_icode == Instruction::IHALT)
		return Status::SHLT;
	return Status::SAOK;
}

/**
 * f_icode
 * This method is returning the icode for fetch stage.
 * @param mem_error takes in boolean value of whether there is an error with memory.
 * @param mem_icode takes in icode from memory.
 * @return Returns correct icode for given case.
 */
uint64_t FetchStage::f_icode(bool mem_error, uint64_t mem_icode)
{
	if (mem_error)
		return Instruction::INOP;
	return mem_icode;
}

/**
 * f_ifun
 * This method is returning ifun for the fetch stage.
 * @param mem_error takes in boolean value of whether there is an error with memory.
 * @param mem_ifun takes in ifun from memory.
 * @return Returns correct ifun for a given case.
 */
uint64_t FetchStage::f_ifun(bool mem_error, uint64_t mem_ifun)
{
	if (mem_error)
		return Instruction::FNONE;
	return mem_ifun;
}

/**
 * stall_F
 * This method is to help prevent hazards by stalling the F stage if needed.
 * @param ereg a pointer pointing to the e register.
 * @param dreg a pointer pointing to the d register.
 * @param mreg a pointer pointing to the m register.
 * @return Returns a boolean value telling us whether we need to stall or not.
 */
bool FetchStage::stall_F(PipeReg *ereg, PipeReg *dreg, PipeReg *mreg)
{
	uint64_t E_icode = ereg->get(E_ICODE);
	uint64_t E_dstM = ereg->get(E_DSTM);
	uint64_t D_icode = dreg->get(D_ICODE);
	uint64_t M_icode = mreg->get(M_ICODE);
	return (((E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)) || (Instruction::IRET == D_icode || Instruction::IRET == E_icode || Instruction::IRET == M_icode));
}

/**
 * stall_D
 * This method is to help prevent hazards by stalling the D stage if needed.
 * @param ereg a pointer pointing to the e register.
 * @return Returns a boolean value telling us whether we need to stall or not.
 */
bool FetchStage::stall_D(PipeReg *ereg)
{
	uint64_t E_icode = ereg->get(E_ICODE);
	uint64_t E_dstM = ereg->get(E_DSTM);

	return (E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB);
}

/**
 * calculateControlSignals
 * This method is used to calculate the control signals.
 * @param ereg a pointer to the e register.
 * @param dreg a pointer to the d register.
 * @param mreg a poitner to the m register.
 */
void FetchStage::calculateControlSignals(PipeReg *ereg, PipeReg *dreg, PipeReg *mreg)
{
	F_stall = stall_F(ereg, dreg, mreg);
	D_stall = stall_D(ereg);
	mispredictedBranch(ereg, dreg, mreg);
}

/**
 * mispredictedBranch
 * This method is used to set D_bubble and is used to help express the condition
 * of a mispredicted branch.
 * @param ereg a pointer to the e register.
 * @param dreg a pointer to the d register.
 * @param mreg a pointer to the m register.
 */
void FetchStage::mispredictedBranch(PipeReg *ereg, PipeReg *dreg, PipeReg *mreg)
{
	uint64_t E_icode = ereg->get(E_ICODE);
	uint64_t E_dstM = ereg->get(E_DSTM);
	uint64_t D_icode = dreg->get(D_ICODE);
	uint64_t M_icode = mreg->get(M_ICODE);
	D_bubble = ((E_icode == Instruction::IJXX && !e_Cnd)) || (!((E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)) && (Instruction::IRET == D_icode || Instruction::IRET == E_icode || Instruction::IRET == M_icode));
}
