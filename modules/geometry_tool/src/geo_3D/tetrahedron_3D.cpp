#include "geo_3D/tetrahedron_3D.h"

namespace tools_3D
{
    tetrahedron::tetrahedron(
        const tools_3D::point& a,
        const tools_3D::point& b,
        const tools_3D::point& c,
        const tools_3D::point& d)
        : vertices_{
              a,
              b,
              c,
              d}
    {}

    auto tetrahedron::vertices() const -> const std::array<
        tools_3D::point,
        4>&
    {
        return vertices_;
    }

    auto tetrahedron::contains_vertex(const tools_3D::point& p) const -> bool
    {
        for (const auto& v : vertices_)
            if (v == p) return true;
        return false;
    }
} // namespace tools_3D
