#include "camera_controller.h"

#include <GLFW/glfw3.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <cmath>

void CameraController::onMouseButton(
    int button,
    int action)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) dragging_ = (action == GLFW_PRESS);
}

void CameraController::onMouseMove(
    double x,
    double y)
{
    if (!dragging_)
    {
        last_x_ = x;
        last_y_ = y;
        return;
    }

    const float dx = static_cast<float>(x - last_x_);
    const float dy = static_cast<float>(y - last_y_);

    yaw_   += dx * 0.005f;
    pitch_ += dy * 0.005f;

    if (pitch_ > 1.4f) pitch_ = 1.4f;
    if (pitch_ < -1.4f) pitch_ = -1.4f;

    last_x_ = x;
    last_y_ = y;
}

void CameraController::onScroll(double yOffset)
{
    distance_ -= static_cast<float>(yOffset) * 0.2f;
    if (distance_ < 0.5f) distance_ = 0.5f;
    if (distance_ > 20.0f) distance_ = 20.0f;
}

auto CameraController::viewProjection(float aspect) const -> glm::mat4
{
    const glm::vec3 eye(
        distance_ * std::cos(pitch_) * std::sin(yaw_),
        distance_ * std::sin(pitch_),
        distance_ * std::cos(pitch_) * std::cos(yaw_));

    const glm::mat4 view =
        glm::lookAt(eye, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 proj =
        glm::perspective(glm::radians(60.0f), aspect, 0.01f, 100.0f);
    proj[1][1] *= -1.0f;
    return proj * view;
}
