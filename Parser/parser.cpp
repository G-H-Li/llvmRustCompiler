#include "ast.h"
#include "..\Lexer\scanner.h"
#include "..\Error\error.h"

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
		Parser(Scanner& scanner) : scanner(scanner) {};
		~Parser();

	private:
		Scanner& scanner;


	private:
		std::unique_ptr<ExprAST> ParseExpression() {
			
		};

		/// numberexpr ::= number
		std::unique_ptr<ExprAST> ParseNumberExpr() {
			auto Result = std::make_unique<NumberExprAST>(scanner.getToken().getFloatValue());
			scanner.getNextToken(); // consume the number
			return std::move(Result);
		};


		/// parenexpr ::= '(' expression ')'
		std::unique_ptr<ExprAST> ParseParenExpr() {
			scanner.getNextToken(); // eat (.
			auto V = ParseExpression();
			if (!V)
				return nullptr;

			if (scanner.getToken().getTokenValue() != TokenValue::LEFT_PAREN) {
				errorParser("expected ')'");
				return nullptr;
			}
				
			scanner.getNextToken(); // eat ).
			return V;
		};



		/// identifierexpr
		///   ::= identifier
		///   ::= identifier '(' expression* ')'
		std::unique_ptr<ExprAST> ParseIdentifierExpr() {
			std::string IdName = scanner.getToken().getStringValue();

			scanner.getNextToken(); // eat identifier.

			// 判断变量类型
			int Type = 0; //默认无类型
			if (scanner.getToken().getTokenValue() == TokenValue::COLON)
			{
				scanner.getNextToken();
				if (scanner.getToken().getTokenType() == TokenType::tok_integer) {
					Type = TokenType::tok_integer;
					scanner.getNextToken();
				}

				if (scanner.getToken().getTokenType() == TokenType::tok_float) {
					Type = TokenType::tok_float;
					scanner.getNextToken();
				}
			}


			if (scanner.getToken().getTokenValue() != TokenValue::LEFT_PAREN) // Simple variable ref.
				return std::make_unique<VariableExprAST>(IdName, Type, true);

			// Call.
			scanner.getNextToken(); // eat (
			std::vector<std::unique_ptr<ExprAST>> Args;
			if (scanner.getToken().getTokenValue() != TokenValue::RIGHT_PAREN) {
				while (true) {
					if (auto Arg = ParseExpression())
						Args.push_back(std::move(Arg));
					else
						return nullptr;

					if (scanner.getToken().getTokenValue() == TokenValue::RIGHT_PAREN)
						break;

					if (scanner.getToken().getTokenValue() != TokenValue::COMMA) {
						errorParser("Expected ')' or ',' in argument list");
						return nullptr;
					}
					scanner.getNextToken();
				}
			}

			// Eat the ')'.
			scanner.getNextToken();

			return std::make_unique<CallExprAST>(IdName, std::move(Args));
		};



		//解析let mut x = 3;
		std::unique_ptr<ExprAST> ParseLet() {
			scanner.getNextToken(); //吃掉let

			//判断有无 mut 确定是否是常量
			bool isConst = false;
			if (scanner.getToken().getTokenValue() == TokenValue::KW_MUT)
			{
				isConst = true;
				scanner.getNextToken();
			}

			if (scanner.getToken().getTokenType() != TokenType::tok_identifier)
				return nullptr;
			if (auto E = ParseIdentifierExpr())
				return E;
			return nullptr;
		};



		/// primary
		///   ::= identifierexpr
		///   ::= numberexpr
		///   ::= parenexpr
		std::unique_ptr<ExprAST> ParsePrimary() {
			switch (scanner.getToken().getTokenType()) {
			default:
				errorParser("unknown token when expecting an expression");
				return nullptr;
			case TokenType::tok_identifier:
				return ParseIdentifierExpr();
			case TokenType::tok_integer:
			case TokenType::tok_float:
				return ParseNumberExpr();
			case TokenType::tok_keywords:
				if (scanner.getToken().getTokenValue() == TokenValue::KW_LET) //let
					return ParseLet();
				else if (scanner.getToken().getTokenValue() == TokenValue::LEFT_PAREN) // '('
					return ParseParenExpr();
				else if (scanner.getToken().getTokenValue() == TokenValue::SEMICOLON) {  // '}'
					scanner.getNextToken();  // ignore top-level semicolons.
					break;
				}
			};
		};


		/// binoprhs
		///   ::= ('+' primary)*
		std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,std::unique_ptr<ExprAST> LHS) {
			// If this is a binop, find its precedence.
			while (true) {
				int TokPrec = scanner.getToken().getSymbolPrecedence();

				// If this is a binop that binds at least as tightly as the current binop,
				// consume it, otherwise we are done.
				if (TokPrec < ExprPrec)
					return LHS;

				// Okay, we know this is a binop.
				int BinOp = (int)scanner.getToken().getTokenValue();
				scanner.getNextToken(); // eat binop

				// Parse the primary expression after the binary operator.
				auto RHS = ParsePrimary();
				if (!RHS)
					return nullptr;

				// If BinOp binds less tightly with RHS than the operator after RHS, let
				// the pending operator take RHS as its LHS.
				int NextPrec = scanner.getToken().getSymbolPrecedence();
				if (TokPrec < NextPrec) {
					RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
					if (!RHS)
						return nullptr;
				}
				// Merge LHS/RHS.
				LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
			}
		};



		/// expression
		///   ::= primary binoprhs
		///
		std::unique_ptr<ExprAST> ParseExpression() {
			auto LHS = ParsePrimary();
			if (!LHS)
				return nullptr;

			return ParseBinOpRHS(0, std::move(LHS));
		}


		//此函数只用于解析 fn main(x:i32, y:f64) 中的x:i32,y:f32参数。不做其他用途
		std::unique_ptr<VariableExprAST> ParseArgument() {
			std::string VarialbeName = scanner.getToken().getStringValue();
			scanner.getNextToken(); //吃掉变量名

			//判断类型
			int Type = 0;
			if (scanner.getToken().getTokenValue() == TokenValue::COLON) {
				scanner.getNextToken(); //吃掉:


				if (scanner.getToken().getTokenType() == TokenType::tok_integer) {
					Type = TokenType::tok_integer;
					scanner.getNextToken(); //吃掉类型
				}

				if (scanner.getToken().getTokenType() == TokenType::tok_float) {
					Type = TokenType::tok_float;
					scanner.getNextToken(); //吃掉类型
				}
			}

			return std::make_unique<VariableExprAST>(VarialbeName, Type, true);
		};



		/// prototype
		///   ::= id '(' id* ')'
		std::unique_ptr<PrototypeAST> ParsePrototype() {
			if (scanner.getToken().getTokenType() != TokenType::tok_identifier) {
				
			}
				

			std::string FnName = scanner.getToken().getStringValue();
			scanner.getNextToken();

			if (scanner.getToken().getTokenValue() != TokenValue::LEFT_PAREN) {
				errorParser("Expected '(' in prototype");
				return nullptr;
			}
		

			scanner.getNextToken(); //吃掉(

			std::vector<std::string> ArgNames;

			while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_PAREN) {
				ArgNames.push_back(scanner.getToken().getStringValue());
				scanner.getNextToken(); //吃掉 ','
			}

			/* while (scanner.getNextToken() == tok_identifier)
				 ArgNames.push_back(scanner.getToken().getStringValue());
			 if (scanner.getToken().getTokenValue() != ')')
				 return LogErrorP("expected ')' in prototype");*/

			scanner.getNextToken(); // eat ')'.

			//判断返回类型
			int return_type = 0;
			if (scanner.getToken().getTokenValue() == TokenValue::POINTER)
			{
				scanner.getNextToken(); //吃掉lamda


				if (scanner.getToken().getTokenType() == TokenType::tok_integer) {
					return_type = TokenType::tok_integer;
					scanner.getNextToken(); //吃掉类型
				}

				if (scanner.getToken().getTokenType() == TokenType::tok_float) {
					return_type = TokenType::tok_float;
					scanner.getNextToken(); //吃掉类型
				}
			}

			return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames), return_type);

			//return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
		};



		/// definition ::= 'def' prototype expression
		std::unique_ptr<FunctionAST> ParseDefinition() {
			scanner.getNextToken(); // 吃掉 fn
			auto Proto = ParsePrototype();
			if (!Proto)
				return nullptr;

			if (scanner.getToken().getTokenValue() != TokenValue::LEFT_BRACE)
				return nullptr;
			scanner.getNextToken(); //吃掉 '{'

			auto E = ParseExpression();
			if (!E)
				return nullptr;

			//把分号吃掉
			if (scanner.getToken().getTokenValue() == TokenValue::SEMICOLON)
				scanner.getNextToken();

			scanner.getNextToken(); //吃掉'}'
			return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
		};




		/// toplevelexpr ::= expression
		std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
			if (auto E = ParseExpression()) {
				// Make an anonymous proto.
				auto Proto = std::make_unique<PrototypeAST>("__anon_expr", std::vector<std::string>(), (int)0);
				return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
			}
			return nullptr;
		};




		std::unique_ptr<ExprAST> ParseIfExpr() {
			scanner.getNextToken();  // eat the if.
			// condition.
			auto Cond = ParseExpression();
			if (!Cond)
				return nullptr;

			// 解析 '{'
			if (scanner.getToken().getTokenValue() != TokenValue::LEFT_BRACE) {
				errorParser("expected '{'");
				return nullptr;
			}
				
			scanner.getNextToken();  // eat {


			auto Body = ParseExpression();
			if (!Body)
				return nullptr;

			//把分号吃掉
			if (scanner.getToken().getTokenValue() == TokenValue::SEMICOLON)
				scanner.getNextToken();

			if (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
				errorParser("expected '}'");
				return nullptr;
			}

			scanner.getNextToken(); //吃掉 '}'


			if (scanner.getToken().getTokenValue() != TokenValue::KW_ELSE) {
				errorParser("expected else");
				return nullptr;
			}
				
			scanner.getNextToken(); //吃掉 else

			// 解析 '{'
			if (scanner.getToken().getTokenValue() != TokenValue::LEFT_BRACE) {
				errorParser("expected '{'");
				return nullptr;
			}
				
			scanner.getNextToken();  // eat {

			auto Else = ParseExpression();
			if (!Else)
				return nullptr;

			//把分号吃掉
			if (scanner.getToken().getTokenValue() == TokenValue::SEMICOLON)
				scanner.getNextToken();

			if (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
				errorParser("expected '}'");
				return nullptr;
			}
				
			scanner.getNextToken(); //吃掉 '}'


			return std::make_unique<IfExprAST>(std::move(Cond), std::move(Body),
				std::move(Else));
		};



		private:
			void HandleDefinition() {
				if (ParseDefinition()) {
					fprintf(stderr, "Parsed a function definition.\n");
				}
				else {
					// Skip token for error recovery.
					scanner.getNextToken();
				}
			}


			void HandleTopLevelExpression() {
				// Evaluate a top-level expression into an anonymous function.
				if (ParseTopLevelExpr()) {
					fprintf(stderr, "Parsed a top-level expr\n");
				}
				else {
					// Skip token for error recovery.
					scanner.getNextToken();
				}
			}

			/// top ::= definition | external | expression | ';'
			void MainLoop() {
				while (true) {
					fprintf(stderr, "ready> ");
					switch (scanner.getToken().getTokenType()) {

					case TokenType::tok_eof:
						return;
					case TokenType::tok_keywords: {
						if (scanner.getToken().getTokenValue() == TokenValue::SEMICOLON) {
							scanner.getNextToken();
							break;
						}
						else if (scanner.getToken().getTokenValue() == TokenValue::KW_FN) {
							HandleDefinition();
							break;
						}
						else if (scanner.getToken().getTokenValue() == TokenValue::KW_IF) {
							ParseIfExpr();
							break;
						}

					}
					default:
						HandleTopLevelExpression();
						break;
					}
				}
			}

		public:
			int main() {

				// Prime the first token.
				fprintf(stderr, "ready> ");
				scanner.getNextToken();

				// Run the main "interpreter loop" now.
				MainLoop();

				return 0;
			}

	};
}