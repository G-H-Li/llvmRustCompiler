/*
* Author: zqf
* Date:2021/1/2
* description:Scanner函数成员的实现
* latest date:2021/1/9
*/

#include <algorithm>
#include <cctype>
#include "scanner.h"
#include "../Error/error.h"


namespace llvmRustCompiler
{
    bool Scanner::errorFlag_ = false;
    bool Scanner::isFileAvailable_ = true;

    Scanner::Scanner(const std::string& srcFileName)
        : fileName_(srcFileName), line_(1), column_(0),
        currentChar_(0), state_(State::NONE)
    {
        input_.open(fileName_);

        if (input_.fail())
        {
            errorReport("打开文件" + fileName_ + "时出错");
            isFileAvailable_ = false;
            errorFlag_ = true;
        }
    }

    //从输入流获取下一个字符
    void Scanner::getNextChar()
    {
        currentChar_ = input_.get();

        if (currentChar_ == '\n')
        {
            line_++;
            column_ = 0;
        }
        else
        {
            column_++;
        }
    }

    char Scanner::peekChar()
    {
        char c = input_.peek();
        return c;
    }

    void Scanner::addToBuffer(char c)
    {
        buffer_.push_back(c);
    }

    void Scanner::reduceBuffer()
    {
        buffer_.pop_back();
    }

    void Scanner::makeToken(TokenType tt, TokenValue tv,
        const TokenLocation& loc, std::string name, int symbolPrecedence)
    {
        token_ = Token(tt, tv, loc, name, symbolPrecedence);
        buffer_.clear();
        state_ = State::NONE;
    }

    void Scanner::makeToken(TokenType tt, TokenValue tv,
        const TokenLocation& loc, long intValue, std::string name)
    {
        token_ = Token(tt, tv, loc, intValue, name);
        buffer_.clear();
        state_ = State::NONE;
    }

    void Scanner::makeToken(TokenType tt, TokenValue tv,
        const TokenLocation& loc, float floatValue, std::string name)
    {
        token_ = Token(tt, tv, loc, floatValue, name);
        buffer_.clear();
        state_ = State::NONE;
    }

    void Scanner::makeToken(TokenType tt, TokenValue tv,
        const TokenLocation& loc, const std::string& strValue, std::string name)
    {
        token_ = Token(tt, tv, loc, strValue, name);
        buffer_.clear();
        state_ = State::NONE;
    }

    void Scanner::preprocess()
    {
        do
        {
            //跳过所有空格
            while (std::isspace(currentChar_))
            {
                getNextChar();
            }
            //跳过所有注释
            handleComment();
        } while (std::isspace(currentChar_));
    }

    //RUST中的注释:
    //    // 这是第一种注释方式
    //    /* 这是第二种注释方式 */
    //    /*
    //    * 这是第三种注释方式
    //    * 这是第三种注释方式
    //    * 这是第三种注释方式
    //    * /
    //    第三种注释可与第二种合并
    void Scanner::handleComment()
    {
        loc_ = getTokenLocation();

        if (currentChar_ == '/' && peekChar() == '/')
        {
            std::string lineComment = "";
            getline(input_, lineComment);//好像没法判断换行符/n只能暂时先这样了，直接吞一行
            getNextChar();
        }

        if (currentChar_ == '/' && peekChar() == '*')
        {
            getNextChar();
            getNextChar();
            while (!(currentChar_ == '*' && peekChar() == '/'))
            {
                getNextChar();
                //缺少 *)
                if (input_.eof())
                {
                    errorReport("未读取到行注释结尾的*/");
                    return;
                }
            }
            if (!input_.eof())
            {
                getNextChar();
                getNextChar();//eat ‘*/’
            }
        }
    }

    Token Scanner::getNextToken()
    {
        bool matched = errorFlag_ = false;

        do
        {
            if (state_ != State::NONE)
            {
                matched = true;
            }

            switch (state_)
            {
            case State::NONE:
                getNextChar();
                break;

            case State::END_OF_FILE:
                handleEOFState();
                break;

            case State::IDENTIFIER:
                handleIdentifierState();
                break;

            case State::NUMBER:
                handleNumberState();
                break;

            case State::CHAR:
                handleCharState();
                break;

            case State::STRING:
                handleStringState();
                break;

            case State::OPERATION:
                handleOperationState();
                break;

            default:
                errorReport("匹配状态出错");
                break;
            }

            if (state_ == State::NONE)
            {
                preprocess();

                if (input_.eof())
                {
                    state_ = State::END_OF_FILE;
                }
                else
                {
                    //标识符
                    if (std::isalpha(currentChar_) || (currentChar_ == '_'))
                    {
                        state_ = State::IDENTIFIER;
                    }
                    //数字
                    else if (std::isdigit(currentChar_))
                    {
                        state_ = State::NUMBER;
                    }
                    else if (currentChar_ == '\'')
                    {
                        state_ = State::CHAR;
                    }
                    //字符串
                    else if (currentChar_ == '\"')
                    {
                        state_ = State::STRING;
                    }
                    else
                    {
                        state_ = State::OPERATION;
                    }
                }
            }
        } while (!matched);

        return token_;
    }

