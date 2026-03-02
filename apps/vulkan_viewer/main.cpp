#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

#include "geo_3D/surface_mesh_3D.h"
#include "geo_3D/triangulation_3D.h"

namespace
{
    auto random_cloud_3d(int count, unsigned seed = 42) -> std::vector<tools_3D::point>
    {
        std::mt19937 rng(seed);
        std::uniform_real_distribution<double> dist(-10.0, 10.0);

        std::vector<tools_3D::point> points;
        points.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i)
            points.emplace_back(dist(rng), dist(rng), dist(rng));
        return points;
    }

    auto create_instance() -> VkInstance
    {
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "Delaunay Vulkan Viewer";
        app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        app_info.pEngineName = "No Engine";
        app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        uint32_t extension_count = 0;
        const char** extensions = glfwGetRequiredInstanceExtensions(&extension_count);

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = extension_count;
        create_info.ppEnabledExtensionNames = extensions;

        VkInstance instance = VK_NULL_HANDLE;
        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
            throw std::runtime_error("failed to create Vulkan instance");

        return instance;
    }
}

int main()
{
    const auto cloud_points = random_cloud_3d(120, 2026);
    const auto tetrahedra = tools_3D::bowyer_watson_3D(cloud_points);
    const auto surface = tools_3D::extract_boundary_surface(tetrahedra);

    std::cout << "3D points: " << cloud_points.size() << '\n';
    std::cout << "tetrahedra: " << tetrahedra.size() << '\n';
    std::cout << "surface vertices: " << surface.vertices.size() << '\n';
    std::cout << "surface triangles: " << surface.triangles.size() << '\n';

    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error("failed to init GLFW");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Delaunay Vulkan Viewer", nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("failed to create GLFW window");
    }

    const VkInstance instance = create_instance();

    while (glfwWindowShouldClose(window) == GLFW_FALSE)
    {
        glfwPollEvents();
    }

    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
