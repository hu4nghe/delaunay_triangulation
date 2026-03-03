#include "geo_3D/surface_mesh_3D.h"

#include <algorithm>
#include <map>
#include <unordered_map>
#include <utility>

namespace
{
    struct face_key
    {
        tools_3D::point a;
        tools_3D::point b;
        tools_3D::point c;

        auto operator==(const face_key& other) const -> bool
        {
            return a == other.a && b == other.b && c == other.c;
        }
    };

    struct face_key_hash
    {
        auto operator()(const face_key& key) const noexcept -> std::size_t
        {
            auto hash_point = [](const tools_3D::point& p) -> std::size_t
            {
                auto [x, y, z]           = p.as_tuple();
                const std::size_t hash_x = std::hash<double>{}(x);
                const std::size_t hash_y = std::hash<double>{}(y);
                const std::size_t hash_z = std::hash<double>{}(z);
                return hash_x ^ (hash_y << 1) ^ (hash_z << 2);
            };

            return hash_point(key.a) ^ (hash_point(key.b) << 1) ^
                   (hash_point(key.c) << 2);
        }
    };

    auto canonical_face(
        const tools_3D::point& p0,
        const tools_3D::point& p1,
        const tools_3D::point& p2) -> face_key
    {
        std::array<tools_3D::point, 3> points{p0, p1, p2};
        std::sort(points.begin(), points.end());
        return {points[0], points[1], points[2]};
    }
} // namespace

namespace tools_3D
{
    auto extract_boundary_surface(
        const std::vector<tools_3D::tetrahedron>& tetrahedra)
        -> tools_3D::surface_mesh
    {
        std::unordered_map<
            face_key,
            std::pair<int, std::array<tools_3D::point, 3>>,
            face_key_hash>
            face_usage;

        for (const auto& tetrahedron : tetrahedra)
        {
            const auto& vertices = tetrahedron.vertices();
            const std::array<std::array<int, 3>, 4> faces{
                {{{0, 1, 2}}, {{0, 1, 3}}, {{0, 2, 3}}, {{1, 2, 3}}}};

            for (const auto& face : faces)
            {
                const auto& p0  = vertices[face[0]];
                const auto& p1  = vertices[face[1]];
                const auto& p2  = vertices[face[2]];
                const auto  key = canonical_face(p0, p1, p2);

                auto it = face_usage.find(key);
                if (it == face_usage.end())
                {
                    face_usage.emplace(
                        key,
                        std::make_pair(
                            1,
                            std::array<tools_3D::point, 3>{p0, p1, p2}));
                }
                else
                {
                    it->second.first += 1;
                }
            }
        }

        tools_3D::surface_mesh                   mesh;
        std::map<tools_3D::point, std::uint32_t> vertex_to_index;

        auto get_or_add_index =
            [&](const tools_3D::point& point) -> std::uint32_t
        {
            auto found = vertex_to_index.find(point);
            if (found != vertex_to_index.end()) return found->second;

            const auto index = static_cast<std::uint32_t>(mesh.vertices.size());
            mesh.vertices.push_back(point);
            vertex_to_index.emplace(point, index);
            return index;
        };

        for (const auto& [_, counted_face] : face_usage)
        {
            if (counted_face.first != 1) continue;

            const auto&         face = counted_face.second;
            const std::uint32_t i0   = get_or_add_index(face[0]);
            const std::uint32_t i1   = get_or_add_index(face[1]);
            const std::uint32_t i2   = get_or_add_index(face[2]);
            mesh.triangles.push_back({i0, i1, i2});
        }

        return mesh;
    }
} // namespace tools_3D
