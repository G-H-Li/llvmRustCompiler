/*
* Author: �����
* Date:2021/1/2
* description:ö��token�Ͷ���AST
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

// ö�ٵ�������
enum Token {
    tok_eof = -1,

    // commands
    tok_let = -2,//��������
    tok_fn = -3,//��������

    tok_ptr = -4, //->
    //�����ؼ���

    // primary
    tok_identifier = -5, //��ʶ��
    tok_illIdentifier = -6, //�����ʶ��
    tok_number = -7 //���ֳ���
};

// ��������﷨���ڵ�
/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual ~ExprAST() = default;
};

///����ֵ�ڵ�
/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double Val;

public:
    NumberExprAST(double Val) : Val(Val) {}
};

///�����ڵ�
/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    std::string Name;

public:
    VariableExprAST(const std::string& Name) : Name(Name) {}
};

///��Ԫ������ڵ�
/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
        std::unique_ptr<ExprAST> RHS)
        : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

///�������ýڵ�
/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;//������
    std::vector<std::unique_ptr<ExprAST>> Args;//����,����Ϊ���ʽ

public:
    CallExprAST(const std::string& Callee,
        std::vector<std::unique_ptr<ExprAST>> Args)
        : Callee(Callee), Args(std::move(Args)) {}
};

///���������ڵ�
class TypePrototype : public ExprAST {
    std::string Name;//������
    std::string Type;//����
public:
    TypePrototype(std::string Name, std::string Type)
        :Name(Name), Type(Type) {}
};

///���������ڵ�
//let identifier:type = vlaue
class VariablePrototype : public ExprAST {
    std::unique_ptr<ExprAST> NameExpr;//����������ʱ�ɼ����Ϳɲ���
    std::unique_ptr<ExprAST> Value;//ֵ
public:
    VariablePrototype(std::unique_ptr<ExprAST> NameExpr,
        std::unique_ptr<ExprAST> Value)
        :NameExpr(std::move(NameExpr)), Value(std::move(Value)) {}
};

///���������ڵ�
///����ʱ�����������
/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public ExprAST {
    std::string Name;//������
    std::vector<std::unique_ptr<ExprAST>> Args;//����,ʹ��TypePrototype
    std::vector<std::unique_ptr<ExprAST>> Body;//������
    std::string ReturnType;//����ֵ����

public:
    FunctionAST(std::string Name, std::vector<std::unique_ptr<ExprAST>> Args,
        std::vector<std::unique_ptr<ExprAST>> Body, std::string ReturnType)
        :Name(Name), Args(std::move(Args)), Body(std::move(Body)), ReturnType(ReturnType) {}
};

//Ϊ�������������ȼ�
static std::map<char, int> BinopPrecedence;

void MainLoop();