cmake_minimum_required(VERSION 3.10)
project(DBoW2)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall  -O3 -march=native ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall  -O3 -march=native")

set(HDRS_DBOW2
  DBoW2/BowVector.h
  DBoW2/FORB.h 
  DBoW2/FClass.h       
  DBoW2/FeatureVector.h
  DBoW2/ScoringObject.h   
  DBoW2/TemplatedVocabulary.h)
set(SRCS_DBOW2
  DBoW2/BowVector.cpp
  DBoW2/FORB.cpp      
  DBoW2/FeatureVector.cpp
  DBoW2/ScoringObject.cpp)

set(HDRS_DUTILS
  DUtils/Random.h
  DUtils/Timestamp.h)
set(SRCS_DUTILS
  DUtils/Random.cpp
  DUtils/Timestamp.cpp)


# Add this before find_package(OpenCV) to see what's happening
message(STATUS "Looking for OpenCV...")

find_package(OpenCV 4 QUIET)
if(NOT OpenCV_FOUND)
   message(STATUS "OpenCV 4 not found, trying OpenCV 3...")
   find_package(OpenCV 3.0 QUIET)
   if(NOT OpenCV_FOUND)
      # Add more debug info
      message(STATUS "OpenCV directories searched: ${OpenCV_DIR}")
      message(FATAL_ERROR "OpenCV > 3.0 not found. Please set OpenCV_DIR or install OpenCV.")
   endif()
endif()

message(STATUS "OpenCV found: ${OpenCV_VERSION}")
message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")

set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

include_directories(${OpenCV_INCLUDE_DIRS})
add_library(DBoW2 SHARED ${SRCS_DBOW2} ${SRCS_DUTILS})
target_link_libraries(DBoW2 ${OpenCV_LIBS})

