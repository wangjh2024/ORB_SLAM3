# ===============================================================
#  g2o CMake 构建脚本（优化版，适配 Windows + VS2022 + Eigen3）
# ===============================================================

#=== 1. CMake 基本配置 ===
cmake_minimum_required(VERSION 3.10...3.28)
project(g2o LANGUAGES C CXX)

# 启用 C++14 支持（ORB_SLAM3 与 g2o 推荐使用 C++14）
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#=== 2. 启用动态库构建（Windows 下自动生成 .dll/.lib） ===
option(BUILD_SHARED_LIBS "Build shared libraries (DLL)" ON)

#=== 3. Windows 平台特定定义（导出符号） ===
if(WIN32)
    add_definitions(-DG2O_EXPORTS)
endif()


#=== 4. 手动指定 Eigen3 头文件路径 ===
# set(EIGEN3_INCLUDE_DIR "E:/3rd_party/eigen-3.4.0-install/include/eigen3")
include_directories(${EIGEN3_INCLUDE_DIR})

#=== 5. 构建类型：Release by default ===
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

#=== 6. 输出路径设置 ===
if(WIN32)
  set(g2o_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin CACHE PATH "Target for the libraries")
else()
  set(g2o_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib CACHE PATH "Target for the libraries")
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${g2o_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${g2o_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${g2o_LIBRARY_OUTPUT_DIRECTORY})

#=== 7. 设置编译器优化选项 ===
if(MSVC)
    # Windows 下 MSVC 优化参数
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /arch:AVX2 /W3")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /arch:AVX2 /W3")
else()
    # Linux/macOS 下 GCC/Clang 优化参数
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -Wall")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -march=native -Wall")
endif()

#=== 8. 寻找 OpenMP（可选） ===
find_package(OpenMP)
set(G2O_USE_OPENMP OFF CACHE BOOL "Build g2o with OpenMP support (EXPERIMENTAL)")
if(OPENMP_FOUND AND G2O_USE_OPENMP)
    set(G2O_OPENMP 1)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    add_definitions(-DEIGEN_DONT_PARALLELIZE)
    message(STATUS "Compiling with OpenMP support")
endif()

#=== 9. 查找 Eigen3（必须） ===
find_package(Eigen3 3.1.0 REQUIRED)
if(EIGEN3_FOUND)
    message(STATUS "Found Eigen3: ${EIGEN3_INCLUDE_DIR}")
    include_directories(${EIGEN3_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Eigen3 not found! Please check EIGEN3_INCLUDE_DIR.")
endif()

#=== 10. 生成 config.h（供源码中引用） ===
set(G2O_CXX_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER}")
configure_file(config.h.in ${CMAKE_SOURCE_DIR}/config.h)

#=== 11. 顶层包含目录 ===
include_directories(
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/types
    ${CMAKE_SOURCE_DIR}/stuff
)

#=== 12. 添加源文件并生成 g2o 动态库 ===
# 注意：Windows 下此处将生成 g2o.dll + g2o.lib
add_library(g2o
    # ====== core 模块 ======
    g2o/core/base_edge.h
    g2o/core/base_binary_edge.h
    g2o/core/hyper_graph_action.cpp
    g2o/core/base_binary_edge.hpp
    g2o/core/hyper_graph_action.h
    g2o/core/base_multi_edge.h
    g2o/core/hyper_graph.cpp
    g2o/core/base_multi_edge.hpp
    g2o/core/hyper_graph.h
    g2o/core/base_unary_edge.h
    g2o/core/linear_solver.h
    g2o/core/base_unary_edge.hpp
    g2o/core/marginal_covariance_cholesky.cpp
    g2o/core/base_vertex.h
    g2o/core/marginal_covariance_cholesky.h
    g2o/core/base_vertex.hpp
    g2o/core/matrix_structure.cpp
    g2o/core/batch_stats.cpp
    g2o/core/matrix_structure.h
    g2o/core/batch_stats.h
    g2o/core/openmp_mutex.h
    g2o/core/block_solver.h
    g2o/core/block_solver.hpp
    g2o/core/parameter.cpp
    g2o/core/parameter.h
    g2o/core/cache.cpp
    g2o/core/cache.h
    g2o/core/optimizable_graph.cpp
    g2o/core/optimizable_graph.h
    g2o/core/solver.cpp
    g2o/core/solver.h
    g2o/core/factory.cpp
    g2o/core/factory.h
    g2o/core/robust_kernel.cpp
    g2o/core/robust_kernel.h
    g2o/core/robust_kernel_factory.cpp
    g2o/core/robust_kernel_factory.h
    g2o/core/robust_kernel_impl.cpp
    g2o/core/robust_kernel_impl.h
    g2o/core/optimization_algorithm.cpp
    g2o/core/optimization_algorithm.h
    g2o/core/optimization_algorithm_levenberg.cpp
    g2o/core/optimization_algorithm_levenberg.h
    g2o/core/optimization_algorithm_gauss_newton.cpp
    g2o/core/optimization_algorithm_gauss_newton.h
    g2o/core/jacobian_workspace.cpp
    g2o/core/jacobian_workspace.h
    g2o/core/estimate_propagator.cpp
    g2o/core/estimate_propagator.h
    g2o/core/parameter_container.cpp
    g2o/core/parameter_container.h
    g2o/core/matrix_structure.cpp
    g2o/core/matrix_structure.h

    # ====== types 模块 ======
    g2o/types/types_sba.h
    g2o/types/types_sba.cpp
    g2o/types/types_six_dof_expmap.cpp
    g2o/types/types_six_dof_expmap.h
    g2o/types/types_seven_dof_expmap.cpp
    g2o/types/types_seven_dof_expmap.h
    g2o/types/se3quat.h
    g2o/types/se3_ops.h
    g2o/types/se3_ops.hpp

    # ====== stuff 模块 ======
    g2o/stuff/string_tools.cpp
    g2o/stuff/string_tools.h
    g2o/stuff/timeutil.cpp
    g2o/stuff/timeutil.h
    g2o/stuff/property.cpp
    g2o/stuff/property.h
    g2o/stuff/os_specific.c
    g2o/stuff/os_specific.h
    g2o/stuff/macros.h
    g2o/stuff/color_macros.h
    g2o/stuff/misc.h
)

#=== 13. 目标属性设置（输出路径） ===
set_target_properties(g2o PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${g2o_LIBRARY_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${g2o_LIBRARY_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY ${g2o_LIBRARY_OUTPUT_DIRECTORY}
)

#=== 14. 打印最终状态 ===
message(STATUS "✅ g2o will be built as a shared library (DLL)")
message(STATUS "✅ Output directory: ${g2o_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "✅ Using Eigen3 include: ${EIGEN3_INCLUDE_DIR}")
