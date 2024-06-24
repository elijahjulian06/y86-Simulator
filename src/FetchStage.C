//TODO add more #includes as you need them
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
#include "Tools.h"
#include "Memory.h"
#include "E.h"




/*
* doClockLow
*
* Performs the Fetch stage combinational logic that is performed when
* the clock edge is low.
*
* @param: pipeRegs - array of the pipeline register
(F, D, E, M, W instances)
*/
bool FetchStage::doClockLow(PipeRegArray * pipeRegs)
{
    PipeReg * freg = pipeRegs->getFetchReg();
    PipeReg * dreg = pipeRegs->getDecodeReg();
    PipeReg * mreg = pipeRegs->getMemoryReg();
    PipeReg * ereg = pipeRegs->getExecuteReg();
    bool mem_error = false;
    uint64_t icode = Instruction::INOP, ifun = Instruction::FNONE;
    uint64_t rA = RegisterFile::RNONE, rB = RegisterFile::RNONE;
    uint64_t valC = 0, valP = 0, stat = 0, predPC = 0;
    bool needvalC = false;
    bool needregId = false;


    //TODO
    //select PC value and read byte from memory
    //set icode and ifun using byte read from memory
    //uint64_t f_pc = .... call your select pc function
    uint64_t f_pc = selctPC(pipeRegs);
    uint8_t byte = mem->getByte(f_pc, mem_error);
    stat = Status::SAOK;

    if(icode == Instruction::IHALT)
    {
        stat = Status::SHLT;
    }

    if(mem_error)
    {
        icode = Instruction::INOP;
    }
    else
    {
        icode = Tools::getBits(byte, 4, 7);
    }

    if(mem_error)
    {
        icode = Instruction::FNONE;
    }
    else
    {
        ifun = Tools::getBits(byte, 0, 3);
    }

    //status of this instruction is SAOK (this will change later)
    bool instrValid = instr_valid(icode);
    stat = statHCL(mem_error, instrValid, icode);
    uint64_t e_icode = ereg->get(E_ICODE);
    uint64_t e_dstM = ereg->get(E_DSTM);
    uint64_t D_icode = dreg->get(D_ICODE);
    uint64_t M_icode = mreg->get(M_ICODE);
   
    calculateControlSignals(e_icode, e_dstM, D_icode, M_icode);
    //TODO
    //In order to calculate the address of the next instruction,
    //you'll need to know whether this current instruction has an
    //immediate field and a register byte. (Look at the instruction encodings.)
    //needvalC = .... call your need valC function
    //needregId = .... call your need regId function
    
    needvalC = needValC(icode);
    needregId = needRegIds(icode);
    uint64_t regByte = mem->getByte(f_pc + 1, mem_error);

    if(needregId)
    {
        getRegIds(regByte, rA, rB);
    }

    if(needvalC)
    {
        valC = buildValC(f_pc, needregId, mem_error);
    }
    //TODO
    //determine the address of the next sequential function
    //valP = ..... call your PC increment function
    valP = PCincrement(f_pc, needregId, needvalC);


    //TODO
    //calculate the predicted PC value
    //predPC = .... call your function that predicts the next PC
    predPC = predictPC(icode, valC, valP);


    //set the input for the PREDPC pipe register field in the F register
    freg->set(F_PREDPC, predPC);

    //set the inputs for the D register
    setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
    return false;
}


/** doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param pipeRegs - array of the pipeline register (F, D, E, M, W instances)
**/
void FetchStage::doClockHigh(PipeRegArray * pipeRegs)
{
    PipeReg * freg = pipeRegs->getFetchReg();
    PipeReg * dreg = pipeRegs->getDecodeReg();
    
    if(F_stall == false)
    {
        freg->normal();
    }
    if(D_bubble == true)
    {
        ((D *)dreg)->bubble();
    }
    else if(D_stall == false)
    {
        dreg->normal();
    }

} 


/** setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param dreg - pointer to the D register instance
 * @param stat - value to be stored in the stat pipeline register within D
 * @param icode - value to be stored in the icode pipeline register within D
 * @param ifun - value to be stored in the ifun pipeline register within D
 * @param rA - value to be stored in the rA pipeline register within D
 * @param rB - value to be stored in the rB pipeline register within D
 * @param valC - value to be stored in the valC pipeline register within D
 * @param valP - value to be stored in the valP pipeline register within D
**/
void FetchStage::setDInput(PipeReg * dreg, uint64_t stat, uint64_t icode,
    uint64_t ifun, uint64_t rA, uint64_t rB, uint64_t valC, uint64_t valP)
{
    dreg->set(D_STAT, stat);
    dreg->set(D_ICODE, icode);
    dreg->set(D_IFUN, ifun);
    dreg->set(D_RA, rA);
    dreg->set(D_RB, rB);
    dreg->set(D_VALC, valC);
    dreg->set(D_VALP, valP);
}
//TODO
//Write your selectPC, needRegIds, needValC, PC increment, and predictPC methods
//Remember to add declarations for these to FetchStage.h


// Here is the HCL describing the behavior for some of these methods.


 

/** selctPc
 * 
 * selects the PC.
 * 
 * @param pipeRegs - passed to make calls to registers from F,M,W stage
 *
 * selectPC method: input is F, M, and W registers
 * word f_pc = [
 * M_icode == IJXX && !M_Cnd : M_valA;
 * W_icode == IRET : W_valM;
 * 1: F_predPC;
 * ];
**/
uint64_t FetchStage::selctPC(PipeRegArray * pipeRegs)
{
    PipeReg * freg = pipeRegs->getFetchReg();
    PipeReg * mreg = pipeRegs->getMemoryReg();
    PipeReg * wreg = pipeRegs->getWritebackReg();


    uint64_t M_icode = mreg->get(M_ICODE);
    uint64_t W_icode = wreg->get(W_ICODE);
    //uint64_t F_predPC = 0;


    if(M_icode == Instruction::IJXX && !(mreg->get(M_CND)))
    {
        return mreg->get(M_VALA);
    }
    else if(W_icode == (Instruction::IRET))
    {
        return wreg->get(W_VALM);
    }
    return freg->get(F_PREDPC);
}


/** needRegIds
 *
 * returns true or false, if it needs register IDs.
 * 
 * @param f_icode - fetch i code to check for i codes -> RRMOVQ, IOPQ, IPUSHQ, IPOPQ, IIRMOVQ, IRMMOVQ, IMRMOVQ
 *
 * needRegIds method: input is f_icode
 * bool need_regids = f_icode in { IRRMOVQ, IOPQ, IPUSHQ, IPOPQ, IIRMOVQ, IRMMOVQ, IMRMOVQ };
**/
bool FetchStage::needRegIds(uint64_t f_icode)
{
    if(f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IOPQ ||
        f_icode == Instruction::IPUSHQ || f_icode == Instruction::IPOPQ ||
            f_icode == Instruction::IRRMOVQ || f_icode == Instruction::IRMMOVQ ||
                f_icode == Instruction::IMRMOVQ)
    {
        return true;
    }
    return false;
}


/** needValC
 *
 * returns true based off the instruction that needs a ValC.
 * 
 * @param f_icode - fetch stage i code to check for i codes -> IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL
 *
 * needValC method: input is f_icode
 * bool need_valC = f_icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ, IJXX, ICALL };
**/
bool FetchStage::needValC(uint64_t f_icode)
{
    if(f_icode == Instruction::IIRMOVQ || f_icode == Instruction::IRMMOVQ ||
        f_icode == Instruction::IMRMOVQ || f_icode == Instruction::IJXX || f_icode == Instruction::ICALL)
    {  
        return true;
    }
    return false;
}


/** predictPC
 *
 * predicts the next PC.
 * 
 * @param f_icode - fetch stage i code - used to chek if its IJXX or ICALL
 * @param f_valC - return value if I code is IJXX or ICALL
 * @param f_valP - return value if I code is not IJXX or ICALL
 *
 * predictPC method: inputs are f_icode, f_valC, f_valP
 * word f_predPC = [
 * f_icode in { IJXX, ICALL } : f_valC;
 * 1: f_valP;
 * ];
*/
uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
    if(f_icode == Instruction::IJXX || f_icode == Instruction::ICALL) 
    {
        return f_valC;
    }
    return f_valP;
}


