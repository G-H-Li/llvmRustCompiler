# CMakeList.txt: llvmRustCompiler 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("llvmRustCompiler")

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 添加子目录
add_subdirectory(Lexer)

# 指定生成目标
add_executable(llvmRustCompiler ${DIR_SRCS} "scanner.h" "scanner.cpp")

# 添加链接库
target_link_libraries(llvmRustCompiler Lexer)
target_link_libraries(llvmRustCompiler Parser)
target_link_libraries(llvmRustCompiler Generator)


# TODO: 如有需要，请添加测试并安装目标。
