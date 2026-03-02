#pragma once

#include <tuple>

namespace tools_3D
{
    class point
    {
        double x_;
        double y_;
        double z_;

    public:
        point();
        point(double x, double y, double z);

        auto operator==(const point& other) const -> bool;
        auto operator!=(const point& other) const -> bool;
        auto operator<(const point& other) const -> bool;

        auto get_x() const -> double;
        auto get_y() const -> double;
        auto get_z() const -> double;

        auto as_tuple() const -> std::tuple<double, double, double>;
    };
}
