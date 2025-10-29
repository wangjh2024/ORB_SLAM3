cmake_minimum_required(VERSION 3.20...3.28)
project(ORB_SLAM3)

# ========== SOPHUS 冲突修复 ==========
# 强制使用项目自带的 Sophus，禁用系统 Sophus
set(Sophus_DIR "")
set(SOPHUS_FOUND FALSE)
set(Sophus_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus)

# 确保不查找系统 Sophus
set(CMAKE_DISABLE_FIND_PACKAGE_Sophus TRUE)

message(STATUS "Forcing use of project Sophus: ${Sophus_INCLUDE_DIRS}")


# 设置所有已知的策略以消除警告
cmake_policy(SET CMP0157 OLD)  # 旧版本兼容性
cmake_policy(SET CMP0167 NEW)  # FindBoost 模块
cmake_policy(SET CMP0148 NEW)  # FindPackageHandleStandardArgs
cmake_policy(SET CMP0135 NEW)  # FindPackage 模式
cmake_policy(SET CMP0126 NEW)  # 策略版本
cmake_policy(SET CMP0115 NEW)  # 源文件存在性检查
cmake_policy(SET CMP0114 NEW)  # 文件 GLOB 递归
# 设置策略以兼容旧版本 CMake
cmake_policy(SET CMP0157 OLD)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)




# 强制禁用 Boost 序列化
add_definitions(-DUSE_BOOST_SERIALIZATION=OFF)
message(STATUS "Boost serialization explicitly disabled.")

# 仍然查找 Boost，但只用于其他组件（例如 system, filesystem），不依赖 serialization
find_package(Boost COMPONENTS system filesystem)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    message(STATUS "Boost found: ${Boost_VERSION}")
else()
    message(WARNING "Boost not found, some features may be disabled.")
endif()

# # 强制禁用 Boost 序列化
# add_definitions(-DUSE_BOOST_SERIALIZATION=OFF)
# message(STATUS "Boost serialization explicitly disabled.")

# # 仍然查找 Boost，但只用于其他组件
# find_package(Boost COMPONENTS system filesystem)
# if(Boost_FOUND)
#     include_directories(${Boost_INCLUDE_DIRS})
#     message(STATUS "Boost found: ${Boost_VERSION}")
# else()
#     message(WARNING "Boost not found, some features may be disabled.")
# endif()






# 平台特定设置
if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    
    
    # Windows 特定修复 - 添加这些定义
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -DNOMINMAX
        -DWIN32_LEAN_AND_MEAN
        -D_HAS_STD_BYTE=0  # 禁用 std::byte，解决冲突
    )
    
    # 修复 byte 冲突问题
    add_compile_options(
        /W3
        /wd4267 /wd4244 /wd4305 /wd4819 /wd4996 /wd4018
        /wd4251 /wd4275
        # /wd2872  # 禁用 byte 冲突警告
        /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
        /D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
        /D_CRT_SECURE_NO_WARNINGS
        /Zc:__cplusplus
        /D_HAS_STD_BYTE=0  # 明确禁用 std::byte
    )

    # # Boost 序列化修复
    # add_definitions(
    #     -DBOOST_SERIALIZATION_ACCESS=friend
    #     -DBOOST_ALL_NO_LIB
    #     -DBOOST_SERIALIZATION_DYN_LINK
    #     -DBOOST_ALLOW_DEPRECATED_HEADERS
    # )
    
    # 设置运行时库
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
        # 检查并添加架构优化
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

# 创建输出目录
file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

# 查找依赖包
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

# 验证 OpenCV 组件
set(REQUIRED_OPENCV_COMPONENTS core features2d calib3d imgproc highgui)
foreach(COMPONENT ${REQUIRED_OPENCV_COMPONENTS})
    if(NOT OpenCV_${COMPONENT}_FOUND)
        message(WARNING "OpenCV component ${COMPONENT} not found")
    endif()
endforeach()

