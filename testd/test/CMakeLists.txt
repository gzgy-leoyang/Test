#当前目录下的所有源文件存入_SRC_FILES变量
aux_source_directory(. _SRC_FILES)
#生成目标为mymath库，因为顶层项目名称为mymath
add_library(${PROJECT_NAME} ${_SRC_FILES})
#target_link_libraries(${PROJECT_NAME} PUBLIC spdlog)
# VERSION指代动态库版本，SOVERSION指代API版本

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES VERSION ${PROJECT_VERSION}
             SOVERSION ${PROJECT_VERSION_MAJOR})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

#安装目标
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin)
