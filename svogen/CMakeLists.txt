cmake_minimum_required(VERSION 3.27)
project(svogen)

set(CMAKE_CXX_STANDARD 23)

add_library(libsvogen INTERFACE)
target_include_directories(libsvogen INTERFACE svogen/)
target_link_libraries(libsvogen INTERFACE glm)

add_executable(svogen svogen.cpp voxelizer.h tiny_obj_loader.h)
target_link_libraries(svogen PRIVATE libsvogen)
