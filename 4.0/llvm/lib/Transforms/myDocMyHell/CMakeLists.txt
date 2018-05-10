message(STATUS "!!!!!!!!!!!!!BANANA   !!!!!!!!!!!!")

add_llvm_loadable_module(LLVMLeandro
  AcceleratorAnalysis.cpp
  SDCModuloScheduler.cpp
  utils.cpp
  ILPModuloScheduler.cpp
  NIModuloScheduler.cpp
  SMModuloScheduler.cpp
  GAModuloScheduler.cpp
  LoopSelect.cpp
)
#link_directories(/opt/gurobi/lib)
#include_directories(/opt/gurobi/include)
target_link_libraries(LLVMLeandro LLVMCodeGen)
#set_target_properties(LLVMLeandro PROPERTIES
#    COMPILE_FLAGS "-L/opt/gurobi/lib/ -lgurobi_c++ -L/opt/gurobi/lib/ -lgurobi75"
#)
#target_link_libraries(${LLVMLeandro} /opt/gurobi/lib/lgurobi_c++ /opt/gurobi/lib/lgurobi75)
