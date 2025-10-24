add_library(csv_parser STATIC src/csv_parser.cpp)

target_compile_features(csv_parser PRIVATE cxx_std_23)

set_target_properties(csv_parser PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    CXX_EXTENSIONS OFF
)

target_include_directories(csv_parser
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)   
