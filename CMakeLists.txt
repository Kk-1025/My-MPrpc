# 设置cmake的最低版本、项目名称
cmake_minimum_required(VERSION 3.0)
project(mprpc)

# 生成debug版本，可以进行gdb调试
set(CMAKE_BUILD_TYPE "Debug")

# 设置 项目可执行文件 输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置 项目库文件 输出的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置 头文件的搜索路径 -I
# CMake会根据指定的目录自动搜索头文件，源文件include时无需表现出目录的层级关系
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)

# 设置 库文件的搜索路径 -L
link_directories(${PROJECT_SOURCE_DIR}/lib)

# 进入子目录下执行 CMakeLists.txt文件
# src包含了mprpc框架所有的相关代码
add_subdirectory(src)
# example包含了mprpc框架使用的示例代码
add_subdirectory(example)