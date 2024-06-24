#include "PipeRegArray.h"
#include "Stage.h"

#ifndef MEMORYSTAGE_H
#define MEMORYSTAGE_H
class MemoryStage: public Stage
{
   private:
      //TODO: provide declarations for new methods
      void setWInput(PipeReg * wreg, uint64_t stat, uint64_t icode, uint64_t valE, uint64_t valM, uint64_t dstE, uint64_t dstM);
      uint64_t addrComp(uint64_t icode, uint64_t m_valE, uint64_t m_valA);
      bool memRead(uint64_t icode);
      bool memWrite(uint64_t icode);
   public:
      //These are the only methods called outside of the class
      bool doClockLow(PipeRegArray * pipeRegs);
      void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
