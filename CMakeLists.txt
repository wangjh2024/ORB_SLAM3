cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3)

#=== 1. 基础与编译选项 ===
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if(MSVC)
    add_compile_options(/O2 /W3 /wd4267 /wd4244 /wd4305 /wd4819 /wd4996 /wd4018)
    add_definitions(
        -D_USE_MATH_DEFINES
        -D_CRT_SECURE_NO_WARNINGS
        -D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
        -DBOOST_ALLOW_DEPRECATED_HEADERS
        -DBOOST_SERIALIZATION_NO_LIB
    )
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG)
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
        if(COMPILER_SUPPORTS_MARCH_NATIVE)
            add_compile_options(-march=native)
        endif()
    else()
        add_compile_options(-O0 -g)
    endif()
endif()

#=== 2. 输出路径 ===
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})
file(MAKE_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})

#=== 3. 查找依赖 ===
set(OpenCV_DIR "E:/3rd_party/opencv/build/x64/vc16/lib")
set(Eigen3_DIR "E:/3rd_party/eigen-install/share/eigen3/cmake")
set(Pangolin_DIR "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")
set(BOOST_ROOT "E:/3rd_party/boost17")
set(realsense2_DIR "E:/3rd_party/librealsense/install/lib/cmake/realsense2")

find_package(OpenCV REQUIRED COMPONENTS core highgui imgproc imgcodecs calib3d features2d flann)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(Pangolin REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization system filesystem)
find_package(Threads REQUIRED)
find_package(realsense2 QUIET)

message(STATUS "=== Dependency Status ===")
message(STATUS "OpenCV: ${OpenCV_VERSION}")
message(STATUS "Eigen3: ${EIGEN3_VERSION_STRING}")
message(STATUS "Pangolin: ${Pangolin_FOUND}")
message(STATUS "Boost: ${Boost_VERSION}")
message(STATUS "RealSense2: ${realsense2_FOUND}")

#=== 4. g2o 子模块或外部库 ===
# add_subdirectory(Thirdparty/g2o)
set(DBoW2_LIB "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib")
set(G2O_LIB "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/g2o.lib")

#=== 5. 源文件与头文件 ===
set(ORB_SLAM3_SOURCES
    src/System.cc src/Tracking.cc src/LocalMapping.cc src/LoopClosing.cc
    src/ORBextractor.cc src/ORBmatcher.cc src/FrameDrawer.cc src/Converter.cc
    src/MapPoint.cc src/KeyFrame.cc src/Atlas.cc src/Map.cc src/MapDrawer.cc
    src/Optimizer.cc src/Frame.cc src/KeyFrameDatabase.cc src/Sim3Solver.cc
    src/Viewer.cc src/ImuTypes.cc src/G2oTypes.cc src/CameraModels/Pinhole.cpp
    src/CameraModels/KannalaBrandt8.cpp src/OptimizableTypes.cpp
    src/MLPnPsolver.cpp src/GeometricTools.cc src/TwoViewReconstruction.cc
    src/Config.cc src/Settings.cc
)
file(GLOB_RECURSE ORB_SLAM3_HEADERS include/*.h)

#=== 6. 生成主库 ===
add_library(${PROJECT_NAME} SHARED ${ORB_SLAM3_SOURCES} ${ORB_SLAM3_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/CameraModels
    ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
)
target_compile_definitions(${PROJECT_NAME} PRIVATE COMPILEDWITHC17)

# === Pangolin 目标检测与兼容层 ===
if(TARGET Pangolin::Pangolin)
    set(PANGOLIN_TARGET Pangolin::Pangolin)
elseif(TARGET Pangolin::pangolin)
    set(PANGOLIN_TARGET Pangolin::pangolin)
elseif(DEFINED Pangolin_LIBRARIES AND DEFINED Pangolin_INCLUDE_DIRS)
    add_library(Pangolin::pangolin INTERFACE IMPORTED)
    set_target_properties(Pangolin::pangolin PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Pangolin_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${Pangolin_LIBRARIES}"
    )
    set(PANGOLIN_TARGET Pangolin::pangolin)
    message(STATUS "Pangolin shim: created IMPORTED target Pangolin::pangolin")
else()
    message(FATAL_ERROR "❌ Pangolin target not found, and no Pangolin_LIBRARIES variable available!")
endif()

#=== 7. 链接依赖 ===
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${PANGOLIN_TARGET}       # ✅ 自动选用 Pangolin::Pangolin / Pangolin::pangolin / shim
    Eigen3::Eigen
    Boost::serialization
    Boost::system
    Boost::filesystem
    Threads::Threads
    ${DBoW2_LIB}
    ${G2O_LIB}
)


#=== 8. RealSense 可选 ===
if(realsense2_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE realsense2::realsense2)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_REALSENSE=1)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_REALSENSE=0)
endif()

#=== 9. 示例函数 ===
function(add_orb_slam3_example name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} ${PROJECT_NAME})
    set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
endfunction()

#=== 10. 示例构建 ===
add_orb_slam3_example(mono_tum Examples/Monocular/mono_tum.cc)
add_orb_slam3_example(stereo_kitti Examples/Stereo/stereo_kitti.cc)
add_orb_slam3_example(rgbd_tum Examples/RGB-D/rgbd_tum.cc)

#=== 11. 安装配置 ===
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/ DESTINATION include FILES_MATCHING PATTERN "*.h")

#=== 12. 构建信息 ===
message(STATUS "")
message(STATUS "=== ORB_SLAM3 Build Summary ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Library output: ${LIBRARY_OUTPUT_PATH}")
message(STATUS "Binary output: ${EXECUTABLE_OUTPUT_PATH}")
message(STATUS "RealSense support: ${realsense2_FOUND}")
message(STATUS "To build: cmake --build . --config Release")
message(STATUS "=================================")
