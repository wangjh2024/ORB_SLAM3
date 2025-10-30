# =============================================================================
# ORB-SLAM3 CMake 配置文件
# =============================================================================

# 1. 基础配置
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3 VERSION 1.0 LANGUAGES C CXX)

# 2. 编译器策略设置
# =============================================================================
# 设置现代 CMake 策略以避免警告
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)  # 处理 PACKAGENAME_ROOT 变量
endif()

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)  # 处理 FindBoost 模块
endif()

# 3. 依赖库路径配置（根据实际环境修改）
# =============================================================================
# OpenCV 配置
set(OpenCV_DIR "E:/3rd_party/opencv/build")

# Pangolin 配置
set(Pangolin_DIR "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")

# Boost 配置
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")
set(Boost_DIR "${BOOST_ROOT}/lib/cmake/Boost-1.83.0")

# GLEW 配置
set(GLEW_ROOT "E:/3rd_party/glew-2.2.0")

# 4. 构建类型配置
# =============================================================================
# 设置默认构建类型为 Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "当前构建类型: ${CMAKE_BUILD_TYPE}")

# 5. 编译器标准设置
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 6. 编译选项配置
# =============================================================================
# 通用编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")

# Release 模式特定优化
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
endif()

# MSVC 编译器特定选项
if(MSVC)
    add_compile_options(
        "/bigobj"           # 处理大量符号
        "/wd4996"           # 禁用某些警告
        "/MP"               # 多处理器编译
    )
endif()

# 7. 预处理器定义
# =============================================================================
add_compile_definitions(
    $<$<COMPILE_LANGUAGE:CXX>:_HAS_STD_BYTE=0>  # 解决 std::byte 冲突
    NOMINMAX                                   # 禁用 min/max 宏
    COMPILEDWITHC11                            # ORB-SLAM3 编译标识
    ORB_SLAM3_EXPORTS                          # DLL 导出标识
)

# 8. 模块路径配置
# =============================================================================
list(APPEND CMAKE_MODULE_PATH 
    ${PROJECT_SOURCE_DIR}/cmake_modules
)

# 9. 依赖库查找
# =============================================================================
message(STATUS "开始查找依赖库...")

# 9.1 OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "✓ OpenCV 版本: ${OpenCV_VERSION}")
else()
    message(FATAL_ERROR "✗ OpenCV 库未找到，请检查路径配置")
endif()

# 9.2 Eigen3
find_package(Eigen3 3.4 REQUIRED)
if(Eigen3_FOUND)
    message(STATUS "✓ Eigen3 版本: ${EIGEN3_VERSION}")
else()
    message(FATAL_ERROR "✗ Eigen3 库未找到")
endif()

# 9.3 线程库
find_package(Threads REQUIRED)
message(STATUS "✓ 线程库配置完成")

# 9.4 Pangolin - 修复目标名称问题
find_package(Pangolin REQUIRED)
if(Pangolin_FOUND)
    message(STATUS "✓ Pangolin 找到")
    # 打印 Pangolin 目标信息用于调试
    message(STATUS "Pangolin 包含目录: ${Pangolin_INCLUDE_DIRS}")
    message(STATUS "Pangolin 库文件: ${Pangolin_LIBRARIES}")
    
    # 检查可用的目标名称
    if(TARGET pangolin)
        message(STATUS "✓ 找到目标: pangolin")
    elseif(TARGET Pangolin::pangolin)
        message(STATUS "✓ 找到目标: Pangolin::pangolin")
    else()
        # 列出所有可用目标
        get_property(targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
        message(STATUS "可用目标: ${targets}")
    endif()
else()
    message(FATAL_ERROR "✗ Pangolin 库未找到")
endif()

# 9.5 Boost 库
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# 尝试使用 CONFIG 模式查找
find_package(Boost 1.83.0 CONFIG QUIET COMPONENTS serialization)

if(NOT Boost_FOUND)
    message(STATUS "Boost CONFIG 模式失败，尝试 MODULE 模式...")
    find_package(Boost 1.83.0 REQUIRED COMPONENTS serialization)
endif()

if(Boost_FOUND)
    message(STATUS "✓ Boost 版本: ${Boost_VERSION}")
else()
    message(FATAL_ERROR "✗ Boost 库未找到")
endif()

# 9.6 GLEW
find_package(GLEW REQUIRED)
if(GLEW_FOUND)
    message(STATUS "✓ GLEW 找到")
else()
    message(FATAL_ERROR "✗ GLEW 库未找到")
endif()

# 10. 主库构建配置
# =============================================================================
message(STATUS "开始构建 ORB-SLAM3 主库...")

# 10.1 设置输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 10.2 创建共享库
add_library(${PROJECT_NAME} SHARED
    # 核心源文件
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
    
    # 相机模型
    src/CameraModels/Pinhole.cpp
    src/CameraModels/KannalaBrandt8.cpp
    
    # 工具类
    src/OptimizableTypes.cpp
    src/MLPnPsolver.cpp
    src/GeometricTools.cc
    src/TwoViewReconstruction.cc
    src/Config.cc
    src/Settings.cc
)

# 10.3 包含目录配置
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/include/CameraModels
        ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
)

