// llvmRustCompiler.cpp: 定义应用程序的入口点。
//

#include "llvmRustCompiler.h"
#include "Lexer/scanner.h"
using namespace std;

int main()
{
    string fileAddress = "D:\\CodeFile\\lexerTest.txt";
    llvmRustCompiler::Scanner scanner(fileAddress);
    if (scanner.getFileAvailable())
    {
        scanner.getNextToken();//开始扫描

        while (scanner.getToken().getTokenType() != llvmRustCompiler::TokenType::tok_eof)
        {
            scanner.getToken().dump();
            if (scanner.getErrorFlag())
            {
                //添加当处理到不合法单词时的处理
                cout << "刚刚发生了一些小错误" << endl;
            }
            scanner.getNextToken();
        }
    }
    else 
    {
        //添加当文件不可用时的处理
    }
    return 0;
}
