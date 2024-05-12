cmake_minimum_required(VERSION 3.1)
project(MyProj)

find_package(LLVM REQUIRED CONFIG)
add_definitions(${LLVM_DEFINITIONS})

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})

set(FRAMEWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/framework")
set(DETECTOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/detector")
set(TYPE_DETERMINATOR_DIR "${FRAMEWORK_DIR}/type_determinator")

add_subdirectory(framework)
add_subdirectory(detector)
