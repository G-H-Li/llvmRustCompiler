// llvmRustCompiler.cpp: 定义应用程序的入口点。
//

#include "llvmRustCompiler.h"
using namespace std;

int main()
{
    string fileAddress = "D:\\CodeFile\\lexerTest.txt";
    llvmRustCompiler::Scanner scanner(fileAddress);
    scanner.getNextToken();//开始扫描
    //注意使用Scanner::getErrorFlag()、scanner.getToken().getTokenType()来指示lexer的运行
    while (!llvmRustCompiler::Scanner::getErrorFlag()) {
        scanner.getToken().dump();
        scanner.getNextToken();
    }
    return 0;
}
