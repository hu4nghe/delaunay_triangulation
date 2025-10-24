cmake_minimum_required(VERSION 3.28)
project(DelaunayTriangulation VERSION 1.0)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(modules/csv_parser)
add_subdirectory(modules/delaunay)

option(build_tests "Build lockfree_queue tests" ON)

if(build_tests)
    enable_testing()
    add_subdirectory(tests)
endif()