/** PCincrement
 *
 * @param f_pc - current pc
 * @param r - result of needRegIds
 * @param c - result of needValC
 *
 * PCincrement takes as input the address of the current instruction (f_pc),
 * the result of needRegIds, and the result of needValC and calculates the address
 * of the next sequential instruction. The value returned by PCincrement is stored in valP.
 * The value of valP is then used as input to predictPC along with the icode value and the value of valC (0 for now).
 * The output of predictPC is the input to the F_predPC register.
**/
uint64_t FetchStage::PCincrement(uint64_t f_pc, bool r, bool c)
{
    return (f_pc + 1 + r + (8*c));
}


/** getRegIds
 *
 * @param regByte - register Byte
 * @param ra - first register that is initalized from bytes 0-3
 * @param rb - second register that is initalized from bytes 4-7
 *
 * getRegIds - if need_regId is true, this method is used to read the register byte and
 * initialize rA and rB to the appropriate bits in the register byte.
 * These are then used as input to the D register.
**/
void FetchStage::getRegIds(uint64_t regByte, uint64_t &rA, uint64_t &rB)
{
    rB = Tools::getBits(regByte, 0, 3);
    rA = Tools::getBits(regByte, 4, 7);
}


/** buildValC
 *
 * @param f_pc - current pc 
 * @param mem_error - Set to true if there is a memory error 
 * @return - returns valC
 *
 * buildValC - if need_valC is true, this method reads 8 bytes from memory and builds and returns the valC that is then used as input to the D register.
 * (Do not use getLong to read those 8 bytes.
 * This requires that the immediate value be aligned in memory, which it may not be.
 * Do make use of your Tools class to build the 64 bit valC from the 8 bytes.)
*/
uint64_t FetchStage::buildValC(uint64_t f_pc, bool needregid, bool &mem_error)
{
    uint8_t tempValC[LONGSIZE];
    if(needregid)
    {
        f_pc = f_pc + 2;
        for(uint i = 0; i < 8; i++)
        {
            tempValC[i] = mem->getByte(i + f_pc, mem_error);
        }
        //return Tools::buildLong(tempValC);
    }
    else
    {
        f_pc = f_pc + 1;
        for(uint i = 0; i < 8; i++)
        {
            tempValC[i] = mem->getByte(i + f_pc, mem_error);
        }
    }
    return Tools::buildLong(tempValC);
}



/** instr_valid
 * 
 * Checks to see if the instruction icode is valid.
 * 
 *  @param: f_icode - fetch stage i code
 *  @return true or false if its a valid instruction.
 * 
 * bool instr_valid = f_icode in { INOP, IHALT, IRRMOVQ, IIRMOVQ, IRMMOVQ, IMRMOVQ, 
 *                                IOPQ, IJXX, ICALL, IRET, IPUSHQ, IPOPQ }; 
*/
bool FetchStage::instr_valid(uint64_t icode)
{
    return (icode == Instruction::INOP || icode == Instruction::IHALT || icode == Instruction::IRRMOVQ ||
            icode == Instruction::IIRMOVQ || icode == Instruction::IRMMOVQ || icode == Instruction::IMRMOVQ ||
            icode == Instruction::IOPQ || icode == Instruction::IJXX || icode == Instruction::ICALL || 
            icode == Instruction::IRET || icode == Instruction::IPUSHQ || icode == Instruction::IPOPQ);
}


/** statHCL
 * 
 * Checks the status of the HCL and whether it should continue running or throw exception
 * 
 * @param mem_error - if true, returns SADR
 * @param instr_valid - if true, returns SINS
 * @param icode - if true returns SHLT
 * 
 *  word f_stat = [
 *    mem_error : SADR;
*    !instr_valid : SINS;
 *    f_icode == IHALT : SHLT;
 *    1 : SAOK;
 * 
 * 
*/
uint64_t FetchStage::statHCL(bool mem_error, bool instr_valid, uint64_t icode)
{
    if(mem_error)
    {
        return Status::SADR;
    }
    else if(!instr_valid)
    {
        return Status::SINS;
    }
    else if(icode == Instruction::IHALT)
    {
        return Status::SHLT;
    }
    else{
        return Status::SAOK;
    }

}


