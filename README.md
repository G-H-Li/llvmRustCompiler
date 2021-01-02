## llvmRustCompiler

基于llvm的RUST语言简易编译器

# 项目编译
本项目依赖于llvm项目，若是采用手动编译llvm项目，请在系统中添加相关环境变量：

* Linux：
`export LLVM_HOME=/path/to/your/llvm/installation`


* windows:
step1：高级系统设置->环境变量->系统环境变量。
step2：添加新变量；变量名：LLVM_HOME 变量值：C:\path\to\your\llvm\installation
