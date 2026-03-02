#pragma once

#include "geo_3D/point_3D.h"

namespace tools_3D
{
    inline constexpr double geometric_epsilon_3D = 1e-12;

    auto orient3d(const tools_3D::point& a, const tools_3D::point& b, const tools_3D::point& c, const tools_3D::point& d) -> double;

    auto in_sphere(
        const tools_3D::point& a,
        const tools_3D::point& b,
        const tools_3D::point& c,
        const tools_3D::point& d,
        const tools_3D::point& e
    ) -> bool;
}
