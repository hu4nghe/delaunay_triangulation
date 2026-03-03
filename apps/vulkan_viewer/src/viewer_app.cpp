#include "viewer_app.h"

#include <glm/mat4x4.hpp>

#include <print>
#include <stdexcept>

void ViewerApp::run()
{
    initWindow();
    initRenderer();
    mainLoop();
    cleanup();
}

void ViewerApp::initWindow()
{
    if (glfwInit() == GLFW_FALSE)
        throw std::runtime_error("failed to initialize GLFW");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window_ = glfwCreateWindow(
        1280,
        720,
        "Delaunay 3D Vulkan Viewer",
        nullptr,
        nullptr);
    if (window_ == nullptr) throw std::runtime_error("failed to create window");

    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
    glfwSetCursorPosCallback(window_, cursorPosCallback);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);
    glfwSetScrollCallback(window_, scrollCallback);
    glfwSetKeyCallback(window_, keyCallback);
}

void ViewerApp::initRenderer()
{
    renderer_.initialize(window_);
    rebuildScene();
}

void ViewerApp::mainLoop()
{
    while (!glfwWindowShouldClose(window_))
    {
        glfwPollEvents();
        const glm::mat4 mvp = camera_.viewProjection(renderer_.aspectRatio());
        renderer_.draw(mvp);
    }
}

void ViewerApp::cleanup()
{
    renderer_.cleanup();

    if (window_ != nullptr) glfwDestroyWindow(window_);

    glfwTerminate();
}

void ViewerApp::rebuildScene()
{
    scene_.rebuild();
    renderer_.setMesh(scene_.mesh());

    std::println(
        "Algorithm: {} | points: {} | vertices: {} | triangles: {}",
        scene_.algorithm() == SceneBridge::Algorithm::BowyerWatson
            ? "Bowyer-Watson"
            : "Guibas-Stolfi",
        scene_.pointCount(),
        scene_.mesh().positions.size(),
        scene_.mesh().indices.size() / 3);
}

void ViewerApp::framebufferResizeCallback(
    GLFWwindow* window,
    int,
    int)
{
    auto* app = static_cast<ViewerApp*>(glfwGetWindowUserPointer(window));
    if (app != nullptr) app->renderer_.setFramebufferResized();
}

void ViewerApp::cursorPosCallback(
    GLFWwindow* window,
    double      x,
    double      y)
{
    auto* app = static_cast<ViewerApp*>(glfwGetWindowUserPointer(window));
    if (app != nullptr) app->camera_.onMouseMove(x, y);
}

void ViewerApp::mouseButtonCallback(
    GLFWwindow* window,
    int         button,
    int         action,
    int)
{
    auto* app = static_cast<ViewerApp*>(glfwGetWindowUserPointer(window));
    if (app != nullptr) app->camera_.onMouseButton(button, action);
}

void ViewerApp::scrollCallback(
    GLFWwindow* window,
    double,
    double yOffset)
{
    auto* app = static_cast<ViewerApp*>(glfwGetWindowUserPointer(window));
    if (app != nullptr) app->camera_.onScroll(yOffset);
}

void ViewerApp::keyCallback(
    GLFWwindow* window,
    int         key,
    int,
    int action,
    int)
{
    if (action != GLFW_PRESS) return;

    auto* app = static_cast<ViewerApp*>(glfwGetWindowUserPointer(window));
    if (app == nullptr) return;

    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }

    if (key == GLFW_KEY_1)
    {
        app->scene_.setAlgorithm(SceneBridge::Algorithm::BowyerWatson);
        app->rebuildScene();
        return;
    }

    if (key == GLFW_KEY_2)
    {
        app->scene_.setAlgorithm(SceneBridge::Algorithm::GuibasStolfi);
        app->rebuildScene();
        return;
    }

    if (key == GLFW_KEY_R)
    {
        app->scene_.reseed();
        app->rebuildScene();
        return;
    }

    if (key == GLFW_KEY_UP)
    {
        app->scene_.setPointCount(app->scene_.pointCount() + 20);
        app->rebuildScene();
        return;
    }

    if (key == GLFW_KEY_DOWN)
    {
        app->scene_.setPointCount(app->scene_.pointCount() - 20);
        app->rebuildScene();
    }
}
