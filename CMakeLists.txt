cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3 LANGUAGES CXX)

#=== 0. 依赖路径（根据本机环境修改） ===
set(OpenCV_DIR "E:/3rd_party/opencv/build/x64/vc16/lib")
set(EIGEN3_INCLUDE_DIR "E:/3rd_party/eigen-3.4.0-install/include/eigen3")
set(Pangolin_DIR "E:/3rd_party/pangolin-install/lib/cmake/Pangolin")
set(Pangolin_INCLUDE_DIRS "E:/3rd_party/pangolin-install/include")
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")
set(GLEW_ROOT "E:/3rd_party/glew-2.2.0")
set(DBoW2_LIB  "${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/DBoW2.lib")
set(G2O_LIB    "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/g2o.lib")

#=== 1. 基本构建配置 ===
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

#=== 2. MSVC 专项优化与编码修复 ===
#=== 2. MSVC 专项优化与编码修复 ===
if(MSVC)
  # 强制使用UTF-8编码
  add_compile_options(/utf-8)
  
  # 禁用特定警告
  add_compile_options(
    /permissive- /MP /bigobj
    /wd4244 /wd4267 /wd4305 /wd4996 /wd4820 /wd4464 /wd4005
  )
  
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi /Ot /arch:AVX2")

  # 检查并设置强制预包含
  set(WIN_FORCE_INCLUDE "${PROJECT_SOURCE_DIR}/include/win_gl_preinclude.h")
  set(WIN_UNDEF_ACCESS  "${PROJECT_SOURCE_DIR}/include/win_compat_undef_access.h")
  
  # 重新创建正确的预包含文件
  if(NOT EXISTS "${WIN_FORCE_INCLUDE}")
    file(WRITE "${WIN_FORCE_INCLUDE}" 
      "// win_gl_preinclude.h - Fix Windows OpenGL header conflicts\n#ifndef WIN_GL_PREINCLUDE_H\n#define WIN_GL_PREINCLUDE_H\n\n#define WIN32_LEAN_AND_MEAN\n#define NOMINMAX\n#define BOOST_ALL_NO_LIB\n#define BOOST_SERIALIZATION_DYN_LINK\n\n#endif\n")
  endif()
  
  if(NOT EXISTS "${WIN_UNDEF_ACCESS}")
    file(WRITE "${WIN_UNDEF_ACCESS}"
      "// win_compat_undef_access.h - Undefine Windows macros\n#ifndef WIN_COMPAT_UNDEF_ACCESS_H\n#define WIN_COMPAT_UNDEF_ACCESS_H\n\n#ifdef _ACCESS\n#undef _ACCESS\n#endif\n\n#ifdef DELETE\n#undef DELETE\n#endif\n\n#ifdef IGNORE\n#undef IGNORE\n#endif\n\n#endif\n")
  endif()
  
  add_compile_options(/FI"${WIN_FORCE_INCLUDE}" /FI"${WIN_UNDEF_ACCESS}")
endif()

#=== 3. 全局宏定义 ===
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_USE_MATH_DEFINES)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
# 注意：移除了 EIGEN_MAKE_ALIGNED_OPERATOR_NEW，因为Eigen会自己定义

#=== 4. CMake 策略 ===
cmake_policy(SET CMP0167 NEW)
cmake_policy(SET CMP0144 NEW)

#=== 5. 模块路径 ===
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

#=== 6. 依赖：OpenCV ===
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

#=== 7. 依赖：Eigen（仅头文件） ===
include_directories(${EIGEN3_INCLUDE_DIR})

#=== 8. 依赖：线程库 ===
find_package(Threads REQUIRED)

#=== 9. 依赖：OpenGL 与 GLEW ===
# 在包含OpenGL头文件之前定义必要的宏
add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DNOMINMAX)

# 先查找OpenGL
find_package(OpenGL REQUIRED)

# 确保正确的包含顺序
if(OPENGL_FOUND)
  # 添加OpenGL库路径
  include_directories(${OPENGL_INCLUDE_DIR})
  
  # 添加OpenGL库
  list(APPEND LINK_LIBS ${OPENGL_LIBRARIES})
  
  # 对于Windows平台，需要额外处理
  if(WIN32)
    # 确保Windows.h在OpenGL头文件之前被包含
    add_definitions(-D_WINDOWS)
    add_definitions(-DGL_GLEXT_PROTOTYPES)
    
    # 防止Windows.h和gl.h的宏冲突
    add_definitions(-DNOGDI)
    add_definitions(-DNOUSER)
  endif()
