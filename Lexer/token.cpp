/*
* Author: zqf
* Date:2021/1/2
* description:token函数成员的实现
* latest date:2021/1/3
*/
#ifndef TOKEN_CPP_
#define TOKEN_CPP_
#include "token.h"

namespace llvmRustCompiler 
{
    TokenLocation::TokenLocation(const std::string& fileName, int line, int column)
        : fileName_(fileName), line_(line), column_(column)
    {}

    TokenLocation::TokenLocation() : fileName_(""), line_(1), column_(0)
    {}

    std::string TokenLocation::toString() const
    {
        return fileName_ + ":" + std::to_string(line_) + ":" + std::to_string(column_) + ":";
    }

    Token::Token() : type_(TokenType::tok_unknown), value_(TokenValue::KW_UNRESERVED),
        location_(std::string(""), 0, 0), name_(""), symbolPrecedence_(-1),floatValue_(0),intValue_(0)
    {}

    Token::Token(TokenType type, TokenValue value, const TokenLocation& location,
        std::string name, int symbolPrecedence)
        : type_(type), value_(value), location_(location), name_(name),
        symbolPrecedence_(symbolPrecedence), floatValue_(0), intValue_(0)
    {}

    Token::Token(TokenType type, TokenValue value, const TokenLocation& location,
        const std::string& strValue, std::string name)
        : type_(type), value_(value), location_(location),
        name_(name), symbolPrecedence_(-1), strValue_(strValue), floatValue_(0), intValue_(0)
    {}

    Token::Token(TokenType type, TokenValue value, const TokenLocation& location,
        long intValue, std::string name)
        : type_(type), value_(value), location_(location),
        name_(name), symbolPrecedence_(-1), intValue_(intValue), floatValue_(0)
    {}

    Token::Token(TokenType type, TokenValue value, const TokenLocation& location,
        float floatValue, std::string name)
        : type_(type), value_(value), location_(location),
        name_(name), symbolPrecedence_(-1), floatValue_(floatValue), intValue_(0)
    {}

    std::string Token::tokenTypeDescription() const
    {
        std::string buffer;

        switch (type_)
        {
        case TokenType::tok_integer:
            buffer = "integer";
            break;

        case TokenType::tok_float:
            buffer = "float";
            break;

        case TokenType::tok_bool:
            buffer = "bool";
            break;

        case TokenType::tok_char:
            buffer = "char";
            break;

        case TokenType::tok_string:
            buffer = "string";
            break;

        case TokenType::tok_identifier:
            buffer = "identifier";
            break;

        case TokenType::tok_keywords:
            buffer = "keywords";
            break;

        case TokenType::tok_operators:
            buffer = "operators";
            break;

        case TokenType::tok_delimiter:
            buffer = "delimiter";
            break;

        case TokenType::tok_eof:
            buffer = "eof";
            break;

        case TokenType::tok_unknown:
            buffer = "unknown";
            break;

        default:
            break;
        }

        return buffer;
    }

    std::string Token::toString() const
    {
        return std::string("Token Type: " + tokenTypeDescription() + "Token Name: " + name_);
    }

    void Token::dump(std::ostream& out /* = std::cout */) const
    {
        out << location_.toString() << "\t" << tokenTypeDescription()
            << "\t" << name_ << "\t\t" << getSymbolPrecedence() << std::endl;
    }
}

#endif