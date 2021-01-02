#include "token.h"

static std::string IdentifierStr; // Filled in if tok_identifier
static double NumVal;             // Filled in if tok_number

//ʶ�𵥴�
static int gettok() {
    static int LastChar = ' ';

    //��������ǰ��Ŀո�
    while (isspace(LastChar))
        LastChar = getchar();

    //��ʶ��:[_a-zA-Z][_a-zA-Z0-9]*
    if (LastChar == '_') {
        do {
            IdentifierStr += LastChar;
            LastChar = getchar();
        } while (isalnum(LastChar) || LastChar == '_');

        if (IdentifierStr.length() > 1)//����ֻ��'_'
            return tok_identifier;
        return tok_illIdentifier;
    }

    //�ؼ��֡���ʶ��[a-zA-Z][a-zA-Z0-9]*(����������ʱ���ڴ���)
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

    //���ֳ���
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

    //Դ�����β
    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF)
        return tok_eof;

    //������δ֪����
    int ThisChar = LastChar;
    LastChar = getchar();
    return ThisChar;
}

static int CurTok;
static int getNextToken() { return CurTok = gettok(); }


//��ȡ�����������ȼ�
static int GetTokPrecedence() {
    if (!isascii(CurTok))
        return -1;

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0)
        return -1;
    return TokPrec;
}

//�ڵ����ɴ������������Ϣ
std::unique_ptr<ExprAST> LogError(const char* Str) { //�����ڵ�Ĵ���
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
}

std::unique_ptr<FunctionAST> LogErrorP(const char* Str) { //�����ڵ�Ĵ���
    LogError(Str);
    return nullptr;
}

///�������ʽ�ڵ�
//���������ڸ���ParseBinOpRHS(ExprPrec,LHS)�Ķ�����ٸ�������
static std::unique_ptr<ExprAST> ParseExpression();

///�������ֽڵ�
static std::unique_ptr<ExprAST> ParseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(NumVal);
    getNextToken(); // consume the number
    return std::move(Result);
}

///���� () ���õĽڵ�
static std::unique_ptr<ExprAST> ParseParenExpr() {
    getNextToken(); // eat (.
    auto V = ParseExpression();//���� () �ڲ��Ľڵ�
    if (!V)
        return nullptr;

    if (CurTok != ')')
        return LogError("expected ')'");
    getNextToken(); // eat ).
    return V;
}

///������������/�������ýڵ�
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
    std::string IdName = IdentifierStr;

    getNextToken(); // eat identifier.

    if (CurTok != '(') //��"()",Ϊ��������
        return std::make_unique<VariableExprAST>(IdName);

    //����'('��Ϊ��������
    getNextToken(); // eat (
    std::vector<std::unique_ptr<ExprAST>> Args;
    if (CurTok != ')') {
        //��������
        while (true) {
            if (auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if (CurTok == ')')
                break;

            if (CurTok != ',')//����֮������á���������
                return LogError("Expected ')' or ',' in argument list");
            getNextToken();
        }
    }

    // Eat the ')'.
    getNextToken();

    return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

///�����ڵ�--���������ýڵ��ۺ�
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

//����������ʽ�ڵ�
/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS) {
    // If this is a binop, find its precedence.
    while (true) {
        int TokPrec = GetTokPrecedence();

        //��ǰ����������ȼ���"ǰһ��"�ͣ���a*b+c��ֱ�ӽ������������
        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken(); // eat binop

        //�ݹ��ȡ�Ҳ�����
        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
        if (!RHS)
            return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = GetTokPrecedence();
        if (TokPrec < NextPrec) {
            //���ڵݹ飬����ǰ���������ȼ�����һ����ʱ����a+b*c�����������������Ҷ˽ڵ�
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }

        //�ϲ��ò��������˵Ľڵ�
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

///������������
///������:����
static std::unique_ptr<ExprAST> ParseTypePrototype() {
    if (CurTok != tok_identifier)
        return LogError("����ı�����");
    std::string VaName = IdentifierStr;
    getNextToken();
    if (CurTok != ':')//δָ������
        return std::make_unique<TypePrototype>(VaName, "");
    getNextToken();
    std::string VaType = IdentifierStr;
    getNextToken();
    return std::make_unique<TypePrototype>(VaName, VaType);
}

/// ������������
static std::unique_ptr<ExprAST> ParseVariablePrototype() {
    getNextToken(); // eat let.
    auto VariableExpr = ParseTypePrototype();
    if (CurTok != '=')
        return LogError("Expected '=' in prototype");
    getNextToken(); // eat =.
    auto ValueExpr = ParsePrimary();
    return std::make_unique<VariablePrototype>(std::move(VariableExpr), std::move(ValueExpr));
}

///������������
static std::unique_ptr<ExprAST> ParseFunctionPrototype() {
    getNextToken(); // eat fn.

    if (CurTok != tok_identifier)
        return LogErrorP("����ĺ�����");

    std::string FnName = IdentifierStr;
    getNextToken();

    if (CurTok != '(')
        return LogErrorP("Expected '(' in prototype");

    std::vector<std::unique_ptr<ExprAST>> Args;
    while (getNextToken() == tok_identifier) {
        Args.push_back(ParseTypePrototype());
        if (CurTok != ',' && CurTok != ')')
            return LogErrorP("����Ĳ������");
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

///�����������ʽ

static std::unique_ptr<ExprAST> ParseTopLevelExpr() {
    
    return ParseExpression();
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

static void HandleVariableDefinition() {
    if (ParseVariablePrototype()) {
        fprintf(stderr, "�ɹ�������һ������.\n");
    }
    else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleFunctionDefinition() {
    if (ParseFunctionPrototype()) {
        fprintf(stderr, "�ɹ�������һ������.\n");
    }
    else {
        // Skip token for error recovery.
        getNextToken();
    }
}

static void HandleTopLevelExpression() {
    // Evaluate a top-level expression into an anonymous function.
    if (ParseTopLevelExpr()) {
        fprintf(stderr, "����һЩ��������.\n");
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
