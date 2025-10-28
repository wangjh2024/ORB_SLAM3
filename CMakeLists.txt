cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3)

# 在 project() 之后立即添加
if(MSVC)
    # 禁用所有导致编译失败的关键警告
    add_compile_options(
        /wd4267 /wd4244 /wd4305 /wd4819 /wd4996 /wd4018
    )
    
    # 解决标准库兼容性
    add_definitions(
        -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
        -DBOOST_ALLOW_DEPRECATED_HEADERS
    )
endif()
# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Project source directory: ${PROJECT_SOURCE_DIR}")

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 平台特定设置
if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)  # 简化 Windows 下共享库的符号导出
endif()

# 编译器优化设置
if(MSVC)
    # Windows/MSVC 编译器选项
    add_compile_options(/O2 /W3)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/Ox)  # MSVC 的最大优化
    endif()
else()
    # Linux/GCC/Clang 编译器选项
    add_compile_options(-Wall -Wextra -Wpedantic)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -march=native -DNDEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O0 -g -DDEBUG)
    endif()
    
    # 检查是否支持 march=native
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
    if(NOT COMPILER_SUPPORTS_MARCH_NATIVE)
        message(WARNING "Compiler does not support -march=native, using generic optimization")
    endif()
endif()

# 添加自定义模块路径
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

# 设置库输出目录
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# 查找依赖包
message(STATUS "Searching for dependencies...")

# 设置可选的第三方库路径（用户可以通过命令行参数覆盖）
if(NOT DEFINED OpenCV_DIR)
    set(OpenCV_DIR "" CACHE PATH "Path to OpenCV build directory")
endif()

if(NOT DEFINED Pangolin_DIR)
    set(Pangolin_DIR "" CACHE PATH "Path to Pangolin build directory")
endif()

# 查找 OpenCV
find_package(OpenCV REQUIRED
    COMPONENTS core highgui imgproc imgcodecs calib3d features2d flann)

# 查找 Eigen3
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# 查找 Pangolin
find_package(Pangolin REQUIRED)

# 查找 Boost
find_package(Boost REQUIRED COMPONENTS serialization system filesystem)

# 查找线程库
find_package(Threads REQUIRED)

# 可选依赖：RealSense
find_package(realsense2 QUIET)

# 验证依赖
message(STATUS "=== Dependency Status ===")
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "Eigen3 version: ${EIGEN3_VERSION_STRING}")
message(STATUS "Pangolin found: ${Pangolin_FOUND}")
message(STATUS "Boost found: ${Boost_FOUND}")
message(STATUS "RealSense2 found: ${realsense2_FOUND}")

if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found! Please set OpenCV_DIR or install OpenCV.")
endif()

if(NOT Pangolin_FOUND)
    message(FATAL_ERROR "Pangolin not found! Please set Pangolin_DIR or install Pangolin.")
endif()

# 检查第三方库是否存在
set(THIRDPARTY_LIBS
    "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib"
    "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/lib"
)

foreach(LIB_PATH ${THIRDPARTY_LIBS})
    if(NOT EXISTS ${LIB_PATH})
        message(WARNING "Thirdparty library path not found: ${LIB_PATH}")
    endif()
endforeach()

