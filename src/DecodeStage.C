#include "PipeRegArray.h"
#include "DecodeStage.h"
#include "D.h"
#include "F.h"
#include "E.h"
#include "Instruction.h"

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
	bool hasError = false;
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
	uint64_t d_rvalA = 0;
	uint64_t d_rvalB = 0;
	


	
	// During compilation, numFields is an unused variable.
	// I don't we need it... -Justin
	// uint64_t numFields = dreg -> get(D_NUMFIELDS);

	// maybe pass numfields.
	
	
	
	SelFwdA(dreg, d_rvalA);
	FwdB(dreg,d_rvalB);
	setEInput(ereg, stat, icode, ifun, valC, valA, valB, setDstE(ereg, icode, dstE, rB, RegisterFile::rsp), setDstM(ereg, icode, dstM, rA), setSrcA(ereg, icode, d_srcA, rA, RegisterFile::rsp), setSrcB(ereg, icode, d_srcB, rB, RegisterFile::rsp));

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

uint64_t DecodeStage::setSrcA(PipeReg *ereg, uint64_t D_icode, uint64_t d_srcA, uint64_t D_rA, uint64_t RSP) {
	if(D_icode == Instruction::IOPQ || Instruction::IRMMOVQ || Instruction::IMRMOVQ) return d_srcA = D_rA;
	if(D_icode == Instruction::IPOPQ || Instruction::IRET) return d_srcA = RegisterFile::rsp;
	return RegisterFile::RNONE;
}

uint64_t DecodeStage::setSrcB(PipeReg *ereg, uint64_t D_icode, uint64_t d_srcB, uint64_t D_rB, uint64_t RSP){
	if(D_icode == Instruction::IOPQ || D_icode == Instruction::IRMMOVQ || D_icode == Instruction::IMRMOVQ) return d_srcB = D_rB;
	if(D_icode == Instruction::IPUSHQ || D_icode == Instruction::IPOPQ || D_icode == Instruction::ICALL || D_icode == Instruction::IRET) return d_srcB = RSP;
	return RegisterFile::RNONE;

}

uint64_t DecodeStage::setDstE(PipeReg *ereg, uint64_t D_icode, uint64_t d_dstE, uint64_t D_rB, uint64_t RSP){
	if(D_icode == Instruction::IRMMOVQ || D_icode == Instruction::IIRMOVQ || D_icode == Instruction::IOPQ) return d_dstE = D_rB;
	if(D_icode == Instruction::IPUSHQ || D_icode == Instruction:: IPOPQ || D_icode ==  Instruction::ICALL || D_icode == Instruction::IRET) return d_dstE = RSP;
	return RegisterFile::RNONE;
}

uint64_t DecodeStage::setDstM(PipeReg *ereg, uint64_t D_icode, uint64_t d_dstM, uint64_t D_rA){
	if(D_icode == Instruction::IMRMOVQ || D_icode == Instruction::IPOPQ) return d_dstM = D_rA;
	return RegisterFile::RNONE;
}

uint64_t DecodeStage::SelFwdA(PipeReg *dreg, uint64_t d_rvalA){
	bool hasError = false;
	return d_rvalA = rf->readRegister(d_srcA, hasError);//-- current option.
	
}

uint64_t DecodeStage::FwdB(PipeReg *dreg, uint64_t d_rvalB){
	bool hasError = false;
	return d_rvalB = rf -> readRegister(d_srcB, hasError);
}

