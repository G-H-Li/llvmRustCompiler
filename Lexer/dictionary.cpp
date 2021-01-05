/*
* Author: zqf
* Date:2021/1/2
* description:定义符号、关键字等，根据RUST词法
* latest date:2021/1/5
*/

#include "dictionary.h"

namespace llvmRustCompiler
{
    Dictionary::Dictionary()
    {
        addToken("as", std::make_tuple(TokenValue::KW_AS, TokenType::tok_keywords, 20));
        addToken("break", std::make_tuple(TokenValue::KW_BREAK, TokenType::tok_keywords, 20));
        addToken("const", std::make_tuple(TokenValue::KW_CONST, TokenType::tok_keywords, 20));
        addToken("continue", std::make_tuple(TokenValue::KW_CONTINUE, TokenType::tok_keywords, 20));
        addToken("crate", std::make_tuple(TokenValue::KW_CRATE, TokenType::tok_keywords, 20));
        addToken("else", std::make_tuple(TokenValue::KW_ELSE, TokenType::tok_keywords, 20));
        addToken("enum", std::make_tuple(TokenValue::KW_ENUM, TokenType::tok_keywords, 20));
        addToken("extern", std::make_tuple(TokenValue::KW_EXTERN, TokenType::tok_keywords, 20));
        addToken("false", std::make_tuple(TokenValue::KW_FALSE, TokenType::tok_keywords, 20));
        addToken("fn", std::make_tuple(TokenValue::KW_FN, TokenType::tok_keywords, 20));
        addToken("for", std::make_tuple(TokenValue::KW_FOR, TokenType::tok_keywords, 20));
        addToken("if", std::make_tuple(TokenValue::KW_IF, TokenType::tok_keywords, 20));
        addToken("impl", std::make_tuple(TokenValue::KW_IMPL, TokenType::tok_keywords, 20));
        addToken("in", std::make_tuple(TokenValue::KW_IN, TokenType::tok_keywords, 20));
        addToken("let", std::make_tuple(TokenValue::KW_LET, TokenType::tok_keywords, 20));
        addToken("loop", std::make_tuple(TokenValue::KW_LOOP, TokenType::tok_keywords, 20));
        addToken("match", std::make_tuple(TokenValue::KW_MATCH, TokenType::tok_keywords, 20));
        addToken("mod", std::make_tuple(TokenValue::KW_MOD, TokenType::tok_keywords, 20));
        addToken("move", std::make_tuple(TokenValue::KW_MOVE, TokenType::tok_keywords, 20));
        addToken("mut", std::make_tuple(TokenValue::KW_MUT, TokenType::tok_keywords, 20));
        addToken("pub", std::make_tuple(TokenValue::KW_PUB, TokenType::tok_keywords, 20));
        addToken("ref", std::make_tuple(TokenValue::KW_REF, TokenType::tok_keywords, 20));
        addToken("return", std::make_tuple(TokenValue::KW_RETURN, TokenType::tok_keywords, 20));
        addToken("self", std::make_tuple(TokenValue::KW_SELFVALUE, TokenType::tok_keywords, 20));
        addToken("Self", std::make_tuple(TokenValue::KW_SELFTYPE, TokenType::tok_keywords, 20));
        addToken("static", std::make_tuple(TokenValue::KW_STATIC, TokenType::tok_keywords, 20));
        addToken("struct", std::make_tuple(TokenValue::KW_STRUCT, TokenType::tok_keywords, 20));
        addToken("super", std::make_tuple(TokenValue::KW_SUPER, TokenType::tok_keywords, 20));
        addToken("trait", std::make_tuple(TokenValue::KW_TRAIT, TokenType::tok_keywords, 20));
        addToken("true", std::make_tuple(TokenValue::KW_TRUE, TokenType::tok_keywords, 20));
        addToken("type", std::make_tuple(TokenValue::KW_TYPE, TokenType::tok_keywords, 20));
        addToken("unsafe", std::make_tuple(TokenValue::KW_UNSAFE, TokenType::tok_keywords, 20));
        addToken("use", std::make_tuple(TokenValue::KW_USE, TokenType::tok_keywords, 20));
        addToken("where", std::make_tuple(TokenValue::KW_WHERE, TokenType::tok_keywords, 20));
        addToken("while", std::make_tuple(TokenValue::KW_WHILE, TokenType::tok_keywords, 20));

        addToken("->", std::make_tuple(TokenValue::POINTER, TokenType::tok_keywords, 20));

        addToken("i8", std::make_tuple(TokenValue::KW_I8, TokenType::tok_keywords, 20));
        addToken("i16", std::make_tuple(TokenValue::KW_I16, TokenType::tok_keywords, 20));
        addToken("i32", std::make_tuple(TokenValue::KW_I32, TokenType::tok_keywords, 20));
        addToken("i64", std::make_tuple(TokenValue::KW_I64, TokenType::tok_keywords, 20));
        addToken("isize", std::make_tuple(TokenValue::KW_ISIZE, TokenType::tok_keywords, 20));
        addToken("u8", std::make_tuple(TokenValue::KW_U8, TokenType::tok_keywords, 20));
        addToken("u16", std::make_tuple(TokenValue::KW_U16, TokenType::tok_keywords, 20));
        addToken("u32", std::make_tuple(TokenValue::KW_U32, TokenType::tok_keywords, 20));
        addToken("u64", std::make_tuple(TokenValue::KW_U64, TokenType::tok_keywords, 20));
        addToken("usize", std::make_tuple(TokenValue::KW_USIZE, TokenType::tok_keywords, 20));
        addToken("f32", std::make_tuple(TokenValue::KW_F32, TokenType::tok_keywords, 20));
        addToken("f64", std::make_tuple(TokenValue::KW_F64, TokenType::tok_keywords, 20));

        addToken("(", std::make_tuple(TokenValue::LEFT_PAREN, TokenType::tok_delimiter, -1));
        addToken(")", std::make_tuple(TokenValue::RIGHT_PAREN, TokenType::tok_delimiter, -1));
        addToken("{", std::make_tuple(TokenValue::LEFT_BRACE, TokenType::tok_delimiter, -1));
        addToken("}", std::make_tuple(TokenValue::RIGHT_BRACE, TokenType::tok_delimiter, -1));

        addToken(",", std::make_tuple(TokenValue::COMMA, TokenType::tok_delimiter, -1));
        addToken(".", std::make_tuple(TokenValue::PERIOD, TokenType::tok_delimiter, -1));
        addToken(":", std::make_tuple(TokenValue::COLON, TokenType::tok_delimiter, -1));
        addToken(";", std::make_tuple(TokenValue::SEMICOLON, TokenType::tok_delimiter, -1));
        addToken("..", std::make_tuple(TokenValue::TRAVERSE, TokenType::tok_delimiter, -1));

        addToken("+", std::make_tuple(TokenValue::PLUS, TokenType::tok_operators, 10));
        addToken("-", std::make_tuple(TokenValue::MINUS, TokenType::tok_operators, 10));
        addToken("*", std::make_tuple(TokenValue::MULTIPLY, TokenType::tok_operators, 20));
        addToken("/", std::make_tuple(TokenValue::DIVIDE, TokenType::tok_operators, 20));
        addToken("%", std::make_tuple(TokenValue::REMAINDER, TokenType::tok_operators, 20));
        
        addToken("&", std::make_tuple(TokenValue::AND, TokenType::tok_operators, 3));
        addToken("|", std::make_tuple(TokenValue::OR, TokenType::tok_operators, 3));
        addToken("^", std::make_tuple(TokenValue::XOR, TokenType::tok_operators, 3));
        addToken(">>", std::make_tuple(TokenValue::SHR, TokenType::tok_operators, 10));
        addToken("<<", std::make_tuple(TokenValue::SHL, TokenType::tok_operators, 10));

        addToken("&&", std::make_tuple(TokenValue::LOGIC_AND, TokenType::tok_operators, 2));
        addToken("||", std::make_tuple(TokenValue::LOGIC_OR, TokenType::tok_operators, 2));
        addToken("!", std::make_tuple(TokenValue::LOGIC_NOT, TokenType::tok_operators, 30));

        addToken(">=", std::make_tuple(TokenValue::GREATER_OR_EQUAL, TokenType::tok_operators, 5));
        addToken(">", std::make_tuple(TokenValue::GREATER_THAN, TokenType::tok_operators, 5));
        addToken("<=", std::make_tuple(TokenValue::LESS_OR_EQUAL, TokenType::tok_operators, 5));
        addToken("<", std::make_tuple(TokenValue::LESS_THAN, TokenType::tok_operators, 5));
        addToken("=", std::make_tuple(TokenValue::EQUAL, TokenType::tok_operators, 2));
        addToken("+=", std::make_tuple(TokenValue::PLUS_EQUAL, TokenType::tok_operators, 2));
        addToken("-=", std::make_tuple(TokenValue::MINUS_EQUAL, TokenType::tok_operators, 2));

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