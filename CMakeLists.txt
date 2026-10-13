cmake_minimum_required(VERSION 4.0.2)
project(random-dungeon-generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/RequireCxx26.cmake)

add_library(rdg INTERFACE)
target_include_directories(rdg INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(rdg INTERFACE cxx_std_26)

add_executable(random-dungeon-generator main.cpp)
target_link_libraries(random-dungeon-generator PRIVATE rdg)

add_executable(unit-tests tests/test_rdg.cpp)
target_link_libraries(unit-tests PRIVATE rdg)

foreach(target_name random-dungeon-generator unit-tests)
  target_compile_options(${target_name} PRIVATE
    $<$<CXX_COMPILER_FRONTEND_VARIANT:GNU>:-Wall -Wextra -Wpedantic -Wconversion>
    $<$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>:/W4 /permissive->
  )
endforeach()

enable_testing()
add_test(NAME unit-tests COMMAND unit-tests)

target_compile_options(unit-tests PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_FRONTEND_VARIANT:GNU>>:-O0 -g --coverage>
)
target_link_options(unit-tests PRIVATE
  $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_FRONTEND_VARIANT:GNU>>:--coverage>
)
