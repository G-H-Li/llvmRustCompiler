#pragma once

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
    /// ����
    class ExprAST {
    public:
        virtual ~ExprAST() = default;
    };

    /// ����
    class NumberExprAST : public ExprAST {
        double Val;

    public:
        NumberExprAST(double Val){}
    };

    /// ������rust����Ҫ�������ͣ�������һ��type����
    /// ��Ϊ��let mut���ڣ������һ��isConst����
    class VariableExprAST : public ExprAST {
        std::string Name;
        int Type;
        bool IsConst;
    public:
        VariableExprAST(const std::string& Name, int Type, bool IsConst){}
    };

    /// BinaryExprAST - Expression class for a binary operator.
    class BinaryExprAST : public ExprAST {
        char Op;
        std::unique_ptr<ExprAST> LHS, RHS;

    public:
        BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
            std::unique_ptr<ExprAST> RHS){}
    };

    /// CallExprAST - Expression class for function calls.
    class CallExprAST : public ExprAST {
        std::string Callee;
        std::vector<std::unique_ptr<ExprAST>> Args;

    public:
        CallExprAST(const std::string& Callee,
            std::vector<std::unique_ptr<ExprAST>> Args){}
    };


    /// ����ԭ�ʹ��з������ͣ����Ҫ��һ��type
    class PrototypeAST {
        std::string Name;
        std::vector<std::string> Args;
        int Type;
    public:
        PrototypeAST(const std::string& Name,
            std::vector<std::string> Args,
            int Type){}

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
            std::unique_ptr<ExprAST> Body) {}
    };


    //if����AST����
    class IfExprAST : public ExprAST {
        std::unique_ptr<ExprAST> Cond, Then, Else;
    public:
        IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then,
            std::unique_ptr<ExprAST> Else) {}

        //Value* codegen() override; //��codegen()��ע�͵�
    };
}