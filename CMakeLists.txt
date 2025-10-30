# =============================================================================
# ORB-SLAM3 CMake 配置文件 (Windows/MSVC 友好版)
# =============================================================================

#=== 1. 基础配置 ===============================================================
cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3 VERSION 1.0 LANGUAGES C CXX)

#=== 2. 编译器策略设置 =========================================================
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

#=== 3. 依赖库路径配置（按需改成本机路径） =====================================
set(OpenCV_DIR "E:/3rd_party/opencv/build")
set(Pangolin_DIR "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")
set(GLEW_ROOT "E:/3rd_party/glew-2.2.0")

#=== 4. 构建类型配置 ===========================================================
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "当前构建类型: ${CMAKE_BUILD_TYPE}")

#=== 5. 编译器标准设置 =========================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#=== 6. 编译器特定选项（MSVC 友好参数） =======================================
if(MSVC)
    #=== 6.1 统一 UTF-8 / 扩大编译容量 / 减少模板峰值 =========================
    add_compile_options(
        "/utf-8"            # 源码与执行字符集使用 UTF-8，解决 C4819
        "/bigobj"           # 增大单个对象节数量上限
        "/Zm200"            # 提升编译器内部堆配额（不够可调到 /Zm300）
        "/Zc:twoPhase-"     # 关闭两阶段查找，降低模板实例化压力
        "/MP"               # 多进程并行编译
        "/O2"               # Release 优化
        "/EHsc"             # 标准异常模型
        "/permissive-"      # 更符合标准的行为
        "/fp:fast"
        "/wd4996"           # 抑制“不安全”警告
    )
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
    )
else()
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -O3")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3")
endif()

#=== 7. 预处理器定义 ===========================================================
add_compile_definitions(
    $<$<COMPILE_LANGUAGE:CXX>:_HAS_STD_BYTE=0>
    NOMINMAX
    COMPILEDWITHC11
    ORB_SLAM3_EXPORTS
    EIGEN_NO_DEBUG                 # 关闭 Eigen 调试路径
    _CRT_DECLARE_NONSTDC_NAMES=0   # 禁止定义 POSIX 别名(如 access)
)
#=== 7.x: 全局中和 Windows 的 access 宏（让它等于自己，任何重定义都无效） ===
add_compile_definitions(
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:_CRT_DECLARE_NONSTDC_NAMES=0>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<COMPILE_LANGUAGE:CXX>>:access=access>
)
#=== 7.y: 目录级全局 /FI，保证每个翻译单元最先包含 boost_compat.h ==========
if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/FI"${BOOST_COMPAT_HEADER}">)
endif()

#=== 8. 依赖库查找与“必改#1：Eigen 导出给子项目” ==============================
message(STATUS "开始查找依赖库...")

#--- 8.0 boost_compat.h 的绝对路径（用于 /FI 强制包含） ------------------------
set(BOOST_COMPAT_HEADER "${PROJECT_SOURCE_DIR}/include/boost_compat.h")

#--- 8.1 OpenCV ---------------------------------------------------------------
find_package(OpenCV REQUIRED)
message(STATUS "✓ OpenCV 版本: ${OpenCV_VERSION}")

#--- 8.2 Eigen3 ---------------------------------------------------------------
find_package(Eigen3 3.4 REQUIRED)
message(STATUS "✓ Eigen3 版本: ${EIGEN3_VERSION}")

#--- 8.2.x 必改#1：在进入 g2o 子目录前，把 Eigen 包含目录导出给子项目 --------
get_target_property(_EIGEN_INC Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
if(_EIGEN_INC)
    # 供老式外部项目（如某些 g2o 分支）读取
    set(EIGEN3_INCLUDE_DIR "${_EIGEN_INC}" CACHE PATH "Eigen3 include dir for subprojects" FORCE)
    # 兼容名称
    set(EIGEN_INCLUDE_DIR  "${_EIGEN_INC}" CACHE PATH "Eigen include dir (compat)" FORCE)
    # 目录级兜底：确保子目录编译能看到
    include_directories(BEFORE ${_EIGEN_INC})
endif()

#--- 8.3 线程库 ---------------------------------------------------------------
find_package(Threads REQUIRED)

#--- 8.4 Pangolin -------------------------------------------------------------
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

#--- 8.5 Boost （手动配置序列化库） -------------------------------------------
message(STATUS "配置 Boost 库...")
set(BOOST_INCLUDE_DIR "${BOOST_ROOT}")
if(NOT EXISTS "${BOOST_INCLUDE_DIR}/boost/serialization/serialization.hpp")
    message(FATAL_ERROR "✗ Boost 头文件未找到，请检查路径: ${BOOST_INCLUDE_DIR}")
else()
    message(STATUS "✓ Boost 头文件目录: ${BOOST_INCLUDE_DIR}")
endif()

set(BOOST_LIB_PATHS
    "${BOOST_ROOT}/stage/lib"
    "${BOOST_ROOT}/lib"
    "${BOOST_ROOT}/lib64-msvc-14.3"
)
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
    add_library(Boost_serialization INTERFACE IMPORTED)
    set_target_properties(Boost_serialization PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${BOOST_SERIALIZATION_LIB}"
    )
    set(BOOST_SERIALIZATION_TARGET Boost_serialization)
    set(Boost_FOUND TRUE)
    set(Boost_SERIALIZATION_FOUND TRUE)
else()
    message(WARNING "✗ 未找到 Boost 序列化库文件，尝试仅使用头文件模式")
    add_library(Boost_serialization INTERFACE IMPORTED)
    set_target_properties(Boost_serialization PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
    )
    set(BOOST_SERIALIZATION_TARGET Boost_serialization)
    set(Boost_FOUND TRUE)
endif()

#--- 8.6 GLEW -----------------------------------------------------------------
find_package(GLEW REQUIRED)
message(STATUS "✓ GLEW 找到")

#=== 9. 主库构建配置（含 必改#2：主库 /FI） ===================================
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

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/include/CameraModels
        ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
        ${BOOST_INCLUDE_DIR}
        ${Pangolin_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS}
)

