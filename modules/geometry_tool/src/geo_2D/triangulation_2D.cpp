#include "geo_2D/triangulation_2D.h"

#include "csv_parser.h"
#include "geo_2D/predicates_2D.h"
#include "geo_2D/quad_edge_2D.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <ranges>
#include <set>
#include <unordered_set>
#include <utility>

namespace
{
    class guibas_stolfi_triangulator
    {
        std::vector<tools_2D::point>     points_;
        tools_2D::detail::quad_edge_mesh mesh_;

        auto left_of(
            const tools_2D::point&                  p,
            tools_2D::detail::quad_edge_mesh::edge* e) const -> bool
        {
            return tools_2D::orient2d(
                       points_[e->origin],
                       points_[e->dest()],
                       p) > tools_2D::geometric_epsilon;
        }

        auto right_of(
            const tools_2D::point&                  p,
            tools_2D::detail::quad_edge_mesh::edge* e) const -> bool
        {
            return tools_2D::orient2d(
                       points_[e->origin],
                       points_[e->dest()],
                       p) < -tools_2D::geometric_epsilon;
        }

        auto valid(
            tools_2D::detail::quad_edge_mesh::edge* e,
            tools_2D::detail::quad_edge_mesh::edge* basel) const -> bool
        {
            return right_of(points_[e->dest()], basel);
        }

        auto build(
            std::size_t l,
            std::size_t r)
            -> std::pair<
                tools_2D::detail::quad_edge_mesh::edge*,
                tools_2D::detail::quad_edge_mesh::edge*>
        {
            using edge_ptr = tools_2D::detail::quad_edge_mesh::edge*;

            if (r - l == 1)
            {
                edge_ptr a =
                    mesh_.make_edge(static_cast<int>(l), static_cast<int>(r));
                return {a, a->sym()};
            }

            if (r - l == 2)
            {
                edge_ptr a = mesh_.make_edge(
                    static_cast<int>(l),
                    static_cast<int>(l + 1));
                edge_ptr b = mesh_.make_edge(
                    static_cast<int>(l + 1),
                    static_cast<int>(r));
                mesh_.splice(a->sym(), b);

                const double o =
                    tools_2D::orient2d(points_[l], points_[l + 1], points_[r]);

                if (o > tools_2D::geometric_epsilon)
                {
                    mesh_.connect(b, a);
                    return {a, b->sym()};
                }

                if (o < -tools_2D::geometric_epsilon)
                {
                    edge_ptr c = mesh_.connect(b, a);
                    return {c->sym(), c};
                }

                return {a, b->sym()};
            }

            const std::size_t mid = l + (r - l) / 2;
            auto [ldo, ldi]       = build(l, mid);
            auto [rdi, rdo]       = build(mid + 1, r);

            while (true)
            {
                if (left_of(points_[rdi->origin], ldi)) ldi = ldi->l_next();
                else if (right_of(points_[ldi->origin], rdi))
                    rdi = rdi->r_prev();
                else break;
            }

            edge_ptr basel = mesh_.connect(rdi->sym(), ldi);

            if (ldi->origin == ldo->origin) ldo = basel->sym();
            if (rdi->origin == rdo->origin) rdo = basel;

            while (true)
            {
                edge_ptr lcand = basel->sym()->o_next();
                if (valid(lcand, basel))
                {
                    while (valid(lcand->o_next(), basel) &&
                           tools_2D::in_circle(
                               points_[basel->dest()],
                               points_[basel->origin],
                               points_[lcand->dest()],
                               points_[lcand->o_next()->dest()]))
                    {
                        edge_ptr t = lcand->o_next();
                        mesh_.delete_edge(lcand);
                        lcand = t;
                    }
                }

                edge_ptr rcand = basel->o_prev();
                if (valid(rcand, basel))
                {
                    while (valid(rcand->o_prev(), basel) &&
                           tools_2D::in_circle(
                               points_[basel->dest()],
                               points_[basel->origin],
                               points_[rcand->dest()],
                               points_[rcand->o_prev()->dest()]))
                    {
                        edge_ptr t = rcand->o_prev();
                        mesh_.delete_edge(rcand);
                        rcand = t;
                    }
                }

                const bool lvalid = valid(lcand, basel);
                const bool rvalid = valid(rcand, basel);

                if (!lvalid && !rvalid) break;

                if (!lvalid || (rvalid && tools_2D::in_circle(
                                              points_[lcand->dest()],
                                              points_[lcand->origin],
                                              points_[rcand->origin],
                                              points_[rcand->dest()])))
                {
                    basel = mesh_.connect(rcand, basel->sym());
                }
                else
                {
                    basel = mesh_.connect(basel->sym(), lcand->sym());
                }
            }

            return {ldo, rdo};
        }

