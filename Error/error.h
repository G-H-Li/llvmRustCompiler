#pragma once
/*
* Author: 李国豪
* Date:2021/1/3
* description:定义error相关函数
* latest date:2021/1/3
*/

#include <string>
#include "..\Lexer\token.h"

namespace llvmRustCompiler
{
	// 使用字符串进行错误词法错误输出
	extern void errorToken(const std::string& msg);
	// 使用token进行错误词法错误输出
	extern void errorToken(const Token& token, const std::string& msg);
	//使用字符串进行错误语法错误输出
	extern void errorSyntax(const std::string& msg);
	
	//使用字符串进行代码生成阶段错误输出
	extern void errorGenerator(const std::string& msg);

}