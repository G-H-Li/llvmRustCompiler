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

        /// numberexpr ::= number
        std::unique_ptr<ExprAST> ParseFPNumberExpr() {
            //获取小数的位置和值
            TokenLocation location = scanner.getToken().getTokenLocation();
            float value = scanner.getToken().getFloatValue();

            auto Result = std::make_unique<FPNumberExprAST>(location, value);
            scanner.getNextToken(); // consume the number
            return std::move(Result);
        }

        /// parenexpr ::= '(' expression ')'
        std::unique_ptr<ExprAST> ParseParenExpr() {
            scanner.getNextToken(); // eat (.
            auto V = ParseExpression();
            if (!V)
                return nullptr;

            if (scanner.getToken().getTokenValue() != TokenValue::RIGHT_PAREN) {
                errorParser("expected ')'");
                return nullptr;
            }
               
            scanner.getNextToken(); // eat ).
            return V;
        }

        /// identifierexpr
        ///   ::= identifier
        ///   ::= identifier '(' expression* ')'
        std::unique_ptr<ExprAST> ParseIdentifierExpr() {
            std::string IdName = scanner.getToken().getStringValue();

            scanner.getNextToken(); // eat identifier.


            // 判断变量类型
            TokenType Type = TokenType::tok_float;//首先默认是浮点数型
            if (scanner.getToken().getTokenValue() == TokenValue::COLON) //判断是否等于冒号 :
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

            //获取地址
            TokenLocation location = scanner.getToken().getTokenLocation();

            if (scanner.getToken().getTokenValue() != TokenValue::LEFT_PAREN) // Simple variable ref.
                return std::make_unique<VariableExprAST>(location,IdName, Type);



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
            
            //获取location
            TokenLocation location = scanner.getToken().getTokenLocation();
            return std::make_unique<CallExprAST>(location,IdName, std::move(Args));
        }


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
        }


        /// ifexpr ::= 'if' expression 'then' expression 'else' expression
        std::unique_ptr<ExprAST> ParseIfExpr() {
            scanner.getNextToken(); // eat the if.

            // condition.
            auto Cond = ParseExpression();
            if (!Cond)
                return nullptr;

            //if函数体内的内容
            std::vector<std::unique_ptr<ExprAST>> If;


            scanner.getNextToken(); //吃掉左大括号 {

            while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
                If.push_back(std::move(ParseExpression()));
                scanner.getNextToken(); //吃掉函数体中的分号 ';'
            }

            scanner.getNextToken(); //吃掉右大括号


            if (scanner.getToken().getTokenValue() != TokenValue::KW_ELSE) {
                errorParser("expected else");
                return nullptr;
            }

            scanner.getNextToken(); //吃掉else

            //else函数体内的内容
            std::vector<std::unique_ptr<ExprAST>> Else;


            scanner.getNextToken(); //吃掉左大括号 {

            while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
                Else.push_back(std::move(ParseExpression()));
                scanner.getNextToken(); //吃掉函数体中的分号 ';'
            }

            scanner.getNextToken(); //吃掉右大括号

            TokenLocation location = scanner.getToken().getTokenLocation();

            return std::make_unique<IfExprAST>(location,std::move(Cond), std::move(If),
                std::move(Else));
        }

        /// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
        std::unique_ptr<ExprAST> ParseForExpr() {
            scanner.getNextToken(); // eat the for.

            if (scanner.getToken().getTokenType() != TokenType::tok_identifier) {
                errorParser("expected identifier after for");
                return nullptr;
            }

            std::string IdName = scanner.getToken().getStringValue(); //解析到 "for i"
            scanner.getNextToken(); // eat identifier.

            if (scanner.getToken().getTokenValue() != TokenValue::KW_IN) {
                errorParser("expected 'in' after for");
                return nullptr;
            }
            scanner.getNextToken(); // eat 'in'.  //解析到 "for i in"

            auto Start = ParseExpression(); //解析到 "for i in 0"
            if (!Start)
                return nullptr;
            if (scanner.getToken().getTokenValue() != TokenValue::PERIOD) {
                errorParser("expected '.' after for start value");
                return nullptr;
            }
            scanner.getNextToken(); //吃掉第一个 '.' 解析到 " for i in 0. "
            scanner.getNextToken(); //吃掉第二个 '.' 解析到 " for i in 0.. "

            auto End = ParseExpression(); //解析到 " for i in 0..100 "
            if (!End)
                return nullptr;

            // The step value is optional.
            //直接让step为空，代码生成时默认为1
            std::unique_ptr<ExprAST> Step = nullptr; //解析到 " for i in 0..100 "


            if (scanner.getToken().getTokenValue() != TokenValue::LEFT_BRACE) {
                errorParser("expected '{' after for");
                return nullptr;
            }


            //for循环体的内容
            std::vector<std::unique_ptr<ExprAST>> Body;


            scanner.getNextToken(); //吃掉左大括号 {

            while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
                Body.push_back(std::move(ParseExpression()));
                scanner.getNextToken(); //吃掉函数体中的分号 ';'
            }

            scanner.getNextToken(); //吃掉右大括号

            TokenLocation location = scanner.getToken().getTokenLocation();
            return std::make_unique<ForExprAST>(location,IdName, std::move(Start), std::move(End),
                std::move(Step), std::move(Body));
        }

        /// primary
        ///   ::= identifierexpr
        ///   ::= numberexpr
        ///   ::= parenexpr
        ///   ::= ifexpr
        ///   ::= forexpr
        std::unique_ptr<ExprAST> ParsePrimary() {
            //大层swich判断类型，处理 identity和number类型
            switch (scanner.getToken().getTokenType())
            {

            case TokenType::tok_identifier:
                return ParseIdentifierExpr();

            case TokenType::tok_float:
            case TokenType::tok_integer:
                return ParseFPNumberExpr();
            default:
                //小层for循环处理 let ( if for
                switch (scanner.getToken().getTokenValue())
                {
                case TokenValue::KW_LET:
                    return ParseLet();
                case TokenValue::LEFT_PAREN:
                    return ParseParenExpr();

                case TokenValue::KW_IF:
                    return ParseIfExpr();
                case TokenValue::KW_FOR:
                    return ParseForExpr();
                default:
                    errorParser("unknown token when expecting an expression");
                    return nullptr;
                    break;
                }
                break;
            }
        }

        /// binoprhs
        ///   ::= ('+' primary)*
        std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
            std::unique_ptr<ExprAST> LHS) {
            // If this is a binop, find its precedence.
            while (true) {
                int TokPrec = scanner.getToken().getSymbolPrecedence();
                // If this is a binop that binds at least as tightly as the current binop,
                // consume it, otherwise we are done.
                if (TokPrec < ExprPrec)
                    return LHS;

                // Okay, we know this is a binop.
                int BinOp = (int)scanner.getToken().getTokenValue(); //需要操作符的ASCII码值
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
                //获取地址
                TokenLocation location = scanner.getToken().getTokenLocation();
                LHS =
                    std::make_unique<BinaryExprAST>(location,BinOp, std::move(LHS), std::move(RHS));
            }
        }

        /// expression
        ///   ::= primary binoprhs
        ///
        std::unique_ptr<ExprAST> ParseExpression() {
            auto LHS = ParsePrimary();
            if (!LHS)
                return nullptr;

            return ParseBinOpRHS(0, std::move(LHS));
        }

        /// prototype
        ///   ::= id '(' id* ')'
        std::unique_ptr<PrototypeAST> ParsePrototype() {
            if (scanner.getToken().getTokenType() != TokenType::tok_identifier) {
                errorParser("Expected function name in prototype");
                return nullptr;
            }

            std::string FnName = scanner.getToken().getStringValue();
            scanner.getNextToken();

            if (scanner.getToken().getTokenValue() != TokenValue::LEFT_PAREN) {
                errorParser("Expected '(' in prototype");
                return nullptr;
            }

            scanner.getNextToken(); //吃掉(


            //std::vector<std::unique_ptr<ExprAST>> ArgNames;
            
            //参数容器
            std::vector<std::pair<TokenType, std::string>> ArgNames;

            //添加参数

            while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_PAREN) {

                //获取一个参数
                std::unique_ptr<ExprAST> E = ParseIdentifierExpr();
                
                ArgNames.push_back(std::move(ParseIdentifierExpr()));
                if (scanner.getToken().getTokenValue() == TokenValue::RIGHT_PAREN)
                    break;
                scanner.getNextToken(); //吃掉 ','
            }

            scanner.getNextToken(); // eat ')'.


            /*while (scanner.getNextToken() == TokenType::tok_identifier)
                ArgNames.push_back(std::move(ParseIdentifierExpr()));
            if (scanner.getToken().getTokenValue() != TokenValue::RIGHT_PAREN)
                return LogErrorP("Expected ')' in prototype");*/

                //判断返回类型
            int return_type = 0;
            if (scanner.getToken().getTokenValue() == TokenValue::POINTER)
            {
                scanner.getNextToken(); //吃掉lamda
                if (scanner.getToken().getTokenType() == TokenType::tok_integer)
                {
                    return_type =(int) TokenType::tok_integer;
                    scanner.getNextToken(); //吃掉类型
                }
                if (scanner.getToken().getTokenType() == TokenType::tok_float)
                {
                    return_type = (int)TokenType::tok_float;
                    scanner.getNextToken(); //吃掉类型
                }
            }


            return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames), return_type);
        }

        /// definition ::= 'def' prototype expression
        std::unique_ptr<FunctionAST> ParseDefinition() {
            scanner.getNextToken(); // eat def.
            auto Proto = ParsePrototype();
            if (!Proto)
                return nullptr;

            //解析大括号
            if (scanner.getToken().getTokenValue() != TokenValue::LEFT_BRACE)
                return nullptr;
            std::vector<std::unique_ptr<ExprAST>> Body;

            scanner.getNextToken(); //吃掉左大括号
            while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
                Body.push_back(std::move(ParseExpression()));
                scanner.getNextToken(); //吃掉函数体中的分号 ';'
            }

            scanner.getNextToken(); //吃掉右大括号
            /*if (auto E = ParseExpression())
                return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));*/

            return std::make_unique<FunctionAST>(std::move(Proto), std::move(Body));
        }

        /// toplevelexpr ::= expression
        std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
            if (auto E = ParseExpression()) {
                // Make an anonymous proto.
                auto Proto = std::make_unique<PrototypeAST>("__anon_expr",
                    std::vector<std::unique_ptr<ExprAST>>(), 0);

                //修改了函数体为数组，因此要添加一个数组创建FunctionAST

                std::vector<std::unique_ptr<ExprAST>> Body;
                Body.push_back(std::move(E));

                return std::make_unique<FunctionAST>(std::move(Proto), std::move(Body));
            }
            return nullptr;
        }



	private:
		void HandleDefinition() {
			if (ParseDefinition()) {
				fprintf(stderr, "Read function definition: \n");
			}
			else {
				// Skip token for error recovery.
				scanner.getNextToken();
			}
		}


		void HandleTopLevelExpression() {
			// Evaluate a top-level expression into an anonymous function.
			if (ParseTopLevelExpr()) {
                fprintf(stderr, "Top Level Expression to Paser \n");
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