#=== B. 目标级：主库再上一次保险 ===
if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:/Uaccess /FI"${BOOST_COMPAT_HEADER}">
  )
endif()
#--- 9.x 必改#2：主库强制优先包含 boost_compat.h（消除 access 宏污染） ---------
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/FI"${BOOST_COMPAT_HEADER}">)
endif()

#=== A. 目录级：最先强制包含 + 起手就 /Uaccess ===
if(MSVC)
  add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:/Uaccess>
    $<$<COMPILE_LANGUAGE:CXX>:/FI"${BOOST_COMPAT_HEADER}">
  )
  add_compile_definitions(_CRT_DECLARE_NONSTDC_NAMES=0)
endif()
#=== 10. 第三方库 g2o（含 必改#3/#4） =========================================
message(STATUS "开始构建第三方库...")

#--- 10.0 为 g2o 临时保存/还原编译选项 -----------------------------------------
set(OLD_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
set(OLD_CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
if(MSVC)
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
endif()

#--- 10.z 必改#4（目录级兜底）：进入 g2o 子目录前，全局加 /FI -----------------
# 仅影响后续子目录（g2o）；你下面马上会还原全局编译选项
if(MSVC)
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/FI"${BOOST_COMPAT_HEADER}">)
endif()
#=== A1: 目录级强制最先包含 boost_compat.h，且起手就 /Uaccess 清场 ===
if(MSVC)
    # 先把历史/环境里残留的 access 宏清掉
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/Uaccess>)
    # 再保证每个翻译单元最先 include 我们的“净化头”
    add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/FI"${BOOST_COMPAT_HEADER}">)
    # 全局禁止 MSVC 为 POSIX 函数生成别名宏（含 access）
    add_compile_definitions(_CRT_DECLARE_NONSTDC_NAMES=0)
endif()

#--- 10.1 添加 g2o 子目录 ------------------------------------------------------
add_subdirectory(Thirdparty/g2o)
#=== B1: 提升编译余量（全局） ===
if(MSVC)
    # /bigobj 你已有；再加一些稳妥的选项
    target_compile_options(${PROJECT_NAME} PRIVATE
        /Zm300            # 提高编译器内部配额
        /FS               # 并行写 PDB 更稳定
    )
endif()

#=== B2: 对 Eigen 重型源文件定点降优化，缓解 C1060 ===
set(EIGEN_HEAVY_SOURCES
    src/Optimizer.cc
    src/Tracking.cc
    src/LocalMapping.cc
    src/LoopClosing.cc
    src/TwoViewReconstruction.cc
    src/FrameDrawer.cc
    src/Frame.cc
    src/Map.cc
    src/MapDrawer.cc
    src/KeyFrame.cc
    src/Atlas.cc
    src/G2oTypes.cc
    src/Sim3Solver.cc
    src/MLPnPsolver.cpp
    src/GeometricTools.cc
    src/CameraModels/Pinhole.cpp
    src/CameraModels/KannalaBrandt8.cpp
)

if(MSVC)
    foreach(src ${EIGEN_HEAVY_SOURCES})
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
            # /O1: 减优化；/Ob1: 降内联；必要时可再加 /Od 调试
            set_source_files_properties(${src} PROPERTIES COMPILE_OPTIONS "/O1;/Ob1")
        endif()
    endforeach()
endif()