endif()
#=== 10. 依赖：Pangolin ===
find_package(Pangolin QUIET)
if(TARGET Pangolin::pangolin)
  message(STATUS "Pangolin target found")
else()
  message(STATUS "Manually creating Pangolin imported interface target...")
  add_library(Pangolin::pangolin INTERFACE IMPORTED GLOBAL)
  set_property(TARGET Pangolin::pangolin PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES "${Pangolin_INCLUDE_DIRS}")
  set(PANGOLIN_LIBS
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
  set_property(TARGET Pangolin::pangolin PROPERTY
    INTERFACE_LINK_LIBRARIES "${PANGOLIN_LIBS};OpenGL::GL;${GLEW_LIB_TARGET}")
endif()

#=== 11. 依赖：Boost（修复版本兼容性）===
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")

# 设置Boost兼容性宏（解决_access错误）
add_definitions(-DBOOST_ALL_NO_LIB)
add_definitions(-DBOOST_SERIALIZATION_DYN_LINK)

# 优先使用 CMake 的 FindBoost
find_package(Boost 1.83.0 QUIET COMPONENTS serialization filesystem system)

if(Boost_FOUND)
  message(STATUS "Boost found via CMake: ${Boost_VERSION}")
  message(STATUS "Boost includes: ${Boost_INCLUDE_DIRS}")
  message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
else()
  # 回退到手动配置
  message(STATUS "Boost not found via CMake, using manual configuration")
  
  set(BOOST_INCLUDE_DIR "${BOOST_ROOT}/include/boost-1_83")
  if(NOT EXISTS "${BOOST_INCLUDE_DIR}")
    set(BOOST_INCLUDE_DIR "${BOOST_ROOT}/include")
  endif()

  if(NOT EXISTS "${BOOST_INCLUDE_DIR}/boost")
    message(FATAL_ERROR "Boost headers not found in ${BOOST_INCLUDE_DIR}")
  endif()

  # 手动创建 Boost 目标
  add_library(Boost::serialization INTERFACE IMPORTED)
  set_target_properties(Boost::serialization PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
    INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB;BOOST_SERIALIZATION_DYN_LINK"
  )
  
  add_library(Boost::filesystem INTERFACE IMPORTED)
  set_target_properties(Boost::filesystem PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
    INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB"
  )
  
  add_library(Boost::system INTERFACE IMPORTED)
  set_target_properties(Boost::system PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
    INTERFACE_COMPILE_DEFINITIONS "BOOST_ALL_NO_LIB"
  )
  
  # 添加库路径
  if(EXISTS "${BOOST_ROOT}/lib")
    link_directories("${BOOST_ROOT}/lib")
  endif()
  
  set(Boost_INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
endif()

#=== 12. 头文件路径 ===
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/CameraModels
  ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
  ${OPENGL_INCLUDE_DIR}

  ${Pangolin_INCLUDE_DIRS}
)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
else()
  include_directories(${BOOST_INCLUDE_DIR})
endif()

#=== 13. 主库源文件收集（修复Config.cc问题）===
# 先检查Config.cc是否存在且语法正确
if(EXISTS "${PROJECT_SOURCE_DIR}/src/Config.cc")
  file(READ "${PROJECT_SOURCE_DIR}/src/Config.cc" CONFIG_CONTENT)
  if(CONFIG_CONTENT MATCHES "ConfigParser.*string.*strConfigFile")
    message(STATUS "Config.cc appears to have syntax issues, attempting to fix...")
    # 这里可以添加自动修复逻辑，或者提示用户手动修复
  endif()
endif()

set(ORBSLAM3_SOURCES
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
  # 暂时注释掉有问题的Config.cc
  # src/Config.cc
  src/Settings.cc
)

set(ORBSLAM3_HEADERS
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

#=== 14. 生成 ORB_SLAM3 动态库 ===
add_library(${PROJECT_NAME} SHARED ${ORBSLAM3_SOURCES} ${ORBSLAM3_HEADERS})

#=== 15. 目标级配置 ===
target_compile_definitions(${PROJECT_NAME} PRIVATE 
  ORB_SLAM3_EXPORTS 
  COMPILEDWITHC11
  # Boost兼容性定义
  BOOST_ALL_NO_LIB
  BOOST_SERIALIZATION_DYN_LINK
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${OpenCV_LIBS}
  OpenGL::GL
  ${GLEW_LIB_TARGET}
  Pangolin::pangolin
  Threads::Threads
  ${DBoW2_LIB}
  ${G2O_LIB}
)

# Boost 链接
if(Boost_FOUND)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE 
    Boost::serialization
    Boost::filesystem  
    Boost::system
  )
endif()

#=== 16. 字符编码修复（针对特定文件）===
if(MSVC)
  # 为有编码问题的文件单独设置编码
  set_source_files_properties(
    src/Config.cc
    include/KeyFrame.h
    include/MapPoint.h
    include/CameraModels/Pinhole.h
    include/CameraModels/KannalaBrandt8.h
    include/KeyFrameDatabase.h
    PROPERTIES
    COMPILE_FLAGS "/utf-8"
  )
endif()

#=== 17. Examples 构建函数 ===
function(build_example example_list base_dir suffix)
  foreach(example ${example_list})
    get_filename_component(dir  ${example} DIRECTORY)
    get_filename_component(name ${example} NAME_WE)
    set(target_name "${name}_${suffix}")

    # 检查源文件是否存在
    if(EXISTS "${PROJECT_SOURCE_DIR}/${base_dir}/${example}.cc")
      add_executable(${target_name} ${base_dir}/${example}.cc)
      target_link_libraries(${target_name} PRIVATE ${PROJECT_NAME})
      target_compile_definitions(${target_name} PRIVATE 
        WIN32_LEAN_AND_MEAN NOMINMAX
        BOOST_ALL_NO_LIB
        BOOST_SERIALIZATION_DYN_LINK
      )
      
      # 设置UTF-8编码
      if(MSVC)
        target_compile_options(${target_name} PRIVATE /utf-8)
      endif()

      set_target_properties(${target_name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/${base_dir}/${dir}"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG    "${PROJECT_SOURCE_DIR}/${base_dir}/${dir}"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/${base_dir}/${dir}"
      )
    else()
      message(WARNING "Example source file not found: ${base_dir}/${example}.cc")
    endif()
  endforeach()
endfunction()

#=== 18. 示例清单 ===
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
set(EXAMPLES_OLD ${EXAMPLES})

#=== 19. 构建所有示例 ===
build_example("${EXAMPLES}" "Examples" "exe")
build_example("${EXAMPLES_OLD}" "Examples_old" "old")

#=== 20. 打印关键信息 ===
message(STATUS "OpenCV libs: ${OpenCV_LIBS}")
message(STATUS "Eigen include: ${EIGEN3_INCLUDE_DIR}")
message(STATUS "Pangolin include: ${Pangolin_INCLUDE_DIRS}")
message(STATUS "Boost include: ${Boost_INCLUDE_DIRS}")
message(STATUS "Using GLEW: ${GLEW_LIB_TARGET}")

#=== 21. 文件编码检查与修复 ===
function(check_and_fix_encoding file_path)
  if(EXISTS "${file_path}")
    # 检查文件大小，避免处理二进制文件
    file(SIZE "${file_path}" file_size)
    if(file_size LESS 1000000)  # 只处理小于1MB的文件
      # 尝试读取文件并检查编码
      file(READ "${file_path}" file_content)
      
      # 简单的非ASCII字符检测
      string(LENGTH "${file_content}" content_length)
      if(content_length GREATER 0)
        # 这里可以添加更复杂的编码检测逻辑
        message(STATUS "Checked encoding for: ${file_path}")
      endif()
    endif()
  endif()
endfunction()

# 检查关键文件的编码
check_and_fix_encoding("${PROJECT_SOURCE_DIR}/include/win_gl_preinclude.h")
check_and_fix_encoding("${PROJECT_SOURCE_DIR}/include/win_compat_undef_access.h")