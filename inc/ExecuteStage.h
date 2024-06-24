#include "PipeRegArray.h"
#include "Stage.h"

#ifndef EXECUTESTAGE_H
#define EXECUTESTAGE_H
class ExecuteStage: public Stage
{
   private:
      //TODO: provide declarations for new methods
      bool m_bubble;
      void setMInput(PipeReg * mreg, uint64_t stat, uint64_t icode, uint64_t cnd, uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM);
      uint64_t setAluA(uint64_t E_icode, PipeReg * ereg);
      uint64_t setAluB(uint64_t E_icode, PipeReg * ereg);
      uint64_t setAluFun(uint64_t E_icode, PipeReg * ereg);
      bool set_CC(uint64_t E_icode, uint64_t m_stat, uint64_t W_stat);
      uint64_t setDstE(uint64_t E_icode, uint64_t E_Cnd, PipeReg * ereg);
      void CC(uint64_t val, bool setCCVal, uint64_t ccNum);
      uint64_t ALU(uint64_t aluFunVal, uint64_t AluA, uint64_t AluB, bool setCC);
      uint64_t Cond(uint64_t icode, uint64_t ifun);
      bool calculateControlSignals(uint64_t m_stat, uint64_t w_stat);
   public:
      //These are the only methods called outside of the class
      bool doClockLow(PipeRegArray * pipeRegs);
      void doClockHigh(PipeRegArray * pipeRegs);
};
#endif