# 包含目录
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/CameraModels
    ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
    ${EIGEN3_INCLUDE_DIR}
    ${Pangolin_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# 添加 g2o 子目录
add_subdirectory(Thirdparty/g2o)

# 定义 ORB_SLAM3 库的源文件
set(ORB_SLAM3_SOURCES
    src/System.cc
    src/Tracking.cc
    src/LocalMapping.cc
    src/LoopClosing.cc
    src/ORBextractor.cc
    src/ORBmatcher.cc
    src/FrameDrawer.cc
    src/Converter.cc
    src/MapPoint.cc
    src/KeyFrame.cc
    src/Atlas.cc
    src/Map.cc
    src/MapDrawer.cc
    src/Optimizer.cc
    src/Frame.cc
    src/KeyFrameDatabase.cc
    src/Sim3Solver.cc
    src/Viewer.cc
    src/ImuTypes.cc
    src/G2oTypes.cc
    src/CameraModels/Pinhole.cpp
    src/CameraModels/KannalaBrandt8.cpp
    src/OptimizableTypes.cpp
    src/MLPnPsolver.cpp
    src/GeometricTools.cc
    src/TwoViewReconstruction.cc
    src/Config.cc
    src/Settings.cc
)

set(ORB_SLAM3_HEADERS
    include/System.h
    include/Tracking.h
    include/LocalMapping.h
    include/LoopClosing.h
    include/ORBextractor.h
    include/ORBmatcher.h
    include/FrameDrawer.h
    include/Converter.h
    include/MapPoint.h
    include/KeyFrame.h
    include/Atlas.h
    include/Map.h
    include/MapDrawer.h
    include/Optimizer.h
    include/Frame.h
    include/KeyFrameDatabase.h
    include/Sim3Solver.h
    include/Viewer.h
    include/ImuTypes.h
    include/G2oTypes.h
    include/CameraModels/GeometricCamera.h
    include/CameraModels/Pinhole.h
    include/CameraModels/KannalaBrandt8.h
    include/OptimizableTypes.h
    include/MLPnPsolver.h
    include/GeometricTools.h
    include/TwoViewReconstruction.h
    include/SerializationUtils.h
    include/Config.h
    include/Settings.h
)

# 创建 ORB_SLAM3 共享库
add_library(${PROJECT_NAME} SHARED ${ORB_SLAM3_SOURCES} ${ORB_SLAM3_HEADERS})

# 设置库属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

# 平台特定的库链接设置
if(WIN32)
    set(DBoW2_LIB "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib")
    set(G2O_LIB "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/lib/g2o.lib")
else()
    set(DBoW2_LIB "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/libDBoW2.so")
    set(G2O_LIB "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/lib/libg2o.so")
endif()

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${Pangolin_LIBRARIES}
    ${Boost_LIBRARIES}
    ${DBoW2_LIB}
    ${G2O_LIB}
    Threads::Threads
)

# 如果找到 RealSense，添加支持
if(realsense2_FOUND)
    message(STATUS "Adding RealSense2 support")
    target_include_directories(${PROJECT_NAME} PRIVATE ${realsense2_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${realsense2_LIBRARY})
    
    # 添加 RealSense 定义
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_REALSENSE=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_REALSENSE=0)
    message(STATUS "RealSense2 not found - skipping RealSense examples")
endif()

