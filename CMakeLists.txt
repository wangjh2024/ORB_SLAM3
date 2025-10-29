cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 平台设置
if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/W3 /wd4267 /wd4244 /wd4996)
endif()

# 查找依赖
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Pangolin REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization)
find_package(Threads REQUIRED)

# 关键修复：直接使用 vcpkg 的库路径
set(VCPKG_DIR "E:/3rd_party/vcpkg/installed/x64-windows")

# 手动设置 DBoW2 和 g2o
set(DBoW2_INCLUDE_DIR "${VCPKG_DIR}/include")
set(DBoW2_LIBRARY "${VCPKG_DIR}/lib/DBoW2.lib")

set(g2o_INCLUDE_DIR "${VCPKG_DIR}/include")
set(g2o_LIBRARY "${VCPKG_DIR}/lib/g2o.lib")

# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${EIGEN3_INCLUDE_DIR}
    ${Pangolin_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${DBoW2_INCLUDE_DIR}
    ${g2o_INCLUDE_DIR}
)

# 查找源文件
file(GLOB_RECURSE SOURCES "src/*.cc" "src/*.cpp")

# 创建库
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${Pangolin_LIBRARIES}
    ${Boost_LIBRARIES}
    ${DBoW2_LIBRARY}
    ${g2o_LIBRARY}
    Threads::Threads
)

message(STATUS "ORB_SLAM3 configuration completed!")