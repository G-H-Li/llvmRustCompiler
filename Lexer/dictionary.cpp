/*
* Author: zqf
* Date:2021/1/2
* description:定义符号、关键字等，根据RUST词法
* latest date:2021/1/3
*/

#include "dictionary.h"

namespace llvmRustCompiler
{
    Dictionary::Dictionary()
    {
        addToken("(", std::make_tuple(TokenValue::LEFT_PAREN, TokenType::tok_delimiter, -1));
        addToken(")", std::make_tuple(TokenValue::RIGHT_PAREN, TokenType::tok_delimiter, -1));
        addToken("{", std::make_tuple(TokenValue::LEFT_BRACE, TokenType::tok_delimiter, -1));
        addToken("}", std::make_tuple(TokenValue::RIGHT_BRACE, TokenType::tok_delimiter, -1));

        addToken("+", std::make_tuple(TokenValue::PLUS, TokenType::tok_operators, 10));
        addToken("-", std::make_tuple(TokenValue::MINUS, TokenType::tok_operators, 10));
        addToken("*", std::make_tuple(TokenValue::MULTIPLY, TokenType::tok_operators, 20));
        addToken("/", std::make_tuple(TokenValue::DIVIDE, TokenType::tok_operators, 20));

        addToken(",", std::make_tuple(TokenValue::COMMA, TokenType::tok_delimiter, -1));
        addToken(".", std::make_tuple(TokenValue::PERIOD, TokenType::tok_delimiter, -1));
        addToken(":", std::make_tuple(TokenValue::COLON, TokenType::tok_delimiter, -1));
        addToken(";", std::make_tuple(TokenValue::SEMICOLON, TokenType::tok_delimiter, -1));

        addToken("=", std::make_tuple(TokenValue::EQUAL, TokenType::tok_operators, 2));
        addToken(">=", std::make_tuple(TokenValue::GREATER_OR_EQUAL, TokenType::tok_operators, 2));
        addToken(">", std::make_tuple(TokenValue::GREATER_THAN, TokenType::tok_operators, 2));
        addToken("<=", std::make_tuple(TokenValue::LESS_OR_EQUAL, TokenType::tok_operators, 2));
        addToken("<", std::make_tuple(TokenValue::LESS_THAN, TokenType::tok_operators, 2));

        addToken("->", std::make_tuple(TokenValue::POINTER, TokenType::tok_keywords, 2));

        addToken("let", std::make_tuple(TokenValue::KW_LET, TokenType::tok_keywords, 20));
        addToken("fn", std::make_tuple(TokenValue::KW_FN, TokenType::tok_keywords, 20));
        addToken("mut", std::make_tuple(TokenValue::KW_MUT, TokenType::tok_keywords, 20));
        addToken("if", std::make_tuple(TokenValue::KW_IF, TokenType::tok_keywords, 20));
        addToken("else", std::make_tuple(TokenValue::KW_ELSE, TokenType::tok_keywords, 20));
        
    }

    void Dictionary::addToken(std::string name,
        std::tuple<TokenValue, TokenType, int> tokenMeta)
    {
        dictionary_.insert(std::pair<decltype(name), decltype(tokenMeta)>(name, tokenMeta));
    }

    // if we can find it in the dictionary, we change the token type
    std::tuple<TokenType, TokenValue, int> Dictionary::lookup(const std::string& name) const
    {
        TokenValue tokenValue = TokenValue::KW_UNRESERVED;
        TokenType  tokenType = TokenType::tok_identifier;
        int        precedence = -1;
        auto iter = dictionary_.find(name);

        if (iter != dictionary_.end())
        {
            tokenValue = std::get<0>(iter->second);
            tokenType = std::get<1>(iter->second);
            precedence = std::get<2>(iter->second);
        }

        return std::make_tuple(tokenType, tokenValue, precedence);
    }

    bool Dictionary::haveToken(const std::string& name) const
    {
        auto iter = dictionary_.find(name);

        if (iter != dictionary_.end())
        {
            return true;
        }

        return false;
    }
}