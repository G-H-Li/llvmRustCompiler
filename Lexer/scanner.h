#/*
* Author: zqf
* Date:2021/1/2
* description:Scanner扫描器，带缓冲区,每次处理一个单词
* latest date:2021/1/9
*/

#ifndef SCANNER_H_
#define SCANNER_H_

#include <fstream>
#include <string>
#include "token.h"
#include "dictionary.h"

namespace llvmRustCompiler
{

    class Scanner
    {
    public:
        explicit        Scanner(const std::string& srcFileName);
        //使用getToken()直接获取缓冲区单词，不会扫描源程序
        Token           getToken() const;
        //使用getNextToken()会扫描源程序,尝试读取下一个单词
        Token           getNextToken();
        //使用getErrorFlag()来获取lexer的报错信息，选择性停止getNextToken()
        static bool     getErrorFlag();
        static void     setErrorFlag(bool flag);
        //当文件不可用时，应停止使用lexer
        static bool     getFileAvailable();
        static void     setFileAvailable(bool flag);

    private:
        void            getNextChar();
        char            peekChar();
        void            addToBuffer(char c);
        void            reduceBuffer();

        void            makeToken(TokenType tt, TokenValue tv,
            const TokenLocation& loc, std::string name, int symbolPrecedence);
        void            makeToken(TokenType tt, TokenValue tv,
            const TokenLocation& loc, long intValue, std::string name);
        void            makeToken(TokenType tt, TokenValue tv,
            const TokenLocation& loc, float floatValue, std::string name);
        void            makeToken(TokenType tt, TokenValue tv,
            const TokenLocation& loc, const std::string& strVale, std::string name);

        //读取单词序列
        void            preprocess();
        void            handleComment();//处理注释
        TokenLocation   getTokenLocation() const;

        void            handleEOFState();
        void            handleIdentifierState();
        void            handleNumberState();
        void            handleCharState();
        void            handleStringState();
        void            handleOperationState();

        //以下为处理数字
        void            handleDigit();
        void            handleXDigit();//处理16进制数字
        void            handleFraction();
        void            handleExponent();

        //当分析出错后单词剩余部分一并处理
        void            handleWrongState();
        void            errorReport(const std::string& msg);

    public:
        enum class State
        {
            NONE,
            END_OF_FILE,
            IDENTIFIER,
            NUMBER,
            CHAR,
            STRING,
            OPERATION
        };

    private:
        std::string         fileName_;
        std::ifstream       input_;
        long                line_;          //当前读取的字符行号
        long                column_;        //当前读取的字符列号
        TokenLocation       loc_;
        char                currentChar_;   //当前读取的字符
        State               state_;         //当前识别的状态
        Token               token_;         //当前读取的单词，待填充
        Dictionary          dictionary_;    //参考字典
        std::string         buffer_;        //缓冲区
        static bool         errorFlag_;     //标识单词识别时是否出错
        static bool         isFileAvailable_; //标识源文件是否可用
    };

    inline Token Scanner::getToken() const
    {
        return token_;
    }

    inline bool Scanner::getErrorFlag()
    {
        return errorFlag_;
    }

    inline TokenLocation Scanner::getTokenLocation() const
    {
        return TokenLocation(fileName_, line_, column_);
    }

    inline bool Scanner::getFileAvailable()
    {
        return isFileAvailable_;
    }
}

#endif