# Eigen3 查找优化
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Pangolin 查找优化
find_package(Pangolin REQUIRED)

# Boost 查找优化
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED COMPONENTS serialization system filesystem)

if(Boost_FOUND AND Boost_VERSION VERSION_LESS 1.71)
    message(WARNING "Boost version ${Boost_VERSION} may have compatibility issues")
endif()

# 其他依赖
find_package(Threads REQUIRED)
find_package(realsense2 QUIET)

# 验证依赖
message(STATUS "=== Dependency Status ===")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "Eigen3 version: ${EIGEN3_VERSION_STRING}")
message(STATUS "Pangolin found: ${Pangolin_FOUND}")
message(STATUS "Boost found: ${Boost_FOUND}")
message(STATUS "RealSense2 found: ${realsense2_FOUND}")

if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found! Please install OpenCV.")
endif()

if(NOT Pangolin_FOUND)
    message(FATAL_ERROR "Pangolin not found! Please install Pangolin.")
endif()

# 检查第三方库是否存在
set(THIRDPARTY_DIRS Thirdparty/g2o Thirdparty/DBoW2)
foreach(DIR ${THIRDPARTY_DIRS})
    if(NOT EXISTS ${PROJECT_SOURCE_DIR}/${DIR}/CMakeLists.txt)
        message(FATAL_ERROR "Thirdparty library not found: ${DIR}")
    endif()
endforeach()

# 修复第三方库的 CMakeLists.txt
message(STATUS "Checking and fixing thirdparty CMakeLists.txt files...")

# 修复 g2o 的 CMakeLists.txt
file(READ "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/CMakeLists.txt" G2O_CMAKE_CONTENT)
string(REPLACE "CMAKE_MINIMUM_REQUIRED(VERSION 3.5)" 
               "cmake_minimum_required(VERSION 3.5...3.28)" 
               G2O_CMAKE_CONTENT_FIXED ${G2O_CMAKE_CONTENT})
file(WRITE "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/CMakeLists.txt" ${G2O_CMAKE_CONTENT_FIXED})

# 修复 DBoW2 的 CMakeLists.txt  
file(READ "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/CMakeLists.txt" DBOW2_CMAKE_CONTENT)
string(REPLACE "cmake_minimum_required(VERSION 3.10)" 
               "cmake_minimum_required(VERSION 3.10...3.28)" 
               DBOW2_CMAKE_CONTENT_FIXED ${DBOW2_CMAKE_CONTENT})
file(WRITE "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/CMakeLists.txt" ${DBOW2_CMAKE_CONTENT_FIXED})

message(STATUS "Thirdparty CMakeLists.txt files fixed")

# 添加第三方库子目录
add_subdirectory(Thirdparty/g2o)
add_subdirectory(Thirdparty/DBoW2)

# 验证第三方库构建
if(NOT TARGET g2o)
    message(FATAL_ERROR "g2o target not created successfully")
endif()
if(NOT TARGET DBoW2)
    message(FATAL_ERROR "DBoW2 target not created successfully")
endif()

# 自动查找所有源文件
message(STATUS "Searching for source files...")

file(GLOB_RECURSE ORB_SLAM3_SOURCES 
    "src/*.cc"
    "src/*.cpp"
    "src/**/*.cc"
    "src/**/*.cpp"
)

# 验证源文件是否存在
set(VALID_SOURCES)
foreach(SRC_FILE ${ORB_SLAM3_SOURCES})
    if(EXISTS ${SRC_FILE})
        list(APPEND VALID_SOURCES ${SRC_FILE})
    else()
        message(WARNING "Source file not found: ${SRC_FILE}")
    endif()
endforeach()

# 检查源文件列表是否为空
list(LENGTH VALID_SOURCES SOURCE_COUNT)
if(SOURCE_COUNT EQUAL 0)
    message(FATAL_ERROR "No valid source files found! Check your project structure.")
