#include "PipeRegArray.h"
#include "MemoryStage.h"
#include "M.h"
#include "W.h"
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
bool MemoryStage::doClockLow(PipeRegArray * pipeRegs)
{
    PipeReg * mreg = pipeRegs->getMemoryReg();
    PipeReg * wreg = pipeRegs->getWritebackReg();
    uint64_t stat = mreg->get(M_STAT);
    uint64_t icode = mreg->get(M_ICODE);
    uint64_t valE = mreg->get(M_VALE);
    //ValM
    uint64_t dstE = mreg->get(M_DSTE);
    uint64_t dstM = mreg->get(M_DSTM);
    uint64_t valA = mreg->get(M_VALA);

    uint64_t mem_addr = addrComp(icode, valE, valA);
    bool mem_error = false;

    if(memRead(icode))
    {
        m_valM = mem->getLong(mem_addr, mem_error);  
    }
    else 
    {
        m_valM = 0;
    }
    if(memWrite(icode))
    {
        mem->putLong(valA, mem_addr, mem_error);
    }

    if(mem_error)
    {
        m_stat = Status::SADR;
    }
    else
    {
        m_stat = stat;
    }

    setWInput(wreg, m_stat, icode, valE, m_valM, dstE, dstM);
    return false;
}

/* doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 * 
 * @param pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void MemoryStage::doClockHigh(PipeRegArray * pipeRegs)
{
    PipeReg * wreg = pipeRegs->getWritebackReg();
    wreg->normal();
}

/** setWinput
 * sets the input values for the next stage, writeback stage.
 * @param wreg - pointer to writeback register
 * @param stat - value to be stored in the stat pipeline register within WB
 * @param icode - value of i code to be stoed in i code pipleline register within WB
 * @param valE -  value to be stored in the valE pipeline register within WB
 * @param valM - value to be stored in the valM pipeline reigster within WB
 * @param dstE - value to be stored in the dstE pipeLine register within WB
 * @param dstM - value to be stored in the dstM pipeLine register within WB
 * 
*/
void MemoryStage::setWInput(PipeReg * wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, uint64_t dstE, uint64_t dstM)
{
    wreg->set(W_STAT, stat);
    wreg->set(W_ICODE, icode);
    wreg->set(W_VALE, valE);
    wreg->set(W_VALM, valM);
    wreg->set(W_DSTE, dstE);
    wreg->set(W_DSTM, dstM);
}
/** addrComp
 *  
 * @param icode - used to check what type of instruction it is.
 * @param m_valE - return value for certain instructions.
 * @param m_valA - return value for certain instructions.
 * @return - either return m_valE or m_valA
 * 
 * //HCL for Addr component
    word mem_addr = [
    M_icode in { IRMMOVQ, IPUSHQ, ICALL, IMRMOVQ } : M_valE;
     M_icode in { IPOPQ, IRET } : M_valA;
     1: 0;  
    ];
 * 
*/
uint64_t MemoryStage::addrComp(uint64_t icode, uint64_t m_valE, uint64_t m_valA)
{
    if((icode == Instruction::IRMMOVQ) || (icode == Instruction::IPUSHQ) || (icode == Instruction::ICALL) || (icode == Instruction::IMRMOVQ)) 
    {
        return m_valE;
    }
    else if((icode = Instruction::IPOPQ) || (icode == Instruction::IRET))
    {
        return m_valA;
    }
    return 0;
}

/** memRead
 * 
 * returns true if it a instruction that reads memory else it returns false.
 * 
 * @param icode - used to check what kind of instruction it is.
 * @reuturn - returns true if icode is a instruction that reads memmory
 * 
 * mem_read = M_icode in { IMRMOVQ, IPOPQ, IRET };
 * 
*/
bool MemoryStage::memRead(uint64_t icode)
{
    if((icode == Instruction::IMRMOVQ) || (icode == Instruction::IPOPQ) || (icode == Instruction::IRET))
    {
        return true;
    }
    return false;
}

/** memWrite
 * 
 * returns true if icode is an instruction that writes to memory 
 * 
 * @param icode - used to check what kind of instruction it is.
 * @return - bool value returns true based off instruction
 * 
 * bool mem_write = M_icode in { IRMMOVQ, IPUSHQ, ICALL };
*/
bool MemoryStage::memWrite(uint64_t icode)
{
    if((icode == Instruction::IRMMOVQ) || (icode == Instruction::IPUSHQ) || (icode == Instruction::ICALL))
    {
        return true;
    }
    return false;
}



