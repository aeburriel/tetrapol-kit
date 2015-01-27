cmake_minimum_required (VERSION 2.4)
project (TETRAPOL_kit)

enable_testing ()

set(CMAKE_C_FLAGS "-std=c11 -Og -g -Wall")

add_subdirectory (apps)
add_subdirectory (lib)

