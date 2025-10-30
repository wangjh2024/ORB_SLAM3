cmake_minimum_required(VERSION 3.20)
project(DBoW2 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 使用vcpkg工具链（如果存在）
if(EXISTS "E:/3rd_party/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "E:/3rd_party/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg toolchain")
endif()

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

# === 手动配置 Boost ===
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")
set(BOOST_INCLUDE_DIR "${BOOST_ROOT}/include/boost-1_83")
if(NOT EXISTS "${BOOST_INCLUDE_DIR}")
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}/include")
endif()

if(NOT EXISTS "${BOOST_INCLUDE_DIR}/boost")
    message(FATAL_ERROR "Boost headers not found in ${BOOST_INCLUDE_DIR}")
endif()

message(STATUS "Using Boost from: ${BOOST_INCLUDE_DIR}")

# 创建Boost目标
add_library(Boost::headers INTERFACE IMPORTED)
set_target_properties(Boost::headers PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
)

# === 手动配置 OpenCV（根据图片修正路径）===
# 根据图片，OpenCV的正确路径应该是：
set(OPENCV_ROOT "E:/3rd_party/opencv")
set(OPENCV_BUILD_DIR "${OPENCV_ROOT}/build")  # 这是图片中显示的路径

# 检查OpenCV配置文件是否存在
if(EXISTS "${OPENCV_BUILD_DIR}/OpenCVConfig.cmake")
    message(STATUS "Found OpenCVConfig.cmake at: ${OPENCV_BUILD_DIR}")
    
    # 使用CMake的find_package来正确配置OpenCV
    set(OpenCV_DIR "${OPENCV_BUILD_DIR}")
    find_package(OpenCV REQUIRED)
    
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
    message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
else()
    message(FATAL_ERROR "OpenCVConfig.cmake not found in ${OPENCV_BUILD_DIR}. "
            "Please check your OpenCV installation.")
endif()

# 编译器设置
if(MSVC)
    add_compile_options(/W3 /wd4267 /wd4244 /wd4996 /MP)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /Ob2 /DNDEBUG)
    else()
        add_compile_options(/Od /Zi)
    endif()
endif()

# 源文件定义
set(DBOW2_SOURCES
    DBoW2/BowVector.cpp
    DBoW2/FORB.cpp
    DBoW2/FeatureVector.cpp
    DBoW2/ScoringObject.cpp
    DUtils/Random.cpp
    DUtils/Timestamp.cpp
)

# 创建库目标
add_library(DBoW2 SHARED ${DBOW2_SOURCES})

# 包含目录
target_include_directories(DBoW2 PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${BOOST_INCLUDE_DIR}
)

# 链接库
target_link_libraries(DBoW2 PUBLIC 
    ${OpenCV_LIBS}
    Boost::headers
)

# 禁用自动DLL复制
set(VCPKG_APPLOCAL_DEPS OFF)

# 配置总结
message(STATUS "")
message(STATUS "=== DBoW2 Configuration Summary ===")
message(STATUS "Build type:          ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard:        14")
message(STATUS "Boost include:       ${BOOST_INCLUDE_DIR}")
message(STATUS "OpenCV root:         ${OPENCV_ROOT}")
message(STATUS "OpenCV build dir:    ${OPENCV_BUILD_DIR}")
message(STATUS "OpenCV version:      ${OpenCV_VERSION}")
message(STATUS "================================")