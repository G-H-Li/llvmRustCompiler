// llvmRustCompiler.cpp: 定义应用程序的入口点。
//

#include "llvmRustCompiler.h"
#include "scanner.h"
using namespace std;

int main()
{
    string fileName = "D:\\CodeFile\\lexerTest.txt";//使用绝对路径
    llvmRustCompiler::Scanner scanner(fileName);
    scanner.getNextToken();

    while (scanner.getToken().getTokenType() != llvmRustCompiler::TokenType::tok_eof)
    {
        scanner.getToken().dump();
        scanner.getNextToken();
    }
    return 0;
}
