/*
* Author: zqf
* Date:2021/1/2
* description:枚举类型、关键字、定义token等
* latest date:2021/1/3
*/

#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>
#include <iostream>
#include <tuple>
#include <map>
#include <cassert>


namespace llvmRustCompiler
{
    //单词的类型
    enum class TokenType
    {
        tok_integer,
        tok_float,
        tok_bool,
        tok_char,
        tok_string,

        tok_identifier, //标识符
        tok_keywords,   //关键字
        tok_operators,  //操作符
        tok_delimiter,  //分隔符
        tok_eof,        //结尾
        tok_unknown
    };

    //关键字、符号等
    enum class TokenValue
    {
        //关键字
        KW_AS,      // as
        KW_BREAK,   // break
        KW_CONST,   // const
        KW_CONTINUE,// continue
        KW_CRATE,   // crate
        KW_ELSE,    // else
        KW_ENUM,    // enum
        KW_EXTERN,  // extern
        KW_FALSE,   // false
        KW_FN,      // fn
        KW_FOR,     // for
        KW_IF,      // if
        KW_IMPL,    // impl
        KW_IN,      // in
        KW_LET,     // let
        KW_LOOP,    // loop
        KW_MATCH,   // match
        KW_MOD,     // mod
        KW_MOVE,    // move
        KW_MUT,     // mut
        KW_PUB,     // pub
        KW_REF,     // ref
        KW_RETURN,  // return
        KW_SELFVALUE,// self
        KW_SELFTYPE,// Self
        KW_STATIC,  // static
        KW_STRUCT,  // struct
        KW_SUPER,   // super
        KW_TRAIT,   // trait
        KW_TRUE,    // true
        KW_TYPE,    // type
        KW_UNSAFE,  // unsafe
        KW_USE,     // use
        KW_WHERE,   // where
        KW_WHILE,   // while
        KW_UNRESERVED,//未使用的关键字

        //符号
        LEFT_PAREN,        // (
        RIGHT_PAREN,       // )
        LEFT_BRACE,       // {
        RIGHT_BRACE,      // }
        PLUS,              // +
        MINUS,             // -
        MULTIPLY,          // *
        DIVIDE,            // /
        COMMA,             // ,
        PERIOD,            // .
        SEMICOLON,         // ;
        COLON,             // :
        LESS_OR_EQUAL,     // <=
        LESS_THAN,         // <
        GREATER_OR_EQUAL,  // >=
        GREATER_THAN,      // >
        EQUAL,             // =
        POINTER            // ->
    };

    //位置信息,用于检索、标错
    class TokenLocation
    {
    public:
        TokenLocation();
        TokenLocation(const std::string& fileName, int line, int column);
        int getLine() const { return this->line_; }
        int getCol() const { return this->column_; }
        std::string toString() const;
    private:
        std::string fileName_;
        int line_;
        int column_;
    };

    class Token
    {
    public:
        Token();
        //符号
        Token(TokenType type, TokenValue value, const TokenLocation& location,
            std::string name, int symbolPrecedence);
        //标识符、关键字、类型名、字符串常量、布尔类型常量等
        Token(TokenType type, TokenValue value, const TokenLocation& location,
            const std::string& strValue, std::string name);
        //整数常量
        Token(TokenType type, TokenValue value, const TokenLocation& location,
            long intValue, std::string name);
        //浮点数常量
        Token(TokenType type, TokenValue value, const TokenLocation& location,
            float floatValue, std::string name);

        TokenType getTokenType() const;
        TokenValue getTokenValue() const;
        const TokenLocation& getTokenLocation() const;
        std::string getTokenName() const;

        //获取优先级
        int getSymbolPrecedence() const;

        //获取具体的值
        long getIntValue() const;
        float getFloatValue() const;
        std::string getStringValue() const;

        void dump(std::ostream& out = std::cout) const;

        std::string getIdentifierName() const;

        std::string tokenTypeDescription() const;
        std::string toString() const;

    private:
        TokenType       type_;
        TokenValue      value_;
        TokenLocation   location_;
        std::string     name_;
        int             symbolPrecedence_;

        //常量值
        long            intValue_;
        float          floatValue_;
        std::string     strValue_;

    };

    inline TokenType Token::getTokenType() const
    {
        return type_;
    }

    inline TokenValue Token::getTokenValue() const
    {
        return value_;
    }

    inline std::string Token::getTokenName() const
    {
        return name_;
    }

    inline const TokenLocation& Token::getTokenLocation() const
    {
        return location_;
    }

    inline long Token::getIntValue() const
    {
        return intValue_;
    }

    inline float Token::getFloatValue() const
    {
        return floatValue_;
    }

    inline std::string Token::getStringValue() const
    {
        return strValue_;
    }

    inline int Token::getSymbolPrecedence() const
    {
        return symbolPrecedence_;
    }

    inline std::string Token::getIdentifierName() const
    {
        assert(type_ == TokenType::tok_identifier && "Token type should be identifier.");
        return name_;
    }
}

#endif