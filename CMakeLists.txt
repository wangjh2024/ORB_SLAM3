cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3)

# 针对MSVC的特定设置
if(MSVC)
    # 提高编译器标准
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # 禁用特定警告
    add_compile_options(/wd4996 /wd4820 /wd4464 /wd4244)
    
    # Eigen相关设置
    add_definitions(-DEIGEN_MAKE_ALIGNED_OPERATOR_NEW)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

#=== 1. 构建配置 ===
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
cmake_policy(SET CMP0167 NEW)
cmake_policy(SET CMP0144 NEW)

#=== 2. 编译定义与选项 ===
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
add_definitions(-D_HAS_STD_BYTE=0 -DNOMINMAX -DCOMPILEDWITHC11 -DORB_SLAM3_EXPORTS)
#=== Windows 平台宏定义，防止 APIENTRY 与 min/max 冲突 ===
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

#=== 3. OpenCV ===
set(OpenCV_DIR "E:/3rd_party/opencv/build/x64/vc16/lib")
find_package(OpenCV REQUIRED)
message("OpenCV version: ${OpenCV_VERSION}")

#=== 4. Eigen3 & Threads ===
set(EIGEN3_INCLUDE_DIR "E:/3rd_party/eigen-3.4.0-install/include/eigen3")
include_directories(${EIGEN3_INCLUDE_DIR})
find_package(Eigen3 REQUIRED)

find_package(Threads REQUIRED)

#=== 5. Pangolin ===
set(Pangolin_DIR "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")
set(Pangolin_INCLUDE_DIRS "E:/3rd_party/pangolin-install/include")

set(Pangolin_LIBRARIES
    E:/3rd_party/pangolin-install/lib/pango_core.lib
    E:/3rd_party/pangolin-install/lib/pango_display.lib
    E:/3rd_party/pangolin-install/lib/pango_opengl.lib
    E:/3rd_party/pangolin-install/lib/pango_geometry.lib
    E:/3rd_party/pangolin-install/lib/pango_image.lib
    E:/3rd_party/pangolin-install/lib/pango_tools.lib
    E:/3rd_party/pangolin-install/lib/pango_vars.lib
    E:/3rd_party/pangolin-install/lib/pango_windowing.lib
    E:/3rd_party/pangolin-install/lib/pango_scene.lib
)

if(NOT TARGET Pangolin::pangolin)
    message(STATUS "Manually creating Pangolin imported target...")
    add_library(Pangolin::pangolin INTERFACE IMPORTED)
    set_target_properties(Pangolin::pangolin PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Pangolin_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${Pangolin_LIBRARIES}"
    )
endif()

#=== 6. Boost ===
set(BOOST_ROOT "E:/3rd_party/boost17")
find_package(Boost REQUIRED COMPONENTS serialization)
if(Boost_FOUND)
  message(STATUS "Boost Found: ${Boost_VERSION}")
  include_directories(${Boost_INCLUDE_DIRS})
else()
  message(FATAL_ERROR "Boost not found.")
endif()

#=== 7. GLEW ===
set(GLEW_ROOT "E:/3rd_party/glew-2.2.0")
find_package(GLEW REQUIRED)
if(GLEW_FOUND)
  include_directories(${GLEW_INCLUDE_DIRS})
endif()

#=== 8. 头文件路径 ===
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/CameraModels
  ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
  ${EIGEN3_INCLUDE_DIR}
  ${Pangolin_INCLUDE_DIRS}
)

#=== 9. 库输出路径 ===
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

#=== 10. 构建主库 ===
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

#=== 11. 链接依赖 ===
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    Eigen3::Eigen
    Pangolin::pangolin
    ${Boost_LIBRARIES}
    Threads::Threads
    ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib
    ${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/g2o.lib
)

#=== 12. 构建示例 ===
set(EXAMPLES
  RGB-D/rgbd_tum
  Stereo/stereo_kitti
  Stereo/stereo_euroc
  Stereo/stereo_tum_vi
  Monocular/mono_tum
  Monocular/mono_kitti
  Monocular/mono_euroc
  Monocular/mono_tum_vi
  Monocular-Inertial/mono_inertial_euroc
  Monocular-Inertial/mono_inertial_tum_vi
  Stereo-Inertial/stereo_inertial_euroc
  Stereo-Inertial/stereo_inertial_tum_vi
)

foreach(example ${EXAMPLES})
  get_filename_component(dir ${example} DIRECTORY)
  get_filename_component(name ${example} NAME_WE)  # 提取文件名部分
  set(target_name "${name}_exe")                   # 构建合法目标名

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/${dir})
  add_executable(${target_name} Examples/${example}.cc)
  target_link_libraries(${target_name} ${PROJECT_NAME})
endforeach()


#=== 13. 旧版本示例 ===
set(EXAMPLES_OLD
  RGB-D/rgbd_tum
  Stereo/stereo_kitti
  Stereo/stereo_euroc
  Stereo/stereo_tum_vi
  Monocular/mono_tum
  Monocular/mono_kitti
  Monocular/mono_euroc
  Monocular/mono_tum_vi
  Monocular-Inertial/mono_inertial_euroc
  Monocular-Inertial/mono_inertial_tum_vi
  Stereo-Inertial/stereo_inertial_euroc
  Stereo-Inertial/stereo_inertial_tum_vi
)

foreach(example ${EXAMPLES_OLD})
  get_filename_component(dir ${example} DIRECTORY)
  get_filename_component(name ${example} NAME_WE)
  set(target_name "${name}_old")

  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples_old/${dir})
  add_executable(${target_name} Examples_old/${example}.cc)
  target_link_libraries(${target_name} ${PROJECT_NAME})
endforeach()

