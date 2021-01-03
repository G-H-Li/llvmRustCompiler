#include "error.h"

void llvmRustCompiler::errorToken(const std::string& msg)
{
	std::cerr << "Token Error:" << msg << std::endl;
	Scanner::setErrorFlag(true);
}

void llvmRustCompiler::errorToken(const Token& token, const std::string& msg)
{
}

void llvmRustCompiler::errorSyntax(const std::string& msg)
{
	std::cerr << "Syntax Error:" << msg << std::endl;
}

void llvmRustCompiler::errorGenerator(const std::string& msg)
{
	std::cerr << "Generator Error:" << msg << std::endl;
}

