cmake_minimum_required(VERSION 3.17)
project(random-dungeon-generator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories(${CMAKE_SOURCE_DIR})

add_executable(random-dungeon-generator main.cpp)

enable_testing()
add_executable(unit-tests tests/test_rdg.cpp)
add_test(NAME unit-tests COMMAND unit-tests)

target_include_directories(unit-tests PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_options(unit-tests PRIVATE -O0 -g --coverage)
target_link_options(unit-tests PRIVATE --coverage)
