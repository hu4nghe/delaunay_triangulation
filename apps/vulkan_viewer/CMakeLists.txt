find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(unofficial-shaderc CONFIG REQUIRED)

add_executable(vulkan_viewer
    src/main.cpp
    src/viewer_app.cpp
    src/vulkan_renderer.cpp
    src/scene_bridge.cpp
    src/camera_controller.cpp
)
target_compile_features(vulkan_viewer PRIVATE cxx_std_23)

target_link_libraries(vulkan_viewer
    PRIVATE
        geometry_tool
        Vulkan::Vulkan
        glfw
        glm::glm-header-only
        unofficial::shaderc::shaderc
)

target_include_directories(vulkan_viewer
    PRIVATE
        ${CMAKE_SOURCE_DIR}/modules/geometry_tool/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