#--- 10.2 还原全局编译选项 ------------------------------------------------------
set(CMAKE_CXX_FLAGS ${OLD_CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${OLD_CMAKE_CXX_FLAGS_RELEASE})

#--- 10.3 必改#3：清理 g2o 的 -O3 / -march=native 并补齐 MSVC 友好选项 --------
if(TARGET g2o)
    # 去掉 GCC/Clang 专属选项，避免 cl D9002
    get_target_property(G2O_OPTS g2o COMPILE_OPTIONS)
    if(G2O_OPTS)
        list(FILTER G2O_OPTS EXCLUDE REGEX "^-O3$|^-march=native$")
        set_target_properties(g2o PROPERTIES COMPILE_OPTIONS "${G2O_OPTS}")
    endif()

    # 给 g2o 加上 MSVC 友好参数
    target_compile_options(g2o PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8 /bigobj /Zm200 /Zc:twoPhase- /EHsc /permissive- /MP>
    )

    # 把 Eigen 作为 PUBLIC 依赖传给 g2o（含包含目录传递）
    target_link_libraries(g2o PUBLIC Eigen3::Eigen)
    get_target_property(EIGEN3_INC Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
    if(EIGEN3_INC)
        target_include_directories(g2o PUBLIC ${EIGEN3_INC})
    endif()

    # 关闭 g2o 内部的 Eigen 调试路径
    target_compile_definitions(g2o PUBLIC EIGEN_NO_DEBUG)

    # 再给 g2o 目标自身补一层 /FI 保险
    if(MSVC)
        target_compile_options(g2o PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/FI"${BOOST_COMPAT_HEADER}">)
    endif()
endif()
#=== 10.3x：清理所有 g2o* 目标上的 GCC 专属选项，并补齐 MSVC 友好参数 ========
get_property(_allTargets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(tgt IN LISTS _allTargets)
    if(tgt MATCHES "^g2o") # 覆盖 g2o, g2o_core, g2o_stuff, g2o_types_* 等
        get_target_property(_opts ${tgt} COMPILE_OPTIONS)
        if(_opts)
            list(FILTER _opts EXCLUDE REGEX "^-O3$|^-march=native$")
            set_target_properties(${tgt} PROPERTIES COMPILE_OPTIONS "${_opts}")
        endif()
        target_compile_options(${tgt} PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/utf-8 /bigobj /Zm200 /Zc:twoPhase- /EHsc /permissive- /MP>
        )
        target_compile_definitions(${tgt} PUBLIC EIGEN_NO_DEBUG)

        # 让每个 g2o* 目标也最先包含 boost_compat.h（双保险）
        if(MSVC)
            target_compile_options(${tgt} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/FI"${BOOST_COMPAT_HEADER}">)
        endif()

        # 给 g2o 子目标可见 Eigen
        if(TARGET Eigen3::Eigen)
            target_link_libraries(${tgt} PUBLIC Eigen3::Eigen)
            get_target_property(_einc Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
            if(_einc)
                target_include_directories(${tgt} PUBLIC ${_einc})
            endif()
        endif()
    endif()
endforeach()

#=== 11. 链接配置 ==============================================================
#--- 11.1 确认 g2o 目标或回退为手动库 ------------------------------------------
if(TARGET g2o)
    set(G2O_TARGET g2o)
else()
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

#--- 11.2 主库链接（优先使用 TARGET） ------------------------------------------
if(PANGOLIN_TARGET AND TARGET ${G2O_TARGET})
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            Eigen3::Eigen
            ${PANGOLIN_TARGET}
            ${OpenCV_LIBS}
        PRIVATE
            Threads::Threads
            ${GLEW_LIBRARIES}
            ${BOOST_SERIALIZATION_TARGET}
            ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib
            ${G2O_TARGET}
    )
else()
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            Eigen3::Eigen
            ${OpenCV_LIBS}
        PRIVATE
            Threads::Threads
            ${GLEW_LIBRARIES}
            ${BOOST_SERIALIZATION_TARGET}
            ${Pangolin_LIBRARIES}
            ${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib
            ${G2O_LINK_LIBS}
    )
endif()

#=== 12. 示例程序（含 必改#2 对示例目标生效） ==================================
message(STATUS "开始构建示例程序...")

function(add_orb_slam3_example example_name example_source example_category)
    set(output_dir ${PROJECT_SOURCE_DIR}/Examples/${example_category})
    add_executable(${example_name} ${example_source})
    set_target_properties(${example_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${output_dir}
    )
    target_link_libraries(${example_name} ${PROJECT_NAME})

    #=== 12.x：示例目标也强制包含 boost_compat.h，避免 _access 污染 ===========
    if(MSVC)
        target_compile_options(${example_name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/FI"${BOOST_COMPAT_HEADER}">)
    endif()

    message(STATUS "✓ 已配置示例: ${example_name}")
endfunction()

add_orb_slam3_example(rgbd_tum Examples/RGB-D/rgbd_tum.cc RGB-D)
add_orb_slam3_example(mono_tum Examples/Monocular/mono_tum.cc Monocular)

#=== 13. 构建总结 ==============================================================
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
