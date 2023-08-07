cmake_minimum_required(VERSION 3.2)
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS")

set (CMAKE_CXX_STANDARD 17)

project(xian)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(src)


add_subdirectory(src/miao)
message("ARCHS ${CMAKE_OSX_ARCHITECTURES}")
message("CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
add_executable(test src/main.cpp)
target_link_libraries(test miao)
