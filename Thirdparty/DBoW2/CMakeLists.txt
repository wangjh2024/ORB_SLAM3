cmake_minimum_required(VERSION 3.20)
project(DBoW2 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# === 生成静态库 ===
set(BUILD_SHARED_LIBS OFF)

# === 编译器设置 ===
if(MSVC)
    # 使用 /utf-8 选项（不要同时使用 /source-charset:utf-8）
    add_compile_options(/utf-8)
    
    # 禁用特定警告
    add_compile_options(/wd4828)  # 字符编码警告
    add_compile_options(/wd4267 /wd4244 /wd4996 /wd4018 /wd4251 /wd4275)
    
    # 多核编译
    add_compile_options(/MP)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /Ob2 /DNDEBUG)
    else()
        add_compile_options(/Od /Zi)
    endif()
endif()

# === 依赖配置 ===
# Boost配置
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")
set(BOOST_INCLUDE_DIR "${BOOST_ROOT}/include/boost-1_83")
if(NOT EXISTS "${BOOST_INCLUDE_DIR}")
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}/include")
endif()

if(NOT EXISTS "${BOOST_INCLUDE_DIR}/boost")
    message(FATAL_ERROR "Boost headers not found in ${BOOST_INCLUDE_DIR}")
endif()

message(STATUS "Using Boost from: ${BOOST_INCLUDE_DIR}")

# OpenCV配置
set(OpenCV_DIR "E:/3rd_party/opencv/build")
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV found: ${OpenCV_VERSION}")

# === 输出目录设置 ===
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# === 源文件定义 ===
set(DBOW2_SOURCES
    DBoW2/BowVector.cpp
    DBoW2/FORB.cpp
    DBoW2/FeatureVector.cpp
    DBoW2/ScoringObject.cpp
    DUtils/Random.cpp
    DUtils/Timestamp.cpp
)

# === 创建静态库 ===
add_library(DBoW2 STATIC ${DBOW2_SOURCES})

# 设置目标属性
set_target_properties(DBoW2 PROPERTIES
    OUTPUT_NAME "DBoW2"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    POSITION_INDEPENDENT_CODE ON
)

# 包含目录
target_include_directories(DBoW2 PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(DBoW2 PUBLIC ${OpenCV_LIBS})

# === Boost序列化修复 ===
# 禁用Boost序列化（解决_access错误）
target_compile_definitions(DBoW2 PRIVATE 
    DONT_USE_BOOST_SERIALIZATION
    BOOST_SERIALIZATION_NO_LIB
)

# 配置总结
message(STATUS "")
message(STATUS "=== DBoW2 Static Library Configuration ===")
message(STATUS "Build type:          STATIC")
message(STATUS "C++ standard:        14")
message(STATUS "Boost include:       ${BOOST_INCLUDE_DIR}")
message(STATUS "OpenCV version:      ${OpenCV_VERSION}")
message(STATUS "Output directory:    ${PROJECT_SOURCE_DIR}/lib")
message(STATUS "================================")