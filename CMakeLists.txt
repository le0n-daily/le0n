cmake_minimum_required(VERSION 3.0)
project(le0n)

# 显式指定使用 GCC/G++
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# 开启详细构建输出
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(${CMAKE_SOURCE_DIR})

set(LIB_SRC
    le0n/log.cc
    le0n/util.cc
    le0n/config.cc
)

add_library(le0n SHARED ${LIB_SRC})

# 建议: 尽量不要将可执行文件命名为 "test"，因为 "make test" 是 CMake 的保留命令，容易冲突。我改成了 "le0n_test"
add_executable(test_config tests/test_config.cc)
add_dependencies(test_config le0n)

# 链接库
target_link_libraries(test_config le0n)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)