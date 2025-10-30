cmake_minimum_required(VERSION 3.20)
project(DBoW2 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# === 关键设置：生成静态库 ===
set(BUILD_SHARED_LIBS OFF)  # 强制生成静态库
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# 使用vcpkg工具链（如果存在）
if(EXISTS "E:/3rd_party/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "E:/3rd_party/vcpkg/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg toolchain")
endif()

# === 设置静态库输出路径 ===
set(DBoW2_LIB_OUTPUT "${PROJECT_SOURCE_DIR}/lib/DBoW2.lib")
message(STATUS "DBoW2 static library will be output to: ${DBoW2_LIB_OUTPUT}")

# 设置输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 根据配置设置输出目录
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/lib)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PROJECT_SOURCE_DIR}/bin)
endforeach()

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

# === 配置 OpenCV（根据图片中的路径）===
set(OpenCV_DIR "E:/3rd_party/opencv/build")
if(EXISTS "${OpenCV_DIR}/OpenCVConfig.cmake")
    find_package(OpenCV REQUIRED)
    message(STATUS "OpenCV found: ${OpenCV_VERSION}")
    message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "OpenCV not found at: ${OpenCV_DIR}")
endif()

# === 编译器设置（针对静态库优化）===
if(MSVC)
    # 静态库编译选项
    add_compile_options(/W3 /wd4267 /wd4244 /wd4996 /MP)
    
    # 设置静态运行时库
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /Ob2 /DNDEBUG /MT)
    else()
        add_compile_options(/Od /Zi /MTd)
    endif()
endif()

# === 源文件定义 ===
set(DBOW2_SOURCES
    DBoW2/BowVector.cpp
    DBoW2/FORB.cpp
    DBoW2/FeatureVector.cpp
    DBoW2/ScoringObject.cpp
    DUtils/Random.cpp
    DUtils/Timestamp.cpp
)

# === 创建静态库目标 ===
add_library(DBoW2 STATIC ${DBOW2_SOURCES})

# 设置目标属性（确保输出到指定路径）
set_target_properties(DBoW2 PROPERTIES
    OUTPUT_NAME "DBoW2"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib"
    POSITION_INDEPENDENT_CODE ON
)

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

# 添加静态库编译定义
target_compile_definitions(DBoW2 PRIVATE 
    DBoW2_STATIC
    DBoW2_EXPORTS
    $<$<CONFIG:Release>:NDEBUG>
)

# 禁用自动DLL复制（静态库不需要）
set(VCPKG_APPLOCAL_DEPS OFF)

# === 安装规则 ===
install(TARGETS DBoW2
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES 
    DBoW2/BowVector.h
    DBoW2/FORB.h
    DBoW2/FClass.h
    DBoW2/FeatureVector.h
    DBoW2/ScoringObject.h
    DBoW2/TemplatedVocabulary.h
    DUtils/Random.h
    DUtils/Timestamp.h
    DESTINATION include/DBoW2
)

# === 构建后验证 ===
# 添加自定义目标来验证库文件生成
add_custom_command(TARGET DBoW2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "DBoW2 static library built successfully: ${DBoW2_LIB_OUTPUT}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/lib"
    COMMENT "Verifying DBoW2 static library output"
)

# === 导出变量供父项目使用 ===
set(DBoW2_LIB "${DBoW2_LIB_OUTPUT}" CACHE INTERNAL "Path to DBoW2 static library")
set(DBoW2_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "Path to DBoW2 include directory")

# 配置总结
message(STATUS "")
message(STATUS "=== DBoW2 Static Library Configuration ===")
message(STATUS "Static library output: ${DBoW2_LIB_OUTPUT}")
message(STATUS "Build type:           STATIC")
message(STATUS "C++ standard:         14")
message(STATUS "Boost include:        ${BOOST_INCLUDE_DIR}")
message(STATUS "OpenCV version:       ${OpenCV_VERSION}")
message(STATUS "Output directory:     ${PROJECT_SOURCE_DIR}/lib")
message(STATUS "================================")