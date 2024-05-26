#添加当前目录下的所有源文件
aux_source_directory(. _SRC_FILES)

# libdrm >>>
#include(GNUInstallDirs)
find_package(PkgConfig REQUIRED)
pkg_check_modules(DRM QUIET libdrm)
if (NOT DRM_FOUND)
  message(FATAL_ERROR "Missing dependency: libdrm")
endif()
# libdrm <<<

# 添加子目录下的源文件,分两种方式:
# 方式1:子目录源码编译为库,
# 添加库文件夹,嵌套下一层包含cmakelists文件的文件夹

# 方式2: 子目录源码直接使用
# 添加源文件夹,将指定文件夹下源文件全部加入 _SRC_FILES 变量
aux_source_directory(./net _SRC_FILES)
aux_source_directory(./service _SRC_FILES)
aux_source_directory(./adapter _SRC_FILES)
aux_source_directory(./support _SRC_FILES)
## 子目录添加完成

# 用于生成共享库
if(${PLATFORM_CHIP_TYPE} STREQUAL "rk3568")
  link_directories(${PROJECT_SOURCE_DIR}/lib)
  add_definitions(-D_RK3568_MCU)
endif()
# 用于生成应用程序
add_executable(${PROJECT_NAME} ${_SRC_FILES})

# libdrm >>>
#target_include_directories(${PROJECT_NAME} PRIVATE
#  ${DRM_INCLUDE_DIRS}
#)
#target_link_directories(${PROJECT_NAME} PRIVATE
#  ${DRM_LIBRARY_DIRS}
#)
#target_link_libraries(${PROJECT_NAME} PRIVATE
#  ${DRM_LIBRARIES}
#)

# libdrm <<<

# 添加链接库
if(${PLATFORM_CHIP_TYPE} STREQUAL "rk3568")
  target_link_libraries(${PROJECT_NAME} 
    ${DRM_LIBRARY_DIRS} 
    mcuuart)
endif()

target_link_libraries(${PROJECT_NAME} 
  pthread 
  ${DRM_LIBRARIES}
)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC 
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/inc>  
  ${DRM_INCLUDE_DIRS})


#安装目标
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME DESTINATION bin)
