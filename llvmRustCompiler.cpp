// llvmRustCompiler.cpp: 定义应用程序的入口点。
//

#include "llvmRustCompiler.h"
using namespace std;

// 词法分析器测试
void lexerTest()
{
       
    //string fileAddress = "D:\\CodeFile\\lexerTest.txt";
    string fileAddress = "C:\\Users\\izumi\\Desktop\\code.txt";
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
}

//语法分析器测试
void parserTest()
{
    string fileAddress = "C:\\Users\\izumi\\Desktop\\code.txt";
    llvmRustCompiler::Scanner scanner(fileAddress);

    llvmRustCompiler::Parser parser(scanner);
    parser.main();
}

int main()
{
    //parserTest();
    return 0;
}
