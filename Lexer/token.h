/*
* Author: 张清锋
* Date:2021/1/2
* description:枚举token和定义AST
* latest date:2021/1/2
*/

#include <string>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

// 枚举单词类型
enum Token {
    tok_eof = -1,

    // commands
    tok_let = -2,//声明变量
    tok_fn = -3,//声明函数

    tok_ptr = -4, //->
    //其他关键字

    // primary
    tok_identifier = -5, //标识符
    tok_illIdentifier = -6, //错误标识符
    tok_number = -7 //数字常量
};

// 定义抽象语法树节点
/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual ~ExprAST() = default;
};

///数字值节点
/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double Val;

public:
    NumberExprAST(double Val) : Val(Val) {}
};

///变量节点
/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    std::string Name;

public:
    VariableExprAST(const std::string& Name) : Name(Name) {}
};

///二元运算符节点
/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
        std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

///函数调用节点
/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;//函数名
    std::vector<std::unique_ptr<ExprAST>> Args;//参数,可能为表达式

public:
    CallExprAST(const std::string& Callee,
        std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
};

///类型声明节点
class TypePrototype : public ExprAST {
    std::string Name;//变量名
    std::string Type;//类型
public:
    TypePrototype(std::string Name, std::string Type)
        :Name(Name), Type(Type) {}
};

///变量声明节点
//let identifier:type = vlaue
class VariablePrototype : public ExprAST {
    std::unique_ptr<ExprAST> NameExpr;//变量名声明时可加类型可不加
    std::unique_ptr<ExprAST> Value;//值
public:
    VariablePrototype(std::unique_ptr<ExprAST> NameExpr,
        std::unique_ptr<ExprAST> Value)
        :NameExpr(std::move(NameExpr)), Value(std::move(Value)) {}
};

///函数声明节点
///声明时必须给出定义
/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public ExprAST {
    std::string Name;//函数名
    std::vector<std::unique_ptr<ExprAST>> Args;//参数,使用TypePrototype
    std::vector<std::unique_ptr<ExprAST>> Body;//函数体
    std::string ReturnType;//返回值类型

public:
    FunctionAST(std::string Name, std::vector<std::unique_ptr<ExprAST>> Args,
        std::vector<std::unique_ptr<ExprAST>> Body, std::string ReturnType)
        :Name(Name), Args(std::move(Args)), Body(std::move(Body)), ReturnType(ReturnType) {}
};

//为操作符定义优先级
static std::map<char, int> BinopPrecedence;

void MainLoop();