        auto collect_triangles() const -> std::vector<std::array<
            int,
            3>>
        {
            std::vector<std::array<int, 3>>   faces;
            std::unordered_set<std::uint64_t> seen;

            auto encode = [](int a, int b, int c) -> std::uint64_t
            {
                std::array<std::uint32_t, 3> v{
                    static_cast<std::uint32_t>(a),
                    static_cast<std::uint32_t>(b),
                    static_cast<std::uint32_t>(c)};
                std::sort(v.begin(), v.end());
                return (static_cast<std::uint64_t>(v[0]) << 42) ^
                       (static_cast<std::uint64_t>(v[1]) << 21) ^
                       static_cast<std::uint64_t>(v[2]);
            };

            for (auto* base : mesh_.primal_edges())
            {
                for (auto* e : {base, base->rot, base->sym(), base->inv_rot()})
                {
                    if (!e->alive || e->origin < 0 || e->dest() < 0) continue;

                    auto* e1 = e->l_next();
                    auto* e2 = e1->l_next();

                    if (!e1->alive || !e2->alive) continue;
                    if (e1->origin < 0 || e1->dest() < 0 || e2->origin < 0 ||
                        e2->dest() < 0)
                        continue;
                    if (e2->l_next() != e) continue;

                    const int a = e->origin;
                    const int b = e->dest();
                    const int c = e1->dest();

                    if (a == b || b == c || c == a) continue;

                    if (tools_2D::orient2d(
                            points_[a],
                            points_[b],
                            points_[c]) <= tools_2D::geometric_epsilon)
                        continue;

                    const std::uint64_t key = encode(a, b, c);
                    if (seen.insert(key).second) faces.push_back({a, b, c});
                }
            }

            return faces;
        }

        public:

        explicit guibas_stolfi_triangulator(std::vector<tools_2D::point> points)
            : points_(std::move(points))
        {}

        auto sorted_points() const -> const std::vector<tools_2D::point>&
        {
            return points_;
        }

        auto triangulate() -> std::vector<std::array<
            int,
            3>>
        {
            if (points_.size() < 3) return {};

            build(0, points_.size() - 1);
            return collect_triangles();
        }
    };

    auto super_triangle(const std::vector<tools_2D::point>& points)
    {
        auto [min_x, max_x] = std::ranges::minmax(
            points |
            std::views::transform(std::mem_fn(&tools_2D::point::get_x)));
        auto [min_y, max_y] = std::ranges::minmax(
            points |
            std::views::transform(std::mem_fn(&tools_2D::point::get_y)));

        double cx = (min_x + max_x) / 2.0;
        double cy = (min_y + max_y) / 2.0;
        double L  = std::max(max_x - min_x, max_y - min_y);

        double alpha = 20.0;
        double S     = std::max(1.0, alpha * L);

        tools_2D::point bl(cx - S / 2.0, cy - S / 2.0);
        tools_2D::point br(cx + S / 2.0, cy - S / 2.0);
        tools_2D::point top_mid(cx, cy + S / 2.0);

        return std::tuple(bl, br, top_mid);
    }
} // namespace

namespace tools_2D
{
    auto boyer_watson_2D(const std::vector<tools_2D::point>& points)
        -> std::vector<tools_2D::triangle>
    {
        if (points.size() < 3) return {};

        auto [p1, p2, p3] = super_triangle(points);
        std::vector<tools_2D::triangle> all_triangles{
            tools_2D::triangle{p1, p2, p3}};

        for (const auto& p : points)
        {
            std::set<tools_2D::segment> polygon;

            std::erase_if(
                all_triangles,
                [&](const tools_2D::triangle& tri)
                {
                    if (tri.circum_circle().contains(p))
                    {
                        for (const auto& edge : tri.get_edges())
                        {
                            if (!polygon.erase(edge)) polygon.insert(edge);
                        }
                        return true;
                    }
                    return false;
                });

            for (const auto& edge : polygon)
                all_triangles.emplace_back(p, edge);
        }

        std::erase_if(
            all_triangles,
            [&](const tools_2D::triangle& tri)
            {
                for (const auto& vertex : {p1, p2, p3})
                    if (tri.containsVertex(vertex)) return true;
                return false;
            });

        return all_triangles;
    }

    auto guibas_stolfi_2D(const std::vector<tools_2D::point>& points)
        -> std::vector<tools_2D::triangle>
    {
        if (points.size() < 3) return {};

        std::vector<tools_2D::point> sorted_points = points;
        std::sort(
            sorted_points.begin(),
            sorted_points.end(),
            [](const tools_2D::point& lhs, const tools_2D::point& rhs)
            {
                if (lhs.get_x() != rhs.get_x())
                    return lhs.get_x() < rhs.get_x();
                return lhs.get_y() < rhs.get_y();
            });

        sorted_points.erase(
            std::unique(
                sorted_points.begin(),
                sorted_points.end(),
                [](const tools_2D::point& lhs, const tools_2D::point& rhs)
                {
                    return lhs.get_x() == rhs.get_x() &&
                           lhs.get_y() == rhs.get_y();
                }),
            sorted_points.end());

        if (sorted_points.size() < 3) return {};

        guibas_stolfi_triangulator triangulator(std::move(sorted_points));
        const auto                 faces      = triangulator.triangulate();
        const auto&                sorted_ref = triangulator.sorted_points();

        std::vector<tools_2D::triangle> triangles;
        triangles.reserve(faces.size());
        for (const auto& [a, b, c] : faces)
            triangles.emplace_back(sorted_ref[a], sorted_ref[b], sorted_ref[c]);

        return triangles;
    }

    auto read_and_triangulate() -> std::vector<tools_2D::triangle>
    {
        auto coords = read_csv_coords("points.csv");

        if (coords.size() <= 3)
            throw std::invalid_argument(
                "At least 4 points are required for triangulation.");

        std::vector<tools_2D::point> points;
        points.reserve(coords.size());

        for (const auto [x, y] : coords) points.emplace_back(x, y);

        return boyer_watson_2D(points);
    }
} // namespace tools_2D
