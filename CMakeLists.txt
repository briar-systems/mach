cmake_minimum_required(VERSION 3.10)
project(mach)

set(SRC_DIR src)
set(INCLUDE_DIR src)
file(GLOB SRCS ${SRC_DIR}/*.c)

include_directories(${INCLUDE_DIR})

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type specified, defaulting to Release")
    set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
else()
    message(STATUS "Release build")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
endif()
    
add_executable(${PROJECT_NAME} ${SRCS})
