#include "geo_3D/point_3D.h"

namespace tools_3D
{
    point::point()
        : x_(0.0),
          y_(0.0),
          z_(0.0)
    {}

    point::point(
        double x,
        double y,
        double z)
        : x_(x),
          y_(y),
          z_(z)
    {}

    auto point::operator==(const point& other) const -> bool
    {
        return x_ == other.x_ && y_ == other.y_ && z_ == other.z_;
    }

    auto point::operator!=(const point& other) const -> bool
    {
        return !(*this == other);
    }

    auto point::operator<(const point& other) const -> bool
    {
        if (x_ != other.x_) return x_ < other.x_;
        if (y_ != other.y_) return y_ < other.y_;
        return z_ < other.z_;
    }

    auto point::get_x() const -> double
    {
        return x_;
    }
    auto point::get_y() const -> double
    {
        return y_;
    }
    auto point::get_z() const -> double
    {
        return z_;
    }

    auto point::as_tuple() const -> std::tuple<
        double,
        double,
        double>
    {
        return {x_, y_, z_};
    }
} // namespace tools_3D