else()
    message(STATUS "Found ${SOURCE_COUNT} source files")
endif()

# 创建主库
add_library(${PROJECT_NAME} SHARED ${VALID_SOURCES})

# 使用现代 CMake 设置目标属性
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# 设置包含目录（使用现代语法）
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/CameraModels>
    $<INSTALL_INTERFACE:include>
)
include_directories(${PROJECT_SOURCE_DIR}/Thirdparty/Sophus)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
    ${EIGEN3_INCLUDE_DIR}
    ${Pangolin_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# 链接库
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${OpenCV_LIBS}
    ${Pangolin_LIBRARIES}
    ${Boost_LIBRARIES}
    # ${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/Release/g2o.lib
    # ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/Release/DBoW2.lib

    DBoW2
    g2o
    Threads::Threads
)

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
    message(STATUS "RealSense2 not found - skipping RealSense examples")
endif()

# 性能优化选项
option(ORB_SLAM3_ENABLE_AVX "Enable AVX instructions" ON)
option(ORB_SLAM3_ENABLE_OPENMP "Enable OpenMP parallelization" ON)
option(ORB_SLAM3_ENABLE_LTO "Enable Link Time Optimization" OFF)

if(ORB_SLAM3_ENABLE_AVX)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /arch:AVX2)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -mavx2 -mfma)
    endif()
endif()

if(ORB_SLAM3_ENABLE_LTO AND CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_OUTPUT)
    if(IPO_SUPPORTED)
        set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

