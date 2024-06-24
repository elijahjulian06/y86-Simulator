#include "PipeRegArray.h"
#include "ExecuteStage.h"
#include "M.h"
#include "E.h"
#include "RegisterFile.h"
#include "Instruction.h"
#include "ConditionCodes.h"
#include "Tools.h"
#include "Status.h"
#include "W.h"

/*
 * doClockLow
 *
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param pipeRegs - array of the pipeline register 
 *                  (F, D, E, M, W instances)
 */
bool ExecuteStage::doClockLow(PipeRegArray * pipeRegs)
{
    PipeReg * ereg = pipeRegs->getExecuteReg();
    PipeReg * mreg = pipeRegs->getMemoryReg();
    PipeReg * wreg = pipeRegs->getWritebackReg();
    uint64_t stat = ereg->get(E_STAT);
    uint64_t icode = ereg->get(E_ICODE);
    uint64_t valA = ereg->get(E_VALA);
    e_dstE = ereg->get(E_DSTE);
    uint64_t dstM = ereg->get(E_DSTM);
    //lab7 
    //Add a statement to set e_valE to E_valC.  The value of e_valE is then stored in M_valE.
    e_valE = ereg->get(E_VALC);

    uint64_t W_stat = wreg->get(W_STAT);
    //lab 8
    bool e_cc = set_CC(icode, m_stat, W_stat); //check if icode is opcode
    uint64_t AluA = setAluA(icode, ereg); //get AluA
    uint64_t AluB = setAluB(icode, ereg); //get AluB
    uint64_t AluFun = setAluFun(icode, ereg); //get instruction

    //uint64_t valE = ALU(AluFun, AluA, AluB, e_cc);
    e_valE = ALU(AluFun, AluA, AluB, e_cc); //peform operations
    //setMInput(mreg, stat, icode, 0, e_valE, valA, dstE, dstM); 

    //lab10 
    m_bubble = calculateControlSignals(m_stat, W_stat);

    uint64_t ifun = ereg->get(E_IFUN);
    e_Cnd = Cond(icode, ifun);
    e_dstE = setDstE(icode, e_Cnd, ereg);
    setMInput(mreg, stat, icode, e_Cnd, e_valE, valA, e_dstE, dstM);  //pass to memory register

    return false;
}

/** doClockHigh
 * @param pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/
void ExecuteStage::doClockHigh(PipeRegArray * pipeRegs)
{ 
    PipeReg * mreg = pipeRegs->getMemoryReg();
    if(m_bubble == true)
    {
        ((M *)mreg)->bubble();
    }
    else
    {
        mreg->normal();
    }
}

/** setMInput
 * 
 * provides the input to potentially be stored in the M register
 * 
 * @param mreg - pointer to memory register
 * @param stat - value to be stored in the stat pipeline register within M
 * @param icode - value of i code to be stoed in i code pipleline register within M
 * @param cnd - value of condition code to be stored in cnd pipeline register within M
 * @param valE - value to be stored in the valE pipeline register within M
 * @param valA - value to be stored in the valA pipeline register within M
 * @param dstE - value to be stored in the dstE pipeLine register within M
 * @param dstM - value to be stored in the dstM pipeline register within M
 * 
*/
void ExecuteStage::setMInput(PipeReg * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM)
{
      mreg->set(M_STAT, stat);
      mreg->set(M_ICODE, icode);
      mreg->set(M_CND, cnd);
      mreg->set(M_VALE, valE);
      mreg->set(M_VALA, valA);
      mreg->set(M_DSTE, dstE);
      mreg->set(M_DSTM, dstM);
}

/** setAluA
 * 
 * @param E_icode - icode that is used set the instruction.
 * @param ereg - Execute register that is used to obtain the value of VALA.
 * @return - returns the value obtained from ereg that is set to ALUA
 * 
 *  HCL for ALU A component
 *   word aluA = [
 *   E_icode in { IRRMOVQ, IOPQ } : E_valA;
 *   E_icode in { IIRMOVQ, IRMMOVQ, IMRMOVQ } : E_valC;
 *   E_icode in { ICALL, IPUSHQ } : -8;
 *   E_icode in { IRET, IPOPQ } : 8;
 *   1: 0;
*  ];
*/
uint64_t ExecuteStage::setAluA(uint64_t E_icode, PipeReg * ereg)
{
    
    if(E_icode == Instruction::IRRMOVQ || E_icode == Instruction:: IOPQ)
    {
        return ereg->get(E_VALA);
    }
    else if(E_icode == Instruction::IIRMOVQ || E_icode == Instruction::IRMMOVQ || E_icode == Instruction::IMRMOVQ)
    {
        return ereg->get(E_VALC);
    }
    else if(E_icode == Instruction::ICALL || E_icode == Instruction::IPUSHQ)
    {
        return -8;
    }
    else if(E_icode == Instruction::IRET || E_icode == Instruction::IPOPQ) 
    {
        return 8;
    }
    return 0;

}


