cmake_minimum_required(VERSION 3.10)
project(example)

set(SOURCE_FILES src/main.cpp)

include_directories(${CMAKE_SOURCE_DIR}/src)
add_executable(inspect_iuia ${SOURCE_FILES})

