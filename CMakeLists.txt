cmake_minimum_required(VERSION 3.20)

#=============================================================================
#=== 0. 工程与全局设置 =========================================================
#==============================================================================
project(ORB_SLAM3 CXX)

#=== 0.1 MSVC 运行库策略（静态/动态多线程） ===
if(MSVC)
  # 允许用 CMAKE_MSVC_RUNTIME_LIBRARY 控制运行库（/MT, /MD 等）
  if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
  endif()
  # 与你原先设置一致：Release 用 /MT，Debug 用 /MTd
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

#=== 0.2 构建类型（默认 Release） ===
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#=== 0.3 C++ 标准 ===
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# MSVC 特定设置
if(MSVC)
    # 禁用严格的分配器检查
    add_definitions(-D_ENFORCE_MATCHING_ALLOCATORS=0)
    add_compile_options(/Zc:allocatorCompat-)
    
    # 禁用特定警告
    add_compile_options(/wd4244)  # 类型转换警告
    add_compile_options(/wd4267)  # size_t 转换警告
    add_compile_options(/wd4996)  # 安全警告
    
    # 设置运行时库（根据你的选择）
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
#=== 0.4 额外编译选项与宏 ===
if(MSVC)
  add_compile_options(/W0 /bigobj)
  add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
else()
  add_compile_options(-O3 -Wno-deprecated)
endif()

#=== 0.5 cmake 模块路径（如有自定义 FindXXX.cmake 放在此处） ===
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

#==============================================================================
#=== 1. 第三方依赖路径（按需修改为你的实际路径） ================================
#==============================================================================
# 说明：OpenCV/Eigen/Pangolin 若已正确安装到注册的 CMake 路径，可不手动设置 *_DIR。
set(OpenCV_DIR           "E:/3rd_party/opencv/build")
set(EIGEN3_INCLUDE_DIR   "E:/3rd_party/eigen-3.4.0/include/eigen3")
set(Pangolin_DIR         "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")
set(BOOST_ROOT           "E:/3rd_party/boost_1_83_0")
# 若 Pangolin 是以 GLEW 构建的，下面的 GLEW_ROOT 可用于 FindGLEW（如需要）
set(GLEW_ROOT            "E:/3rd_party/glew-2.2.0")

#==============================================================================
#=== 2. 查找第三方依赖（顺序很重要：先 Threads，后 Pangolin） ==================
#==============================================================================

#=== 2.1 OpenCV（>=4） ===
find_package(OpenCV 4 REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")

#=== 2.2 Eigen3（>=3.4.0） ===
find_package(Eigen3 3.4.0 REQUIRED)
# Compat: 保留旧式 include 路径，亦可直接用 Eigen3::Eigen 目标
if(EIGEN3_INCLUDE_DIR)
  message(STATUS "Eigen3 include (manual): ${EIGEN3_INCLUDE_DIR}")
endif()

#=== 2.3 线程库（必须在 Pangolin 之前，以提供 Threads::Threads 目标） ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#=== 2.4 Boost（序列化等） ===
#--- 10.5 Boost （手动配置序列化库，仅做头+库路径检查） -----------------------
message(STATUS "配置 Boost 库...")
set(BOOST_INCLUDE_DIR "${BOOST_ROOT}")
if(NOT EXISTS "${BOOST_INCLUDE_DIR}/boost/serialization/serialization.hpp")
    message(FATAL_ERROR "✗ Boost 头文件未找到，请检查路径: ${BOOST_INCLUDE_DIR}")
else()
    message(STATUS "✓ Boost 头文件目录: ${BOOST_INCLUDE_DIR}")
endif()

set(BOOST_LIB_PATHS
    "${BOOST_ROOT}/stage/lib"
    "${BOOST_ROOT}/lib"
    "${BOOST_ROOT}/lib64-msvc-14.3"
)
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
find_library(BOOST_SERIALIZATION_LIB
    NAMES ${BOOST_SERIALIZATION_NAMES}
    PATHS ${BOOST_LIB_PATHS}
    NO_DEFAULT_PATH
)
if(BOOST_SERIALIZATION_LIB)
    message(STATUS "✓ 找到 Boost 序列化库: ${BOOST_SERIALIZATION_LIB}")
    add_library(Boost_serialization INTERFACE IMPORTED)
    set_target_properties(Boost_serialization PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${BOOST_SERIALIZATION_LIB}"
    )
    set(BOOST_SERIALIZATION_TARGET Boost_serialization)
    set(Boost_FOUND TRUE)
    set(Boost_SERIALIZATION_FOUND TRUE)
else()
    message(WARNING "✗ 未找到 Boost 序列化库文件，尝试仅使用头文件模式")
    add_library(Boost_serialization INTERFACE IMPORTED)
    set_target_properties(Boost_serialization PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
    )
    set(BOOST_SERIALIZATION_TARGET Boost_serialization)
    set(Boost_FOUND TRUE)
endif()

#=== 2.5 OpenGL / GLEW（Pangolin 常见依赖，尽量查找，有则链接） ===
find_package(OpenGL REQUIRED)
# GLEW 不一定总需要；QUIET 查找，找到就链。
find_package(GLEW QUIET)

#=== 2.6 Pangolin（依赖 Threads::Threads，故放在其后） ===
find_package(Pangolin REQUIRED)
# 兼容两种导出方式：现代目标 Pangolin::pangolin 或传统变量 ${Pangolin_LIBRARIES}
set(PANGOLIN_TARGET "")
if(TARGET Pangolin::pangolin)
  set(PANGOLIN_TARGET Pangolin::pangolin)
  message(STATUS "Pangolin target: Pangolin::pangolin")
elseif(DEFINED Pangolin_LIBRARIES)
  set(PANGOLIN_TARGET ${Pangolin_LIBRARIES})
  message(STATUS "Pangolin libraries: ${Pangolin_LIBRARIES}")
else()
  message(FATAL_ERROR "Pangolin not providing a known target/variable.")
endif()

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
  ${Boost_INCLUDE_DIRS}
)

