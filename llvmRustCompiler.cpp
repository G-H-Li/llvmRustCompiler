// llvmRustCompiler.cpp: 定义应用程序的入口点。
//

#include "llvmRustCompiler.h"
#include "Lexer/scanner.h"
using namespace std;

int main()
{
    string fileAddress = "D:\\CodeFile\\lexerTest.txt";
    llvmRustCompiler::Scanner scanner("fileAddress");
    scanner.getNextToken();

    while (!llvmRustCompiler::Scanner::getErrorFlag) {
        scanner.getToken().dump();
        scanner.getNextToken();
        if (scanner.getToken().getTokenType() == llvmRustCompiler::TokenType::tok_eof)
            break;
    }
    
    return 0;
}