# 创建示例程序的函数
function(add_orb_slam3_example example_name example_source)
    add_executable(${example_name} ${example_source})
    target_link_libraries(${example_name} ${PROJECT_NAME})
    
    # 设置可执行文件的输出目录
    set_target_properties(${example_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
    )
    
    # 复制词汇表文件（如果存在）
    if(EXISTS "${PROJECT_SOURCE_DIR}/Vocabulary/ORBvoc.txt")
        add_custom_command(TARGET ${example_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/Vocabulary/ORBvoc.txt"
            $<TARGET_FILE_DIR:${example_name}>
            COMMENT "Copying ORB vocabulary file"
        )
    endif()
endfunction()

# 构建示例程序
message(STATUS "Building examples...")

# RGB-D 示例
add_orb_slam3_example(rgbd_tum Examples/RGB-D/rgbd_tum.cc)
if(realsense2_FOUND)
    add_orb_slam3_example(rgbd_realsense_D435i Examples/RGB-D/rgbd_realsense_D435i.cc)
endif()

# RGB-D Inertial 示例
if(realsense2_FOUND)
    add_orb_slam3_example(rgbd_inertial_realsense_D435i Examples/RGB-D-Inertial/rgbd_inertial_realsense_D435i.cc)
endif()

# Stereo 示例
add_orb_slam3_example(stereo_kitti Examples/Stereo/stereo_kitti.cc)
add_orb_slam3_example(stereo_euroc Examples/Stereo/stereo_euroc.cc)
add_orb_slam3_example(stereo_tum_vi Examples/Stereo/stereo_tum_vi.cc)

if(realsense2_FOUND)
    add_orb_slam3_example(stereo_realsense_t265 Examples/Stereo/stereo_realsense_t265.cc)
    add_orb_slam3_example(stereo_realsense_D435i Examples/Stereo/stereo_realsense_D435i.cc)
endif()

# Monocular 示例
add_orb_slam3_example(mono_tum Examples/Monocular/mono_tum.cc)
add_orb_slam3_example(mono_kitti Examples/Monocular/mono_kitti.cc)
add_orb_slam3_example(mono_euroc Examples/Monocular/mono_euroc.cc)
add_orb_slam3_example(mono_tum_vi Examples/Monocular/mono_tum_vi.cc)

if(realsense2_FOUND)
    add_orb_slam3_example(mono_realsense_t265 Examples/Monocular/mono_realsense_t265.cc)
    add_orb_slam3_example(mono_realsense_D435i Examples/Monocular/mono_realsense_D435i.cc)
endif()

# Monocular Inertial 示例
add_orb_slam3_example(mono_inertial_euroc Examples/Monocular-Inertial/mono_inertial_euroc.cc)
add_orb_slam3_example(mono_inertial_tum_vi Examples/Monocular-Inertial/mono_inertial_tum_vi.cc)

if(realsense2_FOUND)
    add_orb_slam3_example(mono_inertial_realsense_t265 Examples/Monocular-Inertial/mono_inertial_realsense_t265.cc)
    add_orb_slam3_example(mono_inertial_realsense_D435i Examples/Monocular-Inertial/mono_inertial_realsense_D435i.cc)
endif()

# Stereo Inertial 示例
add_orb_slam3_example(stereo_inertial_euroc Examples/Stereo-Inertial/stereo_inertial_euroc.cc)
add_orb_slam3_example(stereo_inertial_tum_vi Examples/Stereo-Inertial/stereo_inertial_tum_vi.cc)

if(realsense2_FOUND)
    add_orb_slam3_example(stereo_inertial_realsense_t265 Examples/Stereo-Inertial/stereo_inertial_realsense_t265.cc)
    add_orb_slam3_example(stereo_inertial_realsense_D435i Examples/Stereo-Inertial/stereo_inertial_realsense_D435i.cc)
endif()

# Calibration 示例
if(realsense2_FOUND)
    add_orb_slam3_example(recorder_realsense_D435i Examples/Calibration/recorder_realsense_D435i.cc)
    add_orb_slam3_example(recorder_realsense_T265 Examples/Calibration/recorder_realsense_T265.cc)
endif()

# 旧版本示例（可选构建）
option(BUILD_OLD_EXAMPLES "Build old version examples" OFF)

if(BUILD_OLD_EXAMPLES)
    message(STATUS "Building old examples...")
    
    # 这里可以添加旧版本示例的构建逻辑
    # 为了简洁，这里只展示一个示例
    add_orb_slam3_example(rgbd_tum_old Examples_old/RGB-D/rgbd_tum.cc)
    
    # 可以继续添加其他旧版本示例...
endif()

# 安装目标（可选）
if(WIN32)
    set(CMAKE_INSTALL_BINDIR bin)
    set(CMAKE_INSTALL_LIBDIR lib)
else()
    set(CMAKE_INSTALL_BINDIR bin)
    set(CMAKE_INSTALL_LIBDIR lib)
endif()

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

# 总结信息
message(STATUS "")
message(STATUS "=== ORB_SLAM3 Build Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Library output: ${LIBRARY_OUTPUT_PATH}")
message(STATUS "Binary output: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "RealSense support: ${realsense2_FOUND}")
message(STATUS "Old examples: ${BUILD_OLD_EXAMPLES}")
message(STATUS "=================================")

# cmake .. -DCMAKE_TOOLCHAIN_FILE=E:/3rd_party/vcpkg/scripts/buildsystems/vcpkg.cmake -DGLEW_ROOT_DIR=E:/3rd_party/vcpkg/installed/x64-windows -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF -DBUILD_TOOLS=OFF -DCMAKE_INSTALL_PREFIX=E:/3rd_party/Pangolin/install -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

#  cmake .. -DCMAKE_TOOLCHAIN_FILE="E:/3rd_party/vcpkg/scripts/buildsystems/vcpkg.cmake"                              