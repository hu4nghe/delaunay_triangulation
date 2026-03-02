#pragma once

#include "geo_3D/point_3D.h"
#include "geo_3D/tetrahedron_3D.h"

#include <array>
#include <cstdint>
#include <vector>

namespace tools_3D
{
    struct surface_mesh
    {
        std::vector<tools_3D::point> vertices;
        std::vector<std::array<std::uint32_t, 3>> triangles;
    };

    auto extract_boundary_surface(const std::vector<tools_3D::tetrahedron>& tetrahedra)
        -> tools_3D::surface_mesh;
}
