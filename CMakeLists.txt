cmake_minimum_required(VERSION 3.13)
project(OptAlg VERSION 1.0.0 LANGUAGES CXX)

add_library(optalg SHARED
source/boxing.cpp
source/boxing_greedy.cpp
source/boxing_neighborhood_geometry.cpp
source/boxing_neighborhood_order.cpp
source/boxing_neighborhood_geometry_overlap.cpp)
if (DEBUG_OVERLAPS)
    target_compile_definitions(optalg PRIVATE DEBUG_OVERLAPS)
endif()

add_executable(optalg_cmd source/cmd.cpp)
target_link_libraries(optalg_cmd PRIVATE optalg)

find_package(wxWidgets REQUIRED COMPONENTS core base)
if (wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE})
endif()
add_executable(optalg_gui source/gui.cpp)
target_link_libraries(optalg_gui PRIVATE ${wxWidgets_LIBRARIES})
target_link_libraries(optalg_gui PRIVATE optalg)