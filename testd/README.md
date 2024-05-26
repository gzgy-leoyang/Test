# Hi Pretty 项目

## 项目说明

这是一个 helloworld 型的项目,可作为空白工程扩展实际项目.

## cppcheck 检查配置

```sh
$ cppcheck src --enable=all \
               --suppress=missingIncludeSystem \
               --suppress=unusedFunction \
               --std=c++11 \
               --platform=unix64 \
               -I inc/ include/

```

**以下内容建议保留**  

## 编译 

项目通过 "build.sh + cmake + Unix Makefile/Ninja" 组合实现多平台自动构建.

指定目标平台并启动编译构建过程,如下,-p参数(platform)用于指定目标平台

```sh
$ ./build.sh -p=rk3568  /*或执行 ./build.sh -p=imx6 */
 > 构建项目: /home/dd/Works/RK3568/yq_service/service-template
 > 目标平台: rk3568
 > 环境配置: build.conf/rk3568/rk3568.env
 > 工具配置: build.conf/rk3568/rk3568.cmake
 >>进入预处理
 >>预处理完成
 > 准备更新 Makefile
-- The C compiler identification is GNU 11.3.0
-- The CXX compiler identification is GNU 11.3.0
-- Set up ccache ...
-- ======================================================
-- 编译工具: /home/dd/Works/RK3568/sz-yq/aarch64-buildroot-linux-gnu_sdk-buildroot/bin/ninja
-- 源码目录: /home/dd/Works/RK3568/yq_service/service-template
-- 工具描述: /home/dd/Works/RK3568/yq_service/service-template/build.conf/rk3568/rk3568.cmake
-- 目标平台: rockchip
-- 目标芯片: rk3568
-- 编译类型: Release
-- 输出目录: /tmp/home/dd/Works/RK3568/yq_service/service-template/build/rockchip/rk3568/Release
-- 安装位置: /home/dd/Works/RK3568/yq_service/service-template/out
-- 库输出  : OFF
-- 第三方库: 
-- ======================================================
-- Configuring done (0.2s)
-- Generating done (0.0s)
-- Build files have been written to: /tmp/home/dd/Works/RK3568/yq_service/service-template/build/rockchip/rk3568/Release
 > Makefile 更新完成
 > 执行构建
[5/5] Linking CXX executable src/Hi_pretty
 > 安装部署
[0/1] Install the project...
-- Install configuration: "Release"
-- Installing: /home/dd/Works/RK3568/yq_service/service-template/out/bin/Hi_pretty
-- Set runtime path of "/home/dd/Works/RK3568/yq_service/service-template/out/bin/Hi_pretty" to ""
-- Installing: /home/dd/Works/RK3568/yq_service/service-template/out/lib/libpretty.so
-- Installing: /home/dd/Works/RK3568/yq_service/service-template/out/include/pretty.h
 >>进入后处理...
/home/dd/Works/RK3568/yq_service/service-template/out/bi...le pushed, 0 skipped. 181.9 MB/s (44464 bytes in 0.000s)
/home/dd/Works/RK3568/yq_service/service-template/out/li...file pushed, 0 skipped. 56.7 MB/s (8848 bytes in 0.000s)
 >>后处理完成
 > 编译和安装完成
```

编译完成可在 out/ 目录下获得编译结果,如下,将以下文件部署到目标设备即可.

```sh
.
├── imx6
│  ├── bin
│  │  └── Hi_pretty
│  ├── include
│  │  └── pretty.h
│  └── lib
│     └── libpretty.so
└── rk3568
   ├── bin
   │  └── Hi_pretty
   ├── include
   │  └── pretty.h
   └── lib
      └── libpretty.so

$ file imx6/bin/Hi_pretty 
imx6/bin/Hi_pretty: ELF 32-bit LSB executable, ARM, EABI5 version 1 (SYSV), dynamically linked, interpreter /lib/ld-linux-armhf.so.3, for GNU/Linux 3.1.10, BuildID[sha1]=50fa3b2465de4c1ca5710dff88e25307168a0e97, not stripped

$ file rk3568/bin/Hi_pretty 
rk3568/bin/Hi_pretty: ELF 64-bit LSB shared object, ARM aarch64, version 1 (GNU/Linux), dynamically linked, interpreter /lib/ld-linux-aarch64.so.1, for GNU/Linux 3.7.0, not stripped
```

在必要时可通过 -c 参数(clean)清除之前构建产生的过程文件.

```sh
# service-template
$ ./build.sh -c       
 > 构建项目: /home/dd/Works/RK3568/yq_service/service-template
 > 正在清理: /tmp/home/dd/Works/RK3568/yq_service/service-template/build
 > 正在清理: /home/dd/Works/RK3568/yq_service/service-template/out
 > 完成,退出...
```

另:可通过 help 参数查看使用帮助

```sh
$ ./build.sh help
 用法: ./build.sh [option]
 Option:
   platform | -p=xxx       : 指定目标平台[imx6,rk3568...]
   clean | -c              : 清理过程和输出文件
   toolchain | -t=<*.cmake>: 单独指定工具链描述文件
   notmp | -nt             : 编译过程文件不写入/tmp
   help | -h               : 使用说明
```

---

**注意:以下配置内容由项目维护者修改,项目使用者不需要对其中内容做修改**

### 项目结构和跨平台配置

进入项目根目录下,可见如下结构:

