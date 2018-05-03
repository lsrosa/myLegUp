##===- lib/Transforms/LegUp/Makefile -----------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LEVEL = ../../..
LIBRARYNAME = LLVMLeandro
LOADABLE_MODULE = 1
#USEDLIBS = LLVMCodeGen.a LLVMVerilog.a

include $(LEVEL)/Makefile.common
LIBS += -lgurobi_c++ -lgurobi75
#CFLAGS += -I/opt/gurobi/include/ -L/opt/gurobi/lib/ -lgurobi_c++
#CPPFLAGS += -I/opt/gurobi/include/ -L/opt/gurobi/lib/ -lgurobi_c++
CPP.Flags += -I$(LLVM_SRC_ROOT)/lib/Target/Verilog  \
#-L/opt/gurobi/lib/ -lgurobi_c++ -L/opt/gurobi/lib/ -lgurobi75
#-I/opt/gurobi/include/ -L/opt/gurobi/lib/ -lgurobi_c++  \
# if you modify a file in Target/Verilog then you'll need to run 'make' from
# the legup base directory
#-I$(LLVM_SRC_ROOT)/tools/polly/include \
#-I$(LLVM_SRC_ROOT)/../cloog/install/include \
