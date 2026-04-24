cmake_minimum_required(VERSION 3.17)
project(random-dungeon-generator)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(random-dungeon-generator main.cpp)
target_include_directories(random-dungeon-generator PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_features(random-dungeon-generator PRIVATE cxx_std_23)

add_executable(unit-tests tests/test_rdg.cpp)
target_include_directories(unit-tests PRIVATE ${CMAKE_SOURCE_DIR})
target_compile_features(unit-tests PRIVATE cxx_std_23)

foreach(target_name random-dungeon-generator unit-tests)
  target_compile_options(${target_name} PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Wconversion>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
  )
endforeach()

enable_testing()
add_test(NAME unit-tests COMMAND unit-tests)

target_compile_options(unit-tests PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang>>:-O0 -g --coverage>
)
target_link_options(unit-tests PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU,Clang>>:--coverage>
)
