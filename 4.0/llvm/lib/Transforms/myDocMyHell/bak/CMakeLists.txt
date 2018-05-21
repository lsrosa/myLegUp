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

target_link_libraries(LLVMLeandro LLVMCodeGen)
