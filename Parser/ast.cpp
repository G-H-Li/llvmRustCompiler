#include "ast.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace llvmRustCompiler
{
    /// 基类
    class ExprAST {
    public:
        virtual ~ExprAST() = default;
    };

    /// 数字
    class NumberExprAST : public ExprAST {
        double Val;

    public:
        NumberExprAST(double Val) : Val(Val) {}
    };

    /// 变量。rust变量要声明类型，因此添加一个type属性
    /// 因为有let mut存在，就添加一个isConst属性
    class VariableExprAST : public ExprAST {
        std::string Name;
        int Type;
        bool IsConst;
    public:
        VariableExprAST(const std::string& Name, int Type, bool IsConst) : Name(Name), Type(Type), IsConst(IsConst) {}
    };

    /// BinaryExprAST - Expression class for a binary operator.
    class BinaryExprAST : public ExprAST {
        char Op;
        std::unique_ptr<ExprAST> LHS, RHS;

    public:
        BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
            std::unique_ptr<ExprAST> RHS)
            : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
    };

    /// CallExprAST - Expression class for function calls.
    class CallExprAST : public ExprAST {
        std::string Callee;
        std::vector<std::unique_ptr<ExprAST>> Args;

    public:
        CallExprAST(const std::string& Callee,
            std::vector<std::unique_ptr<ExprAST>> Args)
            : Callee(Callee), Args(std::move(Args)) {}
    };


    /// 函数原型带有返回类型，因此要加一个type
    class PrototypeAST {
        std::string Name;
        std::vector<std::string> Args;
        int Type;
    public:
        PrototypeAST(const std::string& Name,
            std::vector<std::string> Args,
            int Type)
            : Name(Name), Args(std::move(Args)), Type(Type) {}

        /*PrototypeAST(const std::string& Name,
            std::vector<std::string> Args)
            : Name(Name), Args(std::move(Args)) {}*/

        const std::string& getName() const { return Name; }
    };

    /// FunctionAST - This class represents a function definition itself.
    class FunctionAST {
        std::unique_ptr<PrototypeAST> Proto;
        std::unique_ptr<ExprAST> Body;

    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Proto,
            std::unique_ptr<ExprAST> Body)
            : Proto(std::move(Proto)), Body(std::move(Body)) {}
    };


    //if语句的AST定义
    class IfExprAST : public ExprAST {
        std::unique_ptr<ExprAST> Cond, Then, Else;
    public:
        IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then,
            std::unique_ptr<ExprAST> Else)
            : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

        //Value* codegen() override; //无codegen()先注释掉
    };
}