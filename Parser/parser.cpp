#include "..\Error\error.h"
#include "parser.h"


namespace llvmRustCompiler {

    /// numberexpr ::= number
    std::unique_ptr<ExprAST> Parser::ParseFPNumberExpr() {
        //获取小数的位置和值
        TokenLocation location = scanner.getToken().getTokenLocation();
        float value = scanner.getToken().getFloatValue();

        auto Result = std::make_unique<FPNumberExprAST>(location, value);
        scanner.getNextToken(); token = scanner.getToken(); // consume the number
        return std::move(Result);
    };

    //解析整数
    std::unique_ptr<ExprAST> Parser::ParseIntNumberExpr() {
        //获取整数的位置和值
        TokenLocation location = scanner.getToken().getTokenLocation();
        int value = scanner.getToken().getIntValue();

        int bites = 32;//太难判断了，默认32位整数
        bool IsSigned = true; //不好判断，直接默认是带符号的

        auto Result = std::make_unique<IntNumberExprAST>(location, value, bites, IsSigned);
        scanner.getNextToken(); token = scanner.getToken(); // consume the number
        return std::move(Result);
    }

    /// parenexpr ::= '(' expression ')'
    std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
        scanner.getNextToken(); token = scanner.getToken(); // eat (.
        auto V = ParseExpression();
        if (!V)
            return nullptr;

        if (scanner.getToken().getTokenValue() != TokenValue::RIGHT_PAREN) {
            errorParser("expected ')'");
            return nullptr;
        }

