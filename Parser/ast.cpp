/*
* Author: 李国豪
* Date:2021/1/3
* description:抽象语法树定义
* latest date:2021/1/4
*/

#include "ast.h"
#include "../constant.h"

namespace llvmRustCompiler {


	Function* getFunction(std::string Name) {
		// 首先看模块中是否添加此函数
		//if (auto* F = TheModule->getFunction(Name))
			//return F;

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

	Value* IntNumberExprAST::codegen() {
		return ConstantInt::get(TheContext, APInt(Bits, Val, IsSigned));
	}

	Value* VariableExprAST::codegen() {
		// 查看变量名是否被注册
		Value* V = NamedValues[Name];
		if (!V) {
			errorGenerator(std::string("未定义的变量名"));
			return nullptr;
		}
		// Load the value.
		return Builder.CreateLoad(V, Name.c_str());
	}
	
	// 此ast可能废弃，因为rust中并不存在自定义一元操作符的操作
	Value* UnaryExprAST::codegen() {
		Value* OperandV = Operand->codegen();
		if (!OperandV)
			return nullptr;
		// 支持浮点的按位取反
		if (Opcode == '!' && OperandV->getType()->isFloatingPointTy()) {
			return Builder.CreateFNeg(OperandV, "fnegtmp");
		}
		// 以下应该要修改，万花筒值得用户自定义一元操作符函数，但是本项目中只使用一元操作符
		Function* F = getFunction(std::string("unary") + Opcode);
		if (!F) {
			errorGenerator(std::string("未知的一元操作符"));
			return nullptr;
		}
		return Builder.CreateCall(F, OperandV, "unop");
	}
	//获取逻辑值
	Value* getLogicalVal(Value* val) {
		if (val->getType()->isIntegerTy()) {
			return Builder.CreateICmpNE(val, ConstantInt::get(TheContext, APInt(1, 0)));
		}
		else if(val->getType()->isFloatingPointTy())
		{
			return Builder.CreateFCmpONE(
				val, ConstantFP::get(TheContext, APFloat(0.0)));
		}
	}

	Value* BinaryExprAST::codegen() {

		// Special case '=' because we don't want to emit the LHS as an expression.
		if (Op == '='|| Op == '+=' || Op == '-=' || Op == '*=' || Op == '/=' || Op == '%='
			|| Op == '&=' || Op == '|=' || Op == '^=' || Op == '>>=' || Op == '<<=') {
			// 将左部转为变量AST
			VariableExprAST* LHSE = static_cast<VariableExprAST*>(LHS.get());
			if (!LHSE) {
				errorGenerator(std::string("等号左边必须为变量"));
				return nullptr;
			}
			// Codegen the RHS.
			Value* Val = RHS->codegen();
			if (!Val) {
				errorGenerator(std::string("操作符右侧代码生成错误"));
				return nullptr;
			}
			// 检查变量名是否定义
			Value* Variable = NamedValues[LHSE->getName()];
			if (!Variable) {
				errorGenerator(std::string("未定义的变量名"));
				return nullptr;
			}
			// 判断操作符两侧的类型是否一致
			if (Val->getType() != Variable->getType()) {
				errorGenerator(std::string("操作符两侧的类型不一致"));
				return nullptr;
			}
			switch (Op) {
			case '+=':
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateAdd(Variable, Val, "addtmp");
				else if (Variable->getType()->isFloatingPointTy()) Val = Builder.CreateFAdd(Variable, Val, "faddtmp");
				break;
			case '-=':
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateSub(Variable, Val, "subtmp");
				else if (Variable->getType()->isFloatingPointTy()) Val = Builder.CreateFSub(Variable, Val, "fsubtmp");
				break;
			case '*=':
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateMul(Variable, Val, "multmp");
				else if (Variable->getType()->isFloatingPointTy()) Val = Builder.CreateFMul(Variable, Val, "fmultmp");
				break;
			case '/=':
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateSDiv(Variable, Val, "divtmp");
				else if (Variable->getType()->isFloatingPointTy()) Val = Builder.CreateFDiv(Variable, Val, "fdivtmp");
				break;
			case '%=':
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateSRem(Variable, Val, "remtmp");
				else if (Variable->getType()->isFloatingPointTy()) Val = Builder.CreateFRem(Variable, Val, "fremtmp");
				break;
			case '<<=':
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateShl(Variable, Val, "shltmp");
				else {
					errorGenerator(std::string("左移运算必须为整型"));
					return nullptr;
				}
				break;
			case '>>=':
				// 算数右移
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateAShr(Variable, Val, "ashrtmp");
				else {
					errorGenerator(std::string("右移运算必须为整型"));
					return nullptr;
				}
				break;
			case '&=':
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateAnd(Variable, Val, "andtmp");
				else {
					errorGenerator(std::string("与运算必须为整型"));
					return nullptr;
				}
				break;
			case '|=':
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateOr(Variable, Val, "ortmp");
				else {
					errorGenerator(std::string("或运算必须为整型"));
					return nullptr;
				}
				break;
			case '^=':
				if (Variable->getType()->isIntegerTy()) Val = Builder.CreateXor(Variable, Val, "xortmp");
				else {
					errorGenerator(std::string("异或运算必须为整型"));
					return nullptr;
				}
				break;
			default:
				break;
			}
			Builder.CreateStore(Val, Variable);
			return Val;
		}

		Value* L = LHS->codegen();
		Value* R = RHS->codegen();
		if (!L || !R) {
			errorGenerator(std::string("表达式无效"));
			return nullptr;
		}
		//由于&&和||不需要判断类型是否一致，在此处单独运算
		if (Op == '&&' || Op == '||') {
			L = getLogicalVal(L);
			R = getLogicalVal(R);
			switch (Op)
			{
			case '&&':
				return Builder.CreateAnd(L, R, "landtmp");
			case '||':
				return Builder.CreateOr(L, R, "lortmp");
			default:
				break;
			}
		}

		// 判断操作符两侧的类型是否一致
		if (L->getType() != R->getType()) {
			errorGenerator(std::string("操作符两侧的类型不一致"));
			return nullptr;
		}
		
		switch (Op) {
		case '+':
			if (L->getType()->isIntegerTy()) return Builder.CreateAdd(L, R, "addtmp");
			else if (L->getType()->isFloatingPointTy()) return Builder.CreateFAdd(L, R, "faddtmp");
			break;
		case '-':
			if (L->getType()->isIntegerTy()) return Builder.CreateSub(L, R, "subtmp");
			else if(L->getType()->isFloatingPointTy()) return Builder.CreateFSub(L, R, "fsubtmp");
			break;
		case '*':
			if (L->getType()->isIntegerTy()) return Builder.CreateMul(L, R, "multmp");
			else if (L->getType()->isFloatingPointTy()) return Builder.CreateFMul(L, R, "fmultmp");
			break;
		case '/':
			if (L->getType()->isIntegerTy()) return Builder.CreateSDiv(L, R, "divtmp");
			else if (L->getType()->isFloatingPointTy()) return Builder.CreateFDiv(L, R, "fdivtmp");
			break;
		case '%':
			if (L->getType()->isIntegerTy()) return Builder.CreateSRem(L, R, "remtmp");
			else if (L->getType()->isFloatingPointTy()) return Builder.CreateFRem(L, R, "fremtmp");
			break;
		case '<<':
			if (L->getType()->isIntegerTy()) return Builder.CreateShl(L, R, "shltmp");
			else {
				errorGenerator(std::string("左移运算必须为整型"));
				return nullptr;
			}
			break;
		case '>>':
			// 算数右移
			if (L->getType()->isIntegerTy()) return Builder.CreateAShr(L, R, "ashrtmp");
			else {
				errorGenerator(std::string("右移运算必须为整型"));
				return nullptr;
			}
			break;
		case '&':
			if (L->getType()->isIntegerTy()) return Builder.CreateAnd(L, R, "andtmp");
			else {
				errorGenerator(std::string("与运算必须为整型"));
				return nullptr;
			}
			break;
		case '|':
			if (L->getType()->isIntegerTy()) return Builder.CreateOr(L, R, "ortmp");
			else {
				errorGenerator(std::string("或运算必须为整型"));
				return nullptr;
			}
			break;
		case '^':
			if (L->getType()->isIntegerTy()) return Builder.CreateXor(L, R, "xortmp");
			else {
				errorGenerator(std::string("异或运算必须为整型"));
				return nullptr;
			}
			break;
		case '<':
			//整型目前只支持有符号判断
			if (L->getType()->isIntegerTy()) return Builder.CreateICmpSLT(L, R, "slttmp");
			else if (L->getType()->isFloatingPointTy()) L = Builder.CreateFCmpULT(L, R, "ulttmp");
			break; 
		case '<=':
			//整型目前只支持有符号判断
			if (L->getType()->isIntegerTy()) return Builder.CreateICmpSLE(L, R, "sletmp");
			else if (L->getType()->isFloatingPointTy()) L = Builder.CreateFCmpULE(L, R, "uletmp");
			break;
		case '>':
			//整型目前只支持有符号判断
			if (L->getType()->isIntegerTy()) return Builder.CreateICmpSGT(L, R, "sgttmp");
			else if (L->getType()->isFloatingPointTy()) L = Builder.CreateFCmpUGT(L, R, "ugttmp");
			break;
		case '>=':
			//整型目前只支持有符号判断
			if (L->getType()->isIntegerTy()) return Builder.CreateICmpSGE(L, R, "sgetmp");
			else if (L->getType()->isFloatingPointTy()) L = Builder.CreateFCmpUGE(L, R, "ugetmp");
			break;
		case '!=':
			//整型目前只支持有符号判断
			if (L->getType()->isIntegerTy()) return Builder.CreateICmpNE(L, R, "netmp");
			else if (L->getType()->isFloatingPointTy()) L = Builder.CreateFCmpUNE(L, R, "unetmp");
			break;
		case '==':
			//整型目前只支持有符号判断
			if (L->getType()->isIntegerTy()) return Builder.CreateICmpEQ(L, R, "eqtmp");
			else if (L->getType()->isFloatingPointTy()) L = Builder.CreateFCmpUEQ(L, R, "ueqtmp");
			break;
		default:
			break;
		}

		// 用户自定义二元操作符
		Function* F = getFunction(std::string("binary") + Op);
		assert(F && "binary operator not found!");

		Value* Ops[] = { L, R };
		return Builder.CreateCall(F, Ops, "binop");
	}

	Value* CallExprAST::codegen() {
		// Look up the name in the global module table.
		Function* CalleeF = getFunction(Callee);
		if (!CalleeF) {
			errorGenerator(std::string("未知的函数引用"));
			return nullptr;
		}

		// 参数匹配
		if (CalleeF->arg_size() != Args.size()) {
			errorGenerator(std::string("函数参数匹配错误"));
			return nullptr;
		}
			
		std::vector<Value*> ArgsV;
		for (unsigned i = 0, e = Args.size(); i != e; ++i) {
			ArgsV.push_back(Args[i]->codegen());
			if (!ArgsV.back())
				return nullptr;
		}

		return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
	}

	Value* IfExprAST::codegen() {
		Value* CondV = Cond->codegen();
		if (!CondV)
			return nullptr;

		// 转化CondV结果为bool
		if (CondV->getType()->isFloatingPointTy()) {
			CondV = Builder.CreateFCmpONE(
				CondV, ConstantFP::get(TheContext, APFloat(0.0)), "ifcond");
		}
		else if (CondV->getType()->isIntegerTy()) {
			CondV = Builder.CreateICmpNE(
				CondV, ConstantInt::get(TheContext, APInt(1, 0)), "ifcond");
		}
		

		Function* TheFunction = Builder.GetInsertBlock()->getParent();

		// Create blocks for the then and else cases.  Insert the 'then' block at the
		// end of the function.
		BasicBlock* IfBB = BasicBlock::Create(TheContext, "if", TheFunction);
		BasicBlock* ElseBB = BasicBlock::Create(TheContext, "else");
		BasicBlock* MergeBB = BasicBlock::Create(TheContext, "ifcont");

		Builder.CreateCondBr(CondV, IfBB, ElseBB);

		// Emit then value.
		Builder.SetInsertPoint(IfBB);

		std::vector<Value*> IfVs(If.size());
		for (auto& expr : If) {
			Value* IfV = expr->codegen();
			if (!IfV) {
				return nullptr;
			}
			IfVs.push_back(IfV);
		}

		Builder.CreateBr(MergeBB);
		// Codegen of 'Then' can change the current block, update ThenBB for the PHI.
		IfBB = Builder.GetInsertBlock();

		// Emit else block.
		TheFunction->getBasicBlockList().push_back(ElseBB);
		Builder.SetInsertPoint(ElseBB);

		std::vector<Value*> ElseVs(Else.size());
		for (auto& expr : Else) {
			Value* ElseV = expr->codegen();
			if (!ElseV) {
				return nullptr;
			}
			ElseVs.push_back(ElseV);
		}

		Builder.CreateBr(MergeBB);
		// Codegen of 'Else' can change the current block, update ElseBB for the PHI.
		ElseBB = Builder.GetInsertBlock();

		// Emit merge block.
		TheFunction->getBasicBlockList().push_back(MergeBB);
		Builder.SetInsertPoint(MergeBB);
		//此处可能存在问题
		PHINode* PN = Builder.CreatePHI(Type::getDoubleTy(TheContext), 2, "iftmp");

		for (auto& IfV : IfVs) {
			PN->addIncoming(IfV, IfBB);
		}
		for (auto& ElseV : ElseVs) {
			PN->addIncoming(ElseV, ElseBB);
		}
		return PN;
	}

	// rust的for循环并不好实现，建议实现while循环
	Value* ForExprAST::codegen() {
		return nullptr;
	}

	Value* WhileExprAST::codegen() {
		return nullptr;
	}

	Value* VarExprAST::codegen() {
		std::vector<AllocaInst*> OldBindings;

		Function* TheFunction = Builder.GetInsertBlock()->getParent();

		// Register all variables and emit their initializer.
		for (unsigned i = 0, e = VarNames.size(); i != e; ++i) {
			const std::string& VarName = VarNames[i].first;
			ExprAST* Init = VarNames[i].second.get();

			Value* InitVal;
			if (Init) {
				InitVal = Init->codegen();
				if (!InitVal)
					return nullptr;
			}
			else { // If not specified, use 0.0.
				InitVal = ConstantFP::get(TheContext, APFloat(0.0));
			}

			AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, VarName);
			Builder.CreateStore(InitVal, Alloca);

			// Remember the old variable binding so that we can restore the binding when
			// we unrecurse.
			OldBindings.push_back(NamedValues[VarName]);

			// Remember this binding.
			NamedValues[VarName] = Alloca;
		}

		// Codegen the body, now that all vars are in scope.
		Value* BodyVal = Body->codegen();
		if (!BodyVal)
			return nullptr;

		// Pop all our variables from scope.
		for (unsigned i = 0, e = VarNames.size(); i != e; ++i)
			NamedValues[VarNames[i].first] = OldBindings[i];

		// Return the body computation.
		return BodyVal;
	}

