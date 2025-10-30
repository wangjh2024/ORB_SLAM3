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
# 统一依赖根目录（可选，便于管理）
# set(DEPS_ROOT "E:/3rd_party")

# OpenCV 配置
set(OpenCV_DIR "E:/3rd_party/opencv/build")  # 使用 OpenCV_DIR 替代 OpenCV_ROOT

# Eigen3 配置（如果系统路径中没有，需要手动设置）
# set(Eigen3_DIR "E:/3rd_party/eigen-3.4.0/share/eigen3/cmake")

# Pangolin 配置
set(Pangolin_DIR "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")

# Boost 配置（关键修复）
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
    ${PROJECT_SOURCE_DIR}/Thirdparty
)

# 9. 依赖库查找
# =============================================================================
message(STATUS "开始查找依赖库...")

# 9.1 OpenCV
find_package(OpenCV REQUIRED)
if(OpenCV_FOUND)
    message(STATUS "OpenCV 版本: ${OpenCV_VERSION}")
    message(STATUS "OpenCV 库路径: ${OpenCV_LIBS}")
else()
    message(FATAL_ERROR "OpenCV 库未找到，请检查路径配置")
endif()

# 9.2 Eigen3
find_package(Eigen3 3.4 REQUIRED)
if(Eigen3_FOUND)
    message(STATUS "Eigen3 版本: ${EIGEN3_VERSION}")
else()
    message(FATAL_ERROR "Eigen3 库未找到")
endif()

# 9.3 线程库
find_package(Threads REQUIRED)

# 9.4 Pangolin
find_package(Pangolin REQUIRED)
if(Pangolin_FOUND)
    message(STATUS "Pangolin 找到: ${Pangolin_VERSION}")
else()
    message(FATAL_ERROR "Pangolin 库未找到")
endif()

# 9.5 Boost 库（关键配置）
# 设置 Boost 查找参数
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

# 尝试使用 CONFIG 模式查找
find_package(Boost 1.83.0 CONFIG QUIET COMPONENTS serialization)

if(NOT Boost_FOUND)
    # 如果 CONFIG 模式失败，回退到 MODULE 模式
    message(STATUS "Boost CONFIG 模式失败，尝试 MODULE 模式...")
    find_package(Boost 1.83.0 REQUIRED COMPONENTS serialization)
endif()

if(Boost_FOUND)
    message(STATUS "Boost 版本: ${Boost_VERSION}")
    message(STATUS "Boost 包含目录: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost 库目录: ${Boost_LIBRARY_DIRS}")
else()
    message(FATAL_ERROR "Boost 库未找到，请检查安装和路径配置")
endif()

# 9.6 GLEW
find_package(GLEW REQUIRED)
if(GLEW_FOUND)
    message(STATUS "GLEW 找到: ${GLEW_VERSION}")
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
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>     # 公共头文件
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src                           # 私有源文件
        ${PROJECT_SOURCE_DIR}/include/CameraModels          # 相机模型头文件
        ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus             # Sophus 库
)

# 11. 第三方库构建
# =============================================================================
message(STATUS "开始构建第三方库...")

# 11.1 构建 g2o
add_subdirectory(Thirdparty/g2o)

# 12. 依赖链接配置
# =============================================================================
# 12.1 主库依赖链接
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Eigen3::Eigen              # 数学库
        Pangolin::pangolin         # 可视化库
        ${OpenCV_LIBS}             # 计算机视觉库
    
    PRIVATE
        Threads::Threads           # 线程库
        GLEW::GLEW                 # OpenGL 扩展库
        Boost::serialization       # 序列化库
        
        # 内部第三方库
        ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib
        g2o                        # 图优化库
)

# 12.2 设置目标属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON   # 便于链接
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
    
    # 添加调试信息
    message(STATUS "已配置示例: ${example_name} -> ${output_dir}")
endfunction()

# 13.2 RGB-D 示例
add_orb_slam3_example(rgbd_tum Examples/RGB-D/rgbd_tum.cc RGB-D)

# 13.3 立体视觉示例
add_orb_slam3_example(stereo_kitti Examples/Stereo/stereo_kitti.cc Stereo)
add_orb_slam3_example(stereo_euroc Examples/Stereo/stereo_euroc.cc Stereo)
add_orb_slam3_example(stereo_tum_vi Examples/Stereo/stereo_tum_vi.cc Stereo)

# 13.4 单目示例
add_orb_slam3_example(mono_tum Examples/Monocular/mono_tum.cc Monocular)
add_orb_slam3_example(mono_kitti Examples/Monocular/mono_kitti.cc Monocular)
add_orb_slam3_example(mono_euroc Examples/Monocular/mono_euroc.cc Monocular)
add_orb_slam3_example(mono_tum_vi Examples/Monocular/mono_tum_vi.cc Monocular)

# 13.5 单目惯性示例
add_orb_slam3_example(mono_inertial_euroc Examples/Monocular-Inertial/mono_inertial_euroc.cc Monocular-Inertial)
add_orb_slam3_example(mono_inertial_tum_vi Examples/Monocular-Inertial/mono_inertial_tum_vi.cc Monocular-Inertial)

# 13.6 立体惯性示例
add_orb_slam3_example(stereo_inertial_euroc Examples/Stereo-Inertial/stereo_inertial_euroc.cc Stereo-Inertial)
add_orb_slam3_example(stereo_inertial_tum_vi Examples/Stereo-Inertial/stereo_inertial_tum_vi.cc Stereo-Inertial)

# 14. 旧版示例构建（可选）
# =============================================================================
if(EXISTS ${PROJECT_SOURCE_DIR}/Examples_old)
    message(STATUS "开始构建旧版示例...")
    
    # 旧版示例构建函数
    function(add_orb_slam3_old_example example_name example_source example_category)
        set(output_dir ${PROJECT_SOURCE_DIR}/Examples_old/${example_category})
        
        add_executable(${example_name} ${example_source})
        
        set_target_properties(${example_name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${output_dir}
            OUTPUT_NAME ${example_name}
        )
        
        target_link_libraries(${example_name} ${PROJECT_NAME})
    endfunction()
    
    # 旧版示例配置（根据实际需要添加）
    add_orb_slam3_old_example(rgbd_tum_old Examples_old/RGB-D/rgbd_tum.cc RGB-D)
    # ... 其他旧版示例
endif()

# 15. 标定示例配置
# =============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/Calibration)

# 16. 构建总结
# =============================================================================
message(STATUS "==========================================")
message(STATUS "ORB-SLAM3 配置完成")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "主库目标: ${PROJECT_NAME}")
message(STATUS "输出目录: ${PROJECT_SOURCE_DIR}/lib")
message(STATUS "示例目录: ${PROJECT_SOURCE_DIR}/Examples")
message(STATUS "==========================================")