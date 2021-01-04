/*
* Author: 李国豪
* Date:2021/1/3
* description:抽象语法树定义
* latest date:2021/1/4
*/

#include "ast.h"
#include "../constant.h"
#include "../Lexer/token.h"

namespace llvmRustCompiler {

	Function* getFunction(std::string Name) {
		// 首先看模块中是否添加此函数
		if (auto* F = TheModule->getFunction(Name))
			return F;

		// 寻找合适函数，如果没有找到，则对函数原型进行代码生成
		auto FI = FunctionProtos.find(Name);
		if (FI != FunctionProtos.end())
			return FI->second->codegen();

		// 如果不存在函数，返回空指针
		return nullptr;
	}

	/// CreateEntryBlockAlloca - Create an alloca instruction in the entry block of
	/// the function.  This is used for mutable variables etc.
	static AllocaInst* CreateEntryBlockAlloca(Function* TheFunction,
		StringRef VarName) {
		IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
			TheFunction->getEntryBlock().begin());
		return TmpB.CreateAlloca(Type::getDoubleTy(TheContext), nullptr, VarName);
	}

	Value* FPNumberExprAST::codegen() {
		return ConstantFP::get(TheContext, APFloat(Val));
	}

	Value* VariableExprAST::codegen() {

	}

	Value* UnaryExprAST::codegen() {

	}

	Value* BinaryExprAST::codegen() {

	}

	Value* CallExprAST::codegen() {

	}

	Value* IfExprAST::codegen() {

	}

	Value* ForExprAST::codegen() {

	}

	Value* VarExprAST::codegen() {

	}

	Function* PrototypeAST::codegen() {

	}

	Function* FunctionAST::codegen() {

	}
}
