#include "../include/viewer_app.h"

#include <exception>
#include <print>

int main()
{
    try
    {
        ViewerApp app;
        app.run();
        return 0;
    }
    catch (const std::exception& e)
    {
        std::println(stderr, "vulkan_viewer failed: {}", e.what());
        return 1;
    }
}
