// llvmRustCompiler.cpp: 定义应用程序的入口点。
//

#include "llvmRustCompiler.h"
using namespace llvmRustCompiler;
using namespace llvm;
using namespace std;


//对应每个LLVM的线程
LLVMContext TheContext;
//指令生成的辅助对象，用于跟踪插入指令和生成新指令
IRBuilder<> Builder(TheContext);
//代码段中所有函数和全局变量的结构
std::unique_ptr<Module> TheModule;
//记录代码的符号表
std::map<std::string, AllocaInst*> NamedValues;
// Pass 管理
std::unique_ptr<legacy::FunctionPassManager> TheFPM;
//ExecutionEngine* TheExcution;
// JIT
std::unique_ptr<RustJIT> TheJIT;
// 函数映射
std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

//输入文件地址
string fileAddress;
//是否运行
bool isRun;

void InitializeModuleAndPassManager() {
    // Open a new module.
    TheModule = std::make_unique<Module>("my cool jit", TheContext);
    TheModule->setDataLayout(TheJIT->getTargetMachine().createDataLayout());

     // Create a new pass manager attached to it.
    TheFPM = std::make_unique<legacy::FunctionPassManager>(TheModule.get());

    // Do simple "peephole" optimizations and bit-twiddling optzns.
    TheFPM->add(createInstructionCombiningPass());
    // Reassociate expressions.
    TheFPM->add(createReassociatePass());
    // Eliminate Common SubExpressions.
    TheFPM->add(createGVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    TheFPM->add(createCFGSimplificationPass());

    TheFPM->doInitialization();
}

//处理定义函数
void HandleDefinition(Parser& parser) {
    if (auto FnAST = parser.ParseDefinition()) {
        if (auto* FnIR = FnAST->codegen()) {
            fprintf(stderr, "Read function definition:");
            FnIR->print(errs());
            fprintf(stderr, "\n");
            TheJIT->addModule(std::move(TheModule));
            InitializeModuleAndPassManager();
        }
    }
    else {
        // Skip token for error recovery.
        parser.getScanner().getNextToken(); parser.setToken(parser.getScanner().getToken());
    }
}

void HandleTopLevelExpression(Parser& parser) {
    // Evaluate a top-level expression into an anonymous function.
    if (auto FnAST = parser.ParseTopLevelExpr()) {
        if (FnAST->codegen()) {
            // JIT the module containing the anonymous expression, keeping a handle so
            // we can free it later.
            auto H = TheJIT->addModule(std::move(TheModule));
            InitializeModuleAndPassManager();

            // Search the JIT for the __anon_expr symbol.
            auto ExprSymbol = TheJIT->findSymbol("__anon_expr");
            assert(ExprSymbol && "Function not found");

            double (*FP)() = (double (*)())(intptr_t)cantFail(ExprSymbol.getAddress());
            fprintf(stderr, "Evaluated to %f\n", FP());

            // Delete the anonymous expression module from the JIT.
            TheJIT->removeModule(H);
        }
    }
    else {
        // Skip token for error recovery.
        parser.getScanner().getNextToken(); parser.setToken(parser.getScanner().getToken());
    }
}

/// top ::= definition | external | expression | ';'
void MainLoop(Parser& parser) {
    while (true) {
        //TheModule->dump();
        switch (parser.getScanner().getToken().getTokenType()) {
        case TokenType::tok_eof:
            return;
        case TokenType::tok_delimiter: // ignore top-level semicolons.
            parser.getScanner().getNextToken(); parser.setToken(parser.getScanner().getToken());
            break;
        default: {
            switch (parser.getScanner().getToken().getTokenValue()) {

            case TokenValue::KW_FN:
                HandleDefinition(parser);
                break;
            default:
                HandleTopLevelExpression(parser);
                break;
            }
        }
        }
    }
}

// 词法分析器测试
void lexerTest()
{
    Scanner scanner(fileAddress);
    if (scanner.getFileAvailable())
    {
        while (true)
        {
            scanner.getNextToken();
            if (scanner.getErrorFlag())
            {
                //添加当处理到不合法单词时的处理
                cout << "刚刚发生了一些小错误" << endl;
            }
            scanner.getToken().dump();
            if (scanner.getToken().getTokenType() == llvmRustCompiler::TokenType::tok_eof)
                break;
        }
    }
    else
    {
        //添加当文件不可用时的处理
    }
}

//语法分析器测试
void parserTest()
{
    
    Scanner scanner(fileAddress);

    Parser parser(scanner);
    parser.test();
}

//生成器测试
void generatorTest()
{
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
    TheJIT = std::make_unique<RustJIT>();
    InitializeModuleAndPassManager();

    Scanner scanner(fileAddress);
    Parser parser(scanner);

    parser.getScanner().getNextToken(); parser.setToken(parser.getScanner().getToken());

    MainLoop(parser);

    TheModule->dump();
}

/* 以下函数用作输入输出*/
void showUsage()
{
    cout << "Usage   :{lexer|parser|generator|run} <your rust file path>" << endl;
    cout << "Options :" << endl;
    cout << "your rust file path   Your rust file, this option MUST be given, sugggest using absolute address" << endl;
    return;
}

vector<string> split(const string& str, const string& delim) {
    vector<string> res;
    if ("" == str) return res;
    //先将要切割的字符串从string类型转换为char*类型
    char* strs = new char[str.length() + 1]; //不要忘了
    strcpy(strs, str.c_str());

    char* d = new char[delim.length() + 1];
    strcpy(d, delim.c_str());

    char* p = strtok(strs, d);
    while (p) {
        string s = p; //分割得到的字符串转换为string类型
        res.push_back(s); //存入结果数组
        p = strtok(NULL, d);
    }

    return res;
}

int main()
{
    string command;
    showUsage();
    getline(cin, command);
    vector<string> tmp = split(command, " ");
    if (tmp.empty() || tmp.size() < 2) {
        cout << "Please input valid command" << endl;
        showUsage();
    }
    vector<string> path = split(tmp[1], " ");
    if (path.size() > 1) {
        cout << "Please input valid file path" << endl;
    }
    ifstream fin(path[0]);
    if (!fin) {
        cout << "file not exists" << endl;
    }
    else {
        fin.close();
    }
    fileAddress = path[0];
    string device = tmp[0];
    if (device == "lexer") {
        lexerTest();
        cout << endl << "lexer compeleted" << endl;
    }
    else if (device == "parser") {
        parserTest();
        cout << endl << "parser compeleted" << endl;
    }
    else if (device == "generator") {
        isRun = false;
        generatorTest();
        cout << endl << "generator compeleted" << endl;
    }
    else if (device == "run") {
        isRun = true;
        generatorTest();
        cout << endl << "run compeleted" << endl;
    }
    else {
        cout << "command is error" << endl;
    }
    return 0;
}
