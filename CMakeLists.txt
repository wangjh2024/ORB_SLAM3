cmake_minimum_required(VERSION 3.20...3.28)
project(ORB_SLAM3)

# ========== 策略设置 ==========
cmake_policy(SET CMP0167 NEW)  # 解决 FindBoost 模块警告
cmake_policy(SET CMP0157 OLD)
cmake_policy(SET CMP0148 NEW)
cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0126 NEW)
cmake_policy(SET CMP0115 NEW)
cmake_policy(SET CMP0114 NEW)

# ========== SOPHUS 冲突修复 ==========
set(Sophus_DIR "")
set(SOPHUS_FOUND FALSE)
set(Sophus_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus)
set(CMAKE_DISABLE_FIND_PACKAGE_Sophus TRUE)
message(STATUS "Forcing use of project Sophus: ${Sophus_INCLUDE_DIRS}")

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 平台特定设置
if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -DNOMINMAX
        -DWIN32_LEAN_AND_MEAN
        -D_HAS_STD_BYTE=0
    )
    
    add_compile_options(
        /W3
        /wd4267 /wd4244 /wd4305 /wd4819 /wd4996 /wd4018
        /wd4251 /wd4275
        /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
        /D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
        /D_CRT_SECURE_NO_WARNINGS
        /Zc:__cplusplus
        /D_HAS_STD_BYTE=0
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        add_compile_options(/O2 /Ob2 /DNDEBUG)
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
        add_compile_options(/Od /DEBUG)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-march=native" HAS_MARCH_NATIVE)
        if(HAS_MARCH_NATIVE)
            add_compile_options(-march=native)
        endif()
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O0 -g -DDEBUG)
    endif()
endif()

# 设置输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# ========== 修复第三方库配置（在查找依赖之前） ==========
message(STATUS "Fixing thirdparty library configurations...")

# 修复 DBoW2 的 CMakeLists.txt - 移除 Boost 序列化依赖
if(EXISTS "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/CMakeLists.txt")
    file(READ "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/CMakeLists.txt" DBOW2_ORIGINAL)
    
    # 移除 Boost 序列化组件查找
    string(REPLACE 
        "find_package(Boost REQUIRED COMPONENTS serialization)" 
        "# find_package(Boost REQUIRED COMPONENTS serialization) - Removed to avoid vcpkg conflict\n    set(Boost_FOUND TRUE)" 
        DBOW2_MODIFIED ${DBOW2_ORIGINAL})
    
    # 移除版本限制
    string(REPLACE 
        "cmake_minimum_required(VERSION 3.10)" 
        "cmake_minimum_required(VERSION 3.10...3.28)" 
        DBOW2_MODIFIED ${DBOW2_MODIFIED})
    
    file(WRITE "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/CMakeLists.txt" ${DBOW2_MODIFIED})
    message(STATUS "DBoW2 CMakeLists.txt fixed")
endif()

# 修复 g2o 的 CMakeLists.txt
if(EXISTS "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/CMakeLists.txt")
    file(READ "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/CMakeLists.txt" G2O_ORIGINAL)
    string(REPLACE 
        "CMAKE_MINIMUM_REQUIRED(VERSION 3.5)" 
        "cmake_minimum_required(VERSION 3.5...3.28)" 
        G2O_MODIFIED ${G2O_ORIGINAL})
    file(WRITE "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/CMakeLists.txt" ${G2O_MODIFIED})
    message(STATUS "g2o CMakeLists.txt fixed")
endif()

# ========== 查找依赖包 ==========
message(STATUS "Searching for dependencies...")

# OpenCV 查找优化
find_package(OpenCV 4 QUIET)
if(NOT OpenCV_FOUND)
   message(STATUS "OpenCV 4.x not found, trying OpenCV 3.x...")
   find_package(OpenCV 3.4.3 QUIET)
   if(NOT OpenCV_FOUND)
      message(FATAL_ERROR "OpenCV >= 3.4.3 not found. Required components: core, features2d, calib3d")
   endif()
