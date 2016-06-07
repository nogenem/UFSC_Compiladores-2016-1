#include "llvm/IR/DerivedTypes.h"   //for types
#include "llvm/IR/IRBuilder.h"      //for Builder
#include "llvm/IR/LLVMContext.h"    //for Context
#include "llvm/IR/Module.h"         //for Module
#include "llvm/ExecutionEngine/ExecutionEngine.h" //for JIT
#include "llvm/ExecutionEngine/JIT.h"             //for JIT

#include "llvm/Analysis/Verifier.h"

#include <iostream>
#include <vector>

/*
  TODO:
    funcao booleana teste(booleano x, booleano y, booleano z)
      retorna (x & y) | (x & z) | (~z & ~y)

    teste(falso, falso, verdadeiro)

    funcao real exemplo (real a, real b)
      retorna (a*b) - 2.0 * (a mod b)

    exemplo(10.0, 5.0)

    static Type* getDoubleTy(context)
*/
int main(){
  llvm::LLVMContext &Context = llvm::getGlobalContext();
  llvm::Module *OurModule;
  llvm::IRBuilder<> Builder(Context);

  OurModule = new llvm::Module("Our first intermediary code", Context);

  // main
  llvm::Type* intType = llvm::Type::getInt64Ty(Context);
  llvm::FunctionType* typeOfMain = llvm::FunctionType::get(intType,false);

  llvm::Function* ourMain = llvm::Function::Create(typeOfMain,llvm::Function::ExternalLinkage, "main", OurModule);

  llvm::BasicBlock *mainBB = llvm::BasicBlock::Create(Context, "entry", ourMain);

  Builder.SetInsertPoint(mainBB);

  // constants
  auto Zero_I = llvm::ConstantInt::get(Context, llvm::APInt(1, 0));
  auto One_I = llvm::ConstantInt::get(Context, llvm::APInt(1, 1));
  auto Two_D = llvm::ConstantFP::get(Context, llvm::APFloat(2.0));
  auto Five_D = llvm::ConstantFP::get(Context, llvm::APFloat(5.0));
  auto Ten_D = llvm::ConstantFP::get(Context, llvm::APFloat(10.0));

  // teste
  auto boolType = llvm::Type::getInt1Ty(Context);
  std::vector<llvm::Type*> TesteParameters(3, boolType);
  llvm::FunctionType* typeOfTeste = llvm::FunctionType::get(boolType, TesteParameters, false);

  llvm::Function* ourTeste = llvm::Function::Create(typeOfTeste,llvm::Function::ExternalLinkage, "teste", OurModule);

  llvm::Function::arg_iterator params = ourTeste->arg_begin();
  params->setName("x");
  ++params;
  params->setName("y");
  ++params;
  params->setName("z");

  llvm::BasicBlock *testeBB = llvm::BasicBlock::Create(Context, "tst", ourTeste);
  Builder.SetInsertPoint(testeBB);

  auto x = ourTeste->arg_begin();
  auto y = ++(ourTeste->arg_begin());
  auto z = ++(++(ourTeste->arg_begin()));

  auto xANDy = Builder.CreateAnd(x,y,"xANDy");
  auto xANDz = Builder.CreateAnd(x,z,"xANDz");
  auto notZ = Builder.CreateNot(z, "notZ");
  auto notY = Builder.CreateNot(y, "notY");
  auto zANDy = Builder.CreateAnd(notZ, notY,"nzANDny");
  auto tmp = Builder.CreateOr(xANDy, xANDz, "ortmp");
  tmp = Builder.CreateOr(tmp, zANDy, "ortmp");
  Builder.CreateRet(tmp);

  llvm::verifyFunction(*ourTeste);

  // main
  Builder.SetInsertPoint(mainBB);

  std::vector<llvm::Value*> arguments;
  arguments.push_back(Zero_I);
  arguments.push_back(Zero_I);
  arguments.push_back(One_I);
  auto tstReturn = Builder.CreateCall(ourTeste, arguments, "calltst");

  auto intRet = Builder.CreateIntCast(tstReturn, intType, false);
  Builder.CreateRet(intRet);

  llvm::verifyFunction(*ourMain);
  OurModule->dump();

  llvm::ExecutionEngine* OurExecutionEngine;
  std::string Error;
  LLVMInitializeNativeTarget(); //target = generates code for my processor
  OurExecutionEngine = llvm::EngineBuilder(OurModule).setErrorStr(&Error).create();
  if (!OurExecutionEngine) {
      fprintf(stderr, "Could not create OurExecutionEngine: %s\n", Error.c_str());
      exit(1);
  }

  // JIT our main. It returns a function pointer.
  void *mainPointer = OurExecutionEngine->getPointerToFunction(ourMain);
  // Translate the pointer and run our main to get its results
  int (*result)() = (int (*)())(intptr_t)mainPointer;
  std::cout << "Result of our main = " << result() << std::endl;

  return 0;
}
