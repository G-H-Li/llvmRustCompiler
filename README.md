## llvmRustCompiler

基于llvm的RUST语言简易编译器



## 项目编译
本项目依赖于llvm项目，若是采用手动编译llvm项目，请在系统中添加相关环境变量：

* Linux：
`export LLVM_HOME=/path/to/your/llvm/installation`


* windows:

    1. 高级系统设置->环境变量->系统环境变量。
    2. 新建
    
    | 变量名 | 变量值 |
    | ------| :------|
    | `LLVM_HOME` |`C:\path\to\your\llvm\installation`|

### LLVM包依赖
在本项目中主要依赖LLVM以下包，请运行前确认已经对这些进行了编译生成。
* `Support`, `Core`, `MC`, `OrcJIT`, `X86Disassembler`, `X86AsmParser`, `X86CodeGen`, `X86Info`

-------------

## 测试用例运行
测试用例文件：test.rs

本项目采用指令进行运行，主要模板：`{lexer|parser|generator|run} <your rust file path>`
* `<your rust file path>` : test.rs的索引路径
* `lexer rust_file_path`: 指定运行词法分析器
* `parser rust_file_path`: 指定运行语法分析器
* `generator rust_file_path`: 指定运行代码生成器
* `run rust_file_path`: 指定运行解释器

------------
## 支持语法说明

### 表达式解析

> #### 示例
>
> ```rust
> let mut x:f32 =8; x=x+1;(3+4)*y;
> ```
>
> #### 注意
>
> * 允许多条表达式，但必须有分号
>
> ***************

### 函数解析

> ### 示例
>
> ```rust
> fn main(x:i32,y:f32) -> f32 {let mut x = 3;y=5;}
> ```
>
> ### 注意
>
> * 必须有大括号，不能声明无函数体函数
> * 函数体内每条语句必须打分号
>
> *************

### If解析

> #### 示例
>
> ```rust
> if x>4 { x =0;x=x+1;} else {x=1;x=x*78;}
> ```
>
> #### 注意
>
> * 条件 x>4  不能打括号
> * 必须完整包含有 if的双大括号{}, else的双大括号{}
> * 大括号内语句必须分号
>
> ************

### While解析

> #### 示例
>
> ```rust
> for X < 0 {x=x+1;}
> ```
>
> ************

