cmake_minimum_required(VERSION 2.8)

include(CheckCCompilerFlag)

#
# Add files for openpablo
#
FILE(GLOB SOURCE_FILES
  "main.cpp"
  )
FILE(GLOB HEADER_FILES "*.h" "engine/*.hpp")

set(SOURCES ${SOURCE_FILES} ${HEADER_FILES})


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)


# initial compiler flags
add_definitions("-DHAVE_CONFIG_H")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")


#
# Set default compile flags for GCC
#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fno-strict-aliasing")


#if(NOT APPLE)
  # apple build fails on deprecated warnings..
#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
#endif(NOT APPLE)

if(NOT WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
endif(NOT WIN32)

