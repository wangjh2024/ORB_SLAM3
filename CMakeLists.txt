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
set(G2O_LIB    "${PROJECT_SOURCE_DIR}/Thirdparty/g2o/bin/Release/g2o.lib")

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

#=== 2. MSVC 专项优化与“强制预包含” ===
if(MSVC)
  add_compile_options(
    /permissive- /MP /utf-8 /bigobj
    /wd4244 /wd4267 /wd4305 /wd4996 /wd4820 /wd4464
  )
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi /Ot /arch:AVX2")

  # —— 目录级强制预包含（关键修复点）——
  set(WIN_FORCE_INCLUDE "${PROJECT_SOURCE_DIR}/include/win_gl_preinclude.h")
  set(WIN_UNDEF_ACCESS  "${PROJECT_SOURCE_DIR}/include/win_compat_undef_access.h")
  add_compile_options(/FI"${WIN_FORCE_INCLUDE}" /FI"${WIN_UNDEF_ACCESS}")
endif()

# 改进的强制预包含配置
if(MSVC)
  # 检查预包含头文件是否存在
  if(EXISTS "${WIN_FORCE_INCLUDE}" AND EXISTS "${WIN_UNDEF_ACCESS}")
    add_compile_options(/FI"${WIN_FORCE_INCLUDE}" /FI"${WIN_UNDEF_ACCESS}")
  else()
    message(WARNING "Force include headers not found, Windows GL conflicts may occur")
  endif()
endif()

#=== 3. 全局宏定义（修复 Windows + OpenGL 头文件冲突） ===
add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_USE_MATH_DEFINES)
add_definitions(-DEIGEN_MAKE_ALIGNED_OPERATOR_NEW -D_CRT_SECURE_NO_WARNINGS)

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
find_package(OpenGL REQUIRED)

find_package(GLEW QUIET)
if(GLEW_FOUND)
  message(STATUS "GLEW found via CMake")
  if(TARGET GLEW::GLEW)
    set(GLEW_LIB_TARGET GLEW::GLEW)
  elseif(GLEW_LIBRARY AND GLEW_INCLUDE_DIRS)
    include_directories(${GLEW_INCLUDE_DIRS})
    set(GLEW_LIB_TARGET ${GLEW_LIBRARY})
  endif()
else()
  message(WARNING "GLEW config not found; fallback to manual include/link")
  include_directories(${GLEW_ROOT}/include)
  if(EXISTS "${GLEW_ROOT}/lib/Release/x64/glew32.lib")
    link_directories(${GLEW_ROOT}/lib/Release/x64)
    set(GLEW_LIB_TARGET glew32)
  elseif(EXISTS "${GLEW_ROOT}/lib/glew32.lib")
    link_directories(${GLEW_ROOT}/lib)
    set(GLEW_LIB_TARGET glew32)
  else()
    message(FATAL_ERROR "Cannot locate GLEW library under ${GLEW_ROOT}")
  endif()
endif()

#=== 10. 依赖：Pangolin（若找不到，手动建接口目标） ===
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

#=== 11. 依赖：Boost（统一配置） ===
set(BOOST_ROOT "E:/3rd_party/boost_1_83_0")

# 定义需要的组件
set(REQUIRED_BOOST_LIBS serialization filesystem system)

# 优先使用 CMake 的 FindBoost
find_package(Boost 1.83.0 QUIET COMPONENTS ${REQUIRED_BOOST_LIBS})

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

  # 为每个组件创建接口目标
  foreach(component ${REQUIRED_BOOST_LIBS})
    add_library(Boost::${component} INTERFACE IMPORTED)
    set_target_properties(Boost::${component} PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${BOOST_INCLUDE_DIR}"
    )
    
    # 如果有编译的库文件，添加链接
    if(EXISTS "${BOOST_ROOT}/lib")
      link_directories("${BOOST_ROOT}/lib")
      # 尝试查找具体的库文件
      if(MSVC)
        set(lib_suffix "vc143-mt-x64-1_83.lib")  # 根据你的编译环境调整
        if(EXISTS "${BOOST_ROOT}/lib/libboost_${component}${lib_suffix}")
          set_property(TARGET Boost::${component} PROPERTY
            INTERFACE_LINK_LIBRARIES "libboost_${component}${lib_suffix}")
        endif()
      endif()
    endif()
  endforeach()
  
  set(Boost_INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
endif()

#=== 12. 头文件路径（使用传统的 include_directories） ===
include_directories(
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/CameraModels
  ${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
  ${Pangolin_INCLUDE_DIRS}
)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
else()
  include_directories(${BOOST_INCLUDE_DIR})
endif()

#=== 13. 主库源文件收集 ===
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
  src/Config.cc
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

#=== 15. 宏与链接（目标级） ===
#=== 15. 宏与链接（目标级） ===
target_compile_definitions(${PROJECT_NAME} PRIVATE 
  ORB_SLAM3_EXPORTS 
  COMPILEDWITHC11
)

# 使用 target_include_directories 添加私有头文件路径
target_include_directories(${PROJECT_NAME} PRIVATE
  ${PROJECT_SOURCE_DIR}/include/CameraModels
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
  foreach(component ${REQUIRED_BOOST_LIBS})
    if(TARGET Boost::${component})
      target_link_libraries(${PROJECT_NAME} PRIVATE Boost::${component})
    endif()
  endforeach()
endif()
#=== 16. Examples 构建函数 ===
function(build_example example_list base_dir suffix)
  foreach(example ${example_list})
    get_filename_component(dir  ${example} DIRECTORY)
    get_filename_component(name ${example} NAME_WE)
    set(target_name "${name}_${suffix}")

    add_executable(${target_name} ${base_dir}/${example}.cc)
    target_link_libraries(${target_name} PRIVATE ${PROJECT_NAME})
    target_compile_definitions(${target_name} PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)

    set_target_properties(${target_name} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY          "${PROJECT_SOURCE_DIR}/${base_dir}/${dir}"
      RUNTIME_OUTPUT_DIRECTORY_DEBUG    "${PROJECT_SOURCE_DIR}/${base_dir}/${dir}"
      RUNTIME_OUTPUT_DIRECTORY_RELEASE  "${PROJECT_SOURCE_DIR}/${base_dir}/${dir}"
    )
  endforeach()
endfunction()

#=== 17. 示例清单 ===
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

#=== 18. 构建所有示例 ===
build_example("${EXAMPLES}" "Examples" "exe")
build_example("${EXAMPLES_OLD}" "Examples_old" "old")

#=== 19. 打印关键信息 ===
#=== 19. 依赖验证 ===
# 检查必要的头文件是否存在
foreach(header ${ORBSLAM3_HEADERS})
  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/${header}")
    message(WARNING "Header file not found: ${header}")
  endif()
endforeach()

# 检查第三方库文件
if(NOT EXISTS "${DBoW2_LIB}")
  message(FATAL_ERROR "DBoW2 library not found: ${DBoW2_LIB}")
endif()

if(NOT EXISTS "${G2O_LIB}")
  message(FATAL_ERROR "g2o library not found: ${G2O_LIB}")
endif()