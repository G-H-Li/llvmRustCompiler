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
#include <map>

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

    /// 变量 isMutable 表示是否可变变量
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

    /// 判断表达式（if，else if ， else）
    class IfExprAST : public ExprAST {
        std::unique_ptr<ExprAST> Cond, ElseIf, Else;

    public:
        IfExprAST(TokenLocation Loc, std::unique_ptr<ExprAST> Cond,
            std::unique_ptr<ExprAST> ElseIf, std::unique_ptr<ExprAST> Else)
            : ExprAST(Loc), Cond(std::move(Cond)), ElseIf(std::move(ElseIf)),
            Else(std::move(Else)) {}
        raw_ostream& dump(raw_ostream& out, int ind) override {
            ExprAST::dump(out << "if", ind);
            Cond->dump(indent(out, ind) << "Cond:", ind + 1);
            ElseIf->dump(indent(out, ind) << "ElseIf:", ind + 1);
            Else->dump(indent(out, ind) << "Else:", ind + 1);
            return out;
        }

        Value *codegen() override;
    };

    /// For循环 - Expression class for for/in.
    class ForExprAST : public ExprAST {
        std::string VarName;
        std::unique_ptr<ExprAST> Start, End, Step, Body;

    public:
        ForExprAST(TokenLocation Loc, const std::string& VarName, std::unique_ptr<ExprAST> Start,
            std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step,
            std::unique_ptr<ExprAST> Body)
            : ExprAST(Loc), VarName(VarName), Start(std::move(Start)), End(std::move(End)),
            Step(std::move(Step)), Body(std::move(Body)) {}
        
        raw_ostream& dump(raw_ostream& out, int ind) override {
            ExprAST::dump(out << "for", ind);
            Start->dump(indent(out, ind) << "Cond:", ind + 1);
            End->dump(indent(out, ind) << "End:", ind + 1);
            Step->dump(indent(out, ind) << "Step:", ind + 1);
            Body->dump(indent(out, ind) << "Body:", ind + 1);
            return out;
        }
        Value* codegen() override;
    };

    // TODO while循环
    class WhileExprAST : public ExprAST {
    };
    // TODO loop循环
    class LoopExprAST : public ExprAST {

    };

    // 局部变量
    class VarExprAST : public ExprAST {
        std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;
        std::unique_ptr<ExprAST> Body;

    public:
        VarExprAST(TokenLocation Loc, std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames,
            std::unique_ptr<ExprAST> Body)
            : ExprAST(Loc), VarNames(std::move(VarNames)), Body(std::move(Body)) {}
        
        raw_ostream& dump(raw_ostream& out, int ind) override {
            ExprAST::dump(out << "var", ind);
            for (const auto& NamedVar : VarNames)
                NamedVar.second->dump(indent(out, ind) << NamedVar.first << ':', ind + 1);
            Body->dump(indent(out, ind) << "Body:", ind + 1);
            return out;
        }
        Value* codegen() override;
    };

    /// 函数定义原型带有返回类型和参数名及类型
    class PrototypeAST {
        std::string Name;
        std::map<int, std::string> Args;
        int Type;
        int Line;
    public:
        PrototypeAST(TokenLocation Loc, const std::string& Name,
            std::map<int, std::string> Args, int Type)
            : Name(Name), Args(std::move(Args)), Type(Type), Line(Loc.getLine()){}

        const std::string& getName() const { return Name; }
        int getLine() const { return Line; }

        Function* codegen();
    };

    /// 函数AST
    class FunctionAST {
        std::unique_ptr<PrototypeAST> Proto;
        std::unique_ptr<ExprAST> Body;

    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Proto,
            std::unique_ptr<ExprAST> Body)
            : Proto(std::move(Proto)), Body(std::move(Body)) {}
        
        raw_ostream& dump(raw_ostream& out, int ind) {
            indent(out, ind) << "FunctionAST\n";
            ++ind;
            indent(out, ind) << "Body:";
            return Body ? Body->dump(out, ind) : out << "null\n";
        }
        Function* codegen();
    };
}