endif()
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

# Eigen3 查找优化
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Pangolin 查找优化
find_package(Pangolin REQUIRED)

# ========== Boost 配置 - 完全避免 vcpkg 干扰 ==========
set(BOOST_ROOT "E:/code/3rd_party/boost_1_83_0")
message(STATUS "Setting Boost root: ${BOOST_ROOT}")

# 完全禁用 vcpkg 对 Boost 的干扰
set(VCPKG_TOOLCHAIN OFF)  # 临时禁用 vcpkg

# 设置 Boost 查找参数
set(Boost_NO_SYSTEM_PATHS ON)
set(Boost_NO_BOOST_CMAKE ON)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)

# 强制禁用 Boost 序列化
add_definitions(-DBOOST_SERIALIZATION_DYN_LINK=0)
add_definitions(-DUSE_BOOST_SERIALIZATION=OFF)
message(STATUS "Boost serialization explicitly disabled")

# 手动设置 Boost 路径（避免任何自动查找）
set(Boost_INCLUDE_DIR "${BOOST_ROOT}")
set(Boost_LIBRARY_DIR "${BOOST_ROOT}/lib64-msvc-14.3")

# 检查 Boost 目录是否存在
if(NOT EXISTS "${Boost_INCLUDE_DIR}/boost")
    message(WARNING "Boost include directory not found: ${Boost_INCLUDE_DIR}/boost")
endif()

if(NOT EXISTS "${Boost_LIBRARY_DIR}")
    message(WARNING "Boost library directory not found: ${Boost_LIBRARY_DIR}")
endif()

# 手动查找 Boost 库文件
find_library(BOOST_SYSTEM_LIB
    NAMES boost_system-vc143-mt-x64-1_83 libboost_system-vc143-mt-x64-1_83
    PATHS "${Boost_LIBRARY_DIR}"
    NO_DEFAULT_PATH
)

find_library(BOOST_FILESYSTEM_LIB
    NAMES boost_filesystem-vc143-mt-x64-1_83 libboost_filesystem-vc143-mt-x64-1_83
    PATHS "${Boost_LIBRARY_DIR}"
    NO_DEFAULT_PATH
)

if(BOOST_SYSTEM_LIB AND BOOST_FILESYSTEM_LIB)
    set(Boost_FOUND TRUE)
    set(Boost_VERSION 1.83.0)
    set(Boost_INCLUDE_DIRS "${Boost_INCLUDE_DIR}")
    set(Boost_LIBRARIES "${BOOST_SYSTEM_LIB};${BOOST_FILESYSTEM_LIB}")
    message(STATUS "Boost manually configured:")
    message(STATUS "  Include: ${Boost_INCLUDE_DIRS}")
    message(STATUS "  Libraries: ${Boost_LIBRARIES}")
else()
    set(Boost_FOUND FALSE)
    message(WARNING "Boost libraries not found in ${Boost_LIBRARY_DIR}")
endif()

# 重新启用 vcpkg（用于其他包）
set(VCPKG_TOOLCHAIN ON)

# 其他依赖
find_package(Threads REQUIRED)
find_package(realsense2 QUIET)

# ========== 添加第三方库子目录（使用修复后的配置） ==========
message(STATUS "Adding thirdparty libraries...")

# 设置变量告诉第三方库不要查找 Boost
set(Boost_FOUND TRUE)  # 欺骗第三方库，让它们认为 Boost 已找到
set(Boost_VERSION 1.83.0)

add_subdirectory(Thirdparty/g2o)
add_subdirectory(Thirdparty/DBoW2)

# 验证第三方库构建
if(NOT TARGET g2o)
    message(FATAL_ERROR "g2o target not created successfully")
endif()
if(NOT TARGET DBoW2)
    message(FATAL_ERROR "DBoW2 target not created successfully")
endif()

# ========== 主库构建 ==========
message(STATUS "Searching for source files...")

