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
endif()# 在 MSVC 编译选项部分添加

if(MSVC)
    # 禁用严格的分配器检查
    add_definitions(-D_ENFORCE_MATCHING_ALLOCATORS=0)
    add_compile_options(/Zc:allocatorCompat-)
    
    # 强制单线程编译，避免 PDB 冲突
    add_compile_options(/MP1)
    add_compile_options(/FS)
    
    # 或者禁用调试信息（如果不需要调试）
    # add_compile_options(/Z7)  # 使用旧式调试信息
    
    # 禁用特定警告
    add_compile_options(/wd4244)  # 类型转换警告
    add_compile_options(/wd4267)  # size_t 转换警告
    add_compile_options(/wd4996)  # 安全警告
endif()

# 为每个源文件设置独立的 PDB
if(MSVC)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "Embedded")
    set(CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
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
set(OpenCV_DIR           "E:/3rd_party/opencv/build/x64/vc16/lib")
# set(OpenCV_DIR           "E:/3rd_party/opencv/build/")
set(EIGEN3_INCLUDE_DIR   "E:/3rd_party/eigen-3.4.0/include/eigen3")
set(Pangolin_DIR         "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")
# set(BOOST_ROOT           "E:/3rd_party/boost_1_83_0")
set(BOOST_ROOT           "E:/3rd_party/boost_1_83_0/lib/cmake/Boost-1.83.0")

# 若 Pangolin 是以 GLEW 构建的，下面的 GLEW_ROOT 可用于 FindGLEW（如需要）
set(GLEW_ROOT            "E:/3rd_party/glew-2.2.0")

#==============================================================================
#=== 2. 查找第三方依赖（顺序很重要：先 Threads，后 Pangolin） ==================
#==============================================================================

#=== 2.1 OpenCV（>=4） ===
# find_package(OpenCV 4 REQUIRED)
# set(OpenCV_ROOT "D:/CODE-STUDY/_installed/opencv-4.x/x64/vc17/lib")
message("OpenCV_ROOT is " ${OpenCV_ROOT})
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
  message(STATUS "OpenCV version: ${OpenCV_VERSION}")
  message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")
else()
  message(FATAL_ERROR "OpenCV not found.")
endif()




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
message(STATUS "配置 Boost 库...")

# 设置 Boost 根目录
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")
set(Boost_ROOT "E:/3rd_party/boost_1_83_0")

# 首先尝试自动查找
find_package(Boost QUIET COMPONENTS serialization)

if(NOT Boost_FOUND)
    # 如果自动查找失败，手动配置
    message(STATUS "自动查找失败，使用手动配置...")
    
    # 手动设置包含目录
    set(BOOST_INCLUDE_DIR "E:/3rd_party/boost_1_83_0")
    set(Boost_INCLUDE_DIRS "${BOOST_INCLUDE_DIR}")
    
    # 验证头文件存在
    if(EXISTS "${BOOST_INCLUDE_DIR}/boost/serialization/serialization.hpp")
        message(STATUS "✓ Boost 头文件目录: ${BOOST_INCLUDE_DIR}")
        
        # 手动查找库文件
        set(BOOST_LIB_PATHS
            "${BOOST_ROOT}/lib64-msvc-14.3"
            "${BOOST_ROOT}/stage/lib"
            "${BOOST_ROOT}/lib"
        )
        
        find_library(Boost_SERIALIZATION_LIBRARY
            NAMES 
                boost_serialization
                boost_serialization-vc143-mt-x64-1_83
                boost_serialization-vc143-mt-x64-1_83.lib
                libboost_serialization
            PATHS ${BOOST_LIB_PATHS}
            NO_DEFAULT_PATH
        )
        
        if(Boost_SERIALIZATION_LIBRARY)
            message(STATUS "✓ 找到 Boost 序列化库: ${Boost_SERIALIZATION_LIBRARY}")
            set(Boost_SERIALIZATION_FOUND TRUE)
            set(Boost_LIBRARIES "${Boost_SERIALIZATION_LIBRARY}")
        else()
            message(STATUS "⚠ 未找到 Boost 序列化库，使用仅头文件模式")
            set(Boost_SERIALIZATION_FOUND FALSE)
            set(Boost_LIBRARIES "")
        endif()
        
        set(Boost_FOUND TRUE)
        set(Boost_VERSION 1.83.0)
    else()
        message(FATAL_ERROR "✗ Boost 头文件未找到: ${BOOST_INCLUDE_DIR}/boost/serialization/serialization.hpp")
    endif()
else()
    message(STATUS "✓ Boost 自动查找成功: ${Boost_VERSION}")
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
    OpenGL::GL
    $<$<TARGET_EXISTS:GLEW::GLEW>:GLEW::GLEW>
    ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/Release/DBoW2.lib
    ${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/Release/g2o.lib
)
#==============================================================================
#=== 8. 示例程序（分别设置各自的输出目录） =====================================
#==============================================================================

#--- 8.1 RGB-D -----------------------------------------------------------------
add_executable(rgbd_tum Examples/RGB-D/rgbd_tum.cc)
target_include_directories(rgbd_tum PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(rgbd_tum PRIVATE ${PROJECT_NAME})
set_target_properties(rgbd_tum PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/RGB-D"
)

#--- 8.2 Stereo ----------------------------------------------------------------
add_executable(stereo_kitti Examples/Stereo/stereo_kitti.cc)
target_include_directories(stereo_kitti PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(stereo_kitti PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_kitti PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo"
)

add_executable(stereo_euroc Examples/Stereo/stereo_euroc.cc)
target_include_directories(stereo_euroc PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(stereo_euroc PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_euroc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo"
)

add_executable(stereo_tum_vi Examples/Stereo/stereo_tum_vi.cc)
target_include_directories(stereo_tum_vi PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(stereo_tum_vi PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_tum_vi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo"
)

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

add_executable(mono_kitti Examples/Monocular/mono_kitti.cc)
target_include_directories(mono_kitti PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(mono_kitti PRIVATE ${PROJECT_NAME})
set_target_properties(mono_kitti PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
)

add_executable(mono_euroc Examples/Monocular/mono_euroc.cc)
target_include_directories(mono_euroc PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(mono_euroc PRIVATE ${PROJECT_NAME})
set_target_properties(mono_euroc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
)

add_executable(mono_tum_vi Examples/Monocular/mono_tum_vi.cc)
target_include_directories(mono_tum_vi PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(mono_tum_vi PRIVATE ${PROJECT_NAME})
set_target_properties(mono_tum_vi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
)

#--- 8.4 Monocular-Inertial ----------------------------------------------------
add_executable(mono_inertial_euroc Examples/Monocular-Inertial/mono_inertial_euroc.cc)
target_include_directories(mono_inertial_euroc PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(mono_inertial_euroc PRIVATE ${PROJECT_NAME})
set_target_properties(mono_inertial_euroc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular-Inertial"
)

add_executable(mono_inertial_tum_vi Examples/Monocular-Inertial/mono_inertial_tum_vi.cc)
target_include_directories(mono_inertial_tum_vi PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(mono_inertial_tum_vi PRIVATE ${PROJECT_NAME})
set_target_properties(mono_inertial_tum_vi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular-Inertial"
)

#--- 8.5 Stereo-Inertial -------------------------------------------------------
add_executable(stereo_inertial_euroc Examples/Stereo-Inertial/stereo_inertial_euroc.cc)
target_include_directories(stereo_inertial_euroc PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(stereo_inertial_euroc PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_inertial_euroc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo-Inertial"
)

add_executable(stereo_inertial_tum_vi Examples/Stereo-Inertial/stereo_inertial_tum_vi.cc)
target_include_directories(stereo_inertial_tum_vi PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${OpenCV_INCLUDE_DIRS}
)
target_link_libraries(stereo_inertial_tum_vi PRIVATE ${PROJECT_NAME})
set_target_properties(stereo_inertial_tum_vi PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Stereo-Inertial"
)

#==============================================================================


