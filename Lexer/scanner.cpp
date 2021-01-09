/*
* Author: zqf
* Date:2021/1/2
* description:Scanner������Ա��ʵ��
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
            errorReport("���ļ�" + fileName_ + "ʱ����");
            isFileAvailable_ = false;
            errorFlag_ = true;
        }
    }

    //����������ȡ��һ���ַ�
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
            //�������пո�
            while (std::isspace(currentChar_))
            {
                getNextChar();
            }
            //��������ע��
            handleComment();
        } while (std::isspace(currentChar_));
    }

    //RUST�е�ע��:
    //    // ���ǵ�һ��ע�ͷ�ʽ
    //    /* ���ǵڶ���ע�ͷ�ʽ */
    //    /*
    //    * ���ǵ�����ע�ͷ�ʽ
    //    * ���ǵ�����ע�ͷ�ʽ
    //    * ���ǵ�����ע�ͷ�ʽ
    //    * /
    //    ������ע�Ϳ���ڶ��ֺϲ�
    void Scanner::handleComment()
    {
        loc_ = getTokenLocation();

        if (currentChar_ == '/' && peekChar() == '/')
        {
            std::string lineComment = "";
            getline(input_, lineComment);//����û���жϻ��з�/nֻ����ʱ�������ˣ�ֱ����һ��
            getNextChar();
        }

        if (currentChar_ == '/' && peekChar() == '*')
        {
            getNextChar();
            getNextChar();
            while (!(currentChar_ == '*' && peekChar() == '/'))
            {
                getNextChar();
                //ȱ�� *)
                if (input_.eof())
                {
                    errorReport("δ��ȡ����ע�ͽ�β��*/");
                    return;
                }
            }
            if (!input_.eof())
            {
                getNextChar();
                getNextChar();//eat ��*/��
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
                errorReport("ƥ��״̬����");
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
                    //��ʶ��
                    if (std::isalpha(currentChar_) || (currentChar_ == '_'))
                    {
                        state_ = State::IDENTIFIER;
                    }
                    //����
                    else if (std::isdigit(currentChar_))
                    {
                        state_ = State::NUMBER;
                    }
                    else if (currentChar_ == '\'')
                    {
                        state_ = State::CHAR;
                    }
                    //�ַ���
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

    /*���������ֿ�ͷ
    *һ��./e/E        ������
    *����������.  ���� + ..
    * �������������������.  �Ƿ���ʶ��
    *����������e/E  �Ƿ���ʶ��
    * 10���Ƴ�����ĸ  �Ƿ���ʶ��
    */
    void Scanner::handleNumberState()
    {
        loc_ = getTokenLocation();
        bool isFloat = false;
        bool isExponent = false;
        int numberBase = 10;//����
        enum class NumberState
        {
            INTERGER,//����
            FRACTION,//������ С������
            EXPONENT,//ָ�� ָ������
            DONE,   //���ַ������
            WRONG   //���ַ�������
        };
        NumberState numberState = NumberState::INTERGER;

        if (currentChar_ == '0')//��0��ͷ�����
        {
            /*
            *����16����,�������ǿ�ѧ������
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
                //������С���������ǿ�ѧ��������С������
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

            case NumberState::WRONG://���浥������
                break;

            default:
                errorReport("Match number state error.");
                break;
            }

            if (currentChar_ == '.')
            {

                if (isFloat)
                {
                    errorReport(buffer_ + "�Ѿ��Ǹ������ˣ�û��Ҫ����һ�� .");
                }

                if (isExponent)
                {
                    errorReport(buffer_ + "ָ�����ֲ����� .");
                }

                if (numberBase == 16)
                {
                    errorReport("16������������ .");
                }
                //����֮�⣬�����������ʱ������������
                if (peekChar() == '.') {
                    //�ֳ�����һ��. ����..���������ַ���
                    numberState = NumberState::DONE;
                    break;
                }
                numberState = NumberState::FRACTION;
            }
            else if (currentChar_ == 'E' || currentChar_ == 'e')
            {
                if (isExponent)
                {
                    errorReport(buffer_ + "�Ѿ���ָ���ˣ������и���e/E");
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
                    std::stol(buffer_, 0, 0), buffer_);//���Ʋ�����0ʱ���Զ����ݻ���ת��
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
                errorReport("δ��ȡ���ַ����ַ�����β");
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
                errorReport("δ��ȡ���ַ�����β");
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
        //��ʶ���� _ ��ͷʱ����������ĸ
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
        //��ʶ��������ĸ�����ֺ� _
        while (std::isalnum(currentChar_) || currentChar_ == '_')
        {
            addToBuffer(currentChar_);
            getNextChar();
        }

        std::transform(buffer_.begin(), buffer_.end(), buffer_.begin(), ::tolower);
        //����Ƿ��ǹؼ���
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
        //ƥ�����
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
            errorReport("16���Ƹ�ʽ����");
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
            errorReport("ָ������ֻ��������");
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
        errorReport("�Ƿ��ı�ʶ��������:");
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