        scanner.getNextToken(); token = scanner.getToken(); // eat ).
        return V;
    }

    /// identifierexpr
    ///   ::= identifier
    ///   ::= identifier '(' expression* ')'
    std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
        std::string IdName = scanner.getToken().getTokenName();

        scanner.getNextToken(); token = scanner.getToken(); // eat identifier.


        // 判断变量类型
        TokenType Type = TokenType::tok_integer;//首先默认是浮点数型
        if (scanner.getToken().getTokenValue() == TokenValue::COLON) //判断是否等于冒号 :
        {
            scanner.getNextToken(); token = scanner.getToken();
            //判断整数和浮点数
            if (scanner.getToken().getTokenValue() == TokenValue::KW_I32) {
                Type = TokenType::tok_integer;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_I64) {
                Type = TokenType::tok_integer;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_F32) {
                Type = TokenType::tok_float;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_F64) {
                Type = TokenType::tok_float;
                scanner.getNextToken(); token = scanner.getToken();
            }
            
            //判断布尔值
            if (scanner.getToken().getTokenName()._Equal("bool")) {
                Type = TokenType::tok_bool;
                scanner.getNextToken(); token = scanner.getToken();
            }
            
            //判断char
            if (scanner.getToken().getTokenName()._Equal("char")) {
                Type = TokenType::tok_char;
                scanner.getNextToken(); token = scanner.getToken();
            }
            //判断string
            if (scanner.getToken().getTokenName()._Equal("string")) {
                Type = TokenType::tok_string;
                scanner.getNextToken(); token = scanner.getToken();
            }
            
        }

        //获取变量地址
        TokenLocation VariableLocation = scanner.getToken().getTokenLocation();

        if (scanner.getToken().getTokenValue() != TokenValue::LEFT_PAREN) // Simple variable ref.
            return std::make_unique<VariableExprAST>(VariableLocation, IdName, Type);



        // Call.
        scanner.getNextToken(); token = scanner.getToken(); // eat (
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
                scanner.getNextToken(); token = scanner.getToken();
            }
        }

        // Eat the ')'.
        scanner.getNextToken(); token = scanner.getToken();

        //获取函数调用地址Calllocation
        TokenLocation CallLocation = scanner.getToken().getTokenLocation();
        return std::make_unique<CallExprAST>(CallLocation, IdName, std::move(Args));
    }


    //解析let mut x = 3;
    std::unique_ptr<ExprAST> Parser::ParseLet() {
        scanner.getNextToken(); token = scanner.getToken(); //吃掉let

        //判断有无 mut 确定是否是常量
        //此处判断mut无实际作用，无论有无mut的返回值都是一样的。就是让语法能好看点写成"let mut x = 3"
        bool isConst = false;
        if (scanner.getToken().getTokenValue() == TokenValue::KW_MUT)
        {
            isConst = true;
            scanner.getNextToken(); token = scanner.getToken();//吃掉mut
        }

        //创建一个存放变量信息的容器
        std::vector<std::pair<std::string, std::unique_ptr<ExprAST>>> VarNames;

        //判断有无变量名
        if (scanner.getToken().getTokenType() != TokenType::tok_identifier)
            return nullptr;
        
        std::string Name = scanner.getToken().getTokenName();//为函数变量名赋值

        scanner.getNextToken(); token = scanner.getToken();//吃掉变量名

        //设置默认类型是整数
        TokenType Type = TokenType::tok_integer;
        if (scanner.getToken().getTokenValue() == TokenValue::COLON)//如果等于冒号
        {
            scanner.getNextToken(); token = scanner.getToken(); // 吃掉冒号

            //判断整数和浮点数
            if (scanner.getToken().getTokenValue() == TokenValue::KW_I32) {
                Type = TokenType::tok_integer;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_I64) {
                Type = TokenType::tok_integer;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_F32) {
                Type = TokenType::tok_float;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_F64) {
                Type = TokenType::tok_float;
                scanner.getNextToken(); token = scanner.getToken();
            }

            //判断布尔值
            if (scanner.getToken().getTokenName()._Equal("bool")) {
                Type = TokenType::tok_bool;
                scanner.getNextToken(); token = scanner.getToken();
            }

            //判断char
            if (scanner.getToken().getTokenName()._Equal("char")) {
                Type = TokenType::tok_char;
                scanner.getNextToken(); token = scanner.getToken();
            }
            //判断string
            if (scanner.getToken().getTokenName()._Equal("string")) {
                Type = TokenType::tok_string;
                scanner.getNextToken(); token = scanner.getToken();
            }
        }

        // Read the optional initializer.
        std::unique_ptr<ExprAST> Init = nullptr;
        if (scanner.getToken().getTokenValue() == TokenValue::EQUAL) {
            scanner.getNextToken(); token = scanner.getToken(); // 吃掉等号
            Init = ParseExpression();
            if (!Init) return nullptr;
        }
        else {
            errorParser("expected '=' but not found");
            return nullptr;
        }

        //存入变量名以及变量的初始化表达式信息
        VarNames.push_back(std::make_pair(Name, std::move(Init)));
                
        
        //获取地址location
        TokenLocation Location = scanner.getToken().getTokenLocation();

        /*auto Body = ParseExpression();
        if (!Body)
            return nullptr;*/
        //返回结点值
        return std::make_unique<VarExprAST>(Location, std::move(VarNames), Type);
    }


    /// ifexpr ::= 'if' expression 'then' expression 'else' expression
    std::unique_ptr<ExprAST> Parser::ParseIfExpr() {
        scanner.getNextToken(); token = scanner.getToken(); // eat the if.

        // condition.
        auto Cond = ParseExpression();
        if (!Cond)
            return nullptr;

        //if函数体内的内容
        std::vector<std::unique_ptr<ExprAST>> If;


        scanner.getNextToken(); token = scanner.getToken(); //吃掉左大括号 {

        while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
            If.push_back(std::move(ParseExpression()));
            if (scanner.getToken().getTokenValue() == TokenValue::SEMICOLON)
                scanner.getNextToken(); token = scanner.getToken(); //吃掉函数体中的分号 ';'
        }

        scanner.getNextToken(); token = scanner.getToken(); //吃掉右大括号


        if (scanner.getToken().getTokenValue() != TokenValue::KW_ELSE) {
            errorParser("expected else");
            return nullptr;
        }

        scanner.getNextToken(); token = scanner.getToken(); //吃掉else

        //else函数体内的内容
        std::vector<std::unique_ptr<ExprAST>> Else;


        scanner.getNextToken(); token = scanner.getToken(); //吃掉左大括号 {

        while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
            Else.push_back(std::move(ParseExpression()));
            if (scanner.getToken().getTokenValue() == TokenValue::SEMICOLON)
                scanner.getNextToken(); token = scanner.getToken(); //吃掉函数体中的分号 ';'
        }

        scanner.getNextToken(); token = scanner.getToken(); //吃掉右大括号

        TokenLocation location = scanner.getToken().getTokenLocation();

        return std::make_unique<IfExprAST>(location, std::move(Cond), std::move(If),
            std::move(Else));
    }

    /// forexpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
    std::unique_ptr<ExprAST> Parser::ParseForExpr() {
        scanner.getNextToken(); token = scanner.getToken(); // eat the for.

        if (scanner.getToken().getTokenType() != TokenType::tok_identifier) {
            errorParser("expected identifier after for");
            return nullptr;
        }

        std::string IdName = scanner.getToken().getTokenName(); //解析到 "for i"
        scanner.getNextToken(); token = scanner.getToken(); // eat identifier.

        if (scanner.getToken().getTokenValue() != TokenValue::KW_IN) {
            errorParser("expected 'in' after for");
            return nullptr;
        }
        scanner.getNextToken(); token = scanner.getToken(); // eat 'in'.  //解析到 "for i in"

        auto Start = ParseExpression(); //解析到 "for i in 0"
        if (!Start)
            return nullptr;
        if (scanner.getToken().getTokenValue() != TokenValue::PERIOD) {
            errorParser("expected '.' after for start value");
            return nullptr;
        }
        scanner.getNextToken(); token = scanner.getToken(); //吃掉第一个 '.' 解析到 " for i in 0. "
        scanner.getNextToken(); token = scanner.getToken(); //吃掉第二个 '.' 解析到 " for i in 0.. "

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


        scanner.getNextToken(); token = scanner.getToken(); //吃掉左大括号 {

        while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
            Body.push_back(std::move(ParseExpression()));
            scanner.getNextToken(); token = scanner.getToken(); //吃掉函数体中的分号 ';'
        }

        scanner.getNextToken(); token = scanner.getToken(); //吃掉右大括号

        TokenLocation location = scanner.getToken().getTokenLocation();
        return std::make_unique<ForExprAST>(location, IdName, std::move(Start), std::move(End),
            std::move(Step), std::move(Body));
    }



    // 此while仅能解析如下示例
    // while number < 4 { 语句1; 语句2;}
    std::unique_ptr<ExprAST> Parser::ParseWhileExpr() {
        if (scanner.getToken().getTokenValue() != TokenValue::KW_WHILE) {
            errorParser(" expected while keyword");
            return nullptr;
        }
        scanner.getNextToken(); token = scanner.getToken(); //吃掉while

        //直接解析终止条件 End
        auto End = ParseExpression();
        if (!End) {
            errorParser("no end conditon");
            return nullptr;
        }

        scanner.getNextToken(); token = scanner.getToken(); //吃掉左大括号 {

        //创建一个Body容器
        std::vector<std::unique_ptr<ExprAST>> Body;

        while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
            Body.push_back(std::move(ParseExpression()));
            if (scanner.getToken().getTokenValue() == TokenValue::SEMICOLON)
                scanner.getNextToken(); token = scanner.getToken(); //吃掉函数体中的分号 ';'
        }

        scanner.getNextToken(); token = scanner.getToken(); //吃掉右大括号

        TokenLocation location = scanner.getToken().getTokenLocation();
        return std::make_unique<WhileExprAST>(location, std::move(End), std::move(Body));

    }



    /// primary
    ///   ::= identifierexpr
    ///   ::= numberexpr
    ///   ::= parenexpr
    ///   ::= ifexpr
    ///   ::= forexpr
    std::unique_ptr<ExprAST> Parser::ParsePrimary() {
        //大层swich判断类型，处理 identity和number类型
        switch (scanner.getToken().getTokenType())
        {

        case TokenType::tok_identifier:
            return ParseIdentifierExpr();

        case TokenType::tok_float:
            return ParseFPNumberExpr();
            break;
        case TokenType::tok_integer:
            return ParseIntNumberExpr();
            break;
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
            case TokenValue::KW_WHILE:
                return ParseWhileExpr();
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
    std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(int ExprPrec,
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
            scanner.getNextToken(); token = scanner.getToken(); // eat binop

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
                std::make_unique<BinaryExprAST>(location, BinOp, std::move(LHS), std::move(RHS));
        }
    }


    ///解析 函数声明中括号的一个参数。 形状如 " f : i32"
    std::unique_ptr<VariableExprAST> Parser::ParseArgument() {
        if (scanner.getToken().getTokenType() != TokenType::tok_identifier) {
            errorParser("no argument name");
            return nullptr;
        }

        std::string ArgumentName = scanner.getToken().getTokenName();
        scanner.getNextToken(); token = scanner.getToken();//吃掉参数名

        // 判断变量类型
        TokenType Type = TokenType::tok_integer;//首先默认是浮点数型
        if (scanner.getToken().getTokenValue() == TokenValue::COLON) //判断是否等于冒号 :
        {
            
            scanner.getNextToken(); token = scanner.getToken();//吃掉冒号 :
            //判断整数和浮点数
            if (scanner.getToken().getTokenValue() == TokenValue::KW_I32) {
                Type = TokenType::tok_integer;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_I64) {
                Type = TokenType::tok_integer;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_F32) {
                Type = TokenType::tok_float;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_F64) {
                Type = TokenType::tok_float;
                scanner.getNextToken(); token = scanner.getToken();
            }

            //判断布尔值
            if (scanner.getToken().getTokenName()._Equal("bool")) {
                Type = TokenType::tok_bool;
                scanner.getNextToken(); token = scanner.getToken();
            }

            //判断char
            if (scanner.getToken().getTokenName()._Equal("char")) {
                Type = TokenType::tok_char;
                scanner.getNextToken(); token = scanner.getToken();
            }
            //判断string
            if (scanner.getToken().getTokenName()._Equal("string")) {
                Type = TokenType::tok_string;
                scanner.getNextToken(); token = scanner.getToken();
            }
        }

        //获取地址
        TokenLocation location = scanner.getToken().getTokenLocation();

        return std::make_unique<VariableExprAST>(location, ArgumentName, Type);

    }


    /// expression
    ///   ::= primary binoprhs
    ///
    std::unique_ptr<ExprAST> Parser::ParseExpression() {
        auto LHS = ParsePrimary();
        if (!LHS)
            return nullptr;

        //处理分号问题
        switch (scanner.getToken().getTokenValue())
        {
        case TokenValue::KW_LET:
            return LHS;
        case TokenValue::KW_IF:
            return LHS;
        case TokenValue::KW_FOR:
            return LHS;
        case TokenValue::KW_WHILE:
            return LHS;
        default:
            break;
        }

        return ParseBinOpRHS(0, std::move(LHS));
    }

    /// prototype
    ///   ::= id '(' id* ')'
    std::unique_ptr<PrototypeAST> Parser::ParsePrototype() {
        if (scanner.getToken().getTokenType() != TokenType::tok_identifier) {
            errorParser("Expected function name in prototype");
            return nullptr;
        }

        std::string FnName = scanner.getToken().getTokenName();
        scanner.getNextToken(); token = scanner.getToken();

        if (scanner.getToken().getTokenValue() != TokenValue::LEFT_PAREN) {
            errorParser("Expected '(' in prototype");
            return nullptr;
        }

        scanner.getNextToken(); token = scanner.getToken(); //吃掉(


        //std::vector<std::unique_ptr<ExprAST>> ArgNames;

        //参数容器
        std::vector<std::pair<TokenType, std::string>> Args;
        //一个参数的容器
        std::pair<TokenType, std::string> Arg;

        //添加参数
        while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_PAREN) {

            //获取一个参数
            std::unique_ptr<VariableExprAST> E = ParseArgument();
            TokenType  type = E->getType();
            std::string name = E->getName();

            //为pair赋值
            Arg.first = type;
            Arg.second = name;

            //将参数传入容器
            Args.push_back(std::move(Arg));
            if (scanner.getToken().getTokenValue() == TokenValue::RIGHT_PAREN)
                break;
            scanner.getNextToken(); token = scanner.getToken(); //吃掉 ','
        }

        scanner.getNextToken(); token = scanner.getToken(); // eat ')'.


        /*while (scanner.getNextToken() == TokenType::tok_identifier)
            ArgNames.push_back(std::move(ParseIdentifierExpr()));
        if (scanner.getToken().getTokenValue() != TokenValue::RIGHT_PAREN)
            return LogErrorP("Expected ')' in prototype");*/

            //判断返回类型
        TokenType return_type = TokenType::tok_integer; //默认返回类型是float
        if (scanner.getToken().getTokenValue() == TokenValue::POINTER)
        {
            scanner.getNextToken(); token = scanner.getToken(); //吃掉 ->

            //判断整数和浮点数
            if (scanner.getToken().getTokenValue() == TokenValue::KW_I32) {
                return_type = TokenType::tok_integer;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_I64) {
                return_type = TokenType::tok_integer;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_F32) {
                return_type = TokenType::tok_float;
                scanner.getNextToken(); token = scanner.getToken();
            }
            if (scanner.getToken().getTokenValue() == TokenValue::KW_F64) {
                return_type = TokenType::tok_float;
                scanner.getNextToken(); token = scanner.getToken();
            }

            //判断布尔值
            if (scanner.getToken().getTokenName()._Equal("bool")) {
                return_type = TokenType::tok_bool;
                scanner.getNextToken(); token = scanner.getToken();
            }

            //判断char
            if (scanner.getToken().getTokenName()._Equal("char")) {
                return_type = TokenType::tok_char;
                scanner.getNextToken(); token = scanner.getToken();
            }
            //判断string
            if (scanner.getToken().getTokenName()._Equal("string")) {
                return_type = TokenType::tok_string;
                scanner.getNextToken(); token = scanner.getToken();
            }
        }

        //获取地址
        TokenLocation location = scanner.getToken().getTokenLocation();

        return std::make_unique<PrototypeAST>(location, FnName, std::move(Args), return_type);
    }

    /// definition ::= 'def' prototype expression
    std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
        scanner.getNextToken(); token = scanner.getToken(); // eat def.
        auto Proto = ParsePrototype();
        if (!Proto)
            return nullptr;

        //解析大括号
        if (scanner.getToken().getTokenValue() != TokenValue::LEFT_BRACE)
            return nullptr;
        std::vector<std::unique_ptr<ExprAST>> Body;

        scanner.getNextToken(); token = scanner.getToken(); //吃掉左大括号
        while (scanner.getToken().getTokenValue() != TokenValue::RIGHT_BRACE) {
            if (scanner.getToken().getTokenValue() == TokenValue::KW_RETURN) {
                scanner.getNextToken(); token = scanner.getToken();
                continue;
            }
            Body.push_back(std::move(ParseExpression()));
            if (scanner.getToken().getTokenValue() == TokenValue::SEMICOLON)
                scanner.getNextToken(); token = scanner.getToken(); //吃掉函数体中的分号 ';'
        }

        scanner.getNextToken(); token = scanner.getToken(); //吃掉右大括号
        /*if (auto E = ParseExpression())
            return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));*/

        return std::make_unique<FunctionAST>(std::move(Proto), std::move(Body));
    }

    /// toplevelexpr ::= expression
    std::unique_ptr<FunctionAST> Parser::ParseTopLevelExpr() {
        if (auto E = ParseExpression()) {

            //获取地址
            TokenLocation location = scanner.getToken().getTokenLocation();
            // Make an anonymous proto.
            auto Proto = std::make_unique<PrototypeAST>(location, "__anon_expr",
                std::vector<std::pair<TokenType, std::string>>(), TokenType::tok_integer);

            //修改了函数体为数组，因此要添加一个数组创建FunctionAST

            std::vector<std::unique_ptr<ExprAST>> Body;
            Body.push_back(std::move(E));

            return std::make_unique<FunctionAST>(std::move(Proto), std::move(Body));
        }
        return nullptr;
    }

    Scanner& Parser::getScanner()
    {
        return scanner;
    }

    Token& Parser::setToken(Token& token)
    {
        return this->token = token;
    }

    void Parser::HandleDefinition() {
        if (ParseDefinition()) {
            fprintf(stderr, "Read function definition: \n");
        }
        else {
            // Skip token for error recovery.
            scanner.getNextToken(); token = scanner.getToken();
        }
    }


    void Parser::HandleTopLevelExpression() {
        // Evaluate a top-level expression into an anonymous function.
        if (ParseTopLevelExpr()) {
            fprintf(stderr, "Top Level Expression to Paser \n");
        }
        else {
            // Skip token for error recovery.
            scanner.getNextToken(); token = scanner.getToken();
        }
    }


    /// top ::= definition | external | expression | ';'
    void Parser::MainLoop() {
        while (true) {
            fprintf(stderr, "ready> ");
            switch (scanner.getToken().getTokenType()) {

            case TokenType::tok_eof:
                return;
            case TokenType::tok_delimiter:
                scanner.getNextToken(); token = scanner.getToken();
                break;
            default: {
                switch (scanner.getToken().getTokenValue()) {

                case TokenValue::KW_FN:
                    HandleDefinition();
                    break;
                default:
                    HandleTopLevelExpression();
                    break;
                }
            }
            }
        }
    }


    int Parser::test() {

        // Prime the first token.
        fprintf(stderr, "ready> ");
        scanner.getNextToken(); token = scanner.getToken();


        //token = scanner.getToken();

        // Run the main "interpreter loop" now.
        MainLoop();

        return 0;
    }
}