#==============================================================================
#=== 7. 链接依赖 ===============================================================
#==============================================================================
# 注：DBoW2 / g2o 目前写死了 Release 路径，请用 Release 配置编译本项目。
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${OpenCV_LIBS}
    ${PANGOLIN_TARGET}
    Threads::Threads
    Eigen3::Eigen
    ${BOOST_SERIALIZATION_TARGET}
    OpenGL::GL

    # GLEW 可选：找到才链接
    $<$<TARGET_EXISTS:GLEW::GLEW>:GLEW::GLEW>

    # 第三方静态库（请确保与本工程配置/运行库一致）
    ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/Release/DBoW2.lib
    ${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/Release/g2o.lib
)

#==============================================================================
#=== 8. 示例程序（分别设置各自的输出目录） =====================================
#==============================================================================

#--- 8.1 RGB-D -----------------------------------------------------------------
add_executable(rgbd_tum Examples/RGB-D/rgbd_tum.cc)
target_link_libraries(rgbd_tum PRIVATE ${PROJECT_NAME})
set_target_properties(rgbd_tum PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/RGB-D"
)

#--- 8.2 Stereo ----------------------------------------------------------------
add_executable(stereo_kitti Examples/Stereo/stereo_kitti.cc)
target_link_libraries(stereo_kitti PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_kitti PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo"
)

add_executable(stereo_euroc Examples/Stereo/stereo_euroc.cc)
target_link_libraries(stereo_euroc PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_euroc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo"
)

add_executable(stereo_tum_vi Examples/Stereo/stereo_tum_vi.cc)
target_link_libraries(stereo_tum_vi PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_tum_vi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo"
)

#--- 8.3 Monocular --------------------------------------------------------------
add_executable(mono_tum Examples/Monocular/mono_tum.cc)
target_link_libraries(mono_tum PRIVATE ${PROJECT_NAME})
set_target_properties(mono_tum PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
)

add_executable(mono_kitti Examples/Monocular/mono_kitti.cc)
target_link_libraries(mono_kitti PRIVATE ${PROJECT_NAME})
set_target_properties(mono_kitti PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
)

add_executable(mono_euroc Examples/Monocular/mono_euroc.cc)
target_link_libraries(mono_euroc PRIVATE ${PROJECT_NAME})
set_target_properties(mono_euroc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
)

add_executable(mono_tum_vi Examples/Monocular/mono_tum_vi.cc)
target_link_libraries(mono_tum_vi PRIVATE ${PROJECT_NAME})
set_target_properties(mono_tum_vi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
)

#--- 8.4 Monocular-Inertial ----------------------------------------------------
add_executable(mono_inertial_euroc Examples/Monocular-Inertial/mono_inertial_euroc.cc)
target_link_libraries(mono_inertial_euroc PRIVATE ${PROJECT_NAME})
set_target_properties(mono_inertial_euroc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular-Inertial"
)

add_executable(mono_inertial_tum_vi Examples/Monocular-Inertial/mono_inertial_tum_vi.cc)
target_link_libraries(mono_inertial_tum_vi PRIVATE ${PROJECT_NAME})
set_target_properties(mono_inertial_tum_vi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular-Inertial"
)

#--- 8.5 Stereo-Inertial -------------------------------------------------------
add_executable(stereo_inertial_euroc Examples/Stereo-Inertial/stereo_inertial_euroc.cc)
target_link_libraries(stereo_inertial_euroc PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_inertial_euroc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo-Inertial"
)

add_executable(stereo_inertial_tum_vi Examples/Stereo-Inertial/stereo_inertial_tum_vi.cc)
target_link_libraries(stereo_inertial_tum_vi PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_inertial_tum_vi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo-Inertial"
)

#==============================================================================
#=== 9. 备注与提示 =============================================================
#==============================================================================
# 1) 你已将 OpenCV 升级到 4.x，请将源码里的旧头 <opencv/cv.h> 改为 <opencv2/opencv.hpp>。
# 2) 当前链接的 DBoW2/g2o 为 Release 目录的 .lib，请用 Release 配置构建本工程。
# 3) 运行期请将 E:/3rd_party/opencv/build/x64/vc16/bin 等加入 PATH，避免找不到 DLL。
