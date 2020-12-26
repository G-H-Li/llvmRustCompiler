#pragma once
/*
* Author: GH-Li
* Date:2020/11/27
* description:定义token相关关键字和数据结构
* latest date:2020/12/26
*/

#include <string>


namespace llvmRustCompiler 
{
	enum class TokenType
	{
		TOK_NULL,
		TOK_EOF,

		TOK_NEWLINE,
		TOK_WHITESPACE,
		TOK_COMMENT,

		//value tokens

		//Symbols

		//reserved words

	};

}
