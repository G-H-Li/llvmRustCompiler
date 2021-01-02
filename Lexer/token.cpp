#include "token.h"

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number

//识别单词
static int gettok() {
    static int LastChar = ' ';

    //跳过单词前面的空格
    while (isspace(LastChar))
        LastChar = getchar();

    //标识符:[_a-zA-Z][_a-zA-Z0-9]*
    if (LastChar == '_') {
        do {
            IdentifierStr += LastChar;
            LastChar = getchar();
        } while (isalnum(LastChar) || LastChar == '_');

        if (IdentifierStr.length() > 1)//不能只有'_'
            return tok_identifier;
        return tok_illIdentifier;
    }

    //关键字、标识符[a-zA-Z][a-zA-Z0-9]*(类型名等暂时归于此类)
    if (isalpha(LastChar)) {
        do {
            IdentifierStr += LastChar;
            LastChar = getchar();
        } while (isalnum(LastChar) || LastChar == '_');

        if (IdentifierStr == "let")
            return tok_let;
        if (IdentifierStr == "fn")
            return tok_fn;
        return tok_identifier;
    }

    //数字常量
    if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
        std::string NumStr;
        do {
            NumStr += LastChar;
            LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.');

        NumVal = strtod(NumStr.c_str(), nullptr);
        return tok_number;
    }

    //->
    if (LastChar == '-') {
        int ThisChar = LastChar;
        LastChar = getchar();
        if (LastChar == '>')
            return tok_ptr;
        return ThisChar;
    }

    if (LastChar == '#') {
        // Comment until end of line.
        do
            LastChar = getchar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

        if (LastChar != EOF)
            return gettok();
    }

    //源程序结尾
    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF)
        return tok_eof;

    //其他的未知单词
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}

static int CurTok;
static int getNextToken() { return CurTok = gettok(); }


//获取操作符的优先级
static int GetTokPrecedence() {
    if (!isascii(CurTok))
        return -1;

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0)
        return -1;
    return TokPrec;
}

//节点生成错误并输出错误信息
std::unique_ptr<ExprAST> LogError(const char* Str) { //变量节点的错误
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<FunctionAST> LogErrorP(const char* Str) { //函数节点的错误
    LogError(Str);
    return nullptr;
}

///分析表达式节点
//先声明，在给出ParseBinOpRHS(ExprPrec,LHS)的定义后再给出定义
static std::unique_ptr<ExprAST> ParseExpression();

///分析数字节点
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return std::move(Result);
}

///分析 () 引用的节点
static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); // eat (.
    auto V = ParseExpression();//分析 () 内部的节点
    if (!V)
        return nullptr;

    if (CurTok != ')')
        return LogError("expected ')'");
    getNextToken(); // eat ).
    return V;
}

///分析变量引用/函数调用节点
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;

    getNextToken(); // eat identifier.

    if (CurTok != '(') //无"()",为变量引用
        return std::make_unique<VariableExprAST>(IdName);

    //含有'('，为函数调用
    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        //分析参数
        while (true) {
            if (auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if (CurTok == ')')
                break;

            if (CurTok != ',')//参数之间必须用“，”隔开
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    // Eat the ')'.
    getNextToken();

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

///基本节点--常量和引用节点综合
static std::unique_ptr<ExprAST> ParsePrimary() {
    switch (CurTok) {
    case tok_identifier:
        return ParseIdentifierExpr();
    case tok_number:
        return ParseNumberExpr();
    case '(':
        return ParseParenExpr();
    default:
        return LogError("unknown token when expecting an expression");
    }
}

//分析运算表达式节点
/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
    // If this is a binop, find its precedence.
    while (true) {
        int TokPrec = GetTokPrecedence();

        //当前运算符的优先级比"前一个"低，如a*b+c，直接将左操作数返回
        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken(); // eat binop

        //递归读取右操作数
        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
        if (!RHS)
            return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            //用于递归，当当前操作符优先级比下一个低时，如a+b*c，继续分析操作符右端节点
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        //合并该操作符两端的节点
        // Merge LHS/RHS.
        LHS =
            std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}

/// expression
///   ::= primary binoprhs
///
static std::unique_ptr<ExprAST> ParseExpression() {
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}

///分析类型声明
///变量名:类型
static std::unique_ptr<ExprAST> ParseTypePrototype() {
    if (CurTok != tok_identifier)
        return LogError("错误的变量名");
    std::string VaName = IdentifierStr;
    getNextToken();
    if (CurTok != ':')//未指定类型
        return std::make_unique<TypePrototype>(VaName, "");
    getNextToken();
    std::string VaType = IdentifierStr;
    getNextToken();
    return std::make_unique<TypePrototype>(VaName, VaType);
}

/// 分析变量声明
static std::unique_ptr<ExprAST> ParseVariablePrototype() {
    getNextToken(); // eat let.
    auto VariableExpr = ParseTypePrototype();
    if (CurTok != '=')
        return LogError("Expected '=' in prototype");
    getNextToken(); // eat =.
    auto ValueExpr = ParsePrimary();
    return std::make_unique<VariablePrototype>(std::move(VariableExpr), std::move(ValueExpr));
}

///分析函数声明
static std::unique_ptr<ExprAST> ParseFunctionPrototype() {
    getNextToken(); // eat fn.

    if (CurTok != tok_identifier)
        return LogErrorP("错误的函数名");

    std::string FnName = IdentifierStr;
    getNextToken();

    if (CurTok != '(')
        return LogErrorP("Expected '(' in prototype");

    std::vector<std::unique_ptr<ExprAST>> Args;
    while (getNextToken() == tok_identifier) {
        Args.push_back(ParseTypePrototype());
        if (CurTok != ',' && CurTok != ')')
            return LogErrorP("错误的参数组合");
    }

    if (CurTok != ')')
        return LogErrorP("Expected ')' in prototype");

    getNextToken(); // eat ')'.
    std::string ReturnType = "";
    if (CurTok == tok_ptr) {
        getNextToken();
        ReturnType = IdentifierStr;
        getNextToken();
    }

    if (CurTok != '{')
        return LogErrorP("Expected '{' in prototype");
    std::vector<std::unique_ptr<ExprAST>> Body;
    do {
        getNextToken();
        Body.push_back(ParseExpression());
    } while (CurTok != '}');

    getNextToken();
    return std::make_unique<FunctionAST>(FnName, std::move(Args), std::move(Body), ReturnType);
}

///分析顶级表达式

static std::unique_ptr<ExprAST> ParseTopLevelExpr() {
    
    return ParseExpression();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

static void HandleVariableDefinition() {
    if (ParseVariablePrototype()) {
        fprintf(stderr, "成功声明了一个变量.\n");
    }
    else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleFunctionDefinition() {
    if (ParseFunctionPrototype()) {
        fprintf(stderr, "成功声明了一个函数.\n");
    }
    else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (ParseTopLevelExpr()) {
        fprintf(stderr, "做了一些其他事情.\n");
    }
    else {
        // Skip token for error recovery.
        getNextToken();
    }
}

/// top ::= definition | external | expression | ';'
void MainLoop() {
    while (true) {
        fprintf(stderr, "ready> ");
        getNextToken();
        switch (CurTok) {
        case tok_eof:
            return;
        case ';': // ignore top-level semicolons.
            getNextToken();
            break;
        case tok_let:
            HandleVariableDefinition();
            break;
        case tok_fn:
            HandleFunctionDefinition();
            break;
        default:
            HandleTopLevelExpression();
            break;
        }
    }
}