# 11. 第三方库构建
# =============================================================================
message(STATUS "开始构建第三方库...")

# 11.1 构建 g2o
add_subdirectory(Thirdparty/g2o)

# 12. 依赖链接配置 - 修复 Pangolin 目标名称问题
# =============================================================================
# 12.1 确定正确的 Pangolin 目标名称
set(PANGOLIN_TARGET "")
if(TARGET pangolin)
    set(PANGOLIN_TARGET pangolin)
    message(STATUS "使用 Pangolin 目标: pangolin")
elseif(TARGET Pangolin::pangolin)
    set(PANGOLIN_TARGET Pangolin::pangolin)
    message(STATUS "使用 Pangolin 目标: Pangolin::pangolin")
elseif(TARGET Pangolin::Pangolin)
    set(PANGOLIN_TARGET Pangolin::Pangolin)
    message(STATUS "使用 Pangolin 目标: Pangolin::Pangolin")
else()
    # 如果找不到目标，使用传统链接方式
    message(STATUS "未找到 Pangolin CMake 目标，使用传统链接方式")
    set(PANGOLIN_TARGET ${Pangolin_LIBRARIES})
endif()

# 12.2 主库依赖链接
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Eigen3::Eigen
        ${PANGOLIN_TARGET}  # 使用动态确定的 Pangolin 目标
        ${OpenCV_LIBS}
    
    PRIVATE
        Threads::Threads
        ${GLEW_LIBRARIES}
        Boost::serialization
        
        # 内部第三方库
        ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib
        g2o
)

# 12.3 包含目录（传统方式备用）
target_include_directories(${PROJECT_NAME} PRIVATE
    ${Pangolin_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

# 12.4 设置目标属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# 13. 示例程序构建
# =============================================================================
message(STATUS "开始构建示例程序...")

# 13.1 示例构建函数
function(add_orb_slam3_example example_name example_source example_category)
    set(output_dir ${PROJECT_SOURCE_DIR}/Examples/${example_category})
    
    add_executable(${example_name} ${example_source})
    
    set_target_properties(${example_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${output_dir}
        OUTPUT_NAME ${example_name}
    )
    
    target_link_libraries(${example_name} ${PROJECT_NAME})
    
    message(STATUS "✓ 已配置示例: ${example_name}")
endfunction()

# 13.2 构建主要示例（先构建几个测试）
add_orb_slam3_example(rgbd_tum Examples/RGB-D/rgbd_tum.cc RGB-D)
add_orb_slam3_example(mono_tum Examples/Monocular/mono_tum.cc Monocular)
add_orb_slam3_example(stereo_kitti Examples/Stereo/stereo_kitti.cc Stereo)

# 13.3 其他示例（可选，先注释掉以测试）
# add_orb_slam3_example(mono_kitti Examples/Monocular/mono_kitti.cc Monocular)
# add_orb_slam3_example(mono_euroc Examples/Monocular/mono_euroc.cc Monocular)
# add_orb_slam3_example(mono_tum_vi Examples/Monocular/mono_tum_vi.cc Monocular)

# 14. 标定示例配置
# =============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/Calibration)

# 15. 构建总结
# =============================================================================
message(STATUS "==========================================")
message(STATUS "✓ ORB-SLAM3 配置完成")
message(STATUS "✓ 构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "✓ 主库目标: ${PROJECT_NAME}")
message(STATUS "✓ 输出目录: ${PROJECT_SOURCE_DIR}/lib")
message(STATUS "✓ 示例目录: ${PROJECT_SOURCE_DIR}/Examples")
message(STATUS "==========================================")

# 16. 调试信息（可选）
# =============================================================================
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "调试信息:")
    message(STATUS "Pangolin 目标: ${PANGOLIN_TARGET}")
    message(STATUS "OpenCV 库: ${OpenCV_LIBS}")
    message(STATUS "Boost 库: ${Boost_LIBRARIES}")
endif()