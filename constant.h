#pragma once
/*
* Author: 李国豪
* Date:2021/1/4
* description:静态常量
* latest date:2021/1/4
*/

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "Parser/ast.h"
#include "Generator/rustJIT.h"
#include <map>
#include <string>
#include <memory>

using namespace llvm;
using namespace llvm::orc;

namespace llvmRustCompiler
{
	//对应每个LLVM的线程
	static LLVMContext TheContext;
	//指令生成的辅助对象，用于跟踪插入指令和生成新指令
	static IRBuilder<> Builder(TheContext);
	//皴法代码段中所有函数和全局变量的结构
	static std::unique_ptr<Module> TheModule;
	//记录代码的符号表
	static std::map<std::string, AllocaInst*> NamedValues;
	// Pass 管理
	static std::unique_ptr<legacy::FunctionPassManager> TheFPM;
	// JIT
	static std::unique_ptr<RustJIT> TheJIT;
	// 函数映射
	static std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

	static void InitializeModuleAndPassManager() {
		// Open a new module.
		TheModule = std::make_unique<Module>("my cool jit", TheContext);
		TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());

		// Create a new pass manager attached to it.
		TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());

		// Do simple "peephole" optimizations and bit-twiddling optzns.
		TheFPM->add(createInstructionCombiningPass());
		// Reassociate expressions.
		TheFPM->add(createReassociatePass());
		// Eliminate Common SubExpressions.
		TheFPM->add(createGVNPass());
		// Simplify the control flow graph (deleting unreachable blocks, etc).
		TheFPM->add(createCFGSimplificationPass());

		TheFPM->doInitialization();
	}

}