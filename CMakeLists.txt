cmake_minimum_required(VERSION 3.17)
project(random-dungeon-generator)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

FIND_PACKAGE(Boost 1.58 COMPONENTS system REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/vstd)

add_executable(random-dungeon-generator main.cpp)
target_link_libraries(random-dungeon-generator ${Boost_LIBRARIES})