    void Scanner::handleEOFState()
    {
        loc_ = getTokenLocation();
        makeToken(TokenType::tok_eof, TokenValue::KW_UNRESERVED,
            loc_, std::string("END_OF_FILE"), -1);
        input_.close();
        isFileAvailable_ = false;
    }

    /*处理以数字开头
    *一个./e/E        浮点数
    *连续的两个.  数字 + ..
    * 其他情况的两个及以上.  非法标识符
    *两个及以上e/E  非法标识符
    * 10进制出现字母  非法标识符
    */
    void Scanner::handleNumberState()
    {
        loc_ = getTokenLocation();
        bool isFloat = false;
        bool isExponent = false;
        int numberBase = 10;//进制
        enum class NumberState
        {
            INTERGER,//整数
            FRACTION,//浮点数 小数部分
            EXPONENT,//指数 指数部分
            DONE,   //数字分析完成
            WRONG   //数字分析错误
        };
        NumberState numberState = NumberState::INTERGER;

        if (currentChar_ == '0')//以0开头的情况
        {
            /*
            *考虑16进制,不考虑是科学计数法
            */
            addToBuffer(currentChar_);//eat 0
            getNextChar();
            if (currentChar_ == 'x') {
                numberBase = 16;
                addToBuffer(currentChar_);
                getNextChar();
            }
            else if (std::isdigit(currentChar_)) {
                numberState = NumberState::INTERGER;
            }
            else if (currentChar_ == '.') {
                //可以是小数，不能是科学计数法的小数部分
                if (peekChar() == '.') {
                    makeToken(TokenType::tok_integer, TokenValue::KW_UNRESERVED, loc_,
                        std::stol(buffer_, 0, 10), buffer_);
                    return;
                }
                numberState = NumberState::FRACTION;
            }
            else {
                numberState = NumberState::WRONG;
            }
        }

        do
        {
            switch (numberState)
            {
            case NumberState::INTERGER:
                if (numberBase == 10)
                {
                    handleDigit();
                }
                else if (numberBase == 16)
                {
                    handleXDigit();
                }
                break;

            case NumberState::FRACTION:
                handleFraction();
                isFloat = true;
                break;

            case NumberState::EXPONENT:
                handleExponent();
                isExponent = true;
                break;

            case NumberState::DONE:
                break;

            case NumberState::WRONG://后面单独处理
                break;

            default:
                errorReport("Match number state error.");
                break;
            }

            if (currentChar_ == '.')
            {

                if (isFloat)
                {
                    errorReport(buffer_ + "已经是浮点数了，没必要再来一个 .");
                }

                if (isExponent)
                {
                    errorReport(buffer_ + "指数部分不能有 .");
                }

                if (numberBase == 16)
                {
                    errorReport("16进制数不能有 .");
                }
                //除此之外，即处理该数字时仅有整数部分
                if (peekChar() == '.') {
                    //又出现了一个. 考虑..，不在数字范畴
                    numberState = NumberState::DONE;
                    break;
                }
                numberState = NumberState::FRACTION;
            }
            else if (currentChar_ == 'E' || currentChar_ == 'e')
            {
                if (isExponent)
                {
                    errorReport(buffer_ + "已经是指数了，不能有更多e/E");
                }
                numberState = NumberState::EXPONENT;
            }
            else if (std::isalpha(currentChar_))
            {
                numberState = NumberState::WRONG;
                break;
            }
            else
            {
                numberState = NumberState::DONE;
            }

        } while (numberState != NumberState::DONE);

        if (numberState == NumberState::WRONG)
        {
            handleWrongState();
            makeToken(TokenType::tok_unknown, TokenValue::KW_UNRESERVED, loc_,
                buffer_, buffer_);
        }
        else if (errorFlag_) {
            makeToken(TokenType::tok_unknown, TokenValue::KW_UNRESERVED, loc_,
                buffer_, buffer_);
        }
        else
        {
            if (isFloat || isExponent)
            {
                makeToken(TokenType::tok_float, TokenValue::KW_UNRESERVED, loc_,
                    std::stof(buffer_), buffer_);
            }
            else
            {
                makeToken(TokenType::tok_integer, TokenValue::KW_UNRESERVED, loc_,
                    std::stol(buffer_, 0, 0), buffer_);//进制参数用0时将自动根据基数转化
            }
        }

    }

