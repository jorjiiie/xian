cmake_minimum_required(VERSION 3.2)

set (CMAKE_CXX_STANDARD 17)

project(xian)

include_directories(src)


add_subdirectory(src/miao)


add_executable(test src/main.cpp)
target_link_libraries(test PRIVATE miao)
