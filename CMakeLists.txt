cmake_minimum_required(VERSION 3.17)
project(random-dungeon-generator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include_directories(${CMAKE_SOURCE_DIR}/vstd)

add_executable(random-dungeon-generator main.cpp)
