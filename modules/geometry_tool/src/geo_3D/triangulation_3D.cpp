#include "geo_3D/triangulation_3D.h"

#include "geo_3D/predicates_3D.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace
{
    struct tetra_idx
    {
        int a;
        int b;
        int c;
        int d;
    };

    struct face_idx
    {
        int i;
        int j;
        int k;
    };

    struct face_key
    {
        int a;
        int b;
        int c;

        auto operator==(const face_key& other) const -> bool
        {
            return a == other.a && b == other.b && c == other.c;
        }
    };

    struct face_key_hash
    {
        auto operator()(const face_key& key) const noexcept -> std::size_t
        {
            return (static_cast<std::size_t>(key.a) * 73856093u) ^
                   (static_cast<std::size_t>(key.b) * 19349663u) ^
                   (static_cast<std::size_t>(key.c) * 83492791u);
        }
    };

    auto make_face_key(
        int i,
        int j,
        int k) -> face_key
    {
        std::array<int, 3> arr{i, j, k};
        std::sort(arr.begin(), arr.end());
        return {arr[0], arr[1], arr[2]};
    }

    auto tetra_faces(const tetra_idx& t) -> std::array<
        face_idx,
        4>
    {
        return {
            {{t.a, t.b, t.c},
             {t.a, t.b, t.d},
             {t.a, t.c, t.d},
             {t.b, t.c, t.d}}};
    }

    auto super_tetra(const std::vector<tools_3D::point>& points) -> std::array<
        tools_3D::point,
        4>
    {
        double min_x = points.front().get_x();
        double max_x = min_x;
        double min_y = points.front().get_y();
        double max_y = min_y;
        double min_z = points.front().get_z();
        double max_z = min_z;

        for (const auto& p : points)
        {
            min_x = std::min(min_x, p.get_x());
            max_x = std::max(max_x, p.get_x());
            min_y = std::min(min_y, p.get_y());
            max_y = std::max(max_y, p.get_y());
            min_z = std::min(min_z, p.get_z());
            max_z = std::max(max_z, p.get_z());
        }

        const double cx = (min_x + max_x) * 0.5;
        const double cy = (min_y + max_y) * 0.5;
        const double cz = (min_z + max_z) * 0.5;
        const double span =
            std::max({max_x - min_x, max_y - min_y, max_z - min_z});
        const double S = std::max(1.0, span * 20.0);

        return {
            tools_3D::point(cx - 3.0 * S, cy, cz - S),
            tools_3D::point(cx + 3.0 * S, cy, cz - S),
            tools_3D::point(cx, cy + 3.0 * S, cz + S),
            tools_3D::point(cx, cy - 3.0 * S, cz + S)};
    }
} // namespace

namespace tools_3D
{
    auto bowyer_watson_3D(const std::vector<tools_3D::point>& points)
        -> std::vector<tools_3D::tetrahedron>
    {
        if (points.size() < 4) return {};

        std::vector<tools_3D::point> sorted_points = points;
        std::sort(sorted_points.begin(), sorted_points.end());
        sorted_points.erase(
            std::unique(sorted_points.begin(), sorted_points.end()),
            sorted_points.end());

        if (sorted_points.size() < 4) return {};

        const int  original_count = static_cast<int>(sorted_points.size());
        const auto super          = super_tetra(sorted_points);
        for (const auto& p : super) sorted_points.push_back(p);

        std::vector<tetra_idx> tetrahedra;
        tetrahedra.push_back(
            {original_count + 0,
             original_count + 1,
             original_count + 2,
             original_count + 3});

        for (int p_idx = 0; p_idx < original_count; ++p_idx)
        {
            const auto& p = sorted_points[p_idx];

            std::vector<int> bad_ids;
            bad_ids.reserve(tetrahedra.size());

            for (int t = 0; t < static_cast<int>(tetrahedra.size()); ++t)
            {
                const auto& tet = tetrahedra[t];
                if (tools_3D::in_sphere(
                        sorted_points[tet.a],
                        sorted_points[tet.b],
                        sorted_points[tet.c],
                        sorted_points[tet.d],
                        p))
                {
                    bad_ids.push_back(t);
                }
            }

            std::
                unordered_map<face_key, std::pair<int, face_idx>, face_key_hash>
                    face_count;
            for (int bad_id : bad_ids)
            {
                for (const auto& f : tetra_faces(tetrahedra[bad_id]))
                {
                    const auto key = make_face_key(f.i, f.j, f.k);
                    auto       it  = face_count.find(key);
                    if (it == face_count.end())
                        face_count.emplace(key, std::make_pair(1, f));
                    else it->second.first += 1;
                }
            }

            std::vector<char> is_bad(tetrahedra.size(), 0);
            for (int bad_id : bad_ids) is_bad[bad_id] = 1;

            std::vector<tetra_idx> next_tetrahedra;
            next_tetrahedra.reserve(tetrahedra.size() + face_count.size());
            for (int t = 0; t < static_cast<int>(tetrahedra.size()); ++t)
                if (!is_bad[t]) next_tetrahedra.push_back(tetrahedra[t]);

            for (const auto& [_, counted] : face_count)
            {
                if (counted.first != 1) continue;

                const auto&  f      = counted.second;
                const double volume = tools_3D::orient3d(
                    sorted_points[f.i],
                    sorted_points[f.j],
                    sorted_points[f.k],
                    sorted_points[p_idx]);

                if (std::abs(volume) <= tools_3D::geometric_epsilon_3D)
                    continue;

                next_tetrahedra.push_back({f.i, f.j, f.k, p_idx});
            }

            tetrahedra.swap(next_tetrahedra);
        }

        std::vector<tools_3D::tetrahedron> result;
        result.reserve(tetrahedra.size());

        for (const auto& tet : tetrahedra)
        {
            if (tet.a >= original_count || tet.b >= original_count ||
                tet.c >= original_count || tet.d >= original_count)
                continue;

            result.emplace_back(
                sorted_points[tet.a],
                sorted_points[tet.b],
                sorted_points[tet.c],
                sorted_points[tet.d]);
        }

        return result;
    }

    auto guibas_stolfi_3D(const std::vector<tools_3D::point>& points)
        -> std::vector<tools_3D::tetrahedron>
    {
        return bowyer_watson_3D(points);
    }
} // namespace tools_3D
