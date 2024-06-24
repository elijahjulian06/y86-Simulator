#include "PipeRegArray.h"
#include "DecodeStage.h"
#include "D.h"
#include "E.h"
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "Instruction.h"
#include "Status.h"
#include "Stage.h"
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
bool DecodeStage::doClockLow(PipeRegArray * pipeRegs)
{
/**
*
Your DecodeStage::doClockLow will need to contain calls to each of these methods
as well as calls to RegisterFile::readRegister using d_srcA and d_srcB to get d_rvalA and d_rvalB.
Take a look at the decode stage diagram to see how the results returned by these components are used.
Note that the HCL for Sel+FwdA and FwdB is very incomplete at this point.
We'll finish those up in an upcoming lab.
*/


    PipeReg * dreg = pipeRegs->getDecodeReg();
    PipeReg * ereg = pipeRegs->getExecuteReg();
    PipeReg * mreg = pipeRegs->getMemoryReg();
    PipeReg * wreg = pipeRegs->getWritebackReg();

    uint64_t stat = dreg->get(D_STAT);
    uint64_t icode = dreg->get(D_ICODE);
    uint64_t ifun = dreg->get(D_IFUN);
    uint64_t valC = dreg->get(D_VALC);
    if(icode == Instruction::IHALT)
    { 
        stat = Status::SHLT; 
    }
 
    d_srcA = srcAComp(icode, dreg);
    d_srcB = srcBComp(icode, dreg);

    uint64_t e_dstM = ereg->get(E_DSTM);
    uint64_t d_dstE = dstEComp(icode, dreg);
    uint64_t d_dstM = dstMComp(icode, dreg);
    uint64_t e_icode = ereg->get(E_ICODE);
    e_bubble = calculateControlSignals(e_icode, e_dstM); 

    bool error = false;
    uint64_t d_rvalA = rf->readRegister(d_srcA, error);
    uint64_t d_valP = dreg->get(D_VALP);
    uint64_t d_valA = setDValA(icode, d_srcA, d_rvalA, d_valP, mreg, wreg);

    uint64_t d_rvalB = rf->readRegister(d_srcB, error);
    uint64_t d_valB = setDValB(d_srcB, d_rvalB, mreg, wreg);

  
    setEInput(ereg, stat, icode, ifun, valC, d_valA, d_valB, d_dstE, d_dstM, d_srcA, d_srcB);

    return false;


}

/** doClockHigh
 *
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param pipeRegs - array of the pipeline register (F, D, E, M, W instances)
*/

void DecodeStage::doClockHigh(PipeRegArray * pipeRegs)
{ 
    PipeReg * ereg = pipeRegs->getExecuteReg();
    if(e_bubble == true)
    {
        ((E *)ereg)->bubble();
    }
    else
    {
        ereg->normal();
    }
    //ereg->normal();
}




/** setEInput
 *
 * @param ereg - pointer to the E register instance
 * @param stat - value to be stored in the stat pipeline register within E
 * @param icode - value to be stored in the icode pipeline register within E
 * @param ifun - value to be stored in the ifun pipeline register within E
 * @param valC - value to be stored in the valC pipeline register within E
 * @param valA - value to be stored in the valA pipeline register within E
 * @param valB - value to be stored in the valB pipeline register within E
 * @param destE - value to be stored in the destE pipeline register within E
 * @param destM - value to be stored in the destM pipeline register within E
 * @param srcA - value to be stored in the srcA pipeline register within E
 * @param srcB - value to be stored in the srcB pipeline register within E
 *
 * sets the input values for next stage execute stage
 *
*/
void DecodeStage::setEInput(PipeReg * ereg, uint64_t stat, uint64_t icode, uint64_t ifun,
    uint64_t valC, uint64_t valA, uint64_t valB, uint64_t destE, uint64_t destM,
        uint64_t srcA, uint64_t srcB)
{
    ereg->set(E_STAT, stat);
    ereg->set(E_ICODE, icode);
    ereg->set(E_IFUN, ifun);
    ereg->set(E_VALC, valC);
    ereg->set(E_VALA, valA);
    ereg->set(E_VALB, valB);
    ereg->set(E_DSTE, destE);
    ereg->set(E_DSTM, destM);
    ereg->set(E_SRCA, srcA);
    ereg->set(E_SRCB, srcB);
}