	Function* PrototypeAST::codegen() {
		std::vector<llvm::Type *> ArgsType(Args.size());
		Args.push_back(std::make_pair(Type, "result"));
		for (auto& arg : Args) {
			TokenType type = arg.first;
			switch (type)
			{
			case TokenType::tok_integer:
				ArgsType.push_back(Type::getInt32Ty(TheContext));
				break;
			case TokenType::tok_float:
				ArgsType.push_back(Type::getDoubleTy(TheContext));
				break;
			case TokenType::tok_bool:
				ArgsType.push_back(Type::getInt1Ty(TheContext));
				break;
			default:
				break;
			}
		}
		llvm::Type* resultType = ArgsType.back();
		ArgsType.pop_back();
		FunctionType* FT =
			FunctionType::get(resultType, ArgsType, false);

		Function* F =
			Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

		// Set names for all arguments.
		unsigned Idx = 0;
		for (auto& Arg : F->args())
			Arg.setName(Args[Idx++].second);

		return F;
	}

	Function* FunctionAST::codegen() {
		auto& P = *Proto;
		FunctionProtos[Proto->getName()] = std::move(Proto);
		Function* TheFunction = getFunction(P.getName());
		if (!TheFunction) {
			errorGenerator(std::string("函数未定义"));
			return nullptr;
		}

		// Create a new basic block to start insertion into.
		BasicBlock* BB = BasicBlock::Create(TheContext, "entry", TheFunction);
		Builder.SetInsertPoint(BB);

		// Record the function arguments in the NamedValues map.
		NamedValues.clear();
		for (auto& Arg : TheFunction->args()) {
			AllocaInst* Alloca = CreateEntryBlockAlloca(TheFunction, Arg.getName());

			// Store the initial value into the alloca.
			Builder.CreateStore(&Arg, Alloca);

			// Add arguments to variable symbol table.
			NamedValues[std::string(Arg.getName())] = Alloca;
		}
		std::vector<Value*> BodyVals;
		for (auto& expr : Body) {
			Value* fnVal = expr->codegen();
			BodyVals.push_back(fnVal);
		}
		if (Value* RetVal = BodyVals.back()) {
			// Finish off the function.
			Builder.CreateRet(RetVal);

			// Validate the generated code, checking for consistency.
			verifyFunction(*TheFunction);

			// Run the optimizer on the function.
			TheFPM->run(*TheFunction);

			return TheFunction;
		}
		return nullptr;
	}
}
