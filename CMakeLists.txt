cmake_minimum_required(VERSION 3.20)

#=============================================================================
#=== 0. 工程与全局设置 =========================================================
#==============================================================================
project(ORB_SLAM3 CXX)

#=== 0.1 MSVC 运行库策略 ===
if(MSVC)
    if(POLICY CMP0091)
        cmake_policy(SET CMP0091 NEW)
    endif()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    
    # MSVC 编译选项
    add_definitions(-D_ENFORCE_MATCHING_ALLOCATORS=0)
    add_compile_options(/Zc:allocatorCompat- /MP1 /FS /W0 /bigobj)
    add_compile_options(/wd4244 /wd4267 /wd4996)  # 禁用特定警告
    
    # PDB 设置
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "Embedded")
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
    
    add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
else()
    add_compile_options(-O3 -Wno-deprecated -Wall)
endif()

#=== 0.2 构建类型 ===
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#=== 0.3 C++ 标准 ===
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#=== 0.4 宏定义 ===
add_definitions(-DCOMPILEDWITHC11 -DORB_SLAM3_EXPORTS)

#==============================================================================
#=== 1. 第三方依赖路径配置 =====================================================
#==============================================================================
# 设置依赖库路径
set(OpenCV_DIR "E:/3rd_party/opencv/build/x64/vc16/lib")
set(Pangolin_DIR "E:/3rd_party/pangolin-install/lib/cmake/Pangolin") 
set(GLEW_ROOT "E:/3rd_party/glew-2.2.0")
set(Boost_ROOT "E:/3rd_party/boost_1_83_0")

#==============================================================================
#=== 2. 查找第三方依赖 =========================================================
#==============================================================================

#=== 2.1 OpenCV ===
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "✓ OpenCV version: ${OpenCV_VERSION}")
else()
    message(FATAL_ERROR "✗ OpenCV not found")
endif()

#=== 2.2 Eigen3 ===
message(STATUS "配置 Eigen3...")
list(APPEND CMAKE_PREFIX_PATH "E:/3rd_party/eigen-3.4.0-install/share/eigen3/cmake")
find_package(Eigen3 3.4.0 REQUIRED)
if(Eigen3_FOUND)
    message(STATUS "✓ Eigen3 version: ${Eigen3_VERSION}")
else()
    message(FATAL_ERROR "✗ Eigen3 not found")
endif()

#=== 2.3 线程库 ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#=== 2.4 Boost（序列化等） ===
message(STATUS "配置 Boost 库...")
# 设置 Boost 根目录
set(Boost_ROOT "E:/3rd_party/boost_1_83_0")


# 手动指定库文件
set(BOOST_LIBRARY_DIR "E:/3rd_party/boost_1_83_0/lib64-msvc-14.3")
# 手动设置库文件路径
set(BOOST_SERIALIZATION_LIB 
    "${BOOST_LIBRARY_DIR}/boost_serialization-vc143-mt-x64-1_83.lib"
)
# 验证库文件存在
if(EXISTS "${BOOST_SERIALIZATION_LIB}")
    message(STATUS "✓ Boost 序列化库: ${BOOST_SERIALIZATION_LIB}")
    set(Boost_SERIALIZATION_LIBRARY "${BOOST_SERIALIZATION_LIB}")
    set(Boost_SERIALIZATION_FOUND TRUE)
else()
    message(FATAL_ERROR "✗ Boost 序列化库未找到: ${BOOST_SERIALIZATION_LIB}")
endif()

set(Boost_FOUND TRUE)
set(Boost_INCLUDE_DIRS "E:/3rd_party/boost_1_83_0")
set(Boost_LIBRARIES "${Boost_SERIALIZATION_LIBRARY}")
message(STATUS "✓ Boost 配置完成")

#=== 2.5 GLEW ===
find_package(GLEW QUIET)

#=== 2.6 Pangolin ===
set(PANGOLIN_WITH_EIGEN OFF)  # 避免与手动配置的 Eigen 冲突
find_package(Pangolin REQUIRED)

# 确定 Pangolin 目标
if(TARGET Pangolin::pangolin)
    set(PANGOLIN_TARGET Pangolin::pangolin)