/** setAluA
 * 
 * @param E_icode - icode that is used set the instruction.
 * @param ereg - Execute register that is used to obtain the value of VALB.
 * @return - returns the value obtained from ereg that is set to ALUB
 * 
 *  //HCL for ALU B component
 *   word aluB = [s
 *   E_icode in { IRMMOVQ, IMRMOVQ, IOPQ, ICALL, IPUSHQ, IRET, IPOPQ } : E_valB;
 *   E_icode in { IRRMOVQ, IIRMOVQ } : 0;
 *   1: 0;
 *   ];
 * 
*/
uint64_t ExecuteStage::setAluB(uint64_t E_icode, PipeReg * ereg)
{
    if(E_icode == Instruction::IRMMOVQ || E_icode == Instruction::IMRMOVQ || E_icode == Instruction::IOPQ 
        || E_icode == Instruction::ICALL || E_icode == Instruction::IPUSHQ 
            || E_icode == Instruction::IRET || E_icode == Instruction::IPOPQ)   
    {
        return ereg->get(E_VALB);
    }
    else if(E_icode == Instruction::IRRMOVQ || E_icode == Instruction::IIRMOVQ) 
    {
        return 0;
    }
    return 0;
    
}
 
/** setAluFun
 * 
 * Sets a value to tell the Alu what kind of operation is going to be peformed.
 * 
 * @param E_icode - icode that is used set the instruction
 * @param ereg - Execute register that is used to get the value of ifun.
 * @return - returns the value of ifun obtained from ereg.
 * 
*/
uint64_t ExecuteStage::setAluFun(uint64_t E_icode, PipeReg * ereg)
{
    if(E_icode == Instruction::IOPQ)
    {
        return ereg->get(E_IFUN);
    }
    return Instruction::ADDQ;
}

/** set_CCs
 *
 * returns 1 or 0 to tell the hardware whether Condition codes are going to be set or not
 * 
 * @param E_icode - icode that is used set the instruction
 * @return - returns true if the icode is an opcode, else false.
 * 
 * bool set_cc = (E_icode == IOPQ) && j
              !m_stat in { SADR, SINS, SHLT} && !W_stat in { SADR, SINS, SHLT };
*/
bool ExecuteStage::set_CC(uint64_t E_icode, uint64_t m_stat, uint64_t W_stat)
{
    return (E_icode == Instruction::IOPQ) && 
           (m_stat != Status::SADR && m_stat != Status::SINS && m_stat != Status::SHLT) && 
           (W_stat != Status::SADR && W_stat != Status::SINS && W_stat != Status::SHLT);
}

/** setDstE
 * 
 * @param E_icode - Used to check if if ICODE is equal to IRRMOVQ?
 * @param E_Cnd - Execute condition code used to check iof 
 * @param ereg - returns register E_DSTE
 * @return - returns DSTE value 
 * 
*/
uint64_t ExecuteStage::setDstE(uint64_t E_icode, uint64_t E_Cnd, PipeReg * ereg)
{
    if((E_icode == Instruction::IRRMOVQ) && !e_Cnd)
    {
        return RegisterFile::RNONE;
    }
    return ereg->get(E_DSTE);
}

/** CC
 * 
 *  @param val - 1 or 0 this value is what sets the flags for OF SF ZF
 *  @param setCCVal - bool value that is obtained from set_CC method. Used to check if icode is opcode.
 *  @param ccNum - constant value passed in to tell setConditionCodes what kind of flag needs to be set.
 * 
*/
void ExecuteStage::CC(uint64_t val, bool setCCVal, uint64_t ccNum)
{
   bool error = false;
   if(setCCVal == true)
   {
        cc->setConditionCode(val, ccNum, error);
   }
}

