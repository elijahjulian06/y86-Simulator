#makefile to compile the .C files in this directory
#and write the .o output to the ../obj directory
CC = g++
INC = ../inc
BIN = ../bin
OBJD = ../obj
CFLAGS = -g -c -Wall -std=c++11 -I $(INC)

#eli's code
OBJS = $(OBJD)/ConditionCodes.o $(OBJD)/Loader.o $(OBJD)/Memory.o $(OBJD)/RegisterFile.o $(OBJD)/String.o $(OBJD)/Tools.o $(OBJD)/yess.o\
	$(OBJD)/CPU.o $(OBJD)/D.o $(OBJD)/E.o $(OBJD)/F.o $(OBJD)/M.o $(OBJD)/W.o $(OBJD)/FetchStage.o $(OBJD)/DecodeStage.o $(OBJD)/ExecuteStage.o\
	$(OBJD)/MemoryStage.o $(OBJD)/WritebackStage.o $(OBJD)/PipeRegArray.o $(OBJD)/PipeReg.o $(OBJD)/PipeRegField.o $(OBJD)/StageArray.o\
	$(OBJD)/Stage.o

#rule to match .C with object in another directory
$(OBJD)/%.o: %.C 
	@mkdir -p $(OBJD)
	$(CC) $(CFLAGS) $< -o $(OBJD)/$@

# 1) need a macro for the .o files corresponding to the .C 
#    files in this directory (note they will be stored in the ../obj
#    directory: OBJS = ....
 
$(OBJD)/ConditionCodes.o: $(INC)/ConditionCodes.h $(INC)/Tools.h

$(OBJD)/Memory.o: $(INC)/Memory.h $(INC)/Tools.h

$(OBJD)/RegisterFile.o: $(INC)/RegisterFile.h

$(OBJD)/Tools.o: $(INC)/Tools.h 


#lab 5
yess: $(OBJS)
	$(CC) $(OBJS) -o $(BIN)/yess

$(OBJD)/Loader.o: $(INC)/Memory.h $(INC)/String.h $(INC)/Loader.h $(INC)/Tools.h

$(OBJD)/String.o: $(INC)/String.h

$(OBJD)/yess.o: $(INC)/Memory.h $(INC)/Loader.h $(INC)/CPU.h


#lab 6
$(OBJD)/CPU.o: $(INC)/CPU.h $(INC)/PipeRegArray.h $(INC)/StageArray.h $(INC)/Memory.h\
 $(INC)/ConditionCodes.h $(INC)/RegisterFile.h

$(OBJD)/D.o: $(INC)/D.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h\
 $(INC)/Status.h

$(OBJD)/E.o: $(INC)/E.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h\
 $(INC)/Status.h

$(OBJD)/F.o: $(INC)/F.h $(INC)/PipeRegField.h

$(OBJD)/M.o: $(INC)/M.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h\
 $(INC)/Status.h

$(OBJD)/W.o: $(INC)/W.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h\
 $(INC)/Status.h

$(OBJD)/FetchStage.o: $(INC)/PipeRegArray.h $(INC)/PipeReg.h $(INC)/Memory.h \
 $(INC)/FetchStage.h $(INC)/Instruction.h $(INC)/RegisterFile.h $(INC)/Status.h \
 $(INC)/F.h $(INC)/D.h $(INC)/M.h $(INC)/W.h $(INC)/Tools.h $(INC)/Memory.h $(INC)/E.h

$(OBJD)/DecodeStage.o: $(INC)/DecodeStage.h $(INC)/PipeRegArray.h $(INC)/E.h $(INC)/D.h \
	$(INC)/Instruction.h $(INC)/Stage.h $(INC)/RegisterFile.h $(INC)/PipeRegField.h \
		$(INC)/Status.h $(INC)/Stage.h $(INC)/M.h $(INC)/W.h

$(OBJD)/ExecuteStage.o: $(INC)/ExecuteStage.h $(INC)/PipeRegArray.h $(INC)/M.h $(INC)/E.h \
	$(INC)/RegisterFile.h $(INC)/Instruction.h $(INC)/ConditionCodes.h $(INC)/Tools.h $(INC)/Status.h \
	$(INC)/W.h
 
$(OBJD)/MemoryStage.o: $(INC)/MemoryStage.h $(INC)/PipeRegArray.h $(INC)/M.h $(INC)/W.h $(INC)/Instruction.h

$(OBJD)/WritebackStage.o: $(INC)/WritebackStage.h $(INC)/PipeRegArray.h $(INC)/W.h $(INC)/Status.h \
	$(INC)/Stage.h

$(OBJD)/PipeRegArray.o: $(INC)/PipeReg.h $(INC)/PipeRegArray.h $(INC)/F.h $(INC)/D.h \
 $(INC)/E.h $(INC)/M.h $(INC)/W.h

$(OBJD)/PipeReg.o: $(INC)/PipeReg.h

$(OBJD)/PipeRegField.o: $(INC)/PipeRegField.h

$(OBJD)/StageArray.o: $(INC)/StageArray.h $(INC)/Stage.h $(INC)/FetchStage.h \
 $(INC)/DecodeStage.h $(INC)/ExecuteStage.h $(INC)/MemoryStage.h $(INC)/WritebackStage.h

$(OBJD)/Stage.o: $(INC)/Memory.h $(INC)/RegisterFile.h $(INC)/ConditionCodes.h \
 $(INC)/Stage.h

clean:
	rm -f $(OBJD)/*.o