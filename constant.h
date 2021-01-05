#pragma once
/*
* Author: �����
* Date:2021/1/4
* description:��̬����
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
	//��Ӧÿ��LLVM���߳�
	static LLVMContext TheContext;
	//ָ�����ɵĸ����������ڸ��ٲ���ָ���������ָ��
	static IRBuilder<> Builder(TheContext);
	//�巨����������к�����ȫ�ֱ����Ľṹ
	static std::unique_ptr<Module> TheModule;
	//��¼����ķ��ű�
	static std::map<std::string, AllocaInst*> NamedValues;
	// Pass ����
	static std::unique_ptr<legacy::FunctionPassManager> TheFPM;
	// JIT
	static std::unique_ptr<RustJIT> TheJIT;
	// ����ӳ��
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