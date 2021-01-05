#pragma once
#include "ast.h"
#include "..\Lexer\scanner.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace llvmRustCompiler {
	class Parser
	{
	public:
		Parser(Scanner& scanner);
		~Parser();

	private:
		Scanner _scanner;


	private:
		std::unique_ptr<ExprAST> ParseExpression();
		std::unique_ptr<ExprAST> ParseNumberExpr();
		std::unique_ptr<ExprAST> ParseParenExpr();
		std::unique_ptr<ExprAST> ParseIdentifierExpr();
		std::unique_ptr<ExprAST> ParseLet();
		std::unique_ptr<ExprAST> ParseIfExpr();
		std::unique_ptr<ExprAST> ParseForExpr();
		std::unique_ptr<ExprAST> ParsePrimary();
		std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
			std::unique_ptr<ExprAST> LHS);
		std::unique_ptr<VariableExprAST> ParseArgument();
		std::unique_ptr<PrototypeAST> ParsePrototype();
		std::unique_ptr<FunctionAST> ParseDefinition();
		std::unique_ptr<FunctionAST> ParseTopLevelExpr();


	private:
        void HandleDefinition();
        void HandleTopLevelExpression();
        /// top ::= definition | external | expression | ';'
        void MainLoop();

    
    public:
        //===----------------------------------------------------------------------===//
        // Main driver code.
        //===----------------------------------------------------------------------===//
		int main();
	};
}