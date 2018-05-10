#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <llvm/Pass.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/ADT/Statistic.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#include <llvm/Transforms/Scalar.h>

//use namespace std;
//using namespace llvm;

#include <vector>

struct LoopData{
	unsigned instCount, binaryInstCount, startLine;
	llvm::Loop *l;
};
