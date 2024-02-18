cmake_minimum_required(VERSION 3.13)
project(OptAlg VERSION 1.0.0 LANGUAGES CXX)

add_library(optalg SHARED source/boxing.cpp source/boxing_greedy.cpp)

add_executable(optalg_cmd source/cmd.cpp)
target_link_libraries(optalg_cmd PUBLIC optalg)