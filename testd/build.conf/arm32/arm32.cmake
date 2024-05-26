set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

set(CMAKE_FIND_ROOT_PATH  /home/dd/Tools/imx6-SDK/sysroots/armv7at2hf-neon-angstrom-linux-gnueabi)
# 工具链前缀
set(CROSS_PREFIX arm-angstrom-linux-gnueabi-)
set(CMAKE_C_COMPILER ${CROSS_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_PREFIX}g++)

set(CMAKE_C_FLAGS "-march=armv7-a -mthumb -mfpu=neon  -mfloat-abi=hard --sysroot=/home/dd/Tools/imx6-SDK/sysroots/armv7at2hf-neon-angstrom-linux-gnueabi" )
set(CMAKE_CXX_FLAGS "-march=armv7-a -mthumb -mfpu=neon  -mfloat-abi=hard --sysroot=/home/dd/Tools/imx6-SDK/sysroots/armv7at2hf-neon-angstrom-linux-gnueabi" )
#
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CROSS_COMPILATION_ARCHITECTURE armv7-a)
