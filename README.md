## llvmRustCompiler

基于llvm的RUST语言简易编译器

# 项目编译
本项目依赖于llvm项目，若是采用手动编译llvm项目，请在系统中添加相关环境变量：

* Linux：
`export LLVM_HOME=/path/to/your/llvm/installation`


* windows:

    1. 高级系统设置->环境变量->系统环境变量。
    2. 新建
    
    | 变量名 | 变量值 |
    | ------| :------|
    | `LLVM_HOME` |`C:\path\to\your\llvm\installation`|
