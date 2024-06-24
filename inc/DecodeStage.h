#include "PipeRegArray.h"
#include "Stage.h"

#ifndef DECODESTAGE_H
#define DECODESTAGE_H
class DecodeStage: public Stage
{
   private:
    //TODO: provide declarations for new methods
   bool e_bubble;
   void setEInput(PipeReg * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                            uint64_t valC, uint64_t valA, uint64_t valB, uint64_t destE, uint64_t destM,
                            uint64_t srcA, uint64_t srcB);
    uint64_t srcAComp(uint64_t icod, PipeReg * dreg);
    uint64_t srcBComp(uint64_t icode, PipeReg * dreg);
    uint64_t dstEComp(uint64_t icode, PipeReg * dreg);
    uint64_t dstMComp(uint64_t icode, PipeReg * dreg);
    uint64_t registerValAComp(uint64_t d_rvalA);
    uint64_t registerValBComp(uint64_t d_rvalB);
    uint64_t setDValA(uint64_t d_icode, uint64_t d_srcA, uint64_t d_rvalA, uint64_t d_valP, PipeReg * mreg, PipeReg * wreg);
    uint64_t setDValB(uint64_t d_srcB, uint64_t d_rvalB, PipeReg * mreg, PipeReg * wreg);
    bool calculateControlSignals(uint64_t e_icode, uint64_t E_dstM);
   public:
      //These are the only methods called outside of the class
      bool doClockLow(PipeRegArray * pipeRegs);
      void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
