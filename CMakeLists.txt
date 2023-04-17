cmake_minimum_required(VERSION 3.10)
project(libroutine)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -g -ggdb -fno-strict-aliasing -O0 -Wall -export-dynamic -Wall -pipe  -D_GNU_SOURCE -D_REENTRANT -fPIC -Wno-deprecated -m64)
set(CMAKE_CXX_FLAGS "-std=c++11 -g -ggdb -O0 -mavx2 -Wall -fPIC -DRTE_MACHINE_CPUFLAG_AVX2 -fstack-protector-all")
# Use c and asm
enable_language(C ASM)
# 设置外部库的目录
# link_directories("/root/libdir/")

# 收集当前目录下的所有文件路径存放到SRC_FILES变量里
aux_source_directory(. SRC_FILES)
set(SRC_FILES ${SRC_FILES} "./rtctx_swap.S")
message(${SRC_FILES})
add_executable(example_cond example_cond.cpp ${SRC_FILES})
target_link_libraries("example_cond" dl)

