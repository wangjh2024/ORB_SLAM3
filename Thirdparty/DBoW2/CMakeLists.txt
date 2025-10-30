cmake_minimum_required(VERSION 3.20)
project(DBoW2 LANGUAGES CXX)

# ===== 基础配置 =====
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ===== 生成静态库 =====
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# ===== 编译器设置 =====
if(MSVC)
    # 编码设置
    add_compile_options(/utf-8)
    
    # 警告设置
    add_compile_options(
        /wd4828   # 字符编码警告
        /wd4267   # size_t转换警告
        /wd4244   # 类型转换可能丢失数据
        /wd4996   # 不安全函数警告
        /wd4018   # 有符号/无符号不匹配
        /wd4251   # DLL接口警告
        /wd4275   # 非DLL接口基类
    )
    
    # 多核编译
    add_compile_options(/MP)
    
    # 优化设置
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /Ob2 /DNDEBUG)
    else()
        add_compile_options(/Od /Zi)
    endif()
endif()

# ===== 依赖配置 =====
# 8.5 Boost 库 - 完全手动配置
message(STATUS "配置 Boost 库...")

# ===== 修正 Boost 路径配置 =====
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")
set(BOOST_INCLUDE_DIR "${BOOST_ROOT}/include/boost-1_83")

# 检查 Boost 目录是否存在
if(NOT EXISTS "${BOOST_INCLUDE_DIR}/boost/serialization/serialization.hpp")
    message(FATAL_ERROR "✗ Boost 头文件未找到，请检查路径: ${BOOST_INCLUDE_DIR}")
else()
    message(STATUS "✓ Boost 头文件目录: ${BOOST_INCLUDE_DIR}")
endif()

# 查找 Boost 序列化库
set(BOOST_LIB_PATHS
    "${BOOST_ROOT}/stage/lib"
    "${BOOST_ROOT}/lib"
    "${BOOST_ROOT}/lib64-msvc-14.3"
)

# 可能的库文件名（根据编译方式不同）
set(BOOST_SERIALIZATION_NAMES
    "boost_serialization-vc143-mt-x64-1_83"
    "boost_serialization-vc143-mt-x64-1_83.lib"
    "boost_serialization-vc143-mt-1_83"
    "boost_serialization-vc143-mt-1_83.lib"
    "boost_serialization-mt-x64-1_83"
    "boost_serialization-mt-x64-1_83.lib"
    "boost_serialization-mt-1_83"
    "boost_serialization-mt-1_83.lib"
    "boost_serialization"
    "boost_serialization.lib"
    "libboost_serialization"
    "libboost_serialization.lib"
)

# OpenCV配置
set(OpenCV_DIR "E:/3rd_party/opencv/build")
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV found: ${OpenCV_VERSION}")

# ===== 输出目录设置 =====
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# ===== 源文件定义 =====
file(GLOB DBOW2_SOURCES 
    "DBoW2/*.cpp"
    "DUtils/*.cpp"
)

# ===== 创建库 =====
add_library(DBoW2 STATIC ${DBOW2_SOURCES})

# 目标属性设置
set_target_properties(DBoW2 PROPERTIES
    OUTPUT_NAME "DBoW2"
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)


# ===== 包含目录设置 =====
target_include_directories(DBoW2 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(DBoW2 PUBLIC
    ${OpenCV_LIBS}
    ${Boost_SERIALIZATION_LIBRARY})

# ===== 安装配置 =====
install(TARGETS DBoW2
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY DBoW2 DUtils
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# ===== 配置总结 =====
message(STATUS "\n=== DBoW2 配置总结 ===")
message(STATUS "项目名称:       ${PROJECT_NAME}")
message(STATUS "构建类型:       ${CMAKE_BUILD_TYPE}")
message(STATUS "C++标准:        ${CMAKE_CXX_STANDARD}")
message(STATUS "Boost版本:      ${Boost_VERSION}")
message(STATUS "OpenCV版本:     ${OpenCV_VERSION}")
message(STATUS "输出目录:       ${PROJECT_SOURCE_DIR}/lib")
message(STATUS "=========================")