/** setFstall
 * 
 * @param E_icode - used to check excutes icode and whether it needs to bubble/stall.
 * @param E_dstM - used to check if there are dependencies in the execute destination registers.
 * @param D_icode - used to check if IRET.
 * @param M_icode -  used to check if IRET.
 * @return - returns true if there is a mrmovq or ipoq and a dependencyor or if there is a IRET in pipeline.
 *
 * bool F_stall = E_icode in { IMRMOVQ, IPOPQ } &&  E_dstM in { d_srcA, d_srcB } || 
               IRET in { D_icode, E_icode, M_icode };
 * 
*/
bool FetchStage::setFStall(uint64_t E_icode, uint64_t E_dstM, uint64_t D_icode, uint64_t M_icode)
{
    if(((E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == Stage::d_srcA || E_dstM == Stage::d_srcB)) 
        || (Instruction::IRET == D_icode || Instruction::IRET == E_icode || Instruction::IRET == M_icode)) 
    {
        return true;
    }
    return false;
}

/** setDStall
 * 
 * Method returns whether the fetch register needs to stall
 * 
 * @param E_icode - used to check if its mrmovq or ipopq
 * @param E_dstM - used to check if there is a dependency register
 * @return - return true if there is a mrmovq or ipopq and a dependency in the pipeline.
 * 
 * bool D_stall = E_icode in { IMRMOVQ, IPOPQ } &&  E_dstM in { d_srcA, d_srcB };
 * 
 * 
*/
bool FetchStage::setDStall(uint64_t E_icode, uint64_t E_dstM)
{
    if((E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IPOPQ) && (E_dstM == Stage::d_srcA || E_dstM == Stage::d_srcB))
    {
        return true;
    }
    return false;
}

/** setDbubble
 * 
 * Returns true if the Decode register needs to bubble. Returns false if it doesnt not.
 * 
 * @param E_icode - Used to check if it is Jump instruction and where the condition code is not 0.
 * @param E_dstM - Used to check if srca and b are equal to e_dstM.
 * @param D_icode - Used to check if IRET is in the pipeline.
 * @param M_icode - Used to check if IRET is in the pipeline.
 * @return - returns either one or zero based off HCL.
 * 
 * bool D_bubble = ( E_icode == IJXX && !e_Cnd ) ||
                # Bubble while return passes through, but not if load/use hazard 
                !( E_icode in { IMRMOVQ, IPOPQ } && E_dstM in { d_srcA, d_srcB }) && 
                IRET in { D_icode, E_icode, M_icode 
*/
bool FetchStage::setDBubble(uint64_t E_icode, uint64_t E_dstM, uint64_t D_icode, uint64_t M_icode)
{
    if ((E_icode == Instruction::IJXX && !Stage::e_Cnd) || 
         (((E_icode != Instruction::IRMMOVQ && E_icode != Instruction::IPOPQ) ||
          (E_dstM != d_srcA && E_dstM != d_srcB)) &&
            (Instruction::IRET == D_icode || Instruction::IRET == E_icode || Instruction::IRET == M_icode)))
    {
        return true;
    }
    return false;
}

/** calculateControlSignals
 * 
 * Helper method to set all private variables F_stall, D_stall D_bubble
 * 
 * @param E_icode - value used to pass in to the set F_stall, D_stall, D_bubble
 * @param E_dstM - value used to pass in to the set F_stall, D_stall, D_bubble
 * @param D_icode - value used to pass in to the set F_stall, D_stall, D_bubble
 * @param M_icode - value used to pass in to the set F_stall, D_stall, D_bubble
 * 
*/
void FetchStage::calculateControlSignals(uint64_t E_icode, uint64_t E_dstM, uint64_t D_icode, uint64_t M_icode)
{
    F_stall = setFStall(E_icode, E_dstM, D_icode, M_icode);
    D_stall = setDStall(E_icode, E_dstM);
    D_bubble = setDBubble(E_icode, E_dstM, D_icode, M_icode);
}