# 示例程序构建函数
function(add_orbslam3_example example_name example_source example_category)
    # 验证源文件存在
    if(NOT EXISTS ${example_source})
        message(WARNING "Example source file not found: ${example_source}")
        return()
    endif()
    
    add_executable(${example_name} ${example_source})
    
    # 设置 C++ 标准
    target_compile_features(${example_name} PRIVATE cxx_std_17)
    
    # 链接主库
    target_link_libraries(${example_name} PRIVATE ${PROJECT_NAME})
    
    # 设置输出目录
    set_target_properties(${example_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/${example_category}
        OUTPUT_NAME ${example_name}
    )
    
    # 复制词汇表文件
    if(EXISTS "${PROJECT_SOURCE_DIR}/Vocabulary/ORBvoc.txt")
        add_custom_command(TARGET ${example_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/Vocabulary/ORBvoc.txt"
            $<TARGET_FILE_DIR:${example_name}>
            COMMENT "Copying ORB vocabulary to $<TARGET_FILE_DIR:${example_name}>"
            VERBATIM
        )
    else()
        message(WARNING "ORB vocabulary file not found: ${PROJECT_SOURCE_DIR}/Vocabulary/ORBvoc.txt")
    endif()
    
    # 安装示例
    install(TARGETS ${example_name}
        RUNTIME DESTINATION bin
    )
endfunction()

# 构建示例程序
message(STATUS "Building examples...")

# RGB-D examples
add_orbslam3_example(rgbd_tum Examples/RGB-D/rgbd_tum.cc RGB-D)

if(realsense2_FOUND)
    add_orbslam3_example(rgbd_realsense_D435i Examples/RGB-D/rgbd_realsense_D435i.cc RGB-D)
endif()

# RGB-D inertial examples
if(realsense2_FOUND)
    add_orbslam3_example(rgbd_inertial_realsense_D435i Examples/RGB-D-Inertial/rgbd_inertial_realsense_D435i.cc RGB-D-Inertial)
endif()

# Stereo examples
add_orbslam3_example(stereo_kitti Examples/Stereo/stereo_kitti.cc Stereo)
add_orbslam3_example(stereo_euroc Examples/Stereo/stereo_euroc.cc Stereo)
add_orbslam3_example(stereo_tum_vi Examples/Stereo/stereo_tum_vi.cc Stereo)

if(realsense2_FOUND)
    add_orbslam3_example(stereo_realsense_t265 Examples/Stereo/stereo_realsense_t265.cc Stereo)
    add_orbslam3_example(stereo_realsense_D435i Examples/Stereo/stereo_realsense_D435i.cc Stereo)
endif()

# Monocular examples
add_orbslam3_example(mono_tum Examples/Monocular/mono_tum.cc Monocular)
add_orbslam3_example(mono_kitti Examples/Monocular/mono_kitti.cc Monocular)
add_orbslam3_example(mono_euroc Examples/Monocular/mono_euroc.cc Monocular)
add_orbslam3_example(mono_tum_vi Examples/Monocular/mono_tum_vi.cc Monocular)

if(realsense2_FOUND)
    add_orbslam3_example(mono_realsense_t265 Examples/Monocular/mono_realsense_t265.cc Monocular)
    add_orbslam3_example(mono_realsense_D435i Examples/Monocular/mono_realsense_D435i.cc Monocular)
endif()

# Monocular inertial examples
add_orbslam3_example(mono_inertial_euroc Examples/Monocular-Inertial/mono_inertial_euroc.cc Monocular-Inertial)
add_orbslam3_example(mono_inertial_tum_vi Examples/Monocular-Inertial/mono_inertial_tum_vi.cc Monocular-Inertial)

if(realsense2_FOUND)
    add_orbslam3_example(mono_inertial_realsense_t265 Examples/Monocular-Inertial/mono_inertial_realsense_t265.cc Monocular-Inertial)
    add_orbslam3_example(mono_inertial_realsense_D435i Examples/Monocular-Inertial/mono_inertial_realsense_D435i.cc Monocular-Inertial)
endif()

# Stereo Inertial examples
add_orbslam3_example(stereo_inertial_euroc Examples/Stereo-Inertial/stereo_inertial_euroc.cc Stereo-Inertial)
add_orbslam3_example(stereo_inertial_tum_vi Examples/Stereo-Inertial/stereo_inertial_tum_vi.cc Stereo-Inertial)

if(realsense2_FOUND)
    add_orbslam3_example(stereo_inertial_realsense_t265 Examples/Stereo-Inertial/stereo_inertial_realsense_t265.cc Stereo-Inertial)
    add_orbslam3_example(stereo_inertial_realsense_D435i Examples/Stereo-Inertial/stereo_inertial_realsense_D435i.cc Stereo-Inertial)
endif()

# Calibration examples
if(realsense2_FOUND)
    add_orbslam3_example(recorder_realsense_D435i Examples/Calibration/recorder_realsense_D435i.cc Calibration)
    add_orbslam3_example(recorder_realsense_T265 Examples/Calibration/recorder_realsense_T265.cc Calibration)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    EXPORT ORB_SLAM3Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
    PATTERN "*.hpp"
)

# 配置总结
message(STATUS "")
message(STATUS "=== ORB_SLAM3 Configuration Summary ===")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Dependencies:")
message(STATUS "  OpenCV: ${OpenCV_VERSION} (${OpenCV_DIR})")
message(STATUS "  Eigen3: ${EIGEN3_VERSION_STRING}")
message(STATUS "  Pangolin: ${Pangolin_VERSION}")
message(STATUS "  Boost: ${Boost_VERSION}")
message(STATUS "  RealSense2: ${realsense2_FOUND}")
message(STATUS "")
message(STATUS "Paths:")
message(STATUS "  Library Output: ${LIBRARY_OUTPUT_PATH}")
message(STATUS "  Executable Output: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "  Thirdparty Libraries: Built from source")
message(STATUS "")
message(STATUS "Performance Options:")
message(STATUS "  AVX: ${ORB_SLAM3_ENABLE_AVX}")
message(STATUS "  OpenMP: ${ORB_SLAM3_ENABLE_OPENMP}")
message(STATUS "  LTO: ${ORB_SLAM3_ENABLE_LTO}")
message(STATUS "========================================")
message(STATUS "ORB_SLAM3 configuration completed successfully!")