    void Scanner::handleCharState()
    {
        loc_ = getTokenLocation();
        getNextChar();

        while (true)
        {
            if (currentChar_ == '\'')
            {
                if (peekChar() == '\'')
                {
                    getNextChar();
                }
                else
                {
                    break;
                }
            }
            if (input_.eof()) {
                errorReport("未读取到字符或字符串结尾");
                break;
            }
            addToBuffer(currentChar_);
            getNextChar();
        }

        getNextChar();
        if (buffer_.length() == 1)
        {
            makeToken(TokenType::tok_char, TokenValue::KW_UNRESERVED, loc_,
                static_cast<long>(buffer_.at(0)), buffer_);
        }
        else
        {
            makeToken(TokenType::tok_string, TokenValue::KW_UNRESERVED,
                loc_, buffer_, buffer_);
        }
    }

    void Scanner::handleStringState()
    {
        loc_ = getTokenLocation();
        getNextChar();

        while (true)
        {
            if (currentChar_ == '\"')
            {
                if (peekChar() == '\"')
                {
                    getNextChar();
                }
                else
                {
                    break;
                }
            }
            if (input_.eof()) {
                errorReport("未读取到字符串结尾");
                break;
            }
            addToBuffer(currentChar_);
            getNextChar();
        }

        getNextChar();
        makeToken(TokenType::tok_string, TokenValue::KW_UNRESERVED,
            loc_, buffer_, buffer_);
    }

    void Scanner::handleIdentifierState()
    {
        loc_ = getTokenLocation();
        //标识符以 _ 开头时后面必须跟字母
        if (currentChar_ == '_') {
            addToBuffer(currentChar_);
            getNextChar();
            if (!std::isalpha(currentChar_)) {
                handleWrongState();
                makeToken(TokenType::tok_unknown, TokenValue::KW_UNRESERVED, loc_,
                    buffer_, buffer_);
                return;
            }
        }
        addToBuffer(currentChar_);
        getNextChar();
        //标识符接受字母、数字和 _
        while (std::isalnum(currentChar_) || currentChar_ == '_')
        {
            addToBuffer(currentChar_);
            getNextChar();
        }

        std::transform(buffer_.begin(), buffer_.end(), buffer_.begin(), ::tolower);
        //检查是否是关键字
        auto tokenMeta = dictionary_.lookup(buffer_);
        makeToken(std::get<0>(tokenMeta), std::get<1>(tokenMeta), loc_, buffer_, std::get<2>(tokenMeta));
    }

    void Scanner::handleOperationState()
    {
        loc_ = getTokenLocation();
        addToBuffer(currentChar_);
        addToBuffer(peekChar());
        if (dictionary_.haveToken(buffer_))
        {
            getNextChar();
        }
        else
        {
            reduceBuffer();
        }
        //匹配符号
        auto tokenMeta = dictionary_.lookup(buffer_);
        makeToken(std::get<0>(tokenMeta), std::get<1>(tokenMeta), loc_, buffer_, std::get<2>(tokenMeta));
        getNextChar();
    }

    void Scanner::handleDigit()
    {
        addToBuffer(currentChar_);
        getNextChar();

        while (std::isdigit(currentChar_))
        {
            addToBuffer(currentChar_);
            getNextChar();
        }
    }

    void Scanner::handleXDigit()
    {
        bool readFlag = false;

        while (std::isxdigit(currentChar_))
        {
            readFlag = true;
            addToBuffer(currentChar_);
            getNextChar();
        }

        if (!readFlag)
        {
            errorReport("16进制格式出错");
        }
    }

    void Scanner::handleFraction()
    {

        addToBuffer(currentChar_);//eat .
        getNextChar();

        while (std::isdigit(currentChar_))
        {
            addToBuffer(currentChar_);
            getNextChar();
        }
    }

    void Scanner::handleExponent()
    {
        addToBuffer(currentChar_);
        getNextChar();

        if (currentChar_ != '+' && currentChar_ != '-' && !std::isdigit(currentChar_))
        {
            errorReport("指数部分只能是数字");
        }

        if (currentChar_ == '+' || currentChar_ == '-')
        {
            addToBuffer(currentChar_);
            getNextChar();
        }

        while (std::isdigit(currentChar_))
        {
            addToBuffer(currentChar_);
            getNextChar();
        }
    }

    void Scanner::handleWrongState()
    {
        errorReport("非法的标识符或数字:");
        do
        {
            addToBuffer(currentChar_);
            getNextChar();
        } while (!std::isspace(currentChar_));
    }

    void Scanner::errorReport(const std::string& msg)
    {
        errorToken(getTokenLocation().toString() + msg);
    }

    void Scanner::setErrorFlag(bool flag)
    {
        errorFlag_ = flag;
    }

    void Scanner::setFileAvailable(bool flag)
    {
        isFileAvailable_ = flag;
    }
}