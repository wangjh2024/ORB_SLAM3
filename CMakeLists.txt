cmake_minimum_required(VERSION 3.20)
project(ORB_SLAM3)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 平台设置
if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS)
    add_compile_options(/W3 /wd4267 /wd4244 /wd4996)
endif()

# 使用 vcpkg 工具链
set(CMAKE_TOOLCHAIN_FILE "E:/3rd_party/vcpkg/scripts/buildsystems/vcpkg.cmake")

# 设置 vcpkg 安装目录到 CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH "E:/3rd_party/vcpkg/installed/x64-windows")

# 查找依赖
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(Pangolin REQUIRED)
find_package(Boost REQUIRED COMPONENTS serialization)
find_package(Threads REQUIRED)

# 查找 Sophus 和 g2o
find_package(Sophus REQUIRED)
find_package(G2O REQUIRED)

# 包含目录
include_directories(
    ${Sophus_INCLUDE_DIRS}  # vcpkg 的 Sophus 在前
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/CameraModels
    ${Eigen3_INCLUDE_DIRS}
    ${Pangolin_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${G2O_INCLUDE_DIRS}
)

# 手动设置库路径（如果自动查找失败）
set(VCPKG_LIB_DIR "E:/3rd_party/vcpkg/installed/x64-windows/lib")
set(VCPKG_DEBUG_LIB_DIR "E:/3rd_party/vcpkg/installed/x64-windows/debug/lib")

# 查找源文件
file(GLOB_RECURSE SOURCES "src/*.cc" "src/*.cpp")

# 创建库
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# 链接库 - 修复目标名称问题
target_link_libraries(${PROJECT_NAME}
    ${OpenCV_LIBS}
    ${Pangolin_LIBRARIES}  # 改为变量而不是目标
    ${Boost_LIBRARIES}
    Threads::Threads
)

# 手动添加 Sophus 和 g2o 的链接（如果自动查找失败）
if(Sophus_FOUND AND Sophus_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} ${Sophus_LIBRARIES})
else()
    # 手动查找 Sophus 库
    find_library(SOPHUS_LIB 
        NAMES Sophus sophus
        PATHS ${VCPKG_LIB_DIR} ${VCPKG_DEBUG_LIB_DIR}
    )
    if(SOPHUS_LIB)
        target_link_libraries(${PROJECT_NAME} ${SOPHUS_LIB})
        message(STATUS "Found Sophus library: ${SOPHUS_LIB}")
    else()
        message(WARNING "Sophus library not found, trying to continue...")
    endif()
endif()

if(G2O_FOUND AND G2O_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} ${G2O_LIBRARIES})
else()
    # 手动查找 g2o 库
    find_library(G2O_CORE_LIB 
        NAMES g2o_core g2o
        PATHS ${VCPKG_LIB_DIR} ${VCPKG_DEBUG_LIB_DIR}
    )
    find_library(G2O_STUFF_LIB 
        NAMES g2o_stuff
        PATHS ${VCPKG_LIB_DIR} ${VCPKG_DEBUG_LIB_DIR}
    )
    if(G2O_CORE_LIB AND G2O_STUFF_LIB)
        target_link_libraries(${PROJECT_NAME} ${G2O_CORE_LIB} ${G2O_STUFF_LIB})
        message(STATUS "Found g2o libraries: ${G2O_CORE_LIB}, ${G2O_STUFF_LIB}")
    endif()
endif()

# 添加编译定义
target_compile_definitions(${PROJECT_NAME} PRIVATE
    _CRT_SECURE_NO_WARNINGS
    _SCL_SECURE_NO_WARNINGS
)

# 调试信息
message(STATUS "Pangolin libraries: ${Pangolin_LIBRARIES}")
message(STATUS "Sophus libraries: ${Sophus_LIBRARIES}")
message(STATUS "G2O libraries: ${G2O_LIBRARIES}")

message(STATUS "ORB_SLAM3 configuration completed!")