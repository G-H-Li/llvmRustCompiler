#/*
* Author: zqf
* Date:2021/1/2
* description:Scannerɨ��������������,ÿ�δ���һ������
* latest date:2021/1/3
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
        //ʹ��getToken()ֱ�ӻ�ȡ���������ʣ�����ɨ��Դ����
        Token           getToken() const;
        //ʹ��getNextToken()��ɨ��Դ����,���Զ�ȡ��һ������
        Token           getNextToken();
        //ʹ��getErrorFlag()����ȡlexer�ı�����Ϣ��ΪtrueʱӦֹͣgetNextToken()
        static bool     getErrorFlag();
        static void     setErrorFlag(bool flag);

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

        //��ȡ��������
        void            preprocess();
        void            handleLineComment();
        void            handleBlockComment();

        TokenLocation   getTokenLocation() const;

        void            handleEOFState();
        void            handleIdentifierState();
        void            handleNumberState();
        void            handleStringState();
        void            handleOperationState();

        void            handleDigit();
        void            handleXDigit();//����16��������
        void            handleFraction();
        void            handleExponent();
        void            errorReport(const std::string& msg);

    public:
        enum class State
        {
            NONE,
            END_OF_FILE,
            IDENTIFIER,
            NUMBER,
            STRING,
            OPERATION
        };

    private:
        std::string         fileName_;
        std::ifstream       input_;
        long                line_;          //��ǰ��ȡ���ַ��к�
        long                column_;        //��ǰ��ȡ���ַ��к�
        TokenLocation       loc_;
        char                currentChar_;   //��ǰ��ȡ���ַ�
        State               state_;         //��ǰʶ���״̬
        Token               token_;         //��ǰ��ȡ�ĵ��ʣ������
        Dictionary          dictionary_;    //�ο��ֵ�
        std::string         buffer_;        //������
        static bool         errorFlag_;

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
}

#endif