// TODO add more #includes as you need them
#include <cstdint>
#include "PipeRegArray.h"
#include "PipeReg.h"
#include "Memory.h"
#include "FetchStage.h"
#include "Instruction.h"
#include "RegisterFile.h"
#include "Status.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"

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
   uint64_t valC = 0, valP = 0, stat = Status::SAOK, predPC = 0;
   bool needvalC = false;
   bool needregId = false;
   PipeReg *wreg = pipeRegs->getWritebackReg();
   PipeReg *mreg = pipeRegs->getMemoryReg();

   // TODO: read lab assignment
   // TODO
   // select PC value and read byte from memory
   // set icode and ifun using byte read from memory
   F *f;
   M *m;
   W *w;
   uint64_t f_pc = selectPC(f,m,w);
	if(icode == Instruction::IHALT){
		stat = Status::SHLT;
	}

   // set stat for this instruction to Status::SHLT if
   // icode is Instruction::IHALT; otherwise leave it is
   // as initialized to Status::SAOK

   // TODO
   // In order to calculate the address of the next instruction,
   // you'll need to know whether this current instruction has an
   // immediate field and a register byte. (Look at the instruction encodings.)
    needvalC = needValC(icode); // ? // needvalC = ...... call your needValC
	needregId = needRegIds(icode);    // needregId = .... call your need regId function
	

   // TODO
   // determine the address of the next sequential function
    valP = PCincrement(f_pc, needregId, needValC); // ?

   // TODO
   // calculate the predicted PC value
    predPC = predictPC(icode,valC,valP); //.... call your function that predicts the next PC

   // set the input for the PREDPC pipe register field in the F register
   freg->set(F_PREDPC, predPC);

   // set the inputs for the D register
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
   freg->normal();
   dreg->normal();
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
uint64_t FetchStage::selectPC(F *freg, M *mreg, W *wreg)
{
	uint64_t M_icode;
	uint64_t W_icode;
	uint64_t F_predPC;
	uint64_t M_cnd;
	uint64_t m_valA;
	M_icode = freg -> get(M_ICODE);
	M_cnd = mreg -> get(M_CND);
	m_valA = mreg -> get(M_VALA);
	// typedef unsigned int WORD [ M_icode, W_icode, F_predPC];
	/*
      Initial thoughts on approach:
      #1
      - uint64_t m_icode
      - Instruction::IJXX
      - uint64_t m_cnd
      - get mreg valA
      if m_icode is equal to IJXX and not m_cnd then get m_valA
	*/
	M_icode = (Instruction::IJXX && !M_cnd) ? : m_valA;
	/*
      #2
      - uint64_t w_icode
      - Instruction::IRET
      - get wreg valM
      if w_icode is equal to IRET then get w_valM
	*/
	W_icode = (W_icode == Instruction::IRET) ? : wreg -> get(W_VALM);
	


	/*	
      #3
      - get freg predPC
      otherwise return freg predPC (I assume is predicted pc...)

      - Justin
   */
  	if (freg -> get(F_PREDPC) == 1) return freg -> get(F_PREDPC); 
   
   	return freg -> get(F_predPC);
}

// needRegIds  method: input is f_icode
//  bool need_regids = f_icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, IIRMOVQ, IRMMOVQ, IMRMOVQ };
bool FetchStage::needRegIds(uint64_t f_icode)
{
   return (f_icode == Instruction::IRRMOVQ || f_icode == Instruction::IOPQ || f_icode == Instruction::IPUSHQ || f_icode == Instruction::IPOPQ || f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ || f_icode == Instruction::IMRMOVQ);
}

// needValC method: input is f_icode
//  bool need_valC = f_icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL };
bool FetchStage::needValC(uint64_t f_icode) 
{
   return (f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ || f_icode == Instruction:: IJXX || f_icode == Instruction::ICALL);
}

// predictPC method: inputs are f_icode, f_valC, f_valP
//  word f_predPC = [
//      #1 f_icode in { IJXX, ICALL } : f_valC;
//      #2 1: f_valP;
//  ];

uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
	
   /*
      initial thoughts on approach:
      #1 
      - uint64_t f_icode
      - Instruction::IJXX
      - Instruction::ICALL
      - uint64_t f_valC
      if f_icode is IJXX or ICALL then return f_valC
    */
   	if(f_icode == Instruction::IJXX || Instruction::ICALL) return f_valC;
	
    /*
      #2 
      - uint64_t f_valP
      otherwise return f_valP

      - Justin
   */
   return f_valP;
}

uint64_t FetchStage::PCincrement(uint64_t f_pc, bool regResult, bool valCResult){
	if(regResult && valCResult) return f_pc + 1;

	return f_pc;


	
}
