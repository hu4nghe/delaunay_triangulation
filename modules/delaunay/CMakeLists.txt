include(FetchContent)

FetchContent_Declare(
    geometry_tool
    GIT_REPOSITORY https://github.com/hu4nghe/geometry_tool.git
    GIT_TAG main
)
FetchContent_MakeAvailable(geometry_tool)

add_library(delaunay STATIC src/delaunay.cpp)

target_link_libraries(delaunay
    PRIVATE
    csv_parser
    geometry_tool
)

target_compile_features(delaunay PRIVATE cxx_std_23)

set_target_properties(delaunay PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    CXX_EXTENSIONS OFF
)
target_include_directories(delaunay
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)   