#========================================================
# g2o/CMakeLists.txt  —— 替换为本版本
#========================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
SET(CMAKE_LEGACY_CYGWIN_WIN32 0)

PROJECT(g2o)

#=== 0. 兜底定义 BOOST_COMPAT_HEADER（父工程若已 set，将沿用） ===
# 说明：父工程顶层应已 set(BOOST_COMPAT_HEADER "<proj>/include/boost_compat.h")
# 若未设置，这里用顶层路径兜底，保证本子目录可单独编译通过。
if(NOT DEFINED BOOST_COMPAT_HEADER)
  set(BOOST_COMPAT_HEADER "${CMAKE_SOURCE_DIR}/include/boost_compat.h")
endif()

#=== A. 目录级：最先强制包含 + 起手就 /Uaccess ===
if(MSVC)
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:/Uaccess>                       # 起手清场
    $<$<COMPILE_LANGUAGE:CXX>:/FI"${BOOST_COMPAT_HEADER}">    # 最早包含净化头
  )
  add_compile_definitions(_CRT_DECLARE_NONSTDC_NAMES=0)       # 根本不导出别名
endif()

#=== 1. MSVC 兼容选项（移除不兼容 GCC 标志 + 常用优化/警告抑制） ===
if(MSVC)
  # 移除遗留的 GCC 选项（若还有地方加了 -O3/-march）
  string(REPLACE "-O3" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  string(REPLACE "-march=native" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  add_compile_options(/wd4819 /wd4267 /wd4244 /wd4530)
endif()

SET(g2o_C_FLAGS)
SET(g2o_CXX_FLAGS)

#=== 2. Windows 通用宏 ===
if(WIN32)
  add_definitions(-DNOMINMAX -D_CRT_SECURE_NO_WARNINGS)
  if(MSVC)
    string(REPLACE "-O3" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    string(REPLACE "-march=native" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
  endif()
endif()

# vasprintf 的 Win 兼容实现
if(WIN32)
  list(APPEND G2O_STUFF_SOURCES g2o/stuff/vasprintf_win.cpp)
endif()

if(MSVC)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
endif()

#=== 3. 默认构建类型/标准 ===
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "BUILD TYPE: ${CMAKE_BUILD_TYPE}")

set(G2O_LIB_TYPE STATIC)

#=== 4. 历史兼容：MSVC8 特例 ===
if(MSVC90)
  add_definitions(-DEIGEN_DONT_ALIGN_STATICALLY=1)
  message(STATUS "Disabling memory alignment for MSVC8")
endif()

add_definitions(-DWINDOWS)

#=== 5. 产物目录 ===
if(WIN32)
  set(g2o_LIBRARY_OUTPUT_DIRECTORY ${g2o_SOURCE_DIR}/bin CACHE PATH "Target for the libraries")
else()
  set(g2o_LIBRARY_OUTPUT_DIRECTORY ${g2o_SOURCE_DIR}/lib CACHE PATH "Target for the libraries")
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${g2o_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${g2o_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${g2o_RUNTIME_OUTPUT_DIRECTORY})

#=== 6. 模块路径/平台宏 ===
list(APPEND CMAKE_MODULE_PATH ${g2o_SOURCE_DIR}/cmake_modules)

if(UNIX)
  add_definitions(-DUNIX)
  message(STATUS "Compiling on Unix")
endif()

#=== 7. OpenMP（默认关） ===
find_package(OpenMP)
set(G2O_USE_OPENMP OFF CACHE BOOL "Build g2o with OpenMP support (EXPERIMENTAL)")
if(OPENMP_FOUND AND G2O_USE_OPENMP)
  set(G2O_OPENMP 1)
  set(g2o_C_FLAGS "${g2o_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(g2o_CXX_FLAGS "${g2o_CXX_FLAGS} -DEIGEN_DONT_PARALLELIZE ${OpenMP_CXX_FLAGS}")
  message(STATUS "Compiling with OpenMP support")
endif()

#=== 8. GCC 优化标志（仅非 MSVC） ===
if(NOT MSVC)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
  set(CMAKE_C_FLAGS_RELEASE   "${CMAKE_C_FLAGS_RELEASE}   -O3 -march=native")
endif()

#=== 9. 警告等级 ===
if(MSVC)
  set(g2o_C_FLAGS  "${g2o_C_FLAGS} /W3")
  set(g2o_CXX_FLAGS "${g2o_CXX_FLAGS} /W3")
else()
  set(g2o_C_FLAGS  "${g2o_C_FLAGS} -Wall -W")
  set(g2o_CXX_FLAGS "${g2o_CXX_FLAGS} -Wall -W")
endif()

# 汇总到全局 flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${g2o_CXX_FLAGS}")
set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${g2o_C_FLAGS}")

#=== 10. Eigen 路径 ===
set(EIGEN3_INCLUDE_DIR "E:/3rd_party/eigen-3.4.0-install/include/eigen3")
if(EIGEN3_FOUND)
  set(G2O_EIGEN3_INCLUDE ${EIGEN3_INCLUDE_DIR} CACHE PATH "Directory of Eigen3")
else()
  set(G2O_EIGEN3_INCLUDE "" CACHE PATH "Directory of Eigen3")
endif()

#=== 11. 生成 config.h ===
set(G2O_CXX_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER}")
configure_file(config.h.in ${g2o_SOURCE_DIR}/config.h)

#=== 12. 头文件路径 ===
include_directories(
  ${g2o_SOURCE_DIR}/core
  ${g2o_SOURCE_DIR}/types
  ${g2o_SOURCE_DIR}/stuff
  ${G2O_EIGEN3_INCLUDE}
)

#=== 13. g2o 主库 ===
add_library(g2o ${G2O_LIB_TYPE}
  # types
  g2o/types/types_sba.h
  g2o/types/types_six_dof_expmap.h
  g2o/types/types_sba.cpp
  g2o/types/types_six_dof_expmap.cpp
  g2o/types/types_seven_dof_expmap.cpp
  g2o/types/types_seven_dof_expmap.h
  g2o/types/se3quat.h
  g2o/types/se3_ops.h
  g2o/types/se3_ops.hpp
  # core
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
  g2o/core/creators.h
  g2o/core/optimization_algorithm_factory.cpp
  g2o/core/estimate_propagator.cpp
  g2o/core/optimization_algorithm_factory.h
  g2o/core/estimate_propagator.h
  g2o/core/factory.cpp
  g2o/core/optimization_algorithm_property.h
  g2o/core/factory.h
  g2o/core/sparse_block_matrix.h
  g2o/core/sparse_optimizer.cpp
  g2o/core/sparse_block_matrix.hpp
  g2o/core/sparse_optimizer.h
  g2o/core/hyper_dijkstra.cpp
  g2o/core/hyper_dijkstra.h
  g2o/core/parameter_container.cpp
  g2o/core/parameter_container.h
  g2o/core/optimization_algorithm.cpp
  g2o/core/optimization_algorithm.h
  g2o/core/optimization_algorithm_with_hessian.cpp
  g2o/core/optimization_algorithm_with_hessian.h
  g2o/core/optimization_algorithm_levenberg.cpp
  g2o/core/optimization_algorithm_levenberg.h
  g2o/core/optimization_algorithm_gauss_newton.cpp
  g2o/core/optimization_algorithm_gauss_newton.h
  g2o/core/jacobian_workspace.cpp
  g2o/core/jacobian_workspace.h
  g2o/core/robust_kernel.cpp
  g2o/core/robust_kernel.h
  g2o/core/robust_kernel_factory.cpp
  g2o/core/robust_kernel_factory.h
  g2o/core/robust_kernel_impl.cpp
  g2o/core/robust_kernel_impl.h
  # stuff
  g2o/stuff/string_tools.h
  g2o/stuff/color_macros.h
  g2o/stuff/macros.h
  # g2o/stuff/timeutil.cpp
  g2o/stuff/misc.h
  g2o/stuff/timeutil.h
  g2o/stuff/os_specific.c
  g2o/stuff/os_specific.h
  g2o/stuff/string_tools.cpp
  g2o/stuff/property.cpp
  g2o/stuff/property.h
  ${G2O_STUFF_SOURCES}
)

#=== B. 目标级：g2o 再上一次保险（最关键的那一刀） ===
if(TARGET g2o AND MSVC)
  target_compile_options(g2o PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:/Uaccess /FI"${BOOST_COMPAT_HEADER}">
  )
endif()
