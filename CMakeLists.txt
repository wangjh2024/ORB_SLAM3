cmake_minimum_required(VERSION 3.20)

#=============================================================================
#=== 0. 工程与全局设置（仅 Release） ==========================================
#=============================================================================
project(ORB_SLAM3 CXX)

#--- 0.1 仅启用 Release 配置（VS 多配置生成器也只保留 Release） ----------------
if(CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "Release" CACHE STRING "" FORCE)
else()
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()
message(STATUS "Build type (effective): ${CMAKE_BUILD_TYPE}${CMAKE_CONFIGURATION_TYPES}")

#--- 0.2 MSVC 编译策略与兼容性策略 --------------------------------------------
if(MSVC)
  # 使用 /MD 运行库（仅 Release）
  if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
  endif()
  # 继续支持老的 FindBoost 模块
  if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
  endif()
  # 使用 <PACKAGENAME>_ROOT 风格的变量
  if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
  endif()

  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

  # 编译选项（适度保守 & 提升编译速度）
  add_definitions(-D_ENFORCE_MATCHING_ALLOCATORS=0)
  add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES)
  add_compile_options(/MP /W0 /bigobj /EHsc)
  add_compile_options(/wd4244 /wd4267 /wd4996)
else()
  add_compile_options(-O3 -Wno-deprecated -Wall)
endif()

#--- 0.3 C++ 标准 --------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#--- 0.4 宏定义 ---------------------------------------------------------------
add_definitions(-DCOMPILEDWITHC11 -DORB_SLAM3_EXPORTS)

#=============================================================================
#=== 1. 第三方依赖路径配置 ====================================================
#=============================================================================
# 你本地依赖的安装路径（按需修改）
set(OpenCV_DIR    "E:/3rd_party/opencv/build/x64/vc16/lib")
set(Pangolin_DIR  "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")
set(GLEW_ROOT     "E:/3rd_party/glew-2.2.0")

# Boost 安装前缀（建议指向包含 lib/cmake/Boost-1.83.0 的目录）
# 如：E:/3rd_party/boost_1_83_0 或 E:/3rd_party/boost_1_83_0/install
# 可按需启用下一行，或在命令行 -DBoost_ROOT=... 传入
# set(Boost_ROOT "E:/3rd_party/boost_1_83_0")

#=============================================================================
#=== 2. 查找第三方依赖 ========================================================
#=============================================================================

#=== 2.1 OpenCV ===
find_package(OpenCV REQUIRED)
message(STATUS "✓ OpenCV version: ${OpenCV_VERSION}")

#=== 2.2 Eigen3 ===
message(STATUS "配置 Eigen3...")
list(APPEND CMAKE_PREFIX_PATH "E:/3rd_party/eigen-3.4.0-install/share/eigen3/cmake")
find_package(Eigen3 3.4.0 REQUIRED NO_MODULE)
message(STATUS "✓ Eigen3 version: ${Eigen3_VERSION}")

#=== 2.3 线程库（Windows 不使用 pthread，日志提示可忽略） ===
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

message(STATUS "✓ Threads available")

#=== 2.4 Boost（使用 FindBoost 现代目标） ===
#=== 2.4 Boost（仅 Release，手动连接） ===
message(STATUS "配置 Boost 库（手动）...")

# 1) 头文件根目录（确保这里有 boost/ 子目录）
set(BOOST_ROOT_DIR "E:/3rd_party/boost_1_83_0")
include_directories("${BOOST_ROOT_DIR}")

# 2) 库目录（确保这里有 libboost_serialization-*.lib）
set(BOOST_LIB_DIR  "E:/3rd_party/boost_1_83_0/lib64-msvc-14.3")

# 3) 指定需要链接的库（只考虑 Release，/MD 变体通常是 -vc143-mt-x64-1_83）
#    若你的实际文件名不同，请把下面这行改为你本机实际文件名
set(BOOST_SERIALIZATION_LIB
    "${BOOST_LIB_DIR}/libboost_serialization-vc143-mt-x64-1_83.lib")