/** ALU
 * 
 * ALU is used to peform arthimetic and bitwise operations in HCL
 * 
 * @param aluFunVal - value of the op code obtained from AluFun method
 * @param AluA - value A that is used to perform add/sub/xor/and
 * @param AluB - val B that is used to peform add/sub/xor/and
 * @param setCC - boolean value passed that checks if i_code is opcode
 * @return - returns the value result obtained by perform operations from AluA and Alub
 * 
*/
uint64_t ExecuteStage::ALU(uint64_t aluFunVal, uint64_t AluA, uint64_t AluB, bool setCC)
{
    uint64_t result;
    //ADD CASE
    if(aluFunVal == Instruction::ADDQ)
    {
        result = AluA + AluB;
        
        //check OF
        if(Tools::addOverflow(AluA, AluB))
        {
            CC(1, setCC, ConditionCodes::OF);
        }
        else
        {
            CC(0, setCC, ConditionCodes::OF);
        }

        //check SF
        if(Tools::sign(result) == 1)
        {
            CC(1, setCC, ConditionCodes::SF);
        }
        else
        {
             CC(0, setCC, ConditionCodes::SF);
        }

        //check ZF
        if(result == 0)
        {
            CC(1, setCC, ConditionCodes::ZF);
        }
        else
        {
            CC(0, setCC, ConditionCodes::ZF);
        }

    }
    else if(aluFunVal == Instruction::SUBQ) //SUB CASE
    {
        result = AluB - AluA;
        //check OF
        if(Tools::subOverflow(AluA, AluB))
        {
            CC(1, setCC, ConditionCodes::OF);
        }
        else
        {
            CC(0, setCC, ConditionCodes::OF);
        }

        //check SF
        if(Tools::sign(result) == 1)
        {
            CC(1, setCC, ConditionCodes::SF);
        }
        else
        {
             CC(0, setCC, ConditionCodes::SF);
        }

        //check ZF
        if(result == 0)
        {
            CC(1, setCC, ConditionCodes::ZF);
        }
        else
        {
            CC(0, setCC, ConditionCodes::ZF);
        }

    }
    else if(aluFunVal == Instruction::XORQ) //XOR CASE
    {
        result = AluA ^ AluB;

        //check SF
        if(Tools::sign(result) == 1)
        {
            CC(1, setCC, ConditionCodes::SF);
        }
        else
        {
             CC(0, setCC, ConditionCodes::SF);
        }

        //check ZF
        if(result == 0)
        {
            CC(1, setCC, ConditionCodes::ZF);
        }
        else
        {
            CC(0, setCC, ConditionCodes::ZF);
        }

    } 
    else //AND CASE
    {
        result = AluA & AluB;

        //check SF
        if(Tools::sign(result) == 1)
        {
            CC(1, setCC, ConditionCodes::SF);
        }
        else
        {
             CC(0, setCC, ConditionCodes::SF);
        }

        //check ZF
        if(result == 0)
        {
            CC(1, setCC, ConditionCodes::ZF);
        }
        else
        {
            CC(0, setCC, ConditionCodes::ZF);
        }
    }
    return result;
 }

/** Cond 
 * 
 * Method that produces the value for jmps and condition moves, based off of flags.
 * 
 * @param icode - Used to check what kind of instruction.
 * @param ifun -  used to check what kind of function code.
 * @returns - returns the value of E_cnd.
*/
uint64_t ExecuteStage::Cond(uint64_t icode, uint64_t ifun)
{
    bool error = false;
    if(icode != Instruction::IJXX && icode != Instruction::ICMOVXX) 
    {
        return 0;
    }
    bool of = cc->getConditionCode(ConditionCodes::OF, error);
    bool zf = cc->getConditionCode(ConditionCodes::ZF, error);
    bool sf = cc->getConditionCode(ConditionCodes::SF, error);
    if(ifun == Instruction::UNCOND)
    {
        return 1;
    }
    else if(ifun == Instruction::LESSEQ) 
    {
        return ((sf ^ of) | zf);
    }
    else if(ifun == Instruction::LESS)
    {
        return (sf ^ of);
    }
    else if(ifun == Instruction::EQUAL)
    {
        return zf;
    }
    else if(ifun == Instruction::NOTEQUAL)
    {   
        return !zf;
    }
    else if(ifun == Instruction::GREATER)
    {
        return (!(sf ^ of) & !zf);
    }
    else //greater than or equal to zero
    {
        return !(sf ^ of);
    }   
}

/** calculateControlSignals
 * 
 * returns based off the values of m_stat, w_stat.
 * 
 * @param m_stat - checks memory stage status.
 * @param w_stat - checks writeback stage status.
 * @return returns true based off of logic from hcl.
 * 
 * bool M_bubble = m_stat in { SADR, SINS, SHLT } || W_stat in { SADR, SINS, SHLT };
 * 
 * 
*/
bool ExecuteStage::calculateControlSignals(uint64_t m_stat, uint64_t w_stat)
{
     if((m_stat == Status::SADR || m_stat == Status::SINS || m_stat == Status::SHLT) 
        || (w_stat == Status::SADR || w_stat == Status::SINS || w_stat == Status::SHLT))
    {
        return true;
    }
    return false;
} 
