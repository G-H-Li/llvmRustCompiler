/*
* Author: zqf
* Date:2021/1/2
* description:Dictionary���ڶ���ʷ��и����ʵ�����
* latest date:2021/1/3
*/

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <string>
#include <tuple>
#include <map>
#include "token.h"

namespace llvmRustCompiler
{
    class Dictionary
    {
    public:
        Dictionary();
        std::tuple<TokenType, TokenValue, int> lookup(const std::string& name) const;
        bool haveToken(const std::string& name) const;
    private:
        void addToken(std::string name, std::tuple<TokenValue, TokenType, int> tokenMeta);

    private:
        //��������ֵ�����͡����ȼ�
        std::map<std::string, std::tuple<TokenValue, TokenType, int>> dictionary_;
    };
}

#endif 