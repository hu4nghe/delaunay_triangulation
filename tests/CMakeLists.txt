include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.10.0
)
FetchContent_MakeAvailable(Catch2)


add_executable(test_parser test_parser.cpp)
target_compile_features(test_parser PRIVATE cxx_std_23)
set_target_properties(test_parser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/bin"
    CXX_EXTENSIONS OFF
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3"
)
target_link_libraries(test_parser
    PRIVATE
        csv_parser
        Catch2::Catch2WithMain
)

add_executable(test_delaunay test_delaunay.cpp)
target_compile_features(test_delaunay PRIVATE cxx_std_23)
set_target_properties(test_delaunay PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/bin"
    CXX_EXTENSIONS OFF
    CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3"
)
target_link_libraries(test_delaunay
    PRIVATE
        csv_parser
        geometry_tool
        delaunay
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(test_parser)
catch_discover_tests(test_delaunay)
