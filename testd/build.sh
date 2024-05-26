#!/bin/bash
set -e

# 默认参数:编译中间文件位于/tmp
build_in_tmp="on"
# 默认参数:不清理
build_clean="off"

# Step1:解析传入参数
#命令传参,遍历所有传入的参数
for i in "$@"; do
    case $i in
    platform=* | -p=*)
        target_platform="${i#*=}"
        shift #past argument=value
        ;;
    toolchain=* | -t=* )
        toolchain_file="${i#*=}"
        # 参数移动1位,比如$2变为$1,然后检查下一个参数
        shift #past argument=value
        ;;
    clean | -c )
        # 清理编译过程
        build_clean="on"
        ;;
    notmp | -nt )
        echo " > 编译过程文件写入本地磁盘而非/tmp"
        build_in_tmp="off"
        ;;
    help | -h )
        echo " 用法: ./build.sh [option]"
        echo " Option:"
        echo "   platform | -p=xxx       : 指定目标平台[imx6,rk3568...]"
        echo "   clean | -c              : 清理过程和输出文件"
        echo "   toolchain | -t=<*.cmake>: 单独指定工具链描述文件"
        echo "   notmp | -nt             : 编译过程文件不写入/tmp"
        echo "   help | -h               : 使用说明"
        exit
        ;;
    *)
        echo " >* 警告: 输入参数无效"
        echo " > ./build.sh help 查看使用方法"
        echo " > 退出..."
        exit
        ;;
    esac
done
## 参数解析完成 ##

# Step2 : 准备变量
# 2.1 获取build.sh所处的位置,也就是整个项目的root位置
source_dir="$(cd "$(dirname "${BASH_SOURCE[1]} ")" > /dev/null 2>&1 && pwd)"
echo " > 构建项目: ${source_dir}"

# 2.2 编译过程文件输出路径,默认位于 /tmp  
build_root_dir="$source_dir/build"
if test ${build_in_tmp} = "on"
then
  # 通过notmp 参数指定过程输出当前项目目录(磁盘)
  build_root_dir="/tmp$build_root_dir"
fi

# 2.3 编译输出目录
install_dir="$source_dir/out/${target_platform}"

# 判断是否清理相关文件
if test ${build_clean} = "on"
then
  # 清理编译过程
  if [ -d ${build_root_dir} ];then
    echo " > 正在清理: ${build_root_dir}"
    rm $build_root_dir -rf
    echo " > 正在清理: ${install_dir}"
    rm $install_dir -rf
    echo " > 完成,退出..."
  else 
    echo " > 无需清理,退出..."
  fi
  exit
fi

echo " > 目标平台: ${target_platform}"

# 2.4 配置文件目录
# 不存在则直接退出
target_platform_conf_dir="build.conf/${target_platform}"
if [ ! -d ${target_platform_conf_dir} ]
then
  echo " >* 错误: 缺少 ${target_platform_conf_dir} 目标配置"
  echo " >* 中止,退出..."
  exit
fi

# 2.5 预处理和后处理脚本
pre_build="${target_platform_conf_dir}/pre-build.sh"
post_build="${target_platform_conf_dir}/post-build.sh"

# 2.6 构建环境描述文件
user_env_file="${target_platform_conf_dir}/${target_platform}.env"
if [ ! -f "$user_env_file" ]; then
  echo " >* 错误: 缺少环境描述 $user_env_file"
  echo " >* 中止,退出..."
else
  . ${user_env_file}
fi
echo " > 环境配置: ${user_env_file}"

# 2.7 cmake生成makefile时使用的变量
build_type=${USER_BUILD_TYPE:-"Release"}
build_shared_libs=${USER_BUILD_SHARED_LIBS:-"OFF"}
cmake_generator=${USER_CMAKE_GENERATOR:-"Unix Makefiles"}
platform_type=${USER_PLATFORM_TYPE:-"rockchip"}
platform_chip_type=${USER_PLATFORM_CHIP_TYPE:-"rk3568"}

# 2.8 指定工具链配置文件
# 当默认*.cmake 不存在时,采用*.env中的配置项导入
toolchain_file="${target_platform_conf_dir}/${target_platform}.cmake"
if [ ! -f ${toolchain_file} ]; then
  echo " > 提示: 缺少默认工具配置 ${toolchain_file}"
  echo " > 采用 *.env 配置导入工具链"
  toolchain_file=${USER_TOOLCHAIN_FILE:-"rk3568.toolchain.cmake"}
fi
echo " > 工具配置: ${toolchain_file}"

# Step3: 开始执行构建
# 3.1 准备编译临时目录,只有首次执行
build_dir=$build_root_dir/$platform_type/$platform_chip_type/$build_type
if [ ! -d ${build_dir} ]; then
  mkdir -p "$build_dir" || true
  mkdir -p "$install_dir/include" || true
  mkdir -p "$install_dir/lib" || true
fi

# 3.2 执行预处理脚本
# 在预处理过程中可以使用全部的目录
if [ -f ${pre_build} ]
then
  ./${pre_build}
fi

# 3.3 保存当前路径后,进入新位置
pushd "$build_dir" > /dev/null;
# 3.4 如果编译目录不存在makefile,则新建
if [ ! -f ${build_dir}/Makefile ] && 
   [ ! -f ${build_dir}/build.ninja ]; then
  # 首先生成makefile文件
  echo " > 准备更新 Makefile"
  ## -G <generator-name> :  指定生成器(默认使用:CMAKE_GENERATOR )
  cmake "$source_dir" -G "$cmake_generator" \
      -DCMAKE_INSTALL_PREFIX="$install_dir" \
      -DCMAKE_TOOLCHAIN_FILE="$toolchain_file" \
      -DPLATFORM_TYPE="$platform_type" \
      -DPLATFORM_CHIP_TYPE="$platform_chip_type" \
      -DCMAKE_BUILD_TYPE="$build_type" \
      -DBUILD_SHARED_LIBS="$build_shared_libs"
  echo " > Makefile 更新完成"
fi

# 3.5 执行 make
# --build <dir> :指定编译,必须是第一个参数
# --parallel [<jobs>], -j [<jobs>] 指定使用的并行核心数
echo " > 执行构建"
cmake --build . --parallel
#--target <tgt> : 指定编译目标
echo " > 安装部署"
cmake --build . --target install
# popd,按照已经建立的"路径栈",恢复路径
popd > /dev/null;

# 3.6 执行后处理
if [ -f ${post_build} ]
then
  ./${post_build} ${install_dir}
fi
echo " > 编译和安装完成"

# 全部完成
