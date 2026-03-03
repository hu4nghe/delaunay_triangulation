#pragma once

#include "camera_controller.h"
#include "scene_bridge.h"
#include "vulkan_renderer.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class ViewerApp
{
    public:

    void run();

    private:

    GLFWwindow*      window_{};
    SceneBridge      scene_;
    CameraController camera_;
    VulkanRenderer   renderer_;

    void initWindow();
    void initRenderer();
    void mainLoop();
    void cleanup();

    void rebuildScene();

    static void framebufferResizeCallback(
        GLFWwindow* window,
        int         width,
        int         height);
    static void cursorPosCallback(
        GLFWwindow* window,
        double      x,
        double      y);
    static void mouseButtonCallback(
        GLFWwindow* window,
        int         button,
        int         action,
        int         mods);
    static void scrollCallback(
        GLFWwindow* window,
        double      xOffset,
        double      yOffset);
    static void keyCallback(
        GLFWwindow* window,
        int         key,
        int         scancode,
        int         action,
        int         mods);
};
