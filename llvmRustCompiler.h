﻿// llvmRustCompiler.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include "Error/error.h"
#include "Lexer/scanner.h"
#include "Lexer/token.h"
#include "Generator/rustJIT.h"
#include "Parser/ast.h"
#include "Parser/parser.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Support/TargetSelect.h"
#include <iostream>