add_llvm_loadable_module( LLVMLeandro
  HwOnly.cpp
  MemProf.cpp
  PostLTO.cpp
  PreLTO.cpp
  LegUpProfile.cpp
  SwOnly.cpp
  utils.cpp
  WatchVariables.cpp
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
