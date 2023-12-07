#include "PipeRegArray.h"
#include "MemoryStage.h"
#include "W.h"
#include "M.h"
#include "E.h"
#include "Stage.h"
#include "Instruction.h"
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
bool MemoryStage::doClockLow(PipeRegArray *pipeRegs)
{

	PipeReg *mreg = pipeRegs->getMemoryReg();
	PipeReg *wreg = pipeRegs->getWritebackReg();
	Memory *memory = Memory::getInstance();
	uint64_t stat = mreg->get(M_STAT);
	uint64_t icode = mreg->get(M_ICODE);
	uint32_t dstM = mreg->get(M_DSTM);
	uint64_t dstE = mreg->get(M_DSTE);
	uint64_t valE = mreg->get(M_VALE);
	uint64_t valA = mreg->get(M_VALA);
	Stage::m_valM = 0;

	uint64_t address = mem_addr(icode, valE, valA);

	bool error = false;
	if (mem_read(icode))
	{
		m_valM = memory->getLong(address, error);
	}
	else if (mem_write(icode))
	{
		memory->putLong(valA, address, error);
	}
	if (error)
		m_stat = Status::SADR;
	else
		m_stat = stat;
	setWInput(wreg, m_stat, icode, valE, m_valM, dstE, dstM);

	return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pipeRegs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeRegArray *pipeRegs)
{
	PipeReg *wreg = pipeRegs->getWritebackReg();
	wreg->normal();
}

/**
 * setWInput
 * provides the input to potentially be stored in the W register during doClockHigh
 * @param wreg - pointer to w register instance
 * @param stat - value to be stored in the stat pipeline register within W
 * @param icode - value to be stored in the icode pipeline register within W
 * @param valE - value to be stored in the valE pipeline register within W
 * @param valM - value to be stored in valM pipeline register within W
 * @param dstE - value to be stored in dstE pipeline register within W
 * @param dstM - value to be stored in dstM pipeline register within W
 */
void MemoryStage::setWInput(PipeReg *wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, uint64_t dstE, uint32_t dstM)
{
	wreg->set(W_STAT, stat);
	wreg->set(W_ICODE, icode);
	wreg->set(W_VALE, valE);
	wreg->set(W_VALM, valM);
	wreg->set(W_DSTE, dstE);
	wreg->set(W_DSTM, dstM);
}

/**
 * mem_addr
 * this will calculate the address to be written to ?
 * @param M_icode - value to check to see what value to pass depending on instruction
 * @param M_valE - value to be returned for address
 * @param M_valA - value to be returned for address
 * @return - value of the address
 */
uint64_t MemoryStage::mem_addr(uint64_t M_icode, uint64_t M_valE, uint64_t M_valA)
{
	if (M_icode == Instruction::IRMMOVQ || M_icode == Instruction::IPUSHQ || M_icode == Instruction::ICALL || M_icode == Instruction::IMRMOVQ)
	{
		return M_valE;
	}
	if (M_icode == Instruction::IPOPQ || M_icode == Instruction::IRET)
	{
		return M_valA;
	}
	return 0;
}

/**
 * mem_read
 * this finds whether a value from memory needs to be read.
 * @param M_icode - value to be checked to see if you are reading from memory.
 * @return - true or false if you need to read from memory.
 */
bool MemoryStage::mem_read(uint64_t M_icode)
{
	return (M_icode == Instruction::IMRMOVQ || M_icode == Instruction::IPOPQ || M_icode == Instruction::IRET);
}

/**
 * mem_write
 * this finds out whether or not you need to write to memory depending on the
 * instruction.
 * @param M_icode - value to be checked to see if anything needs to be written.
 * @return - true or false, if you need to write to memory.
 */
bool MemoryStage::mem_write(uint64_t M_icode)
{
	return (M_icode == Instruction::IRMMOVQ || M_icode == Instruction::IPUSHQ || M_icode == Instruction::ICALL);
}