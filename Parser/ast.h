/*
* Author: 李国豪
* Date:2021/1/3
* description:抽象语法树定义
* latest date:2021/1/4
*/
#pragma once

#include "../Lexer/token.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Value.h"
#include "../Lexer/token.h"
#include "../Error/error.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <map>
#include <memory>
#include <vector>
using namespace llvm;

namespace llvmRustCompiler
{
    namespace {
        raw_ostream& indent(raw_ostream& O, int size) {
            return O << std::string(size, ' ');
        }
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

        //获取必要的值
    };

    /// 浮点数字,
    class FPNumberExprAST : public ExprAST {
        double Val;

    public:
        FPNumberExprAST(TokenLocation Loc, double Val): ExprAST(Loc), Val(Val){}
        raw_ostream& dump(raw_ostream& out, int ind) override {
            return ExprAST::dump(out << Val, ind);
        }
        Value* codegen() override;
    };

    // 整型数值
    class IntNumberExprAST : public ExprAST {
        long long Val;
        int Bits;
        bool IsSigned;

    public:
        IntNumberExprAST(TokenLocation Loc, long long Val, int Bits, bool IsSigned) 
            : ExprAST(Loc), Val(Val), Bits(Bits), IsSigned(IsSigned) {}
        raw_ostream& dump(raw_ostream& out, int ind) override {
            return ExprAST::dump(out << Val<< Bits<< IsSigned, ind);
        }
        Value* codegen() override;
    };

    /// 变量
    class VariableExprAST : public ExprAST {
        std::string Name;
        TokenType Type;
    public:
        VariableExprAST(TokenLocation Loc, const std::string& Name, TokenType Type)
            : ExprAST(Loc), Name(Name), Type(Type) {}
        const std::string& getName() const { return Name; }

        std::string& getName() { return Name; }
        TokenType getType() { return Type; }
        
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
    /// if体和else体得包含多条语句，改成std::vector<std::unique_ptr<ExprAST>>
    class IfExprAST : public ExprAST {
        std::unique_ptr<ExprAST> Cond;
        std::vector<std::unique_ptr<ExprAST>> If;
        std::vector<std::unique_ptr<ExprAST>> Else;
    public:
        IfExprAST(TokenLocation Loc, 
            std::unique_ptr<ExprAST> Cond,
            std::vector<std::unique_ptr<ExprAST>> If, 
            std::vector<std::unique_ptr<ExprAST>> Else)
            : ExprAST(Loc), Cond(std::move(Cond)),If(std::move(If)),
            Else(std::move(Else)) {}
        raw_ostream& dump(raw_ostream& out, int ind) override {
            ExprAST::dump(out << "if", ind);
            Cond->dump(indent(out, ind) << "Cond:", ind + 1);
            //因为变成了多条语句，因此加for循环遍历
            for (int i = 0; i < If.size(); i++) {
                If.at(i)->dump(indent(out, ind) << "If:", ind + 1);
            }

            for (int i = 0; i < Else.size(); i++) {
                Else.at(i)->dump(indent(out, ind) << "Else:", ind + 1);
            }
            return out;
        }

        Value *codegen() override;
    };

    /// For循环 - Expression class for for/in.
    /// For函数体包含多条语句。改成std::vector<std::unique_ptr<ExprAST>> 
    /// 只支持 for i in 0..100 { body }
    /// Varname : i  
    /// start: 0; 
    ///  .. 词法分析不要把点当成小数点返回数字
    /// end: 100  万花筒5里面end是i<n的不等于形式而这里是一个整数形式，代码生成注意差别
    /// Step默认为空，代码生成时默认设置为1
    class ForExprAST : public ExprAST {
        std::string VarName;
        std::unique_ptr<ExprAST> Start, End, Step;
        std::vector<std::unique_ptr<ExprAST>> Body;
    public:
        ForExprAST(TokenLocation Loc,
            const std::string& VarName,
            std::unique_ptr<ExprAST> Start,
            std::unique_ptr<ExprAST> End,
            std::unique_ptr<ExprAST> Step,
            std::vector<std::unique_ptr<ExprAST>> Body)
            : ExprAST(Loc), VarName(VarName),
            Start(std::move(Start)), End(std::move(End)),
            Step(std::move(Step)), Body(std::move(Body)) {}
        
        raw_ostream& dump(raw_ostream& out, int ind) override {
            ExprAST::dump(out << "for", ind);
            Start->dump(indent(out, ind) << "Cond:", ind + 1);
            End->dump(indent(out, ind) << "End:", ind + 1);
            Step->dump(indent(out, ind) << "Step:", ind + 1);
            //因为变成了多条语句，因此加for循环遍历
            for (int i = 0; i < Body.size(); i++) {
                Body.at(i)->dump(indent(out, ind) << "Then:", ind + 1);
            }
            
            return out;
        }
        Value* codegen() override;
    };

    // while循环
    // 修改Body为多条语句。定义改为 
    class WhileExprAST : public ExprAST {
        std::unique_ptr<ExprAST> End;
        std::vector<std::unique_ptr<ExprAST>> Body;

    public:
        WhileExprAST(TokenLocation Loc, std::unique_ptr<ExprAST> End,
            std::vector<std::unique_ptr<ExprAST>> Body)
            : ExprAST(Loc), End(std::move(End)), Body(std::move(Body)) {}

        raw_ostream& dump(raw_ostream& out, int ind) override {
            ExprAST::dump(out << "while", ind);
            End->dump(indent(out, ind) << "End:", ind + 1);
            
            //因为变成了多条语句，因此加for循环遍历
            for (int i = 0; i < Body.size(); i++) {
                Body.at(i)->dump(indent(out, ind) << "Then:", ind + 1);
            }
            return out;
        }
        Value* codegen() override;
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
        std::vector<std::pair<TokenType, std::string>> Args;
        TokenType Type;
        int Line;
    public:

        PrototypeAST(TokenLocation Loc, 
            const std::string& Name,
            std::vector<std::pair<TokenType,
            std::string>> Args,
            TokenType Type)
            : Name(Name), Args(std::move(Args)), Type(Type), Line(Loc.getLine()){}

        const std::string& getName() const { return Name; }
        int getLine() const { return Line; }

        Function* codegen();
    };

    /// 函数AST
    /// 函数体包含多条语句，改成std::vector<std::unique_ptr<ExprAST>>
    class FunctionAST {
        std::unique_ptr<PrototypeAST> Proto;
        std::vector<std::unique_ptr<ExprAST>> Body;

    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Proto,
            std::vector<std::unique_ptr<ExprAST>> Body)
            : Proto(std::move(Proto)), Body(std::move(Body)) {}
        
        raw_ostream& dump(raw_ostream& out, int ind) {
            indent(out, ind) << "FunctionAST\n";
            ++ind;
            indent(out, ind) << "Body:";

            return Body.at(0) ? Body.at(0)->dump(out, ind) : out << "null\n";
        }
        Function* codegen();
    };
}