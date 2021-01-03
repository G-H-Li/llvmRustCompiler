#pragma once
/*
* Author: �����
* Date:2021/1/3
* description:����error��غ���
* latest date:2021/1/3
*/

#include <string>
#include "..\Lexer\token.h"

namespace llvmRustCompiler
{
	// ʹ���ַ������д���ʷ��������
	extern void errorToken(const std::string& msg);
	// ʹ��token���д���ʷ��������
	extern void errorToken(const Token& token, const std::string& msg);
	//ʹ���ַ������д����﷨�������
	extern void errorSyntax(const std::string& msg);
	
	//ʹ���ַ������д������ɽ׶δ������
	extern void errorGenerator(const std::string& msg);

}