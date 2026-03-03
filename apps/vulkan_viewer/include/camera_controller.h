#pragma once

#include <glm/mat4x4.hpp>

class CameraController
{
    public:

    void onMouseButton(
        int button,
        int action);
    void onMouseMove(
        double x,
        double y);
    void onScroll(double yOffset);

    auto viewProjection(float aspect) const -> glm::mat4;

    private:

    double last_x_{};
    double last_y_{};
    bool   dragging_{false};

    float yaw_{0.8f};
    float pitch_{0.5f};
    float distance_{3.0f};
};