/** srcAComp
 *
 * get the register value for srcA
 * 
 * @param icode - icode that is used to check if srcA should be D_RA or rsp
 * @param dreg - point to Decode Register
 *
 *    word d_srcA = [
 *       D_icode in { IRRMOVQ, IRMMOVQ, IOPQ, IPUSHQ } : D_rA;
 *       D_icode in { IPOPQ, IRET } : RSP;
 *       1: RNONE;  //no register needed
 *    ];
*/
uint64_t DecodeStage::srcAComp(uint64_t icode, PipeReg * dreg)
{
    if(icode == Instruction::IRRMOVQ || icode == Instruction::IRMMOVQ || icode == Instruction::IOPQ || icode == Instruction::IPUSHQ)
    {
        return dreg->get(D_RA);
    }
    if(icode == Instruction::IPOPQ || icode == Instruction::IRET)
    {
        return RegisterFile::rsp;
    }

    return RegisterFile::RNONE;
}


/** srcBComp
 *
 * gets the register value for srcB
 * 
 * @param icode - i-code passed in to set srcB
 * @param dreg - pointer to decode register 
 *
 * HCL for srcB component
 * word d_srcB = [
 * D_icode in { IOPQ, IRMMOVQ, IMRMOVQ } : D_rB;
 * D_icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RSP;
 * 1: RNONE; //no register needed
 * ]; 
**/
uint64_t DecodeStage::srcBComp(uint64_t icode, PipeReg * dreg)
{
    if(icode == Instruction::IOPQ|| icode == Instruction::IRMMOVQ || icode == Instruction::IMRMOVQ)
    {
        return dreg->get(D_RB);
    }
    if(icode == Instruction::IPUSHQ || icode == Instruction::IPOPQ || icode == Instruction::ICALL || icode == Instruction::IRET)
    {
        return RegisterFile::rsp;
    }
    return RegisterFile::RNONE;
}




/** destEComp
 *
 * @param icode - i-code use to set dstE 
 * @param dreg - pointer to Decode register
 *
 * HCL for dstE component
 * word d_dstE = [
 * D_icode in { IRRMOVQ, IIRMOVQ, IOPQ} : D_rB;
 * D_icode in { IPUSHQ, IPOPQ, ICALL, IRET } : RSP;
 * 1: RNONE;
 * ];
*/ 
uint64_t DecodeStage::dstEComp(uint64_t icode, PipeReg * dreg)
{
    if(icode == Instruction::IRRMOVQ || icode == Instruction::IIRMOVQ || icode == Instruction::IOPQ)
    {
        return dreg->get(D_RB);
    } 
    if(icode == Instruction::IPUSHQ || icode == Instruction::IPOPQ || icode == Instruction::ICALL || icode == Instruction::IRET)
    {
        return RegisterFile::rsp;
    }
    return RegisterFile::RNONE;
}




/** dstMComp
 *
 * @param icode - i-code used to set dstM
 * @param dreg - pointer to Decode register
 *
 *
 * HCL for dstM component
 * word d_dstM = [
 * D_icode in { IMRMOVQ, IPOPQ } : D_rA;
 * 1: RNONE;
 * ];
*/
uint64_t DecodeStage::dstMComp(uint64_t icode, PipeReg * dreg)
{
    if(icode == Instruction::IMRMOVQ|| icode == Instruction::IPOPQ)
    {
        return dreg->get(D_RA);
    }
    return RegisterFile::RNONE;
}




/** d_rvalA
 * 
 *  @param d_rvalA - parameter to set d_rvalA -> not fully implemented
 * 
 *  HCL for Sel+FwdA
 * word d_valA = [
 * 1: d_rvalA; //value from register file
 * ];
*/
uint64_t DecodeStage::registerValAComp(uint64_t d_rvalA)
{
    return d_rvalA;
}

