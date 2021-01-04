#pragma once
/*
* Author: 李国豪
* Date:2021/1/3
* description:抽象语法树定义
* latest date:2021/1/4
*/


#include "../Lexer/token.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/raw_ostream.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>

using namespace llvm;

namespace llvmRustCompiler
{
    raw_ostream& indent(raw_ostream& O, int size) {
        return O << std::string(size, ' ');
    }

    // 基类
    class ExprAST {
        TokenLocation Loc;
    public:
        ExprAST() {}
        ExprAST(TokenLocation Loc) : Loc(Loc) {}
        virtual ~ExprAST() = default;
        // 中间代码生成函数
        virtual Value* codegen() = 0;
        virtual raw_ostream& dump(raw_ostream& out, int ind) {
            return out << ':' << Loc.getLine() << ':' << Loc.getCol() << '\n';
        }
    };

    /// 数字
    class NumberExprAST : public ExprAST {
        double Val;

    public:
        NumberExprAST(TokenLocation Loc, double Val): ExprAST(Loc), Val(Val) {}
        raw_ostream& dump(raw_ostream& out, int ind) override {
            return ExprAST::dump(out << Val, ind);
        }
        Value* codegen() override;
    };

    /// 变量 ,let mut存在，就添加一个isMutable属性,表示是否可变
    class VariableExprAST : public ExprAST {
        std::string Name;
        int Type;
        bool IsMutable;
    public:
        VariableExprAST(TokenLocation Loc, const std::string& Name, int Type, bool IsMutable)
            : ExprAST(Loc), Name(Name), Type(Type), IsMutable(IsMutable) {}
        const std::string& getName() const { return Name; }
        raw_ostream& dump(raw_ostream& out, int ind) override {
            return ExprAST::dump(out << Name, ind);
        }
        Value* codegen() override;
    };

    /// 一元操作符
    class UnaryExprAST : public ExprAST {
        char Opcode;
        std::unique_ptr<ExprAST> Operand;

    public:
        UnaryExprAST(TokenLocation Loc, char Opcode, std::unique_ptr<ExprAST> Operand)
            : ExprAST(Loc), Opcode(Opcode), Operand(std::move(Operand)) {}
        raw_ostream& dump(raw_ostream& out, int ind) override {
            ExprAST::dump(out << "unary" << Opcode, ind);
            Operand->dump(out, ind + 1);
            return out;
        }
        Value* codegen() override;
    };

    /// 二元操作符
    class BinaryExprAST : public ExprAST {
        char Op;
        std::unique_ptr<ExprAST> LHS, RHS;

    public:
        BinaryExprAST(TokenLocation Loc, char Op, std::unique_ptr<ExprAST> LHS,
            std::unique_ptr<ExprAST> RHS)
            : ExprAST(Loc), Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
        raw_ostream& dump(raw_ostream& out, int ind) override {
            ExprAST::dump(out << "binary" << Op, ind);
            LHS->dump(indent(out, ind) << "LHS:", ind + 1);
            RHS->dump(indent(out, ind) << "RHS:", ind + 1);
            return out;
        }
        Value* codegen() override;
    };

    /// 函数调用
    class CallExprAST : public ExprAST {
        std::string Callee;
        std::vector<std::unique_ptr<ExprAST>> Args;

    public:
        CallExprAST(TokenLocation Loc, const std::string& Callee,
            std::vector<std::unique_ptr<ExprAST>> Args)
            : ExprAST(Loc), Callee(Callee), Args(std::move(Args)) {}
        
        raw_ostream& dump(raw_ostream& out, int ind) override {
            ExprAST::dump(out << "call " << Callee, ind);
            for (const auto& Arg : Args)
                Arg->dump(indent(out, ind + 1), ind + 1);
            return out;
        }
        Value* codegen() override;
    };

    //TODO

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