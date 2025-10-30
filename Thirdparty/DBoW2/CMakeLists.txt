cmake_minimum_required(VERSION 3.20)
project(DBoW2)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# 完全禁用 Boost 序列化
add_definitions(-DUSE_BOOST_SERIALIZATION=OFF)

# 编译器设置
if(MSVC)
    add_compile_options(/W3 /wd4267 /wd4244 /wd4996)
endif()

# 源文件
set(SRCS_DBOW2
  DBoW2/BowVector.cpp
  DBoW2/FORB.cpp      
  DBoW2/FeatureVector.cpp
  DBoW2/ScoringObject.cpp)

set(SRCS_DUTILS
  DUtils/Random.cpp
  DUtils/Timestamp.cpp)

# OpenCV
find_package(OpenCV REQUIRED)

# 创建库
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
include_directories(${OpenCV_INCLUDE_DIRS})
add_library(DBoW2 SHARED ${SRCS_DBOW2} ${SRCS_DUTILS})
target_link_libraries(DBoW2 ${OpenCV_LIBS})