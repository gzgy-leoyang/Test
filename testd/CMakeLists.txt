cmake_minimum_required (VERSION 3.10)

#project( Hi_pretty VERSION 1.0.0 DESCRIPTION "Hi,pretty")
project( yqsrv_testd )

# 规范C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

# Wall 打开全部检查(实际不是全部)
# Wextra,打开另外一部分检查,不一定就是wall没有打开的
# pedantic,以ANSI/ISO C标准列出的所有警告
# Werror,以error标记警告,任何警告都可以中止编译
#-Wvariadic-macros
# -std=c99 支持变宏参数
# -Wno-variadic-macros 禁止对可变宏参数的报警
add_compile_options(-Wall -Wextra -pedantic -Wno-variadic-macros )

## 是否安装ccache, 通过缓存方式加速编译
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  message(STATUS "Set up ccache ...")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
else()
  message(WARNING "ccache has not been found")
endif()

## 在生成Makefile过程中,输出编译配置相关信息
message(STATUS "======================================================")
message(STATUS "编译工具: ${CMAKE_MAKE_PROGRAM}")
message(STATUS "源码目录: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "工具描述: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "目标平台: ${PLATFORM_TYPE}")
message(STATUS "目标芯片: ${PLATFORM_CHIP_TYPE}")
message(STATUS "编译类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "输出目录: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "安装位置: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "库输出  : ${BUILD_SHARED_LIBS}")
message(STATUS "第三方库: ${THIRD_PARTY_PREFIX}")
message(STATUS "======================================================")

## 编译库实现代码
include_directories(${PROJECT_SOURCE_DIR}/inc)
include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)
## 编译测试代码
#add_subdirectory(test)
## 创建库配置文件
#include(cmake/create-config.cmake)
