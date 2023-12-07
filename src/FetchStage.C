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

	// TODO: read lab assignment
	// TODO
	// select PC value and read byte from memory

	// set icode and ifun using byte read from memory

	uint64_t f_pc = selectPC(freg, mreg, wreg);
	uint64_t byte = mem->getByte(f_pc, mem_error);
	// uint64_t f_icode = Tools::getBits(byte, 4, 7);
	// uint64_t f_ifun = Tools::getBits(byte, 0, 3);
	
	// Fetch stage modifications, I switched f_ifun and f_icode to just ifun and icode
	// below is where I set stat icode and ifun
	icode = f_icode(mem_error, Tools::getBits(byte, 4, 7));
	stat = f_stat(icode, mem_error);
	ifun = f_ifun(mem_error,Tools::getBits(byte, 0, 3));
	// // f_ifun = f_ifun(mem_error,f_ifun);

	

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
	if(!F_stall){
	freg->normal();
	}
	if(D_bubble){
	((D *)dreg)->bubble();
	}
	else if(!D_stall){
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
// TODO
// Write your selectPC, needRegIds, needValC, PC increment, and predictPC methods
// Remember to add declarations for these to FetchStage.h

// Here is the HCL describing the behavior for some of these methods.

// selectPC method: input is F, M, and W registers
//  word f_pc = [
//     #1 M_icode == IJXX && !M_Cnd : M_valA;
//     #2 W_icode == IRET : W_valM;
//     #3 1: F_predPC;
//  ];
uint64_t FetchStage::selectPC(PipeReg *freg, PipeReg *mreg, PipeReg *wreg)
{
	uint64_t M_icode = mreg->get(M_ICODE);
	uint64_t W_icode = wreg->get(W_ICODE);
	uint64_t M_cnd = mreg->get(M_CND);

	if ((M_icode == Instruction::IJXX) && !M_cnd)
		return mreg->get(M_VALA);

	else if (W_icode == Instruction::IRET)
		return wreg->get(W_VALM);

	return freg->get(F_PREDPC); // we set f_predpc to our predicted PC earlier.
}


bool FetchStage::needRegIds(uint64_t f_icode)
{
	return (f_icode == Instruction::IRRMOVQ || f_icode == Instruction::IOPQ || f_icode == Instruction::IPUSHQ || f_icode == Instruction::IPOPQ || f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ || f_icode == Instruction::IMRMOVQ);
}


bool FetchStage::needValC(uint64_t f_icode)
{
	return (f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ || f_icode == Instruction::IJXX || f_icode == Instruction::ICALL || f_icode == Instruction::IMRMOVQ);
}

// predictPC method: inputs are f_icode, f_valC, f_valP
//  word f_predPC = [
//      #1 f_icode in { IJXX, ICALL } : f_valC;
//      #2 1: f_valP;
//  ];
uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{

	
	if (f_icode == Instruction::IJXX || f_icode == Instruction::ICALL)
		return f_valC;

	
	else return f_valP;
}

uint64_t FetchStage::PCincrement(uint64_t f_pc, bool regResult, bool valCResult)
{
	if (regResult)
		f_pc += 1;
	if (valCResult)
		f_pc += 8;

	return f_pc += 1;
}
/*
   if need_regID is true...

   Are we calling this method in doClockLow?

   This method is used to read the register byte and initialize
   rA and rB to the appropriate bits in the register byte. These are then used as input
   to the D register.
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

// first HCL
bool FetchStage::instr_valid(uint64_t f_icode)
{

	return (f_icode == Instruction::INOP || f_icode == Instruction::IHALT ||
			f_icode == Instruction::IRRMOVQ || f_icode == Instruction::IIRMOVQ ||
			f_icode == Instruction::IRMMOVQ || f_icode == Instruction::IMRMOVQ ||
			f_icode == Instruction::IOPQ || f_icode == Instruction::IJXX ||
			f_icode == Instruction::ICALL || f_icode == Instruction::IRET ||
			f_icode == Instruction::IPUSHQ || f_icode == Instruction::IPOPQ);
}

// second HCL
uint64_t FetchStage::f_stat(uint64_t f_icode, bool mem_error)
{
	if (mem_error)
		return Status::SADR;
	//instr_valid method used here
	if (!instr_valid(f_icode))
		return Status::SINS;
	if (f_icode == Instruction::IHALT)
		return Status::SHLT;
	
	return Status::SAOK;
}

// third HCL
uint64_t FetchStage::f_icode(bool mem_error, uint64_t mem_icode)
{
	if (mem_error)
		return Instruction::INOP;
	return mem_icode;
}

// fourth HCL
uint64_t FetchStage::f_ifun(bool mem_error, uint64_t mem_ifun)
{
	if (mem_error)
		return Instruction::FNONE;
	return mem_ifun;
}

bool FetchStage::stall_F(PipeReg *ereg, PipeReg *dreg, PipeReg *mreg){
uint64_t E_icode = ereg -> get(E_ICODE);
uint64_t E_dstM = ereg -> get (E_DSTM);
uint64_t D_icode = dreg -> get(D_ICODE);
uint64_t M_icode = mreg -> get(M_ICODE);
	return (((E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)) || (Instruction::IRET == D_icode || Instruction::IRET == E_icode || Instruction::IRET == M_icode));
}


bool FetchStage::stall_D(PipeReg *ereg) {
uint64_t E_icode = ereg -> get(E_ICODE);
uint64_t E_dstM = ereg -> get (E_DSTM);

return (E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB); 
}


void FetchStage::calculateControlSignals(PipeReg *ereg, PipeReg *dreg, PipeReg *mreg){
	F_stall = stall_F(ereg,dreg,mreg);
	D_stall = stall_D(ereg);
	mispredictedBranch(ereg,dreg,mreg);

	
	
}

void FetchStage::mispredictedBranch(PipeReg *ereg, PipeReg *dreg, PipeReg *mreg){
	uint64_t E_icode = ereg -> get(E_ICODE);
	uint64_t E_dstM = ereg -> get(E_DSTM);
	uint64_t D_icode = dreg -> get(D_ICODE);
	uint64_t M_icode = mreg -> get(M_ICODE);
	 D_bubble = ((E_icode == Instruction::IJXX && !e_Cnd)) ||  (!((E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)) && (Instruction::IRET == D_icode || Instruction::IRET == E_icode || Instruction::IRET == M_icode));
}
