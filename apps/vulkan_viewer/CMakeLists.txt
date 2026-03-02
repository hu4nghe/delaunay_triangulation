find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_executable(vulkan_viewer main.cpp)
target_compile_features(vulkan_viewer PRIVATE cxx_std_23)

target_link_libraries(vulkan_viewer
    PRIVATE
        geometry_tool
        Vulkan::Vulkan
        glfw
)

target_include_directories(vulkan_viewer
    PRIVATE
        ${CMAKE_SOURCE_DIR}/modules/geometry_tool/include
)
