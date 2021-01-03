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
add_subdirectory(Parser)
add_subdirectory(CodeGenerator)
add_subdirectory(Error)

# 指定生成目标
add_executable(llvmRustCompiler ${DIR_SRCS})

# 判断环境变量
if(NOT DEFINED ENV{LLVM_HOME})
    message(FATAL_ERROR "$LLVM_HOME is not defined")
endif()
if(NOT DEFINED ENV{LLVM_DIR})
    set(ENV{LLVM_DIR} $ENV{LLVM_HOME}/lib/cmake/llvm)
endif()
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# 找到与我们希望使用的LLVM组件相对应的库
llvm_map_components_to_libnames(llvm_libs 
    Support 
    Core
    MC
    irreader)

# 添加链接库
target_link_libraries(llvmRustCompiler PUBLIC
    ${llvm_libs}
    Error
    Lexer
    Parser
    Generator)




# TODO: 如有需要，请添加测试并安装目标。