file(GLOB_RECURSE ORB_SLAM3_SOURCES 
    "src/*.cc"
    "src/*.cpp"
    "src/**/*.cc"
    "src/**/*.cpp"
)

# 验证源文件
set(VALID_SOURCES)
foreach(SRC_FILE ${ORB_SLAM3_SOURCES})
    if(EXISTS ${SRC_FILE})
        list(APPEND VALID_SOURCES ${SRC_FILE})
    endif()
endforeach()

list(LENGTH VALID_SOURCES SOURCE_COUNT)
if(SOURCE_COUNT EQUAL 0)
    message(FATAL_ERROR "No valid source files found!")
else()
    message(STATUS "Found ${SOURCE_COUNT} source files")
endif()

# 创建主库
add_library(${PROJECT_NAME} SHARED ${VALID_SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# 包含目录
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/CameraModels>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
    ${EIGEN3_INCLUDE_DIR}
    ${Pangolin_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
)

if(Boost_FOUND)
    target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${Boost_INCLUDE_DIRS})
endif()

# 链接库
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${OpenCV_LIBS}
    ${Pangolin_LIBRARIES}
    DBoW2
    g2o
    Threads::Threads
)

if(Boost_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES})
endif()

# 设置目标属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# RealSense 支持
if(realsense2_FOUND)
    message(STATUS "Adding RealSense2 support")
    target_include_directories(${PROJECT_NAME} PRIVATE ${realsense2_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${realsense2_LIBRARY})
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_REALSENSE=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_REALSENSE=0)
endif()

# 性能优化选项
option(ORB_SLAM3_ENABLE_AVX "Enable AVX instructions" ON)
option(ORB_SLAM3_ENABLE_OPENMP "Enable OpenMP parallelization" ON)

if(ORB_SLAM3_ENABLE_AVX)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -mavx2 -mfma)
    endif()
endif()

# ========== 示例程序构建（简化版） ==========
function(add_orbslam3_example example_name example_source example_category)
    if(NOT EXISTS ${example_source})
        message(WARNING "Example source file not found: ${example_source}")
        return()
    endif()
    
    add_executable(${example_name} ${example_source})
    target_compile_features(${example_name} PRIVATE cxx_std_17)
    target_link_libraries(${example_name} PRIVATE ${PROJECT_NAME})
    
    set_target_properties(${example_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/${example_category}
        OUTPUT_NAME ${example_name}
    )
    
    if(EXISTS "${PROJECT_SOURCE_DIR}/Vocabulary/ORBvoc.txt")
        add_custom_command(TARGET ${example_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/Vocabulary/ORBvoc.txt"
            $<TARGET_FILE_DIR:${example_name}>
            COMMENT "Copying ORB vocabulary"
            VERBATIM
        )
    endif()
endfunction()

# 构建基本示例
add_orbslam3_example(rgbd_tum Examples/RGB-D/rgbd_tum.cc RGB-D)
add_orbslam3_example(stereo_kitti Examples/Stereo/stereo_kitti.cc Stereo)
add_orbslam3_example(mono_tum Examples/Monocular/mono_tum.cc Monocular)

# ========== 安装规则 ==========
install(TARGETS ${PROJECT_NAME}
    EXPORT ORB_SLAM3Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.h")

# ========== 配置总结 ==========
message(STATUS "")
message(STATUS "=== ORB_SLAM3 Configuration Summary ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  OpenCV: ${OpenCV_VERSION}")
message(STATUS "  Eigen3: ${EIGEN3_VERSION_STRING}")
message(STATUS "  Pangolin: ${Pangolin_FOUND}")
message(STATUS "  Boost: ${Boost_FOUND} (Manual setup)")
message(STATUS "  RealSense2: ${realsense2_FOUND}")
message(STATUS "")
message(STATUS "Thirdparty Libraries: Fixed and built successfully")
message(STATUS "========================================")
message(STATUS "ORB_SLAM3 configuration completed!")