```sh
# service-template
 .
 ├── build.conf/      /* 集中多个平台的编译配置文件*/
 │  ├── imx6/        /* 具体平台配置文件夹*/
 │  └── rk3568/
 ├── build.sh         /*** 执行编译过程 ***/
 ├── CMakeLists.txt   /* 第一层cmake描述*/
 ├── doc/              /* 项目文档,包括设计,变更记录等*/
 ├── inc/              /* 项目内头文件目录,对齐src/路径结构*/
 │  ├── main.h
 │  ├── sub_lib/
 │  └── sub_src/
 ├── include          /* 外部项目头文件*/
 │  └── plog/         /* 默认使用 plog 作为日志支持*/
 ├── lib/              /* 外部项目库*/
 ├── out/              /* 项目编译输出,包括二进制文件,库和头文件*/
 │  ├── bin/          /* 运行于目标系统的二进制ELF文件*/
 │  ├── include/      /* 由sub_lib生成的库的头文件*/
 │  └── lib/          /* 由sub_lib生成的库文件*/
 ├── README.md          /* 项目说明*/
 ├── src/               /* 项目C/C++文件*/
 │  ├── CMakeLists.txt /* 第二层cmake描述*/
 │  ├── main.cpp
 │  ├── sub_lib/       /* 生成库的源文件,需要包含第三场cmake描述*/
 │  └── sub_src/       /* 直接使用源文件,不需再嵌套cmake描述*/
 └── test               /* 测试相关*/
    └── CMakeLists.txt  /* 第二层cmake描述 */
```

对项目维护者来说,跨平台编译是基于多个配置文件和工具实现,其中主要是对cmake的配置,包含对以下两个部分的修改:

1. 配置 *.env

```sh
# 指定用于对应平台的cmake工具链描述文件(备用)
USER_TOOLCHAIN_FILE="/home/dd/Works/RK3568/yq_service/cmake_configs/yq-3568-aarch64.cmake"
#平台类型
USER_PLATFORM_TYPE="rockchip"                      
#平台芯片类型
USER_PLATFORM_CHIP_TYPE="rk3568"                   
# 编译类型,分为 Release/Debug 等
USER_BUILD_TYPE="Release"
# 共享库类型
USER_BUILD_SHARED_LIBS="OFF"
# 指定编译系统,可选 Unix Makefiles 或是 Ninja
USER_CMAKE_GENERATOR="Ninja"
#USER_CMAKE_GENERATOR="Unix Makefiles"
```

2. 配置 *.cmake

在调用cmake时可以通过复杂的输入参数实现各种组合功能,但如果都采用命令行参数方式会导致太长的参数列表,不利于修改和阅读.基于此可通过 *.cmake 文件进行配置,如下:

**在编译时,首先搜索 build.conf/platform/x.cmake 文件,若不存在则按照 x.env 文件中的USER_TOOLCHAIN_FILE参数设置**

```sh
set(CMAKE_SYSTEM_NAME Linux)
# 指定编译目标架构
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CROSS_COMPILATION_ARCHITECTURE armv8)

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# 工具链前缀,如果$PATH没有加入合适的路径,则需要指定工具链绝对地址
set(CROSS_PREFIX aarch64-linux-)

set(CMAKE_C_COMPILER ${CROSS_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_PREFIX}g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
```
> **重要说明:若当前终端未导入的对应平台的,与编译器相关的环境变量,就需要针对修改 CROSS_PREFIX 指向实际位置**

### 添加新源码

项目中添加新的源文件可以采用两种方式:

- 直接使用源码方式,该模块代码不需要以库方式导出的时可以采用这种类型

- 编译为库进行调用的方式,本项目中通过调用库接口实现功能,同时该库也可以被其他项目使用.

#### 采用c/cpp和h文件

在 src/ 和 inc/ 下为新的代码模块建立对应目录,分别管理C/C++文件和h文件.
另外需要修改 src/CMakeLists.txt 文件,在其中如下添加,将该目录下所有源文件暴露到 "全局的_SRC_FILES" 列表.

```sh
# 方式2: 子目录源码直接使用
# 添加源文件夹,将指定文件夹下源文件全部加入 _SRC_FILES 变量
aux_source_directory(./sub_src _SRC_FILES)
```

#### 采用so/a 和 h 文件

若某个模块的功能可以在其他项目中被复用,则可以将其编译为so/a库,结合h文件使用.
对需要编译为库的模块,同样在 src/new_module 和 inc/new_module 中为新代码模块建立目录,并在库源文件目录下建立第三层 CMakeLists.txt描述.
需要修改两个部分:

1. 第二层 src/CMakeLsits.txt,添加需要以库方式编译的模块目录

```sh
# 添加库文件夹,嵌套下一层包含cmakelists文件的文件夹
add_subdirectory(sub_lib)
```

2. 第三层 src/new_module/CMakeLsits.txt,说明编译为库的源文件,输出库名,安装位置等

```sh
# 修改1: 添加需要导出安装的配合so文件的h文件
set(INSTALL_HEAD_FILES 模块对应头文件名 )

# 修改2: 添加需要编译为库的源文件
aux_source_directory(. _SUB_SRC_FILES)
# 修改3: 指定这些源文件编译输出库
add_library ( 库名称 SHARED ${_SUB_SRC_FILES})
#
# 修改4::被安装的库文件,自动添加 lib 前缀
install(
  TARGETS 库名称 
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# 修改5: 输出库h文件
install(
  FILES ${PROJECT_SOURCE_DIR}/inc/path/to/headfile.h
  DESTINATION include )
```
