cmake_minimum_required(VERSION 3.10)
project(mach C)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

file(GLOB SOURCES "src/*.c")

add_executable(mach ${SOURCES})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -std=c99")
if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
endif()

target_link_libraries(mach PRIVATE LLVM)

if(NOT TARGET LLVM)
  message(STATUS "Falling back to explicit LLVM component linking")
  llvm_map_components_to_libnames(llvm_libs
    Core
    Support
    BitWriter
    Analysis
    Target
    TransformUtils
    InstCombine
    ScalarOpts
    ipo
    CodeGen
    MC
    MCParser
    MCDisassembler
    Option
    Passes
    native
    )
  target_link_libraries(mach PRIVATE ${llvm_libs})
endif()
