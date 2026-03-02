cmake_minimum_required(VERSION 3.28)
project(DelaunayTriangulation VERSION 1.0)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(modules/csv_parser)
add_subdirectory(modules/geometry_tool)

option(build_tests "Build delaunay triangulation tests" ON)
option(build_vulkan_viewer "Build Vulkan 3D visualization app" ON)

if(build_tests)
    enable_testing()
    add_subdirectory(tests)
endif()

if(build_vulkan_viewer)
    add_subdirectory(apps/vulkan_viewer)
endif()
