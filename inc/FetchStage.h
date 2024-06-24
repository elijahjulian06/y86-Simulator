#include "PipeRegArray.h"
#include "PipeReg.h"
#include "Stage.h"

#ifndef FETCHSTAGE_H
#define FETCHSTAGE_H
class FetchStage: public Stage
{
   private:
      //TODO: provide declarations for new methods
        bool F_stall;
        bool D_stall;
        bool D_bubble;
        void setDInput(PipeReg * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);

        uint64_t selctPC(PipeRegArray * piepRegs);
        bool needRegIds(uint64_t f_icode);    
        bool needValC(uint64_t f_icode);
        uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
        uint64_t PCincrement(uint64_t f_pc, bool needregId, bool needvalC);
        void getRegIds(uint64_t regByte, uint64_t &rA, uint64_t &rB);
        uint64_t buildValC(uint64_t f_pc, bool needregid, bool &mem_error);
        bool instr_valid(uint64_t icode);
        uint64_t statHCL(bool mem_error, bool instr_valid, uint64_t icode);
        uint64_t icodeHCL(bool mem_error, uint64_t mem_icode);
        uint64_t ifunHCL(bool mem_error, uint64_t mem_ifun);
        bool setFStall(uint64_t E_icode, uint64_t E_dstM, uint64_t D_icode, uint64_t M_icode);
        bool setDStall(uint64_t E_icode, uint64_t E_dstM);
        void calculateControlSignals(uint64_t E_icode, uint64_t E_dstM, uint64_t D_icode, uint64_t M_icode);
        bool setDBubble(uint64_t E_icode, uint64_t E_dstM, uint64_t D_icode, uint64_t M_icode);
   public:
         //These are the only methods called outside of the class
         bool doClockLow(PipeRegArray * pipeRegs);
         void doClockHigh(PipeRegArray * pipeRegs);
};
#endif
