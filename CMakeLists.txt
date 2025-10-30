# =============================================================================
# ORB-SLAM3 CMake 配置文件
# =============================================================================

# 1. 基础配置
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3 VERSION 1.0 LANGUAGES C CXX)

# 2. 编译器策略设置
# =============================================================================
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

# 3. 依赖库路径配置
# =============================================================================
set(OpenCV_DIR "E:/3rd_party/opencv/build")
set(Pangolin_DIR "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")
set(GLEW_ROOT "E:/3rd_party/glew-2.2.0")

# 4. 构建类型配置
# =============================================================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "当前构建类型: ${CMAKE_BUILD_TYPE}")

# 5. 编译器标准设置
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 6. 编译器特定选项配置
# =============================================================================
if(MSVC)
    # MSVC 编译器选项
    add_compile_options(
        "/bigobj"
        "/wd4996"
        "/MP"
        "/O2"
        "/fp:fast"
    )
    
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
    )
else()
    # GCC/Clang 编译器选项
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
endif()

# 7. 预处理器定义
# =============================================================================
add_compile_definitions(
    $<$<COMPILE_LANGUAGE:CXX>:_HAS_STD_BYTE=0>
    NOMINMAX
    COMPILEDWITHC11
    ORB_SLAM3_EXPORTS
)

# 8. 依赖库查找
# =============================================================================
message(STATUS "开始查找依赖库...")

# 8.1 OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "✓ OpenCV 版本: ${OpenCV_VERSION}")

# 8.2 Eigen3
find_package(Eigen3 3.4 REQUIRED)
message(STATUS "✓ Eigen3 版本: ${EIGEN3_VERSION}")

# 8.3 线程库
find_package(Threads REQUIRED)

# 8.4 Pangolin
find_package(Pangolin REQUIRED)
if(Pangolin_FOUND)
    message(STATUS "✓ Pangolin 找到")
    
    if(TARGET pangolin)
        set(PANGOLIN_TARGET pangolin)
    elseif(TARGET Pangolin::pangolin)
        set(PANGOLIN_TARGET Pangolin::pangolin)
    else()
        set(PANGOLIN_LINK_LIBS ${Pangolin_LIBRARIES})
    endif()
endif()

# 8.5 Boost 库 - 完全手动配置
message(STATUS "配置 Boost 库...")

set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")
set(BOOST_INCLUDE_DIR "${BOOST_ROOT}")

# 检查 Boost 目录是否存在
if(NOT EXISTS "${BOOST_INCLUDE_DIR}/boost/serialization/serialization.hpp")
    message(FATAL_ERROR "✗ Boost 头文件未找到，请检查路径: ${BOOST_INCLUDE_DIR}")
else()
    message(STATUS "✓ Boost 头文件目录: ${BOOST_INCLUDE_DIR}")
endif()

# 查找 Boost 序列化库
set(BOOST_LIB_PATHS
    "${BOOST_ROOT}/stage/lib"
    "${BOOST_ROOT}/lib"
    "${BOOST_ROOT}/lib64-msvc-14.3"
)

# 可能的库文件名（根据编译方式不同）
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
    
    # 手动创建 Boost 目标
    add_library(Boost_serialization INTERFACE IMPORTED)
    set_target_properties(Boost_serialization PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${BOOST_SERIALIZATION_LIB}"
    )
    
    # 设置变量供后续使用
    set(BOOST_SERIALIZATION_TARGET Boost_serialization)
    set(Boost_FOUND TRUE)
    set(Boost_SERIALIZATION_FOUND TRUE)
    
else()
    message(WARNING "✗ 未找到 Boost 序列化库文件，尝试仅使用头文件模式")
    
    # 仅使用头文件模式（如果不需要链接库）
    add_library(Boost_serialization INTERFACE IMPORTED)
    set_target_properties(Boost_serialization PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
    )
    
    set(BOOST_SERIALIZATION_TARGET Boost_serialization)
    set(Boost_FOUND TRUE)
endif()

# 8.6 GLEW
find_package(GLEW REQUIRED)
message(STATUS "✓ GLEW 找到")

# 9. 主库构建配置
# =============================================================================
message(STATUS "开始构建 ORB-SLAM3 主库...")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

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
)

# 包含目录配置
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/include/CameraModels
        ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
        ${BOOST_INCLUDE_DIR}        # 手动添加 Boost 头文件
        ${Pangolin_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
)

# 10. 第三方库构建
# =============================================================================
message(STATUS "开始构建第三方库...")

# 保存当前编译选项
set(OLD_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(OLD_CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})

# 为 g2o 设置 MSVC 兼容的编译选项
if(MSVC)
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
endif()

# 构建 g2o
add_subdirectory(Thirdparty/g2o)

# 恢复原始编译选项
set(CMAKE_CXX_FLAGS ${OLD_CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${OLD_CMAKE_CXX_FLAGS_RELEASE})

# 11. 依赖链接配置
# =============================================================================
# 检查 g2o 目标
if(TARGET g2o)
    set(G2O_TARGET g2o)
else()
    # 手动查找 g2o 库
    find_library(G2O_LIBRARY
        NAMES g2o libg2o
        PATHS ${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/RelWithDebInfo
              ${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/Release
        NO_DEFAULT_PATH
    )
    if(G2O_LIBRARY)
        set(G2O_LINK_LIBS ${G2O_LIBRARY})
    else()
        message(FATAL_ERROR "✗ 未找到 g2o 库")
    endif()
endif()

# 主库依赖链接 - 使用手动配置的 Boost 目标
if(PANGOLIN_TARGET AND TARGET ${G2O_TARGET})
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            Eigen3::Eigen
            ${PANGOLIN_TARGET}
            ${OpenCV_LIBS}
        PRIVATE
            Threads::Threads
            ${GLEW_LIBRARIES}
            ${BOOST_SERIALIZATION_TARGET}  # 使用手动创建的目标
            ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib
            ${G2O_TARGET}
    )
else()
    # 传统链接方式
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            Eigen3::Eigen
            ${OpenCV_LIBS}
        PRIVATE
            Threads::Threads
            ${GLEW_LIBRARIES}
            ${BOOST_SERIALIZATION_TARGET}  # 使用手动创建的目标
            ${Pangolin_LIBRARIES}
            ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib
            ${G2O_LINK_LIBS}
    )
endif()

# 12. 示例程序构建
# =============================================================================
message(STATUS "开始构建示例程序...")

function(add_orb_slam3_example example_name example_source example_category)
    set(output_dir ${PROJECT_SOURCE_DIR}/Examples/${example_category})
    add_executable(${example_name} ${example_source})
    set_target_properties(${example_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${output_dir}
    )
    target_link_libraries(${example_name} ${PROJECT_NAME})
    message(STATUS "✓ 已配置示例: ${example_name}")
endfunction()

add_orb_slam3_example(rgbd_tum Examples/RGB-D/rgbd_tum.cc RGB-D)
add_orb_slam3_example(mono_tum Examples/Monocular/mono_tum.cc Monocular)

# 13. 构建总结
# =============================================================================
message(STATUS "==========================================")
message(STATUS "✓ ORB-SLAM3 配置完成")
message(STATUS "✓ Boost 配置: 手动模式")
message(STATUS "✓ Boost 头文件: ${BOOST_INCLUDE_DIR}")
if(BOOST_SERIALIZATION_LIB)
    message(STATUS "✓ Boost 库文件: ${BOOST_SERIALIZATION_LIB}")
else()
    message(STATUS "⚠ Boost 库文件: 仅头文件模式")
endif()
message(STATUS "==========================================")