/** registerValBComp
 * 
 * @param d_rvalB - parameter to set d_rvalB -> not fully implemented
 *
 * HCL for FwdB
 * word d_valB = [
 * 1: d_rvalB; //value from register file
 * ];
*/
uint64_t DecodeStage::registerValBComp(uint64_t d_rvalB)
{
    return d_rvalB;
}

/** setDValA
 * 
 * @param d_srcA - used to check if dstE is equal to srcA 
 * @param d_rvalA - returned during else case
 * @param mreg - passed into to allow access to memory register
 * @param wreg - passed into to allow access to writback register
 * @return - resturns the fowarded value of valA
 * 
 * word d_valA = [
    d_srcA == e_dstE : e_valE;   # value computed by ExecuteStage
    d_srcA == M_dstE: M_valE;    # value in M register
    d_srcA == W_dstE : W_valE;   # value in W register
    1 : d_rvalA;                 # value from register file
    ];
*/
uint64_t DecodeStage::setDValA(uint64_t d_icode, uint64_t d_srcA, uint64_t d_rvalA, uint64_t d_valP, PipeReg * mreg, PipeReg * wreg)
{
    if(d_icode == Instruction::ICALL || d_icode == Instruction::IJXX)
    {
        return d_valP;
    }
    if (d_srcA == RegisterFile::RNONE) 
    {
        return 0;
    }
    else if(d_srcA == e_dstE)
    {
        return e_valE;
    }
    else if(d_srcA == mreg->get(M_DSTM))
    {
        return m_valM;
    }
    else if(d_srcA == mreg->get(M_DSTE))
    {
        return mreg->get(M_VALE);
    }
    else if(d_srcA == wreg->get(W_DSTM))
    {
        return wreg->get(W_VALM);
    }
    else if(d_srcA == wreg->get(W_DSTE))
    {
        return wreg->get(W_VALE);
    }
    return d_rvalA;

}


/** setDValB
 * 
 * @param dsrcB - used to check if dstE is equal to srcB
 * @param d_rvalB - returned during else case
 * @param mreg - passed into to allow access to memory register
 * @param wreg - passed into to allow access to writback register
 * @return - returns the value for DValB
 * 
    word d_valB = [
    d_srcB == e_dstE : e_valE;   # value computed by ExecuteStage
    d_srcB == M_dstE: M_valE;    # value in M register
    d_srcB == W_dstE : W_valE;   # value in W register
    1 : d_rvalB;                 # value from register file
    ];
 */
uint64_t DecodeStage::setDValB(uint64_t d_srcB, uint64_t d_rvalB, PipeReg * mreg, PipeReg * wreg)
{
    if (d_srcB == RegisterFile::RNONE)
    {
        return 0;
    }
    else if(d_srcB == e_dstE)
    {
        return e_valE;
    }
    else if(d_srcB == mreg->get(M_DSTM))
    {
        return m_valM;
    }
    else if(d_srcB == mreg->get(M_DSTE))
    {
        return mreg->get(M_VALE);
    }
    else if(d_srcB == wreg->get(W_DSTM))
    {
        return wreg->get(W_VALM);
    }
    else if(d_srcB == wreg->get(W_DSTE))
    {
        return wreg->get(W_VALE);
    }
    return d_rvalB;
}

/** calculateControlSignals
 * 
 * returns true to calculate whether the e register needs to bubble.
 * 
 * @param e_icode 
 * @param E_dstM
 * @return return true or false based off of hcl
 * 
 * bool E_bubble = ( E_icode == IJXX && !e_Cnd ) ||  
                ( E_icode in { IMRMOVQ, IPOPQ } &&  E_dstM in { d_srcA, d_srcB } );
*/
bool DecodeStage::calculateControlSignals(uint64_t e_icode, uint64_t E_dstM) //possibly use d_srcA and d_srcB from Stage
{
    return (e_icode == Instruction::IJXX && (!e_Cnd)) || 
           ((e_icode == Instruction::IMRMOVQ || e_icode == Instruction::IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB));
}
