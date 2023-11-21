cmake_minimum_required(VERSION 3.26)
project(graph)

set(CMAKE_CXX_STANDARD 23)

add_library(directed_graph directed_graph.h)
set_target_properties(directed_graph PROPERTIES LINKER_LANGUAGE CXX)
add_executable(graph main.cpp)