elseif(TARGET pangolin)
    set(PANGOLIN_TARGET pangolin)
else()
    set(PANGOLIN_TARGET ${Pangolin_LIBRARIES})
endif()
message(STATUS "✓ Pangolin 配置完成")


#==============================================================================
#=== 3. 头文件包含路径 =========================================================
#==============================================================================
# 仅添加必要的 include；OpenCV/Eigen/Boost/GL/GLEW/Pangolin 多由目标传递，但保守起见补充
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/CameraModels
  ${EIGEN3_INCLUDE_DIR}          # 兼容你的手动路径
)

# Pangolin 有些包会提供 Pangolin_INCLUDE_DIRS 变量，存在则补充
if(DEFINED Pangolin_INCLUDE_DIRS)
  include_directories(${Pangolin_INCLUDE_DIRS})
endif()

#==============================================================================
#=== 4. 输出目录（库放 lib/） ==================================================
#==============================================================================
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

#==============================================================================
#=== 5. 源码与目标 =============================================================
#==============================================================================
add_library(${PROJECT_NAME} SHARED
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
  src/PnPsolver.cc
  src/Frame.cc
  src/KeyFrameDatabase.cc
  src/Sim3Solver.cc
  src/Initializer.cc
  src/Viewer.cc
  src/ImuTypes.cc
  src/G2oTypes.cc
  src/CameraModels/Pinhole.cpp
  src/CameraModels/KannalaBrandt8.cpp
  src/OptimizableTypes.cpp
  src/MLPnPsolver.cpp
  src/TwoViewReconstruction.cc

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
  include/PnPsolver.h
  include/Frame.h
  include/KeyFrameDatabase.h
  include/Sim3Solver.h
  include/Initializer.h
  include/Viewer.h
  include/ImuTypes.h
  include/G2oTypes.h
  include/CameraModels/GeometricCamera.h
  include/CameraModels/Pinhole.h
  include/CameraModels/KannalaBrandt8.h
  include/OptimizableTypes.h
  include/MLPnPsolver.h
  include/TwoViewReconstruction.h
)

#==============================================================================
#=== 6. 目标级包含目录（优先使用“现代 CMake”传递） ============================
#==============================================================================
target_include_directories(${PROJECT_NAME} PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  # E:/3rd_party/opencv/build/include  # 手动指定 OpenCV 包含目录
  ${Boost_INCLUDE_DIRS}
)

#==============================================================================
#=== 7. 链接依赖 ===============================================================
#==============================================================================
# 注：DBoW2 / g2o 目前写死了 Release 路径，请用 Release 配置编译本项目。
# 删除这一行（第252行）：
# target_link_libraries(${PROJECT_NAME} opencv_core opencv_imgproc opencv_highgui opencv_features2d opencv_calib3d)

# 保留这一部分（第254-269行）：
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${OpenCV_LIBRARIES}
    # OpenCV 核心库
    # E:/3rd_party/opencv/build/x64/vc16/lib/opencv_world4100.lib

    ${PANGOLIN_TARGET}
    Threads::Threads
    Eigen3::Eigen
    ${BOOST_SERIALIZATION_TARGET}
    # OpenGL::GL
    $<$<TARGET_EXISTS:GLEW::GLEW>:GLEW::GLEW>
    ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/Release/DBoW2.lib
    ${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/Release/g2o.lib
)
#==============================================================================
#=== 8. 示例程序（分别设置各自的输出目录） =====================================
#==============================================================================

#--- 8.3 Monocular --------------------------------------------------------------
add_executable(mono_tum Examples/Monocular/mono_tum.cc)
target_include_directories(mono_tum PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)
target_link_libraries(mono_tum PRIVATE ${PROJECT_NAME})
set_target_properties(mono_tum PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
)

# add_executable(mono_kitti Examples/Monocular/mono_kitti.cc)
# target_include_directories(mono_kitti PRIVATE 
#     ${PROJECT_SOURCE_DIR}
#     ${PROJECT_SOURCE_DIR}/include
#     ${OpenCV_INCLUDE_DIRS}
# )
# target_link_libraries(mono_kitti PRIVATE ${PROJECT_NAME})
# set_target_properties(mono_kitti PROPERTIES
#   RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
# )
