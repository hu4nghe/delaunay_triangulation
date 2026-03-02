#pragma once

#include "geo_3D/point_3D.h"

#include <array>

namespace tools_3D
{
    class tetrahedron
    {
        std::array<tools_3D::point, 4> vertices_;

    public:
        tetrahedron(const tools_3D::point& a, const tools_3D::point& b, const tools_3D::point& c, const tools_3D::point& d);

        auto vertices() const -> const std::array<tools_3D::point, 4>&;
        auto contains_vertex(const tools_3D::point& p) const -> bool;
    };
}
