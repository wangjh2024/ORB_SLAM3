cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3)

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

#=== 2. 编译定义与选项 ===
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
add_definitions(-D_HAS_STD_BYTE=0 -DNOMINMAX -DCOMPILEDWITHC11 -DORB_SLAM3_EXPORTS)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)


#=== 3. 查找依赖 ===
# set(realsense2_DIR "E:/3rd_party/librealsense/install/lib/cmake/realsense2")



#=== 3. OpenCV ===
set(OpenCV_DIR "E:/3rd_party/opencv/build/x64/vc16/lib")

# set(OpenCV_ROOT "D:/CODE-STUDY/_installed/opencv-4.x/x64/vc17/lib")
find_package(OpenCV REQUIRED)
message("OpenCV version: ${OpenCV_VERSION}")

#=== 4. Eigen3 & Threads ===
set(Eigen3_DIR "E:/3rd_party/eigen-install/share/eigen3/cmake")
find_package(Eigen3 REQUIRED)
find_package(Threads REQUIRED)

# === Pangolin 路径 ===
set(Pangolin_DIR "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")
set(Pangolin_INCLUDE_DIRS "E:/3rd_party/pangolin-install/include")

# === 手动列出 Pangolin 静态库 ===
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

# === Pangolin 兼容层 ===
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

# === 8. OpenSSL ===
# 显式设置路径（防止 CMake 误找 Anaconda 自带的版本）
set(OPENSSL_ROOT_DIR "E:/3rd_party/OpenSSL-Win64")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libcrypto.lib")
set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libssl.lib")

# 设置查找策略
cmake_policy(SET CMP0074 NEW)

# 如果需要可选支持
option(USE_OPENSSL "Use OpenSSL (optional)" ON)

if(USE_OPENSSL)
    find_package(OpenSSL REQUIRED)
    if(OpenSSL_FOUND)
        message(STATUS "✅ OpenSSL Found: ${OPENSSL_VERSION}")
        message(STATUS "Include dir: ${OPENSSL_INCLUDE_DIR}")
        message(STATUS "Crypto lib: ${OPENSSL_CRYPTO_LIBRARY}")
        message(STATUS "SSL lib: ${OPENSSL_SSL_LIBRARY}")
    else()
        message(FATAL_ERROR "❌ OpenSSL not found. Please check OPENSSL_ROOT_DIR.")
    endif()

    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()


#=== 9. 头文件路径 ===
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/CameraModels
  ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
  ${EIGEN3_INCLUDE_DIR}
  ${Pangolin_INCLUDE_DIRS}
)

#=== 10. 库输出路径 ===
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

#=== 11. 构建主库 ===
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

#=== 12. 外部依赖 ===
# add_subdirectory(Thirdparty/g2o)

target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    Eigen3::Eigen
    ${PANGOLIN_TARGET}
    ${Boost_LIBRARIES}
    Threads::Threads
    ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib
    ${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/g2o.lib
    OpenSSL::Crypto
)

#=== 13. 构建各示例 ===
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
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/${dir})
  add_executable(${example}_exe Examples/${example}.cc)
  target_link_libraries(${example}_exe ${PROJECT_NAME})
endforeach()

#=== 14. old 版本示例 ===
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
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples_old/${dir})
  add_executable(${example}_old Examples_old/${example}.cc)
  target_link_libraries(${example}_old ${PROJECT_NAME})
endforeach()