# 4) 防止 MSVC 下 Boost 的 auto-link（/DEFAULTLIB）与我们手动链接冲突
add_compile_definitions(BOOST_ALL_NO_LIB)

# 5) 基本存在性检查（友好报错）
if(NOT EXISTS "${BOOST_ROOT_DIR}/boost/version.hpp")
  message(FATAL_ERROR "未找到 Boost 头文件，请检查 BOOST_ROOT_DIR（当前：${BOOST_ROOT_DIR}）")
endif()
if(NOT EXISTS "${BOOST_SERIALIZATION_LIB}")
  message(FATAL_ERROR "未找到 Boost.Serialization 库：${BOOST_SERIALIZATION_LIB}\n请确认文件名与路径、工具集（vc143）、位宽（x64）与版本（1_83）一致。")
endif()

message(STATUS "✓ Boost headers : ${BOOST_ROOT_DIR}")
message(STATUS "✓ Boost lib dir : ${BOOST_LIB_DIR}")
message(STATUS "✓ Boost link    : ${BOOST_SERIALIZATION_LIB}")


#=== 2.5 GLEW（可选） ===
find_package(GLEW QUIET)

#=== 2.6 Pangolin ===
set(PANGOLIN_WITH_EIGEN OFF)  # 避免与手动配置的 Eigen 冲突
find_package(Pangolin REQUIRED)

# 统一 Pangolin 目标名
if(TARGET Pangolin::pangolin)
  set(PANGOLIN_TARGET Pangolin::pangolin)
elseif(TARGET pangolin)
  set(PANGOLIN_TARGET pangolin)
else()
  set(PANGOLIN_TARGET ${Pangolin_LIBRARIES})
endif()
message(STATUS "✓ Pangolin 配置完成")

#=============================================================================
#=== 3. 头文件包含路径（保守补充） ============================================
#=============================================================================
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/CameraModels
  ${EIGEN3_INCLUDE_DIR}      # 兼容你的手动路径变量
)

if(DEFINED Pangolin_INCLUDE_DIRS)
  include_directories(${Pangolin_INCLUDE_DIRS})
endif()
if(DEFINED Boost_INCLUDE_DIRS)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

#=============================================================================
#=== 4. 输出目录（库默认放到 lib/） ===========================================
#=============================================================================
# 对于 SHARED 库
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# 对于静态库（如有）
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
# 对于可执行文件由其各自设置输出目录

#=============================================================================
#=== 5. 源码与目标（先创建再链接） ============================================
#=============================================================================
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

#=============================================================================
#=== 6. 目标级包含目录（现代 CMake 传递） =====================================
#=============================================================================
target_include_directories(${PROJECT_NAME} PRIVATE
  ${OpenCV_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)

#=============================================================================
#=== 7. 链接依赖（全部在 add_library 之后进行） ===============================
#=============================================================================
#=== 7. 链接依赖（全部在 add_library 之后进行） ===============================
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${OpenCV_LIBRARIES}
    ${PANGOLIN_TARGET}
    Threads::Threads
    Eigen3::Eigen
    "${BOOST_SERIALIZATION_LIB}"                   # ← 手动链接 Boost.Serialization（Release）
    $<$<TARGET_EXISTS:GLEW::GLEW>:GLEW::GLEW>
    ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/Release/DBoW2.lib
    ${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/Release/g2o.lib
)

#=============================================================================
#=== 8. 示例程序（Monocular） ==================================================
#=============================================================================
#=== 8.1 mono_tum --------------------------------------------------------------
add_executable(mono_tum Examples/Monocular/mono_tum.cc)
target_include_directories(mono_tum PRIVATE 
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${OpenCV_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
)
target_link_libraries(mono_tum PRIVATE
  ${PROJECT_NAME}
  "${BOOST_SERIALIZATION_LIB}"                   # ← 手动链接 Boost.Serialization（Release）
)

# 仅 Release 输出目录
set_target_properties(mono_tum PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/Examples/Monocular"
)

#=============================================================================
#=== 9. 结束语 =================================================================
#=============================================================================
message(STATUS "✓ CMake 配置完成（Release-only, MSVC /MD）")
