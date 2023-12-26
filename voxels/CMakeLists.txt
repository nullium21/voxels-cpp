cmake_minimum_required(VERSION 3.27)
project(voxels)

set(CMAKE_CXX_STANDARD 23)

add_executable(voxels main.cpp)
target_link_libraries(voxels PRIVATE libsvogen glm stb)
