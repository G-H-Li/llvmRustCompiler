#pragma once
/*
* Author: 李国豪
* Date:2021/1/4
* description:静态常量
* latest date:2021/1/4
*/

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "Parser/ast.h"
#include <map>
#include <string>
#include <memory>

using namespace llvm;

namespace llvmRustCompiler
{
	static LLVMContext TheContext;
	static IRBuilder<> Builder(TheContext);
	static std::unique_ptr<Module> TheModule;
	static std::map<std::string, AllocaInst*> NamedValues;
	static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;
}