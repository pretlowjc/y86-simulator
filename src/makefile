#makefile to compile the .C files in this directory
#and write the .o output to the ../obj directory
CC = g++
INC = ../inc
BIN = ../bin
OBJD = ../obj
CFLAGS = -g -c -Wall -std=c++11 -I $(INC)
# 1) need a macro for the .o files corresponding to the .C 
#    files in this directory (note they will be stored in the ../obj
#    directory: OBJS = ....
OBJS = $(OBJD)/ConditionCodes.o  $(OBJD)/Memory.o $(OBJD)/RegisterFile.o  $(OBJD)/Tools.o $(OBJD)/Loader.o $(OBJD)/String.o $(OBJD)/yess.o $(OBJD)/CPU.o $(OBJD)/PipeReg.o $(OBJD)/PipeRegArray.o $(OBJD)/PipeRegField.o $(OBJD)/MemoryStage.o $(OBJD)/StageArray.o $(OBJD)/WritebackStage.o $(OBJD)/W.o $(OBJD)/F.o $(OBJD)/D.o $(OBJD)/M.o $(OBJD)/E.o $(OBJD)/FetchStage.o $(OBJD)/DecodeStage.o $(OBJD)/ExecuteStage.o $(OBJD)/Stage.o


#rule to match .C with object in another directory
$(OBJD)/%.o: %.C 
	@mkdir -p $(OBJD)
	$(CC) $(CFLAGS) $< -o $(OBJD)/$@


# 2) need a target (with a name that doesn't match a file; for example, all:)
#    that is dependent upon the .o files. With this target, you can type:
#    make all
#    and all of the .C files will be compiled. 
#    Note: all is a phony target.  It doesn't correspond to file.  
#    No executable is created. We need it just to be able to compile 
#    all of the .C files with one make command.

yess:
	make all
	$(CC) $(OBJS)  -o $(BIN)/yess
	

all: $(OBJS)





# 3) need a .o target for each .C
#    Remember each of these is dependent upon the .h files included by the .C
## gotta continue adding rest of includes
$(OBJD)/yess.o : $(INC)/Memory.h $(INC)/CPU.h $(INC)/Loader.h
$(OBJD)/ConditionCodes.o : $(INC)/ConditionCodes.h $(INC)/Tools.h
$(OBJD)/Memory.o : $(INC)/Memory.h $(INC)/Tools.h
$(OBJD)/RegisterFile.o : $(INC)/RegisterFile.h
$(OBJD)/Tools.o : $(INC)/Tools.h $(INC)/String.h 
$(OBJD)/Loader.o : $(INC)/Loader.h $(INC)/Memory.h
$(OBJD)/String.o : $(INC)/String.h
$(OBJD)/CPU.o : $(INC)/CPU.h
$(OBJD)/PipeReg.o : $(INC)/PipeReg.h
$(OBJD)/PipeRegArray.o : $(INC)/PipeRegArray.h 
$(OBJD)/PipeRegField.o : $(INC)/PipeRegField.h
$(OBJD)/MemoryStage.o : $(INC)/MemoryStage.h
$(OBJD)/StageArray.o : $(INC)/StageArray.h
$(OBJD)/WritebackStage.o : $(INC)/WritebackStage.h
$(OBJD)/FetchStage.o : $(INC)/FetchStage.h
$(OBJD)/DecodeStage.o : $(INC)/DecodeStage.h
$(OBJD)/ExecuteStage.o : $(INC)/ExecuteStage.h




$(OBJD)/W.o : $(INC)/W.h
$(OBJD)/F.o : $(INC)/F.h
$(OBJD)/D.o : $(INC)/D.h
$(OBJD)/M.o : $(INC)/M.h
$(OBJD)/E.o : $(INC)/E.h

$(OBJD)/Stage.o : $(INC)/Stage.h
#$(OBJD)/Status.o : $(INC)/Status.h


















# 4) need a clean target to delete the .o files corresponding to
#    the .C files in this directory from the ../obj directory

clean: 
	rm -f $(OBJS)
