cmake_minimum_required(VERSION 3.28)
project(geometry_tool LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(geometry_tool STATIC)
target_compile_features(geometry_tool PUBLIC cxx_std_23)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(geometry_tool PRIVATE -stdlib=libc++)
    target_link_options(geometry_tool PRIVATE -stdlib=libc++)
endif()

file(GLOB_RECURSE GEO_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/geo_2D/*.cpp"
)

target_sources(geometry_tool
    PRIVATE
        ${GEO_SOURCES}
)

target_include_directories(geometry_tool
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../csv_parser/include>
        $<INSTALL_INTERFACE:include>)

include(GNUInstallDirs)
install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS geometry_tool EXPORT geometryTargets)
install(EXPORT geometryTargets
    FILE geometry_tool.cmake
    NAMESPACE geometry_tool::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/geometry_tool)