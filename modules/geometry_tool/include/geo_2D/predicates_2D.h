#pragma once

#include "geo_2D/point_2D.h"

namespace tools_2D
{
    inline constexpr double geometric_epsilon = 1e-12;

    auto orient2d(const tools_2D::point& a, const tools_2D::point& b, const tools_2D::point& c) -> double;

    auto in_circle(
        const tools_2D::point& a,
        const tools_2D::point& b,
        const tools_2D::point& c,
        const tools_2D::point& d
    ) -> bool;
}
