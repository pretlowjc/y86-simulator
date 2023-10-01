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
OBJS = $(OBJD)/ConditionCodes.o  $(OBJD)/Memory.o $(OBJD)/RegisterFile.o  $(OBJD)/Tools.o

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

all: $(OBJS)


# 3) need a .o target for each .C
#    Remember each of these is dependent upon the .h files included by the .C
$(OBJD)/ConditionCodes.o : $(INC)/ConditionCodes.h
$(OBJD)/Memory.o : $(INC)/Memory.h
$(OBJD)/RegisterFile.o : $(INC)/RegisterFile.h
$(OBJD)/Tools.o : $(INC)/Tools.h

# 4) need a clean target to delete the .o files corresponding to
#    the .C files in this directory from the ../obj directory

clean: 
	rm -f $(OBJS)